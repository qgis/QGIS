/***************************************************************************
     maplayertest.h
     --------------------------------------
    Date                 : Sun Sep 16 12:10:21 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPLAYERTESTPLUGIN_H
#define QGSMAPLAYERTESTPLUGIN_H
#include <q3mainwindow.h>
#include <qmenubar.h>
#include "../../src/qgsmaplayerinterface.h"
#include "../../src/qgsmaptopixel.h"
class MapLayerTest : public QgsMapLayerInterface
{
    Q_OBJECT
  public:
    MapLayerTest();
    void setQgisMainWindow( Q3MainWindow *app );
    void setCoordinateTransform( QgsMapToPixel *xform );
  public slots:
    void initGui();
    void open();
    void unload();
    void draw();
  private:
    Q3MainWindow *qgisApp;
    QMenuBar *menu;
    int menuId;
    QgsMapToPixel *coordinateTransform;
};


#endif //QGSMAPLAYERTESTPLUGIN_H
