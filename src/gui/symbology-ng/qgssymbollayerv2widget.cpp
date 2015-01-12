/***************************************************************************
 qgssymbollayerv2widget.cpp - symbol layer widgets

 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerv2widget.h"

#include "qgslinesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"

#include "characterwidget.h"
#include "qgsdashspacedialog.h"
#include "qgsdatadefinedsymboldialog.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssvgcache.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsstylev2.h" //for symbol selector dialog

#include "qgsapplication.h"

#include "qgslogger.h"

#include <QAbstractButton>
#include <QColorDialog>
#include <QCursor>
#include <QDir>
#include <QFileDialog>
#include <QPainter>
#include <QSettings>
#include <QStandardItemModel>
#include <QSvgRenderer>
#include <QMessageBox>
#include <QTextDocument>

#define QGS_BLOCK_SIGNAL_DURING_CALL( widget, call )\
  {\
    widget->blockSignals( true );\
    widget->call;\
    widget->blockSignals( false );\
  }

QString QgsSymbolLayerV2Widget::dataDefinedPropertyLabel( const QString &entryName )
{
  QString label = entryName;
  if ( entryName == "size" )
  {
    label = tr( "Size" );
    QgsMarkerSymbolLayerV2 * layer = dynamic_cast<QgsMarkerSymbolLayerV2 *>( symbolLayer() );
    if ( layer )
    {
      switch ( layer->scaleMethod() )
      {
        case QgsSymbolV2::ScaleArea:
          label += " (" + tr( "area" ) + ")";
          break;
        case QgsSymbolV2::ScaleDiameter:
          label += " (" + tr( "diameter" ) + ")";
          break;
      }
    }
  }
  return label;
}

QgsSimpleLineSymbolLayerV2Widget::QgsSimpleLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mPenWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mDashPatternUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  btnChangeColor->setAllowAlpha( true );
  btnChangeColor->setColorDialogTitle( tr( "Select line color" ) );
  btnChangeColor->setContext( "symbology" );

  spinOffset->setClearValue( 0.0 );

  if ( vl && vl->geometryType() != QGis::Polygon )
  {
    //draw inside polygon checkbox only makes sense for polygon layers
    mDrawInsideCheckBox->hide();
  }

  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( penWidthChanged() ) );
  connect( btnChangeColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( colorChanged( const QColor& ) ) );
  connect( cboPenStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  connect( spinOffset, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( cboCapStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  updatePatternIcon();


}

void QgsSimpleLineSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "SimpleLine" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleLineSymbolLayerV2*>( layer );

  // set units
  QGS_BLOCK_SIGNAL_DURING_CALL( mPenWidthUnitWidget, setUnit( mLayer->widthUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mPenWidthUnitWidget, setMapUnitScale( mLayer->widthMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mDashPatternUnitWidget, setUnit( mLayer->customDashPatternUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mDashPatternUnitWidget, setMapUnitScale( mLayer->customDashPatternMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mDashPatternUnitWidget, setMapUnitScale( mLayer->customDashPatternMapUnitScale() ) )

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( spinWidth, setValue( mLayer->width() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColor, setColor( mLayer->color() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffset, setValue( mLayer->offset() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( cboPenStyle, setPenStyle( mLayer->penStyle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( cboJoinStyle, setPenJoinStyle( mLayer->penJoinStyle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( cboCapStyle, setPenCapStyle( mLayer->penCapStyle() ) )

  //use a custom dash pattern?
  const bool useCustomDashPattern = mLayer->useCustomDashPattern();
  mChangePatternButton->setEnabled( useCustomDashPattern );
  mDashPatternDDBtn->setEnabled( useCustomDashPattern );
  label_3->setEnabled( !useCustomDashPattern );
  cboPenStyle->setEnabled( !useCustomDashPattern );
  QGS_BLOCK_SIGNAL_DURING_CALL( mCustomCheckBox, setCheckState( useCustomDashPattern ? Qt::Checked : Qt::Unchecked ) )

  //draw inside polygon?
  const bool drawInsidePolygon = mLayer->drawInsidePolygon();
  QGS_BLOCK_SIGNAL_DURING_CALL( mDrawInsideCheckBox, setCheckState( drawInsidePolygon ? Qt::Checked : Qt::Unchecked ) )

  updatePatternIcon();

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mColorDDBtn, "color", String, QgsDataDefinedSymbolDialog::colorHelpText())
  QGS_REGISTER_DATA_DEFINED_BUTTON( mPenWidthDDBtn, "width", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mOffsetDDBtn, "offset", String, QgsDataDefinedSymbolDialog::offsetHelpText())
  QGS_REGISTER_DATA_DEFINED_BUTTON( mDashPatternDDBtn, "customdash", String, tr( "'<dash>;<space>' e.g. '8;2;1;2'" ) )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mJoinStyleDDBtn, "joinstyle", String, tr( "'bevel'|'miter'|'round'" ))
  QGS_REGISTER_DATA_DEFINED_BUTTON( mCapStyleDDBtn, "capstyle", String, tr( "'square'|'flat'|'round'" ) )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsSimpleLineSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleLineSymbolLayerV2Widget::penWidthChanged()
{
  mLayer->setWidth( spinWidth->value() );
  updatePatternIcon();
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::colorChanged( const QColor& color )
{
  mLayer->setColor( color );
  updatePatternIcon();
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::penStyleChanged()
{
  mLayer->setPenStyle( cboPenStyle->penStyle() );
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  mLayer->setPenCapStyle( cboCapStyle->penCapStyle() );
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::offsetChanged()
{
  mLayer->setOffset( spinOffset->value() );
  updatePatternIcon();
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::on_mCustomCheckBox_stateChanged( int state )
{
  bool checked = ( state == Qt::Checked );
  mChangePatternButton->setEnabled( checked );
  mDashPatternDDBtn->setEnabled( checked );
  label_3->setEnabled( !checked );
  cboPenStyle->setEnabled( !checked );

  mLayer->setUseCustomDashPattern( checked );
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::on_mChangePatternButton_clicked()
{
  QgsDashSpaceDialog d( mLayer->customDashVector() );
  if ( d.exec() == QDialog::Accepted )
  {
    mLayer->setCustomDashVector( d.dashDotVector() );
    updatePatternIcon();
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::on_mPenWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mPenWidthUnitWidget->getUnit() );
    mLayer->setWidthUnit( unit );
    mLayer->setWidthMapUnitScale( mPenWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::on_mDashPatternUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mDashPatternUnitWidget->getUnit() );
    mLayer->setCustomDashPatternUnit( unit );
    mLayer->setCustomDashPatternMapUnitScale( mDashPatternUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::on_mDrawInsideCheckBox_stateChanged( int state )
{
  bool checked = ( state == Qt::Checked );
  mLayer->setDrawInsidePolygon( checked );
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::updatePatternIcon()
{
  if ( !mLayer )
  {
    return;
  }
  QgsSimpleLineSymbolLayerV2* layerCopy = dynamic_cast<QgsSimpleLineSymbolLayerV2*>( mLayer->clone() );
  if ( !layerCopy )
  {
    return;
  }
  layerCopy->setUseCustomDashPattern( true );
  QIcon buttonIcon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( layerCopy, QgsSymbolV2::MM, mChangePatternButton->iconSize() );
  mChangePatternButton->setIcon( buttonIcon );
  delete layerCopy;
}


///////////


QgsSimpleMarkerSymbolLayerV2Widget::QgsSimpleMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mSizeUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOutlineWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  btnChangeColorFill->setAllowAlpha( true );
  btnChangeColorFill->setColorDialogTitle( tr( "Select fill color" ) );
  btnChangeColorFill->setContext( "symbology" );
  btnChangeColorFill->setShowNoColor( true );
  btnChangeColorFill->setNoColorString( tr( "Transparent fill" ) );
  btnChangeColorBorder->setAllowAlpha( true );
  btnChangeColorBorder->setColorDialogTitle( tr( "Select border color" ) );
  btnChangeColorBorder->setContext( "symbology" );
  btnChangeColorBorder->setShowNoColor( true );
  btnChangeColorBorder->setNoColorString( tr( "Transparent border" ) );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  QSize size = lstNames->iconSize();
  QStringList names;
  names << "circle" << "rectangle" << "diamond" << "pentagon" << "cross" << "cross2" << "triangle"
  << "equilateral_triangle" << "star" << "regular_star" << "arrow" << "line" << "arrowhead" << "filled_arrowhead";
  double markerSize = DEFAULT_POINT_SIZE * 2;
  for ( int i = 0; i < names.count(); ++i )
  {
    QgsSimpleMarkerSymbolLayerV2* lyr = new QgsSimpleMarkerSymbolLayerV2( names[i], QColor( 200, 200, 200 ), QColor( 0, 0, 0 ), markerSize );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, size );
    QListWidgetItem* item = new QListWidgetItem( icon, QString(), lstNames );
    item->setData( Qt::UserRole, names[i] );
    delete lyr;
  }

  connect( lstNames, SIGNAL( currentRowChanged( int ) ), this, SLOT( setName() ) );
  connect( btnChangeColorBorder, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColorBorder( const QColor& ) ) );
  connect( btnChangeColorFill, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColorFill( const QColor& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
}

void QgsSimpleMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SimpleMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleMarkerSymbolLayerV2*>( layer );

  // set values
  QString name = mLayer->name();
  for ( int i = 0; i < lstNames->count(); ++i )
  {
    if ( lstNames->item( i )->data( Qt::UserRole ).toString() == name )
    {
      lstNames->setCurrentRow( i );
      break;
    }
  }
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColorBorder, setColor( mLayer->borderColor() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColorFill, setColor( mLayer->color() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinSize, setValue( mLayer->size() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinAngle, setValue( mLayer->angle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOutlineStyleComboBox, setPenStyle( mLayer->outlineStyle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOutlineWidthSpinBox, setValue( mLayer->outlineWidth() ) )
  // without blocking signals the value gets changed because of slot setOffset()
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetY, setValue( mLayer->offset().y() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSizeUnitWidget, setUnit( mLayer->sizeUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSizeUnitWidget, setMapUnitScale( mLayer->sizeMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOutlineWidthUnitWidget, setUnit( mLayer->outlineWidthUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOutlineWidthUnitWidget, setMapUnitScale( mLayer->outlineWidthMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalAnchorComboBox, setCurrentIndex( mLayer->horizontalAnchorPoint() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalAnchorComboBox, setCurrentIndex( mLayer->verticalAnchorPoint() ) )

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mNameDDBtn, "name", String, "'square'|'rectangle'|'diamond'|'pentagon'|'triangle'|'equilateral_triangle'|'star'|'regular_star'|'arrow'|'filled_arrowhead'|'circle'|'cross'|'x'|'cross2'|'line'|'arrowhead'" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mFillColorDDBtn, "color", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderColorDDBtn, "color_border", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mOutlineWidthDDBtn, "outline_width", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mSizeDDBtn, "size", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mAngleDDBtn, "angle", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mOffsetDDBtn, "offset", String, QgsDataDefinedSymbolDialog::offsetHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mHorizontalAnchorDDBtn, "horizontal_anchor_point", String, QgsDataDefinedSymbolDialog::horizontalAnchorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mVerticalAnchorDDBtn, "vertical_anchor_point", String, QgsDataDefinedSymbolDialog::verticalAnchorHelpText() )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleMarkerSymbolLayerV2Widget::setName()
{
  mLayer->setName( lstNames->currentItem()->data( Qt::UserRole ).toString() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setColorBorder( const QColor& color )
{
  mLayer->setBorderColor( color );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setColorFill( const QColor& color )
{
  mLayer->setColor( color );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setSize()
{
  mLayer->setSize( spinSize->value() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setAngle()
{
  mLayer->setAngle( spinAngle->value() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mOutlineStyleComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( mLayer )
  {
    mLayer->setOutlineStyle( mOutlineStyleComboBox->penStyle() );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mSizeUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSizeUnitWidget->getUnit() );
    mLayer->setSizeUnit( unit );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOutlineWidthUnitWidget->getUnit() );
    mLayer->setOutlineWidthUnit( unit );
    mLayer->setOutlineWidthMapUnitScale( mOutlineWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint(( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint ) index );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint(( QgsMarkerSymbolLayerV2::VerticalAnchorPoint ) index );
    emit changed();
  }
}


///////////

QgsSimpleFillSymbolLayerV2Widget::QgsSimpleFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mBorderWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  btnChangeColor->setAllowAlpha( true );
  btnChangeColor->setColorDialogTitle( tr( "Select fill color" ) );
  btnChangeColor->setContext( "symbology" );
  btnChangeColor->setShowNoColor( true );
  btnChangeColor->setNoColorString( tr( "Transparent fill" ) );
  btnChangeBorderColor->setAllowAlpha( true );
  btnChangeBorderColor->setColorDialogTitle( tr( "Select border color" ) );
  btnChangeBorderColor->setContext( "symbology" );
  btnChangeBorderColor->setShowNoColor( true );
  btnChangeBorderColor->setNoColorString( tr( "Transparent border" ) );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  connect( btnChangeColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor( const QColor& ) ) );
  connect( cboFillStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setBrushStyle() ) );
  connect( btnChangeBorderColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setBorderColor( const QColor& ) ) );
  connect( spinBorderWidth, SIGNAL( valueChanged( double ) ), this, SLOT( borderWidthChanged() ) );
  connect( cboBorderStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( borderStyleChanged() ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( borderStyleChanged() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
}

void QgsSimpleFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SimpleFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleFillSymbolLayerV2*>( layer );

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColor, setColor( mLayer->color() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( cboFillStyle, setBrushStyle( mLayer->brushStyle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeBorderColor, setColor( mLayer->borderColor() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( cboBorderStyle, setPenStyle( mLayer->borderStyle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinBorderWidth, setValue( mLayer->borderWidth() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( cboJoinStyle, setPenJoinStyle( mLayer->penJoinStyle() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetY, setValue( mLayer->offset().y() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mBorderWidthUnitWidget, setUnit( mLayer->borderWidthUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mBorderWidthUnitWidget, setMapUnitScale( mLayer->borderWidthMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )

  mDataDefinedPropertyButtons.clear();
  
#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mFillColorDDBtn, "color", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderColorDDBtn, "color_border", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderWidthDDBtn, "width_border", Double, "" )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsSimpleFillSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}
QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleFillSymbolLayerV2Widget::setColor( const QColor& color )
{
  mLayer->setColor( color );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::setBorderColor( const QColor& color )
{
  mLayer->setBorderColor( color );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::setBrushStyle()
{
  mLayer->setBrushStyle( cboFillStyle->brushStyle() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::borderWidthChanged()
{
  mLayer->setBorderWidth( spinBorderWidth->value() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::borderStyleChanged()
{
  mLayer->setBorderStyle( cboBorderStyle->penStyle() );
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::offsetChanged()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::on_mBorderWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mBorderWidthUnitWidget->getUnit() );
    mLayer->setBorderWidthUnit( unit );
    mLayer->setBorderWidthMapUnitScale( mBorderWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleFillSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

///////////

QgsGradientFillSymbolLayerV2Widget::QgsGradientFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  cboGradientColorRamp->setShowGradientOnly( true );
  cboGradientColorRamp->populate( QgsStyleV2::defaultStyle() );

  btnChangeColor->setAllowAlpha( true );
  btnChangeColor->setColorDialogTitle( tr( "Select gradient color" ) );
  btnChangeColor->setContext( "symbology" );
  btnChangeColor->setShowNoColor( true );
  btnChangeColor->setNoColorString( tr( "Transparent" ) );
  btnChangeColor2->setAllowAlpha( true );
  btnChangeColor2->setColorDialogTitle( tr( "Select gradient color" ) );
  btnChangeColor2->setContext( "symbology" );
  btnChangeColor2->setShowNoColor( true );
  btnChangeColor2->setNoColorString( tr( "Transparent" ) );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  connect( btnChangeColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor( const QColor& ) ) );
  connect( btnChangeColor2, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor2( const QColor& ) ) );
  connect( cboGradientColorRamp, SIGNAL( currentIndexChanged( int ) ), this, SLOT( applyColorRamp() ) );
  connect( cboGradientType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setGradientType( int ) ) );
  connect( cboCoordinateMode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setCoordinateMode( int ) ) );
  connect( cboGradientSpread, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setGradientSpread( int ) ) );
  connect( radioTwoColor, SIGNAL( toggled( bool ) ), this, SLOT( colorModeChanged() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( spinRefPoint1X, SIGNAL( valueChanged( double ) ), this, SLOT( referencePointChanged() ) );
  connect( spinRefPoint1Y, SIGNAL( valueChanged( double ) ), this, SLOT( referencePointChanged() ) );
  connect( checkRefPoint1Centroid, SIGNAL( toggled( bool ) ), this, SLOT( referencePointChanged() ) );
  connect( spinRefPoint2X, SIGNAL( valueChanged( double ) ), this, SLOT( referencePointChanged() ) );
  connect( spinRefPoint2Y, SIGNAL( valueChanged( double ) ), this, SLOT( referencePointChanged() ) );
  connect( checkRefPoint2Centroid, SIGNAL( toggled( bool ) ), this, SLOT( referencePointChanged() ) );
}

void QgsGradientFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "GradientFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsGradientFillSymbolLayerV2*>( layer );

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColor, setColor( mLayer->color() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColor2, setColor( mLayer->color2() ) )

  if ( mLayer->gradientColorType() == QgsGradientFillSymbolLayerV2::SimpleTwoColor )
  {
    radioTwoColor->setChecked( true );
    cboGradientColorRamp->setEnabled( false );
  }
  else
  {
    radioColorRamp->setChecked( true );
    btnChangeColor->setEnabled( false );
    btnChangeColor2->setEnabled( false );
  }

  // set source color ramp
  if ( mLayer->colorRamp() )
    QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientColorRamp, setSourceColorRamp( mLayer->colorRamp() ) )

  switch ( mLayer->gradientType() )
  {
    case QgsGradientFillSymbolLayerV2::Linear:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientType, setCurrentIndex( 0 ) )
      break;
    case QgsGradientFillSymbolLayerV2::Radial:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientType, setCurrentIndex( 1 ) )
      break;
    case QgsGradientFillSymbolLayerV2::Conical:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientType, setCurrentIndex( 2 ) )
      break;
  }

  switch ( mLayer->coordinateMode() )
  {
    case QgsGradientFillSymbolLayerV2::Viewport:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboCoordinateMode, setCurrentIndex( 1 ) )
      QGS_BLOCK_SIGNAL_DURING_CALL( checkRefPoint1Centroid, setEnabled( false ) )
      QGS_BLOCK_SIGNAL_DURING_CALL( checkRefPoint2Centroid, setEnabled( false ) )
      break;
    case QgsGradientFillSymbolLayerV2::Feature:
    default:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboCoordinateMode, setCurrentIndex( 0 ) )
      break;
  }

  switch ( mLayer->gradientSpread() )
  {
    case QgsGradientFillSymbolLayerV2::Pad:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientSpread, setCurrentIndex( 0 ) )
      break;
    case QgsGradientFillSymbolLayerV2::Repeat:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientSpread, setCurrentIndex( 1 ) )
      break;
    case QgsGradientFillSymbolLayerV2::Reflect:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientSpread, setCurrentIndex( 2 ) )
      break;
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( spinRefPoint1X, setValue( mLayer->referencePoint1().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinRefPoint1Y, setValue( mLayer->referencePoint1().y() ) )

  checkRefPoint1Centroid->blockSignals( true );
  checkRefPoint1Centroid->setChecked( mLayer->referencePoint1IsCentroid() );
  if ( mLayer->referencePoint1IsCentroid() )
  {
    spinRefPoint1X->setEnabled( false );
    spinRefPoint1Y->setEnabled( false );
  }
  checkRefPoint1Centroid->blockSignals( false );

  QGS_BLOCK_SIGNAL_DURING_CALL( spinRefPoint2X, setValue( mLayer->referencePoint2().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinRefPoint2Y, setValue( mLayer->referencePoint2().y() ) )

  checkRefPoint2Centroid->blockSignals( true );
  checkRefPoint2Centroid->setChecked( mLayer->referencePoint2IsCentroid() );
  if ( mLayer->referencePoint2IsCentroid() )
  {
    spinRefPoint2X->setEnabled( false );
    spinRefPoint2Y->setEnabled( false );
  }
  checkRefPoint2Centroid->blockSignals( false );

  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetY, setValue( mLayer->offset().y() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinAngle, setValue( mLayer->angle() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mStartColorDDBtn, "color", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mEndColorDDBtn, "color2", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mAngleDDBtn, "angle", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mGradientTypeDDBtn, "gradient_type", String, QgsDataDefinedSymbolDialog::gradientTypeHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mCoordinateModeDDBtn, "coordinate_mode", String, QgsDataDefinedSymbolDialog::gradientCoordModeHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mSpreadDDBtn, "spread", Double, QgsDataDefinedSymbolDialog::gradientSpreadHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRefPoint1XDDBtn, "reference1_x", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRefPoint1YDDBtn, "reference1_y", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRefPoint1CentroidDDBtn, "reference1_iscentroid", Int, QgsDataDefinedSymbolDialog::boolHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRefPoint2XDDBtn, "reference2_x", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRefPoint2YDDBtn, "reference2_y", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRefPoint2CentroidDDBtn, "reference2_iscentroid", Int, QgsDataDefinedSymbolDialog::boolHelpText() )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsGradientFillSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2* QgsGradientFillSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsGradientFillSymbolLayerV2Widget::setColor( const QColor& color )
{
  mLayer->setColor( color );
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::setColor2( const QColor& color )
{
  mLayer->setColor2( color );
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::colorModeChanged()
{
  if ( radioTwoColor->isChecked() )
  {
    mLayer->setGradientColorType( QgsGradientFillSymbolLayerV2::SimpleTwoColor );
  }
  else
  {
    mLayer->setGradientColorType( QgsGradientFillSymbolLayerV2::ColorRamp );
  }
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::applyColorRamp()
{
  QgsVectorColorRampV2* ramp = cboGradientColorRamp->currentColorRamp();
  if ( ramp == NULL )
    return;

  mLayer->setColorRamp( ramp );
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::on_mButtonEditRamp_clicked()
{
  if ( mLayer->colorRamp()->type() == "gradient" )
  {
    QgsVectorColorRampV2* ramp = mLayer->colorRamp()->clone();
    QgsVectorGradientColorRampV2* gradRamp = static_cast<QgsVectorGradientColorRampV2*>( ramp );
    QgsVectorGradientColorRampV2Dialog dlg( gradRamp, this );

    if ( dlg.exec() && gradRamp )
    {
      mLayer->setColorRamp( gradRamp );
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientColorRamp, setSourceColorRamp( mLayer->colorRamp() ) )
      emit changed();
    }
    else
    {
      delete ramp;
    }
  }
}

void QgsGradientFillSymbolLayerV2Widget::setGradientType( int index )
{
  switch ( index )
  {
    case 0:
      mLayer->setGradientType( QgsGradientFillSymbolLayerV2::Linear );
      //set sensible default reference points
      spinRefPoint1X->setValue( 0.5 );
      spinRefPoint1Y->setValue( 0 );
      spinRefPoint2X->setValue( 0.5 );
      spinRefPoint2Y->setValue( 1 );
      break;
    case 1:
      mLayer->setGradientType( QgsGradientFillSymbolLayerV2::Radial );
      //set sensible default reference points
      spinRefPoint1X->setValue( 0 );
      spinRefPoint1Y->setValue( 0 );
      spinRefPoint2X->setValue( 1 );
      spinRefPoint2Y->setValue( 1 );
      break;
    case 2:
      mLayer->setGradientType( QgsGradientFillSymbolLayerV2::Conical );
      spinRefPoint1X->setValue( 0.5 );
      spinRefPoint1Y->setValue( 0.5 );
      spinRefPoint2X->setValue( 1 );
      spinRefPoint2Y->setValue( 1 );
      break;
  }
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::setCoordinateMode( int index )
{

  switch ( index )
  {
    case 0:
      //feature coordinate mode
      mLayer->setCoordinateMode( QgsGradientFillSymbolLayerV2::Feature );
      //allow choice of centroid reference positions
      checkRefPoint1Centroid->setEnabled( true );
      checkRefPoint2Centroid->setEnabled( true );
      break;
    case 1:
      //viewport coordinate mode
      mLayer->setCoordinateMode( QgsGradientFillSymbolLayerV2::Viewport );
      //disable choice of centroid reference positions
      checkRefPoint1Centroid->setChecked( Qt::Unchecked );
      checkRefPoint1Centroid->setEnabled( false );
      checkRefPoint2Centroid->setChecked( Qt::Unchecked );
      checkRefPoint2Centroid->setEnabled( false );
      break;
  }

  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::setGradientSpread( int index )
{
  switch ( index )
  {
    case 0:
      mLayer->setGradientSpread( QgsGradientFillSymbolLayerV2::Pad );
      break;
    case 1:
      mLayer->setGradientSpread( QgsGradientFillSymbolLayerV2::Repeat );
      break;
    case 2:
      mLayer->setGradientSpread( QgsGradientFillSymbolLayerV2::Reflect );
      break;
  }

  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::offsetChanged()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::referencePointChanged()
{
  mLayer->setReferencePoint1( QPointF( spinRefPoint1X->value(), spinRefPoint1Y->value() ) );
  mLayer->setReferencePoint1IsCentroid( checkRefPoint1Centroid->isChecked() );
  mLayer->setReferencePoint2( QPointF( spinRefPoint2X->value(), spinRefPoint2Y->value() ) );
  mLayer->setReferencePoint2IsCentroid( checkRefPoint2Centroid->isChecked() );
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::on_mSpinAngle_valueChanged( double value )
{
  mLayer->setAngle( value );
  emit changed();
}

void QgsGradientFillSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

///////////

QgsShapeburstFillSymbolLayerV2Widget::QgsShapeburstFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mDistanceUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  btnChangeColor->setAllowAlpha( true );
  btnChangeColor->setColorDialogTitle( tr( "Select gradient color" ) );
  btnChangeColor->setContext( "symbology" );
  btnChangeColor->setShowNoColor( true );
  btnChangeColor->setNoColorString( tr( "Transparent" ) );
  btnChangeColor2->setAllowAlpha( true );
  btnChangeColor2->setColorDialogTitle( tr( "Select gradient color" ) );
  btnChangeColor2->setContext( "symbology" );
  btnChangeColor2->setShowNoColor( true );
  btnChangeColor2->setNoColorString( tr( "Transparent" ) );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  cboGradientColorRamp->setShowGradientOnly( true );
  cboGradientColorRamp->populate( QgsStyleV2::defaultStyle() );

  connect( cboGradientColorRamp, SIGNAL( currentIndexChanged( int ) ), this, SLOT( applyColorRamp() ) );
  connect( btnChangeColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor( const QColor& ) ) );
  connect( btnChangeColor2, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor2( const QColor& ) ) );
  connect( radioTwoColor, SIGNAL( toggled( bool ) ), this, SLOT( colorModeChanged() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );

  connect( mBlurSlider, SIGNAL( valueChanged( int ) ), mSpinBlurRadius, SLOT( setValue( int ) ) );
  connect( mSpinBlurRadius, SIGNAL( valueChanged( int ) ), mBlurSlider, SLOT( setValue( int ) ) );
}

void QgsShapeburstFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "ShapeburstFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsShapeburstFillSymbolLayerV2*>( layer );

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColor, setColor( mLayer->color() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( btnChangeColor2, setColor( mLayer->color2() ) )

  if ( mLayer->colorType() == QgsShapeburstFillSymbolLayerV2::SimpleTwoColor )
  {
    radioTwoColor->setChecked( true );
    cboGradientColorRamp->setEnabled( false );
  }
  else
  {
    radioColorRamp->setChecked( true );
    btnChangeColor->setEnabled( false );
    btnChangeColor2->setEnabled( false );
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinBlurRadius, setValue( mLayer->blurRadius() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mBlurSlider, setValue( mLayer->blurRadius() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinMaxDistance, setValue( mLayer->maxDistance() ) )

  if ( mLayer->useWholeShape() )
  {
    QGS_BLOCK_SIGNAL_DURING_CALL( mRadioUseWholeShape, setChecked( true ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mSpinMaxDistance, setEnabled( false ) )
    mDistanceUnitWidget->setEnabled( false );
    mShadeDistanceDDBtn->setEnabled( false );
  }
  else
  {
    QGS_BLOCK_SIGNAL_DURING_CALL( mRadioUseMaxDistance, setChecked( true ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mSpinMaxDistance, setEnabled( true ) )
    mDistanceUnitWidget->setEnabled( true );
    mShadeDistanceDDBtn->setEnabled( true );
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( mDistanceUnitWidget, setUnit( mLayer->distanceUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mDistanceUnitWidget, setMapUnitScale( mLayer->distanceMapUnitScale() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mIgnoreRingsCheckBox, setCheckState( mLayer->ignoreRings() ? Qt::Checked : Qt::Unchecked ) )

  // set source color ramp
  if ( mLayer->colorRamp() )
    QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientColorRamp, setSourceColorRamp( mLayer->colorRamp() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetY, setValue( mLayer->offset().y() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mStartColorDDBtn, "color", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mEndColorDDBtn, "color2", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBlurRadiusDDBtn, "blur_radius", Int, tr( "Integer between 0 and 18" ) )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mShadeWholeShapeDDBtn, "use_whole_shape", Int, QgsDataDefinedSymbolDialog::boolHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mShadeDistanceDDBtn, "max_distance", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mIgnoreRingsDDBtn, "ignore_rings", Int, QgsDataDefinedSymbolDialog::boolHelpText() )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2* QgsShapeburstFillSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsShapeburstFillSymbolLayerV2Widget::setColor( const QColor& color )
{
  if ( mLayer )
  {
    mLayer->setColor( color );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::setColor2( const QColor& color )
{
  if ( mLayer )
  {
    mLayer->setColor2( color );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::colorModeChanged()
{
  if ( !mLayer )
  {
    return;
  }

  if ( radioTwoColor->isChecked() )
  {
    mLayer->setColorType( QgsShapeburstFillSymbolLayerV2::SimpleTwoColor );
  }
  else
  {
    mLayer->setColorType( QgsShapeburstFillSymbolLayerV2::ColorRamp );
  }
  emit changed();
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mSpinBlurRadius_valueChanged( int value )
{
  if ( mLayer )
  {
    mLayer->setBlurRadius( value );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mSpinMaxDistance_valueChanged( double value )
{
  if ( mLayer )
  {
    mLayer->setMaxDistance( value );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mDistanceUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mDistanceUnitWidget->getUnit() );
    mLayer->setDistanceUnit( unit );
    mLayer->setDistanceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mRadioUseWholeShape_toggled( bool value )
{
  if ( mLayer )
  {
    mLayer->setUseWholeShape( value );
    mDistanceUnitWidget->setEnabled( !value );
    mShadeDistanceDDBtn->setEnabled( !value );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::applyColorRamp()
{
  QgsVectorColorRampV2* ramp = cboGradientColorRamp->currentColorRamp();
  if ( ramp == NULL )
    return;

  mLayer->setColorRamp( ramp );
  emit changed();
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mButtonEditRamp_clicked()
{
  if ( mLayer->colorRamp()->type() == "gradient" )
  {
    QgsVectorColorRampV2* ramp = mLayer->colorRamp()->clone();
    QgsVectorGradientColorRampV2* gradRamp = static_cast<QgsVectorGradientColorRampV2*>( ramp );
    QgsVectorGradientColorRampV2Dialog dlg( gradRamp, this );

    if ( dlg.exec() && gradRamp )
    {
      mLayer->setColorRamp( gradRamp );
      QGS_BLOCK_SIGNAL_DURING_CALL( cboGradientColorRamp, setSourceColorRamp( mLayer->colorRamp() ) )
      emit changed();
    }
    else
    {
      delete ramp;
    }
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::offsetChanged()
{
  if ( mLayer )
  {
    mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::on_mIgnoreRingsCheckBox_stateChanged( int state )
{
  bool checked = ( state == Qt::Checked );
  mLayer->setIgnoreRings( checked );
  emit changed();
}

///////////

QgsMarkerLineSymbolLayerV2Widget::QgsMarkerLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mIntervalUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetAlongLineUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  spinOffset->setClearValue( 0.0 );

  connect( spinInterval, SIGNAL( valueChanged( double ) ), this, SLOT( setInterval( double ) ) );
  connect( mSpinOffsetAlongLine, SIGNAL( valueChanged( double ) ), this, SLOT( setOffsetAlongLine( double ) ) );
  connect( chkRotateMarker, SIGNAL( clicked() ), this, SLOT( setRotate() ) );
  connect( spinOffset, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( radInterval, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radVertex, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radVertexLast, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radVertexFirst, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radCentralPoint, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
}

void QgsMarkerLineSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "MarkerLine" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsMarkerLineSymbolLayerV2*>( layer );

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( spinInterval, setValue( mLayer->interval() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinOffsetAlongLine, setValue( mLayer->offsetAlongLine() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( chkRotateMarker, setChecked( mLayer->rotateMarker() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffset, setValue( mLayer->offset() ) )

  if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::Interval )
    radInterval->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::Vertex )
    radVertex->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::LastVertex )
    radVertexLast->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::CentralPoint )
    radCentralPoint->setChecked( true );
  else
    radVertexFirst->setChecked( true );

  // set units
  QGS_BLOCK_SIGNAL_DURING_CALL( mIntervalUnitWidget, setUnit( mLayer->intervalUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mIntervalUnitWidget, setMapUnitScale( mLayer->intervalMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetAlongLineUnitWidget, setUnit( mLayer->offsetAlongLineUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetAlongLineUnitWidget, setMapUnitScale( mLayer->offsetAlongLineMapUnitScale() ) )

  setPlacement(); // update gui

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mIntervalDDBtn, "interval", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mLineOffsetDDBtn, "offset", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mPlacementDDBtn, "placement", String, "'vertex'|'lastvertex'|'firstvertex'|'centerpoint'")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mOffsetAlongLineDDBtn, "offset_along_line", Double, "")
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsMarkerLineSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsMarkerLineSymbolLayerV2Widget::setInterval( double val )
{
  mLayer->setInterval( val );
  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setOffsetAlongLine( double val )
{
  mLayer->setOffsetAlongLine( val );
  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setRotate()
{
  mLayer->setRotateMarker( chkRotateMarker->isChecked() );
  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( spinOffset->value() );
  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setPlacement()
{
  bool interval = radInterval->isChecked();
  spinInterval->setEnabled( interval );
  mSpinOffsetAlongLine->setEnabled( radInterval->isChecked() || radVertexLast->isChecked() || radVertexFirst->isChecked() );
  //mLayer->setPlacement( interval ? QgsMarkerLineSymbolLayerV2::Interval : QgsMarkerLineSymbolLayerV2::Vertex );
  if ( radInterval->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::Interval );
  else if ( radVertex->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::Vertex );
  else if ( radVertexLast->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::LastVertex );
  else if ( radVertexFirst->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::FirstVertex );
  else
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::CentralPoint );

  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::on_mIntervalUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mIntervalUnitWidget->getUnit() );
    mLayer->setIntervalUnit( unit );
    mLayer->setIntervalMapUnitScale( mIntervalUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsMarkerLineSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsMarkerLineSymbolLayerV2Widget::on_mOffsetAlongLineUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetAlongLineUnitWidget->getUnit() );
    mLayer->setOffsetAlongLineUnit( unit );
    mLayer->setOffsetAlongLineMapUnitScale( mOffsetAlongLineUnitWidget->getMapUnitScale() );
  }
  emit changed();
}

///////////


QgsSvgMarkerSymbolLayerV2Widget::QgsSvgMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mSizeUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mBorderWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  viewGroups->setHeaderHidden( true );

  mChangeColorButton->setColorDialogTitle( tr( "Select fill color" ) );
  mChangeColorButton->setContext( "symbology" );
  mChangeBorderColorButton->setColorDialogTitle( tr( "Select border color" ) );
  mChangeColorButton->setContext( "symbology" );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  populateList();

  connect( viewImages->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setName( const QModelIndex& ) ) );
  connect( viewGroups->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( populateIcons( const QModelIndex& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
}

#include <QTime>
#include <QAbstractListModel>
#include <QPixmapCache>
#include <QStyle>

class QgsSvgListModel : public QAbstractListModel
{
  public:
    QgsSvgListModel( QObject* parent ) : QAbstractListModel( parent )
    {
      mSvgFiles = QgsSymbolLayerV2Utils::listSvgFiles();
    }

    // Constructor to create model for icons in a specific path
    QgsSvgListModel( QObject* parent, QString path ) : QAbstractListModel( parent )
    {
      mSvgFiles = QgsSymbolLayerV2Utils::listSvgFilesAt( path );
    }

    int rowCount( const QModelIndex & parent = QModelIndex() ) const OVERRIDE
    {
      Q_UNUSED( parent );
      return mSvgFiles.count();
  }

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const OVERRIDE
    {
      QString entry = mSvgFiles.at( index.row() );

      if ( role == Qt::DecorationRole ) // icon
  {
    QPixmap pixmap;
    if ( !QPixmapCache::find( entry, pixmap ) )
      {
        // render SVG file
        QColor fill, outline;
        double outlineWidth;
        bool fillParam, outlineParam, outlineWidthParam;
        QgsSvgCache::instance()->containsParams( entry, fillParam, fill, outlineParam, outline, outlineWidthParam, outlineWidth );

        bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
        const QImage& img = QgsSvgCache::instance()->svgAsImage( entry, 30.0, fill, outline, outlineWidth, 3.5 /*appr. 88 dpi*/, 1.0, fitsInCache );
        pixmap = QPixmap::fromImage( img );
        QPixmapCache::insert( entry, pixmap );
      }

      return pixmap;
    }
    else if ( role == Qt::UserRole || role == Qt::ToolTipRole )
  {
    return entry;
  }

  return QVariant();
}

