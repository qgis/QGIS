#ifndef QGSMAPLAYERTESTPLUGIN_H
#define QGSMAPLAYERTESTPLUGIN_H
#include <qmainwindow.h>
#include <qmenubar.h>
#include "../../src/qgsmaplayerinterface.h"
#include "../../src/qgsmaptopixel.h"
class MapLayerTest : public QgsMapLayerInterface{
Q_OBJECT
public:
MapLayerTest();
void setQgisMainWindow(QMainWindow *app);
void setCoordinateTransform(QgsMapToPixel *xform);
public slots:
void initGui();
void open();
void unload();
void draw();
private:
	QMainWindow *qgisApp;
	QMenuBar *menu;
	int menuId;
	QgsMapToPixel *coordTransform;
};


#endif //QGSMAPLAYERTESTPLUGIN_H
