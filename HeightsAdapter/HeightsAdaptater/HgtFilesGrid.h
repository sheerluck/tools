#ifndef _DEMFILESGRID_H_
#define _DEMFILESGRID_H_

#include "HgtFormat.h"

class HgtFilesGrid
{
public:
	HgtFilesGrid(int maxLoadedFiles, std::string_view filesPath);
	~HgtFilesGrid();

	//void Init( int maxLoadedFiles, std::string_view filesPath);
	signed short GetHeight( int iSquare, int jSquare, int i, int j );
	bool IsExists(int i, int j);
	std::string GetFileName( int i, int j );


private:
	int MAX_LOADED_FILES;
	int loadedFiles;

	static const int NCols = 1201;
	static const int NRows = 1201;

	struct FileFlag;

	struct Heights {
		signed short height[NRows][NCols];
		FileFlag *pFileFlag = nullptr;
	} *dataStack;

	struct FileFlag {
		std::string fileName = "";
		bool isLoaded = false;
		Heights *pHeightData = nullptr;
	} hgtFilesGrid[181][361];
};

#endif
