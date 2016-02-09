// $Id: catfile.cpp,v 1.4 2015-02-11 12:28:57-08 - - $

//
// NAME
//    catfile - display files to stdout
//
// SYNOPSIS
//    catfile [filename...]
//
// DESCRIPTION
//    Display contents of files given on the command line.  If no
//    files are specified, display stdin.  Interpret "-" as the 
//    name of stdin.
//

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <libgen.h>

void catfile (istream& infile) {
   for(;;) {
      string line;
      getline (infile, line);
      if (infile.eof()) break;
      cout << line << endl;
   }
}

int main (int argc, char** argv) {
   int status = 0;
   string progname (basename (argv[0]));
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() == 0) catfile (cin);
   else {
      for (const auto& filename: args) {
         if (filename == "-") catfile (cin);
         else {
            ifstream infile (filename);
            if (infile.fail()) {
               status = 1;
               cerr << progname << ": " << filename << ": "
                    << strerror (errno) << endl;
            }else {
               catfile (infile);
               infile.close();
            }
         }
      }
   }
   return status;
}
