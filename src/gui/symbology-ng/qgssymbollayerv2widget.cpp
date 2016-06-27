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
#include "qgsgeometrygeneratorsymbollayerv2.h"
#include "qgssymbolslistwidget.h"

#include "characterwidget.h"
#include "qgsdashspacedialog.h"
#include "qgsdatadefinedsymboldialog.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssvgcache.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsvectorgradientcolorrampv2dialog.h"
#include "qgsdatadefined.h"
#include "qgsstylev2.h" //for symbol selector dialog
#include "qgsmapcanvas.h"
#include "qgsapplication.h"

#include "qgslogger.h"
#include "qgssizescalewidget.h"

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

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsSymbolLayerV2Widget* widget = ( const QgsSymbolLayerV2Widget* ) context;

  if ( widget->expressionContext() )
    return QgsExpressionContext( *widget->expressionContext() );

  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( widget->mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( widget->mapCanvas()->mapSettings() )
    << new QgsExpressionContextScope( widget->mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  const QgsVectorLayer* layer = widget->vectorLayer();
  if ( layer )
    expContext << QgsExpressionContextUtils::layerScope( layer );

  QgsExpressionContextScope* symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  if ( const QgsSymbolLayerV2* symbolLayer = const_cast< QgsSymbolLayerV2Widget* >( widget )->symbolLayer() )
  {
    //cheat a bit - set the symbol color variable to match the symbol layer's color (when we should really be using the *symbols*
    //color, but that's not accessible here). 99% of the time these will be the same anyway
    symbolScope->setVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, symbolLayer->color() );
  }
  expContext << symbolScope;
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, 1, true ) );

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );
  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR
                                      << QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT << QgsExpressionContext::EXPR_GEOMETRY_PART_NUM
                                      << QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT << QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM );

  return expContext;
}

void QgsSymbolLayerV2Widget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  Q_FOREACH ( QgsUnitSelectionWidget* unitWidget, findChildren<QgsUnitSelectionWidget*>() )
  {
    unitWidget->setMapCanvas( mMapCanvas );
  }
  Q_FOREACH ( QgsDataDefinedButton* ddButton, findChildren<QgsDataDefinedButton*>() )
  {
    if ( ddButton->assistant() )
      ddButton->assistant()->setMapCanvas( mMapCanvas );
  }
}

const QgsMapCanvas* QgsSymbolLayerV2Widget::mapCanvas() const
{
  return mMapCanvas;
}

void QgsSymbolLayerV2Widget::registerDataDefinedButton( QgsDataDefinedButton * button, const QString & propertyName, QgsDataDefinedButton::DataType type, const QString & description )
{
  const QgsDataDefined* dd = symbolLayer()->getDataDefinedProperty( propertyName );
  button->init( mVectorLayer, dd, type, description );
  button->setProperty( "propertyName", propertyName );
  connect( button, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( button, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  button->registerGetExpressionContextCallback( &_getExpressionContext, this );
}

void QgsSymbolLayerV2Widget::updateDataDefinedProperty()
{
  QgsDataDefinedButton* button = qobject_cast<QgsDataDefinedButton*>( sender() );
  const QString propertyName( button->property( "propertyName" ).toString() );

  QgsDataDefined* dd = symbolLayer()->getDataDefinedProperty( propertyName );
  if ( !dd )
  {
    dd = new QgsDataDefined();
    symbolLayer()->setDataDefinedProperty( propertyName, dd );
  }
  button->updateDataDefined( dd );

  emit changed();
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
          label += " (" + tr( "area" ) + ')';
          break;
        case QgsSymbolV2::ScaleDiameter:
          label += " (" + tr( "diameter" ) + ')';
          break;
      }
    }
  }
  return label;
}

QgsSimpleLineSymbolLayerV2Widget::QgsSimpleLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mPenWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mDashPatternUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

  btnChangeColor->setAllowAlpha( true );
  btnChangeColor->setColorDialogTitle( tr( "Select line color" ) );
  btnChangeColor->setContext( "symbology" );

  spinOffset->setClearValue( 0.0 );

  if ( vl && vl->geometryType() != QGis::Polygon )
  {
    //draw inside polygon checkbox only makes sense for polygon layers
    mDrawInsideCheckBox->hide();
  }

  //make a temporary symbol for the size assistant preview
  mAssistantPreviewSymbol = new QgsLineSymbolV2();

  if ( mVectorLayer )
    mPenWidthDDBtn->setAssistant( tr( "Width Assistant..." ), new QgsSizeScaleWidget( mVectorLayer, mAssistantPreviewSymbol ) );


  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( penWidthChanged() ) );
  connect( btnChangeColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( colorChanged( const QColor& ) ) );
  connect( cboPenStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  connect( spinOffset, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( cboCapStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );

  updatePatternIcon();

  connect( this, SIGNAL( changed() ), this, SLOT( updateAssistantSymbol() ) );
}

QgsSimpleLineSymbolLayerV2Widget::~QgsSimpleLineSymbolLayerV2Widget()
{
  delete mAssistantPreviewSymbol;
}

void QgsSimpleLineSymbolLayerV2Widget::updateAssistantSymbol()
{
  for ( int i = mAssistantPreviewSymbol->symbolLayerCount() - 1 ; i >= 0; --i )
  {
    mAssistantPreviewSymbol->deleteSymbolLayer( i );
  }
  mAssistantPreviewSymbol->appendSymbolLayer( mLayer->clone() );
  QgsDataDefined* ddWidth = mLayer->getDataDefinedProperty( "width" );
  if ( ddWidth )
    mAssistantPreviewSymbol->setDataDefinedWidth( *ddWidth );
}


void QgsSimpleLineSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "SimpleLine" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleLineSymbolLayerV2*>( layer );

  // set units
  mPenWidthUnitWidget->blockSignals( true );
  mPenWidthUnitWidget->setUnit( mLayer->widthUnit() );
  mPenWidthUnitWidget->setMapUnitScale( mLayer->widthMapUnitScale() );
  mPenWidthUnitWidget->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );
  mDashPatternUnitWidget->blockSignals( true );
  mDashPatternUnitWidget->setUnit( mLayer->customDashPatternUnit() );
  mDashPatternUnitWidget->setMapUnitScale( mLayer->customDashPatternMapUnitScale() );
  mDashPatternUnitWidget->setMapUnitScale( mLayer->customDashPatternMapUnitScale() );
  mDashPatternUnitWidget->blockSignals( false );

  // set values
  spinWidth->blockSignals( true );
  spinWidth->setValue( mLayer->width() );
  spinWidth->blockSignals( false );
  btnChangeColor->blockSignals( true );
  btnChangeColor->setColor( mLayer->color() );
  btnChangeColor->blockSignals( false );
  spinOffset->blockSignals( true );
  spinOffset->setValue( mLayer->offset() );
  spinOffset->blockSignals( false );
  cboPenStyle->blockSignals( true );
  cboJoinStyle->blockSignals( true );
  cboCapStyle->blockSignals( true );
  cboPenStyle->setPenStyle( mLayer->penStyle() );
  cboJoinStyle->setPenJoinStyle( mLayer->penJoinStyle() );
  cboCapStyle->setPenCapStyle( mLayer->penCapStyle() );
  cboPenStyle->blockSignals( false );
  cboJoinStyle->blockSignals( false );
  cboCapStyle->blockSignals( false );

  //use a custom dash pattern?
  bool useCustomDashPattern = mLayer->useCustomDashPattern();
  mChangePatternButton->setEnabled( useCustomDashPattern );
  label_3->setEnabled( !useCustomDashPattern );
  cboPenStyle->setEnabled( !useCustomDashPattern );
  mCustomCheckBox->blockSignals( true );
  mCustomCheckBox->setCheckState( useCustomDashPattern ? Qt::Checked : Qt::Unchecked );
  mCustomCheckBox->blockSignals( false );

  //draw inside polygon?
  bool drawInsidePolygon = mLayer->drawInsidePolygon();
  mDrawInsideCheckBox->blockSignals( true );
  mDrawInsideCheckBox->setCheckState( drawInsidePolygon ? Qt::Checked : Qt::Unchecked );
  mDrawInsideCheckBox->blockSignals( false );

  updatePatternIcon();

  registerDataDefinedButton( mColorDDBtn, "color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mPenWidthDDBtn, "width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mDashPatternDDBtn, "customdash", QgsDataDefinedButton::String, QgsDataDefinedButton::customDashDesc() );
  registerDataDefinedButton( mPenStyleDDBtn, "line_style", QgsDataDefinedButton::String, QgsDataDefinedButton::lineStyleDesc() );
  registerDataDefinedButton( mJoinStyleDDBtn, "joinstyle", QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );
  registerDataDefinedButton( mCapStyleDDBtn, "capstyle", QgsDataDefinedButton::String, QgsDataDefinedButton::capStyleDesc() );

  updateAssistantSymbol();
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
    mLayer->setWidthUnit( mPenWidthUnitWidget->unit() );
    mLayer->setWidthMapUnitScale( mPenWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::on_mDashPatternUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setCustomDashPatternUnit( mDashPatternUnitWidget->unit() );
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
  QgsSimpleLineSymbolLayerV2* layerCopy = mLayer->clone();
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
  mLayer = nullptr;

  setupUi( this );
  mSizeUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOutlineWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

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

  //make a temporary symbol for the size assistant preview
  mAssistantPreviewSymbol = new QgsMarkerSymbolV2();

  if ( mVectorLayer )
    mSizeDDBtn->setAssistant( tr( "Size Assistant..." ), new QgsSizeScaleWidget( mVectorLayer, mAssistantPreviewSymbol ) );

  QSize size = lstNames->iconSize();
  double markerSize = DEFAULT_POINT_SIZE * 2;
  Q_FOREACH ( QgsSimpleMarkerSymbolLayerBase::Shape shape, QgsSimpleMarkerSymbolLayerBase::availableShapes() )
  {
    QgsSimpleMarkerSymbolLayerV2* lyr = new QgsSimpleMarkerSymbolLayerV2( shape, markerSize );
    lyr->setColor( QColor( 200, 200, 200 ) );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, size );
    QListWidgetItem* item = new QListWidgetItem( icon, QString(), lstNames );
    item->setData( Qt::UserRole, static_cast< int >( shape ) );
    item->setToolTip( QgsSimpleMarkerSymbolLayerBase::encodeShape( shape ) );
    delete lyr;
  }

  connect( lstNames, SIGNAL( currentRowChanged( int ) ), this, SLOT( setName() ) );
  connect( btnChangeColorBorder, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColorBorder( const QColor& ) ) );
  connect( btnChangeColorFill, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColorFill( const QColor& ) ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penJoinStyleChanged() ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( this, SIGNAL( changed() ), this, SLOT( updateAssistantSymbol() ) );
}

