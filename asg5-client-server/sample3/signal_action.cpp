// $Id: signal_action.cpp,v 1.6 2014-05-27 17:16:42-07 - - $

/*
 * This program was completed using paired programming.
 * Patrick Mathieu - pmathieu
 * Louie Chatta - lchatta
 */

#include <cstring>
#include <string>
#include <unordered_map>
using namespace std;

#include "signal_action.h"

signal_action::signal_action (int signal, void (*handler) (int)) {
   action.sa_handler = handler;
   sigfillset (&action.sa_mask);
   action.sa_flags = 0;
   int rc = sigaction (signal, &action, nullptr);
   if (rc < 0) throw signal_error (signal);
}

vector<string> util::split(const string& line,
  const string& delim){
   vector<string> words;
   size_t end = 0;
   for(;;){
    size_t start = line.find_first_not_of (delim, end);
   if (start == string::npos) break;
   end = line.find_first_of (delim, start);
   words.push_back (line.substr (start, end - start));
  } return words;

}

signal_error::signal_error (int signal):
              runtime_error (string ("signal_error(")
                             + strsignal (signal) + ")"),
              signal(signal) {}

