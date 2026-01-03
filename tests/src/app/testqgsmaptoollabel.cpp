/***************************************************************************
     testqgsmaptoollabel.cpp
     -----------------------
    Date                 : July 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfontutils.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoollabel.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerlabelprovider.h"

#include <QMouseEvent>
#include <QObject>
#include <QString>

class TestQgsMapToolLabel : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolLabel() = default;

  private:
  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testSelectLabel()
    {
      auto vl1 = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:3946&field=text:string"_s, u"vl1"_s, u"memory"_s );
      QVERIFY( vl1->isValid() );
      QgsFeature f1;
      f1.setAttributes( QgsAttributes() << u"label"_s );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << u"l"_s );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );

      auto vl2 = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:3946&field=text:string"_s, u"vl1"_s, u"memory"_s );
      QVERIFY( vl2->isValid() );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      f1.setAttributes( QgsAttributes() << u"label"_s );
      QVERIFY( vl2->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << u"label2"_s );
      QVERIFY( vl2->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 1 ) ) );
      f1.setAttributes( QgsAttributes() << u"label3"_s );
      QVERIFY( vl2->dataProvider()->addFeature( f1 ) );

      auto canvas = std::make_unique<QgsMapCanvas>();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1.get() << vl2.get() );
      const std::unique_ptr<QgsAdvancedDigitizingDockWidget> advancedDigitizingDockWidget = std::make_unique<QgsAdvancedDigitizingDockWidget>( canvas.get() );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 500, 500 ) );
      mapSettings.setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      mapSettings.setLayers( QList<QgsMapLayer *>() << vl1.get() << vl2.get() );

      canvas->setFrameStyle( QFrame::NoFrame );
      canvas->resize( 500, 500 );
      canvas->setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      canvas->show(); // to make the canvas resize
      canvas->hide();
      QCOMPARE( canvas->mapSettings().outputSize(), QSize( 500, 500 ) );
      QCOMPARE( canvas->mapSettings().visibleExtent(), QgsRectangle( -1, -1, 4, 4 ) );

      auto tool = std::make_unique<QgsMapToolLabel>( canvas.get(), advancedDigitizingDockWidget.get() );

      // no labels yet
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      std::unique_ptr<QMouseEvent> event( new QMouseEvent(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      ) );
      QgsLabelPosition pos;
      QVERIFY( !tool->labelAtPosition( event.get(), pos ) );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = u"text"_s;
      pls1.placement = Qgis::LabelPlacement::OverPoint;
      pls1.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Over );
      pls1.placementSettings().setAllowDegradedPlacement( true );
      pls1.placementSettings().setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );

      QgsTextFormat format = pls1.format();
      format.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );
      format.setSize( 12 );
      pls1.setFormat( format );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QEventLoop loop;
      connect( canvas.get(), &QgsMapCanvas::mapCanvasRefreshed, &loop, &QEventLoop::quit );
      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      QVERIFY( canvas->labelingResults() );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"l"_s );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 1 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( !tool->labelAtPosition( event.get(), pos ) );

      // label second layer
      vl2->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl2->setLabelsEnabled( true );

      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      // should prioritize current layer
      canvas->setCurrentLayer( vl1.get() );
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"l"_s );

      //... but fallback to any labels if nothing in current layer
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 1 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, u"label3"_s );

      canvas->setCurrentLayer( vl2.get() );
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, u"label"_s );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, u"label2"_s );
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 1 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, u"label3"_s );

      canvas->setCurrentLayer( nullptr );

      // when multiple candidates exist, pick the smallest
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"l"_s );
    }

    void testAlignment()
    {
      QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?crs=epsg:3946&field=halig:string&field=valig:string"_s, u"vl1"_s, u"memory"_s );
      QVERIFY( vl1->isValid() );
      QgsProject::instance()->addMapLayer( vl1 );
      QgsFeature f1;
      f1.setAttributes( QgsAttributes() << u"right"_s << u"top"_s );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << u"center"_s << u"base"_s );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );

      auto canvas = std::make_unique<QgsMapCanvas>();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1 );
      const std::unique_ptr<QgsAdvancedDigitizingDockWidget> advancedDigitizingDockWidget = std::make_unique<QgsAdvancedDigitizingDockWidget>( canvas.get() );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 500, 500 ) );
      mapSettings.setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      mapSettings.setLayers( QList<QgsMapLayer *>() << vl1 );

      canvas->setFrameStyle( QFrame::NoFrame );
      canvas->resize( 500, 500 );
      canvas->setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      canvas->show(); // to make the canvas resize
      canvas->hide();
      QCOMPARE( canvas->mapSettings().outputSize(), QSize( 500, 500 ) );
      QCOMPARE( canvas->mapSettings().visibleExtent(), QgsRectangle( -1, -1, 4, 4 ) );

      auto tool = std::make_unique<QgsMapToolLabel>( canvas.get(), advancedDigitizingDockWidget.get() );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = u"'label'"_s;
      pls1.isExpression = true;
      pls1.placement = Qgis::LabelPlacement::OverPoint;
      pls1.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Over );
      pls1.placementSettings().setAllowDegradedPlacement( true );
      pls1.placementSettings().setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );

      QgsTextFormat format = pls1.format();
      format.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );
      format.setSize( 12 );
      pls1.setFormat( format );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QEventLoop loop;
      connect( canvas.get(), &QgsMapCanvas::mapCanvasRefreshed, &loop, &QEventLoop::quit );
      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      QVERIFY( canvas->labelingResults() );
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      std::unique_ptr<QMouseEvent> event( new QMouseEvent(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      ) );
      QgsLabelPosition pos;
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      // defaults to half center
      QgsMapToolLabel::LabelAlignment labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::HalfCenter );

      // using field bound alignment
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Hali, QgsProperty::fromField( u"halig"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Vali, QgsProperty::fromField( u"valig"_s ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );

      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::TopRight );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::BaseCenter );

      // now try with expression based alignment
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Hali, QgsProperty::fromExpression( u"case when $id % 2 = 0 then 'right' else 'left' end"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Vali, QgsProperty::fromExpression( u"case when $id % 2 = 0 then 'half' else 'cap' end"_s ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );

      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::CapLeft );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique<QMouseEvent>(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      );

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::HalfRight );
    }

    void testAlignmentQuadrant()
    {
      QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?crs=epsg:3946&field=halig:string&field=valig:string"_s, u"vl1"_s, u"memory"_s );
      QVERIFY( vl1->isValid() );
      QgsProject::instance()->addMapLayer( vl1 );
      QgsFeature f1;
      f1.setAttributes( QgsAttributes() << u"right"_s << u"top"_s );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << u"center"_s << u"base"_s );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );

      auto canvas = std::make_unique<QgsMapCanvas>();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1 );
      const std::unique_ptr<QgsAdvancedDigitizingDockWidget> advancedDigitizingDockWidget = std::make_unique<QgsAdvancedDigitizingDockWidget>( canvas.get() );

      QgsMapSettings mapSettings;
      mapSettings.setOutputSize( QSize( 500, 500 ) );
      mapSettings.setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      QVERIFY( mapSettings.hasValidSettings() );

      mapSettings.setLayers( QList<QgsMapLayer *>() << vl1 );

      canvas->setFrameStyle( QFrame::NoFrame );
      canvas->resize( 500, 500 );
      canvas->setExtent( QgsRectangle( -1, -1, 4, 4 ) );
      canvas->show(); // to make the canvas resize
      canvas->hide();
      QCOMPARE( canvas->mapSettings().outputSize(), QSize( 500, 500 ) );
      QCOMPARE( canvas->mapSettings().visibleExtent(), QgsRectangle( -1, -1, 4, 4 ) );

      auto tool = std::make_unique<QgsMapToolLabel>( canvas.get(), advancedDigitizingDockWidget.get() );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = u"'label'"_s;
      pls1.isExpression = true;
      pls1.placement = Qgis::LabelPlacement::OverPoint;
      pls1.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Over );
      pls1.placementSettings().setAllowDegradedPlacement( true );
      pls1.placementSettings().setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );

      QgsTextFormat format = pls1.format();
      format.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );
      format.setSize( 12 );
      pls1.setFormat( format );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QEventLoop loop;
      connect( canvas.get(), &QgsMapCanvas::mapCanvasRefreshed, &loop, &QEventLoop::quit );
      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      QVERIFY( canvas->labelingResults() );
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      std::unique_ptr<QMouseEvent> event( new QMouseEvent(
        QEvent::MouseButtonPress,
        QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
      ) );
      QgsLabelPosition pos;
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, u"label"_s );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      QgsMapToolLabel::LabelAlignment labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::HalfCenter );

      // defaults to bottom left if qudrant is not relevant
      pls1.placement = Qgis::LabelPlacement::OrderedPositionsAroundPoint;
      tool->mCurrentLabel.settings = pls1;
      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::BottomLeft );

      // now try with quadrant property
      pls1.placement = Qgis::LabelPlacement::OverPoint;
      pls1.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::BelowLeft );
      tool->mCurrentLabel.settings = pls1;
      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::TopRight );

      // using field bound alignment
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Hali, QgsProperty::fromField( u"halig"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Vali, QgsProperty::fromField( u"valig"_s ) );
      tool->mCurrentLabel.settings = pls1;
      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::TopRight );

      // now try with expression based alignment
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Hali, QgsProperty::fromExpression( u"case when $id % 2 = 0 then 'right' else 'left' end"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Vali, QgsProperty::fromExpression( u"case when $id % 2 = 0 then 'half' else 'cap' end"_s ) );
      tool->mCurrentLabel.settings = pls1;
      labelAlignment = tool->currentAlignment();
      QCOMPARE( labelAlignment, QgsMapToolLabel::LabelAlignment::CapLeft );
    }

    void dataDefinedColumnName()
    {
      QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?crs=epsg:3946&field=label_x_1:string&field=label_y_1:string&field=label_x_2:string&field=label_y_2:string&field=override_x_field:string"_s, u"vl1"_s, u"memory"_s );
      QVERIFY( vl1->isValid() );
      QgsProject::instance()->addMapLayer( vl1 );

      auto canvas = std::make_unique<QgsMapCanvas>();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1 );
      const std::unique_ptr<QgsAdvancedDigitizingDockWidget> advancedDigitizingDockWidget = std::make_unique<QgsAdvancedDigitizingDockWidget>( canvas.get() );

      auto tool = std::make_unique<QgsMapToolLabel>( canvas.get(), advancedDigitizingDockWidget.get() );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), u"var_1"_s, u"1"_s );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = u"'label'"_s;

      // not using a column
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionX, QgsProperty::fromValue( 5 ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionY, QgsProperty::fromValue( 6 ) );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QgsMapToolLabel::PropertyStatus status = QgsMapToolLabel::PropertyStatus::DoesNotExist;
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // using direct field references
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionX, QgsProperty::fromField( u"label_x_2"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionY, QgsProperty::fromField( u"label_y_2"_s ) );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), u"label_x_2"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), u"label_y_2"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // using expressions which are just field references, should still work
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionX, QgsProperty::fromExpression( u"\"label_x_1\""_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionY, QgsProperty::fromExpression( u"\"label_y_1\""_s ) );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), u"label_x_1"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), u"label_y_1"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );


      // using complex expressions which change field depending on a project level variable

      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionX, QgsProperty::fromExpression( u"case when @var_1 = '1' then \"label_x_1\" else \"label_x_2\" end"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionY, QgsProperty::fromExpression( u"case when @var_1 = '1' then \"label_y_1\" else \"label_y_2\" end"_s ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), u"label_x_1"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), u"label_y_1"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), u"var_1"_s, u"2"_s );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), u"label_x_2"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), u"label_y_2"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // another smart situation -- an expression which uses coalesce to store per-feature overrides in a field, otherwise falling back to some complex expression
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionX, QgsProperty::fromExpression( u"coalesce(\"override_x_field\", $x + 10)"_s ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionY, QgsProperty::fromExpression( u"COALESCE(case when @var_1 = '1' then \"label_y_1\" else \"label_y_2\" end, $y-20)"_s ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), u"var_1"_s, u"1"_s );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), u"override_x_field"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), u"label_y_1"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), u"var_1"_s, u"2"_s );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), u"override_x_field"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, pls1, vl1, status ), u"label_y_2"_s );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // with an invalid expression set
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::PositionX, QgsProperty::fromExpression( u"\"this field does not exist\""_s ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::CurrentExpressionInvalid );
    }
};

QGSTEST_MAIN( TestQgsMapToolLabel )
#include "testqgsmaptoollabel.moc"
