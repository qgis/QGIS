
#ifndef qgisplugintest_h
#define qgisplugintest_h
#include "qgisplugin.h"
#include <qwidget.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include "qgisplugin.h"
#include "qgistestplugin.h"
#include "qgsworkerclass.h"
#include "../src/qgisapp.h"


class QgisTestPlugin : public QObject, public QgisPlugin{
Q_OBJECT
public:
	QgisTestPlugin(QgisApp *qgis, QgisIface *qI);
	virtual QString name();
	virtual QString version();
	virtual QString description();
	
	virtual ~QgisTestPlugin();
public slots:
	void open();
	void newThing();
	void save();
private:
	QString pName;
	QString pVersion;
	QString pDescription;
	
	QgisApp *qgisMainWindow;
	QgisIface *qI;
};

#endif
