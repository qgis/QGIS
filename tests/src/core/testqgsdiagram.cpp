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
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QPainter>

//qgis includes...
// #include <qgisapp.h>
#include "diagram/qgspiediagram.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgsstackedbardiagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsmaplayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
//qgis test includes
#include "qgsmultirenderchecker.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsshadoweffect.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

/**
 * \ingroup UnitTests
 * Unit tests for the diagram renderer
 */
class TestQgsDiagram : public QObject
{
    Q_OBJECT

  public:
    TestQgsDiagram() = default;

  private:
    bool mTestHasError =  false ;
    QgsMapSettings *mMapSettings = nullptr;
    QgsVectorLayer *mPointsLayer = nullptr;
    QString mTestDataDir;
    QString mReport;

    bool imageCheck( const QString &testType );

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
      mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                         myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

      //we don't want to render the points themselves, just the diagrams
      QVariantMap symbolProps;
      symbolProps.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,0" ) );
      symbolProps.insert( QStringLiteral( "outline_style" ), QStringLiteral( "no" ) );
      QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple( symbolProps );
      QgsSingleSymbolRenderer *symbolRenderer = new QgsSingleSymbolRenderer( symbol );
      mPointsLayer->setRenderer( symbolRenderer );

      // Create map composition to draw on
      QgsProject::instance()->addMapLayer( mPointsLayer );
      mMapSettings->setLayers( QList<QgsMapLayer *>() << mPointsLayer );