QgsSimpleMarkerSymbolLayerV2Widget::~QgsSimpleMarkerSymbolLayerV2Widget()
{
  delete mAssistantPreviewSymbol;
}

void QgsSimpleMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SimpleMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleMarkerSymbolLayerV2*>( layer );

  // set values
  QgsSimpleMarkerSymbolLayerBase::Shape shape = mLayer->shape();
  for ( int i = 0; i < lstNames->count(); ++i )
  {
    if ( static_cast< QgsSimpleMarkerSymbolLayerBase::Shape >( lstNames->item( i )->data( Qt::UserRole ).toInt() ) == shape )
    {
      lstNames->setCurrentRow( i );
      break;
    }
  }
  btnChangeColorBorder->blockSignals( true );
  btnChangeColorBorder->setColor( mLayer->borderColor() );
  btnChangeColorBorder->blockSignals( false );
  btnChangeColorFill->blockSignals( true );
  btnChangeColorFill->setColor( mLayer->fillColor() );
  btnChangeColorFill->setEnabled( QgsSimpleMarkerSymbolLayerBase::shapeIsFilled( mLayer->shape() ) );
  btnChangeColorFill->blockSignals( false );
  spinSize->blockSignals( true );
  spinSize->setValue( mLayer->size() );
  spinSize->blockSignals( false );
  spinAngle->blockSignals( true );
  spinAngle->setValue( mLayer->angle() );
  spinAngle->blockSignals( false );
  mOutlineStyleComboBox->blockSignals( true );
  mOutlineStyleComboBox->setPenStyle( mLayer->outlineStyle() );
  mOutlineStyleComboBox->blockSignals( false );
  mOutlineWidthSpinBox->blockSignals( true );
  mOutlineWidthSpinBox->setValue( mLayer->outlineWidth() );
  mOutlineWidthSpinBox->blockSignals( false );
  cboJoinStyle->blockSignals( true );
  cboJoinStyle->setPenJoinStyle( mLayer->penJoinStyle() );
  cboJoinStyle->blockSignals( false );

  // without blocking signals the value gets changed because of slot setOffset()
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );

  mSizeUnitWidget->blockSignals( true );
  mSizeUnitWidget->setUnit( mLayer->sizeUnit() );
  mSizeUnitWidget->setMapUnitScale( mLayer->sizeMapUnitScale() );
  mSizeUnitWidget->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );
  mOutlineWidthUnitWidget->blockSignals( true );
  mOutlineWidthUnitWidget->setUnit( mLayer->outlineWidthUnit() );
  mOutlineWidthUnitWidget->setMapUnitScale( mLayer->outlineWidthMapUnitScale() );
  mOutlineWidthUnitWidget->blockSignals( false );

  //anchor points
  mHorizontalAnchorComboBox->blockSignals( true );
  mVerticalAnchorComboBox->blockSignals( true );
  mHorizontalAnchorComboBox->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  mVerticalAnchorComboBox->setCurrentIndex( mLayer->verticalAnchorPoint() );
  mHorizontalAnchorComboBox->blockSignals( false );
  mVerticalAnchorComboBox->blockSignals( false );

  registerDataDefinedButton( mNameDDBtn, "name", QgsDataDefinedButton::String, tr( "string " ) + QLatin1String( "[<b>square</b>|<b>rectangle</b>|<b>diamond</b>|"
                             "<b>pentagon</b>|<b>hexagon</b>|<b>triangle</b>|<b>equilateral_triangle</b>|"
                             "<b>star</b>|<b>arrow</b>|<b>filled_arrowhead</b>|"
                             "<b>circle</b>|<b>cross</b>|<b>cross_fill</b>|<b>x</b>|"
                             "<b>line</b>|<b>arrowhead</b>|<b>cross2</b>|<b>semi_circle</b>|<b>third_circle</b>|<b>quarter_circle</b>|"
                             "<b>quarter_square</b>|<b>half_square</b>|<b>diagonal_half_square</b>|<b>right_half_triangle</b>|<b>left_half_triangle</b>]" ) );
  registerDataDefinedButton( mFillColorDDBtn, "color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBorderColorDDBtn, "color_border", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mOutlineWidthDDBtn, "outline_width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mOutlineStyleDDBtn, "outline_style", QgsDataDefinedButton::String, QgsDataDefinedButton::lineStyleDesc() );
  registerDataDefinedButton( mJoinStyleDDBtn, "join_style", QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );
  registerDataDefinedButton( mSizeDDBtn, "size", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mAngleDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::String, QgsDataDefinedButton::doubleXYDesc() );
  registerDataDefinedButton( mHorizontalAnchorDDBtn, "horizontal_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::horizontalAnchorDesc() );
  registerDataDefinedButton( mVerticalAnchorDDBtn, "vertical_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::verticalAnchorDesc() );

  updateAssistantSymbol();
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleMarkerSymbolLayerV2Widget::setName()
{
  mLayer->setShape( static_cast< QgsSimpleMarkerSymbolLayerBase::Shape>( lstNames->currentItem()->data( Qt::UserRole ).toInt() ) );
  btnChangeColorFill->setEnabled( QgsSimpleMarkerSymbolLayerBase::shapeIsFilled( mLayer->shape() ) );
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

void QgsSimpleMarkerSymbolLayerV2Widget::penJoinStyleChanged()
{
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
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
    mLayer->setSizeUnit( mSizeUnitWidget->unit() );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleMarkerSymbolLayerV2Widget::on_mOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthUnit( mOutlineWidthUnitWidget->unit() );
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

void QgsSimpleMarkerSymbolLayerV2Widget::updateAssistantSymbol()
{
  for ( int i = mAssistantPreviewSymbol->symbolLayerCount() - 1 ; i >= 0; --i )
  {
    mAssistantPreviewSymbol->deleteSymbolLayer( i );
  }
  mAssistantPreviewSymbol->appendSymbolLayer( mLayer->clone() );
  QgsDataDefined* ddSize = mLayer->getDataDefinedProperty( "size" );
  if ( ddSize )
    mAssistantPreviewSymbol->setDataDefinedSize( *ddSize );
}


///////////

QgsSimpleFillSymbolLayerV2Widget::QgsSimpleFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mBorderWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

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
  btnChangeColor->blockSignals( true );
  btnChangeColor->setColor( mLayer->color() );
  btnChangeColor->blockSignals( false );
  cboFillStyle->blockSignals( true );
  cboFillStyle->setBrushStyle( mLayer->brushStyle() );
  cboFillStyle->blockSignals( false );
  btnChangeBorderColor->blockSignals( true );
  btnChangeBorderColor->setColor( mLayer->borderColor() );
  btnChangeBorderColor->blockSignals( false );
  cboBorderStyle->blockSignals( true );
  cboBorderStyle->setPenStyle( mLayer->borderStyle() );
  cboBorderStyle->blockSignals( false );
  spinBorderWidth->blockSignals( true );
  spinBorderWidth->setValue( mLayer->borderWidth() );
  spinBorderWidth->blockSignals( false );
  cboJoinStyle->blockSignals( true );
  cboJoinStyle->setPenJoinStyle( mLayer->penJoinStyle() );
  cboJoinStyle->blockSignals( false );
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );

  mBorderWidthUnitWidget->blockSignals( true );
  mBorderWidthUnitWidget->setUnit( mLayer->borderWidthUnit() );
  mBorderWidthUnitWidget->setMapUnitScale( mLayer->borderWidthMapUnitScale() );
  mBorderWidthUnitWidget->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  registerDataDefinedButton( mFillColorDDBtn, "color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBorderColorDDBtn, "color_border", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBorderWidthDDBtn, "width_border", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mFillStyleDDBtn, "fill_style", QgsDataDefinedButton::String, QgsDataDefinedButton::fillStyleDesc() );
  registerDataDefinedButton( mBorderStyleDDBtn, "border_style", QgsDataDefinedButton::String, QgsDataDefinedButton::lineStyleDesc() );
  registerDataDefinedButton( mJoinStyleDDBtn, "join_style", QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );

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
    mLayer->setBorderWidthUnit( mBorderWidthUnitWidget->unit() );
    mLayer->setBorderWidthMapUnitScale( mBorderWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSimpleFillSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

///////////

QgsFilledMarkerSymbolLayerWidget::QgsFilledMarkerSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mSizeUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  //make a temporary symbol for the size assistant preview
  mAssistantPreviewSymbol = new QgsMarkerSymbolV2();

  if ( mVectorLayer )
    mSizeDDBtn->setAssistant( tr( "Size Assistant..." ), new QgsSizeScaleWidget( mVectorLayer, mAssistantPreviewSymbol ) );

  QSize size = lstNames->iconSize();
  double markerSize = DEFAULT_POINT_SIZE * 2;
  Q_FOREACH ( QgsSimpleMarkerSymbolLayerBase::Shape shape, QgsSimpleMarkerSymbolLayerBase::availableShapes() )
  {
    if ( !QgsSimpleMarkerSymbolLayerBase::shapeIsFilled( shape ) )
      continue;

    QgsSimpleMarkerSymbolLayerV2* lyr = new QgsSimpleMarkerSymbolLayerV2( shape, markerSize );
    lyr->setColor( QColor( 200, 200, 200 ) );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, size );
    QListWidgetItem* item = new QListWidgetItem( icon, QString(), lstNames );
    item->setData( Qt::UserRole, static_cast< int >( shape ) );
    item->setToolTip( QgsSimpleMarkerSymbolLayerBase::encodeShape( shape ) );
    delete lyr;
  }

  connect( lstNames, SIGNAL( currentRowChanged( int ) ), this, SLOT( setShape() ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( this, SIGNAL( changed() ), this, SLOT( updateAssistantSymbol() ) );
}

QgsFilledMarkerSymbolLayerWidget::~QgsFilledMarkerSymbolLayerWidget()
{
  delete mAssistantPreviewSymbol;
}

void QgsFilledMarkerSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "FilledMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsFilledMarkerSymbolLayer*>( layer );

  // set values
  QgsSimpleMarkerSymbolLayerBase::Shape shape = mLayer->shape();
  for ( int i = 0; i < lstNames->count(); ++i )
  {
    if ( static_cast< QgsSimpleMarkerSymbolLayerBase::Shape >( lstNames->item( i )->data( Qt::UserRole ).toInt() ) == shape )
    {
      lstNames->setCurrentRow( i );
      break;
    }
  }
  whileBlocking( spinSize )->setValue( mLayer->size() );
  whileBlocking( spinAngle )->setValue( mLayer->angle() );
  whileBlocking( spinOffsetX )->setValue( mLayer->offset().x() );
  whileBlocking( spinOffsetY )->setValue( mLayer->offset().y() );

  mSizeUnitWidget->blockSignals( true );
  mSizeUnitWidget->setUnit( mLayer->sizeUnit() );
  mSizeUnitWidget->setMapUnitScale( mLayer->sizeMapUnitScale() );
  mSizeUnitWidget->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  //anchor points
  whileBlocking( mHorizontalAnchorComboBox )->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  whileBlocking( mVerticalAnchorComboBox )->setCurrentIndex( mLayer->verticalAnchorPoint() );

  registerDataDefinedButton( mNameDDBtn, "name", QgsDataDefinedButton::String, tr( "string " ) + QLatin1String( "[<b>square</b>|<b>rectangle</b>|<b>diamond</b>|"
                             "<b>pentagon</b>|<b>hexagon</b>|<b>triangle</b>|<b>equilateral_triangle</b>|"
                             "<b>star</b>|<b>arrow</b>|<b>filled_arrowhead</b>|"
                             "<b>circle</b>|<b>cross</b>|<b>cross_fill</b>|<b>x</b>|"
                             "<b>line</b>|<b>arrowhead</b>|<b>cross2</b>|<b>semi_circle</b>|<b>third_circle</b>|<b>quarter_circle</b>|"
                             "<b>quarter_square</b>|<b>half_square</b>|<b>diagonal_half_square</b>|<b>right_half_triangle</b>|<b>left_half_triangle</b>]" ) );
  registerDataDefinedButton( mSizeDDBtn, "size", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mAngleDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::String, QgsDataDefinedButton::doubleXYDesc() );
  registerDataDefinedButton( mHorizontalAnchorDDBtn, "horizontal_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::horizontalAnchorDesc() );
  registerDataDefinedButton( mVerticalAnchorDDBtn, "vertical_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::verticalAnchorDesc() );

  updateAssistantSymbol();
}

QgsSymbolLayerV2* QgsFilledMarkerSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsFilledMarkerSymbolLayerWidget::setShape()
{
  mLayer->setShape( static_cast< QgsSimpleMarkerSymbolLayerBase::Shape>( lstNames->currentItem()->data( Qt::UserRole ).toInt() ) );
  emit changed();
}

void QgsFilledMarkerSymbolLayerWidget::setSize()
{
  mLayer->setSize( spinSize->value() );
  emit changed();
}

void QgsFilledMarkerSymbolLayerWidget::setAngle()
{
  mLayer->setAngle( spinAngle->value() );
  emit changed();
}

void QgsFilledMarkerSymbolLayerWidget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsFilledMarkerSymbolLayerWidget::on_mSizeUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSizeUnit( mSizeUnitWidget->unit() );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsFilledMarkerSymbolLayerWidget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsFilledMarkerSymbolLayerWidget::on_mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint(( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint ) index );
    emit changed();
  }
}

void QgsFilledMarkerSymbolLayerWidget::on_mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint(( QgsMarkerSymbolLayerV2::VerticalAnchorPoint ) index );
    emit changed();
  }
}

void QgsFilledMarkerSymbolLayerWidget::updateAssistantSymbol()
{
  for ( int i = mAssistantPreviewSymbol->symbolLayerCount() - 1 ; i >= 0; --i )
  {
    mAssistantPreviewSymbol->deleteSymbolLayer( i );
  }
  mAssistantPreviewSymbol->appendSymbolLayer( mLayer->clone() );
  QgsDataDefined* ddSize = mLayer->getDataDefinedProperty( "size" );
  if ( ddSize )
    mAssistantPreviewSymbol->setDataDefinedSize( *ddSize );
}


///////////

QgsGradientFillSymbolLayerV2Widget::QgsGradientFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

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
  connect( cboGradientColorRamp, SIGNAL( sourceRampEdited() ), this, SLOT( applyColorRamp() ) );
  connect( mButtonEditRamp, SIGNAL( clicked() ), cboGradientColorRamp, SLOT( editSourceRamp() ) );
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
  btnChangeColor->blockSignals( true );
  btnChangeColor->setColor( mLayer->color() );
  btnChangeColor->blockSignals( false );
  btnChangeColor2->blockSignals( true );
  btnChangeColor2->setColor( mLayer->color2() );
  btnChangeColor2->blockSignals( false );

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
  {
    cboGradientColorRamp->blockSignals( true );
    cboGradientColorRamp->setSourceColorRamp( mLayer->colorRamp() );
    cboGradientColorRamp->blockSignals( false );
  }

  cboGradientType->blockSignals( true );
  switch ( mLayer->gradientType() )
  {
    case QgsGradientFillSymbolLayerV2::Linear:
      cboGradientType->setCurrentIndex( 0 );
      break;
    case QgsGradientFillSymbolLayerV2::Radial:
      cboGradientType->setCurrentIndex( 1 );
      break;
    case QgsGradientFillSymbolLayerV2::Conical:
      cboGradientType->setCurrentIndex( 2 );
      break;
  }
  cboGradientType->blockSignals( false );

  cboCoordinateMode->blockSignals( true );
  switch ( mLayer->coordinateMode() )
  {
    case QgsGradientFillSymbolLayerV2::Viewport:
      cboCoordinateMode->setCurrentIndex( 1 );
      checkRefPoint1Centroid->setEnabled( false );
      checkRefPoint2Centroid->setEnabled( false );
      break;
    case QgsGradientFillSymbolLayerV2::Feature:
    default:
      cboCoordinateMode->setCurrentIndex( 0 );
      break;
  }
  cboCoordinateMode->blockSignals( false );

  cboGradientSpread->blockSignals( true );
  switch ( mLayer->gradientSpread() )
  {
    case QgsGradientFillSymbolLayerV2::Pad:
      cboGradientSpread->setCurrentIndex( 0 );
      break;
    case QgsGradientFillSymbolLayerV2::Repeat:
      cboGradientSpread->setCurrentIndex( 1 );
      break;
    case QgsGradientFillSymbolLayerV2::Reflect:
      cboGradientSpread->setCurrentIndex( 2 );
      break;
  }
  cboGradientSpread->blockSignals( false );

  spinRefPoint1X->blockSignals( true );
  spinRefPoint1X->setValue( mLayer->referencePoint1().x() );
  spinRefPoint1X->blockSignals( false );
  spinRefPoint1Y->blockSignals( true );
  spinRefPoint1Y->setValue( mLayer->referencePoint1().y() );
  spinRefPoint1Y->blockSignals( false );
  checkRefPoint1Centroid->blockSignals( true );
  checkRefPoint1Centroid->setChecked( mLayer->referencePoint1IsCentroid() );
  if ( mLayer->referencePoint1IsCentroid() )
  {
    spinRefPoint1X->setEnabled( false );
    spinRefPoint1Y->setEnabled( false );
  }
  checkRefPoint1Centroid->blockSignals( false );
  spinRefPoint2X->blockSignals( true );
  spinRefPoint2X->setValue( mLayer->referencePoint2().x() );
  spinRefPoint2X->blockSignals( false );
  spinRefPoint2Y->blockSignals( true );
  spinRefPoint2Y->setValue( mLayer->referencePoint2().y() );
  spinRefPoint2Y->blockSignals( false );
  checkRefPoint2Centroid->blockSignals( true );
  checkRefPoint2Centroid->setChecked( mLayer->referencePoint2IsCentroid() );
  if ( mLayer->referencePoint2IsCentroid() )
  {
    spinRefPoint2X->setEnabled( false );
    spinRefPoint2Y->setEnabled( false );
  }
  checkRefPoint2Centroid->blockSignals( false );

  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );
  mSpinAngle->blockSignals( true );
  mSpinAngle->setValue( mLayer->angle() );
  mSpinAngle->blockSignals( false );

  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  registerDataDefinedButton( mStartColorDDBtn, "color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mEndColorDDBtn, "color2", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mAngleDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mGradientTypeDDBtn, "gradient_type", QgsDataDefinedButton::String, QgsDataDefinedButton::gradientTypeDesc() );
  registerDataDefinedButton( mCoordinateModeDDBtn, "coordinate_mode", QgsDataDefinedButton::String, QgsDataDefinedButton::gradientCoordModeDesc() );
  registerDataDefinedButton( mSpreadDDBtn, "spread", QgsDataDefinedButton::Double, QgsDataDefinedButton::gradientSpreadDesc() );
  registerDataDefinedButton( mRefPoint1XDDBtn, "reference1_x", QgsDataDefinedButton::Double, QgsDataDefinedButton::double0to1Desc() );
  registerDataDefinedButton( mRefPoint1YDDBtn, "reference1_y", QgsDataDefinedButton::Double, QgsDataDefinedButton::double0to1Desc() );
  registerDataDefinedButton( mRefPoint1CentroidDDBtn, "reference1_iscentroid", QgsDataDefinedButton::Int, QgsDataDefinedButton::boolDesc() );
  registerDataDefinedButton( mRefPoint2XDDBtn, "reference2_x", QgsDataDefinedButton::Double, QgsDataDefinedButton::double0to1Desc() );
  registerDataDefinedButton( mRefPoint2YDDBtn, "reference2_y", QgsDataDefinedButton::Double, QgsDataDefinedButton::double0to1Desc() );
  registerDataDefinedButton( mRefPoint2CentroidDDBtn, "reference2_iscentroid", QgsDataDefinedButton::Int, QgsDataDefinedButton::boolDesc() );
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
  if ( !ramp )
    return;

  mLayer->setColorRamp( ramp );
  emit changed();
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
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

///////////

QgsShapeburstFillSymbolLayerV2Widget::QgsShapeburstFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mDistanceUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

  QButtonGroup* group1 = new QButtonGroup( this );
  group1->addButton( radioColorRamp );
  group1->addButton( radioTwoColor );
  QButtonGroup* group2 = new QButtonGroup( this );
  group2->addButton( mRadioUseMaxDistance );
  group2->addButton( mRadioUseWholeShape );
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

  cboGradientColorRamp->populate( QgsStyleV2::defaultStyle() );

  connect( cboGradientColorRamp, SIGNAL( currentIndexChanged( int ) ), this, SLOT( applyColorRamp() ) );
  connect( cboGradientColorRamp, SIGNAL( sourceRampEdited() ), this, SLOT( applyColorRamp() ) );
  connect( mButtonEditRamp, SIGNAL( clicked() ), cboGradientColorRamp, SLOT( editSourceRamp() ) );

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
  btnChangeColor->blockSignals( true );
  btnChangeColor->setColor( mLayer->color() );
  btnChangeColor->blockSignals( false );
  btnChangeColor2->blockSignals( true );
  btnChangeColor2->setColor( mLayer->color2() );
  btnChangeColor2->blockSignals( false );

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

  mSpinBlurRadius->blockSignals( true );
  mBlurSlider->blockSignals( true );
  mSpinBlurRadius->setValue( mLayer->blurRadius() );
  mBlurSlider->setValue( mLayer->blurRadius() );
  mSpinBlurRadius->blockSignals( false );
  mBlurSlider->blockSignals( false );

  mSpinMaxDistance->blockSignals( true );
  mSpinMaxDistance->setValue( mLayer->maxDistance() );
  mSpinMaxDistance->blockSignals( false );

  mRadioUseWholeShape->blockSignals( true );
  mRadioUseMaxDistance->blockSignals( true );
  if ( mLayer->useWholeShape() )
  {
    mRadioUseWholeShape->setChecked( true );
    mSpinMaxDistance->setEnabled( false );
    mDistanceUnitWidget->setEnabled( false );
  }
  else
  {
    mRadioUseMaxDistance->setChecked( true );
    mSpinMaxDistance->setEnabled( true );
    mDistanceUnitWidget->setEnabled( true );
  }
  mRadioUseWholeShape->blockSignals( false );
  mRadioUseMaxDistance->blockSignals( false );

  mDistanceUnitWidget->blockSignals( true );
  mDistanceUnitWidget->setUnit( mLayer->distanceUnit() );
  mDistanceUnitWidget->setMapUnitScale( mLayer->distanceMapUnitScale() );
  mDistanceUnitWidget->blockSignals( false );

  mIgnoreRingsCheckBox->blockSignals( true );
  mIgnoreRingsCheckBox->setCheckState( mLayer->ignoreRings() ? Qt::Checked : Qt::Unchecked );
  mIgnoreRingsCheckBox->blockSignals( false );

  // set source color ramp
  if ( mLayer->colorRamp() )
  {
    cboGradientColorRamp->blockSignals( true );
    cboGradientColorRamp->setSourceColorRamp( mLayer->colorRamp() );
    cboGradientColorRamp->blockSignals( false );
  }

  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  registerDataDefinedButton( mStartColorDDBtn, "color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mEndColorDDBtn, "color2", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBlurRadiusDDBtn, "blur_radius", QgsDataDefinedButton::Int, tr( "Integer between 0 and 18" ) );
  registerDataDefinedButton( mShadeWholeShapeDDBtn, "use_whole_shape", QgsDataDefinedButton::Int, QgsDataDefinedButton::boolDesc() );
  registerDataDefinedButton( mShadeDistanceDDBtn, "max_distance", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mIgnoreRingsDDBtn, "ignore_rings", QgsDataDefinedButton::Int, QgsDataDefinedButton::boolDesc() );
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
    mLayer->setDistanceUnit( mDistanceUnitWidget->unit() );
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
    emit changed();
  }
}

void QgsShapeburstFillSymbolLayerV2Widget::applyColorRamp()
{
  QgsVectorColorRampV2* ramp = cboGradientColorRamp->currentColorRamp();
  if ( !ramp )
    return;

  mLayer->setColorRamp( ramp );
  emit changed();
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
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
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
  mLayer = nullptr;

  setupUi( this );
  mIntervalUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetAlongLineUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

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
  connect( radCurvePoint, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
}

void QgsMarkerLineSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "MarkerLine" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsMarkerLineSymbolLayerV2*>( layer );

  // set values
  spinInterval->blockSignals( true );
  spinInterval->setValue( mLayer->interval() );
  spinInterval->blockSignals( false );
  mSpinOffsetAlongLine->blockSignals( true );
  mSpinOffsetAlongLine->setValue( mLayer->offsetAlongLine() );
  mSpinOffsetAlongLine->blockSignals( false );
  chkRotateMarker->blockSignals( true );
  chkRotateMarker->setChecked( mLayer->rotateMarker() );
  chkRotateMarker->blockSignals( false );
  spinOffset->blockSignals( true );
  spinOffset->setValue( mLayer->offset() );
  spinOffset->blockSignals( false );
  if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::Interval )
    radInterval->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::Vertex )
    radVertex->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::LastVertex )
    radVertexLast->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::CentralPoint )
    radCentralPoint->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::CurvePoint )
    radCurvePoint->setChecked( true );
  else
    radVertexFirst->setChecked( true );

  // set units
  mIntervalUnitWidget->blockSignals( true );
  mIntervalUnitWidget->setUnit( mLayer->intervalUnit() );
  mIntervalUnitWidget->setMapUnitScale( mLayer->intervalMapUnitScale() );
  mIntervalUnitWidget->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );
  mOffsetAlongLineUnitWidget->blockSignals( true );
  mOffsetAlongLineUnitWidget->setUnit( mLayer->offsetAlongLineUnit() );
  mOffsetAlongLineUnitWidget->setMapUnitScale( mLayer->offsetAlongLineMapUnitScale() );
  mOffsetAlongLineUnitWidget->blockSignals( false );

  setPlacement(); // update gui

  registerDataDefinedButton( mIntervalDDBtn, "interval", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mLineOffsetDDBtn, "offset", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mPlacementDDBtn, "placement", QgsDataDefinedButton::String, tr( "string " ) + QLatin1String( "[<b>vertex</b>|<b>lastvertex</b>|<b>firstvertex</b>|<b>centerpoint</b>]" ) );
  registerDataDefinedButton( mOffsetAlongLineDDBtn, "offset_along_line", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
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
  else if ( radCurvePoint->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::CurvePoint );
  else
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::CentralPoint );

  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::on_mIntervalUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setIntervalUnit( mIntervalUnitWidget->unit() );
    mLayer->setIntervalMapUnitScale( mIntervalUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsMarkerLineSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsMarkerLineSymbolLayerV2Widget::on_mOffsetAlongLineUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetAlongLineUnit( mOffsetAlongLineUnitWidget->unit() );
    mLayer->setOffsetAlongLineMapUnitScale( mOffsetAlongLineUnitWidget->getMapUnitScale() );
  }
  emit changed();
}

