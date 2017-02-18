
#include <fstream>
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include <filesystem>
#include "Shlwapi.h"

using namespace std::experimental::filesystem::v1;

typedef int (*f_main)(int, char*[]);

int main( int argc, char *argv[] )
{
	//TODO There is a better way to do this because MAX_PATH isn't
	// enough anymore.
	// TODO Test network paths, etc
	char filepath[MAX_PATH];
	GetModuleFileName(NULL, filepath, MAX_PATH);

	std::experimental::filesystem::path buildfile("qgisbuildpath.txt");
	bool runningfrombuild = std::experimental::filesystem::exists(buildfile);
	std::string qgisappdll = "qgis_app.dll";
	if (!runningfrombuild)
		{
			std::experimental::filesystem::path path(filepath);
			// Extract the folder name from the exe
			std::string name = path.filename().string();
			std::size_t found = name.find(".");
			std::string foldername = name.substr(0, found);

			std::string root = path.parent_path().parent_path().string();
			std::string binroot = root + std::string("\\bin");
			std::string appsroot = root + std::string("\\apps\\");

			// Uses the exe name to find the real qgis folder with all the libs.
			std::string qgisfolder = appsroot + std::string(foldername);
			qgisappdll = qgisfolder + std::string("\\bin\\") + qgisappdll;
			std::string pythonfolder = appsroot + std::string("Python36");

			std::string envpythonhome = std::string("PYTHONHOME=") + pythonfolder;
			std::string envpluginpath = std::string("QT_PLUGIN_PATH=") + appsroot +
					std::string("Qt5\\plugins;") +
					qgisfolder + std::string("\\qtplugins");

			std::string envgdalshare = std::string("GDAL_DATA=") + root + std::string("\\share\\gdal");
			std::string envgdaldriverpath = std::string("GDAL_DRIVER_PATH=") + binroot + std::string("\\gdalplugins");

			std::string envqgisprefix = std::string("QGIS_PREFIX_PATH=") + qgisfolder;
			std::replace(envqgisprefix.begin(), envqgisprefix.end(), '\\', '/');

			std::stringstream ss;
			ss << "PATH=";
			ss << qgisfolder;
			ss << ";" << qgisfolder + std::string("\\bin");
			ss << ";" << appsroot + std::string("Qt5\\bin");
			ss << ";" << pythonfolder;
			ss << ";" << binroot;
			ss << ";" << getenv("PATH");
			putenv(ss.str().c_str());
			putenv("PYTHONPATH=");
			putenv(envpythonhome.c_str());
			putenv(envpluginpath.c_str());
			putenv(envqgisprefix.c_str());

			// GDAL ENV
			putenv("VSI_CACHE=TRUE");
			putenv("VSI_CACHE_SIZE=1000000");
			putenv("GDAL_FILENAME_IS_UTF8=TRUE");

			putenv(envgdalshare.c_str());
			putenv(envgdaldriverpath.c_str());

			OutputDebugString("FILEPATH\n");
			OutputDebugString(path.string().c_str());
			OutputDebugString("QGIS FOLDER NAME\n");
			OutputDebugString(foldername.c_str());
			OutputDebugString("QGIS_PREFIX_PATH\n");
			OutputDebugString(envqgisprefix.c_str());
		}

	OutputDebugString("PATH\n");
	OutputDebugString(getenv("PATH"));
	OutputDebugString("\n");
	OutputDebugString("QT_PLUGIN_PATH\n");
	OutputDebugString(getenv("QT_PLUGIN_PATH"));
	OutputDebugString("\n");
	OutputDebugString("PYTHONHOME\n");
	OutputDebugString(getenv("PYTHONHOME"));
	OutputDebugString("\n");

	OutputDebugString("Loading qgis_app.dll from \n");
	OutputDebugString(qgisappdll.c_str());

	HINSTANCE hGetProcIDDLL = LoadLibrary(qgisappdll.c_str());
	if (!hGetProcIDDLL) {
			OutputDebugString("Could not load the qgis_app.dll");
			std::cout << "Could not load the qgis_app.dll library" << std::endl;
			return EXIT_FAILURE;
		}

	f_main realmain = (f_main)GetProcAddress(hGetProcIDDLL, "main");
	if (!realmain) {
			OutputDebugString("Can't find main...");
			std::cout << "could not locate main function in qgis_app.dll" << std::endl;
			return EXIT_FAILURE;
		}

	OutputDebugString("Calling real main..");
	std::cout << "Calling real main " << std::endl;
	return realmain(argc, argv);

}
