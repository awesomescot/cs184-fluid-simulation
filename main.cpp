#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include "algebra3.h"
#include "grid.h"

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#ifdef OSX
	#include <GLUT/glut.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/glut.h>
	#include <GL/glu.h>
#endif

// mouse stuff
#define MAX_VERTS 1000
bool dragging;
float xPos, yPos, zPos;
struct verts{float x, y, z;} verts[MAX_VERTS], velVerts[MAX_VERTS];
int numVerts = 0;

int numVelVerts = 0;

int WindowHeight, WindowWidth;

float deltaT = (float)1/30;

using namespace std;

class Viewport;

grid grd;

class Viewport {
private:
  vector<vec3> particles;
  
public:
  int w, h;
  float tx, ty, tz;
  int rotx, roty, rotz;
  bool g, v;
  
  void addParticle(vec3 loc) {
    particles.push_back(loc);
  }
  
  vec3 getLoc(int i) {
    return particles[i];
  }
  
  int numParticles() {
    return particles.size();
  }
  
  void update() {
    for (int i = 0; i < (int)particles.size(); i++) {
      vec3 particleLoc = particles[i];
      vec3 velocity = grd.getVelosity(particleLoc);
      //cout << i << " velocity: " << velocity << endl;
      particles[i] = particles[i] + (velocity * deltaT);
    }
  }
  
  void clearParticles() {
    particles.clear();
  }
  
  void particlesLocs() {
    for (int i = 0; i < (int)particles.size(); i++) {
      cout << i << ": " << particles[i] << endl;
    }
  }
};

void showVelocities() {
  /**
   * render a vector originating in the center of each cubeGrid
   * representing the velocity
   * get point, add vertex, add velocity vector to point, add vertex
   * later, draw all the lines
   * Q: how long to render based on magnitude?
   * for now, just normalize everything and make, say, 1/2 cube length
   * Q: don't we have to clear the velocities and redo at each time step?
   */
  // TODO: button to toggle velocity vectors
  
  vec3 vel, point;
  
  // first, clear it from before
  if (numVelVerts != 0) {
    for (int i = 0; i < numVelVerts; i++) {
      // clear all the velocity vertex info
      velVerts[i].x = 0;
      velVerts[i].y = 0;
      velVerts[i].z = 0;
    }
    numVelVerts = 0;
  }
  
  // center of cube = cube index * cube length + 1/2 cube length
  for (int xi = 0; xi < grd.xSplit; xi++) {
    for (int yi = 0; yi < grd.ySplit; yi++) {
      for (int zi = 0; zi < grd.zSplit; zi++) {
	point = vec3((xi * grd.xCubeSize + grd.xCubeSize / 2), (yi * grd.yCubeSize + grd.yCubeSize / 2), (zi * grd.zCubeSize + grd.zCubeSize / 2));
	velVerts[numVelVerts].x = point[0];
	velVerts[numVelVerts].y = point[1];
	velVerts[numVelVerts].z = point[2];
	numVelVerts++;
	vel = grd.getVelosity(point).normalize(); // normalize for now to keep it simple
	point += vel;
	velVerts[numVelVerts].x = point[0];
	velVerts[numVelVerts].y = point[1];
	velVerts[numVelVerts].z = point[2];
	numVelVerts++;
	
	//draw the line:
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(velVerts[numVelVerts-2].x, velVerts[numVelVerts-2].y, velVerts[numVelVerts-2].z);
	glVertex3f(velVerts[numVelVerts-1].x, velVerts[numVelVerts-1].y, velVerts[numVelVerts-1].z);
	glEnd();
      }
    }
  }
}

