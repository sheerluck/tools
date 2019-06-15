//#include <conio.h>
#include <iostream>
#include "date.h"
#include <string_view>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

constexpr std::string_view logAllFileName = "all.log";
constexpr std::string_view logErrFileName = "err.log";

enum class LOGTYPE {
	ALL,
	ERROR
};

/*void getDateTime( char* dateTime) {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strcpy_s( dateTime, 32, asctime(timeinfo) );
	int len = strlen(dateTime);
	dateTime[len-1] = ':';
	dateTime[len] = ' ';
	dateTime[len+1] = '\0';
}*/

inline void writeLog(std::string_view logFileName, std::string_view dateTime, std::string_view msg) {
	auto path = fs::u8path(std::cbegin(logFileName), std::cend(logFileName));
	std::ofstream fp{ path, std::ios::app /*| std::ios::binary*/ };
	if (!fp)
		throw std::runtime_error("Cannot open " + std::string{logFileName});
	fp << dateTime << msg;
	/*
	FILE* fp;
	errno_t err;
	if( err = fopen_s( &fp, logAllFileName, "a+t") != 0 ) {
		std::cout << dateTime << " log file error\n";
		return;
	}
	fwrite( dateTime, sizeof(char), strlen(dateTime), fp);
	fwrite( msg, sizeof(char), strlen(msg), fp);
	fclose(fp);*/
}

void Log( const char* msg, LOGTYPE type = LOGTYPE::ALL ) {
	std::cout << msg;
	auto currTime = []() {
		using namespace date;
		std::ostringstream ss;
		ss << std::chrono::system_clock::now() << ' ';
		return ss.str();
	}();
	writeLog( logAllFileName, currTime, msg );
	if( type == LOGTYPE::ERROR ) {
		writeLog( logErrFileName, currTime, msg );
	}
}

inline void LogAll(const char* msg) {
	Log(msg);
}

inline void LogErr(const char* msg) {
	Log(msg, LOGTYPE::ERROR);
}
