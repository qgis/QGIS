#ifndef qgisplugin_h
#define qgisplugin_h
#include <qstring.h>

class QgisPlugin{
public:
	virtual QString pluginName() = 0;
	virtual QString pluginVersion() =0;
	virtual QString pluginDescription() = 0;
};
typedef QgisPlugin* create_t();
typedef void unload_t(QgisPlugin *);
#endif //qgisplugin_h
