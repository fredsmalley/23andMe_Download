#include "functions.h"

bool fillMap(map<string, pair<Position, string>>& rsMap, const string& fileName) {
	bool filled = false;

	fstream file;
	string line;
	string rsID;
	string ref;
	unsigned int begin;
	unsigned int end;

	struct stat buffer;
	if (stat(fileName.c_str(), &buffer) == 0) {
		file.open(fileName, fstream::in);
		if (file.is_open()) {
			while (getline(file, line)) {
				rsID = line.substr(0, line.find(":"));		// save rsID
				line = line.substr(line.find("(") + 1);
				begin = stoi(line.substr(0, line.find(",")));	// save begin
				line = line.substr(line.find(",") + 1);
				end = stoi(line.substr(0, line.find(")")));	// save end
				ref = line.substr(line.find(")") + 1);		// save reference
				rsMap.insert(pair<string, pair<Position, string>>(rsID, pair<Position, string>(Position(begin, end, false), ref)));
			}

			file.close();
			filled = true;
		} else
			cerr << "Unable to open " << fileName << endl;
	} else
		cerr << fileName << " not found" << endl;

	return filled;
}

void writeMap(const map<string, pair<Position, string>>& rsMap, const string& fileName) {
	bool toScreen = false;
	fstream outFile;
	if (fileName.length() == 0)
		toScreen = true;
	else {
		outFile.open(fileName, fstream::out);
		if (!outFile.is_open()) {
			cerr << "Unable to open " << fileName << endl;
			return;
		}
	}

	for (map<string, pair<Position, string>>::const_iterator itr = rsMap.begin();
			itr != rsMap.end(); itr++) {
		if (toScreen)
			cout << itr->first << ":\t(" << itr->second.first.getBegin() << "," << itr->second.first.getEnd() << ")" << itr->second.second << endl;
		else
			outFile << itr->first << ":\t(" << itr->second.first.getBegin() << "," << itr->second.first.getEnd() << ")" << itr->second.second << endl;
	}
}