void smoothing() {
  int done = 0;

  grid newGrid = grid(grd.x, grd.y, grd.z, grd.xSplit, grd.ySplit, grd.zSplit);

  float potential[grd.xSplit][grd.ySplit][grd.zSplit];

  //this assumes that the cells will always be cubes
  float deltaTao = grd.xCubeSize;
  
  float epsilon = 0.001;
  
  int exitCheck[grd.xSplit][grd.ySplit][grd.zSplit];
  int terminate = 0;

  for (int ei = 0; ei<grd.xSplit; ei++) {
    for (int ej = 0; ej<grd.ySplit; ej++) {
      for (int ek = 0; ek<grd.zSplit; ek++ ) {
	exitCheck[ei][ej][ek] = 0;
      }
    }
  }


  while (terminate != 1) {

    for (int ei = 0; ei<grd.xSplit; ei++) {
      for (int ej = 0; ej<grd.ySplit; ej++) {
	for (int ek = 0; ek<grd.zSplit; ek++ ) {
	  exitCheck[ei][ej][ek] = 0;
	}
      }
    }
    
    for (int xi = 0 ; xi < newGrid.xSplit ; xi++) {
      //int prevX = xi - 1;
      int nextX = xi + 1;
      for (int yi = 0; yi < newGrid.ySplit; yi++) {
	//int prevY = yi - 1;
	int nextY = yi + 1;
	for (int zi = 0; zi < newGrid.zSplit; zi++) {
	  //int prevZ = zi - 1;
	  int nextZ = zi + 1;
	  
	  //float prevPX = 0.0;
	  //if (prevX >= 0) {
	  //  prevPX = potential[xi-1][yi][zi];
	  //}

	  float nextPX = 0.0;
	  if (nextX < newGrid.xSplit) {
	    nextPX = potential [xi+1][yi][zi];
	  }
	  
	  //float prevPY = 0.0;
	  //if (prevY >= 0) {
	  //  prevPY = potential[xi][yi-1][zi];
	  //}

	  float nextPY = 0.0;
	  if (nextY < newGrid.ySplit) {
	    nextPY = potential [xi][yi+1][zi];
	  }
	  
	  //float prevPZ = 0.0;
	  //if (prevZ >= 0) {
	  //  prevPZ = potential[xi][yi][zi-1];
	  //}

	  float nextPZ = 0.0;
	  if (nextZ < newGrid.zSplit) {
	    nextPZ = potential [xi][yi][zi+1];
	  }
	  

	  float uXAdd = 0;
	  if (nextX < grd.xSplit) {
	    uXAdd = grd.cubeGrid[xi+1][yi][zi].u;
	  }


	  float uYAdd = 0;
	  if (nextY < grd.ySplit) {
	    uYAdd = grd.cubeGrid[xi][yi+1][zi].v;
	  }

	  float uZAdd = 0;
	  if (nextZ < grd.zSplit) {
	    uZAdd = grd.cubeGrid[xi][yi][zi+1].w;
	  }
	  
	  float uPart = uXAdd - (grd.cubeGrid[xi][yi][zi]).u;

	  float vPart = uYAdd - (grd.cubeGrid[xi][yi][zi]).v;

	  float zPart = uZAdd - (grd.cubeGrid[xi][yi][zi]).w;

	  float deltaDotU = (1/deltaTao) * (uPart + vPart + zPart);


	  //float deltaDotU = (1/deltaTao)*((uXAdd - (grd.cubeGrid([xi][yi][zi])).u) + (uYAdd - (grd.cubeGrid[xi][yi][zi]).v) + (uZAdd - (grd.cubeGrid[xi][yi][zi]).w));
	  
	  float oldPotential = potential [xi][yi][zi];

	  potential[xi][yi][zi] = ((2/(8/(deltaTao*deltaTao)))*(-deltaDotU + (1/(deltaTao*deltaTao))* (nextPX + potential[xi][yi][zi] + nextPY + potential[xi][yi][zi] + nextPZ + potential[xi][yi][zi]))) - potential[xi][yi][zi];
	
	  float numerator = fabs(potential[xi][yi][zi]) - fabs(oldPotential);
	  
	  float denominator = fabs(potential[xi][yi][zi]) + fabs(oldPotential);
	  float numDiv = numerator/denominator;
	  
	  float epsilonCheck = fabs(numDiv);

	  //	  float epsilonCheck = fabs((fabs(potential[xi][y1][zi]) - fabs(oldPotential))/(fabs(potential[xi][yi][zi]) + fabs(oldPotential)));

	  if (epsilonCheck >= epsilon) {
	    exitCheck[xi][yi][zi] == 1;
	  }

	}
      }
    } 
   
    int done = 1;

    for (int ei = 0; ei<grd.xSplit; ei++) {
      for (int ej = 0; ej<grd.ySplit; ej++) {
	for (int ek = 0; ek<grd.zSplit; ek++ ) {
	  if (exitCheck[ei][ej][ek] == 0) {
	    done = 0;
	  }
	  else {
	    done = 1;
	  }
	  if (done == 1) {
	    break;
	  }
	}
	if (done == 1) {
	  break;
	}
      }
      if (done == 1) {
	break;
      }
    }

    if (done == 1) {
      terminate == 0;
    }
    if (done == 0) {
      terminate == 1;
    }
  }
	
      
  

  for (int xi = 1; xi < grd.xSplit; xi++) {
    int nextX = xi + 1;
    for (int yi = 1; yi< grd.ySplit; yi++) {
      int nextY = yi + 1;
      for (int zi = 1; zi < grd.ySplit; zi++) {	
	//float dTX = deltaTao;
	//float dTY = deltaTao;
	//float dTZ = deltaTao;


	//int nextZ = zi + 1;

	//float nextPX = 0.0;
	//if (nextX < grd.xSplit) {
	//  nextPX = potential[xi+1][yi][zi];
	//  dTX = dTX/2;
	//}


	//float nextPY = 0.0;
	//if (nextY < grd.ySplit) {
	//  nextPY = potential[xi][yi+1][zi];
	//  dTY = dTY/2;
	//}

	//float nextPZ = 0.0;
	//if (nextZ < grd.ySplit) {
	//  nextPZ = potential[xi][yi][zi+1];
	//  dTZ = dTZ/2;
	//}


	grd.cubeGrid[xi][yi][zi].u = grd.cubeGrid[xi][yi][zi].u - (potential[xi][yi][zi]-potential[xi-1][yi][zi]);
	grd.cubeGrid[xi][yi][zi].v = grd.cubeGrid[xi][yi][zi].v - (potential[xi][yi][zi]-potential[xi][yi-1][zi]);
	grd.cubeGrid[xi][yi][zi].w = grd.cubeGrid[xi][yi][zi].w - (potential[xi][yi][zi]-potential[xi][yi][zi-1]);
      }
    }
  }

  
}


