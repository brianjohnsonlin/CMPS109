// $Id: graphics.cpp,v 1.11 2014-05-15 16:42:55-07 - - $

#include <iostream>
using namespace std;

#include <GL/freeglut.h>

#include "graphics.h"
#include "util.h"

int window::width = 640; // in pixels
int window::height = 480; // in pixels
vector<object> window::objects;
size_t window::selected_obj = 0;
string window::border_color = "red";
float window::border_thickness = 4;
float window::moveby = 4;

// Executed when window system signals to shut down.
void window::close() {
   DEBUGF ('g', sys_info::execname() << ": exit ("
           << sys_info::exit_status() << ")");
   exit (sys_info::exit_status());
}

// Called to display the objects in the window.
void window::display() {
   glClear (GL_COLOR_BUFFER_BIT);
   for (auto& object: window::objects) object.draw(false);
   window::objects[window::selected_obj].draw(true);
   glutSwapBuffers();
}

// Called when window is opened and when resized.
void window::reshape (int width, int height) {
   DEBUGF ('g', "width=" << width << ", height=" << height);
   window::width = width;
   window::height = height;
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D (0, window::width, 0, window::height);
   glMatrixMode (GL_MODELVIEW);
   glViewport (0, 0, window::width, window::height);
   glClearColor (0.25, 0.25, 0.25, 1.0);
   glutPostRedisplay();
}

// Executed when a regular keyboard key is pressed.
enum {BS=8, TAB=9, ESC=27, SPACE=32, DEL=127};
void window::keyboard (GLubyte key, int x, int y) {
   DEBUGF ('g', "key=" << (unsigned)key << ", x=" << x << ", y=" << y);
   switch (key) {
      case 'Q': case 'q': case ESC:
         window::close();
         break;
      case 'H': case 'h':
         move_object(objects[selected_obj], -1*moveby, 0);
         break;
      case 'J': case 'j':
         move_object(objects[selected_obj], 0, -1*moveby);
         break;
      case 'K': case 'k':
         move_object(objects[selected_obj], 0, moveby);
         break;
      case 'L': case 'l':
         move_object(objects[selected_obj], moveby, 0);
         break;
      case 'N': case 'n': case SPACE: case TAB:
         if(selected_obj + 1 < objects.size())
             selected_obj++;
         else selected_obj = 0;
         break;
      case 'P': case 'p': case BS:
         if(selected_obj > 0)
             selected_obj--;
         else selected_obj = objects.size() - 1;
         break;
      case '0'...'9':
         if(key - '0' < (signed)objects.size())
             selected_obj = key - '0';
         break;
      default:
         cerr << (unsigned)key << ": invalid keystroke" << endl;
         break;
   }
   glutPostRedisplay();
}

void window::move_object(object &o, GLfloat dx, GLfloat dy){
   DEBUGF ('g', "move_object: " << o.center.xpos << "+"  << dx << " " <<
                                   o.center.ypos << "+"  << dy << endl);
   o.move(dx,dy);
   if(o.center.xpos >= window::width) o.center.xpos -= window::width;
   if(o.center.ypos >= window::height)o.center.ypos -= window::height;
   if(o.center.xpos < 0) o.center.xpos += window::width;
   if(o.center.ypos < 0) o.center.ypos += window::height;
}

// Executed when a special function key is pressed.
void window::special (int key, int x, int y) {
   DEBUGF ('g', "key=" << key << ", x=" << x << ", y=" << y);
   switch (key) {
      case GLUT_KEY_LEFT:
         move_object(objects[selected_obj], -1*moveby, 0);
         break;
      case GLUT_KEY_DOWN:
         move_object(objects[selected_obj], 0, -1*moveby);
         break;
      case GLUT_KEY_UP:
         move_object(objects[selected_obj], 0, moveby);
         break;
      case GLUT_KEY_RIGHT:
         move_object(objects[selected_obj], moveby, 0);
         break;
      case GLUT_KEY_F1: if(objects.size() > 1) selected_obj = 1; break;
      case GLUT_KEY_F2: if(objects.size() > 2) selected_obj = 2; break;
      case GLUT_KEY_F3: if(objects.size() > 3) selected_obj = 3; break;
      case GLUT_KEY_F4: if(objects.size() > 4) selected_obj = 4; break;
      case GLUT_KEY_F5: if(objects.size() > 5) selected_obj = 5; break;
      case GLUT_KEY_F6: if(objects.size() > 6) selected_obj = 6; break;
      case GLUT_KEY_F7: if(objects.size() > 7) selected_obj = 7; break;
      case GLUT_KEY_F8: if(objects.size() > 8) selected_obj = 8; break;
      case GLUT_KEY_F9: if(objects.size() > 9) selected_obj = 9; break;
      case GLUT_KEY_F10:if(objects.size() >10) selected_obj =10; break;
      case GLUT_KEY_F11:if(objects.size() >11) selected_obj =11; break;
      case GLUT_KEY_F12:if(objects.size() >12) selected_obj =12; break;
      default:
         cerr << (unsigned)key << ": invalid function key" << endl;
         break;
   }
   glutPostRedisplay();
}

void window::main () {
   static int argc = 0;
   glutInit (&argc, nullptr);
   glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE);
   glutInitWindowSize (window::width, window::height);
   glutInitWindowPosition (128, 128);
   glutCreateWindow (sys_info::execname().c_str());
   glutCloseFunc (window::close);
   glutDisplayFunc (window::display);
   glutReshapeFunc (window::reshape);
   glutSpecialFunc (window::special);
   glutKeyboardFunc (window::keyboard);
   DEBUGF ('g', "Calling glutMainLoop()");
   glutMainLoop();
}
