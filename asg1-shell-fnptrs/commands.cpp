// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

#include "commands.h"
#include "debug.h"

commands::commands(): map ({
	{"#"     , fn_comment},
	{"cat"   , fn_cat    },
	{"cd"    , fn_cd     },
	{"echo"  , fn_echo   },
	{"exit"  , fn_exit   },
	{"ls"    , fn_ls     },
	{"lsr"   , fn_lsr    },
	{"make"  , fn_make   },
	{"mkdir" , fn_mkdir  },
	{"prompt", fn_prompt },
	{"pwd"   , fn_pwd    },
	{"rm"    , fn_rm     },
	{"rmr"   , fn_rmr    },
}){}

command_fn commands::at (const string& cmd) {
	// Note: value_type is pair<const key_type, mapped_type>
	// So: iterator->first is key_type (string)
	// So: iterator->second is mapped_type (command_fn)
	command_map::const_iterator result = map.find (cmd);
	if (result == map.end()) {
		throw yshell_exn (cmd + ": no such function");
	}
	return result->second;
}

void fn_comment (inode_state& state, const wordvec& words){
	inode_ptr temp = state.root;
	wordvec tempwords = words;
}

void fn_cat (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() == 1) throw yshell_exn("cat: invalid input");
	for(size_t i = 1; i < words.size(); i++){
		inode_ptr curr;
		if(words[i][0] == '/') curr = state.root;
		else curr = state.cwd;
		curr = directory_ptr_of(curr->contents)->find_inode(words[i]);
		if(curr->get_type() == DIR_INODE) throw yshell_exn("cat: cannot preform cat on a directory");
		wordvec data = plain_file_ptr_of(curr->contents)->readfile();
		for(size_t j = 0; j < data.size(); j++) cout << data[j] << " ";
		cout << "\n";
	}
}

void fn_cd (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() > 2) throw yshell_exn("cd: invalid input");
	if(words.size() == 1){
		state.cwd = state.root;
		return;
	}
	inode_ptr curr;
	if(words[1][0] == '/') curr = state.root;
	else curr = state.cwd;
	state.cwd = directory_ptr_of(curr->contents)->find_inode(words[1]);
}

void fn_echo (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	for(int i = 1; i < (signed)words.size(); i++) cout << words[i] + ' ';
	cout << '\n';
}

void fn_exit (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	//free(&state);
	if(words.size() < 2) exit(0);
	if(words.size() == 2){
		int i;
		try{
			i = stoi(words[1]);
			exit(i);
		}catch(invalid_argument){}
	}
	exit(127);
}

void fn_ls (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() == 1){
		wordvec newwords = words;
		newwords.push_back(".");
		fn_ls(state, newwords);
	}
	for(size_t i = 1; i < words.size(); i++){
		inode_ptr dir = state.cwd;
		if(words[i][0] == '/') dir = state.root;
		dir = directory_ptr_of(dir->contents)->find_inode(words[i]);
		if(dir->get_type() == PLAIN_INODE) throw yshell_exn("ls: cannot preform ls on a file");
		string workdir = "";
		inode_ptr curr = dir;
		while(curr != state.root){
			inode_ptr child = curr;
			curr = directory_ptr_of(curr->contents)->find_inode("..");
			workdir = "/" + directory_ptr_of(curr->contents)->get_name(child) + workdir;
		}
		if(workdir.size() == 0) workdir += "/";
		cout << workdir << ":\n";
		directory_ptr_of(dir->contents)->ls();
	}
}

void fn_lsr (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() == 1){
		wordvec newwords = words;
		newwords.push_back(".");
		fn_lsr(state, newwords);
	}
	for(size_t i = 1; i < words.size(); i++){
		inode_ptr dir = state.cwd;
		if(words[i][0] == '/') dir = state.root;
		dir = directory_ptr_of(dir->contents)->find_inode(words[i]);
		if(dir->get_type() == PLAIN_INODE) throw yshell_exn("lsr: cannot preform ls on a file");
		string workdir = "";
		inode_ptr curr = dir;
		while(curr != state.root){
			inode_ptr child = curr;
			curr = directory_ptr_of(curr->contents)->find_inode("..");
			workdir = "/" + directory_ptr_of(curr->contents)->get_name(child) + workdir;
		}
		if(workdir.size() == 0) workdir += "/";
		wordvec newwords;
		newwords.push_back("lsr");
		newwords.push_back(workdir);
		fn_ls(state, newwords);
		wordvec dirs = directory_ptr_of(dir->contents)->get_dirs();
		for(size_t i = 0; i < dirs.size(); i++){
			newwords.clear();
			newwords.push_back("lsr");
			newwords.push_back(workdir + "/" + dirs[i]);
			fn_lsr(state, newwords);
		}
	}
}

