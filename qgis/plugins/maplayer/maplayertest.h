#ifndef QGSMAPLAYERTESTPLUGIN_H
#define QGSMAPLAYERTESTPLUGIN_H
#include <qmainwindow.h>
#include <qmenubar.h>
#include "../../src/qgsmaplayerinterface.h"
#include "../../src/qgscoordinatetransform.h"
class MapLayerTest : public QgsMapLayerInterface{
Q_OBJECT
public:
MapLayerTest();
void setQgisMainWindow(QMainWindow *app);
void setCoordinateTransform(QgsCoordinateTransform *xform);
public slots:
void initGui();
void open();
void unload();
void draw();
private:
	QMainWindow *qgisApp;
	QMenuBar *menu;
	int menuId;
	QgsCoordinateTransform *coordTransform;
};


#endif //QGSMAPLAYERTESTPLUGIN_H
