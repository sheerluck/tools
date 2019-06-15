#include "HgtFilesGrid.h"
#include "tinyformat.h"
#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;


HgtFilesGrid::HgtFilesGrid(int maxLoadedFiles, std::string_view filesPath)
{
	this->loadedFiles = 0;
	this->MAX_LOADED_FILES = maxLoadedFiles;
	this->dataStack = new Heights[maxLoadedFiles];

	for (int i = 0; i <= 180; i++) {
		for (int j = 0; j <= 360; j++) {
			auto fileName = tfm::format("%s/%s.hgt", filesPath, HgtFormat::crdtodem(90 - i, -180 + j));
			auto path = fs::u8path(std::cbegin(fileName), std::cend(fileName));
			this->hgtFilesGrid[i][j].fileName = fs::exists(path) ? fileName : "";
		}
	}

	for (int i = 0; i <= 180; i++) {
		this->hgtFilesGrid[i][360] = this->hgtFilesGrid[i][0];
	}
}


HgtFilesGrid::~HgtFilesGrid()
{
	delete[] this->dataStack;
}

/*
void HgtFilesGrid::Init(int maxLoadedFiles, std::string_view filesPath)
{
	this->loadedFiles = 0;
	this->MAX_LOADED_FILES = maxLoadedFiles;
	this->dataStack = new Heights[maxLoadedFiles];

	for (int i = 0; i <= 180; i++) {
		for (int j = 0; j <= 360; j++) {
			auto fileName = tfm::format("%s/%s.hgt", filesPath, HgtFormat::crdtodem(90 - i, -180 + j));
			auto path = fs::u8path(std::cbegin(fileName), std::cend(fileName));
			this->hgtFilesGrid[i][j].fileName = fs::exists(path) ? fileName : "";
		}
	}

	for (int i = 0; i <= 180; i++) {
		this->hgtFilesGrid[i][360] = this->hgtFilesGrid[i][0];
	}
}*/

bool HgtFilesGrid::IsExists(int i, int j){
	return 0 != this->hgtFilesGrid[i][j].fileName.length();
}

std::string HgtFilesGrid::GetFileName( int i, int j) {
	return this->hgtFilesGrid[i][j].fileName;
}

signed short HgtFilesGrid::GetHeight( int iSquare, int jSquare, int i, int j ) 
{
	if( this->hgtFilesGrid[iSquare][jSquare].isLoaded ) 
	{
		return this->hgtFilesGrid[iSquare][jSquare].pHeightData->height[i][j];
	}
	else 
	{
		if( IsExists(iSquare, jSquare) ) {
			/*
			FILE *fp;
			errno_t err;
			if( err = fopen_s( &fp, this->hgtFilesGrid[iSquare][jSquare].fileName.c_str(), "rb+") != 0 ) {
				return 0;
			}*/
			std::ifstream fp{ this->hgtFilesGrid[iSquare][jSquare].fileName.c_str(),
			                  std::ios::in | std::ios::binary };

			if( this->loadedFiles >= this->MAX_LOADED_FILES ) {
				this->loadedFiles = 0;
			}

			if( this->dataStack[this->loadedFiles].pFileFlag )
				this->dataStack[this->loadedFiles].pFileFlag->isLoaded = false;

			int size = this->NCols * this->NRows;
			//short int *buf = new short int[size];
			auto buf = std::vector<short int>{};
			buf.reserve(size);
			auto fsize = fp.tellg();
			fp.seekg(0, std::ios::beg);
			fp.read((char *)buf.data(), fsize);
			//fread( buf, sizeof(short int), size, fp);
			//fclose(fp);
			for( int i=0; i < size; i++ ) {
				auto low = buf[i] & 255;
				auto hi  = buf[i] >> 8;
				signed short height = (signed short)(low << 8 | (hi & 255));
				this->dataStack[this->loadedFiles].height[i / this->NRows][i % this->NCols] = height;
			}
			//delete[] buf;
			this->hgtFilesGrid[iSquare][jSquare].pHeightData = &this->dataStack[this->loadedFiles];
			this->dataStack[this->loadedFiles].pFileFlag = &this->hgtFilesGrid[iSquare][jSquare];
			this->loadedFiles++;
			this->hgtFilesGrid[iSquare][jSquare].isLoaded = true;

			return this->hgtFilesGrid[iSquare][jSquare].pHeightData->height[i][j];
		} 
		else
		{
			return 0;
		}
	}
}
