// $Id: shape.cpp,v 1.7 2014-05-08 18:32:56-07 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"
#include "graphics.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font, const string& textdata):
      glut_bitmap_font(glut_bitmap_font), textdata(textdata) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}

polygon::polygon (const vertex_list& vert){
   DEBUGF ('c', this);
   vertex_list v;
   GLfloat xavg = 0;
   GLfloat yavg = 0;
   for(size_t i = 0; i < vert.size(); i++){
      xavg += vert[i].xpos; yavg += vert[i].ypos;
   }
   xavg /= vert.size(); yavg /= vert.size();
   for(size_t i = 0; i < vert.size(); i++)
      v.push_back({vert[i].xpos - xavg, vert[i].ypos - yavg});
   vertices = v;
}

rectangle::rectangle (GLfloat width, GLfloat height):
   polygon({
      {width, height}, {0, height},
      {0, 0}, {width, 0}
   }) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (GLfloat width, GLfloat height): 
   polygon({
      {width/2, height}, {width, height/ 2},
      {width/2, 0}, {0, height/2}
   }) {
   DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vertices): polygon(vertices) {
   DEBUGF ('c', this);
}

right_triangle::right_triangle (GLfloat width, GLfloat height):
   triangle({ {0, 0}, {width, 0}, {0, height} }) {
   DEBUGF ('c', this);
}

iscosceles::iscosceles (GLfloat width, GLfloat height):
   triangle({ {0, 0}, {width, 0}, {width/2, height} }) {
   DEBUGF ('c', this);
}

equilateral::equilateral (GLfloat width):
   triangle({ {0, 0}, {width, 0},
            {width/2, (GLfloat)((sqrt(3)/2)*width)}}) {
   DEBUGF ('c', this);
}

void text::draw (const vertex& center,
                 rgbcolor& color, bool outline) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   auto text = reinterpret_cast<const GLubyte*> (textdata.c_str());
   size_t width = glutBitmapLength (glut_bitmap_font, text);
   size_t height = glutBitmapHeight (glut_bitmap_font);
   if(outline){
      glColor3ubv(rgbcolor(window::border_color).ubvec3());
      glLineWidth(window::border_thickness);
      glBegin(GL_LINE_LOOP);
      //commented code is for centered text, but then the test file
      //will be blank because the text will be off the screen,
      //so the code is left aligned
      //glVertex2f (center.xpos + width/2, center.ypos + height/2);
      //glVertex2f (center.xpos - width/2, center.ypos + height/2);
      //glVertex2f (center.xpos - width/2, center.ypos - height/2);
      //glVertex2f (center.xpos + width/2, center.ypos - height/2);
      glVertex2f (center.xpos + width, center.ypos + height);
      glVertex2f (center.xpos        , center.ypos + height);
      glVertex2f (center.xpos        , center.ypos         );
      glVertex2f (center.xpos + width, center.ypos         );
      glEnd();
   }else{
      glColor3ubv (color.ubvec3());
      //glRasterPos2f (center.xpos - width/2, center.ypos - height/2);
      glRasterPos2f (center.xpos, center.ypos);
      glutBitmapString (glut_bitmap_font, text);
   }
}

void ellipse::draw (const vertex& center,
                    rgbcolor& color, bool outline) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glColor3ubv(color.ubvec3());
   glEnable (GL_LINE_SMOOTH);
   if(outline){
      glColor3ubv(rgbcolor(window::border_color).ubvec3());
      glLineWidth(window::border_thickness);
      glBegin(GL_LINE_LOOP);
   }else{
      glColor3ubv(color.ubvec3());
      glBegin(GL_POLYGON);
   }
   for (float theta = 0; theta < 2 * M_PI; theta += (M_PI/16))
      glVertex2f (dimension.xpos/2 * cos (theta) + center.xpos,
                  dimension.ypos/2 * sin (theta) + center.ypos);
   glEnd();
}

void polygon::draw (const vertex& center,
                    rgbcolor& color, bool outline) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glEnable (GL_LINE_SMOOTH);
   if(outline){
      glColor3ubv(rgbcolor(window::border_color).ubvec3());
      glLineWidth(window::border_thickness);
      glBegin(GL_LINE_LOOP);
   }else{
      glColor3ubv(color.ubvec3());
      glBegin(GL_POLYGON);
   }
   for(size_t i = 0; i < vertices.size(); i++)
      glVertex2f(vertices[i].xpos + center.xpos,
                 vertices[i].ypos + center.ypos);
   glEnd();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