void advection() {
  /*
   * velocity advection(){
   *   for every velocity
   *     calculate xyz vector for that side
   *     move negative xyz*delta T in that direction
   *     calculate the velocity at that spot
   *     copy that velocity to the new grid at our original location(a sides velocity vector)
   *
   * getVelosity on one of the face centers
   * go backwards along velocity vector by delta T time
   * getVelosity at this previous point
   * take the appropriate part of the previous vector, put into a blank copy of the grid
   * repeat for every center of every face of every cube in the grid
   * done
   * no particular timestep length or speed of velocity yet
   *
   
   */
  // grd = our global grid
  grid newGrid = grid(grd.x, grd.y, grd.z, grd.xSplit, grd.ySplit, grd.zSplit);
  vec3 vel, point;
  
  // xi, yi, zi = indices for the cubes in the grid
  for (int xi = 0; xi < grd.x; xi++) { // < or <= ? depends on how to handle border cases
    for (int yi = 0; yi < grd.y; yi++) {
      for (int zi = 0; zi < grd.z; zi++) {
	// copy old grid info into new grid while we're at it
	newGrid.cubeGrid[xi][yi][zi].temp = grd.cubeGrid[xi][yi][zi].temp;
	
	// iterate through the x, y, and z faces (other three faces handled by other cubes)
	// take point on face, get velocity, move back by (vel * deltaT), get new velocity, update on new grid
	// TODO: what happens if we end up outside of the grid?
	
	// x/u face
	// multiply by cube size to get correct input (want point in the grid, not cube index) for getVelosity
	point = vec3(xi * grd.xCubeSize, yi * grd.yCubeSize + (grd.yCubeSize)/2, zi * grd.zCubeSize + (grd.zCubeSize)/2);
	vel = grd.getVelosity(point);
	point = point - (vel * deltaT);
	vel = grd.getVelosity(point);
	newGrid.cubeGrid[xi][yi][zi].u = vel[0];
	
	// y/v face
	// multiply by cube size to get correct input (want point in the grid, not cube index) for getVelosity
	point = vec3(xi * grd.xCubeSize + (grd.xCubeSize)/2, yi * grd.yCubeSize, zi * grd.zCubeSize + (grd.zCubeSize)/2);
	vel = grd.getVelosity(point);
	point = point - (vel * deltaT);
	vel = grd.getVelosity(point);
	newGrid.cubeGrid[xi][yi][zi].v = vel[1];
	
	// z/w face
	// multiply by cube size to get correct input (want point in the grid, not cube index) for getVelosity
	point = vec3(xi * grd.xCubeSize + (grd.xCubeSize)/2, yi * grd.yCubeSize + (grd.yCubeSize)/2, zi * grd.zCubeSize);
	vel = grd.getVelosity(point);
	point = point - (vel * deltaT);
	vel = grd.getVelosity(point);
	newGrid.cubeGrid[xi][yi][zi].w = vel[2];
      }
    }
  }
  
  grd = newGrid;
}

