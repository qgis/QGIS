#ifdef _MSC_VER
#undef APP_EXPORT
#define APP_EXPORT __declspec(dllimport)
#endif

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>

typedef int (*f_main)(int, char*[]);

int main( int argc, char *argv[] )
{
	std::cout << "FIRST ARG" << argv[0] << std::endl;
	std::stringstream ss;
	ss << "PATH=";
	ss << "c:\\osgeo4w\\apps\\qgis";
	ss << ";" << "c:\\osgeo4w\\apps\\qt5\\bin";
	ss << ";" << "c:\\osgeo4w\\bin";
	ss << ";" << "c:\\osgeo4w\\apps\\Python36";
	ss << ";" << getenv("PATH");
	std::string env = ss.str();
	putenv(env.c_str());
	putenv("PYTHONPATH=");
	putenv("PYTHONHOME=c:\\osgeo4w\\apps\\Python36");
	putenv("QT_PLUGIN_PATH=C:\\OSGeo4W\\apps\\Qt5\\plugins;C:\\OSGeo4W\\apps\\qgis\\qtplugins");
	std::cout << "The current path is\n " << getenv("PATH") << std::endl;
	std::cout << "The current QT_PLUGIN_PATH is\n " << getenv("QT_PLUGIN_PATH") << std::endl;

	HINSTANCE hGetProcIDDLL = LoadLibrary("qgis_app.dll");
	if (!hGetProcIDDLL) {
			std::cout << "could not load the dynamic library" << std::endl;
			return EXIT_FAILURE;
		}

	f_main realmain = (f_main)GetProcAddress(hGetProcIDDLL, "main");
	if (!realmain) {
			std::cout << "could not locate the function" << std::endl;
			return EXIT_FAILURE;
		}

	return realmain(argc, argv);
}
