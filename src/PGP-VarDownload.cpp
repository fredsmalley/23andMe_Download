#include "PGP-VarDownload.h"

uint32_t Position::num = 0;

int main(int argc, char** argv) {
	RInside R(argc, argv);

	// Make rsID map
	map<string, Position> rsMap;
	string mapSaveFile = "../dbSNPMod/files/rsMap_sav.tsv";
	if(!fillMap(rsMap, mapSaveFile)) {
		cerr << "Unable to read in map from " << mapSaveFile << endl;
		return -1;
	}

	//writeMap(rsMap);

	string websiteFileName = "files/PGP_Public_Genetic_Data.html";
	string websiteName = "https://my.pgp-hms.org/public_genetic_data";

	downloadWebsite(websiteName, websiteFileName, R);

	downloadFiles(websiteFileName, R);

	findFileTypes();

	writeFileStatistics("files/fileTypes.txt", "files/fileStats.txt");

	return 0;
}
