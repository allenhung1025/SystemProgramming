#include <sys/types.h>
#include <dirent.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <string>
#include <endian.h>
#include "md5.h"
using namespace std;
int main(int argc, char const *argv[])
{
	if(strcmp(argv[0], "./loser") != 0){
		printf("wrong command\n");
		return 0;
	}
	else{
			if(strcmp(argv[1], "status") == 0){ // compare files under directory with the latest commit
				// open .loser_record file
				ifstream loserstream;
				char *path = new char[1024];
				sprintf(path, "%s/.loser_record", argv[2]);
				loserstream.open(path, ios::in | ios::binary);
				delete[] path;
				if(!loserstream.is_open()){
					cout << "[new_file]" << endl;
					struct dirent **namelist;
					int nx = scandir(argv[2], &namelist, NULL, alphasort);
					for(int i = 0; i < nx; i++){
						if(strcmp(namelist[i] -> d_name, ".") == 0 || strcmp(namelist[i] -> d_name, "..") == 0)
							continue;
						cout << namelist[i] -> d_name << endl;
						free(namelist[i]);
					}
					free(namelist);
					cout << "[modified]" << endl;
					cout << "[copied]" << endl;
					cout << "[deleted]" << endl;
					return 0;
				}
				//open .loser_record success
				uint32_t commitnum, numoffile, numofadd, numofmod, numofcpy, numofdel, commitsize;
				uint8_t filenamesize;
				loserstream.seekg(0, ios::end);
				int length = loserstream.tellg();
				loserstream.seekg(0, ios::beg);
				while(1){
					int now = loserstream.tellg();
					loserstream.seekg(24, ios::cur);
					loserstream.read(reinterpret_cast<char*>(&commitsize), sizeof(commitsize));
					loserstream.seekg(unsigned(commitsize) - 28, ios::cur);
					int lengthnow = loserstream.tellg();
					if(lengthnow == length){
						loserstream.seekg(now , ios::beg);
						break;
					}
				}
				loserstream.read((char*)&commitnum, sizeof(commitnum));
				loserstream.read((char*)&numoffile, sizeof(numoffile));
				loserstream.read((char*)&numofadd, sizeof(numofadd));
				loserstream.read((char*)&numofmod, sizeof(numofmod));
				loserstream.read((char*)&numofcpy, sizeof(numofcpy));
				loserstream.read((char*)&numofdel, sizeof(numofdel));
				loserstream.read((char*)&commitsize, sizeof(commitsize));
				for(unsigned l = 0; l < unsigned(numofadd); l++){
					loserstream.read((char*)&filenamesize, sizeof(filenamesize));
					loserstream.seekg(unsigned(filenamesize), ios::cur);
				}
				for(unsigned l = 0; l < unsigned(numofmod); l++){
					loserstream.read((char*)&filenamesize, sizeof(filenamesize));
					loserstream.seekg(unsigned(filenamesize), ios::cur);						
				}
				for(unsigned l = 0; l < unsigned(numofcpy); l++){
					loserstream.read((char*)&filenamesize, sizeof(filenamesize));
					loserstream.seekg(unsigned(filenamesize), ios::cur);
					loserstream.read((char*)&filenamesize, sizeof(filenamesize));
					loserstream.seekg(unsigned(filenamesize), ios::cur);
				}
				for(unsigned l = 0; l < unsigned(numofdel); l++){
					loserstream.read((char*)&filenamesize, sizeof(filenamesize));
					loserstream.seekg(unsigned(filenamesize), ios::cur);
				}
				// store md5 and filename into md5_array and file_array
				char file_array[numoffile][256];
				char md5_array[numoffile][33];
				uint8_t md5;
				for(unsigned l = 0; l < unsigned(numoffile); l++){
					loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
					loserstream.read(file_array[l], filenamesize);
					file_array[l][filenamesize] = '\0';
					for(int k = 0; k < 16; k++){
						loserstream.read(reinterpret_cast<char*>(&md5), sizeof(md5));
						sprintf(md5_array[l] + k * 2, "%02x", md5);
					}
					md5_array[l][32] = '\0';
				}
				loserstream.close(); // close .loser_record file
				// iterate files under the directory and store file name into directory_file array
				struct dirent **namelist;
				int n = scandir(argv[2], &namelist, NULL, alphasort);
				char directory_file[n][256];
				char filename_out[n][256];
				int status[n]; // 0 newfile, 1 modified, 2 copy
				memset(status, -1, sizeof(status));
				int copy_idx[n];
				int idx_dir = 0;
				int idx_out = 0;
				for(int i = 0; i < n; i++){
					if(strcmp(namelist[i] -> d_name, ".") == 0 || strcmp(namelist[i] -> d_name, "..") == 0
						|| strcmp(namelist[i] -> d_name, ".loser_record") == 0){
						free(namelist[i]);
						continue;
					}
					strcpy(directory_file[idx_dir], namelist[i] -> d_name);
					idx_dir++;
					char *path = new char[1024];
					sprintf(path, "%s/%s", argv[2], namelist[i] -> d_name);
					ifstream nowfile;
					nowfile.open(path, ios::binary | ios::in);
					delete[] path;
					nowfile.seekg(0, ios::end);
					uint32_t length = nowfile.tellg();
					//cout << length << endl;
					nowfile.seekg(0, ios::beg);
					char *content = new char[length + 1];
					nowfile.read(content, length);
					content[length] = '\0';
					nowfile.close();
					MD5 hello(content, length);
					string temp = hello.hexdigest();
					delete[] content;
					// compare the file with the latest commit
					int see = 0;
					for(uint32_t j = 0; j < numoffile; j++){
						if(strcmp(namelist[i] -> d_name, file_array[j]) == 0){
							see = 1;
							string md5_o(md5_array[j]);
							if(temp != md5_o){ //modified
								status[idx_out] = 1;
								strcpy(filename_out[idx_out], namelist[i] -> d_name);
								idx_out++;
							}
						}
					}
					if(!see){
						int copy = 0;
						for(uint32_t j = 0; j < numoffile; j++){ // copy
							string md5_o(md5_array[j]);
							if(temp == md5_o){ // copy
								status[idx_out] = 2;
								strcpy(filename_out[idx_out], namelist[i] -> d_name);
								copy_idx[idx_out] = j;
								idx_out++;
								copy = 1;
								break;
							}
						}
						if(!copy){// newfile
							strcpy(filename_out[idx_out], namelist[i] -> d_name);
							status[idx_out] = 0;
							idx_out++;
						}
					}
					free(namelist[i]);
				}
				free(namelist);
				int dir_file_num = idx_dir;
				int out_num = idx_out;
				//cout << out_num << endl;
				for(int k = 0; k <= 2; k++){
					if(k == 0){ // newfile
						cout << "[new_file]" << endl;
						for(int i = 0; i < out_num; i++){
							if(status[i] == 0){
								cout << filename_out[i] << endl;
							}
						}
					}
					else if(k == 1){ // modified
						cout << "[modified]" << endl;
						for(int i = 0; i < out_num; i++){
							if(status[i] == 1){
								cout << filename_out[i] << endl;
							}
						}
					}
					else{ // copy
						cout << "[copied]" << endl;
						for(int i = 0; i < out_num; i++){
							if(status[i] == 2){
								cout << file_array[copy_idx[i]];
								cout << " => " << filename_out[i] << endl;
							}
						}
					}
				}

				// deleted
				cout << "[deleted]" << endl;
				for(uint32_t i = 0; i < numoffile; i++){
					int flag = 0;
					for(int j = 0; j < dir_file_num; j++){
						if(strcmp(file_array[i], directory_file[j]) == 0){
							flag = 1;
							break;
						}
					}
					if(flag == 0){
						cout << file_array[i] << endl;
					}
				}
			}
			else if(strcmp(argv[1], "commit") == 0){ // commit latest files under the directory
				ofstream loserstream_write;
				ifstream loserstream_read;
				char *path = new char [1024];
				sprintf(path, "%s/.loser_record", argv[2]);
				loserstream_write.open(path, ios::app | ios::binary);
				if(loserstream_write.tellp() == 0){ // first commit
					struct dirent **namelist;
					uint32_t nx = scandir(argv[2], &namelist, NULL, alphasort);
					uint8_t file_size_array[nx];
					char filename_array[nx][256];
					char filemd5_array[nx][33];
					int idx = 0;
					for(int i = 0; i < nx; i++){
						if(strcmp(namelist[i] -> d_name, ".") == 0 || strcmp(namelist[i] -> d_name, "..") == 0 ||
							strcmp(namelist[i] -> d_name, ".loser_record") == 0){
							free(namelist[i]);
							continue;
						}
						strcpy(filename_array[idx], namelist[i] -> d_name);
						file_size_array[idx] = strlen(namelist[i]-> d_name);
						char *pathfile = new char[1024];
						sprintf(pathfile, "%s/%s", argv[2], namelist[i] -> d_name);
						ifstream filestream;
						filestream.open(pathfile, ios::binary | ios::in);
						delete[] pathfile;
						filestream.seekg(0, ios::end);
						uint32_t lenoffile = filestream.tellg();
						filestream.seekg(0, ios::beg);
						char *content = new char[lenoffile + 1];
						filestream.read(content, lenoffile);
						filestream.close();
						content[lenoffile] = '\0';
						//cout << content;
						MD5 hello(content, lenoffile);
						delete[] content;
						string temp;
						temp = hello.hexdigest();
						strcpy(filemd5_array[idx], temp.c_str());
						filemd5_array[idx][32] = '\0';
						idx++;
						free(namelist[i]);
					}
					uint32_t commit_num = 0x00000001;
					uint32_t numoffile = idx;
					uint32_t numofadd = idx;
					uint32_t numofmod = 0;
					uint32_t numofcpy = 0;
					uint32_t numofdel = 0;
					uint32_t commitsize = 28;
					for(uint32_t i = 0; i < numofadd; i++){
						commitsize += 1;
						commitsize += file_size_array[i];
					}
					for(uint32_t i = 0; i < numoffile; i++){
						commitsize += 1;
						commitsize += file_size_array[i];
						commitsize += 16;
					}
					uint8_t out[4];
					out[0] = (commit_num) & 0xFF;
					out[1] = (commit_num >> 8u) & 0xFF;
					out[2] = (commit_num >> 16u) & 0xFF;
					out[3] = (commit_num >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numoffile) & 0xFF;
					out[1] = (numoffile >> 8u) & 0xFF;
					out[2] = (numoffile >> 16u) & 0xFF;
					out[3] = (numoffile >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofadd) & 0xFF;
					out[1] = (numofadd >> 8u) & 0xFF;
					out[2] = (numofadd >> 16u) & 0xFF;
					out[3] = (numofadd >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofmod) & 0xFF;
					out[1] = (numofmod >> 8u) & 0xFF;
					out[2] = (numofmod >> 16u) & 0xFF;
					out[3] = (numofmod >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofcpy) & 0xFF;
					out[1] = (numofcpy >> 8u) & 0xFF;
					out[2] = (numofcpy >> 16u) & 0xFF;
					out[3] = (numofcpy >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofdel) & 0xFF;
					out[1] = (numofdel >> 8u) & 0xFF;
					out[2] = (numofdel >> 16u) & 0xFF;
					out[3] = (numofdel >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (commitsize) & 0xFF;
					out[1] = (commitsize >> 8u) & 0xFF;
					out[2] = (commitsize >> 16u) & 0xFF;
					out[3] = (commitsize >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					for(uint32_t i = 0;i < numofadd; i++){
						loserstream_write.write(reinterpret_cast<char*>(&file_size_array[i]), sizeof(file_size_array[i]));
						loserstream_write.write(reinterpret_cast<char*>(filename_array[i]), file_size_array[i]);
					}
					for(uint32_t i = 0; i < numoffile; i++){
						loserstream_write.write(reinterpret_cast<char*>(&file_size_array[i]), sizeof(file_size_array[i]));
						loserstream_write.write(reinterpret_cast<char*>(filename_array[i]), file_size_array[i]);
						char temp_md5[17];
						for(int j = 0; j < 16; j++){
							uint8_t first, second;
							if(isalpha(filemd5_array[i][j * 2])){
								if(filemd5_array[i][j * 2] == 'a') first = 10;
								else if(filemd5_array[i][j * 2] == 'b') first = 11;
								else if(filemd5_array[i][j * 2] == 'c') first = 12;
								else if(filemd5_array[i][j * 2] == 'd') first = 13;
								else if (filemd5_array[i][j * 2] == 'e') first = 14;
								else if(filemd5_array[i][j * 2] == 'f') first = 15;
							}
							else{
								first = filemd5_array[i][j * 2] - '0';
							}
							if(isalpha(filemd5_array[i][j * 2 + 1])){
								if(filemd5_array[i][j * 2 + 1] == 'a') second = 10;
								else if(filemd5_array[i][j * 2 + 1] == 'b') second = 11;
								else if(filemd5_array[i][j * 2 + 1] == 'c') second = 12;
								else if(filemd5_array[i][j * 2 + 1] == 'd') second = 13;
								else if (filemd5_array[i][j * 2 + 1] == 'e') second = 14;
								else if(filemd5_array[i][j * 2 + 1] == 'f') second = 15;
							}
							else{
								second = filemd5_array[i][j * 2 + 1] - '0';
							}
							temp_md5[j] = first * 16 + second;
						}
						temp_md5[16] = '\0';
						loserstream_write.write(temp_md5, 16);
					}
					free(namelist);
				}
				else{ // commit number larger than one
					loserstream_read.open(path, ios::in | ios::binary);
					uint32_t commitnum, numoffile, numofadd, numofmod, numofcpy, numofdel, commitsize;
					uint8_t filenamesize;
					// seek to the right place
					loserstream_read.seekg(0, ios::end);
					int length = loserstream_read.tellg();
					loserstream_read.seekg(0, ios::beg);
					while(1){
						int now = loserstream_read.tellg();
						loserstream_read.seekg(24, ios::cur);
						loserstream_read.read(reinterpret_cast<char*>(&commitsize), sizeof(commitsize));
						loserstream_read.seekg(unsigned(commitsize) - 28, ios::cur);
						int lengthnow = loserstream_read.tellg();
						if(lengthnow == length){
							loserstream_read.seekg(now , ios::beg);
							break;
						}
					}
					loserstream_read.read((char*)&commitnum, sizeof(commitnum));
					loserstream_read.read((char*)&numoffile, sizeof(numoffile));
					loserstream_read.read((char*)&numofadd, sizeof(numofadd));
					loserstream_read.read((char*)&numofmod, sizeof(numofmod));
					loserstream_read.read((char*)&numofcpy, sizeof(numofcpy));
					loserstream_read.read((char*)&numofdel, sizeof(numofdel));
					loserstream_read.read((char*)&commitsize, sizeof(commitsize));
					for(unsigned l = 0; l < unsigned(numofadd); l++){
						loserstream_read.read((char*)&filenamesize, sizeof(filenamesize));
						loserstream_read.seekg(unsigned(filenamesize), ios::cur);
					}
					for(unsigned l = 0; l < unsigned(numofmod); l++){
						loserstream_read.read((char*)&filenamesize, sizeof(filenamesize));
						loserstream_read.seekg(unsigned(filenamesize), ios::cur);						
					}
					for(unsigned l = 0; l < unsigned(numofcpy); l++){
						loserstream_read.read((char*)&filenamesize, sizeof(filenamesize));
						loserstream_read.seekg(unsigned(filenamesize), ios::cur);
						loserstream_read.read((char*)&filenamesize, sizeof(filenamesize));
						loserstream_read.seekg(unsigned(filenamesize), ios::cur);
					}
					for(unsigned l = 0; l < unsigned(numofdel); l++){
						loserstream_read.read((char*)&filenamesize, sizeof(filenamesize));
						loserstream_read.seekg(unsigned(filenamesize), ios::cur);
					}
					// store the latest commit into the file_array and md5_array
					char file_array[numoffile][256];
					char md5_array[numoffile][33];
					uint8_t md5;
					for(unsigned l = 0; l < unsigned(numoffile); l++){
						loserstream_read.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						loserstream_read.read(file_array[l], filenamesize);
						file_array[l][filenamesize] = '\0';
						for(int k = 0; k < 16; k++){
							loserstream_read.read(reinterpret_cast<char*>(&md5), sizeof(md5));
							sprintf(md5_array[l] + k * 2, "%02x", md5);
						}
						md5_array[l][32] = '\0';
					}
					loserstream_read.close();
					// new_file, modified, or copied in the directory file
					struct dirent **namelist;
					int n = scandir(argv[2], &namelist, NULL, alphasort);
					char directory_file[n][256];
					char filename_out[n][256];
					int status[n]; // 0 newfile, 1 modified, 2 copy
					char md5_dirfile[n][33];
					memset(status, -1, sizeof(status));
					int copy_idx[n];
					int idx_dir = 0;
					int idx_out = 0;
					for(int i = 0; i < n; i++){
						if(strcmp(namelist[i] -> d_name, ".") == 0 || strcmp(namelist[i] -> d_name, "..") == 0
							|| strcmp(namelist[i] -> d_name, ".loser_record") == 0){
							free(namelist[i]);
							continue;
						}
						strcpy(directory_file[idx_dir], namelist[i] -> d_name);
						char *pathfile = new char[1024];
						sprintf(pathfile, "%s/%s", argv[2], namelist[i] -> d_name);
						ifstream nowfile;
						nowfile.open(pathfile, ios::binary | ios::in);
						delete[] pathfile;
						nowfile.seekg(0, ios::end);
						uint32_t length = nowfile.tellg();
						nowfile.seekg(0, ios::beg);
						char *content = new char[length + 1];
						nowfile.read(content, length);
						content[length] = '\0';
						nowfile.close();
						MD5 hello(content, length);
						string temp = hello.hexdigest();
						strcpy(md5_dirfile[idx_dir], temp.c_str());
						md5_dirfile[idx_dir][32] = '\0';
						idx_dir++;
						delete[] content;
						// compare the file with the latest commit
						int see = 0;
						for(uint32_t j = 0; j < numoffile; j++){
							if(strcmp(namelist[i] -> d_name, file_array[j]) == 0){
								see = 1;
								string md5_o(md5_array[j]);
								if(temp != md5_o){ //modified
									//cout << md5_o << endl;
									//cout << temp << endl;
									status[idx_out] = 1;
									strcpy(filename_out[idx_out], namelist[i] -> d_name);
									idx_out++;
									//cout << "modified: " << namelist[i] -> d_name << endl;
								}
							}
						}
						if(!see){
							int copy = 0;
							for(uint32_t j = 0; j < numoffile; j++){ // copy
								string md5_o(md5_array[j]);
								if(temp == md5_o){ // copy
									status[idx_out] = 2;
									strcpy(filename_out[idx_out], namelist[i] -> d_name);
									copy_idx[idx_out] = j;
									// cout << "copied: " << file_array[j] << " => " << namelist[i] ->d_name << endl;
									idx_out++;
									copy = 1;
									break;
								}
							}
							if(!copy){// newfile
								strcpy(filename_out[idx_out], namelist[i] -> d_name);
								status[idx_out] = 0;
								//cout << "new_file: " << namelist[i] -> d_name << endl;
								idx_out++;
							}
						}
						free(namelist[i]);
					}
					free(namelist);
					// delete file
					int dir_file_num = idx_dir;
					int out_num = idx_out;
					char delete_file[numoffile][256];
					int del_idx = 0;
					for(uint32_t i = 0; i < numoffile; i++){
						int flag = 0;
						for(int j = 0; j < dir_file_num; j++){
							if(strcmp(file_array[i], directory_file[j]) == 0){
								flag = 1;
								break;
							}
						}
						if(flag == 0){
							strcpy(delete_file[del_idx], file_array[i]);
							//cout << file_array[i] << endl;
							del_idx++;
						}
					}
					int del_num = del_idx;
					// commit 
					commitnum = commitnum + 1;
					numoffile = dir_file_num;
					numofadd = 0, numofmod = 0, numofcpy = 0;
					for(int i = 0; i < dir_file_num; i++){
						if(status[i] == 0) numofadd++;
						else if(status[i] == 1) numofmod++;
						else if(status[i] == 2) numofcpy++;
					}
					numofdel = del_num;
					commitsize = 28;
					for(int i = 0; i < dir_file_num; i++){
						if(status[i] == 0){
							commitsize += 1;
							commitsize += strlen(filename_out[i]);
						}
					}
					for(int i = 0; i < dir_file_num; i++){
						if(status[i] == 1){
							commitsize += 1;
							commitsize += strlen(filename_out[i]);
						}
					}
					for(int i = 0; i < del_num; i++){
						commitsize += 1;
						commitsize += strlen(delete_file[i]);
					}
					for(int i = 0; i < dir_file_num; i++){
						if(status[i] == 2){
							commitsize += 1;
							commitsize += strlen(file_array[copy_idx[i]]);
							commitsize += 1;
							commitsize += strlen(filename_out[i]);
						}
					}
					for(int i = 0; i < numoffile; i++){
						commitsize += 1;
						commitsize += strlen(directory_file[i]);
						commitsize += 16;
					}
					//cout << commitsize << endl;
					uint8_t out[4];
					out[0] = (commitnum) & 0xFF;
					out[1] = (commitnum >> 8u) & 0xFF;
					out[2] = (commitnum >> 16u) & 0xFF;
					out[3] = (commitnum >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numoffile) & 0xFF;
					out[1] = (numoffile >> 8u) & 0xFF;
					out[2] = (numoffile >> 16u) & 0xFF;
					out[3] = (numoffile >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofadd) & 0xFF;
					out[1] = (numofadd >> 8u) & 0xFF;
					out[2] = (numofadd >> 16u) & 0xFF;
					out[3] = (numofadd >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofmod) & 0xFF;
					out[1] = (numofmod >> 8u) & 0xFF;
					out[2] = (numofmod >> 16u) & 0xFF;
					out[3] = (numofmod >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofcpy) & 0xFF;
					out[1] = (numofcpy >> 8u) & 0xFF;
					out[2] = (numofcpy >> 16u) & 0xFF;
					out[3] = (numofcpy >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (numofdel) & 0xFF;
					out[1] = (numofdel >> 8u) & 0xFF;
					out[2] = (numofdel >> 16u) & 0xFF;
					out[3] = (numofdel >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					out[0] = (commitsize) & 0xFF;
					out[1] = (commitsize >> 8u) & 0xFF;
					out[2] = (commitsize >> 16u) & 0xFF;
					out[3] = (commitsize >> 24u) & 0xFF;
					loserstream_write.write(reinterpret_cast<char*>(out), sizeof(out));
					for(int i = 0; i < dir_file_num; i++){
						if(status[i] == 0){ // new_file
							uint8_t templen = strlen(filename_out[i]);
							loserstream_write.write(reinterpret_cast<char*>(&templen), 1);
							loserstream_write.write(reinterpret_cast<char*>(filename_out[i]), strlen(filename_out[i]));
						}
					}
					for(int i = 0; i < dir_file_num; i++){ // modified
						if(status[i] == 1){
							uint8_t templen = strlen(filename_out[i]);
							loserstream_write.write(reinterpret_cast<char*>(&templen), 1);
							loserstream_write.write(reinterpret_cast<char*>(filename_out[i]), strlen(filename_out[i]));
						}
					}
					for(int i = 0; i < dir_file_num; i++){ // copied
						if(status[i] == 2){
							uint8_t templen = strlen(file_array[copy_idx[i]]);
							loserstream_write.write(reinterpret_cast<char*>(&templen), 1);
							loserstream_write.write(reinterpret_cast<char*>(file_array[copy_idx[i]]), templen);
							templen = strlen(filename_out[i]);
							loserstream_write.write(reinterpret_cast<char*>(&templen), 1);
							loserstream_write.write(reinterpret_cast<char*>(filename_out[i]), templen);
						}
					}
					for(int i = 0; i < del_num; i++){
						uint8_t templen = strlen(delete_file[i]);
						loserstream_write.write(reinterpret_cast<char*>(&templen), 1);
						loserstream_write.write(reinterpret_cast<char*>(delete_file[i]), templen);
					}
					for(int i = 0; i < numoffile; i++){
						uint8_t templen = strlen(directory_file[i]);
						loserstream_write.write(reinterpret_cast<char*>(&templen), 1);
						loserstream_write.write(reinterpret_cast<char*>(directory_file[i]), templen);
						char temp_md5[17];
						for(int j = 0; j < 16; j++){
							uint8_t first, second;
							if(isalpha(md5_dirfile[i][j * 2])){
								if(md5_dirfile[i][j * 2] == 'a') first = 10;
								else if(md5_dirfile[i][j * 2] == 'b') first = 11;
								else if(md5_dirfile[i][j * 2] == 'c') first = 12;
								else if(md5_dirfile[i][j * 2] == 'd') first = 13;
								else if (md5_dirfile[i][j * 2] == 'e') first = 14;
								else if(md5_dirfile[i][j * 2] == 'f') first = 15;
							}
							else{
								first = md5_dirfile[i][j * 2] - '0';
							}
							if(isalpha(md5_dirfile[i][j * 2 + 1])){
								if(md5_dirfile[i][j * 2 + 1] == 'a') second = 10;
								else if(md5_dirfile[i][j * 2 + 1] == 'b') second = 11;
								else if(md5_dirfile[i][j * 2 + 1] == 'c') second = 12;
								else if(md5_dirfile[i][j * 2 + 1] == 'd') second = 13;
								else if (md5_dirfile[i][j * 2 + 1] == 'e') second = 14;
								else if(md5_dirfile[i][j * 2 + 1] == 'f') second = 15;
							}
							else{
								second = md5_dirfile[i][j * 2 + 1] - '0';
							}
							temp_md5[j] = first * 16 + second;
						}
						temp_md5[16] = '\0';
						loserstream_write.write(temp_md5, 16);
					}
				}
				loserstream_write.close();
				delete[] path;
			}
			else if(strcmp(argv[1], "log") == 0){ // format: loser log <num> <directory>
				ifstream loserstream;
				char *path = new char [256];
				sprintf(path, "%s/.loser_record", argv[3]);
				loserstream.open(path, ios::in | ios::binary);
				delete[] path;
				if(!loserstream.is_open()){
						return 0;
				}
				uint32_t commitnum, numoffile, numofadd, numofmod, numofcpy, numofdel, commitsize;
				uint8_t filenamesize;
				uint32_t maxcommit;
				vector<uint32_t> commit_v;
				loserstream.seekg(0, ios::end);
				int length = loserstream.tellg();
				loserstream.seekg(0, ios::beg);
				while(1){
					loserstream.read(reinterpret_cast<char*>(&commitnum), sizeof(commitnum));
					loserstream.seekg(20, ios::cur);
					loserstream.read(reinterpret_cast<char*>(&commitsize), sizeof(commitsize));
					loserstream.seekg(commitsize - 28, ios::cur);
					commit_v.push_back(commitsize);
					int lengthnow = loserstream.tellg();
					if(lengthnow == length){
						maxcommit = commitnum;
						break;
					}
				}
				uint32_t lognum = atoi(argv[2]);
				if(lognum > maxcommit)
					lognum = maxcommit;
				loserstream.seekg(0, ios::end);
				for(uint32_t i = maxcommit; i >= maxcommit - lognum + 1; i--){
					int temp = -commit_v[i - 1];
					loserstream.seekg(temp, ios::cur);
					loserstream.seekg(4, ios::cur);
					loserstream.read(reinterpret_cast<char*>(&numoffile), sizeof(numoffile));
					loserstream.read(reinterpret_cast<char*>(&numofadd), sizeof(numofadd));
					loserstream.read(reinterpret_cast<char*>(&numofmod), sizeof(numofmod));
					loserstream.read(reinterpret_cast<char*>(&numofcpy), sizeof(numofcpy));
					loserstream.read(reinterpret_cast<char*>(&numofdel), sizeof(numofdel));
					loserstream.seekg(4, ios::cur);
					cout << "# commit " << i << endl;
					cout << "[new_file]" << endl;
					for(uint32_t j = 0; j < numofadd; j++){
						loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						char *str = new char[filenamesize + 1];
						loserstream.read((char*)str, filenamesize);
						str[filenamesize] = '\0';
						cout << str << endl;
						delete[] str;
					}
					cout << "[modified]" << endl;
					for(uint32_t j = 0 ; j < numofmod; j++){
						loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						char *str = new char[filenamesize + 1];
						loserstream.read((char*)str, filenamesize);
						str[filenamesize] = '\0';
						cout << str << endl;
						delete[] str;
					}
					cout << "[copied]" << endl;
					for(uint32_t j = 0; j < numofcpy; j++){
						loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						char *str = new char[filenamesize + 1];
						loserstream.read((char*)str, filenamesize);
						str[filenamesize] = '\0';
						cout << str;
						delete[] str;
						cout << " => ";
						loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						char *str_next = new char[filenamesize + 1];
						loserstream.read((char*)str_next, filenamesize);
						str_next[filenamesize] = '\0';
						cout << str_next << endl;
						delete[] str_next;
					}
					cout << "[deleted]" << endl;
					for(uint32_t j = 0; j < numofdel; j++){
						loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						char *str = new char[filenamesize + 1];
						loserstream.read((char*)str, filenamesize);
						str[filenamesize] = '\0';
						cout << str << endl;
						delete[] str;
					}
					cout << "(MD5)" << endl;
					for(uint32_t j = 0; j < numoffile; j++){
						loserstream.read(reinterpret_cast<char*>(&filenamesize), sizeof(filenamesize));
						char *str = new char[filenamesize + 1];
						loserstream.read((char*)str, filenamesize);
						str[filenamesize] = '\0';
						cout << str << " ";
						delete[] str;
						uint8_t md5;
						char *s = new char[33];
						for(int k = 0; k < 16; k++){
							loserstream.read(reinterpret_cast<char*>(&md5), sizeof(md5));
							sprintf(s + k * 2, "%02x", md5);
						}
						s[32] = '\0';
						cout << s << endl;
						delete[] s;
					}
					loserstream.seekg(temp, ios::cur);
					if(i != maxcommit - lognum + 1)
						cout << endl;
				}
				loserstream.close();
			}
	}
	return 0;
}
