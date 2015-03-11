#ifndef FUNCTIONS_H
#define FUNCTIONS_H 1

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <utility>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <set>

#include <RInside.h>

#include "Position.h"

using std::string;
using std::map;
using std::set;
using std::multiset;
using std::pair;
using std::cout;
using std::endl;
using std::cerr;
using std::fstream;
using std::stoi;

bool fillMap(map<string, Position>&, const string&);
void writeMap(const map<string, Position>&, const string& fileName = "");
int convertWebEncoding(string&);
void downloadFiles(const string&, RInside&);
void findNewFiles(const string&, map<string, pair<string, string>>&);
void downloadNewFiles(const map<string, pair<string, string>>&, RInside&);
bool downloadWebsite(const string&, string&, RInside& R);
bool findFileTypes();
int getFileTypes(const string&);
void writeFileStatistics(const string&, const string& saveFile = "");

#endif
