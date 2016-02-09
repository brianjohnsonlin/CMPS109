// $Id: cix-client.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

/*
 * This program was completed using paired programming.
 * Patrick Mathieu - pmathieu
 * Louie Chatta - lchatta
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"put" , CIX_PUT },
   {"rm"  , CIX_RM  },
   {"get" , CIX_GET },
   
};

void cix_help() {
   static vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.cix_command = CIX_LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.cix_command != CIX_LSOUT) {
      log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      char buffer[header.cix_nbytes + 1];
      recv_packet (server, buffer, header.cix_nbytes);
      log << "received " << header.cix_nbytes << " bytes" << endl;
      buffer[header.cix_nbytes] = '\0';
      cout << buffer;
   }
}

void cix_get (client_socket& server, const string& info) {
   cix_header header;
   header.cix_command = CIX_GET;
  
   for(size_t i =0;i<info.size();i++)
     header.cix_filename[i]=info[i]; 
   string file_name = header.cix_filename;
  // convert to char array
   cout<<header<<": "<<endl;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.cix_command != CIX_ACK) {
      log << "sent RM, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
   }else {
      ofstream outfile (header.cix_filename); 
      char buffer[header.cix_nbytes + 1];
      recv_packet (server, buffer, header.cix_nbytes);
      log << "received " << header.cix_nbytes << " bytes" << endl;
      buffer[header.cix_nbytes] = '\0';
      for(auto c: buffer)cout<<c;
      cout<<endl;
      outfile.write(buffer, header.cix_nbytes);
      outfile.close();
     cout << file_name<<": saved on local directory"<<endl;
   }
}
void cix_put (client_socket& server, const string& info) {
   cix_header header;
   header.cix_command = CIX_PUT;
   cout<<info<<endl; 
   for(size_t i =0;i<info.size();i++){
     header.cix_filename[i]=info[i];  
   }
    //load filename  
      ifstream is (header.cix_filename, ifstream::binary); 
      int length = 0;
   if (is == nullptr) {
      log << "get : popen failed: " << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = errno;
      send_packet (server, &header, sizeof header);
      cout<<"sent failure"<<endl;
   }else{
       is.seekg(0, is.end);
       length = is.tellg();
       is.seekg (0, is.beg);
       char* buffer = new char[length];
       is.read(buffer,length);
       header.cix_nbytes = length;
       log << "sending header " << header << endl;
       send_packet (server, &header, sizeof header);
       send_packet (server, buffer,
        header.cix_nbytes);
       log << "sent " << length << " bytes" << endl;
       recv_packet(server, &header, sizeof header);
       recv_packet(server, buffer, header.cix_nbytes);
       is.close();
       delete[] buffer;
    //
   }
 }

void cix_rm (client_socket& server, const string& info) {
   cix_header header;
   header.cix_command = CIX_RM;
   string file_name = info; 
   strcpy(header.cix_filename, info.c_str());
   //convert to char array
   cout<<header<<": "<<endl;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.cix_command != CIX_ACK) {
      log << "sent RM, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
      cerr<<file_name<<": removal unsuccessfull"<<endl;
     
   }
 if(header.cix_command == CIX_ACK)
  cout<<file_name<<": removed successfully"<<endl;
}
void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

bool SIGINT_throw_cix_exit {false};
void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   switch (signal) {
      case SIGINT: case SIGTERM: SIGINT_throw_cix_exit = true; break;
      default: break;
   }
}

int main (int argc, char** argv) {
   util utilz;
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   signal_action (SIGINT, signal_handler);
   signal_action (SIGTERM, signal_handler);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   //
   in_port_t port = get_cix_server_port (args, 1);
   //this line?
  // port = (in_port_t)1200;
   cout<<"ma port: "<<port<<endl;
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
        
        
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         if (SIGINT_throw_cix_exit) throw cix_exit();
         log << "command " << line << endl;
         vector<string> full_cmd = utilz.split(line, " ");
         string cmd_info;
         string cmd_core;
         if(full_cmd.size()>=2) 
           cmd_info = full_cmd.at(1);
         if(full_cmd.size()>=1)
           cmd_core = full_cmd.at(0);  
         else throw cix_exit();
         const auto& itor = command_map.find (cmd_core);
         cix_command cmd = itor == command_map.end()
                         ? CIX_ERROR : itor->second;
             switch (cmd) {
            case CIX_EXIT:
               throw cix_exit();
               break;
            case CIX_HELP:
               cix_help();
               break;
            case CIX_LS:
               cix_ls (server);
               break;
            case CIX_PUT:
               cix_put(server, cmd_info);
               break;
            case CIX_GET:
               cix_get(server, cmd_info);
               break;
            case CIX_RM:
               cix_rm(server, cmd_info);
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}
