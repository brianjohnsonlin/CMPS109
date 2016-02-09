// $Id: interp.cpp,v 1.18 2015-02-19 16:50:37-08 - - $

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

unordered_map<string,interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"border" , &interpreter::do_border   },
   {"moveby" , &interpreter::do_moveby   },
};

unordered_map<string,interpreter::factoryfn>
interpreter::factory_map {
   {"text"          , &interpreter::make_text          },
   {"ellipse"       , &interpreter::make_ellipse       },
   {"circle"        , &interpreter::make_circle        },
   {"polygon"       , &interpreter::make_polygon       },
   {"rectangle"     , &interpreter::make_rectangle     },
   {"square"        , &interpreter::make_square        },
   {"diamond"       , &interpreter::make_diamond       },
   {"triangle"      , &interpreter::make_triangle      },
   {"right_triangle", &interpreter::make_right_triangle},
   {"isosceles"     , &interpreter::make_isosceles     },
   {"equilateral"   , &interpreter::make_equilateral   }
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

interpreter::shape_map interpreter::objmap;

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}

void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   rgbcolor color {begin[0]};
   window::push_back(*(new object(itor->second, where, color)));
}

void interpreter::do_border(param begin, param end){
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   window::border_color = *begin++;
   window::border_thickness = stoi(*begin);
}

void interpreter::do_moveby(param begin, param end){
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1) throw runtime_error ("syntax error");
   window::moveby = stoi(*begin);
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   if (end - begin < 2) throw runtime_error ("syntax error");
   DEBUGF ('f', range (begin, end));
   auto font = fontcode.find(*begin++);
   if(font == fontcode.end()) 
      throw runtime_error ("no such font");
   string s = *begin++;
   while(begin != end) s += " " + *begin++;
   return make_shared<text> (font->second, s);
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   GLfloat w = stoi(*begin++);
   GLfloat h = stoi(*begin++);
   return make_shared<ellipse> (w, h);
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1) throw runtime_error ("syntax error");
   return make_shared<circle> (stoi(*begin));
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin < 2 || (end - begin) % 2 != 0)
      throw runtime_error ("syntax error");
   vertex_list v;
   while(begin != end){
      GLfloat x = stoi(*begin++);
      GLfloat y = stoi(*begin++);
      v.push_back({x,y});
   }
   return make_shared<polygon> (v);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   GLfloat w = stoi(*begin++);
   GLfloat h = stoi(*begin++);
   return make_shared<rectangle> (w, h);
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1) throw runtime_error ("syntax error");
   return make_shared<square> (stoi(*begin));
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   GLfloat w = stoi(*begin++);
   GLfloat h = stoi(*begin++);
   return make_shared<diamond> (w, h);
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 6) throw runtime_error ("syntax error");
   vertex_list v;
   while(begin != end){
      GLfloat x = stoi(*begin++);
      GLfloat y = stoi(*begin++);
      v.push_back({x,y});
   }
   return make_shared<triangle> (v);
}

shape_ptr interpreter::make_right_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   GLfloat w = stoi(*begin++);
   GLfloat h = stoi(*begin++);
   return make_shared<right_triangle> (w, h);
}

shape_ptr interpreter::make_isosceles (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2) throw runtime_error ("syntax error");
   GLfloat w = stoi(*begin++);
   GLfloat h = stoi(*begin++);
   return make_shared<iscosceles> (w, h);
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1) throw runtime_error ("syntax error");
   return make_shared<equilateral> (stoi(*begin));
}