      mReport += QLatin1String( "<h1>Diagram Tests</h1>\n" );
    }

    // will be called after the last testfunction was executed.
    void cleanupTestCase()
    {
      delete mMapSettings;
      delete mPointsLayer;

      const QString myReportFile = QDir::tempPath() + "/qgistest.html";
      QFile myFile( myReportFile );
      if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
      {
        QTextStream myQTextStream( &myFile );
        myQTextStream << mReport;
        myFile.close();
        //QDesktopServices::openUrl( "file:///" + myReportFile );
      }
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
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram" ) );
    }

    void testPaintEffect()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setPaintEffect( new QgsDropShadowEffect() );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "diagram_effects" ) );
    }

    void testHistogram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "histogram" ) );
    }

    void testHistogramSpacing()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setSpacing( 17 );
      ds.setSpacingUnit( QgsUnitTypes::RenderPoints );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "histogram_spacing" ) );
    }

    void testHistogramAxis()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setShowAxis( true );

      QVariantMap props;
      props.insert( QStringLiteral( "width" ), QStringLiteral( "2" ) );
      props.insert( QStringLiteral( "color" ), QStringLiteral( "#ff00ff" ) );
      ds.setAxisLineSymbol( QgsLineSymbol::createSimple( props ) );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "histogram_axis_top" ) );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "histogram_axis_bottom" ) );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "histogram_axis_left" ) );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "histogram_axis_right" ) );
    }

    void testHistogramOrientation()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "histogram_right" ) );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "histogram_left" ) );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "histogram_down" ) );

    }

    void testStackedFixSize()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      QVERIFY( imageCheck( "stacked_up" ) );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_right" ) );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_left" ) );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_down" ) );

    }

    void testStackedVaryingFixSize()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Up;
      ds.setSpacing( 3 );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );
      QVERIFY( imageCheck( "stacked_varying_up" ) );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_varying_right" ) );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_varying_left" ) );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_varying_down" ) );

    }

    void testStackedAxis()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setSpacing( 3 );
      ds.setShowAxis( true );

      QVariantMap props;
      props.insert( QStringLiteral( "width" ), QStringLiteral( "2" ) );
      props.insert( QStringLiteral( "color" ), QStringLiteral( "#ff00ff" ) );
      ds.setAxisLineSymbol( QgsLineSymbol::createSimple( props ) );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "stacked_axis_up" ) );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_axis_down" ) );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_axis_left" ) );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_axis_right" ) );
    }

    void testStackedNegative()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "-\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;
      ds.setSpacing( 3 );
      ds.setShowAxis( true );

      QVariantMap props;
      props.insert( QStringLiteral( "width" ), QStringLiteral( "2" ) );
      props.insert( QStringLiteral( "color" ), QStringLiteral( "#ff00ff" ) );
      ds.setAxisLineSymbol( QgsLineSymbol::createSimple( props ) );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( QStringLiteral( "Staff" ) );
      dr->setDiagram( new QgsStackedBarDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "stacked_negative_up" ) );

      ds.diagramOrientation = QgsDiagramSettings::Down;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_negative_down" ) );

      ds.diagramOrientation = QgsDiagramSettings::Left;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_negative_left" ) );

      ds.diagramOrientation = QgsDiagramSettings::Right;
      dr->setDiagramSettings( ds );
      QVERIFY( imageCheck( "stacked_negative_right" ) );
    }

    void testPieDiagramExpression()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "ln(Pilots + 1)" ) << QStringLiteral( "ln(\"Cabin Crew\" + 1)" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.rotationOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationAttributeIsExpression( true );
      dr->setClassificationAttributeExpression( QStringLiteral( "ln(Staff + 1)" ) );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      // dls.setRenderer( dr );

      mPointsLayer->setDiagramRenderer( dr );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_expression" ) );

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
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "ln(Pilots + 1)" ) << QStringLiteral( "ln(\"Cabin Crew\" + 1)" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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

      QVERIFY( imageCheck( "piediagram_clockwise" ) );

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
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "$x + -5" ), true ) );
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "$y + 5" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_position" ) );
    }

    void testDataDefinedStroke()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(\"Pilots\">1,'0,0,0,255','255,0,0,255')" ), true ) );
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::StrokeWidth, QgsProperty::fromExpression( QStringLiteral( "\"Staff\" / 2.0" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_outline" ) );
    }

    void testDataDefinedStartAngle()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::StartAngle, QgsProperty::fromExpression( QStringLiteral( "360.0-\"Importance\"/20.0 * 360.0" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_startangle" ) );
    }

    void testDataDefinedDistance()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Distance, QgsProperty::fromExpression( QStringLiteral( "\"Staff\"*2" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_distance" ) );
    }

    void testDataDefinedShow()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Show, QgsProperty::fromExpression( QStringLiteral( "\"Pilots\"=1" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_show" ) );
    }

    void testDataDefinedPriority()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Priority, QgsProperty::fromExpression( QStringLiteral( "\"importance\"/2" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_priority" ) );
    }

    void testDataDefinedZIndex()
    {
      QgsDiagramSettings ds;
      const QColor col1 = Qt::red;
      const QColor col2 = Qt::yellow;
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::ZIndex, QgsProperty::fromExpression( QStringLiteral( "\"importance\"/2" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_zindex" ) );
    }

    void testDataDefinedAlwaysShow()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::Priority, QgsProperty::fromExpression( QStringLiteral( "\"importance\"/2" ), true ) );
      //setup data defined "always show"
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::AlwaysShow, QgsProperty::fromExpression( QStringLiteral( "\"Staff\">=6" ), true ) );


      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_datadefined_alwaysshow" ) );
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
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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
      dls.dataDefinedProperties().setProperty( QgsDiagramLayerSettings::BackgroundColor, QgsProperty::fromExpression( QStringLiteral( "if(\"Pilots\">1,'0,0,255,150','255,0,0,150')" ), true ) );

      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "textdiagram_datadefined_background" ) );
    }

    void testClipping()
    {
      const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
      std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( filename, QStringLiteral( "lines" ), QStringLiteral( "ogr" ) ) );

      QVariantMap props;
      props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#487bb6" ) );
      props.insert( QStringLiteral( "outline_width" ), QStringLiteral( "1" ) );
      std::unique_ptr< QgsLineSymbol > symbol( QgsLineSymbol::createSimple( props ) );
      vl2->setRenderer( new QgsSingleSymbolRenderer( symbol.release() ) );

      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Value\"" );
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
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

      const bool res = imageCheck( QStringLiteral( "diagram_clipping" ) );
      mMapSettings->setClippingRegions( QList< QgsMapClippingRegion >() );
      mMapSettings->setLayers( QList<QgsMapLayer *>() << mPointsLayer );

      QVERIFY( res );
    }



};

bool TestQgsDiagram::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image

  const QgsRectangle extent( -126, 23, -70, 47 );
  mMapSettings->setExtent( extent );
  mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
  mMapSettings->setOutputDpi( 96 );
  QgsMultiRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "diagrams" ) );
  checker.setControlName( "expected_" + testType );
  checker.setMapSettings( *mMapSettings );
  checker.setColorTolerance( 15 );
  const bool resultFlag = checker.runTest( testType, 200 );
  mReport += checker.report();
  return resultFlag;
}

QGSTEST_MAIN( TestQgsDiagram )
#include "testqgsdiagram.moc"
