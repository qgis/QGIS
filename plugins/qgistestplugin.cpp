/* Test plugin for QGis
* This code is a test plugin for QGis and a demonstration of the API
* All QGis plugins must inherit from the abstract base class QgisPlugin. A
* plugin must implement the virtual functions defined in QgisPlugin:
* 	*pluginName
*	*pluginVersion
*	*pluginDescription
*
* This list will grow as the API is expanded.
* 
* In addition, a plugin must implement a the classFactory and unload
* functions. Note that these functions must be declared as extern "C"
*/
#include <qstring.h>

#ifndef qgisplugintest_h
#define qgisplugintest_h
#include "qgisplugin.h"

class QgisTestPlugin : public QgisPlugin{
public:
	QgisTestPlugin();
	virtual QString pluginName();
	virtual QString pluginVersion();
	virtual QString pluginDescription();
	virtual ~QgisTestPlugin();
private:
	QString name;
	QString version;
	QString description;
};

#endif
QgisTestPlugin::QgisTestPlugin(){
	name = "Test Plugin";
	version = "Version 0.0";
	description = "This test plugin does nothing but tell you its name, version, and description";
}
QgisTestPlugin::~QgisTestPlugin(){
	
}
QString QgisTestPlugin::pluginName(){
	return name;
}
QString QgisTestPlugin::pluginVersion(){
	return version;
	
}
QString QgisTestPlugin::pluginDescription(){
	return description;
	
}
extern "C" QgisPlugin * classFactory(){
	return new QgisTestPlugin();
}

extern "C" void unload(QgisPlugin *p){
	delete p;
}
