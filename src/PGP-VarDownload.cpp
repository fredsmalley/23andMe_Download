#include "PGP-VarDownload.h"

uint32_t Position::num = 0;

int main(int argc, char** argv) {
	RInside R(argc, argv);

	// Make rsID map
	map<string, pair<Position, string>> rsMap;
	string mapSaveFile = "../dbSNPMod/files/rsMap_sav.tsv";
	if(!fillMap(rsMap, mapSaveFile)) {
		cerr << "Unable to read in map from " << mapSaveFile << endl;
		return -1;
	}

	//writeMap(rsMap);

	string websiteFileName = "files/PGP_Public_Genetic_Data.html";
	string websiteName = "https://my.pgp-hms.org/public_genetic_data";

//	downloadWebsite(websiteName, websiteFileName, R);

//	downloadFiles(websiteFileName, R);

	string fileTypes = "files/fileTypes.txt";
	string fileStats = "files/fileStats.txt";

	findFileTypes(fileTypes);
	writeFileStatistics(fileTypes, fileStats);

//	addExtensions("files/fileTypes.txt");
//	downloadRedirects(fileTypes, R);
	unzipGZipFiles();
	unzipZipFiles(fileTypes);

	fileTypes = "files/fileTypes2.txt";
	fileStats = "files/fileStats2.txt";

	findFileTypes(fileTypes);
	writeFileStatistics(fileTypes, fileStats);

	string b37Files = "files/GRCh37_list.txt";

	findBuild37Files(fileTypes, b37Files);

	return 0;
}