vec3 GetOGLPos(int x, int y) {
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat winX, winY, winZ;
  GLdouble posX, posY, posZ;
  
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  glGetIntegerv( GL_VIEWPORT, viewport );
  
  winX = (float)x;
  winY = (float)viewport[3] - (float)y;
  glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
  
  gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
  
  return vec3(posX, posY, posZ);
}

void updatePos(int x, int y) {
  // xPos = 1.0 - ((float)x)/((float)(WindowWidth/2 - 1));
  // yPos = 1.0 - ((float)y)/((float)(WindowHeight/2 -1));
  vec3 pos = GetOGLPos(x, y);
  xPos = pos[0];
  yPos = pos[1];
  zPos = pos[2];
}

/* original
   void dragMouse(int button, int state, int x, int y) {
   if (button == GLUT_LEFT_BUTTON) {
   if (state == GLUT_DOWN) {
   dragging = true;
   updatePos(x, y);
   // cout << x << " " << y << " " << xPos << " " << yPos << endl;
   } else if (state == GLUT_UP && dragging) {
   if (numVerts < MAX_VERTS) {
   // cout << xPos << " " << yPos << endl;
   verts[numVerts].x = (float) xPos;
   verts[numVerts].y = (float) yPos;
   verts[numVerts].z = (float) zPos;
   // cout << verts[numVerts].x << " " << verts[numVerts].y << endl;
   numVerts++;
   }
   dragging = false;
   glutPostRedisplay();
   }
   }
   }
*/
void dragMouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      dragging = true;
      updatePos(x, y);
      verts[numVerts].x = (float) xPos;
      verts[numVerts].y = (float) yPos;
      verts[numVerts].z = (float) zPos;
      numVerts++;
    } else if (state == GLUT_UP && dragging) {
      if (numVerts < MAX_VERTS) {
	verts[numVerts].x = (float) xPos;
	verts[numVerts].y = (float) yPos;
	verts[numVerts].z = (float) zPos;
	numVerts++;
	dragging = false;
      }
      glutPostRedisplay();
    }
  }
}

void moveMouse(int x, int y) {
  if (dragging) {
    updatePos(x, y);
    glutPostRedisplay();
  }
}


Viewport viewport;

