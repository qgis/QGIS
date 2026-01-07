/***************************************************************************
     testqgsdiagram.cpp
     --------------------------------------
    Date                 : Sep 7 2012
    Copyright            : (C) 2012 by Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QPainter>
#include <QString>
#include <QStringList>

//qgis includes...
// #include <qgisapp.h>
#include "diagram/qgspiediagram.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgsstackedbardiagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
//qgis test includes
#include "qgsproject.h"
#include "qgsshadoweffect.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

/**
 * \ingroup UnitTests
 * Unit tests for the diagram renderer
 */
class TestQgsDiagram : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsDiagram()
      : QgsTest( u"Diagram Tests"_s, u"diagrams"_s ) {}

  private:
    bool mTestHasError = false;
    QgsMapSettings *mMapSettings = nullptr;
    QgsVectorLayer *mPointsLayer = nullptr;
    QString mTestDataDir;

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase()
    {
      mTestHasError = false;
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsApplication::showSettings();

      mMapSettings = new QgsMapSettings();

      //create some objects that will be used in all tests...

      //
      //create a non spatial layer that will be used in all tests...
      //
      const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
      mTestDataDir = myDataDir + '/';

      //
      //create a point layer that will be used in all tests...
      //
      const QString myPointsFileName = mTestDataDir + "points.shp";
      const QFileInfo myPointFileInfo( myPointsFileName );
      mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(), myPointFileInfo.completeBaseName(), u"ogr"_s );

      //we don't want to render the points themselves, just the diagrams
      QVariantMap symbolProps;
      symbolProps.insert( u"color"_s, u"0,0,0,0"_s );
      symbolProps.insert( u"outline_style"_s, u"no"_s );
      QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple( symbolProps ).release();
      QgsSingleSymbolRenderer *symbolRenderer = new QgsSingleSymbolRenderer( symbol );
      mPointsLayer->setRenderer( symbolRenderer );

      // Create map composition to draw on
      QgsProject::instance()->addMapLayer( mPointsLayer );
      mMapSettings->setLayers( QList<QgsMapLayer *>() << mPointsLayer );
    }

    // will be called after the last testfunction was executed.
    void cleanupTestCase()
    {
      delete mMapSettings;
      QgsProject::instance()->removeAllMapLayers();

      QgsApplication::exitQgis();
    }

    // will be called before each testfunction is executed
    void init()
    {
      mPointsLayer->setDiagramRenderer( nullptr );
      const QgsDiagramLayerSettings dls;
      mPointsLayer->setDiagramLayerSettings( dls );
    }

    // will be called after every testfunction.
    void cleanup()
    {
    }

    void testPieDiagram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( -126, 23, -70, 47 );
      mMapSettings->setExtent( extent );
      mMapSettings->setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram", "piediagram", *mMapSettings, 200, 15 );
    }

    void testPieDiagramOpacity()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 1 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( -126, 23, -70, 47 );
      mMapSettings->setExtent( extent );
      mMapSettings->setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_opacity", "piediagram_opacity", *mMapSettings, 200, 15 );
    }

    void testPieDiagramAggregate()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\"/sum(\"Pilots\")"_s << u"\"Cabin Crew\"/sum(\"Cabin Crew\")"_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( -126, 23, -70, 47 );
      mMapSettings->setExtent( extent );
      mMapSettings->setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_aggregate", "piediagram_aggregate", *mMapSettings, 200, 15 );
    }

    void testDiagramWithGeometryBasedExpressionAttribute()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"abs($x)"_s << u"$y"_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( -126, 23, -70, 47 );
      mMapSettings->setExtent( extent );
      mMapSettings->setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_geometry_based_expression", "piediagram_geometry_based_expression", *mMapSettings, 200, 15 );
    }

    void testPaintEffect()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setPaintEffect( new QgsDropShadowEffect() );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( -126, 23, -70, 47 );
      mMapSettings->setExtent( extent );
      mMapSettings->setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "diagram_effects", "diagram_effects", *mMapSettings, 200, 15 );
    }

    void testHistogram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram", "histogram", *mMapSettings, 200, 15 );
    }

    void testHistogramSpacing()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setSpacing( 17 );
      ds.setSpacingUnit( Qgis::RenderUnit::Points );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_spacing", "histogram_spacing", *mMapSettings, 200, 15 );
    }

    void testHistogramAxis()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setShowAxis( true );

      QVariantMap props;
      props.insert( u"width"_s, u"2"_s );
      props.insert( u"color"_s, u"#ff00ff"_s );
      ds.setAxisLineSymbol( QgsLineSymbol::createSimple( props ).release() );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_axis_top", "histogram_axis_top", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_axis_bottom", "histogram_axis_bottom", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_axis_left", "histogram_axis_left", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_axis_right", "histogram_axis_right", *mMapSettings, 200, 15 );
    }

    void testHistogramOrientation()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_right", "histogram_right", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_left", "histogram_left", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "histogram_down", "histogram_down", *mMapSettings, 200, 15 );
    }

    void testStackedFixSize()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Up;
      ds.setSpacing( 3 );

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_up", "stacked_up", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_right", "stacked_right", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_left", "stacked_left", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_down", "stacked_down", *mMapSettings, 200, 15 );
    }

    void testStackedVaryingFixSize()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Up;
      ds.setSpacing( 3 );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_varying_up", "stacked_varying_up", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_varying_right", "stacked_varying_right", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_varying_left", "stacked_varying_left", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_varying_down", "stacked_varying_down", *mMapSettings, 200, 15 );
    }

    void testStackedAxis()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setSpacing( 3 );
      ds.setShowAxis( true );

      QVariantMap props;
      props.insert( u"width"_s, u"2"_s );
      props.insert( u"color"_s, u"#ff00ff"_s );
      ds.setAxisLineSymbol( QgsLineSymbol::createSimple( props ).release() );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_axis_up", "stacked_axis_up", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_axis_down", "stacked_axis_down", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_axis_left", "stacked_axis_left", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_axis_right", "stacked_axis_right", *mMapSettings, 200, 15 );
    }

    void testStackedNegative()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"-\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setSpacing( 3 );
      ds.setShowAxis( true );

      QVariantMap props;
      props.insert( u"width"_s, u"2"_s );
      props.insert( u"color"_s, u"#ff00ff"_s );
      ds.setAxisLineSymbol( QgsLineSymbol::createSimple( props ).release() );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( u"Staff"_s );
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_negative_up", "stacked_negative_up", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_negative_down", "stacked_negative_down", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_negative_left", "stacked_negative_left", *mMapSettings, 200, 15 );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stacked_negative_right", "stacked_negative_right", *mMapSettings, 200, 15 );
    }

    void testPieDiagramExpression()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"ln(Pilots + 1)"_s << u"ln(\"Cabin Crew\" + 1)"_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationAttributeIsExpression( true );
      dr->setClassificationAttributeExpression( u"ln(Staff + 1)"_s );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      // dls.setRenderer( dr );

      mPointsLayer->setDiagramRenderer( dr );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_expression", "piediagram_expression", *mMapSettings, 200, 15 );

      mPointsLayer->setDiagramRenderer( nullptr );
    }

    void testPieDiagramDirection()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"ln(Pilots + 1)"_s << u"ln(\"Cabin Crew\" + 1)"_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 90;
      ds.setDirection( QgsDiagramSettings::Clockwise );

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      // dls.setRenderer( dr );

      mPointsLayer->setDiagramRenderer( dr );
      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_clockwise", "piediagram_clockwise", *mMapSettings, 200, 15 );

      mPointsLayer->setDiagramRenderer( nullptr );
    }

    void testDataDefinedPosition()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );

      //Set data defined position
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::PositionX, QgsProperty::fromExpression( u"$x + -5"_s, true ) );
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::PositionY, QgsProperty::fromExpression( u"$y + 5"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_position", "piediagram_datadefined_position", *mMapSettings, 200, 15 );
    }

    void testDataDefinedStroke()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );

      //setup data defined stroke
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::StrokeColor, QgsProperty::fromExpression( u"if(\"Pilots\">1,'0,0,0,255','255,0,0,255')"_s, true ) );
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::StrokeWidth, QgsProperty::fromExpression( u"\"Staff\" / 2.0"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_outline", "piediagram_datadefined_outline", *mMapSettings, 200, 15 );
    }

    void testDataDefinedStartAngle()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );

      //setup data defined start angle
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::StartAngle, QgsProperty::fromExpression( u"360.0-\"Importance\"/20.0 * 360.0"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_startangle", "piediagram_datadefined_startangle", *mMapSettings, 200, 15 );
    }

    void testDataDefinedDistance()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::AroundPoint );
      dls.setShowAllDiagrams( true );

      //setup data defined distance
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::Distance, QgsProperty::fromExpression( u"\"Staff\"*2"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_distance", "piediagram_datadefined_distance", *mMapSettings, 200, 15 );
    }

    void testDataDefinedShow()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );

      //setup data defined show
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::Show, QgsProperty::fromExpression( u"\"Pilots\"=1"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_show", "piediagram_datadefined_show", *mMapSettings, 200, 15 );
    }

    void testDataDefinedPriority()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 50, 50 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( false );

      //setup data defined priority
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::Priority, QgsProperty::fromExpression( u"\"importance\"/2"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_priority", "piediagram_datadefined_priority", *mMapSettings, 200, 15 );
    }

    void testDataDefinedZIndex()
    {
      QgsDiagramSettings ds;
      const QColor col1 = Qt::red;
      const QColor col2 = Qt::yellow;
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 50, 50 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );

      //setup data defined z index
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::ZIndex, QgsProperty::fromExpression( u"\"importance\"/2"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_zindex", "piediagram_datadefined_zindex", *mMapSettings, 200, 15 );
    }

    void testDataDefinedAlwaysShow()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 50, 50 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( false );

      //setup data defined priority (required to only show certain diagrams)
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::Priority, QgsProperty::fromExpression( u"\"importance\"/2"_s, true ) );
      //setup data defined "always show"
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::AlwaysShow, QgsProperty::fromExpression( u"\"Staff\">=6"_s, true ) );


      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "piediagram_datadefined_alwaysshow", "piediagram_datadefined_alwaysshow", *mMapSettings, 200, 15 );
    }


    void testDataDefinedBackgroundColor()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      //hide the text so we are just testing the background color
      col1.setAlphaF( 0 );
      col2.setAlphaF( 0 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << u"\"Pilots\""_s << u"\"Cabin Crew\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 15, 15 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsTextDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );

      //setup data defined stroke
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Property::BackgroundColor, QgsProperty::fromExpression( u"if(\"Pilots\">1,'0,0,255,150','255,0,0,150')"_s, true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QGSVERIFYRENDERMAPSETTINGSCHECK( "textdiagram_datadefined_background", "textdiagram_datadefined_background", *mMapSettings, 200, 15 );
    }

    void testClipping()
    {
      const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
      auto vl2 = std::make_unique<QgsVectorLayer>( filename, u"lines"_s, u"ogr"_s );

      QVariantMap props;
      props.insert( u"outline_color"_s, u"#487bb6"_s );
      props.insert( u"outline_width"_s, u"1"_s );
      std::unique_ptr<QgsLineSymbol> symbol( QgsLineSymbol::createSimple( props ) );
      vl2->setRenderer( new QgsSingleSymbolRenderer( symbol.release() ) );

      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1;
      ds.categoryAttributes = QList<QString>() << u"\"Value\""_s;
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      vl2->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::Line );
      dls.setShowAllDiagrams( true );
      vl2->setDiagramLayerSettings( dls );

      mMapSettings->setLayers( QList<QgsMapLayer *>() << vl2.get() );

      QgsMapClippingRegion region1( QgsGeometry::fromWkt( "Polygon ((-92 45, -99 36, -94 29, -82 29, -81 45, -92 45))" ) );
      region1.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipToIntersection );
      mMapSettings->addClippingRegion( region1 );

      QgsMapClippingRegion region2( QgsGeometry::fromWkt( "Polygon ((-85 36, -85 46, -107 47, -108 28, -85 28, -85 36))" ) );
      region2.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly );
      mMapSettings->addClippingRegion( region2 );

      const bool res = QGSRENDERMAPSETTINGSCHECK( "diagram_clipping", "diagram_clipping", *mMapSettings, 200, 15 );
      mMapSettings->setClippingRegions( QList<QgsMapClippingRegion>() );
      mMapSettings->setLayers( QList<QgsMapLayer *>() << mPointsLayer );

      QVERIFY( res );
    }
};


QGSTEST_MAIN( TestQgsDiagram )
#include "testqgsdiagram.moc"
