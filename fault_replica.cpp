#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include "fault_replica.h"
#include "connections.h"
#define port 60000





bool write_to_log_M(string log_file, vector<string> data, vector <Node> group){
	ofstream f (log_file);//flag
	if(f.is_open()){
		string info;
		for(int i = 0; i < group.size(); i++){
			info = group[i].ip_str + " " + log_file + "\n";
			f << info;
		}
		for(int i = 0; i < data.size(); i++){
			f << data[i];
		}

	}
	else{
		return false;
	}
	return true;
}

vector<string> read_from_log_M(string log_file){
	vector<string> Addr_File;
	string temp;
	ifstream f(log_file);
	if(f.is_open()){
		while(!f.eof()){
			getline(f,temp);
			Addr_File.push_back(temp);
		}
		f.close();
	}
	return Addr_File;
}




//modify the original from main

bool getFileRequest_M( string sdfsfilename, string localfilename ,vector<Node> members)
{
	char buf[1024];
	for(int i=0; i < members.size(); i++)//members?
	{
		bzero(buf, 1024);
		int connectionFd;
		connect_to_server(members[i].ip_str.c_str(), port+3, &connectionFd);
	   // getFileSocket = listen_socket(getFileSocket);
		getFile(connectionFd, sdfsfilename, localfilename, buf, 1024);
		cout<<"success get"<<endl;
	}
	return true;
}

//////




void putFileHelper_M(string localfilename, string sdfsfilename, string desc)
{
	int connectionFd;
	connect_to_server(desc.c_str(), port+2, &connectionFd);//members
	putFile(connectionFd, localfilename, sdfsfilename, desc, port+2);//members
	//cout<<"success put"<<endl;
}
//problem

bool putFileRequest_M(string localfilename, string sdfsfilename, vector<Node> group)
{

	for(int i=0; i < group.size(); i++)//members
	{
		putFileHelper_M(localfilename, sdfsfilename, group[i].ip_str.c_str());
	}
	vector<string> data;
	data = read_from_log_M("file_location_log.txt");
	write_to_log_M("file_location_log.txt", data, group);
	
	for(int i=0; i < group.size(); i++)//members
	{
		putFileHelper_M("file_location_log.txt", "file_location_log.txt", group[i].ip_str.c_str());
	}
	
	return true;
}


bool closest(vector<Node> members, string machine_fail_ip, string my_ip){
	vector<string> machine_names;
	for(int i=0;i<members.size();i++){
		machine_names.push_back(members[i].ip_str);
	}
	std::sort(machine_names.begin(),machine_names.end());
	for(int i=0;i<machine_names.size();i++){
		if((machine_names[i].compare(machine_fail_ip) == 0) && (i != machine_names.size() -1) ){
			return (machine_names[i+1].compare(my_ip) == 0); 
		}
		else if((machine_names[i].compare(machine_fail_ip) == 0) && (i == machine_names.size() -1)){
			return (machine_names[0].compare(my_ip) == 0); 
		}
	}
	return false;//true if it is closest using the member list.
}

//members need to contain fail machine for now.
int replica(string machine_fail_ip, string my_ip, vector<Node> members, string log_file, vector<Node> group) {
	bool is_right_machine = closest(members, machine_fail_ip, my_ip);
	if(!is_right_machine){
		return 0;
	}
	
	//check the document to extract all the document into a vector.
	vector<string> file_to_replicate;
	vector<string> doc;//every line
	vector<string> new_file;
	string temp;
	ifstream f(log_file);
	if(f.is_open()){
		while(!f.eof()){
			getline(f,temp);
			//boost::split(doc, temp, boost::is_any_of(" "));
			std::istringstream buf(temp);
			std::istream_iterator<std::string> beg(buf), end;
			std::vector<std::string> tokens(beg, end); // done!
			for(auto& s: tokens){
				doc.push_back(s);
				//std::cout <<  s << '\n';
			}
			if(doc[0].compare(machine_fail_ip) == 0){
				file_to_replicate.push_back(doc[1]);//assume no duplicate
			}
			else{//if not fail machine
				new_file.push_back(temp);
			}
		}
		f.close();
	}
	
	//update log first
	//FILE* s;
	
	ofstream s (log_file);
	//s = fopen(log_file,"w+");
	if(s.is_open()){
		for(int i = 0; i < new_file.size(); i++){
			//fputs(new_file[i] + '\n',f);
			s << new_file[i] + '\n';
		}
	}

	//get these file from other machines, put them in random.
	for(int i=0; i< file_to_replicate.size();i++ ){
		getFileRequest_M(file_to_replicate[i], file_to_replicate[i], members);
		//put file and write to log file;
		putFileRequest_M(file_to_replicate[i], file_to_replicate[i], group);//group?
		
		
		//ask file from each of the server.
		
		
		
		
		
	}
	return 1;
}