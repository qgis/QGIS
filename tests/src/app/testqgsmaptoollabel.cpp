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

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QMouseEvent>

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoollabel.h"
#include "qgsfontutils.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsexpressioncontextutils.h"

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
      std::unique_ptr< QgsVectorLayer > vl1 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:3946&field=text:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
      QVERIFY( vl1->isValid() );
      QgsFeature f1;
      f1.setAttributes( QgsAttributes() << QStringLiteral( "label" ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << QStringLiteral( "l" ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );

      std::unique_ptr< QgsVectorLayer > vl2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:3946&field=text:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
      QVERIFY( vl2->isValid() );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      f1.setAttributes( QgsAttributes() << QStringLiteral( "label" ) );
      QVERIFY( vl2->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << QStringLiteral( "label2" ) );
      QVERIFY( vl2->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 1 ) ) );
      f1.setAttributes( QgsAttributes() << QStringLiteral( "label3" ) );
      QVERIFY( vl2->dataProvider()->addFeature( f1 ) );

      std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1.get() << vl2.get() );
      const std::unique_ptr< QgsAdvancedDigitizingDockWidget > advancedDigitizingDockWidget = std::make_unique< QgsAdvancedDigitizingDockWidget >( canvas.get() );

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

      std::unique_ptr< QgsMapToolLabel > tool( new QgsMapToolLabel( canvas.get(), advancedDigitizingDockWidget.get() ) );

      // no labels yet
      QgsPointXY pt;
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      std::unique_ptr< QMouseEvent > event( new QMouseEvent(
                                              QEvent::MouseButtonPress,
                                              QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
                                            ) );
      QgsLabelPosition pos;
      QVERIFY( !tool->labelAtPosition( event.get(), pos ) );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = QStringLiteral( "text" );
      pls1.placement = QgsPalLayerSettings::OverPoint;
      pls1.quadOffset = QgsPalLayerSettings::QuadrantOver;
      pls1.displayAll = true;
      QgsTextFormat format = pls1.format();
      format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
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
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "l" ) );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 1 );
      event = std::make_unique< QMouseEvent >(
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
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "l" ) );

      //... but fallback to any labels if nothing in current layer
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 1 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label3" ) );

      canvas->setCurrentLayer( vl2.get() );
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label2" ) );
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 1 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl2->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label3" ) );

      canvas->setCurrentLayer( nullptr );

      // when multiple candidates exist, pick the smallest
      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "l" ) );
    }

    void testAlignment()
    {
      QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=halig:string&field=valig:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
      QVERIFY( vl1->isValid() );
      QgsProject::instance()->addMapLayer( vl1 );
      QgsFeature f1;
      f1.setAttributes( QgsAttributes() << QStringLiteral( "right" ) << QStringLiteral( "top" ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 1 ) ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );
      f1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 3, 3 ) ) );
      f1.setAttributes( QgsAttributes() << QStringLiteral( "center" ) << QStringLiteral( "base" ) );
      QVERIFY( vl1->dataProvider()->addFeature( f1 ) );

      std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1 );
      const std::unique_ptr< QgsAdvancedDigitizingDockWidget > advancedDigitizingDockWidget = std::make_unique< QgsAdvancedDigitizingDockWidget >( canvas.get() );

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

      std::unique_ptr< QgsMapToolLabel > tool( new QgsMapToolLabel( canvas.get(), advancedDigitizingDockWidget.get() ) );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = QStringLiteral( "'label'" );
      pls1.isExpression = true;
      pls1.placement = QgsPalLayerSettings::OverPoint;
      pls1.quadOffset = QgsPalLayerSettings::QuadrantOver;
      pls1.displayAll = true;
      QgsTextFormat format = pls1.format();
      format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
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
      std::unique_ptr< QMouseEvent > event( new QMouseEvent(
                                              QEvent::MouseButtonPress,
                                              QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
                                            ) );
      QgsLabelPosition pos;
      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      // defaults to bottom left
      QString hali, vali;
      tool->currentAlignment( hali, vali );
      QCOMPARE( hali, QStringLiteral( "Left" ) );
      QCOMPARE( vali, QStringLiteral( "Bottom" ) );

      // using field bound alignment
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Hali, QgsProperty::fromField( QStringLiteral( "halig" ) ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Vali, QgsProperty::fromField( QStringLiteral( "valig" ) ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );

      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      tool->currentAlignment( hali, vali );
      QCOMPARE( hali, QStringLiteral( "right" ) );
      QCOMPARE( vali, QStringLiteral( "top" ) );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      tool->currentAlignment( hali, vali );
      QCOMPARE( hali, QStringLiteral( "center" ) );
      QCOMPARE( vali, QStringLiteral( "base" ) );

      // now try with expression based alignment
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Hali, QgsProperty::fromExpression( QStringLiteral( "case when $id % 2 = 0 then 'right' else 'left' end" ) ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Vali, QgsProperty::fromExpression( QStringLiteral( "case when $id % 2 = 0 then 'half' else 'cap' end" ) ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );

      canvas->refreshAllLayers();
      canvas->show();
      loop.exec();

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 1, 1 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      tool->currentAlignment( hali, vali );
      QCOMPARE( hali, QStringLiteral( "left" ) );
      QCOMPARE( vali, QStringLiteral( "cap" ) );

      pt = tool->canvas()->mapSettings().mapToPixel().transform( 3, 3 );
      event = std::make_unique< QMouseEvent >(
                QEvent::MouseButtonPress,
                QPoint( std::round( pt.x() ), std::round( pt.y() ) ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier
              );

      QVERIFY( tool->labelAtPosition( event.get(), pos ) );
      QCOMPARE( pos.layerID, vl1->id() );
      QCOMPARE( pos.labelText, QStringLiteral( "label" ) );
      tool->mCurrentLabel = QgsMapToolLabel::LabelDetails( pos, canvas.get() );

      tool->currentAlignment( hali, vali );
      QCOMPARE( hali, QStringLiteral( "right" ) );
      QCOMPARE( vali, QStringLiteral( "half" ) );
    }

    void dataDefinedColumnName()
    {
      QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=label_x_1:string&field=label_y_1:string&field=label_x_2:string&field=label_y_2:string&field=override_x_field:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
      QVERIFY( vl1->isValid() );
      QgsProject::instance()->addMapLayer( vl1 );

      std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
      canvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
      canvas->setLayers( QList<QgsMapLayer *>() << vl1 );
      const std::unique_ptr< QgsAdvancedDigitizingDockWidget > advancedDigitizingDockWidget = std::make_unique< QgsAdvancedDigitizingDockWidget >( canvas.get() );

      std::unique_ptr< QgsMapToolLabel > tool( new QgsMapToolLabel( canvas.get(), advancedDigitizingDockWidget.get() ) );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), QStringLiteral( "var_1" ), QStringLiteral( "1" ) );

      // add some labels
      QgsPalLayerSettings pls1;
      pls1.fieldName = QStringLiteral( "'label'" );

      // not using a column
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromValue( 5 ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromValue( 6 ) );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QgsMapToolLabel::PropertyStatus status = QgsMapToolLabel::PropertyStatus::DoesNotExist;
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // using direct field references
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "label_x_2" ) ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "label_y_2" ) ) );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QStringLiteral( "label_x_2" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QStringLiteral( "label_y_2" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // using expressions which are just field references, should still work
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "\"label_x_1\"" ) ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "\"label_y_1\"" ) ) );

      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QStringLiteral( "label_x_1" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QStringLiteral( "label_y_1" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );


      // using complex expressions which change field depending on a project level variable

      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "case when @var_1 = '1' then \"label_x_1\" else \"label_x_2\" end" ) ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "case when @var_1 = '1' then \"label_y_1\" else \"label_y_2\" end" ) ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QStringLiteral( "label_x_1" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QStringLiteral( "label_y_1" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), QStringLiteral( "var_1" ), QStringLiteral( "2" ) );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QStringLiteral( "label_x_2" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QStringLiteral( "label_y_2" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // another smart situation -- an expression which uses coalesce to store per-feature overrides in a field, otherwise falling back to some complex expression
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "coalesce(\"override_x_field\", $x + 10)" ) ) );
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "COALESCE(case when @var_1 = '1' then \"label_y_1\" else \"label_y_2\" end, $y-20)" ) ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), QStringLiteral( "var_1" ), QStringLiteral( "1" ) );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QStringLiteral( "override_x_field" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QStringLiteral( "label_y_1" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      QgsExpressionContextUtils::setProjectVariable( QgsProject::instance(), QStringLiteral( "var_1" ), QStringLiteral( "2" ) );

      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::AlwaysShow, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::DoesNotExist );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QStringLiteral( "override_x_field" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionY, pls1, vl1, status ), QStringLiteral( "label_y_2" ) );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::Valid );

      // with an invalid expression set
      pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "\"this field does not exist\"" ) ) );
      vl1->setLabeling( new QgsVectorLayerSimpleLabeling( pls1 ) );
      vl1->setLabelsEnabled( true );
      QCOMPARE( tool->dataDefinedColumnName( QgsPalLayerSettings::PositionX, pls1, vl1, status ), QString() );
      QCOMPARE( status, QgsMapToolLabel::PropertyStatus::CurrentExpressionInvalid );
    }


};

QGSTEST_MAIN( TestQgsMapToolLabel )
#include "testqgsmaptoollabel.moc"