void fn_make (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() == 1) throw yshell_exn("make: invalid input");
	inode_ptr curr;
	if(words[1][0] == '/') curr = state.root;
	else curr = state.cwd;
	wordvec path = split(words[1],"/");
	string nodename = path[path.size()-1];
	string pathname = "";
	for(size_t i = 0; i < path.size() - 1; i++) pathname += path[i] + "/";
	curr = directory_ptr_of(curr->contents)->find_inode(pathname);
	inode newfile = directory_ptr_of(curr->contents)->mkfile(nodename);
	wordvec data;
	for(size_t i = 2; i < words.size(); i++) data.push_back(words[i]);
	plain_file_ptr_of(newfile.contents)->writefile(data);
}

void fn_mkdir (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() != 2) throw yshell_exn("mkdir: invalid input");
	inode_ptr curr;
	if(words[1][0] == '/') curr = state.root;
	else curr = state.cwd;
	wordvec path = split(words[1],"/");
	string nodename = path[path.size()-1];
	string pathname = "";
	for(size_t i = 0; i < path.size() - 1; i++) pathname += path[i] + "/";
	curr = directory_ptr_of(curr->contents)->find_inode(pathname);
	inode newdir = directory_ptr_of(curr->contents)->mkdir(nodename);
	directory_ptr_of(newdir.contents)->set_parent(curr);
}

void fn_prompt (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	string p;
	if(words.size()>1){
		for(int i=1; i < (signed)words.size() - 1; i++) p += words[i] + " ";
		p += words[words.size()-1];
	}else p = "%";
	state.prompt = p;
}

void fn_pwd (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() > 1) throw yshell_exn("pwd: invalid input");
	inode_ptr curr = state.cwd;
	string workdir = "";
	while(curr != state.root){
		inode_ptr child = curr;
		curr = directory_ptr_of(curr->contents)->find_inode("..");
		workdir = "/" + directory_ptr_of(curr->contents)->get_name(child) + workdir;
	}
	if(workdir.size() == 0) workdir += "/";
	cout << workdir << "\n";
}

void fn_rm (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() != 2) throw yshell_exn("rm: invalid input");
	inode_ptr curr;
	if(words[1][0] == '/') curr = state.root;
	else curr = state.cwd;
	wordvec path = split(words[1],"/");
	string nodename = path[path.size()-1];
	string pathname = "";
	for(size_t i = 0; i < path.size() - 1; i++) pathname += path[i] + "/";
	curr = directory_ptr_of(curr->contents)->find_inode(pathname);
	inode_ptr node = directory_ptr_of(curr->contents)->find_inode(nodename);
	if(node->get_type() == DIR_INODE) throw yshell_exn("rm: cannot remove directory, use rmr instead");
	directory_ptr_of(curr->contents)->remove(nodename);
}

void fn_rmr (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if(words.size() != 2) throw yshell_exn("rmr: invalid input");
	inode_ptr curr;
	if(words[1][0] == '/') curr = state.root;
	else curr = state.cwd;
	wordvec path = split(words[1],"/");
	string nodename = path[path.size()-1];
	string pathname = "";
	for(size_t i = 0; i < path.size() - 1; i++) pathname += path[i] + "/";
	curr = directory_ptr_of(curr->contents)->find_inode(pathname);
	inode_ptr node = directory_ptr_of(curr->contents)->find_inode(nodename);
	if(node->get_type() == DIR_INODE){
		wordvec chinodes = directory_ptr_of(node->contents)->get_chinodes();		
		for(size_t i = 0; i < chinodes.size(); i++){
			wordvec newwords;
			newwords.push_back("lsr");
			newwords.push_back(words[1] + "/" + chinodes[i]);
			fn_rmr(state, newwords);
		}
	}
	directory_ptr_of(curr->contents)->remove(nodename);
}

int exit_status_message() {
	int exit_status = exit_status::get();
	cout << execname() << ": exit(" << exit_status << ")" << endl;
	return exit_status;
}

