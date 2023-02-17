/***************************************************************************
     TestQgsMapToolTrimExtendFeature.cpp
     --------------------------------
    Date                 : October 2018
    Copyright            : (C) 2018 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgssnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgisapp.h"

#include "qgsmaptooltrimextendfeature.h"


class TestQgsMapToolTrimExtendFeature : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolTrimExtendFeature() = default;

  private:
    std::unique_ptr<QgsVectorLayer> vlPolygon, vlMultiLine, vlLineZ, vlTopoEdit, vlTopoLimit;
    QgsFeature f1, f2;
    QgsMapCanvas *mCanvas = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      // Will make sure the settings dir with the style file for color ramp is created
      QgsApplication::createDatabase();
      QgsApplication::showSettings();


      // vector layer with a triangle in a rectangle:
      // (0,3) +-------------------+ (3,3)
      //       | (1,2) +---+ (2,2) |
      //       |        \  |       |
      //       |         \ |       |
      //       |          \|       |
      //       |           + (2,1) |
      // (0,0) +-------------------+ (3,0)
      vlPolygon.reset( new QgsVectorLayer( QStringLiteral( "MultiPolygon?field=fld:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) ) );
      const int idx = vlPolygon->fields().indexFromName( QStringLiteral( "fld" ) );
      QVERIFY( idx != -1 );
      f1.initAttributes( 1 );
      f2.initAttributes( 1 );

      QgsPolygonXY polygon;
      QgsPolylineXY polyline;
      polyline << QgsPointXY( 1, 2 ) << QgsPointXY( 2, 1 ) << QgsPointXY( 2, 2 ) << QgsPointXY( 1, 2 );
      polygon << polyline;
      const QgsGeometry polygonGeom = QgsGeometry::fromPolygonXY( polygon );
      f1.setGeometry( polygonGeom );
      f1.setAttribute( idx, QVariant( 1 ) );
      QgsFeatureList flist;
      flist << f1;
      vlPolygon->dataProvider()->addFeatures( flist );

      QgsPolygonXY polygon2;
      QgsPolylineXY polyline2;
      polyline2 << QgsPointXY( 0, 0 ) << QgsPointXY( 3, 0 ) << QgsPointXY( 3, 3 ) << QgsPointXY( 0, 3 ) << QgsPointXY( 0, 0 );
      polygon2 << polyline2;
      const QgsGeometry polygonGeom2 = QgsGeometry::fromPolygonXY( polygon2 );
      f2.setGeometry( polygonGeom2 );
      f2.setAttribute( idx, QVariant( 2 ) );
      QgsFeatureList flist2;
      flist2 << f2;
      vlPolygon->dataProvider()->addFeatures( flist2 );


      /*
       *      |
       *  |   |
       *      |
       * -----|
       *      |
       */

      vlMultiLine.reset( new QgsVectorLayer( QStringLiteral( "MultiLineString?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
      QVERIFY( vlMultiLine->isValid() );
      QgsFeature multi( vlMultiLine->dataProvider()->fields(), 1 );
      multi.setAttribute( QStringLiteral( "pk" ), 1 );
      multi.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
                           "MultiLineString ((10 0, 14 0),(11 1, 11 0.5),(14 -2, 14 2))" ) ) );

      vlMultiLine->dataProvider()->addFeatures( QgsFeatureList() << multi );


      /*     (3 8 200)
             /
            /  (2 6 10)
           /  \
      (0 5 100)      (3 5 5)
      */
      vlLineZ.reset( new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
      QVERIFY( vlLineZ->isValid() );
      QgsFeature linez( vlLineZ->dataProvider()->fields(), 1 );
      linez.setAttribute( QStringLiteral( "pk" ), 1 );
      linez.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
                           "LineStringZ (3 5 5, 2 6 10)" ) ) );
      QgsFeature linez2( vlLineZ->dataProvider()->fields(), 2 );
      linez2.setAttribute( QStringLiteral( "pk" ), 2 );
      linez2.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
                            "LineStringZ (0 5 100, 3 8 200)" ) ) );

      vlLineZ->dataProvider()->addFeatures( QgsFeatureList() << linez << linez2 );


      /*
       *       |
       * ----  |
       *       |
       *
       */
      vlTopoEdit.reset( new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
      QVERIFY( vlTopoEdit->isValid() );
      vlTopoLimit.reset( new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
      QVERIFY( vlTopoLimit->isValid() );
      QgsFeature lineEdit( vlTopoEdit->dataProvider()->fields(), 1 );
      lineEdit.setAttribute( QStringLiteral( "pk" ), 1 );
      lineEdit.setGeometry( QgsGeometry::fromWkt( QStringLiteral( " LineString (20 15, 25 15) " ) ) );
      QgsFeature lineLimit( vlTopoLimit->dataProvider()->fields(), 1 );
      lineLimit.setAttribute( QStringLiteral( "pk" ), 1 );
      lineLimit.setGeometry( QgsGeometry::fromWkt( QStringLiteral( " LineString (30 0, 30 30) " ) ) );

      vlTopoEdit->dataProvider()->addFeatures( QgsFeatureList() << lineEdit );
      vlTopoLimit->dataProvider()->addFeatures( QgsFeatureList() << lineLimit );


      mCanvas = new QgsMapCanvas();
      mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
      mCanvas->setLayers( QList<QgsMapLayer *>() << vlPolygon.get() << vlMultiLine.get() << vlLineZ.get() << vlTopoEdit.get() << vlTopoLimit.get() );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 50, 50 ) );
      mapSettings.setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      mapSettings.setLayers( QList<QgsMapLayer *>() << vlPolygon.get() << vlMultiLine.get() << vlLineZ.get() << vlTopoEdit.get() << vlTopoLimit.get() );

      QgsSnappingUtils *mSnappingUtils = new QgsMapCanvasSnappingUtils( mCanvas, this );
      QgsSnappingConfig snappingConfig = mSnappingUtils->config();
      mSnappingUtils->setMapSettings( mapSettings );
      snappingConfig.setEnabled( true );
      snappingConfig.setTolerance( 100 );
      snappingConfig.setTypeFlag( static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) );
      snappingConfig.setUnits( QgsTolerance::Pixels );
      snappingConfig.setMode( Qgis::SnappingMode::AllLayers );
      mSnappingUtils->setConfig( snappingConfig );

      mSnappingUtils->locatorForLayer( vlPolygon.get() )->init();
      mSnappingUtils->locatorForLayer( vlMultiLine.get() )->init();
      mSnappingUtils->locatorForLayer( vlLineZ.get() )->init();
      mSnappingUtils->locatorForLayer( vlTopoEdit.get() )->init();
      mSnappingUtils->locatorForLayer( vlTopoLimit.get() )->init();

      mCanvas->setSnappingUtils( mSnappingUtils );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }



    void testPolygon()
    {

      // vector layer with a triangle in a rectangle:
      // (0,3) +-------------------+ (3,3)
      //       | (1,2) +---+ (2,2) |
      //       |        \  |       |
      //       |         \ |       |
      //       |          \|       |
      //       |           + (2,1) |
      // (0,0) +-------------------+ (3,0)
      mCanvas->setCurrentLayer( vlPolygon.get() );
      std::unique_ptr< QgsMapToolTrimExtendFeature > tool( new QgsMapToolTrimExtendFeature( mCanvas ) );

      vlPolygon->startEditing();
      // Limit
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 0, 0 );
      std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
            mCanvas,
            QEvent::MouseMove,
            QPoint( std::round( pt.x() ), std::round( pt.y() ) )
          ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      // Extend
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1.5 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );


      // vector layer with a trianglev in a rectangle:
      // (0,3) +-------------------+ (3,3)
      //       | (1,2) +---+ (2,2) |
      //       |        \   \      |
      //       |           \  \    |
      //       |               \ \ |
      //       |                  \|
      // (0,0) +-------------------+ (3,0)
      const QgsFeature f = vlPolygon->getFeature( 1 );

      const QString wkt = "Polygon ((1 2, 3 0, 2 2, 1 2))";
      QCOMPARE( f.geometry().asWkt(), wkt );

      vlPolygon->rollBack();
    }


    void testMultiLine()
    {
      /*
       *      |
       *  |   |
       *      |
       * -----|
       *      |
       */
      mCanvas->setCurrentLayer( vlMultiLine.get() );
      std::unique_ptr< QgsMapToolTrimExtendFeature > tool( new QgsMapToolTrimExtendFeature( mCanvas ) );

      vlMultiLine->startEditing();
      // Limit
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 12, 0 );
      std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
            mCanvas,
            QEvent::MouseMove,
            QPoint( std::round( pt.x() ), std::round( pt.y() ) )
          ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      // Extend
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 11, 0.8 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      /*
       *(11 1)|
       *  |   |
       *  |   |
       * -----|
       *(11 0)|
       */
      QgsFeature f = vlMultiLine->getFeature( 1 );

      QString wkt = "MultiLineString ((10 0, 14 0),(11 1, 11 0),(14 -2, 14 2))";
      QCOMPARE( f.geometry().asWkt(), wkt );


      // Limit
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 12, 0 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      // Extend
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 14, 1 );

      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      /*
       *
       *  |
       *  |
       * ------ (14 0)
       *      |
       *        (14 -2)
       */
      f = vlMultiLine->getFeature( 1 );
      wkt = "MultiLineString ((10 0, 14 0),(11 1, 11 0),(14 -2, 14 0))";
      QCOMPARE( f.geometry().asWkt(), wkt );

      vlMultiLine->rollBack();


      vlMultiLine->startEditing();
      // Limit
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 12, 0 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      // Extend
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 14, -1 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      /*
       *      | (14 2)
       *  |   |
       *      |
       * ------ (14 0)
       *(10 0)
       */
      f = vlMultiLine->getFeature( 1 );

      wkt = "MultiLineString ((10 0, 14 0),(11 1, 11 0.5),(14 0, 14 2))";
      QCOMPARE( f.geometry().asWkt(), wkt );
    }

    void testLineZ()
    {

      /*     (3 8 200)
             /
            /  (2 6 10)
           /  \
      (0 5 100)      (3 5 5)
      */
      mCanvas->setCurrentLayer( vlLineZ.get() );
      std::unique_ptr< QgsMapToolTrimExtendFeature > tool( new QgsMapToolTrimExtendFeature( mCanvas ) );

      vlLineZ->startEditing();
      // Limit
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 0, 5 );
      std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
            mCanvas,
            QEvent::MouseMove,
            QPoint( std::round( pt.x() ), std::round( pt.y() ) )
          ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      // Extend
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 5 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );



      /*     (3 8 200)
             /
            /\ (1.5 6.5 150)
           /  \
      (0 5 100)      (3 5 5)
      */
      const QgsFeature f = vlLineZ->getFeature( 1 );

      const QString wkt = "LineStringZ (3 5 5, 1.5 6.5 150)";
      QCOMPARE( f.geometry().asWkt(), wkt );

      vlLineZ->rollBack();
    }

    void testTopologicalPoints()
    {
      const bool topologicalEditing = QgsProject::instance()->topologicalEditing();
      QgsProject::instance()->setTopologicalEditing( true );

      mCanvas->setCurrentLayer( vlTopoEdit.get() );
      std::unique_ptr< QgsMapToolTrimExtendFeature > tool( new QgsMapToolTrimExtendFeature( mCanvas ) );

      vlTopoLimit->startEditing();
      vlTopoEdit->startEditing();
      // Limit
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 30, 15 );
      std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
            mCanvas,
            QEvent::MouseMove,
            QPoint( std::round( pt.x() ), std::round( pt.y() ) )
          ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      // Extend
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 22, 15 );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseMove,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) )
                   ) );
      tool->canvasMoveEvent( event.get() );
      event.reset( new QgsMapMouseEvent(
                     mCanvas,
                     QEvent::MouseButtonRelease,
                     QPoint( std::round( pt.x() ), std::round( pt.y() ) ),
                     Qt::LeftButton
                   ) );
      tool->canvasReleaseEvent( event.get() );

      const QgsFeature fEdit = vlTopoEdit->getFeature( 1 );
      const QgsFeature fLimit = vlTopoLimit->getFeature( 1 );

      const QString wktEdit = "LineString (20 15, 30 15)";
      const QString wktLimit = "LineString (30 0, 30 15, 30 30)";
      QCOMPARE( fEdit.geometry().asWkt(), wktEdit );
      QCOMPARE( fLimit.geometry().asWkt(), wktLimit );

      vlTopoEdit->rollBack();

      QgsProject::instance()->setTopologicalEditing( topologicalEditing );
    }

};

QGSTEST_MAIN( TestQgsMapToolTrimExtendFeature )
#include "testqgsmaptooltrimextendfeature.moc"