void myReshape(int w, int h) {
  WindowHeight = (h>1) ? h : 2;
  WindowWidth = (w>1) ? w : 2;
  viewport.w = w;
  viewport.h = h;
  GLfloat ratio = (GLfloat)w/(GLfloat)h;
  
  glViewport(0, 0, viewport.w, viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  //glOrtho(-2, 2, -2, 2, 1, 100);
  gluPerspective(45, ratio, 1, 100);
}

void initScene() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
  glEnable(GL_DEPTH_TEST);
  
  // create grid
  grd = grid(1, 1, 1, 5, 5, 5);
  grd.cubeGrid[1][1][1].u = .5;
  grd.cubeGrid[1][1][1].v = .25;
  grd.cubeGrid[1][2][1].w = .25;
  
  viewport.tx = -.85*grd.x;
  viewport.ty = -grd.y/5;
  viewport.tz = -2;
  viewport.rotx = -30;
  viewport.roty = 61;
  viewport.rotz = 0;
  viewport.g = true;
  viewport.v = false;
  
  // 5 random particles
  // keep this within the grid from (0, 0, 0) to (-1, 1, 1) for now
  viewport.clearParticles();
  viewport.addParticle(vec3(0.5, 0.5, 0.5));
  viewport.addParticle(vec3(0.2, 0.2, 0.2));
  
  myReshape(viewport.w, viewport.h);
}