protected:
  QStringList mSvgFiles;
};

class QgsSvgGroupsModel : public QStandardItemModel
{
public:
  QgsSvgGroupsModel( QObject* parent ) : QStandardItemModel( parent )
    {
      QStringList svgPaths = QgsApplication::svgPaths();
      QStandardItem *parentItem = invisibleRootItem();

      for ( int i = 0; i < svgPaths.size(); i++ )
      {
        QDir dir( svgPaths[i] );
        QStandardItem *baseGroup;

        if ( dir.path().contains( QgsApplication::pkgDataPath() ) )
        {
          baseGroup = new QStandardItem( QString( "App Symbols" ) );
        }
        else if ( dir.path().contains( QgsApplication::qgisSettingsDirPath() ) )
        {
          baseGroup = new QStandardItem( QString( "User Symbols" ) );
        }
        else
        {
          baseGroup = new QStandardItem( dir.dirName() );
        }
        baseGroup->setData( QVariant( svgPaths[i] ) );
        baseGroup->setEditable( false );
        baseGroup->setCheckable( false );
        baseGroup->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
        baseGroup->setToolTip( dir.path() );
        parentItem->appendRow( baseGroup );
        createTree( baseGroup );
        QgsDebugMsg( QString( "SVG base path %1: %2" ).arg( i ).arg( baseGroup->data().toString() ) );
      }
    }
  private:
    void createTree( QStandardItem* &parentGroup )
    {
      QDir parentDir( parentGroup->data().toString() );
      foreach ( QString item, parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
      {
        QStandardItem* group = new QStandardItem( item );
        group->setData( QVariant( parentDir.path() + "/" + item ) );
        group->setEditable( false );
        group->setCheckable( false );
        group->setToolTip( parentDir.path() + "/" + item );
        group->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
        parentGroup->appendRow( group );
        createTree( group );
      }
    }
};

void QgsSvgMarkerSymbolLayerV2Widget::populateList()
{
  QgsSvgGroupsModel* g = new QgsSvgGroupsModel( viewGroups );
  viewGroups->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    viewGroups->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  // Initally load the icons in the List view without any grouping
  QgsSvgListModel* m = new QgsSvgListModel( viewImages );
  viewImages->setModel( m );
}

void QgsSvgMarkerSymbolLayerV2Widget::populateIcons( const QModelIndex& idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QgsSvgListModel* m = new QgsSvgListModel( viewImages, path );
  viewImages->setModel( m );

  connect( viewImages->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setName( const QModelIndex& ) ) );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setGuiForSvg( const QgsSvgMarkerSymbolLayerV2* layer )
{
  if ( !layer )
  {
    return;
  }

  //activate gui for svg parameters only if supported by the svg file
  bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth;
  QgsSvgCache::instance()->containsParams( layer->path(), hasFillParam, defaultFill, hasOutlineParam, defaultOutline, hasOutlineWidthParam, defaultOutlineWidth );
  mChangeColorButton->setEnabled( hasFillParam );
  mFillColorDDBtn->setEnabled( hasFillParam );
  mChangeBorderColorButton->setEnabled( hasOutlineParam );
  mBorderColorDDBtn->setEnabled( hasOutlineParam );
  mBorderWidthSpinBox->setEnabled( hasOutlineWidthParam );
  mBorderWidthDDBtn->setEnabled( hasOutlineWidthParam );

  if ( hasFillParam )
  {
    if ( layer->fillColor().isValid() )
    {
      mChangeColorButton->setColor( layer->fillColor() );
    }
    else
    {
      mChangeColorButton->setColor( defaultFill );
    }
  }
  if ( hasOutlineParam )
  {
    if ( layer->outlineColor().isValid() )
    {
      mChangeBorderColorButton->setColor( layer->outlineColor() );
    }
    else
    {
      mChangeBorderColorButton->setColor( defaultOutline );
    }
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( mFileLineEdit, setText( layer->path() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mBorderWidthSpinBox, setValue( layer->outlineWidth() ) )
}


void QgsSvgMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer )
  {
    return;
  }

  if ( layer->layerType() != "SvgMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSvgMarkerSymbolLayerV2*>( layer );

  // set values

  QAbstractItemModel* m = viewImages->model();
  QItemSelectionModel* selModel = viewImages->selectionModel();
  for ( int i = 0; i < m->rowCount(); i++ )
  {
    QModelIndex idx( m->index( i, 0 ) );
    if ( m->data( idx ).toString() == mLayer->path() )
    {
      selModel->select( idx, QItemSelectionModel::SelectCurrent );
      selModel->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
      setName( idx );
      break;
    }
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( spinSize, setValue( mLayer->size() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinAngle, setValue( mLayer->angle() ) )

  // without blocking signals the value gets changed because of slot setOffset()
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetY, setValue( mLayer->offset().y() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mSizeUnitWidget, setUnit( mLayer->sizeUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSizeUnitWidget, setMapUnitScale( mLayer->sizeMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mBorderWidthUnitWidget, setUnit( mLayer->outlineWidthUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mBorderWidthUnitWidget, setMapUnitScale( mLayer->outlineWidthMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )

  //anchor points
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalAnchorComboBox, setCurrentIndex( mLayer->horizontalAnchorPoint() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalAnchorComboBox, setCurrentIndex( mLayer->verticalAnchorPoint() ) )

  setGuiForSvg( mLayer );

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mSizeDDBtn, "size", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderWidthDDBtn, "outline-width", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mAngleDDBtn, "angle", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mOffsetDDBtn, "offset", String, QgsDataDefinedSymbolDialog::offsetHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mFilenameDDBtn, "name", String, QgsDataDefinedSymbolDialog::fileNameHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mFillColorDDBtn, "fill", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderColorDDBtn, "outline", String, QgsDataDefinedSymbolDialog::colorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mHorizontalAnchorDDBtn, "horizontal_anchor_point", String, QgsDataDefinedSymbolDialog::horizontalAnchorHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mVerticalAnchorDDBtn, "vertical_anchor_point", String, QgsDataDefinedSymbolDialog::verticalAnchorHelpText() )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSvgMarkerSymbolLayerV2Widget::setName( const QModelIndex& idx )
{
  QString name = idx.data( Qt::UserRole ).toString();
  mLayer->setPath( name );
  mFileLineEdit->setText( name );

  setGuiForSvg( mLayer );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setSize()
{
  mLayer->setSize( spinSize->value() );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setAngle()
{
  mLayer->setAngle( spinAngle->value() );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mFileToolButton_clicked()
{
  QSettings s;
  QString file = QFileDialog::getOpenFileName( 0,
                 tr( "Select SVG file" ),
                 s.value( "/UI/lastSVGMarkerDir" ).toString(),
                 tr( "SVG files" ) + " (*.svg)" );
  QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() )
  {
    return;
  }
  mFileLineEdit->setText( file );
  mLayer->setPath( file );
  s.setValue( "/UI/lastSVGMarkerDir", fi.absolutePath() );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mFileLineEdit_textEdited( const QString& text )
{
  if ( !QFileInfo( text ).exists() )
  {
    return;
  }
  mLayer->setPath( text );
  setGuiForSvg( mLayer );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mFileLineEdit_editingFinished()
{
  if ( !QFileInfo( mFileLineEdit->text() ).exists() )
  {
    QUrl url( mFileLineEdit->text() );
    if ( !url.isValid() )
    {
      return;
    }
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  mLayer->setPath( mFileLineEdit->text() );
  QApplication::restoreOverrideCursor();

  setGuiForSvg( mLayer );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mChangeColorButton_colorChanged( const QColor& color )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setFillColor( color );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mChangeBorderColorButton_colorChanged( const QColor& color )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setOutlineColor( color );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mBorderWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mSizeUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSizeUnitWidget->getUnit() );
    mLayer->setSizeUnit( unit );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mBorderWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mBorderWidthUnitWidget->getUnit() );
    mLayer->setOutlineWidthUnit( unit );
    mLayer->setOutlineWidthMapUnitScale( mBorderWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( index ) );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( index ) );
    emit changed();
  }
}

/////////////

#include <QFileDialog>

QgsSVGFillSymbolLayerWidget::QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = 0;
  setupUi( this );
  mTextureWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mSvgOutlineWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mSvgTreeView->setHeaderHidden( true );
  insertIcons();

  mChangeColorButton->setColorDialogTitle( tr( "Select fill color" ) );
  mChangeColorButton->setContext( "symbology" );
  mChangeBorderColorButton->setColorDialogTitle( tr( "Select border color" ) );
  mChangeBorderColorButton->setContext( "symbology" );

  connect( mSvgListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setFile( const QModelIndex& ) ) );
  connect( mSvgTreeView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( populateIcons( const QModelIndex& ) ) );
}

void QgsSVGFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer )
  {
    return;
  }

  if ( layer->layerType() != "SVGFill" )
  {
    return;
  }

  mLayer = dynamic_cast<QgsSVGFillSymbolLayer*>( layer );
  if ( mLayer )
  {
    QGS_BLOCK_SIGNAL_DURING_CALL( mTextureWidthSpinBox, setValue( mLayer->patternWidth() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mSVGLineEdit, setText( mLayer->svgFilePath() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mRotationSpinBox, setValue( mLayer->angle() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mTextureWidthUnitWidget, setUnit( mLayer->patternWidthUnit() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mTextureWidthUnitWidget, setMapUnitScale( mLayer->patternWidthMapUnitScale() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mSvgOutlineWidthUnitWidget, setUnit( mLayer->svgOutlineWidthUnit() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mSvgOutlineWidthUnitWidget, setMapUnitScale( mLayer->svgOutlineWidthMapUnitScale() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mChangeColorButton, setColor( mLayer->svgFillColor() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mChangeBorderColorButton, setColor( mLayer->svgOutlineColor() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mBorderWidthSpinBox, setValue( mLayer->svgOutlineWidth() ) )
  }
  updateParamGui( false );

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mTextureWidthDDBtn, "width", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mSVGDDBtn, "svgFile", String, QgsDataDefinedSymbolDialog::fileNameHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRotationDDBtn, "angle", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mFilColorDDBtn, "svgFillColor", String, QgsDataDefinedSymbolDialog::colorHelpText())
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderColorDDBtn, "svgOutlineColor", String, QgsDataDefinedSymbolDialog::colorHelpText())
  QGS_REGISTER_DATA_DEFINED_BUTTON( mBorderWidthDDBtn, "svgOutlineWidth", Double, "")
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsSVGFillSymbolLayerWidget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}


QgsSymbolLayerV2* QgsSVGFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsSVGFillSymbolLayerWidget::on_mBrowseToolButton_clicked()
{
  QString filePath = QFileDialog::getOpenFileName( 0, tr( "Select SVG texture file" ), QString(), tr( "SVG file" ) + " (*.svg);;" + tr( "All files" ) + " (*.*)" );
  if ( !filePath.isNull() )
  {
    mSVGLineEdit->setText( filePath );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mTextureWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setPatternWidth( d );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mSVGLineEdit_textEdited( const QString & text )
{
  if ( !mLayer )
  {
    return;
  }

  QFileInfo fi( text );
  if ( !fi.exists() )
  {
    return;
  }
  mLayer->setSvgFilePath( text );
  updateParamGui();
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::on_mSVGLineEdit_editingFinished()
{
  if ( !mLayer )
  {
    return;
  }

  QFileInfo fi( mSVGLineEdit->text() );
  if ( !fi.exists() )
  {
    QUrl url( mSVGLineEdit->text() );
    if ( !url.isValid() )
    {
      return;
    }
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  mLayer->setSvgFilePath( mSVGLineEdit->text() );
  QApplication::restoreOverrideCursor();

  updateParamGui();
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::setFile( const QModelIndex& item )
{
  QString file = item.data( Qt::UserRole ).toString();
  mLayer->setSvgFilePath( file );
  mSVGLineEdit->setText( file );

  updateParamGui();
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::insertIcons()
{
  QgsSvgGroupsModel* g = new QgsSvgGroupsModel( mSvgTreeView );
  mSvgTreeView->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    mSvgTreeView->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  QgsSvgListModel* m = new QgsSvgListModel( mSvgListView );
  mSvgListView->setModel( m );
}

void QgsSVGFillSymbolLayerWidget::populateIcons( const QModelIndex& idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QgsSvgListModel* m = new QgsSvgListModel( mSvgListView, path );
  mSvgListView->setModel( m );

  connect( mSvgListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setFile( const QModelIndex& ) ) );
  emit changed();
}


void QgsSVGFillSymbolLayerWidget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::updateParamGui( bool resetValues )
{
  //activate gui for svg parameters only if supported by the svg file
  bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth;
  QgsSvgCache::instance()->containsParams( mSVGLineEdit->text(), hasFillParam, defaultFill, hasOutlineParam, defaultOutline, hasOutlineWidthParam, defaultOutlineWidth );
  if ( hasFillParam && resetValues )
  {
    mChangeColorButton->setColor( defaultFill );
  }
  mChangeColorButton->setEnabled( hasFillParam );
  mFilColorDDBtn->setEnabled( hasFillParam );
  if ( hasOutlineParam && resetValues )
  {
    mChangeBorderColorButton->setColor( defaultOutline );
  }
  mChangeBorderColorButton->setEnabled( hasOutlineParam );
  mBorderColorDDBtn->setEnabled( hasOutlineParam );
  if ( hasOutlineWidthParam && resetValues )
  {
    mBorderWidthSpinBox->setValue( defaultOutlineWidth );
  }
  mBorderWidthSpinBox->setEnabled( hasOutlineWidthParam );
  mBorderWidthDDBtn->setEnabled( hasOutlineWidthParam );
}

void QgsSVGFillSymbolLayerWidget::on_mChangeColorButton_colorChanged( const QColor& color )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setSvgFillColor( color );
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::on_mChangeBorderColorButton_colorChanged( const QColor& color )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setSvgOutlineColor( color );
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::on_mBorderWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSvgOutlineWidth( d );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mTextureWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mTextureWidthUnitWidget->getUnit() );
    mLayer->setPatternWidthUnit( unit );
    mLayer->setPatternWidthMapUnitScale( mTextureWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mSvgOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSvgOutlineWidthUnitWidget->getUnit() );
    mLayer->setSvgOutlineWidthUnit( unit );
    mLayer->setSvgOutlineWidthMapUnitScale( mSvgOutlineWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

/////////////

QgsLinePatternFillSymbolLayerWidget::QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ):
    QgsSymbolLayerV2Widget( parent, vl ), mLayer( 0 )
{
  setupUi( this );
  mDistanceUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
}

void QgsLinePatternFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "LinePatternFill" )
  {
    return;
  }

  QgsLinePatternFillSymbolLayer* patternLayer = static_cast<QgsLinePatternFillSymbolLayer*>( layer );
  if ( patternLayer )
  {
    mLayer = patternLayer;
    QGS_BLOCK_SIGNAL_DURING_CALL( mAngleSpinBox, setValue( mLayer->lineAngle() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mDistanceSpinBox, setValue( mLayer->distance() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetSpinBox, setValue( mLayer->offset() ) )

    //units
    QGS_BLOCK_SIGNAL_DURING_CALL( mDistanceUnitWidget, setUnit( mLayer->distanceUnit() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mDistanceUnitWidget, setMapUnitScale( mLayer->distanceMapUnitScale() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
    QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )
  }

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mAngleDDBtn, "lineangle", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mDistanceDDBtn, "distance", Double, "")
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsLinePatternFillSymbolLayerWidget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}


QgsSymbolLayerV2* QgsLinePatternFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsLinePatternFillSymbolLayerWidget::on_mAngleSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setLineAngle( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mDistanceSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDistance( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mOffsetSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOffset( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mDistanceUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mDistanceUnitWidget->getUnit() );
    mLayer->setDistanceUnit( unit );
    mLayer->setDistanceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}



/////////////

QgsPointPatternFillSymbolLayerWidget::QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ):
    QgsSymbolLayerV2Widget( parent, vl ), mLayer( 0 )
{
  setupUi( this );
  mHorizontalDistanceUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mVerticalDistanceUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mHorizontalDisplacementUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mVerticalDisplacementUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
}


void QgsPointPatternFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "PointPatternFill" )
  {
    return;
  }

  mLayer = static_cast<QgsPointPatternFillSymbolLayer*>( layer );
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalDistanceSpinBox, setValue( mLayer->distanceX() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalDistanceSpinBox, setValue( mLayer->distanceY() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalDisplacementSpinBox, setValue( mLayer->displacementX() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalDisplacementSpinBox, setValue( mLayer->displacementY() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalDistanceUnitWidget, setUnit( mLayer->distanceXUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalDistanceUnitWidget, setMapUnitScale( mLayer->distanceXMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalDistanceUnitWidget, setUnit( mLayer->distanceYUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalDistanceUnitWidget, setMapUnitScale( mLayer->distanceYMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalDisplacementUnitWidget, setUnit( mLayer->displacementXUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalDisplacementUnitWidget, setMapUnitScale( mLayer->displacementXMapUnitScale() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalDisplacementUnitWidget, setUnit( mLayer->displacementYUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalDisplacementUnitWidget, setMapUnitScale( mLayer->displacementYMapUnitScale() ) )

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mHorizontalDistanceDDBtn, "distance_x", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mVerticalDistanceDDBtn, "distance_y", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mHorizontalDisplacementDDBtn, "displacement_x", Double, "")
  QGS_REGISTER_DATA_DEFINED_BUTTON( mVerticalDisplacementDDBtn, "displacement_y", Double, "")
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsPointPatternFillSymbolLayerWidget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}


QgsSymbolLayerV2* QgsPointPatternFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDistanceSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDistanceX( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDistanceSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDistanceY( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDisplacementSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDisplacementX( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDisplacementSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDisplacementY( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDistanceUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mHorizontalDistanceUnitWidget->getUnit() );
    mLayer->setDistanceXUnit( unit );
    mLayer->setDistanceXMapUnitScale( mHorizontalDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDistanceUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mVerticalDistanceUnitWidget->getUnit() );
    mLayer->setDistanceYUnit( unit );
    mLayer->setDistanceYMapUnitScale( mVerticalDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDisplacementUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mHorizontalDisplacementUnitWidget->getUnit() );
    mLayer->setDisplacementXUnit( unit );
    mLayer->setDisplacementXMapUnitScale( mHorizontalDisplacementUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDisplacementUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mVerticalDisplacementUnitWidget->getUnit() );
    mLayer->setDisplacementYUnit( unit );
    mLayer->setDisplacementYMapUnitScale( mVerticalDisplacementUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

/////////////

QgsFontMarkerSymbolLayerV2Widget::QgsFontMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  mSizeUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  widgetChar = new CharacterWidget;
  scrollArea->setWidget( widgetChar );

  btnColor->setAllowAlpha( true );
  btnColor->setColorDialogTitle( tr( "Select symbol color" ) );
  btnColor->setContext( "symbology" );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  connect( cboFont, SIGNAL( currentFontChanged( const QFont & ) ), this, SLOT( setFontFamily( const QFont& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize( double ) ) );
  connect( btnColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor( const QColor& ) ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle( double ) ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( widgetChar, SIGNAL( characterSelected( const QChar & ) ), this, SLOT( setCharacter( const QChar & ) ) );
}


void QgsFontMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "FontMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsFontMarkerSymbolLayerV2*>( layer );

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( cboFont, setCurrentFont( QFont( mLayer->fontFamily() ) ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinSize, setValue( mLayer->size() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( btnColor, setColor( mLayer->color() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinAngle, setValue( mLayer->angle() ) )

  //block
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( spinOffsetY, setValue( mLayer->offset().y() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mSizeUnitWidget, setUnit( mLayer->sizeUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSizeUnitWidget, setMapUnitScale( mLayer->sizeMapUnitScale() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )

  //anchor points
  QGS_BLOCK_SIGNAL_DURING_CALL( mHorizontalAnchorComboBox, setCurrentIndex( mLayer->horizontalAnchorPoint() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mVerticalAnchorComboBox, setCurrentIndex( mLayer->verticalAnchorPoint() ) )
}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsFontMarkerSymbolLayerV2Widget::setFontFamily( const QFont& font )
{
  mLayer->setFontFamily( font.family() );
  widgetChar->updateFont( font );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setColor( const QColor& color )
{
  mLayer->setColor( color );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setSize( double size )
{
  mLayer->setSize( size );
  //widgetChar->updateSize(size);
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setAngle( double angle )
{
  mLayer->setAngle( angle );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setCharacter( const QChar& chr )
{
  mLayer->setCharacter( chr );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::on_mSizeUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSizeUnitWidget->getUnit() );
    mLayer->setSizeUnit( unit );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsFontMarkerSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsFontMarkerSymbolLayerV2Widget::on_mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( index ) );
    emit changed();
  }
}

void QgsFontMarkerSymbolLayerV2Widget::on_mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( index ) );
    emit changed();
  }
}


///////////////


QgsCentroidFillSymbolLayerV2Widget::QgsCentroidFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
}

void QgsCentroidFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "CentroidFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsCentroidFillSymbolLayerV2*>( layer );

  // set values
  QGS_BLOCK_SIGNAL_DURING_CALL( mDrawInsideCheckBox, setChecked( mLayer->pointOnSurface() ) )
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsCentroidFillSymbolLayerV2Widget::on_mDrawInsideCheckBox_stateChanged( int state )
{
  mLayer->setPointOnSurface( state == Qt::Checked );
  emit changed();
}

///////////////

QgsRasterFillSymbolLayerWidget::QgsRasterFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = 0;
  setupUi( this );

  mWidthUnitWidget->setUnits( QStringList() << tr( "Pixels" ) << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  mSpinOffsetX->setClearValue( 0.0 );
  mSpinOffsetY->setClearValue( 0.0 );

  connect( cboCoordinateMode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setCoordinateMode( int ) ) );
  connect( mSpinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( mSpinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );

}

void QgsRasterFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2 *layer )
{
  if ( !layer )
  {
    return;
  }

  if ( layer->layerType() != "RasterFill" )
  {
    return;
  }

  mLayer = dynamic_cast<QgsRasterFillSymbolLayer*>( layer );
  if ( !mLayer )
  {
    return;
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( mImageLineEdit, setText( mLayer->imageFilePath() ) )

  switch ( mLayer->coordinateMode() )
  {
    case QgsRasterFillSymbolLayer::Viewport:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboCoordinateMode, setCurrentIndex( 1 ) )
      break;
    case QgsRasterFillSymbolLayer::Feature:
    default:
      QGS_BLOCK_SIGNAL_DURING_CALL( cboCoordinateMode, setCurrentIndex( 0 ) )
      break;
  }

  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinTransparency, setValue( mLayer->alpha() * 100.0 ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSliderTransparency, setValue( mLayer->alpha() * 100.0 ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mRotationSpinBox, setValue( mLayer->angle() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinOffsetX, setValue( mLayer->offset().x() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mSpinOffsetY, setValue( mLayer->offset().y() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setUnit( mLayer->offsetUnit() ) )
  QGS_BLOCK_SIGNAL_DURING_CALL( mOffsetUnitWidget, setMapUnitScale( mLayer->offsetMapUnitScale() ) )

  QGS_BLOCK_SIGNAL_DURING_CALL( mWidthSpinBox, setValue( mLayer->width() ) )

  switch ( mLayer->widthUnit() )
  {
    case QgsSymbolV2::MM:
      QGS_BLOCK_SIGNAL_DURING_CALL( mWidthUnitWidget, setUnit( 1 ) )
      break;
    case QgsSymbolV2::MapUnit:
      QGS_BLOCK_SIGNAL_DURING_CALL( mWidthUnitWidget, setUnit( 2 ) )
      break;
    case QgsSymbolV2::Pixel:
    default:
      QGS_BLOCK_SIGNAL_DURING_CALL( mWidthUnitWidget, setUnit( 0 ) )
      break;
  }
  QGS_BLOCK_SIGNAL_DURING_CALL( mWidthUnitWidget, setMapUnitScale( mLayer->widthMapUnitScale() ) )
  updatePreviewImage();

  mDataDefinedPropertyButtons.clear();

#define QGS_REGISTER_DATA_DEFINED_BUTTON( button, name, type, description )\
  {\
    QgsDataDefined dd( mLayer->dataDefinedProperty( name ) );\
    button->init( mVectorLayer, &dd, QgsDataDefinedButton::type, Qt::escape( description ) );\
    mDataDefinedPropertyButtons.insert( name, button );\
  }
  QGS_REGISTER_DATA_DEFINED_BUTTON( mFilenameDDBtn, "file", String, QgsDataDefinedSymbolDialog::fileNameHelpText() )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mOpacityDDBtn, "alpha", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mRotationDDBtn, "angle", Double, "" )
  QGS_REGISTER_DATA_DEFINED_BUTTON( mWidthDDBtn, "width", Double, "" )
#undef QGS_REGISTER_DATA_DEFINED_BUTTON

  for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
          = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
  {
    connect( it.value(), SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
    connect( it.value(), SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  }
}

void QgsRasterFillSymbolLayerWidget::updateDataDefinedProperty()
{
    mLayer->removeDataDefinedProperties();
    for (QMap< QString, QgsDataDefinedButton* >::const_iterator it 
            = mDataDefinedPropertyButtons.begin(); it != mDataDefinedPropertyButtons.end(); ++it )
    {
        if ( it.value()->isActive() ) 
            mLayer->setDataDefinedProperty( it.key(), it.value()->currentDefinition() );
    }
    emit changed();
}

QgsSymbolLayerV2 *QgsRasterFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsRasterFillSymbolLayerWidget::on_mBrowseToolButton_clicked()
{
  QSettings s;
  QString openDir;
  QString lineEditText = mImageLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( "/UI/lastRasterFillImageDir", "" ).toString();
  }

  //show file dialog
  QString filePath = QFileDialog::getOpenFileName( 0, tr( "Select image file" ), openDir );
  if ( !filePath.isNull() )
  {
    //check if file exists
    QFileInfo fileInfo( filePath );
    if ( !fileInfo.exists() || !fileInfo.isReadable() )
    {
      QMessageBox::critical( 0, "Invalid file", "Error, file does not exist or is not readable" );
      return;
    }

    s.setValue( "/UI/lastRasterFillImageDir", fileInfo.absolutePath() );
    mImageLineEdit->setText( filePath );
    on_mImageLineEdit_editingFinished();
  }
}

void QgsRasterFillSymbolLayerWidget::on_mImageLineEdit_editingFinished()
{
  if ( !mLayer )
  {
    return;
  }

  QFileInfo fi( mImageLineEdit->text() );
  if ( !fi.exists() )
  {
    QUrl url( mImageLineEdit->text() );
    if ( !url.isValid() )
    {
      return;
    }
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  mLayer->setImageFilePath( mImageLineEdit->text() );
  updatePreviewImage();
  QApplication::restoreOverrideCursor();

  emit changed();
}

void QgsRasterFillSymbolLayerWidget::setCoordinateMode( int index )
{
  switch ( index )
  {
    case 0:
      //feature coordinate mode
      mLayer->setCoordinateMode( QgsRasterFillSymbolLayer::Feature );
      break;
    case 1:
      //viewport coordinate mode
      mLayer->setCoordinateMode( QgsRasterFillSymbolLayer::Viewport );
      break;
  }

  emit changed();
}

void QgsRasterFillSymbolLayerWidget::on_mSpinTransparency_valueChanged( int value )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setAlpha( value / 100.0 );
  emit changed();
  updatePreviewImage();
}

void QgsRasterFillSymbolLayerWidget::offsetChanged()
{
  mLayer->setOffset( QPointF( mSpinOffsetX->value(), mSpinOffsetY->value() ) );
  emit changed();
}

void QgsRasterFillSymbolLayerWidget::on_mOffsetUnitWidget_changed()
{
  if ( !mLayer )
  {
    return;
  }
  QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
  mLayer->setOffsetUnit( unit );
  mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsRasterFillSymbolLayerWidget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
    emit changed();
  }
}

void QgsRasterFillSymbolLayerWidget::on_mWidthUnitWidget_changed()
{
  if ( !mLayer )
  {
    return;
  }
  QgsSymbolV2::OutputUnit unit;
  switch ( mWidthUnitWidget->getUnit() )
  {
    case 0:
      unit = QgsSymbolV2::Pixel;
      break;
    case 2:
      unit = QgsSymbolV2::MapUnit;
      break;
    case 1:
    default:
      unit = QgsSymbolV2::MM;
      break;
  }

  mLayer->setWidthUnit( unit );
  mLayer->setWidthMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsRasterFillSymbolLayerWidget::on_mWidthSpinBox_valueChanged( double d )
{
  if ( !mLayer )
  {
    return;
  }
  mLayer->setWidth( d );
  emit changed();
}

void QgsRasterFillSymbolLayerWidget::updatePreviewImage()
{
  if ( !mLayer )
  {
    return;
  }

  QImage image( mLayer->imageFilePath() );
  if ( image.isNull() )
  {
    mLabelImagePreview->setPixmap( QPixmap() );
    return;
  }

  if ( image.height() > 150 || image.width() > 150 )
  {
    image = image.scaled( 150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation );
  }

  QImage previewImage( 150, 150, QImage::Format_ARGB32 );
  previewImage.fill( Qt::transparent );
  QRect imageRect(( 150 - image.width() ) / 2.0, ( 150 - image.height() ) / 2.0, image.width(), image.height() );
  QPainter p;
  p.begin( &previewImage );
  //draw a checkerboard background
  uchar pixDataRGB[] = { 150, 150, 150, 150,
                         100, 100, 100, 150,
                         100, 100, 100, 150,
                         150, 150, 150, 150
                       };
  QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
  QPixmap pix = QPixmap::fromImage( img.scaled( 8, 8 ) );
  QBrush checkerBrush;
  checkerBrush.setTexture( pix );
  p.fillRect( imageRect, checkerBrush );

  if ( mLayer->alpha() < 1.0 )
  {
    p.setOpacity( mLayer->alpha() );
  }

  p.drawImage( imageRect.left(), imageRect.top(), image );
  p.end();
  mLabelImagePreview->setPixmap( QPixmap::fromImage( previewImage ) );
}

#undef QGS_BLOCK_SIGNAL_DURING_CALL