///////////


QgsSvgMarkerSymbolLayerV2Widget::QgsSvgMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mSizeUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mBorderWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  viewGroups->setHeaderHidden( true );
  mChangeColorButton->setAllowAlpha( true );
  mChangeColorButton->setColorDialogTitle( tr( "Select fill color" ) );
  mChangeColorButton->setContext( "symbology" );
  mChangeBorderColorButton->setAllowAlpha( true );
  mChangeBorderColorButton->setColorDialogTitle( tr( "Select border color" ) );
  mChangeBorderColorButton->setContext( "symbology" );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  populateList();

  connect( viewImages->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setName( const QModelIndex& ) ) );
  connect( viewGroups->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( populateIcons( const QModelIndex& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( this, SIGNAL( changed() ), this, SLOT( updateAssistantSymbol() ) );

  //make a temporary symbol for the size assistant preview
  mAssistantPreviewSymbol = new QgsMarkerSymbolV2();
  if ( mVectorLayer )
    mSizeDDBtn->setAssistant( tr( "Size Assistant..." ), new QgsSizeScaleWidget( mVectorLayer, mAssistantPreviewSymbol ) );
}

QgsSvgMarkerSymbolLayerV2Widget::~QgsSvgMarkerSymbolLayerV2Widget()
{
  delete mAssistantPreviewSymbol;
}

#include <QTime>
#include <QAbstractListModel>
#include <QPixmapCache>
#include <QStyle>


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
  bool hasFillParam, hasFillOpacityParam, hasOutlineParam, hasOutlineWidthParam, hasOutlineOpacityParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth, defaultFillOpacity, defaultOutlineOpacity;
  bool hasDefaultFillColor, hasDefaultFillOpacity, hasDefaultOutlineColor, hasDefaultOutlineWidth, hasDefaultOutlineOpacity;
  QgsSvgCache::instance()->containsParams( layer->path(), hasFillParam, hasDefaultFillColor, defaultFill,
      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
      hasOutlineParam, hasDefaultOutlineColor, defaultOutline,
      hasOutlineWidthParam, hasDefaultOutlineWidth, defaultOutlineWidth,
      hasOutlineOpacityParam, hasDefaultOutlineOpacity, defaultOutlineOpacity );
  mChangeColorButton->setEnabled( hasFillParam );
  mChangeColorButton->setAllowAlpha( hasFillOpacityParam );
  mChangeBorderColorButton->setEnabled( hasOutlineParam );
  mChangeBorderColorButton->setAllowAlpha( hasOutlineOpacityParam );
  mBorderWidthSpinBox->setEnabled( hasOutlineWidthParam );

  if ( hasFillParam )
  {
    QColor fill = layer->fillColor();
    double existingOpacity = hasFillOpacityParam ? fill.alphaF() : 1.0;
    if ( hasDefaultFillColor )
    {
      fill = defaultFill;
    }
    fill.setAlphaF( hasDefaultFillOpacity ? defaultFillOpacity : existingOpacity );
    mChangeColorButton->setColor( fill );
  }
  if ( hasOutlineParam )
  {
    QColor outline = layer->outlineColor();
    double existingOpacity = hasOutlineOpacityParam ? outline.alphaF() : 1.0;
    if ( hasDefaultOutlineColor )
    {
      outline = defaultOutline;
    }
    outline.setAlphaF( hasDefaultOutlineOpacity ? defaultOutlineOpacity : existingOpacity );
    mChangeBorderColorButton->setColor( outline );
  }

  mFileLineEdit->blockSignals( true );
  mFileLineEdit->setText( layer->path() );
  mFileLineEdit->blockSignals( false );

  mBorderWidthSpinBox->blockSignals( true );
  mBorderWidthSpinBox->setValue( hasDefaultOutlineWidth ? defaultOutlineWidth : layer->outlineWidth() );
  mBorderWidthSpinBox->blockSignals( false );
}

void QgsSvgMarkerSymbolLayerV2Widget::updateAssistantSymbol()
{
  for ( int i = mAssistantPreviewSymbol->symbolLayerCount() - 1 ; i >= 0; --i )
  {
    mAssistantPreviewSymbol->deleteSymbolLayer( i );
  }
  mAssistantPreviewSymbol->appendSymbolLayer( mLayer->clone() );
  QgsDataDefined* ddSize = mLayer->getDataDefinedProperty( "size" );
  if ( ddSize )
    mAssistantPreviewSymbol->setDataDefinedSize( *ddSize );
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

  spinSize->blockSignals( true );
  spinSize->setValue( mLayer->size() );
  spinSize->blockSignals( false );
  spinAngle->blockSignals( true );
  spinAngle->setValue( mLayer->angle() );
  spinAngle->blockSignals( false );

  // without blocking signals the value gets changed because of slot setOffset()
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );

  mSizeUnitWidget->blockSignals( true );
  mSizeUnitWidget->setUnit( mLayer->sizeUnit() );
  mSizeUnitWidget->setMapUnitScale( mLayer->sizeMapUnitScale() );
  mSizeUnitWidget->blockSignals( false );
  mBorderWidthUnitWidget->blockSignals( true );
  mBorderWidthUnitWidget->setUnit( mLayer->outlineWidthUnit() );
  mBorderWidthUnitWidget->setMapUnitScale( mLayer->outlineWidthMapUnitScale() );
  mBorderWidthUnitWidget->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  //anchor points
  mHorizontalAnchorComboBox->blockSignals( true );
  mVerticalAnchorComboBox->blockSignals( true );
  mHorizontalAnchorComboBox->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  mVerticalAnchorComboBox->setCurrentIndex( mLayer->verticalAnchorPoint() );
  mHorizontalAnchorComboBox->blockSignals( false );
  mVerticalAnchorComboBox->blockSignals( false );

  setGuiForSvg( mLayer );

  registerDataDefinedButton( mSizeDDBtn, "size", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mBorderWidthDDBtn, "outline_width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mAngleDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::String, QgsDataDefinedButton::doubleXYDesc() );
  registerDataDefinedButton( mFilenameDDBtn, "name", QgsDataDefinedButton::String, QgsDataDefinedButton::filePathDesc() );
  registerDataDefinedButton( mFillColorDDBtn, "fill", QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );
  registerDataDefinedButton( mBorderColorDDBtn, "outline", QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );
  registerDataDefinedButton( mHorizontalAnchorDDBtn, "horizontal_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::horizontalAnchorDesc() );
  registerDataDefinedButton( mVerticalAnchorDDBtn, "vertical_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::verticalAnchorDesc() );

  updateAssistantSymbol();
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
  QString file = QFileDialog::getOpenFileName( nullptr,
                 tr( "Select SVG file" ),
                 s.value( "/UI/lastSVGMarkerDir", QDir::homePath() ).toString(),
                 tr( "SVG files" ) + " (*.svg)" );
  QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() )
  {
    return;
  }
  mFileLineEdit->setText( file );
  mLayer->setPath( file );
  s.setValue( "/UI/lastSVGMarkerDir", fi.absolutePath() );
  setGuiForSvg( mLayer );
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
    mLayer->setSizeUnit( mSizeUnitWidget->unit() );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mBorderWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthUnit( mBorderWidthUnitWidget->unit() );
    mLayer->setOutlineWidthMapUnitScale( mBorderWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
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

QgsSVGFillSymbolLayerWidget::QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;
  setupUi( this );
  mTextureWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mSvgOutlineWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
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
    double width = mLayer->patternWidth();
    mTextureWidthSpinBox->blockSignals( true );
    mTextureWidthSpinBox->setValue( width );
    mTextureWidthSpinBox->blockSignals( false );
    mSVGLineEdit->setText( mLayer->svgFilePath() );
    mRotationSpinBox->blockSignals( true );
    mRotationSpinBox->setValue( mLayer->angle() );
    mRotationSpinBox->blockSignals( false );
    mTextureWidthUnitWidget->blockSignals( true );
    mTextureWidthUnitWidget->setUnit( mLayer->patternWidthUnit() );
    mTextureWidthUnitWidget->setMapUnitScale( mLayer->patternWidthMapUnitScale() );
    mTextureWidthUnitWidget->blockSignals( false );
    mSvgOutlineWidthUnitWidget->blockSignals( true );
    mSvgOutlineWidthUnitWidget->setUnit( mLayer->svgOutlineWidthUnit() );
    mSvgOutlineWidthUnitWidget->setMapUnitScale( mLayer->svgOutlineWidthMapUnitScale() );
    mSvgOutlineWidthUnitWidget->blockSignals( false );
    mChangeColorButton->blockSignals( true );
    mChangeColorButton->setColor( mLayer->svgFillColor() );
    mChangeColorButton->blockSignals( false );
    mChangeBorderColorButton->blockSignals( true );
    mChangeBorderColorButton->setColor( mLayer->svgOutlineColor() );
    mChangeBorderColorButton->blockSignals( false );
    mBorderWidthSpinBox->blockSignals( true );
    mBorderWidthSpinBox->setValue( mLayer->svgOutlineWidth() );
    mBorderWidthSpinBox->blockSignals( false );
  }
  updateParamGui( false );

  registerDataDefinedButton( mTextureWidthDDBtn, "width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mSVGDDBtn, "svgFile", QgsDataDefinedButton::String, QgsDataDefinedButton::svgPathDesc() );
  registerDataDefinedButton( mRotationDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mFilColorDDBtn, "svgFillColor", QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );
  registerDataDefinedButton( mBorderColorDDBtn, "svgOutlineColor", QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );
  registerDataDefinedButton( mBorderWidthDDBtn, "svgOutlineWidth", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsSVGFillSymbolLayerWidget::on_mBrowseToolButton_clicked()
{
  QString filePath = QFileDialog::getOpenFileName( nullptr, tr( "Select SVG texture file" ), QDir::homePath(), tr( "SVG file" ) + " (*.svg);;" + tr( "All files" ) + " (*.*)" );
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
  bool hasFillParam, hasFillOpacityParam, hasOutlineParam, hasOutlineWidthParam, hasOutlineOpacityParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth, defaultFillOpacity, defaultOutlineOpacity;
  bool hasDefaultFillColor, hasDefaultFillOpacity, hasDefaultOutlineColor, hasDefaultOutlineWidth, hasDefaultOutlineOpacity;
  QgsSvgCache::instance()->containsParams( mSVGLineEdit->text(), hasFillParam, hasDefaultFillColor, defaultFill,
      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
      hasOutlineParam, hasDefaultOutlineColor, defaultOutline,
      hasOutlineWidthParam, hasDefaultOutlineWidth, defaultOutlineWidth,
      hasOutlineOpacityParam, hasDefaultOutlineOpacity, defaultOutlineOpacity );
  if ( resetValues )
  {
    QColor fill = mChangeColorButton->color();
    double newOpacity = hasFillOpacityParam ? fill.alphaF() : 1.0;
    if ( hasDefaultFillColor )
    {
      fill = defaultFill;
    }
    fill.setAlphaF( hasDefaultFillOpacity ? defaultFillOpacity : newOpacity );
    mChangeColorButton->setColor( fill );
  }
  mChangeColorButton->setEnabled( hasFillParam );
  mChangeColorButton->setAllowAlpha( hasFillOpacityParam );
  if ( resetValues )
  {
    QColor outline = mChangeBorderColorButton->color();
    double newOpacity = hasOutlineOpacityParam ? outline.alphaF() : 1.0;
    if ( hasDefaultOutlineColor )
    {
      outline = defaultOutline;
    }
    outline.setAlphaF( hasDefaultOutlineOpacity ? defaultOutlineOpacity : newOpacity );
    mChangeBorderColorButton->setColor( outline );
  }
  mChangeBorderColorButton->setEnabled( hasOutlineParam );
  mChangeBorderColorButton->setAllowAlpha( hasOutlineOpacityParam );
  if ( hasDefaultOutlineWidth && resetValues )
  {
    mBorderWidthSpinBox->setValue( defaultOutlineWidth );
  }
  mBorderWidthSpinBox->setEnabled( hasOutlineWidthParam );
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
    mLayer->setPatternWidthUnit( mTextureWidthUnitWidget->unit() );
    mLayer->setPatternWidthMapUnitScale( mTextureWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mSvgOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSvgOutlineWidthUnit( mSvgOutlineWidthUnitWidget->unit() );
    mLayer->setSvgOutlineWidthMapUnitScale( mSvgOutlineWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

/////////////

QgsLinePatternFillSymbolLayerWidget::QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ):
    QgsSymbolLayerV2Widget( parent, vl ), mLayer( nullptr )
{
  setupUi( this );
  mDistanceUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetSpinBox->setClearValue( 0 );
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
    mAngleSpinBox->blockSignals( true );
    mAngleSpinBox->setValue( mLayer->lineAngle() );
    mAngleSpinBox->blockSignals( false );
    mDistanceSpinBox->blockSignals( true );
    mDistanceSpinBox->setValue( mLayer->distance() );
    mDistanceSpinBox->blockSignals( false );
    mOffsetSpinBox->blockSignals( true );
    mOffsetSpinBox->setValue( mLayer->offset() );
    mOffsetSpinBox->blockSignals( false );

    //units
    mDistanceUnitWidget->blockSignals( true );
    mDistanceUnitWidget->setUnit( mLayer->distanceUnit() );
    mDistanceUnitWidget->setMapUnitScale( mLayer->distanceMapUnitScale() );
    mDistanceUnitWidget->blockSignals( false );
    mOffsetUnitWidget->blockSignals( true );
    mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
    mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
    mOffsetUnitWidget->blockSignals( false );
  }

  registerDataDefinedButton( mAngleDDBtn, "lineangle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mDistanceDDBtn, "distance", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
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
    mLayer->setDistanceUnit( mDistanceUnitWidget->unit() );
    mLayer->setDistanceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

/////////////

QgsPointPatternFillSymbolLayerWidget::QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ):
    QgsSymbolLayerV2Widget( parent, vl ), mLayer( nullptr )
{
  setupUi( this );
  mHorizontalDistanceUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mVerticalDistanceUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mHorizontalDisplacementUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mVerticalDisplacementUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
}


void QgsPointPatternFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "PointPatternFill" )
  {
    return;
  }

  mLayer = static_cast<QgsPointPatternFillSymbolLayer*>( layer );
  mHorizontalDistanceSpinBox->blockSignals( true );
  mHorizontalDistanceSpinBox->setValue( mLayer->distanceX() );
  mHorizontalDistanceSpinBox->blockSignals( false );
  mVerticalDistanceSpinBox->blockSignals( true );
  mVerticalDistanceSpinBox->setValue( mLayer->distanceY() );
  mVerticalDistanceSpinBox->blockSignals( false );
  mHorizontalDisplacementSpinBox->blockSignals( true );
  mHorizontalDisplacementSpinBox->setValue( mLayer->displacementX() );
  mHorizontalDisplacementSpinBox->blockSignals( false );
  mVerticalDisplacementSpinBox->blockSignals( true );
  mVerticalDisplacementSpinBox->setValue( mLayer->displacementY() );
  mVerticalDisplacementSpinBox->blockSignals( false );

  mHorizontalDistanceUnitWidget->blockSignals( true );
  mHorizontalDistanceUnitWidget->setUnit( mLayer->distanceXUnit() );
  mHorizontalDistanceUnitWidget->setMapUnitScale( mLayer->distanceXMapUnitScale() );
  mHorizontalDistanceUnitWidget->blockSignals( false );
  mVerticalDistanceUnitWidget->blockSignals( true );
  mVerticalDistanceUnitWidget->setUnit( mLayer->distanceYUnit() );
  mVerticalDistanceUnitWidget->setMapUnitScale( mLayer->distanceYMapUnitScale() );
  mVerticalDistanceUnitWidget->blockSignals( false );
  mHorizontalDisplacementUnitWidget->blockSignals( true );
  mHorizontalDisplacementUnitWidget->setUnit( mLayer->displacementXUnit() );
  mHorizontalDisplacementUnitWidget->setMapUnitScale( mLayer->displacementXMapUnitScale() );
  mHorizontalDisplacementUnitWidget->blockSignals( false );
  mVerticalDisplacementUnitWidget->blockSignals( true );
  mVerticalDisplacementUnitWidget->setUnit( mLayer->displacementYUnit() );
  mVerticalDisplacementUnitWidget->setMapUnitScale( mLayer->displacementYMapUnitScale() );
  mVerticalDisplacementUnitWidget->blockSignals( false );

  registerDataDefinedButton( mHorizontalDistanceDDBtn, "distance_x", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mVerticalDistanceDDBtn, "distance_y", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mHorizontalDisplacementDDBtn, "displacement_x", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mVerticalDisplacementDDBtn, "displacement_y", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
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
    mLayer->setDistanceXUnit( mHorizontalDistanceUnitWidget->unit() );
    mLayer->setDistanceXMapUnitScale( mHorizontalDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDistanceUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setDistanceYUnit( mVerticalDistanceUnitWidget->unit() );
    mLayer->setDistanceYMapUnitScale( mVerticalDistanceUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDisplacementUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setDisplacementXUnit( mHorizontalDisplacementUnitWidget->unit() );
    mLayer->setDisplacementXMapUnitScale( mHorizontalDisplacementUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDisplacementUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setDisplacementYUnit( mVerticalDisplacementUnitWidget->unit() );
    mLayer->setDisplacementYMapUnitScale( mVerticalDisplacementUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

/////////////

QgsFontMarkerSymbolLayerV2Widget::QgsFontMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
  mSizeUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mBorderWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  widgetChar = new CharacterWidget;
  scrollArea->setWidget( widgetChar );

  btnColor->setAllowAlpha( true );
  btnColor->setColorDialogTitle( tr( "Select symbol fill color" ) );
  btnColor->setContext( "symbology" );
  btnBorderColor->setAllowAlpha( true );
  btnBorderColor->setColorDialogTitle( tr( "Select symbol outline color" ) );
  btnBorderColor->setContext( "symbology" );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );

  //make a temporary symbol for the size assistant preview
  mAssistantPreviewSymbol = new QgsMarkerSymbolV2();

  if ( mVectorLayer )
    mSizeDDBtn->setAssistant( tr( "Size Assistant..." ), new QgsSizeScaleWidget( mVectorLayer, mAssistantPreviewSymbol ) );

  connect( cboFont, SIGNAL( currentFontChanged( const QFont & ) ), this, SLOT( setFontFamily( const QFont& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize( double ) ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penJoinStyleChanged() ) );
  connect( btnColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor( const QColor& ) ) );
  connect( btnBorderColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColorBorder( const QColor& ) ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penJoinStyleChanged() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle( double ) ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( widgetChar, SIGNAL( characterSelected( const QChar & ) ), this, SLOT( setCharacter( const QChar & ) ) );
  connect( this, SIGNAL( changed() ), this, SLOT( updateAssistantSymbol() ) );
}

QgsFontMarkerSymbolLayerV2Widget::~QgsFontMarkerSymbolLayerV2Widget()
{
  delete mAssistantPreviewSymbol;
}

void QgsFontMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "FontMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsFontMarkerSymbolLayerV2*>( layer );

  QFont layerFont( mLayer->fontFamily() );
  // set values
  whileBlocking( cboFont )->setCurrentFont( layerFont );
  whileBlocking( spinSize )->setValue( mLayer->size() );
  whileBlocking( btnColor )->setColor( mLayer->color() );
  whileBlocking( btnBorderColor )->setColor( mLayer->outlineColor() );
  whileBlocking( mBorderWidthSpinBox )->setValue( mLayer->outlineWidth() );
  whileBlocking( spinAngle )->setValue( mLayer->angle() );

  widgetChar->blockSignals( true );
  widgetChar->updateFont( layerFont );
  widgetChar->setCharacter( mLayer->character() );
  widgetChar->blockSignals( false );

  //block
  whileBlocking( spinOffsetX )->setValue( mLayer->offset().x() );
  whileBlocking( spinOffsetY )->setValue( mLayer->offset().y() );

  mSizeUnitWidget->blockSignals( true );
  mSizeUnitWidget->setUnit( mLayer->sizeUnit() );
  mSizeUnitWidget->setMapUnitScale( mLayer->sizeMapUnitScale() );
  mSizeUnitWidget->blockSignals( false );

  mBorderWidthUnitWidget->blockSignals( true );
  mBorderWidthUnitWidget->setUnit( mLayer->outlineWidthUnit() );
  mBorderWidthUnitWidget->setMapUnitScale( mLayer->outlineWidthMapUnitScale() );
  mBorderWidthUnitWidget->blockSignals( false );

  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  whileBlocking( cboJoinStyle )->setPenJoinStyle( mLayer->penJoinStyle() );

  //anchor points
  whileBlocking( mHorizontalAnchorComboBox )->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  whileBlocking( mVerticalAnchorComboBox )->setCurrentIndex( mLayer->verticalAnchorPoint() );

  registerDataDefinedButton( mSizeDDBtn, "size", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mRotationDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mColorDDBtn, "color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBorderColorDDBtn, "color_border", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBorderWidthDDBtn, "outline_width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mJoinStyleDDBtn, "join_style", QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::String, QgsDataDefinedButton::doubleXYDesc() );
  registerDataDefinedButton( mHorizontalAnchorDDBtn, "horizontal_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::horizontalAnchorDesc() );
  registerDataDefinedButton( mVerticalAnchorDDBtn, "vertical_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::verticalAnchorDesc() );
  registerDataDefinedButton( mCharDDBtn, "char", QgsDataDefinedButton::String, QgsDataDefinedButton::charDesc() );

  updateAssistantSymbol();
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

void QgsFontMarkerSymbolLayerV2Widget::setColorBorder( const QColor& color )
{
  mLayer->setOutlineColor( color );
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

void QgsFontMarkerSymbolLayerV2Widget::setCharacter( QChar chr )
{
  mLayer->setCharacter( chr );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::penJoinStyleChanged()
{
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::on_mSizeUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSizeUnit( mSizeUnitWidget->unit() );
    mLayer->setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsFontMarkerSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsFontMarkerSymbolLayerV2Widget::on_mBorderWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthUnit( mSizeUnitWidget->unit() );
    mLayer->setOutlineWidthMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
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

void QgsFontMarkerSymbolLayerV2Widget::on_mBorderWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsFontMarkerSymbolLayerV2Widget::updateAssistantSymbol()
{
  for ( int i = mAssistantPreviewSymbol->symbolLayerCount() - 1 ; i >= 0; --i )
  {
    mAssistantPreviewSymbol->deleteSymbolLayer( i );
  }
  mAssistantPreviewSymbol->appendSymbolLayer( mLayer->clone() );
  QgsDataDefined* ddSize = mLayer->getDataDefinedProperty( "size" );
  if ( ddSize )
    mAssistantPreviewSymbol->setDataDefinedSize( *ddSize );
}

///////////////


QgsCentroidFillSymbolLayerV2Widget::QgsCentroidFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;

  setupUi( this );
}

void QgsCentroidFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "CentroidFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsCentroidFillSymbolLayerV2*>( layer );

  // set values
  whileBlocking( mDrawInsideCheckBox )->setChecked( mLayer->pointOnSurface() );
  whileBlocking( mDrawAllPartsCheckBox )->setChecked( mLayer->pointOnAllParts() );
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

void QgsCentroidFillSymbolLayerV2Widget::on_mDrawAllPartsCheckBox_stateChanged( int state )
{
  mLayer->setPointOnAllParts( state == Qt::Checked );
  emit changed();
}

///////////////

QgsRasterFillSymbolLayerWidget::QgsRasterFillSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = nullptr;
  setupUi( this );

  mWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::Pixel << QgsSymbolV2::MM << QgsSymbolV2::MapUnit );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

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

  mImageLineEdit->blockSignals( true );
  mImageLineEdit->setText( mLayer->imageFilePath() );
  mImageLineEdit->blockSignals( false );

  cboCoordinateMode->blockSignals( true );
  switch ( mLayer->coordinateMode() )
  {
    case QgsRasterFillSymbolLayer::Viewport:
      cboCoordinateMode->setCurrentIndex( 1 );
      break;
    case QgsRasterFillSymbolLayer::Feature:
    default:
      cboCoordinateMode->setCurrentIndex( 0 );
      break;
  }
  cboCoordinateMode->blockSignals( false );
  mSpinTransparency->blockSignals( true );
  mSpinTransparency->setValue( mLayer->alpha() * 100.0 );
  mSpinTransparency->blockSignals( false );
  mSliderTransparency->blockSignals( true );
  mSliderTransparency->setValue( mLayer->alpha() * 100.0 );
  mSliderTransparency->blockSignals( false );
  mRotationSpinBox->blockSignals( true );
  mRotationSpinBox->setValue( mLayer->angle() );
  mRotationSpinBox->blockSignals( false );

  mSpinOffsetX->blockSignals( true );
  mSpinOffsetX->setValue( mLayer->offset().x() );
  mSpinOffsetX->blockSignals( false );
  mSpinOffsetY->blockSignals( true );
  mSpinOffsetY->setValue( mLayer->offset().y() );
  mSpinOffsetY->blockSignals( false );
  mOffsetUnitWidget->blockSignals( true );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  mOffsetUnitWidget->blockSignals( false );

  mWidthSpinBox->blockSignals( true );
  mWidthSpinBox->setValue( mLayer->width() );
  mWidthSpinBox->blockSignals( false );
  mWidthUnitWidget->blockSignals( true );
  mWidthUnitWidget->setUnit( mLayer->widthUnit() );
  mWidthUnitWidget->setMapUnitScale( mLayer->widthMapUnitScale() );
  mWidthUnitWidget->blockSignals( false );
  updatePreviewImage();

  registerDataDefinedButton( mFilenameDDBtn, "file", QgsDataDefinedButton::String, QgsDataDefinedButton::filePathDesc() );
  registerDataDefinedButton( mOpacityDDBtn, "alpha", QgsDataDefinedButton::Double, QgsDataDefinedButton::double0to1Desc() );
  registerDataDefinedButton( mRotationDDBtn, "angle", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mWidthDDBtn, "width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
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
    openDir = s.value( "/UI/lastRasterFillImageDir", QDir::homePath() ).toString();
  }

  //show file dialog
  QString filePath = QFileDialog::getOpenFileName( nullptr, tr( "Select image file" ), openDir );
  if ( !filePath.isNull() )
  {
    //check if file exists
    QFileInfo fileInfo( filePath );
    if ( !fileInfo.exists() || !fileInfo.isReadable() )
    {
      QMessageBox::critical( nullptr, "Invalid file", "Error, file does not exist or is not readable" );
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
  mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
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
  mLayer->setWidthUnit( mWidthUnitWidget->unit() );
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


/// @cond PRIVATE

QgsSvgListModel::QgsSvgListModel( QObject* parent ) : QAbstractListModel( parent )
{
  mSvgFiles = QgsSymbolLayerV2Utils::listSvgFiles();
}

QgsSvgListModel::QgsSvgListModel( QObject* parent, const QString& path ) : QAbstractListModel( parent )
{
  mSvgFiles = QgsSymbolLayerV2Utils::listSvgFilesAt( path );
}

int QgsSvgListModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );
  return mSvgFiles.count();
}

QVariant QgsSvgListModel::data( const QModelIndex& index, int role ) const
{
  QString entry = mSvgFiles.at( index.row() );

  if ( role == Qt::DecorationRole ) // icon
  {
    QPixmap pixmap;
    if ( !QPixmapCache::find( entry, pixmap ) )
    {
      // render SVG file
      QColor fill, outline;
      double outlineWidth, fillOpacity, outlineOpacity;
      bool fillParam, fillOpacityParam, outlineParam, outlineWidthParam, outlineOpacityParam;
      bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultOutlineColor = false,
                                 hasDefaultOutlineWidth = false, hasDefaultOutlineOpacity = false;
      QgsSvgCache::instance()->containsParams( entry, fillParam, hasDefaultFillColor, fill,
          fillOpacityParam, hasDefaultFillOpacity, fillOpacity,
          outlineParam, hasDefaultOutlineColor, outline,
          outlineWidthParam, hasDefaultOutlineWidth, outlineWidth,
          outlineOpacityParam, hasDefaultOutlineOpacity, outlineOpacity );

      //if defaults not set in symbol, use these values
      if ( !hasDefaultFillColor )
        fill = QColor( 200, 200, 200 );
      fill.setAlphaF( hasDefaultFillOpacity ? fillOpacity : 1.0 );
      if ( !hasDefaultOutlineColor )
        outline = Qt::black;
      outline.setAlphaF( hasDefaultOutlineOpacity ? outlineOpacity : 1.0 );
      if ( !hasDefaultOutlineWidth )
        outlineWidth = 0.6;

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


QgsSvgGroupsModel::QgsSvgGroupsModel( QObject* parent ) : QStandardItemModel( parent )
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

void QgsSvgGroupsModel::createTree( QStandardItem*& parentGroup )
{
  QDir parentDir( parentGroup->data().toString() );
  Q_FOREACH ( const QString& item, parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
  {
    QStandardItem* group = new QStandardItem( item );
    group->setData( QVariant( parentDir.path() + '/' + item ) );
    group->setEditable( false );
    group->setCheckable( false );
    group->setToolTip( parentDir.path() + '/' + item );
    group->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
    parentGroup->appendRow( group );
    createTree( group );
  }
}


/// @endcond






QgsGeometryGeneratorSymbolLayerWidget::QgsGeometryGeneratorSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
    , mLayer( nullptr )
{
  setupUi( this );
  modificationExpressionSelector->setLayer( const_cast<QgsVectorLayer*>( vl ) );
  modificationExpressionSelector->loadFieldNames();
  modificationExpressionSelector->setExpressionContext( _getExpressionContext( this ) );
  cbxGeometryType->addItem( QgsApplication::getThemeIcon( "/mIconPolygonLayer.svg" ), tr( "Polygon / MultiPolygon" ), QgsSymbolV2::Fill );
  cbxGeometryType->addItem( QgsApplication::getThemeIcon( "/mIconLineLayer.svg" ), tr( "LineString / MultiLineString" ), QgsSymbolV2::Line );
  cbxGeometryType->addItem( QgsApplication::getThemeIcon( "/mIconPointLayer.svg" ), tr( "Point / MultiPoint" ), QgsSymbolV2::Marker );
  connect( modificationExpressionSelector, SIGNAL( expressionParsed( bool ) ), this, SLOT( updateExpression() ) );
  connect( cbxGeometryType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateSymbolType() ) );
}

void QgsGeometryGeneratorSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* l )
{
  mLayer = static_cast<QgsGeometryGeneratorSymbolLayerV2*>( l );

  if ( mPresetExpressionContext )
    modificationExpressionSelector->setExpressionContext( *mPresetExpressionContext );
  modificationExpressionSelector->setExpressionText( mLayer->geometryExpression() );
  cbxGeometryType->setCurrentIndex( cbxGeometryType->findData( mLayer->symbolType() ) );
}

QgsSymbolLayerV2* QgsGeometryGeneratorSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsGeometryGeneratorSymbolLayerWidget::updateExpression()
{
  mLayer->setGeometryExpression( modificationExpressionSelector->expressionText() );

  emit changed();
}

void QgsGeometryGeneratorSymbolLayerWidget::updateSymbolType()
{
  mLayer->setSymbolType( static_cast<QgsSymbolV2::SymbolType>( cbxGeometryType->itemData( cbxGeometryType->currentIndex() ).toInt() ) );

  emit symbolChanged();
}
