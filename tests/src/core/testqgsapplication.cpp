/***************************************************************************
     testqgsapplication.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:49 AKDT 2007
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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <iostream>

#include <QApplication>
#include <QImage>
#include <QPainter>

#include <qgsapplication.h>
#include <qgsmaplayerregistry.h>
#include <qgsmaprender.h>
#include <qgsspatialrefsys.h>
#include <qgsvectorlayer.h>

//header for class being tested
#include <qgsapplication.h>

class TestQgsApplication: public QObject
{
  Q_OBJECT;
  private slots:
        void QgsApplicationQgsApplication()
        {
          // init QGIS's paths - true means that all path will be inited from prefix
          QString qgisPath = QCoreApplication::applicationDirPath ();
          QgsApplication::setPrefixPath(qgisPath, TRUE);

          std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
          std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
          std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
          std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

          QgsApplication::initQgis();
/*          QStringList myLayerList;
          myLayerList << "../../../points.shp";
          QStringList::Iterator myIterator = myLayerList.begin();
          while( myIterator!= myLayerList.end() )
          {
            QString myLayerName=*myIterator;
            QgsVectorLayer* layer = new QgsVectorLayer(myLayerName, myLayerName, "ogr");
            if (layer && layer->isValid())
            {
              std::cout << "Added layer is valid " << std::endl;
            }
            // something's wrong
            else if (layer)
            {
              //test must be failed here!
              delete layer;
            }
            //Register this layer with the layers registry
            QgsMapLayerRegistry::instance()->addMapLayer(layer);
            ++myIterator;

          }


          // create image and painter for it
          QImage img(QSize(800,600), QImage::Format_ARGB32_Premultiplied);
          QColor bgColor(255,255,255);
          img.fill(bgColor.rgb());
          QPainter p;
          p.begin(&img);
          p.setRenderHint(QPainter::Antialiasing); // use antialiasing!

          QgsMapRender mapRender;

          // set layers
          mapRender.setLayerSet(myLayerList);

          // use projection
          mapRender.setProjectionsEnabled(TRUE);
          QgsSpatialRefSys srs(32633); // postgis srid: wgs84 / utm zone 33N
          mapRender.setDestinationSrs(srs);

          // set extent
          QgsRect r = mapRender.fullExtent();
          r.scale(1.1); // make rectangle a bit bigger
          mapRender.setExtent(r);

          // set output size
          mapRender.setOutputSize(img.size(), img.logicalDpiX());

          // render!
          mapRender.render(&p);

          // painting done
          p.end();

          // delete layers
          QgsMapLayerRegistry::instance()->removeAllMapLayers();

          // save image
          img.save("render.png","png");*/
          QgsApplication::exitQgis();
        };
    void QgsApplicationselectTheme()
{

};
    void QgsApplicationauthorsFilePath()
{

};
    void QgsApplicationdeveloperPath()
{

};
    void QgsApplicationhelpAppPath()
{

};
    void QgsApplicationi18nPath()
{

};
    void QgsApplicationqgisMasterDbFilePath()
{

};
    void QgsApplicationqgisSettingsDirPath()
{

};
    void QgsApplicationqgisUserDbFilePath()
{

};
    void QgsApplicationsplashPath()
{

};
    void QgsApplicationsrsDbFilePath()
{

};
    void QgsApplicationsvgPath()
{

};

};

QTEST_MAIN(TestQgsApplication)
#include "moc_testqgsapplication.cxx"

