/***************************************************************************
     testqgsdiagram.cpp
     --------------------------------------
    Date                 : Sep 7 2012
    Copyright            : (C) 2012 by Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QPainter>

#include <iostream>
//qgis includes...
// #include <qgisapp.h>
#include <diagram/qgspiediagram.h>
#include <qgsdiagramrendererv2.h>
#include <qgscomposition.h>
#include <qgscompositionchecker.h>
#include <qgscomposermap.h>
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgsrendererv2.h>
//qgis test includes
#include "qgsrenderchecker.h"
#include "qgspallabeling.h"
#include "qgsproject.h"

/** \ingroup UnitTests
 * This is a unit test for the vector layer class.
 */
class TestQgsDiagram : public QObject
{
    Q_OBJECT
  private:
    bool mTestHasError;
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mPointsLayer;
    QgsComposition * mComposition;
    QString mTestDataDir;
    QString mReport;
    QgsPieDiagram * mPieDiagram;
    QgsComposerMap * mComposerMap;

  private slots:


    // will be called before the first testfunction is executed.
    void initTestCase()
    {
      mTestHasError = false;
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsApplication::showSettings();

      //create some objects that will be used in all tests...

      //
      //create a non spatial layer that will be used in all tests...
      //
      QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
      mTestDataDir = myDataDir + QDir::separator();

      //
      //create a point layer that will be used in all tests...
      //
      QString myPointsFileName = mTestDataDir + "points.shp";
      QFileInfo myPointFileInfo( myPointsFileName );
      mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                         myPointFileInfo.completeBaseName(), "ogr" );

      // Register the layer with the registry
      QgsMapLayerRegistry::instance()->addMapLayers(
        QList<QgsMapLayer *>() << mPointsLayer );

      // Create diagrams
      mPieDiagram = new QgsPieDiagram();

      // Create map composition to draw on
      mMapSettings.setLayers( QStringList() << mPointsLayer->id() );
      // TODO mMapSettings.setLabelingEngine( new QgsPalLabeling() );
      mComposition = new QgsComposition( mMapSettings );
      mComposition->setPaperSize( 297, 210 ); // A4 landscape
      mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
      mComposerMap->setFrameEnabled( true );
      mComposition->addComposerMap( mComposerMap );

      mReport += "<h1>Diagram Tests</h1>\n";
    }

    // will be called after the last testfunction was executed.
    void cleanupTestCase()
    {
      delete mComposition;
      QgsApplication::exitQgis();
      QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
      QFile myFile( myReportFile );
      if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
      {
        QTextStream myQTextStream( &myFile );
        myQTextStream << mReport;
        myFile.close();
        //QDesktopServices::openUrl( "file:///" + myReportFile );
      }
    }
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void testPieDiagram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << "\"Pilots\"" << "\"Cabin Crew\"";
      ds.maxScaleDenominator = -1;
      ds.minScaleDenominator = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsDiagramSettings::MM;
      ds.size = QSizeF( 15, 15 );
      ds.angleOffset = 0;


      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 100, 100 ) );
      dr->setClassificationAttribute( 5 ); // Staff
      dr->setDiagram( mPieDiagram );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.placement = QgsDiagramLayerSettings::OverPoint;

      QgsProject::instance()->writeEntry( "PAL", "/ShowingAllLabels", true );

      mPointsLayer->setDiagramLayerSettings( dls );

      mComposerMap->setNewExtent( QgsRectangle( -122, -79, -70, 47 ) );
      QgsCompositionChecker checker( "piediagram", mComposition );

      QVERIFY( checker.testComposition( mReport ) );

      mPointsLayer->setDiagramRenderer( 0 );
    }
};

QTEST_MAIN( TestQgsDiagram )
#include "testqgsdiagram.moc"
