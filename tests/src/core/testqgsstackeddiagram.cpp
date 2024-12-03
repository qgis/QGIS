/***************************************************************************
     testqgsstackeddiagram.cpp
     --------------------------------------
    Date                 : Jun 25 2024
    Copyright            : (C) 2024 by Germán Carrillo
    Email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include "diagram/qgspiediagram.h"
#include "diagram/qgstextdiagram.h"
#include "diagram/qgsstackedbardiagram.h"
#include "diagram/qgshistogramdiagram.h"
#include "diagram/qgsstackeddiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsproject.h"
#include "qgsmarkersymbol.h"

#include <QString>

/**
 * \ingroup UnitTests
 * Unit tests for stacked diagrams
 * (Based on TestQgsDiagram)
 */
class TestQgsStackedDiagram : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsStackedDiagram()
      : QgsTest( QStringLiteral( "Stacked Diagram Tests" ), QStringLiteral( "stackeddiagrams" ) ) {}

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

      const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
      mTestDataDir = myDataDir + '/';

      //
      //create a point layer that will be used in all tests...
      //
      const QString myPointsFileName = mTestDataDir + "stacked_diagrams.gpkg|layername=centroids";
      mPointsLayer = new QgsVectorLayer( myPointsFileName, QStringLiteral( "population" ), QStringLiteral( "ogr" ) );

      //Add points to diagrams, so that it's easier to also verify diagram positioning
      QVariantMap symbolProps { { QStringLiteral( "color" ), QStringLiteral( "0,0,0,0" ) } };
      QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple( symbolProps );
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

    void testStackedHistograms()
    {
      // Histogram 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Left;

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Pixels );
      ds.setStackedDiagramSpacing( 0 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedhistograms", "stackedhistograms", *mMapSettings, 200, 15 );
    }

    void testDisabledSubDiagram()
    {
      // Histogram 1 (disabled)
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Left;
      ds1.enabled = false;

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Pixels );
      ds.setStackedDiagramSpacing( 0 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "disabledsubdiagram", "disabledsubdiagram", *mMapSettings, 200, 15 );
    }

    void testScaledDependentVisibilitySubDiagram()
    {
      // Histogram 1 (disabled)
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Left;
      ds1.scaleBasedVisibility = true;
      ds1.maximumScale = 0;
      ds1.minimumScale = 50000;

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Pixels );
      ds.setStackedDiagramSpacing( 0 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "scaledependentvisibilitysubdiagram", "scaledependentvisibilitysubdiagram", *mMapSettings, 200, 15 );
    }

    void testVerticallyStackedHistograms()
    {
      // Histogram 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Up;

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Down;

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Vertical;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Pixels );
      ds.setStackedDiagramSpacing( 0 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "verticallystackedhistograms", "verticallystackedhistograms", *mMapSettings, 200, 15 );
    }

    void testVerticallyStackedHistogramsWithSpacing()
    {
      // Histogram 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Up;

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Down;

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Vertical;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 8 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "verticallystackedhistogramswithspacing", "verticallystackedhistogramswithspacing", *mMapSettings, 200, 15 );
    }

    void testStackedHistogramsWithSpacing()
    {
      // Histogram 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Left;
      ds1.setSpacing( 8 );
      ds1.setSpacingUnit( Qgis::RenderUnit::Points );

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;
      ds2.setSpacing( 8 );
      ds2.setSpacingUnit( Qgis::RenderUnit::Points );

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 8 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedhistogramswithspacing", "stackedhistogramswithspacing", *mMapSettings, 200, 15 );
    }

    void testStackedHistogramsWithSpacing2()
    {
      // Histogram 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Left;
      ds1.setSpacing( 0 );
      ds1.setSpacingUnit( Qgis::RenderUnit::Points );

      QgsLinearlyInterpolatedDiagramRenderer *dr1 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr1->setDiagram( new QgsHistogramDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->setLowerValue( 0.0 );
      dr1->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr1->setUpperValue( 15000 );
      dr1->setUpperSize( QSizeF( 20, 20 ) );
      //dr1->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;
      ds2.setSpacing( 0 );
      ds2.setSpacingUnit( Qgis::RenderUnit::Points );

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      //dr2->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 8 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedhistogramswithspacing2", "stackedhistogramswithspacing2", *mMapSettings, 200, 15 );
    }

    void testMenHistogram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::black;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 3, 3 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Left;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 15000 );
      dr->setUpperSize( QSizeF( 20, 20 ) );
      dr->setClassificationField( QStringLiteral( "max( \"maennlich_6_17\", \"maennlich_18_64\", \"maennlich_ab_65\",  \"maennlich_unter_6\" )" ) ); //#spellok
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedmenhistogram", "stackedmenhistogram", *mMapSettings, 200, 15 );
    }

    void testWomenHistogram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::black;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 3, 3 );
      ds.rotationOffset = 0;
      ds.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 15000 );
      dr->setUpperSize( QSizeF( 20, 20 ) );
      dr->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) ); //#spellok
      dr->setDiagram( new QgsHistogramDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedwomenhistogram", "stackedwomenhistogram", *mMapSettings, 200, 15 );
    }

    void testStackedPiesVertical()
    {
      // Pie 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.size = QSizeF( 10, 10 );
      ds1.rotationOffset = 270;
      ds1.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr1 = new QgsSingleCategoryDiagramRenderer();
      dr1->setDiagram( new QgsPieDiagram() );
      dr1->setDiagramSettings( ds1 );

      // Pie 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.size = QSizeF( 10, 10 );
      ds2.rotationOffset = 270;
      ds2.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr2 = new QgsSingleCategoryDiagramRenderer();
      dr2->setDiagram( new QgsPieDiagram() );
      dr2->setDiagramSettings( ds2 );

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Vertical;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedpies", "stackedpies", *mMapSettings, 200, 15 );
    }

    void testStackedPiesVerticalWithSpacing()
    {
      // Pie 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.size = QSizeF( 10, 10 );
      ds1.rotationOffset = 270;
      ds1.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr1 = new QgsSingleCategoryDiagramRenderer();
      dr1->setDiagram( new QgsPieDiagram() );
      dr1->setDiagramSettings( ds1 );

      // Pie 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.size = QSizeF( 10, 10 );
      ds2.rotationOffset = 270;
      ds2.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr2 = new QgsSingleCategoryDiagramRenderer();
      dr2->setDiagram( new QgsPieDiagram() );
      dr2->setDiagramSettings( ds2 );

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Vertical;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 8 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedpiesverticalwithspacing", "stackedpiesverticalwithspacing", *mMapSettings, 200, 15 );
    }

    void testStackedPiesHorizontalWithSpacing()
    {
      // Pie 1
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.size = QSizeF( 10, 10 );
      ds1.rotationOffset = 270;
      ds1.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr1 = new QgsSingleCategoryDiagramRenderer();
      dr1->setDiagram( new QgsPieDiagram() );
      dr1->setDiagramSettings( ds1 );

      // Pie 2
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.size = QSizeF( 10, 10 );
      ds2.rotationOffset = 270;
      ds2.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr2 = new QgsSingleCategoryDiagramRenderer();
      dr2->setDiagram( new QgsPieDiagram() );
      dr2->setDiagramSettings( ds2 );

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 8 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedpieshorizontalwithspacing", "stackedpieshorizontalwithspacing", *mMapSettings, 200, 15 );
    }

    void testMenPie()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::black;
      ds.penWidth = .5;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 10, 10 );
      ds.rotationOffset = 270;
      ds.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedmenpie", "stackedmenpie", *mMapSettings, 200, 15 );
    }

    void testWomenPie()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds.minimumScale = -1;
      ds.maximumScale = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::black;
      ds.penWidth = .5;
      ds.sizeType = Qgis::RenderUnit::Millimeters;
      ds.size = QSizeF( 10, 10 );
      ds.rotationOffset = 270;
      ds.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr = new QgsSingleCategoryDiagramRenderer();
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedwomenpie", "stackedwomenpie", *mMapSettings, 200, 15 );
    }

    void testStackedPieHistogram()
    {
      // Pie
      QgsDiagramSettings ds1;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds1.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.size = QSizeF( 10, 10 );
      ds1.rotationOffset = 270;
      ds1.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr1 = new QgsSingleCategoryDiagramRenderer();
      dr1->setDiagram( new QgsPieDiagram() );
      dr1->setDiagramSettings( ds1 );

      // Histogram
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr2 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr2->setLowerValue( 0.0 );
      dr2->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr2->setUpperValue( 15000 );
      dr2->setUpperSize( QSizeF( 20, 20 ) );
      dr2->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) ); //#spellok
      dr2->setDiagram( new QgsHistogramDiagram() );
      dr2->setDiagramSettings( ds2 );

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 8 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedpiehistogram", "stackedpiehistogram", *mMapSettings, 200, 15 );
    }

    void testStackedDiagramsNested()
    {
      // Nested stacked histograms (just because we can :))
      // 1 vertically stacked diagram:
      // Above:
      //  + Horizontally stacked diagram
      //    + 2 histograms
      // Below:
      //  + 1 pie

      // Histogram 1
      QgsDiagramSettings ds11;
      QColor col1 = Qt::blue;
      QColor col2 = Qt::red;
      QColor col3 = Qt::yellow;
      QColor col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds11.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds11.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" ); //#spellok
      ds11.minimumScale = -1;
      ds11.maximumScale = -1;
      ds11.minimumSize = 0;
      ds11.penColor = Qt::black;
      ds11.penWidth = .5;
      ds11.scaleByArea = true;
      ds11.sizeType = Qgis::RenderUnit::Millimeters;
      ds11.barWidth = 3;
      ds11.rotationOffset = 0;
      ds11.diagramOrientation = QgsDiagramSettings::Left;

      QgsLinearlyInterpolatedDiagramRenderer *dr11 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr11->setDiagram( new QgsHistogramDiagram() );
      dr11->setDiagramSettings( ds11 );
      dr11->setLowerValue( 0.0 );
      dr11->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr11->setUpperValue( 15000 );
      dr11->setUpperSize( QSizeF( 20, 20 ) );
      //dr11->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"maennlich_6_17\", \"maennlich_unter_6\")" ) );  //#spellok

      // Histogram 2
      QgsDiagramSettings ds12;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds12.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds12.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" ); //#spellok
      ds12.minimumScale = -1;
      ds12.maximumScale = -1;
      ds12.minimumSize = 0;
      ds12.penColor = Qt::black;
      ds12.penWidth = .5;
      ds12.scaleByArea = true;
      ds12.barWidth = 3;
      ds12.sizeType = Qgis::RenderUnit::Millimeters;
      ds12.rotationOffset = 0;
      ds12.diagramOrientation = QgsDiagramSettings::Right;

      QgsLinearlyInterpolatedDiagramRenderer *dr12 = new QgsLinearlyInterpolatedDiagramRenderer();
      dr12->setDiagram( new QgsHistogramDiagram() );
      dr12->setDiagramSettings( ds12 );
      dr12->setLowerValue( 0.0 );
      dr12->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr12->setUpperValue( 15000 );
      dr12->setUpperSize( QSizeF( 20, 20 ) );
      //dr12->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok

      QgsDiagramSettings ds1;
      ds1.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds1.categoryAttributes = ds11.categoryAttributes + ds12.categoryAttributes;
      ds1.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Pixels );
      ds1.setStackedDiagramSpacing( 0 );

      QgsStackedDiagramRenderer *dr1 = new QgsStackedDiagramRenderer();
      dr1->setDiagram( new QgsStackedDiagram() );
      dr1->setDiagramSettings( ds1 );
      dr1->addRenderer( dr11 );
      dr1->addRenderer( dr12 );

      // Pie
      QgsDiagramSettings ds2;
      col1 = Qt::blue;
      col2 = Qt::red;
      col3 = Qt::yellow;
      col4 = Qt::green;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      col3.setAlphaF( 0.5 );
      col4.setAlphaF( 0.5 );
      ds2.categoryColors = QList<QColor>() << col1 << col2 << col3 << col4;
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"gesamt_ab_65\"" ) << QStringLiteral( "\"gesamt_18_64\"" ) << QStringLiteral( "\"gesamt_6_17\"" ) << QStringLiteral( "\"gesamt_unter_6\"" ); //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.size = QSizeF( 10, 10 );
      ds2.rotationOffset = 270;
      ds2.setDirection( QgsDiagramSettings::Counterclockwise );

      QgsSingleCategoryDiagramRenderer *dr2 = new QgsSingleCategoryDiagramRenderer();
      dr2->setDiagram( new QgsPieDiagram() );
      dr2->setDiagramSettings( ds2 );

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Vertical;
      ds.categoryAttributes = ds11.categoryAttributes + ds12.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Points );
      ds.setStackedDiagramSpacing( 4 );

      QgsStackedDiagramRenderer *dr = new QgsStackedDiagramRenderer();
      dr->setDiagram( new QgsStackedDiagram() );
      dr->setDiagramSettings( ds );
      dr->addRenderer( dr1 );
      dr->addRenderer( dr2 );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      const QgsRectangle extent( 9.7, 53.5, 9.95, 53.6 );
      mMapSettings->setExtent( extent );
      mMapSettings->setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );
      mMapSettings->setOutputDpi( 96 );
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackeddiagramsnested", "stackeddiagramsnested", *mMapSettings, 200, 15 );
    }
};


QGSTEST_MAIN( TestQgsStackedDiagram )
#include "testqgsstackeddiagram.moc"
