// $Id: cix-server.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

#include <iostream>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/stat.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

string rename(const string & name) {
   string new_name;
   unsigned found = name.find_last_of("/");
   new_name = name.substr(found + 1);

   return new_name;
}

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
   pclose(ls_pipe);
}

void reply_put (accepted_socket& client_sock, cix_header& header) {
   char buffer[header.cix_nbytes + 1];
   recv_packet (client_sock, buffer, header.cix_nbytes);
   log << "received " << header.cix_nbytes << " bytes \n";
   buffer[header.cix_nbytes] = '\0';
   //cout << "from server: " << buffer;
   //write from buffer to file
   string filename(header.cix_filename);
   filename = rename(filename);
   filename = "server_" + filename;
   //cout << "filename is : " << f ilename << endl;
   //
   ofstream myfile;
   myfile.open(filename);
   myfile << buffer;
   myfile.close();
   //reply to client
   string put_out = "201 OK\n";
   header.cix_command = CIX_ACK;
   header.cix_nbytes = put_out.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, put_out.c_str(), put_out.size());
   log << "sent " << put_out.size() << " bytes" << endl;
}

void reply_get (accepted_socket& client_sock, cix_header& header) {
   string filename(header.cix_filename);
   cout << filename << endl;
   //write from file to buffer
   FILE * pfile;
   char buff[5000];
   pfile = fopen(filename.c_str(), "r");
   if (pfile == nullptr)
      perror("404 not found");
   else
      while (! feof(pfile)) {
         if (fgets(buff, 5000, pfile) == NULL)
            break;
      }
   fclose(pfile);
   //get file size
   struct stat st;
   stat (filename.c_str(), &st);
   size_t size = st.st_size;
   header.cix_nbytes = size;         
   header.cix_command = CIX_FILE;
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, buff, header.cix_nbytes);
}

void reply_rm (accepted_socket& client_sock, cix_header& header) {
   string filename(header.cix_filename);
   unlink(filename.c_str());
   //reply to client
   string rm_out = "delete successful\n";
   header.cix_command = CIX_ACK;
   header.cix_nbytes = rm_out.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, rm_out.c_str(), rm_out.size());
   log << "sent " << rm_out.size() << " bytes" << endl;
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
            case CIX_PUT:
               reply_put (client_sock, header);
               break;
            case CIX_GET:
               reply_get (client_sock, header);
               break;
            case CIX_RM:
               reply_rm (client_sock, header);
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

