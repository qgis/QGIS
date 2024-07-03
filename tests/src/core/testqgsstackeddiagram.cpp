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
#include "diagram/qgsstackeddiagram.h"
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
 * Unit tests for stacked diagrams
 * (Based on TestQgsDiagram)
 */
class TestQgsStackedDiagram : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsStackedDiagram() : QgsTest( QStringLiteral( "Stacked Diagram Tests" ), QStringLiteral( "stackeddiagrams" ) ) {}

  private:
    bool mTestHasError =  false ;
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
      mPointsLayer = new QgsVectorLayer( myPointsFileName,
                                         QStringLiteral( "population" ), QStringLiteral( "ogr" ) );

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

    void testStackedHistogram()
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
      ds1.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" );  //#spellok
      ds1.minimumScale = -1;
      ds1.maximumScale = -1;
      ds1.minimumSize = 0;
      ds1.penColor = Qt::black;
      ds1.penWidth = .5;
      ds1.scaleByArea = true;
      ds1.sizeType = Qgis::RenderUnit::Millimeters;
      ds1.rotationOffset = 0;
      ds1.diagramOrientation = QgsDiagramSettings::Left;

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
      ds2.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" );  //#spellok
      ds2.minimumScale = -1;
      ds2.maximumScale = -1;
      ds2.minimumSize = 0;
      ds2.penColor = Qt::black;
      ds2.penWidth = .5;
      ds2.scaleByArea = true;
      ds2.sizeType = Qgis::RenderUnit::Millimeters;
      ds2.rotationOffset = 0;
      ds2.diagramOrientation = QgsDiagramSettings::Right;

      QgsDiagramSettings ds;
      ds.stackedDiagramMode = QgsDiagramSettings::Horizontal;
      ds.categoryAttributes = ds1.categoryAttributes + ds2.categoryAttributes;
      ds.setStackedDiagramSpacingUnit( Qgis::RenderUnit::Pixels );
      ds.setStackedDiagramSpacing( 0 );

      QgsStackedDiagram *stackedDiagram = new QgsStackedDiagram();
      stackedDiagram->addSubDiagram( new QgsHistogramDiagram(), &ds1 );
      stackedDiagram->addSubDiagram( new QgsHistogramDiagram(), &ds2 );

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 15000 );
      dr->setUpperSize( QSizeF( 20, 20 ) );
      dr->setClassificationField( QStringLiteral( "max(\"maennlich_18_64\", \"maennlich_ab_65\", \"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok
      dr->setDiagram( stackedDiagram );
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
      QGSVERIFYRENDERMAPSETTINGSCHECK( "stackedhistograms", "stackedhistograms", *mMapSettings, 200, 15 );
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
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"maennlich_ab_65\"" ) << QStringLiteral( "\"maennlich_18_64\"" ) << QStringLiteral( "\"maennlich_6_17\"" ) << QStringLiteral( "\"maennlich_unter_6\"" );  //#spellok
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
      dr->setClassificationField( QStringLiteral( "max( \"maennlich_6_17\", \"maennlich_18_64\", \"maennlich_ab_65\",  \"maennlich_unter_6\" )" ) );  //#spellok
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
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"weiblich_ab_65\"" ) << QStringLiteral( "\"weiblich_18_64\"" ) << QStringLiteral( "\"weiblich_6_17\"" ) << QStringLiteral( "\"weiblich_unter_6\"" );  //#spellok
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
      dr->setClassificationField( QStringLiteral( "max(\"weiblich_unter_6\", \"weiblich_6_17\", \"weiblich_18_64\", \"weiblich_ab_65\")" ) );  //#spellok
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

};


QGSTEST_MAIN( TestQgsStackedDiagram )
#include "testqgsstackeddiagram.moc"
