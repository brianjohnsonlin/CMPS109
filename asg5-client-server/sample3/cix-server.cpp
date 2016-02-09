// $Id: cix-server.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

/*
 * This program was completed using paired programming.
 * Patrick Mathieu - pmathieu
 * Louie Chatta - lchatta
 */

#include <iostream>
using namespace std;

#include <libgen.h>
#include <iostream>
#include <fstream>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

void reply_ls (accepted_socket& client_sock, cix_header& header) {
   FILE* ls_pipe = popen ("ls -l", "r");
   if (ls_pipe == NULL) {
      log << "ls -l: popen failed: " << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = errno;
      send_packet (client_sock, &header, sizeof header);
   }
   string ls_output;
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      ls_output.append (buffer);
   }
   header.cix_command = CIX_LSOUT;
   header.cix_nbytes = ls_output.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   log << "sent " << ls_output.size() << " bytes" << endl;
}
void reply_put(accepted_socket& client_sock, cix_header& header){
   char buffer[header.cix_nbytes +1];
   recv_packet (client_sock, buffer, /*sizeof*/ header.cix_nbytes);
   log << "received header " << header << endl;
   ofstream outfile (header.cix_filename);
   string file_name = header.cix_filename;
   if(outfile){
      buffer[header.cix_nbytes] = '\0';
      for(auto c: buffer)cout<<c;
      cout<<endl;
      outfile.write(buffer, header.cix_nbytes);
      outfile.close();
      header.cix_command = CIX_ACK;
      send_packet(client_sock, &header, sizeof header);
      send_packet (client_sock, buffer, header.cix_nbytes);
      cout << file_name<<": saved on server"<<endl;
   }else{
      header.cix_command = CIX_NAK;
      send_packet(client_sock, &header, sizeof header);
   }
 
}

void reply_get (accepted_socket& client_sock, cix_header& header) {
   string get_output = "";
   ifstream is (header.cix_filename, ifstream::binary);
   int length = 0; 
   if (is == nullptr) {
      log << "get : popen failed: " << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = errno;
      send_packet (client_sock, &header, sizeof header);
      cout<<"sent failure"<<endl;
   }else{
      is.seekg(0, is.end);
      length = is.tellg();
      is.seekg (0, is.beg);  
      char* buffer = new char[length]; 
      is.read(buffer,length);
      if(is){
         cout<<"complete transfer"<<endl;
     
     } else cout<<"error only: "<<is.gcount()<<"read"<<endl;
      is.close();
  
      header.cix_command = CIX_ACK;
      header.cix_nbytes = length;
      log << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
      send_packet (client_sock, buffer, 
        header.cix_nbytes);
      log << "sent " << length << " bytes" << endl;
      delete[] buffer;
   }
}

void reply_rm (accepted_socket& client_sock, cix_header& header) {
   string rm_output = "";
   //failure code not working ?
   int rc = unlink(header.cix_filename);
   if (rc <0) {
      log << "rm : popen failed: " << strerror (rc) << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = rc;
      send_packet (client_sock, &header, sizeof header);
      cout<<"sent failure"<<endl;
   }

   header.cix_command = CIX_ACK;
   header.cix_nbytes = rm_output.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, rm_output.c_str(), rm_output.size());
   log << "sent " << rm_output.size() << " bytes" << endl;
}

bool SIGINT_throw_cix_exit = false;
void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   switch (signal) {
      case SIGINT: case SIGTERM: SIGINT_throw_cix_exit = true; break;
      default: break;
   }
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   signal_action (SIGINT, signal_handler);
   signal_action (SIGTERM, signal_handler);
   int client_fd = args.size() == 0 ? -1 : stoi (args[0]);
   log << "starting client_fd " << client_fd << endl;
   try {
      accepted_socket client_sock (client_fd);
      log << "connected to " << to_string (client_sock) << endl;
      for (;;) {
         if (SIGINT_throw_cix_exit) throw cix_exit();
         cix_header header;
         recv_packet (client_sock, &header, sizeof header);
         log << "received header " << header << endl;
         switch (header.cix_command) {
            case CIX_LS:
               reply_ls (client_sock, header);
               break;
            case CIX_GET:
               reply_get(client_sock, header);
               break;
            case CIX_RM:
               reply_rm(client_sock, header);
               break;
            case CIX_PUT:
               reply_put(client_sock, header);
               break;
            default:
               log << "invalid header from client" << endl;
               log << "cix_nbytes = " << header.cix_nbytes << endl;
               log << "cix_command = " << header.cix_command << endl;
               log << "cix_filename = " << header.cix_filename << endl;
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

