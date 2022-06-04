/***************************************************************************
     testqgscallout.cpp
     --------------------------------------
    Date                 : July 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
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
#include <QDir>
#include <QPainter>
#include <QPicture>

#include "qgscallout.h"
#include "qgscalloutsregistry.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgssymbollayerutils.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgslayout.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutpagecollection.h"
#include "qgspallabeling.h"
#include "qgsfontutils.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgsnullsymbolrenderer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

//qgis test includes
#include "qgsmultirenderchecker.h"

//dummy callout for testing
class DummyCallout : public QgsCallout
{
  public:
    DummyCallout( const QString &prop1, const QString &prop2 )
      : mProp1( prop1 )
      , mProp2( prop2 )
    {}
    QString type() const override { return QStringLiteral( "Dummy" ); }
    QgsCallout *clone() const override { return new DummyCallout( mProp1, mProp2 ); }
    static QgsCallout *create( const QVariantMap &props, const QgsReadWriteContext & ) { return new DummyCallout( props[QStringLiteral( "testProp" )].toString(), props[QStringLiteral( "testProp2" )].toString() ); }
    QVariantMap properties( const QgsReadWriteContext & ) const override
    {
      QVariantMap props;
      props[QStringLiteral( "testProp" )] = mProp1;
      props[QStringLiteral( "testProp2" )] = mProp2;
      return props;
    }
    void readProperties( const QVariantMap &props, const QgsReadWriteContext & ) override
    {
      mProp1 = props[QStringLiteral( "testProp" )].toString();
      mProp2 = props[QStringLiteral( "testProp2" )].toString();
    }

    QString prop1() { return mProp1; }
    QString prop2() { return mProp2; }

  protected:

    void draw( QgsRenderContext &, const QRectF &, const double, const QgsGeometry &, QgsCallout::QgsCalloutContext & ) override { }

  private:
    QString mProp1;
    QString mProp2;
};


class TestSimpleCalloutUnder : public QgsSimpleLineCallout
{
  public:

    QString type() const override { return QStringLiteral( "SimpleUnder" ); }
    TestSimpleCalloutUnder *clone() const override { return new TestSimpleCalloutUnder( *this ); }
    QVariantMap properties( const QgsReadWriteContext & ) const override
    {
      QVariantMap props;
      return props;
    }
    void readProperties( const QVariantMap &, const QgsReadWriteContext & ) override
    {
    }
    QgsCallout::DrawOrder drawOrder() const override
    {
      return QgsCallout::OrderBelowIndividualLabels;
    }
};


class TestQgsCallout: public QObject
{
    Q_OBJECT

  public:
    TestQgsCallout();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void saveRestore();

    void calloutsInLabeling();
    void calloutsWithRotation();
    void calloutsInLayout();
    void calloutsDisabled();
    void calloutsDataDefinedDisabled();
    void calloutDataDefinedSymbol();
    void calloutDataDefinedSymbolColor();
    void calloutMinimumDistance();
    void calloutDataDefinedMinimumDistance();
    void calloutOffsetFromAnchor();
    void calloutDataDefinedOffsetFromAnchor();
    void calloutOffsetFromLabel();
    void calloutDataDefinedOffsetFromLabel();
    void calloutLabelAnchorTopRight();
    void calloutLabelAnchorTopLeft();
    void calloutLabelAnchorTop();
    void calloutLabelAnchorBottomLeft();
    void calloutLabelAnchorBottom();
    void calloutLabelAnchorBottomRight();
    void calloutLabelAnchorLeft();
    void calloutLabelAnchorRight();
    void calloutLabelAnchorCentroid();
    void calloutLabelDataDefinedAnchor();
    void calloutBehindLabel();
    void calloutBehindIndividualLabels();
    void calloutNoDrawToAllParts();
    void calloutDrawToAllParts();
    void calloutDataDefinedDrawToAllParts();
    void calloutPointOnExterior();
    void calloutDataDefinedAnchorPoint();
    void calloutDataDefinedDestination();
    void calloutDataDefinedOrigin();
    void manhattan();
    void manhattanRotated();
    void manhattanNoDrawToAllParts();
    void manhattanDrawToAllParts();
    void manhattanDataDefinedDrawToAllParts();
    void manhattanDataDefinedDestination();
    void manhattanDataDefinedOrigin();
    void curvedAutoLeavingLabelsAtBottomLeft();
    void curvedAutoLeavingLabelsAtBottomRight();
    void curvedAutoLeavingLabelsAtTopLeft();
    void curvedAutoLeavingLabelsAtTopRight();
    void curvedAutoLeavingLabelsAtTop();
    void curvedAutoLeavingLabelsAtBottom();
    void curvedAutoLeavingLabelsAtLeft();
    void curvedAutoLeavingLabelsAtRight();
    void curvedAutoHorizontalLines();
    void curvedAutoVerticalLines();
    void curvedClockwise();
    void curvedCounterClockwise();
    void curvedCurvature();
    void balloonCallout();
    void balloonCalloutMargin();
    void balloonCalloutWedgeWidth();
    void balloonCalloutCornerRadius();
    void blendMode();
    void calloutsBlend();

  private:
    bool imageCheck( const QString &testName, QImage &image, unsigned int mismatchCount = 0 );

    QString mReport;
    QString mTestDataDir;
    QgsVectorLayer *vl = nullptr;

};


TestQgsCallout::TestQgsCallout() = default;

void TestQgsCallout::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport += QLatin1String( "<h1>Callout Tests</h1>\n" );

  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  registry->addCalloutType( new QgsCalloutMetadata( QStringLiteral( "Dummy" ), QStringLiteral( "Dummy callout" ), QIcon(), DummyCallout::create ) );

  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
}

void TestQgsCallout::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
  QgsApplication::exitQgis();
}

void TestQgsCallout::init()
{
  const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
  vl = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl->isValid() );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );

  vl->setRenderer( new QgsSingleSymbolRenderer( marker ) );
  QgsProject::instance()->addMapLayer( vl );
}

void TestQgsCallout::cleanup()
{
  QgsProject::instance()->removeMapLayer( vl->id() );
  vl = nullptr;
}

void TestQgsCallout::saveRestore()
{
  DummyCallout *callout = new DummyCallout( QStringLiteral( "a" ), QStringLiteral( "b" ) );

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement noNode;
  QCOMPARE( callout->saveProperties( doc, noNode, QgsReadWriteContext() ), false );

  //test writing with node
  QDomElement calloutParentElem = doc.createElement( QStringLiteral( "parent" ) );
  rootNode.appendChild( calloutParentElem );
  QVERIFY( callout->saveProperties( doc, calloutParentElem, QgsReadWriteContext() ) );

  //check if callout node was written
  const QDomNodeList evalNodeList = calloutParentElem.elementsByTagName( QStringLiteral( "callout" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  const QDomElement calloutElem = evalNodeList.at( 0 ).toElement();
  QCOMPARE( calloutElem.attribute( "type" ), QString( "Dummy" ) );

  //test reading empty node
  QgsCallout *restoredCallout = QgsApplication::calloutRegistry()->createCallout( QStringLiteral( "Dummy" ), noNode, QgsReadWriteContext() );
  QVERIFY( restoredCallout );

  //test reading bad node
  const QDomElement badCalloutElem = doc.createElement( QStringLiteral( "parent" ) );
  restoredCallout = QgsApplication::calloutRegistry()->createCallout( QStringLiteral( "Dummy" ), badCalloutElem, QgsReadWriteContext() );
  QVERIFY( restoredCallout );

  //test reading node
  restoredCallout = QgsApplication::calloutRegistry()->createCallout( QStringLiteral( "Dummy" ), calloutElem, QgsReadWriteContext() );
  QVERIFY( restoredCallout );
  DummyCallout *restoredDummyCallout = dynamic_cast<DummyCallout *>( restoredCallout );
  QVERIFY( restoredDummyCallout );

  //test properties
  QCOMPARE( restoredDummyCallout->prop1(), callout->prop1() );
  QCOMPARE( restoredDummyCallout->prop2(), callout->prop2() );

  delete callout;
  delete restoredCallout;
}

void TestQgsCallout::calloutsInLabeling()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "simple_callout_labels", img, 20 ) );

  // now let's test the variant when integrated into rendering loop
  //note the reference images are slightly different due to use of renderer for this test

  job.start();
  job.waitForFinished();
  QImage img2 = job.renderedImage();

  vl->setLabeling( nullptr );

  QVERIFY( imageCheck( "simple_callout_labels", img2, 20 ) );
}

void TestQgsCallout::calloutsBlend()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->lineSymbol()->setColor( QColor( 200, 200, 200 ) );
  callout->setBlendMode( QPainter::CompositionMode_Multiply );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "simple_callout_labels_blend", img, 20 ) );
}

void TestQgsCallout::calloutsWithRotation()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "simple_callout_rotated", img, 20 ) );
}

void TestQgsCallout::calloutsInLayout()
{
  //test rendering callouts inside a layout (tests DPI scaling of callouts)
  QgsLayout l( QgsProject::instance() );
  std::unique_ptr< QgsLayoutItemPage > page = std::make_unique< QgsLayoutItemPage >( &l );
  page->setPageSize( QgsLayoutSize( 50, 50 ) );
  l.pageCollection()->addPage( page.release() );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 5;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 1, 1, 48, 48 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( vl->extent() );
  map->setLayers( QList<QgsMapLayer *>() << vl );

  QImage outputImage( 591, 591, QImage::Format_RGB32 );
  outputImage.setDotsPerMeterX( 300 / 25.4 * 1000 );
  outputImage.setDotsPerMeterY( 300 / 25.4 * 1000 );
  QgsMultiRenderChecker::drawBackground( &outputImage );
  QPainter p( &outputImage );
  const QgsLayoutExporter exporter( &l );
  exporter.renderPage( &p, 0 );
  p.end();

  const bool result = imageCheck( QStringLiteral( "callouts_layout" ), outputImage );
  QVERIFY( result );
}

void TestQgsCallout::calloutsDisabled()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( false );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_disabled", img, 20 ) );
}

void TestQgsCallout::calloutsDataDefinedDisabled()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::CalloutDraw, QgsProperty::fromExpression( QStringLiteral( "Class = 'Jet'" ) ) );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_enabled", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedSymbol()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->lineSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "case when Class='Jet' then 'green' else 'blue' end" ) ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_symbol", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedSymbolColor()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->lineSymbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "@symbol_color" ) ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_symbol_color", img, 20 ) );
}

void TestQgsCallout::calloutMinimumDistance()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelDistance, QgsProperty::fromExpression( QStringLiteral( "case when Class='Jet' then 20 else 5 end" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setMinimumLength( 10 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_minimum_length", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedMinimumDistance()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::MinimumCalloutLength, QgsProperty::fromExpression( QStringLiteral( "case when Class='Jet' then 30 else 10 end" ) ) );

  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_minimum_length", img, 20 ) );
}

void TestQgsCallout::calloutOffsetFromAnchor()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setOffsetFromAnchor( 4 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_offset_from_anchor", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedOffsetFromAnchor()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::OffsetFromAnchor, QgsProperty::fromExpression( QStringLiteral( "case when Class='Jet' then 2 else 6 end" ) ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_offset_from_anchor", img, 20 ) );
}

void TestQgsCallout::calloutOffsetFromLabel()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setOffsetFromAnchor( 4 );
  callout->setOffsetFromLabel( 4 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_offset_from_label", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedOffsetFromLabel()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::OffsetFromAnchor, QgsProperty::fromExpression( QStringLiteral( "case when Class='Jet' then 2 else 6 end" ) ) );
  callout->dataDefinedProperties().setProperty( QgsCallout::OffsetFromLabel, QgsProperty::fromExpression( QStringLiteral( "case when Class='Jet' then 3 else 4 end" ) ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_offset_from_label", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorTopRight()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelTopRight );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_top_right", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_top_right_rotated", img, 20 ) );

}

void TestQgsCallout::calloutLabelAnchorTopLeft()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelTopLeft );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_top_left", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_top_left_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorTop()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelTopMiddle );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_top_middle", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_top_middle_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorBottomLeft()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelBottomLeft );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_bottom_left", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_bottom_left_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorBottom()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelBottomMiddle );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_bottom_middle", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_bottom_middle_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorBottomRight()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelBottomRight );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_bottom_right", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_bottom_right_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorLeft()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelMiddleLeft );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_left", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_left_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorRight()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelMiddleRight );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_right", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_right_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelAnchorCentroid()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelCentroid );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_anchor_centroid", img, 20 ) );

  img = job.renderedImage();
  p.begin( &img );
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.xOffset = 6;
  settings.yOffset = -6;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromValue( 15 ) );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsDefaultLabelingEngine engine2;
  engine2.setMapSettings( mapSettings );
  engine2.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine2.run( context );
  p.end();
  QVERIFY( imageCheck( "callout_label_anchor_centroid_rotated", img, 20 ) );
}

void TestQgsCallout::calloutLabelDataDefinedAnchor()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelCentroid );
  callout->dataDefinedProperties().setProperty( QgsCallout::LabelAnchorPointPosition, QgsProperty::fromExpression( QStringLiteral( "'TL'" ) ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_label_datadefined_anchor", img, 20 ) );
}

void TestQgsCallout::calloutBehindLabel()
{
  // test that callouts are rendered below labels
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "case when $id = 1 then %1 end" ).arg( mapSettings.extent().center().x() ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "case when $id = 1 then %1 end" ).arg( mapSettings.extent().center().y() ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::ZIndex, QgsProperty::fromExpression( QStringLiteral( "100 - $id" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 2 );
  callout->lineSymbol()->setColor( QColor( 255, 100, 100 ) );

  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_behind_labels", img, 20 ) );
}

void TestQgsCallout::calloutBehindIndividualLabels()
{
  // test that callouts can be rendered below individual labels
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "case when $id = 1 then %1 end" ).arg( mapSettings.extent().center().x() ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "case when $id = 1 then %1 end" ).arg( mapSettings.extent().center().y() ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::ZIndex, QgsProperty::fromExpression( QStringLiteral( "100 - $id" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  TestSimpleCalloutUnder *callout = new TestSimpleCalloutUnder();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 2 );
  callout->lineSymbol()->setColor( QColor( 255, 100, 100 ) );

  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_behind_individual_labels", img, 20 ) );
}

void TestQgsCallout::calloutNoDrawToAllParts()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );
  vl2->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 190040 << 5000030 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190084 5000000 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 << 190040 << 5000050 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190084 5000060 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_no_draw_to_all_parts_simple", img, 20 ) );
}

void TestQgsCallout::calloutDrawToAllParts()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );
  vl2->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 190040 << 5000030 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190084 5000000 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 << 190040 << 5000050 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190084 5000060 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setDrawCalloutToAllParts( true );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_draw_to_all_parts_simple", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedDrawToAllParts()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );
  vl2->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 190040 << 5000030 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190084 5000000 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 << 190040 << 5000050 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190084 5000060 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::DrawCalloutToAllParts, QgsProperty::fromExpression( QStringLiteral( "\"id\"=1" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_draw_to_all_parts_simple", img, 20 ) );
}

void TestQgsCallout::calloutPointOnExterior()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFillSymbol *fill = static_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  fill->setColor( QColor( 255, 0, 0 ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( fill ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 189950 << 5000000 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((190000 4999900, 190100 5000100, 190100 5000100, 190000 5000100, 190000 4999900 ))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 189900, 4999800, 190200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setAnchorPoint( QgsCallout::PointOnExterior );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_point_on_exterior", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedAnchorPoint()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFillSymbol *fill = static_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  fill->setColor( QColor( 255, 0, 0 ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( fill ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 189950 << 5000000 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((190000 4999900, 190100 5000100, 190100 5000100, 190000 5000100, 190000 4999900 ))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 189900, 4999800, 190200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::AnchorPointPosition, QgsProperty::fromExpression( QStringLiteral( "'centroid'" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_anchor_point", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedDestination()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFillSymbol *fill = static_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  fill->setColor( QColor( 255, 0, 0 ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( fill ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 189950 << 5000000 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((190000 4999900, 190100 5000100, 190100 5000100, 190000 5000100, 190000 4999900 ))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  mapSettings.setExtent( QgsRectangle( -15.543214, 42.611493, -15.537179, 42.614680 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::DestinationX, QgsProperty::fromExpression( QStringLiteral( "190004.33" ) ) );
  callout->dataDefinedProperties().setProperty( QgsCallout::DestinationY, QgsProperty::fromExpression( QStringLiteral( "5000096.84" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_destination", img, 20 ) );
}

void TestQgsCallout::calloutDataDefinedOrigin()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFillSymbol *fill = static_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  fill->setColor( QColor( 255, 0, 0 ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( fill ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 189950 << 5000000 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((190000 4999900, 190100 5000100, 190100 5000100, 190000 5000100, 190000 4999900 ))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  mapSettings.setExtent( QgsRectangle( -15.543214, 42.611493, -15.537179, 42.614680 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsSimpleLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::OriginX, QgsProperty::fromExpression( QStringLiteral( "189959.47" ) ) );
  callout->dataDefinedProperties().setProperty( QgsCallout::OriginY, QgsProperty::fromExpression( QStringLiteral( "4999948.34" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "callout_data_defined_origin", img, 20 ) );
}

void TestQgsCallout::manhattan()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_callout", img, 20 ) );
}

void TestQgsCallout::manhattanRotated()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 20;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsSimpleLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_callout_rotated", img, 20 ) );
}

void TestQgsCallout::manhattanNoDrawToAllParts()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );
  vl2->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 190040 << 5000030 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190084 5000000 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 << 190040 << 5000050 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190084 5000060 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsManhattanLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_no_draw_to_all_parts_simple", img, 20 ) );
}

void TestQgsCallout::manhattanDrawToAllParts()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );
  vl2->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 190040 << 5000030 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190084 5000000 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 << 190040 << 5000050 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190084 5000060 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsManhattanLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setDrawCalloutToAllParts( true );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_draw_to_all_parts_simple", img, 20 ) );
}

void TestQgsCallout::manhattanDataDefinedDrawToAllParts()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPoint?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );
  vl2->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 190040 << 5000030 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190084 5000000 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 << 190040 << 5000050 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190084 5000060 )" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( vl2->crs() );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsManhattanLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::DrawCalloutToAllParts, QgsProperty::fromExpression( QStringLiteral( "\"id\"=1" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_data_defined_draw_to_all_parts_simple", img, 20 ) );
}

void TestQgsCallout::manhattanDataDefinedDestination()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFillSymbol *fill = static_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  fill->setColor( QColor( 255, 0, 0 ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( fill ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 189950 << 5000000 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((190000 4999900, 190100 5000100, 190100 5000100, 190000 5000100, 190000 4999900 ))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  mapSettings.setExtent( QgsRectangle( -15.543214, 42.611493, -15.537179, 42.614680 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsManhattanLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::DestinationX, QgsProperty::fromExpression( QStringLiteral( "190004.33" ) ) );
  callout->dataDefinedProperties().setProperty( QgsCallout::DestinationY, QgsProperty::fromExpression( QStringLiteral( "5000096.84" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_data_defined_destination", img, 20 ) );
}

void TestQgsCallout::manhattanDataDefinedOrigin()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsFillSymbol *fill = static_cast< QgsFillSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  fill->setColor( QColor( 255, 0, 0 ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( fill ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 189950 << 5000000 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((190000 4999900, 190100 5000100, 190100 5000100, 190000 5000100, 190000 4999900 ))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  mapSettings.setExtent( QgsRectangle( -15.543214, 42.611493, -15.537179, 42.614680 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsManhattanLineCallout *callout = new QgsManhattanLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->dataDefinedProperties().setProperty( QgsCallout::OriginX, QgsProperty::fromExpression( QStringLiteral( "189959.47" ) ) );
  callout->dataDefinedProperties().setProperty( QgsCallout::OriginY, QgsProperty::fromExpression( QStringLiteral( "4999948.34" ) ) );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "manhattan_data_defined_origin", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtBottomLeft()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelBottomLeft );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_bottom_left", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtBottomRight()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelBottomRight );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_bottom_right", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtTopLeft()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelTopLeft );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_top_left", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtTopRight()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelTopRight );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_top_right", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtTop()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelTopMiddle );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_top", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtBottom()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelBottomMiddle );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_bottom", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtLeft()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelMiddleLeft );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_left", img, 20 ) );
}

void TestQgsCallout::curvedAutoLeavingLabelsAtRight()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setLabelAnchorPoint( QgsCallout::LabelMiddleRight );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_leaving_labels_at_right", img, 20 ) );
}

void TestQgsCallout::curvedAutoHorizontalLines()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000148 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180180 << 5000148 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000167 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000170)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180180 << 5000167 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 20 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_horizontal_lines", img, 20 ) );
}

void TestQgsCallout::curvedAutoVerticalLines()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180110 << 5000180 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180120 5000160)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180110 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 1 << 180175 << 5000180 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180180 5000160)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180175 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 20 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_auto_vertical_lines", img, 20 ) );
}

void TestQgsCallout::curvedClockwise()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setOrientation( QgsCurvedLineCallout::Clockwise );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_clockwise", img, 20 ) );
}

void TestQgsCallout::curvedCounterClockwise()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setOrientation( QgsCurvedLineCallout::CounterClockwise );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_counterclockwise", img, 20 ) );
}

void TestQgsCallout::curvedCurvature()
{
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=id:integer&field=labelx:integer&field=labely:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 180120 << 5000160 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (180150 5000150)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << 180120 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 << 180120 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 4 << 180130 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 5 << 180140 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 6 << 180160 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 7 << 180170 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8 << 180180 << 5000180 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 9 << 180180 << 5000170 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 10 << 180180 << 5000160 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 11 << 180180 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 12 << 180180 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 13 << 180180 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 14 << 180170 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 15 << 180160 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 16 << 180140 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 17 << 180130 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 18 << 180120 << 5000120 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 19 << 180120 << 5000130 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 20 << 180120 << 5000140 );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  const QSize size( 640, 640 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mapSettings.setExtent( QgsRectangle( 180100, 5000100, 180200, 5000200 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );
  context.setCoordinateTransform( QgsCoordinateTransform( vl2->crs(), mapSettings.destinationCrs(), QgsProject::instance()->transformContext() ) );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsCurvedLineCallout *callout = new QgsCurvedLineCallout();
  callout->setEnabled( true );
  callout->lineSymbol()->setWidth( 1 );
  callout->setOrientation( QgsCurvedLineCallout::Clockwise );
  callout->setCurvature( 0.3 );
  settings.setCallout( callout );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2.get(), QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "curved_curvature", img, 20 ) );
}

void TestQgsCallout::balloonCallout()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 7;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsBalloonCallout *callout = new QgsBalloonCallout();
  callout->setEnabled( true );
  callout->setFillSymbol( QgsFillSymbol::createSimple( QVariantMap( { { "color", "#ffcccc"},
    { "outline-width", "1"}
  } ) ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "balloon_callout_render", img, 20 ) );
}

void TestQgsCallout::balloonCalloutMargin()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsBalloonCallout *callout = new QgsBalloonCallout();
  callout->setEnabled( true );
  callout->setFillSymbol( QgsFillSymbol::createSimple( QVariantMap( { { "color", "#ffcccc"},
    { "outline-width", "1"}
  } ) ) );
  callout->setMargins( QgsMargins( 1, 2, 3, 4 ) );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "balloon_callout_margin", img, 20 ) );
}

void TestQgsCallout::balloonCalloutWedgeWidth()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsBalloonCallout *callout = new QgsBalloonCallout();
  callout->setEnabled( true );
  callout->setFillSymbol( QgsFillSymbol::createSimple( QVariantMap( { { "color", "#ffcccc"},
    { "outline-width", "1"}
  } ) ) );
  callout->setWedgeWidth( 6 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "balloon_callout_wedge_width", img, 20 ) );
}

void TestQgsCallout::balloonCalloutCornerRadius()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = Qgis::LabelPlacement::AroundPoint;
  settings.dist = 10;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 20 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );

  QgsBalloonCallout *callout = new QgsBalloonCallout();
  callout->setEnabled( true );
  callout->setFillSymbol( QgsFillSymbol::createSimple( QVariantMap( { { "color", "#ffcccc"},
    { "outline-width", "1"}
  } ) ) );
  callout->setCornerRadius( 3 );
  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "balloon_callout_corner_radius", img, 20 ) );
}

void TestQgsCallout::blendMode()
{
  QgsManhattanLineCallout *callout = new QgsManhattanLineCallout();
  QCOMPARE( callout->containsAdvancedEffects(), false );

  callout->setBlendMode( QPainter::CompositionMode_Multiply );
  QCOMPARE( callout->blendMode(), QPainter::CompositionMode_Multiply );
  QCOMPARE( callout->containsAdvancedEffects(), true );

  callout->setBlendMode( QPainter::CompositionMode_SourceOver );
  QCOMPARE( callout->containsAdvancedEffects(), false );

  callout->dataDefinedProperties().setProperty( QgsCallout::BlendMode, QStringLiteral( "multiply" ) );
  QCOMPARE( callout->containsAdvancedEffects(), true );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsCallout::imageCheck( const QString &testName, QImage &image, unsigned int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  mReport += "<h2>" + testName + "</h2>\n";
  const QString tempDir = QDir::tempPath() + '/';
  const QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsMultiRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "callouts" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  const bool resultFlag = checker.runTest( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}


QGSTEST_MAIN( TestQgsCallout )
#include "testqgscallout.moc"
