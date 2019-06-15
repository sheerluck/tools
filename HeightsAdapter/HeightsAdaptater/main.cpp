//#include <conio.h>
#include <iostream>
//#include <io.h>
//#include <direct.h>
#include "HgtFormat.h"
#include "log.h"
#include "vec3.h"
#include "HgtFilesGrid.h"
#include <filesystem>
namespace fs = std::filesystem;

const double PI = 3.141592653589793238462;
const double POLE = 20037508.34;

vec_t getHeightFromGrid(HgtFilesGrid& grid, const HgtFormat& format, int i, int j) {
	int demFileIndex_i = (int)floor((double)i / (double)(format.nrows - 1));
	int demFileIndex_j = (int)floor((double)j / (double)(format.ncols - 1));

	int i00 = i + demFileIndex_i - format.nrows * demFileIndex_i;
	int j00 = j + demFileIndex_j - format.ncols * demFileIndex_j;

	vec_t h = grid.GetHeight(demFileIndex_i, demFileIndex_j, i00, j00);

	return h;
}

double Merc2Lon(double x) {
	return 180.0 * x / POLE;
}

double Merc2Lat(double y) {
	return 180.0 / PI * (2 * atan(exp((y / POLE) * PI)) - PI / 2);
}

double Lon2Merc(double lon) {
	return lon * POLE / 180.0;
}

double Lat2Merc(double lat) {
	return log(tan((90.0 + lat) * PI / 360.0)) / PI * POLE;
}


double DegTail(double deg) {
	if (deg >= 0) {
		return deg - floor(deg);
	}
	return (-1)*floor(deg) + deg;
}