int convertWebEncoding(string& websiteFile) {
	string newFile = websiteFile + ".utf-8";

	int fd;
	pid_t pid = fork();
	switch (pid) {
	case -1:
		cerr << "fork() failed.\n";
		return -1;
	case 0:
		fd = open(newFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		dup2(fd, 1);

		execl("/usr/bin/iconv", "iconv", "-f", "iso-8859-1", "-t", "utf-8", websiteFile.c_str(), NULL);
		cerr << "execl() failed.\n";
		return 1;
	default:
		cout << "Converting web page...";

		int status = 0;
		waitpid(pid, &status, 0);
		kill(pid, SIGSTOP);

		cout << "Done\n";
	}

	websiteFile = newFile;

	return 0;
}

bool downloadWebsite(const string& websiteName, string& websiteFileName, RInside& R) {
	R["website"] = websiteName;
	R["websave"] = websiteFileName;

	struct stat buffer;
	if (stat(websiteFileName.c_str(), &buffer) != 0) {
		cout << "Downloading web page " << websiteName << "...";
		R.parseEvalQ("download.file(website, websave, method=\"curl\", quiet=TRUE)");
		cout << "Done" << endl;

		if (convertWebEncoding(websiteFileName) != 0) {
			cerr << "Problem converting " << websiteFileName << " to utf-8" << endl;
			return false;
		}
	} else {
		cout << websiteFileName << " already exists, not downloading." << endl;
	}
}

void downloadFiles(const string& webFileName, RInside& R) {
	map<string, pair<string, string>> newFiles;

	findNewFiles(webFileName, newFiles);
	if (newFiles.size() != 0) {
		cout << "Downloading new files...";
		downloadNewFiles(newFiles, R);
		cout << "Done" << endl;
		newFiles.clear();
	} else
		cout << "No new files to download" << endl;
}

void findNewFiles(const string& webFileName, map<string, pair<string, string>>& newFiles) {
	set<string> CGparts;
	string line;
	fstream CGpartFile;
	CGpartFile.open("../GeneVar/files/CGparticipants.tsv");
	if (CGpartFile.is_open()) {
		getline(CGpartFile, line);	// skip title
		while (getline(CGpartFile, line))
			CGparts.insert(line);

		CGpartFile.close();
	} else {
		cout << "Could not find Complete Genomics participants" << endl;
		return;
	}

	fstream websiteFile;
	websiteFile.open(webFileName, fstream::in);

	string saveFileName;
	string partID;

	struct stat buffer;
	if (websiteFile.is_open()) {								// check if file is open
		while(getline(websiteFile, line)) {					// loop on each line in file
			if (line.find("/profile/") != line.npos) {			// if the line contains participant ID
				partID = line.substr(line.find("profile") + 8);
				partID = partID.substr(0, partID.find("\""));				// save participant ID
				
				if (CGparts.count(partID) != 0)					// skip if partID already available elsewhere
					continue;

				saveFileName = "files/var/" + partID + "_23andMe";			// save name to save file to
				while(getline(websiteFile, line)) {					// continue looking through lines
					if (line.find("auto\" data-summarize-as=\"list") != line.npos &&
							line.find("23andMe") != line.npos) {				// look for 23andMe line
						while (getline(websiteFile, line)) {					// continue looking through lines
							if (line.find(">Download<") != line.npos) {
								line = line.substr(line.find("/"));
								line = line.substr(0, line.find("rel") - 2);
								line = "https://my.pgp-hms.org" + line;			// save download file name
								if (stat(saveFileName.c_str(), &buffer) != 0) {		// check if file already exists
									// insert new file into map
									newFiles.insert(pair<string, pair<string, string>>(partID, pair<string, string>(saveFileName, line)));
								}
								break;			// break if file already exists or downloading
							}
						}
						break;					// break if not 23andMe line or done with current
					}
				}
			}
		}
		websiteFile.close();
	} else
		cout << "Unable to open " << webFileName << endl;
}

void downloadNewFiles(const map<string, pair<string, string>>& newFiles, RInside& R) {
	for (map<string, pair<string, string>>::const_iterator itr = newFiles.begin();
			itr != newFiles.end(); itr++) {
		R["saveFileName"] = itr->second.first;
		R["downloadFileName"] = itr->second.second;

		R.parseEvalQ("download.file(downloadFileName, saveFileName, method=\"curl\")");
	}
}

bool findFileTypes(const string& fileTypes) {
	//string fileTypes = "files/fileTypes.txt";

	if (getFileTypes(fileTypes) != 0)
		return false;

	return true;
}

int getFileTypes(const string& fileName) {
	int fd;
	int status;
	string list = "files/varList.txt";
	string modlist = "files/modvarList.txt";

	pid_t pid = fork();

	switch(pid) {
	case -1:
		cerr << "fork() failed" << endl;
		return -1;
	case 0:
		fd = open(list.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		dup2(fd, 1);

		execl("/usr/bin/ls", "ls", "files/var/", NULL);
		cerr << "execl() failed" << endl;
		return 1;
	default:
		cout << "Getting list of files at files/var/...";

		status = 0;
		waitpid(pid, &status, 0);
		kill(pid, SIGSTOP);

		cout << "Done" << endl;
	}

	string line;
	fstream inFile;
	fstream outFile;
	inFile.open(list, fstream::in);
	if (inFile.is_open()) {
		outFile.open(modlist, fstream::out);
		if (outFile.is_open()) {
			while (getline(inFile, line))
				outFile << "files/var/" << line << endl;
			outFile.close();
		} else {
			cerr << "Unable to open " << modlist << endl;
			return -1;
		}
		inFile.close();
	} else {
		cerr << "Unable to open " << list << endl;
		return -1;
	}

	pid = fork();

	switch(pid) {
	case -1:
		cerr << "fork() failed" << endl;
		return -1;
	case 0:
		fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		dup2(fd, 1);

		execl("/usr/bin/file", "file", "-f", modlist.c_str(), NULL);
		cerr << "execl() failed" << endl;
		return 1;
	default:
		cout << "Getting list of file types at files/var/ ...";

		status = 0;
		waitpid(pid, &status, 0);
		kill(pid, SIGSTOP);

		cout << "Done" << endl;
		return 0;
	}
}

void writeFileStatistics(const string& fileName, const string& output) {
	cout << "Writting file statistics";

	bool toScreen = false;
	fstream outFile;
	if (output.length() == 0) {
		toScreen = true;
		cout << " to screen";
	} else {
		outFile.open(output, fstream::out);
		if (!outFile.is_open()) {
			cerr << "Unable to open " << output << endl;
			return;
		}	
	}
	cout << "...";

	multiset<string> lines;
	set<string> uniqueLines;

	fstream file;
	file.open(fileName, fstream::in);
	string line;
	while(getline(file, line)) {
		line = line.substr(line.find(":") + 1);
		while (isspace(line[0]))
			line = line.substr(1);
		line = line.substr(0, line.find(","));

		lines.insert(line);
		uniqueLines.insert(line);
	}

	for (set<string>::const_iterator itr = uniqueLines.begin();
			itr != uniqueLines.end(); itr++) {
		if (toScreen) {
			cout << lines.count(*itr) << ": " << *itr << endl;
		} else
			outFile << lines.count(*itr) << ": " << *itr << endl;
	}

	lines.clear();
	uniqueLines.clear();

	if (!toScreen)
		outFile.close();

	cout << "Done" << endl;
}

void downloadRedirects(const string& fileTypes, RInside& R) {
	fstream fileTypeList;
	fileTypeList.open(fileTypes);

	string line;
	string downloadPath;
	string savePath;
	fstream file;
	if (fileTypeList.is_open()) {
		while(getline(fileTypeList, line)) {
			if (line.find("HTML") != line.npos) {
				savePath = line.substr(0, line.find(":"));
				file.open(savePath);
				if (file.is_open()) {
					getline(file, line);
					if (line.find("a href") == line.npos)
						cout << savePath << " is not a redirection!" << endl;
					else {
						line = line.substr(line.find("\"") + 1);
						downloadPath = line.substr(0, line.find("\""));

						// find extension
						line = downloadPath;
						while (line.find("/") != line.npos)
							line = line.substr(line.find("/") + 1);
						line = line.substr(line.find("."));
						savePath += line;

						if (!exists(savePath)) {
							R["saveFileName"] = savePath;
							R["downloadFileName"] = downloadPath;
							cout << savePath << endl;
							R.parseEvalQ("download.file(downloadFileName, saveFileName, method=\"curl\", extra=\"-g\")");
						}
					}
					file.close();
				} else {
					cout << "Unable to open" << savePath << endl;
				}
			}
		}
		fileTypeList.close();
	} else
		cerr << "Unable to open " << fileTypes << endl;
}

void unzipGZipFiles() {
	return;
}

void unzipZipFiles(const string& listFileName) {
	string zipFileName;
	string fileName;
	string tempFileName = "temp.txt";
	string command;
	int fd;
	pid_t pid;

	string line;
	fstream listFile;
	fstream tempFile;
	listFile.open (listFileName, fstream::in);
	if (listFile.is_open()) {
		while (getline (listFile, line)) {
			if (line.find ("Zip archive") != string::npos &&
					line.find ("empty") == string::npos) {
				zipFileName = line.substr (0, line.find(":"));
				fileName = zipFileName.substr (0, zipFileName.find (".")) + ".txt";

				cout << zipFileName << "\t" << fileName << endl;

				if (exists(fileName))
					continue;

				pid = fork();
				switch (pid) {
				case -1:
					cout << "fork() failed!" << endl;
					return;
				case 0:
					fd = open (tempFileName.c_str (),
						O_WRONLY | O_CREAT | O_TRUNC,
						S_IRUSR | S_IWUSR);
					dup2 (fd, 1);

					execl ("/usr/bin/unzip", "unzip", zipFileName.c_str (), NULL);
					cout << "execl() failed!" << endl;
					return;
				default:
					cout << "Unzipping " << zipFileName << endl;

					int status = 0;
					waitpid (pid, &status, 0);
					kill (pid, SIGSTOP);

					cout << "Done" << endl;
				}

				tempFile.open (tempFileName.c_str (), fstream::in);
				if (tempFile.is_open ()) {
					while (getline (tempFile, line)) {
						if (line.find ("inflating") != string::npos) {
							line = line.substr (line.find(":") + 1);
							while (isspace (line[0]))
								line = line.substr (1);

							command = "mv " + line + " " + fileName;
							cout << command << endl;
							system (command.c_str ());

							break;
						}
					}

					tempFile.close();
				} else
					cout << "Unable to open " << tempFileName << endl;

				command = "rm " + tempFileName;
				system (command.c_str ());
			}
		}

		listFile.close();
	} else
		cout << "Unable to open " << fileName << endl;
}

void addExtensions(const string& fileName) {
	int status;
	pid_t pid;
	string ext;

	string line;
	fstream file;
	file.open(fileName);
	if (file.is_open()) {
		while (getline(file, line)) {
			if (line.find("ASCII") != line.npos)
				ext = ".txt";
			else if (line.find("Zip") != line.npos)
				ext = ".zip";
			else if (line.find("gzip") != line.npos)
				ext = ".gzip";
			else
				continue;

			line = line.substr(0, line.find(":"));
			ext = line + ext;

			if (line.find(".") != line.npos)
				continue;

			pid = fork();

			switch (pid) {
			case -1:
				cerr << "fork() failed.\n";
				break;
			case 0:
				execl("/bin/cp", "cp", line.c_str(), ext.c_str(), NULL);
				cerr << "execl() failed.\n";
				break;
			default:
				status = 0;
				waitpid(pid, &status, 0);
				kill(pid, SIGSTOP);
			}
		}
		file.close();
	} else
		cout << "Unable to open " << fileName << endl;
}

bool exists(const string& file) {
	struct stat buffer;

	if (stat(file.c_str(), &buffer) == 0)
		return true;
	else
		return false;
}

void findBuild37Files(const string& fileList, const string& output) {
	string line;
	string fileName;
	fstream list;
	fstream out;
	fstream file;
	list.open(fileList, fstream::in);
	out.open(output, fstream::out);
	if (list.is_open() && out.is_open()) {
		while(getline(list, line)) {
			if (line.find("ASCII") != line.npos) {
				fileName = line.substr(0, line.find(":"));
				file.open(fileName);
				if (file.is_open()) {
					while(getline(file,line)) {
						if (line[0] == '#') {
							if (line.find("build 37") != line.npos) {
								out << fileName << endl;
								break;
							}
						} else
							break;
					}
					file.close();
				} else
					continue;
			}
		}
		out.close();
		list.close();
	} else {
		cerr << "Unable to open either " << fileList
			<< " or " << output << endl;
	}
}