void initLights() {
  glEnable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  
  GLfloat global_ambient[] = { 0.1f, 0.1f, 0.1f, 0.1f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
  
  //define and enable light 0
  GLfloat ambient[] = {0.1f, 0.1f, 0.1f};
  GLfloat diffuse[] = {0.6f, 0.5f, 0.5f};
  GLfloat specular[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat pos[] = {-3, 0, 2, 1};
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_LIGHT0);
  
  GLfloat ambient1[] = {0.1f, 0.1f, 0.1f};
  GLfloat diffuse1[] = {0.5f, 0.5f, 0.5f};
  GLfloat specular1[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat pos1[] = {3, 0, -2, 1};
  glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
  glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
  glLightfv(GL_LIGHT1, GL_POSITION, pos1);
  glEnable(GL_LIGHT1);
  
  GLfloat mat_specular[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat mat_diffuse[] = {0.0, 0.8, 0.5, 1.0};
  GLfloat mat_ambient[] = {0.0, 0.1, 0.1, 1.0};
  //GLfloat mat_emission[] = {0.2, 0.0, 0.0, 1.0};
  GLfloat mat_shininess = {10.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  
  glEnable(GL_NORMALIZE);
}

void processNormalKeys(unsigned char key, int x, int y) {
  switch(key) {
  case 27 :
    exit(0);
  case 'r' :
    initScene();
    break;
  case 'g' :
    viewport.g = !viewport.g;
    break;
  case 'v' :
    viewport.v = !viewport.v;
    break;
  }
}

void processInputKeys(int key, int x, int y) {
  
  int mod = glutGetModifiers();
  switch(key) {
  case GLUT_KEY_LEFT :
    if (mod == GLUT_ACTIVE_ALT)
      viewport.roty = viewport.roty + 1;
    else
      viewport.tx = viewport.tx - .1;
    break;
  case GLUT_KEY_RIGHT :
    if (mod == GLUT_ACTIVE_ALT)
      viewport.roty = viewport.roty - 1;
    else
      viewport.tx = viewport.tx + .1;
    break;
  case GLUT_KEY_UP :
    if (mod == GLUT_ACTIVE_SHIFT)
      viewport.tz = viewport.tz + .1;
    else if (mod == GLUT_ACTIVE_ALT)
      viewport.rotx = viewport.rotx - 1;
    else
      viewport.ty = viewport.ty - .1;
    break;
  case GLUT_KEY_DOWN :
    if (mod == GLUT_ACTIVE_SHIFT)
      viewport.tz = viewport.tz - .1;
    else if (mod == GLUT_ACTIVE_ALT)
      viewport.rotx = viewport.rotx + 1;
    else
      viewport.ty = viewport.ty + .1;
    break;
  }
}

void myDisplay() {
  
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0, 0, -1, 0, 0, 0, 0, 1, 0);
  
  glTranslatef(viewport.tx, viewport.ty, -viewport.tz);
  glRotatef(viewport.rotx, 1, 0, 0);
  glRotatef(viewport.roty, 0, 1, 0);
  glRotatef(viewport.rotz, 0, 0, 1);
  
  // before drawing, update new particle locations
  //advection();
  smoothing();
  viewport.update();
  
  // start drawing here
  glColor3f(1.0f, 1.0f, 1.0f);
  
  if (viewport.g) {
    //draw grid
    //z-axis aligned
    glBegin(GL_LINES);
    float xs = (float)grd.x/grd.xSplit;
    float ys = (float)grd.y/grd.ySplit;
    float zs = (float)grd.z/grd.zSplit;
    
    // cout << xs << endl;
    for (int i = 0; i < grd.xSplit + 1; i++) {
      for (int j = 0; j < grd.ySplit + 1; j++) {
	glVertex3f(xs*i, ys*j, 0);
	glVertex3f(xs*i, ys*j, grd.z);
      }
    }
    //y-axis aligned
    for (int i = 0; i < grd.xSplit + 1; i++) {
      for (int k = 0; k < grd.zSplit + 1; k++) {
	glVertex3f(xs*i, 0, zs*k);
	glVertex3f(xs*i, grd.y, zs*k);
      }
		}
    //x-axis aligned
    for (int j = 0; j < grd.ySplit + 1; j++) {
      for (int k = 0; k < grd.zSplit + 1; k++) {
	glVertex3f(0, ys*j, zs*k);
	glVertex3f(grd.x, ys*j, zs*k);
      }
    }
    
    // mouse stuff
    for (int i = 0; i < numVerts; i++)
      glVertex3f(verts[i].x, verts[i].y, verts[i].z);
    if (dragging)
      glVertex3f((float) xPos, (float) yPos, (float) zPos);
    
    glEnd();
  }
  
  //show velocities
  if (viewport.v)
    showVelocities();
  
  glEnable(GL_LIGHTING);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  // draw particles
  for (int i = 0; i < viewport.numParticles(); i++){
    
    vec3 part = viewport.getLoc(i);
    
    glPushMatrix();
    glTranslatef(part[0], part[1], part[2]);
    glutSolidSphere(0.05, 50, 50);			// glutSolidSphere(radius, splice(long), splices(lat))
    glPopMatrix();
  }
  
  glutSwapBuffers();
}

void testDisplay(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0, 0.0, 0.0);
  if (numVerts > 1) {
    int i;
    glBegin(GL_LINES);
    for (i = 0; i < numVerts; i++)
      glVertex2f(verts[i].x, verts[i].y);
    glEnd();
  }
  glFlush();
}

void myFrameMove() {
  //nothing here for now
#ifdef _WIN32
  Sleep(10);				//give ~10ms back to OS
#endif
  glutPostRedisplay();	//force glut to call display
}

int main(int argc, char *argv[]) {
  //initialize glut
  glutInit(&argc, argv);
  
  //double-buffered window with red, green, blue
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  
  //initialize viewport size
  viewport.w = 400;
  viewport.h = 400;
  
  //size and position of window
  glutInitWindowSize(viewport.w, viewport.h);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("Particle Simulation");
  
  //initialize scene
  initScene();
  initLights();
  
  //keyboard interaction
  glutKeyboardFunc(processNormalKeys);
  glutSpecialFunc(processInputKeys);
  glutMouseFunc(dragMouse);
  glutMotionFunc(moveMouse);
  
  glutDisplayFunc(myDisplay);
  glutReshapeFunc(myReshape);
  glutIdleFunc(myFrameMove);
  glutMainLoop();
  
  return 0;
}
