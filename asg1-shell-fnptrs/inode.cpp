// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $
// Brian Lin bjlin
// Yunyi Ding yding13

#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "inode.h"

int inode::next_inode_nr {1};

inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type)
{
	switch (type) {
		case PLAIN_INODE:
			contents = make_shared<plain_file>();
			break;
		case DIR_INODE:
			contents = make_shared<directory>();
			break;
	}
	DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
	DEBUGF ('i', "inode = " << inode_nr);
	return inode_nr;
}

inode_t inode::get_type(){
	return type;
}

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
	plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
	if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
	return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
	directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
	if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
	return dirptr;
}

size_t plain_file::size() const {
	size_t size {0};
	size += data.size();
	for(size_t i = 0; i < data.size(); i++) size += data[i].size();
	DEBUGF ('i', "size = " << size);
	return size;
}

const wordvec& plain_file::readfile() const {
	DEBUGF ('i', data);
	return data;
}

void plain_file::writefile (const wordvec& words) {
	DEBUGF ('i', words);
	data = words;
}

size_t directory::size() const {
	size_t size {0};
	size += dirents.size();
	DEBUGF ('i', "size = " << size);
	return size;
}

void directory::remove (const string& filename) {
	DEBUGF ('i', filename);
	if(dirents.count(filename) == 0) throw yshell_exn("rm: cannot remove \'" + filename + "\': No such file or directory");
	dirents.erase(filename);
}

inode_state::inode_state() {
	DEBUGF ('i', "root = " << root << ", cwd = " << cwd
		<< ", prompt = \"" << prompt << "\"");
}

ostream& operator<< (ostream& out, const inode_state& state) {
	out << "inode_state: root = " << state.root
		<< ", cwd = " << state.cwd;
	return out;
}

inode& directory::mkdir (const string& dirname){
	if(dirents.count(dirname) != 0) throw yshell_exn("mkdir: file or directory already exists");
	inode* newnode = new inode(DIR_INODE);
	directory_ptr_of(newnode->contents)->insert_dirents(".", inode_ptr(newnode));
	insert_dirents(dirname, inode_ptr(newnode));
	return *dirents.at(dirname);
}

inode& directory::mkfile (const string& filename){
	if(dirents.count(filename) != 0)
		if(find_inode(filename)->get_type() == DIR_INODE)
			throw yshell_exn("mkfile: \'"+filename+"\' is already a directory");
	inode* newnode = new inode(PLAIN_INODE);
	insert_dirents(filename, inode_ptr(newnode));
	return *dirents.at(filename);
}

void directory::set_parent(inode_ptr parent){
	insert_dirents("..", parent);
}

void directory::insert_dirents(const string& nodename, inode_ptr newnode){
	dirents.insert(make_pair(nodename, newnode));
}

inode_ptr directory::find_inode(const string& nodename){
	wordvec path = split(nodename, "/");
	if(path.size() == 0) return dirents.at(".");
	inode_ptr curr;
	try{curr = dirents.at(path[0]);}
	catch(std::out_of_range&){throw yshell_exn("pathname does not exist");}
	for(int i = 1; i < (signed)path.size(); i++){
		try{curr = directory_ptr_of(curr->contents)->dirents.at(path[i]);}
		catch(std::out_of_range&){throw yshell_exn("pathname does not exist");}
		catch(std::invalid_argument&){throw yshell_exn(path[i]+" is a file");}
	} //if only name is specified, for-loop is skipped
	return curr;
}

void directory::ls(){
	for(map<string,inode_ptr>::iterator i = dirents.begin(); i != dirents.end(); i++){
		std::printf("%6d %6d ", i->second->get_inode_nr(), (int)i->second->contents->size());
		cout << i->first;
		if(i->second->get_type() == DIR_INODE && i->first != "." && i->first != "..") cout << "/";
		cout << "\n";
	}
}

string directory::get_name(inode_ptr node){
	for(map<string,inode_ptr>::iterator i = dirents.begin(); i != dirents.end(); i++)
		if(i->second == node) return i->first;
	return nullptr;
}

wordvec directory::get_dirs(){
	wordvec dirs;
	for(map<string,inode_ptr>::iterator i = dirents.begin(); i != dirents.end(); i++)
		if(i->second->get_type() == DIR_INODE && i->first != "." && i->first != "..")
			dirs.push_back(i->first);
	return dirs;
}

wordvec directory::get_chinodes(){
	wordvec chinodes;
	for(map<string,inode_ptr>::iterator i = dirents.begin(); i != dirents.end(); i++)
		if(i->first != "." && i->first != "..")
			chinodes.push_back(i->first);
	return chinodes;
}
