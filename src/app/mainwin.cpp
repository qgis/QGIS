
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include "Shlwapi.h"

typedef int (*f_main)(int, char*[]);

int main( int argc, char *argv[] )
{
  // TODO Handle running from build dir.
	char filepath[MAX_PATH];
	GetModuleFileName(NULL, filepath, MAX_PATH);
	char *path;
	path = filepath;
	PSTR name = PathFindFileName(path);
	PathRemoveFileSpec(path);
	PathRemoveFileSpec(path);
	std::cout << "The filepath is\n " << path << std::endl;
	std::cout << "The filename is\n " << name << std::endl;
//	std::string root = std::string(path);
	std::string root = std::string("C:\\OSGeo4W");
	std::string binroot = root + std::string("\\bin");
	std::string appsroot = root + std::string("\\apps\\");
	// TODO Replace with QGIS name from exe
	std::string qgisfolder = appsroot + std::string("qgis");
	std::string pythonfolder = appsroot + std::string("Python36");

	std::string envpythonhome = std::string("PYTHONHOME=") + pythonfolder;
	std::string envpluginpath = std::string("QT_PLUGIN_PATH=") + appsroot +
				 std::string("Qt5\\plugins;") +
				 qgisfolder + std::string("\\qtplugins");

	std::string envgdalshare = std::string("GDAL_DATA=") + root + std::string("share\\gdal");
	std::string envgdaldriverpath = std::string("GDAL_DRIVER_PATH=") + binroot + std::string("gdalplugins");

	std::stringstream ss;
	ss << "PATH=";
	ss << qgisfolder;
	ss << ";" << appsroot + std::string("qt5\\bin");
	ss << ";" << pythonfolder;
	ss << ";" << binroot;
	ss << ";" << getenv("PATH");
	putenv(ss.str().c_str());
	putenv("PYTHONPATH=");
	putenv(envpythonhome.c_str());
	putenv(envpluginpath.c_str());

	// GDAL ENV
	putenv("VSI_CACHE=TRUE");
	putenv("VSI_CACHE_SIZE=1000000");
	putenv("GDAL_FILENAME_IS_UTF8=TRUE");

	putenv(envgdalshare.c_str());
	putenv(envgdaldriverpath.c_str());

	std::cout << "The current path is\n " << getenv("PATH") << std::endl;
	std::cout << "The current QT_PLUGIN_PATH is\n " << getenv("QT_PLUGIN_PATH") << std::endl;
	std::cout << "The current PYTHONHOME is\n " << getenv("PYTHONHOME") << std::endl;

	std::cout << "Loading lib" << std::endl;
	HINSTANCE hGetProcIDDLL = LoadLibrary("qgis_app.dll");
	if (!hGetProcIDDLL) {
		std::cout << "could not load the qgis_app.dll library" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Get address" << std::endl;
	f_main realmain = (f_main)GetProcAddress(hGetProcIDDLL, "main");
	if (!realmain) {
		std::cout << "could not locate main function in qgis_app.dll" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Calling real main " << std::endl;
	return realmain(argc, argv);
}
