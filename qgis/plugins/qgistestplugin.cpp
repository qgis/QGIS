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
#include "qgisplugingui.h"

class QgisTestPlugin : public QgisPlugin{
public:
	QgisTestPlugin();
	virtual QString name();
	virtual QString version();
	virtual QString description();
	virtual QgisPluginGui *gui();
	virtual ~QgisTestPlugin();
private:
	QString pName;
	QString pVersion;
	QString pDescription;
};

#endif
QgisTestPlugin::QgisTestPlugin(){
	pName = "Test Plugin";
	pVersion = "Version 0.0";
	pDescription = "This test plugin does nothing but tell you its name, version, and description";
}
QgisTestPlugin::~QgisTestPlugin(){
	
}
QString QgisTestPlugin::name(){
	return pName;
}
QString QgisTestPlugin::version(){
	return pVersion;
	
}
QString QgisTestPlugin::description(){
	return pDescription;
	
}
QgisPluginGui * QgisTestPlugin::gui(){
	// stub
	return 0;
}

extern "C" QgisPlugin * classFactory(){
	return new QgisTestPlugin();
}

extern "C" void unload(QgisPlugin *p){
	delete p;
}
