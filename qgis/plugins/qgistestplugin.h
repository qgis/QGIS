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