int main(int argc, char* argv[])
{
	// https://github.com/HowardHinnant/date
	// https://github.com/c42f/tinyformat

	LogAll("Adatation is started\n");
	LogAll("DEM files grid is creating...\n");
	auto demGrid = std::make_unique<HgtFilesGrid>(4, "elevation");
	LogAll("DEM files grid created.\n");
	
	std::string destTilesPath = "dest-earth3-14";

	LogAll("Preparing adaptation parameters...\n");


	constexpr int zoom = 14;
	constexpr int quadSize = 129;//33
	const int quadsCount = static_cast<int>(std::pow(2, zoom)); //(dstFieldSize - 1) / 32;
	const int dstFieldSize = quadsCount * (quadSize - 1) + 1;//524289;// = 2^19 + 1 > 1201 * 360 - (360 - 1)

	HgtFormat srcHgtFormat(1201, 1201, 1.0 / 1200.0);// 3 / 3600 == 1 / ( 1201 - 1 ) deg.
	HgtFormat srcField(srcHgtFormat.nrows * 180 - (180 - 1), srcHgtFormat.ncols * 360 - (360 - 1));
	HgtFormat dstField(dstFieldSize, dstFieldSize);

	int quadSize2 = quadSize * quadSize;
	vec_t* quadHeightData = new vec_t[quadSize2];

	vec_t incLat_merc, incLon_merc;

	incLat_merc = incLon_merc = 2.0 * POLE / (vec_t)(dstFieldSize - 1);

	vec3_t tr[3];
	vec3_t line[2];
	vecSet(line[0], 0.0, 1000000.0, 0.0);
	vecSet(line[1], 0.0, -1000000.0, 0.0);
	LogAll("Adaptation prepared to proceed...\n");

	double lon_d = 0, lat_d = 0;
	double coordi = 0, coordj = 0;

	for (int qm = 0; qm < quadsCount; qm++) {
		std::cout << qm << " ";
		if (qm > 14759)
			for (int qn = 0; qn < quadsCount; qn++)
			{
				bool isZeroHeight = true;

				for (int i = 0; i < quadSize; i++) {
					for (int j = 0; j < quadSize; j++) {
						coordi = POLE - ((quadSize - 1) * qm + i) * incLat_merc;
						coordj = (-1)*POLE + ((quadSize - 1) * qn + j) * incLon_merc;

						lat_d = Merc2Lat(coordi);
						lon_d = Merc2Lon(coordj);

						int demFileIndex_i = (int)ceil(90.0 - lat_d);
						int demFileIndex_j = (int)floor(180.0 + lon_d);

						vec_t onedlat = DegTail(lat_d);
						vec_t onedlon = DegTail(lon_d);

						int indLat = (int)floor(onedlat / srcHgtFormat.cellsize);
						int i00 = 1200 - 1 - indLat;
						int j00 = (int)floor(onedlon / srcHgtFormat.cellsize);

						vec_t h00 = demGrid->GetHeight(demFileIndex_i, demFileIndex_j, i00, j00);
						vec_t h01 = demGrid->GetHeight(demFileIndex_i, demFileIndex_j, i00, j00 + 1);
						vec_t h10 = demGrid->GetHeight(demFileIndex_i, demFileIndex_j, i00 + 1, j00);

						vec_t cornerLat = 90 - demFileIndex_i;
						vec_t cornerLon = -180 + demFileIndex_j;

						vecSet(tr[0],
							cornerLon + j00 * srcHgtFormat.cellsize,
							h00,
							cornerLat + (indLat + 1) * srcHgtFormat.cellsize);

						vecSet(tr[2],
							cornerLon + (j00 + 1) * srcHgtFormat.cellsize,
							j00 < srcHgtFormat.ncols - 1 ? h01 : h00,
							cornerLat + (indLat + 1) * srcHgtFormat.cellsize);

						vecSet(tr[1],
							cornerLon + j00 * srcHgtFormat.cellsize,
							i00 < srcHgtFormat.nrows - 1 ? h10 : h00,
							cornerLat + indLat * srcHgtFormat.cellsize);

						auto X = static_cast<int>(XYZ::X);
						auto Y = static_cast<int>(XYZ::Y);
						auto Z = static_cast<int>(XYZ::Z);
						line[0][X] = line[1][X] = lon_d;
						line[0][Z] = line[1][Z] = lat_d;

						vec_t h11 = 0;

						vec_t edge = ((line[0][X] - tr[2][X]) * (tr[1][Z] - tr[2][Z]) - (line[0][Z] - tr[2][Z]) * (tr[1][X] - tr[2][X]));

						if (edge < 0.0)
						{
							h11 = demGrid->GetHeight(demFileIndex_i, demFileIndex_j, i00 + 1, j00 + 1);

							vecSet(tr[0],
								cornerLon + (j00 + 1) * srcHgtFormat.cellsize,
								i00 < srcHgtFormat.nrows - 1 && j00 < srcHgtFormat.ncols - 1 ? h11 :
								i00 < srcHgtFormat.nrows - 1 ? h10 :
								j00 < srcHgtFormat.nrows - 1 ? h01 :
								h00,
								cornerLat + indLat * srcHgtFormat.cellsize);
						}

						quadHeightData[i * quadSize + j] = 0;

						if (h00 != 0 || h01 != 0 || h10 != 0 || h11 != 0) {
							vec_t h = LineIntersectPlane(tr, line);
							quadHeightData[i * quadSize + j] = h;
							if (h > 0)
								isZeroHeight = false;
						}
					}
				}


				if (!isZeroHeight) {
					auto subdir = tfm::format("%s/%d", destTilesPath, qm);
					{
						auto path = fs::u8path(std::cbegin(subdir), std::cend(subdir));
						if (!fs::exists(subdir)) fs::create_directories(path);
					}
					auto fileName = tfm::format("%s/%s.ddm", subdir, qn);
					auto path = fs::u8path(std::cbegin(fileName), std::cend(fileName));
					std::ofstream fp{ path, std::ios::ate | std::ios::binary };
					for (int i = 0; i < quadSize2; i++) {
						fp << static_cast<float>(quadHeightData[i]);
					}

					/*
					char ccn[10];
					FILE *fp;
					errno_t err;

					_itoa(qm, ccn, 10);
					_mkdir(std::string(destTilesPath).append(ccn).c_str());

					std::string fileName(destTilesPath);
					fileName.append(ccn).append("\\");
					fileName.append(_itoa(qn, ccn, 10)).append(".ddm");

					if (err = fopen_s(&fp, fileName.c_str(), "wb") != 0) {
						LogAll(std::string("Error: ").append(fileName).append("\n").c_str());
						return;
					}

					float* quadHeightData_fl = new float[quadSize2];
					for (int i = 0; i < quadSize2; i++) {
						quadHeightData_fl[i] = static_cast<float>(quadHeightData[i]);
					}

					fwrite(quadHeightData_fl, sizeof(float), quadSize2, fp);
					delete[] quadHeightData_fl;
					fclose(fp);*/
				}

			}
	}

	delete[] quadHeightData;
	
	LogAll("Adaptaion successfully complete.\n");
}
