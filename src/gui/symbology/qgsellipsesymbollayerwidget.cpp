/***************************************************************************
 qgsellipsesymbollayerwidget.cpp
 ---------------------
 begin                : June 2011
 copyright            : (C) 2011 by Marco Hugentobler
 email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsellipsesymbollayerwidget.h"
#include "qgsellipsesymbollayer.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include <QColorDialog>

QgsEllipseSymbolLayerWidget::QgsEllipseSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent )
  : QgsSymbolLayerWidget( parent, vl )

{
  setupUi( this );
  connect( mShapeListWidget, &QListWidget::itemSelectionChanged, this, &QgsEllipseSymbolLayerWidget::mShapeListWidget_itemSelectionChanged );
  connect( mWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsEllipseSymbolLayerWidget::mWidthSpinBox_valueChanged );
  connect( mHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsEllipseSymbolLayerWidget::mHeightSpinBox_valueChanged );
  connect( mRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsEllipseSymbolLayerWidget::mRotationSpinBox_valueChanged );
  connect( mStrokeStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEllipseSymbolLayerWidget::mStrokeStyleComboBox_currentIndexChanged );
  connect( mStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsEllipseSymbolLayerWidget::mStrokeWidthSpinBox_valueChanged );
  connect( btnChangeColorStroke, &QgsColorButton::colorChanged, this, &QgsEllipseSymbolLayerWidget::btnChangeColorStroke_colorChanged );
  connect( btnChangeColorFill, &QgsColorButton::colorChanged, this, &QgsEllipseSymbolLayerWidget::btnChangeColorFill_colorChanged );
  connect( mSymbolWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsEllipseSymbolLayerWidget::mSymbolWidthUnitWidget_changed );
  connect( mStrokeWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsEllipseSymbolLayerWidget::mStrokeWidthUnitWidget_changed );
  connect( mSymbolHeightUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsEllipseSymbolLayerWidget::mSymbolHeightUnitWidget_changed );
  connect( mOffsetUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsEllipseSymbolLayerWidget::mOffsetUnitWidget_changed );
  connect( mHorizontalAnchorComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEllipseSymbolLayerWidget::mHorizontalAnchorComboBox_currentIndexChanged );
  connect( mVerticalAnchorComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEllipseSymbolLayerWidget::mVerticalAnchorComboBox_currentIndexChanged );

  mSymbolWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                    << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mSymbolHeightUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mStrokeWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                    << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  btnChangeColorFill->setAllowOpacity( true );
  btnChangeColorFill->setColorDialogTitle( tr( "Select Fill Color" ) );
  btnChangeColorFill->setContext( QStringLiteral( "symbology" ) );
  btnChangeColorFill->setShowNoColor( true );
  btnChangeColorFill->setNoColorString( tr( "Transparent Fill" ) );
  btnChangeColorStroke->setAllowOpacity( true );
  btnChangeColorStroke->setColorDialogTitle( tr( "Select Stroke Color" ) );
  btnChangeColorStroke->setContext( QStringLiteral( "symbology" ) );
  btnChangeColorStroke->setShowNoColor( true );
  btnChangeColorStroke->setNoColorString( tr( "Transparent Stroke" ) );

  mFillColorDDBtn->registerLinkedWidget( btnChangeColorFill );
  mStrokeColorDDBtn->registerLinkedWidget( btnChangeColorStroke );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );
  mRotationSpinBox->setClearValue( 0.0 );

  int size = mShapeListWidget->iconSize().width();
  size = std::max( 30, static_cast< int >( std::round( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 3 ) ) );
  mShapeListWidget->setGridSize( QSize( size * 1.2, size * 1.2 ) );
  mShapeListWidget->setIconSize( QSize( size, size ) );

  const double markerSize = size * 0.8;
  const auto shapes = QgsEllipseSymbolLayer::availableShapes();
  for ( const QgsEllipseSymbolLayer::Shape shape : shapes )
  {
    QgsEllipseSymbolLayer *lyr = new QgsEllipseSymbolLayer();
    lyr->setSymbolWidthUnit( QgsUnitTypes::RenderPixels );
    lyr->setSymbolHeightUnit( QgsUnitTypes::RenderPixels );
    lyr->setShape( shape );
    lyr->setStrokeColor( QColor( 0, 0, 0 ) );
    lyr->setFillColor( QColor( 200, 200, 200 ) );
    lyr->setSymbolWidth( markerSize );
    lyr->setSymbolHeight( markerSize * 0.75 );
    const QIcon icon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( lyr, QgsUnitTypes::RenderPixels, QSize( size, size ) );
    QListWidgetItem *item = new QListWidgetItem( icon, QString(), mShapeListWidget );
    item->setData( Qt::UserRole, static_cast< int >( shape ) );
    item->setToolTip( QgsEllipseSymbolLayer::encodeShape( shape ) );
    delete lyr;
  }
  // show at least 2 rows (only 1 row is required, but looks too cramped)
  mShapeListWidget->setMinimumHeight( mShapeListWidget->gridSize().height() * 2.1 );

  connect( spinOffsetX, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsEllipseSymbolLayerWidget::setOffset );
  connect( spinOffsetY, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsEllipseSymbolLayerWidget::setOffset );
  connect( cboJoinStyle, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEllipseSymbolLayerWidget::penJoinStyleChanged );
  connect( cboCapStyle, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEllipseSymbolLayerWidget::penCapStyleChanged );
}

void QgsEllipseSymbolLayerWidget::setSymbolLayer( QgsSymbolLayer *layer )
{
  if ( !layer || layer->layerType() != QLatin1String( "EllipseMarker" ) )
  {
    return;
  }

  mLayer = static_cast<QgsEllipseSymbolLayer *>( layer );
  mWidthSpinBox->setValue( mLayer->symbolWidth() );
  mHeightSpinBox->setValue( mLayer->symbolHeight() );
  mRotationSpinBox->setValue( mLayer->angle() );
  mStrokeStyleComboBox->setPenStyle( mLayer->strokeStyle() );
  mStrokeWidthSpinBox->setValue( mLayer->strokeWidth() );
  btnChangeColorStroke->setColor( mLayer->strokeColor() );
  btnChangeColorFill->setColor( mLayer->fillColor() );

  const QgsEllipseSymbolLayer::Shape shape = mLayer->shape();
  for ( int i = 0; i < mShapeListWidget->count(); ++i )
  {
    if ( static_cast< QgsEllipseSymbolLayer::Shape >( mShapeListWidget->item( i )->data( Qt::UserRole ).toInt() ) == shape )
    {
      mShapeListWidget->setCurrentRow( i );
      break;
    }
  }
  btnChangeColorFill->setEnabled( QgsEllipseSymbolLayer::shapeIsFilled( mLayer->shape() ) );

  //set combo entries to current values
  blockComboSignals( true );
  mSymbolWidthUnitWidget->setUnit( mLayer->symbolWidthUnit() );
  mSymbolWidthUnitWidget->setMapUnitScale( mLayer->symbolWidthMapUnitScale() );
  mStrokeWidthUnitWidget->setUnit( mLayer->strokeWidthUnit() );
  mStrokeWidthUnitWidget->setMapUnitScale( mLayer->strokeWidthMapUnitScale() );
  mSymbolHeightUnitWidget->setUnit( mLayer->symbolHeightUnit() );
  mSymbolHeightUnitWidget->setMapUnitScale( mLayer->symbolHeightMapUnitScale() );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  const QPointF offsetPt = mLayer->offset();
  spinOffsetX->setValue( offsetPt.x() );
  spinOffsetY->setValue( offsetPt.y() );
  mHorizontalAnchorComboBox->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  mVerticalAnchorComboBox->setCurrentIndex( mLayer->verticalAnchorPoint() );
  cboJoinStyle->setPenJoinStyle( mLayer->penJoinStyle() );
  cboCapStyle->setPenCapStyle( mLayer->penCapStyle() );
  blockComboSignals( false );

  registerDataDefinedButton( mSymbolWidthDDBtn, QgsSymbolLayer::PropertyWidth );
  registerDataDefinedButton( mSymbolHeightDDBtn, QgsSymbolLayer::PropertyHeight );
  registerDataDefinedButton( mRotationDDBtn, QgsSymbolLayer::PropertyAngle );
  registerDataDefinedButton( mStrokeWidthDDBtn, QgsSymbolLayer::PropertyStrokeWidth );
  registerDataDefinedButton( mFillColorDDBtn, QgsSymbolLayer::PropertyFillColor );
  registerDataDefinedButton( mStrokeColorDDBtn, QgsSymbolLayer::PropertyStrokeColor );
  registerDataDefinedButton( mStrokeStyleDDBtn, QgsSymbolLayer::PropertyStrokeStyle );
  registerDataDefinedButton( mJoinStyleDDBtn, QgsSymbolLayer::PropertyJoinStyle );
  registerDataDefinedButton( mCapStyleDDBtn, QgsSymbolLayer::PropertyCapStyle );
  registerDataDefinedButton( mShapeDDBtn, QgsSymbolLayer::PropertyName );
  registerDataDefinedButton( mOffsetDDBtn, QgsSymbolLayer::PropertyOffset );
  registerDataDefinedButton( mHorizontalAnchorDDBtn, QgsSymbolLayer::PropertyHorizontalAnchor );
  registerDataDefinedButton( mVerticalAnchorDDBtn, QgsSymbolLayer::PropertyVerticalAnchor );

}

QgsSymbolLayer *QgsEllipseSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsEllipseSymbolLayerWidget::mShapeListWidget_itemSelectionChanged()
{
  if ( mLayer )
  {
    mLayer->setShape( static_cast< QgsEllipseSymbolLayer::Shape>( mShapeListWidget->currentItem()->data( Qt::UserRole ).toInt() ) );
    btnChangeColorFill->setEnabled( QgsEllipseSymbolLayer::shapeIsFilled( mLayer->shape() ) );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mHeightSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolHeight( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mStrokeStyleComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( mLayer )
  {
    mLayer->setStrokeStyle( mStrokeStyleComboBox->penStyle() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mStrokeWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setStrokeWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::btnChangeColorStroke_colorChanged( const QColor &newColor )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setStrokeColor( newColor );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::btnChangeColorFill_colorChanged( const QColor &newColor )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setFillColor( newColor );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::mSymbolWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSymbolWidthUnit( mSymbolWidthUnitWidget->unit() );
    mLayer->setSymbolWidthMapUnitScale( mSymbolWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mStrokeWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setStrokeWidthUnit( mStrokeWidthUnitWidget->unit() );
    mLayer->setStrokeWidthMapUnitScale( mStrokeWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mSymbolHeightUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSymbolHeightUnit( mSymbolHeightUnitWidget->unit() );
    mLayer->setSymbolHeightMapUnitScale( mSymbolHeightUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::penJoinStyleChanged()
{
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::penCapStyleChanged()
{
  mLayer->setPenCapStyle( cboCapStyle->penCapStyle() );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::blockComboSignals( bool block )
{
  mSymbolWidthUnitWidget->blockSignals( block );
  mStrokeWidthUnitWidget->blockSignals( block );
  mSymbolHeightUnitWidget->blockSignals( block );
  mHorizontalAnchorComboBox->blockSignals( block );
  mVerticalAnchorComboBox->blockSignals( block );
  mSymbolWidthUnitWidget->blockSignals( block );
  mStrokeWidthUnitWidget->blockSignals( block );
  mSymbolHeightUnitWidget->blockSignals( block );
  mOffsetUnitWidget->blockSignals( block );
  cboJoinStyle->blockSignals( block );
  cboCapStyle->blockSignals( block );
}

void QgsEllipseSymbolLayerWidget::mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint( ( QgsMarkerSymbolLayer::HorizontalAnchorPoint ) index );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint( ( QgsMarkerSymbolLayer::VerticalAnchorPoint ) index );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}


