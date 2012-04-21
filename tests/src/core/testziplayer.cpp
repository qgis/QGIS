/***************************************************************************
     testziplayer.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2012 Tim Sutton
    Email                : tim@linfiniti.com
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
#include <QObject>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>

/** \ingroup UnitTests
 * This is a unit test to verify that zip vector layers work
 */
class TestZipLayer: public QObject
{
    Q_OBJECT;

  private slots:

    void testZipLayer()
    {
      QgsApplication::init();
      QgsProviderRegistry::instance( QgsApplication::pluginPath() );
      //
      //create a point layer that will be used in all tests...
      //
      QString myDataDir( TEST_DATA_DIR );
      myDataDir += QDir::separator();
      QString myPointsFileName = myDataDir + "points.zip";
      QFileInfo myPointFileInfo( myPointsFileName );
      QgsVectorLayer * mypPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
          myPointFileInfo.completeBaseName(), "ogr" );
      QVERIFY( mypPointsLayer->isValid() );
      delete mypPointsLayer;
    }

};

QTEST_MAIN( TestZipLayer )
#include "moc_testziplayer.cxx"




