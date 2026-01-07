/***************************************************************************
    qgslayerpropertieswidget.cpp
    ----------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerpropertieswidget.h"

#include "qgsapplication.h"
#include "qgsarrowsymbollayerwidget.h"
#include "qgsellipsesymbollayerwidget.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbol.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsinterpolatedlinesymbollayerwidget.h"
#include "qgslinesymbol.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmasksymbollayerwidget.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgspanelwidget.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbollayerutils.h"
#include "qgssymbollayerwidget.h"
#include "qgstemporalcontroller.h"
#include "qgsvectorfieldsymbollayerwidget.h"
#include "qgsvectorlayer.h"

#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPicture>
#include <QStandardItem>

#include "moc_qgslayerpropertieswidget.cpp"

static bool _initWidgetFunction( const QString &name, QgsSymbolLayerWidgetFunc f )
{
  QgsSymbolLayerRegistry *reg = QgsApplication::symbolLayerRegistry();

  QgsSymbolLayerAbstractMetadata *abstractMetadata = reg->symbolLayerMetadata( name );
  if ( !abstractMetadata )
  {
    QgsDebugError( "Failed to find symbol layer's entry in registry: " + name );
    return false;
  }
  QgsSymbolLayerMetadata *metadata = dynamic_cast<QgsSymbolLayerMetadata *>( abstractMetadata );
  if ( !metadata )
  {
    QgsDebugError( "Failed to cast symbol layer's metadata: " + name );
    return false;
  }
  metadata->setWidgetFunction( f );
  return true;
}

static void _initWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  _initWidgetFunction( u"SimpleLine"_s, QgsSimpleLineSymbolLayerWidget::create );
  _initWidgetFunction( u"MarkerLine"_s, QgsMarkerLineSymbolLayerWidget::create );
  _initWidgetFunction( u"HashLine"_s, QgsHashedLineSymbolLayerWidget::create );
  _initWidgetFunction( u"ArrowLine"_s, QgsArrowSymbolLayerWidget::create );
  _initWidgetFunction( u"InterpolatedLine"_s, QgsInterpolatedLineSymbolLayerWidget::create );
  _initWidgetFunction( u"RasterLine"_s, QgsRasterLineSymbolLayerWidget::create );
  _initWidgetFunction( u"Lineburst"_s, QgsLineburstSymbolLayerWidget::create );
  _initWidgetFunction( u"FilledLine"_s, QgsFilledLineSymbolLayerWidget::create );
  _initWidgetFunction( u"LinearReferencing"_s, QgsLinearReferencingSymbolLayerWidget::create );

  _initWidgetFunction( u"SimpleMarker"_s, QgsSimpleMarkerSymbolLayerWidget::create );
  _initWidgetFunction( u"FilledMarker"_s, QgsFilledMarkerSymbolLayerWidget::create );
  _initWidgetFunction( u"SvgMarker"_s, QgsSvgMarkerSymbolLayerWidget::create );
  _initWidgetFunction( u"RasterMarker"_s, QgsRasterMarkerSymbolLayerWidget::create );
  _initWidgetFunction( u"AnimatedMarker"_s, QgsAnimatedMarkerSymbolLayerWidget::create );
  _initWidgetFunction( u"FontMarker"_s, QgsFontMarkerSymbolLayerWidget::create );
  _initWidgetFunction( u"EllipseMarker"_s, QgsEllipseSymbolLayerWidget::create );
  _initWidgetFunction( u"VectorField"_s, QgsVectorFieldSymbolLayerWidget::create );
  _initWidgetFunction( u"MaskMarker"_s, QgsMaskMarkerSymbolLayerWidget::create );

  _initWidgetFunction( u"SimpleFill"_s, QgsSimpleFillSymbolLayerWidget::create );
  _initWidgetFunction( u"GradientFill"_s, QgsGradientFillSymbolLayerWidget::create );
  _initWidgetFunction( u"ShapeburstFill"_s, QgsShapeburstFillSymbolLayerWidget::create );
  _initWidgetFunction( u"RasterFill"_s, QgsRasterFillSymbolLayerWidget::create );
  _initWidgetFunction( u"SVGFill"_s, QgsSVGFillSymbolLayerWidget::create );
  _initWidgetFunction( u"CentroidFill"_s, QgsCentroidFillSymbolLayerWidget::create );
  _initWidgetFunction( u"LinePatternFill"_s, QgsLinePatternFillSymbolLayerWidget::create );
  _initWidgetFunction( u"PointPatternFill"_s, QgsPointPatternFillSymbolLayerWidget::create );
  _initWidgetFunction( u"RandomMarkerFill"_s, QgsRandomMarkerFillSymbolLayerWidget::create );

  _initWidgetFunction( u"GeometryGenerator"_s, QgsGeometryGeneratorSymbolLayerWidget::create );

  sInitialized = true;
}


QgsLayerPropertiesWidget::QgsLayerPropertiesWidget( QgsSymbolLayer *layer, const QgsSymbol *symbol, QgsVectorLayer *vl, QWidget *parent )
  : QgsPanelWidget( parent )
  , mLayer( layer )
  , mSymbol( symbol )
  , mVectorLayer( vl )
{
  setupUi( this );
  connect( mEnabledCheckBox, &QCheckBox::toggled, this, &QgsLayerPropertiesWidget::mEnabledCheckBox_toggled );
  // initialize the sub-widgets
  // XXX Should this thing be here this way? Initialize all the widgets just for the sake of one layer?
  // TODO Make this on demand creation
  _initWidgetFunctions();

  // TODO Algorithm
  //
  // 3. populate the combo box with the supported layer type
  // 4. set the present layer type
  // 5. create the widget for the present layer type and set in stacked widget
  // 6. connect comboBox type changed to two things
  //     1. emit signal that type has beed changed
  //     2. remove the widget and place the new widget corresponding to the changed layer type
  //
  populateLayerTypes();
  // update layer type combo box
  const int idx = cboLayerType->findData( mLayer->layerType() );
  cboLayerType->setCurrentIndex( idx );

  connect( mEnabledCheckBox, &QAbstractButton::toggled, mEnabledDDBtn, &QWidget::setEnabled );
  mEnabledCheckBox->setChecked( mLayer->enabled() );

  // set the corresponding widget
  updateSymbolLayerWidget( layer );
  connect( cboLayerType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayerPropertiesWidget::layerTypeChanged );

  connect( mEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsLayerPropertiesWidget::emitSignalChanged );

  this->connectChildPanel( mEffectWidget );

  if ( !mLayer->paintEffect() )
  {
    mLayer->setPaintEffect( QgsPaintEffectRegistry::defaultStack() );
    mLayer->paintEffect()->setEnabled( false );
  }
  mEffectWidget->setPaintEffect( mLayer->paintEffect() );

  registerDataDefinedButton( mEnabledDDBtn, QgsSymbolLayer::Property::LayerEnabled );
}

void QgsLayerPropertiesWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  if ( mSymbol )
    mContext.setSymbolType( mSymbol->type() );

  QgsSymbolLayerWidget *w = dynamic_cast<QgsSymbolLayerWidget *>( stackedWidget->currentWidget() );
  if ( w )
    w->setContext( mContext );
}

QgsSymbolWidgetContext QgsLayerPropertiesWidget::context() const
{
  return mContext;
}

void QgsLayerPropertiesWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mEffectWidget->setDockMode( this->dockMode() );
}

void QgsLayerPropertiesWidget::populateLayerTypes()
{
  const QStringList symbolLayerIds = QgsApplication::symbolLayerRegistry()->symbolLayersForType( mSymbol->type() );

  const auto constSymbolLayerIds = symbolLayerIds;
  for ( const QString &symbolLayerId : constSymbolLayerIds )
    cboLayerType->addItem( QgsApplication::symbolLayerRegistry()->symbolLayerMetadata( symbolLayerId )->visibleName(), symbolLayerId );

  if ( mSymbol->type() == Qgis::SymbolType::Fill )
  {
    const QStringList lineLayerIds = QgsApplication::symbolLayerRegistry()->symbolLayersForType( Qgis::SymbolType::Line );
    const auto constLineLayerIds = lineLayerIds;
    for ( const QString &lineLayerId : constLineLayerIds )
    {
      QgsSymbolLayerAbstractMetadata *layerInfo = QgsApplication::symbolLayerRegistry()->symbolLayerMetadata( lineLayerId );
      if ( layerInfo->type() != Qgis::SymbolType::Hybrid )
      {
        const QString visibleName = layerInfo->visibleName();
        const QString name = tr( "Outline: %1" ).arg( visibleName );
        cboLayerType->addItem( name, lineLayerId );
      }
    }
  }
}

void QgsLayerPropertiesWidget::updateSymbolLayerWidget( QgsSymbolLayer *layer )
{
  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    if ( QgsSymbolLayerWidget *w = qobject_cast<QgsSymbolLayerWidget *>( stackedWidget->currentWidget() ) )
      disconnect( w, &QgsSymbolLayerWidget::changed, this, &QgsLayerPropertiesWidget::emitSignalChanged );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }

  QgsSymbolLayerRegistry *pReg = QgsApplication::symbolLayerRegistry();

  const QString layerType = layer->layerType();
  QgsSymbolLayerAbstractMetadata *am = pReg->symbolLayerMetadata( layerType );
  if ( am )
  {
    QgsSymbolLayerWidget *w = am->createSymbolLayerWidget( mVectorLayer );
    if ( w )
    {
      w->setContext( mContext );
      w->setSymbolLayer( layer );
      stackedWidget->addWidget( w );
      stackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, &QgsSymbolLayerWidget::changed, this, &QgsLayerPropertiesWidget::emitSignalChanged );
      connect( w, &QgsSymbolLayerWidget::symbolChanged, this, &QgsLayerPropertiesWidget::reloadLayer );
      return;
    }
  }
  // When anything is not right
  stackedWidget->setCurrentWidget( pageDummy );
}

QgsExpressionContext QgsLayerPropertiesWidget::createExpressionContext() const
{
  if ( auto *lExpressionContext = mContext.expressionContext() )
    return *lExpressionContext;

  QgsExpressionContext expContext;
  if ( auto *lMapCanvas = mContext.mapCanvas() )
  {
    expContext = lMapCanvas->createExpressionContext();
  }
  else
  {
    expContext << QgsExpressionContextUtils::globalScope()
               << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
               << QgsExpressionContextUtils::atlasScope( nullptr )
               << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  expContext << QgsExpressionContextUtils::layerScope( mVectorLayer );

  QgsExpressionContextScope *symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  if ( mLayer )
  {
    //cheat a bit - set the symbol color variable to match the symbol layer's color (when we should really be using the *symbols*
    //color, but that's not accessible here). 99% of the time these will be the same anyway
    symbolScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, mLayer->color(), true ) );
  }
  expContext << symbolScope;
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, 0, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"symbol_layer_count"_s, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"symbol_layer_index"_s, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"symbol_marker_row"_s, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"symbol_marker_column"_s, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"symbol_frame"_s, 1, true ) );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );

  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR << QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT << QgsExpressionContext::EXPR_GEOMETRY_PART_NUM << QgsExpressionContext::EXPR_GEOMETRY_RING_NUM << QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT << QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM << QgsExpressionContext::EXPR_CLUSTER_COLOR << QgsExpressionContext::EXPR_CLUSTER_SIZE << u"symbol_layer_count"_s << u"symbol_layer_index"_s << u"symbol_frame"_s );

  return expContext;
}

void QgsLayerPropertiesWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key )
{
  button->init( static_cast<int>( key ), mLayer->dataDefinedProperties(), QgsSymbolLayer::propertyDefinitions(), mVectorLayer );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsLayerPropertiesWidget::updateProperty );
  button->registerExpressionContextGenerator( this );
}

void QgsLayerPropertiesWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsSymbolLayer::Property key = static_cast<QgsSymbolLayer::Property>( button->propertyKey() );
  mLayer->setDataDefinedProperty( key, button->toProperty() );
  emit changed();
}

void QgsLayerPropertiesWidget::layerTypeChanged()
{
  QgsSymbolLayer *layer = mLayer;
  if ( !layer )
    return;
  const QString newLayerType = cboLayerType->currentData().toString();
  if ( layer->layerType() == newLayerType )
    return;

  // get creation function for new layer from registry
  QgsSymbolLayerRegistry *pReg = QgsApplication::symbolLayerRegistry();
  QgsSymbolLayerAbstractMetadata *am = pReg->symbolLayerMetadata( newLayerType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change layer to a new (with different type)
  // base new layer on existing layer's properties
  QVariantMap properties = layer->properties();

  // if the old symbol layer was a "geometry generator" layer then
  // we instead get the properties from the generator
  if ( QgsGeometryGeneratorSymbolLayer *generator = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( layer ) )
  {
    if ( generator->subSymbol() && generator->subSymbol()->symbolLayerCount() > 0 )
      properties = generator->subSymbol()->symbolLayer( 0 )->properties();
  }

  QgsSymbolLayer *newLayer = am->createSymbolLayer( properties );
  if ( !newLayer )
    return;

  // if a symbol layer is changed to a "geometry generator" layer, then we move the old symbol layer into the
  // geometry generator's subsymbol.
  if ( QgsGeometryGeneratorSymbolLayer *generator = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( newLayer ) )
  {
    if ( mSymbol )
    {
      switch ( mSymbol->type() )
      {
        case Qgis::SymbolType::Marker:
        {
          auto markerSymbol = std::make_unique<QgsMarkerSymbol>( QgsSymbolLayerList( { layer->clone() } ) );
          generator->setSymbolType( Qgis::SymbolType::Marker );
          generator->setSubSymbol( markerSymbol.release() );
          break;
        }
        case Qgis::SymbolType::Line:
        {
          auto lineSymbol = std::make_unique<QgsLineSymbol>( QgsSymbolLayerList( { layer->clone() } ) );
          generator->setSymbolType( Qgis::SymbolType::Line );
          generator->setSubSymbol( lineSymbol.release() );
          break;
        }
        case Qgis::SymbolType::Fill:
        {
          auto fillSymbol = std::make_unique<QgsFillSymbol>( QgsSymbolLayerList( { layer->clone() } ) );
          generator->setSymbolType( Qgis::SymbolType::Fill );
          generator->setSubSymbol( fillSymbol.release() );
          break;
        }
        case Qgis::SymbolType::Hybrid:
          break;
      }
    }
  }
  else
  {
    // try to copy the subsymbol, if its the same type as the new symbol layer's subsymbol
    if ( newLayer->subSymbol() && layer->subSymbol() && newLayer->subSymbol()->type() == layer->subSymbol()->type() )
    {
      newLayer->setSubSymbol( layer->subSymbol()->clone() );
    }
  }

  // special logic for when NEW symbol layers are created from GUI only...
  // TODO: find a nicer generic way to handle this!
  if ( QgsFontMarkerSymbolLayer *fontMarker = dynamic_cast<QgsFontMarkerSymbolLayer *>( newLayer ) )
  {
    const QString defaultFont = fontMarker->fontFamily();
    const QFontDatabase fontDb;
    if ( !fontDb.hasFamily( defaultFont ) )
    {
      // default font marker font choice doesn't exist on system, so just use first available symbol font
      const QStringList candidates = fontDb.families( QFontDatabase::WritingSystem::Symbol );
      bool foundGoodCandidate = false;
      for ( const QString &candidate : candidates )
      {
        if ( fontDb.writingSystems( candidate ).size() == 1 )
        {
          // family ONLY offers symbol writing systems, so it's a good candidate!
          fontMarker->setFontFamily( candidate );
          foundGoodCandidate = true;
          break;
        }
      }
      if ( !foundGoodCandidate && !candidates.empty() )
      {
        // fallback to first available family which advertises symbol writing system
        QString candidate = candidates.at( 0 );
        fontMarker->setFontFamily( candidate );
      }
    }

    // search (briefly!!) for a unicode character which actually exists in the font
    const QFontMetrics fontMetrics( fontMarker->fontFamily() );
    ushort character = fontMarker->character().at( 0 ).unicode();
    for ( ; character < 1000; ++character )
    {
      if ( fontMetrics.inFont( QChar( character ) ) )
      {
        fontMarker->setCharacter( QChar( character ) );
        break;
      }
    }
  }

  updateSymbolLayerWidget( newLayer );
  emit changeLayer( newLayer );
}

void QgsLayerPropertiesWidget::emitSignalChanged()
{
  emit changed();

  // also update paint effect preview
  bool paintEffectToggled = false;
  if ( mLayer->paintEffect() && mLayer->paintEffect()->enabled() )
  {
    mLayer->paintEffect()->setEnabled( false );
    paintEffectToggled = true;
  }
  mEffectWidget->setPreviewPicture( QgsSymbolLayerUtils::symbolLayerPreviewPicture( mLayer, Qgis::RenderUnit::Millimeters, QSize( 60, 60 ), QgsMapUnitScale(), mSymbol ? mSymbol->type() : Qgis::SymbolType::Hybrid ) );
  if ( paintEffectToggled )
  {
    mLayer->paintEffect()->setEnabled( true );
  }
  emit widgetChanged();
}

void QgsLayerPropertiesWidget::reloadLayer()
{
  emit changeLayer( mLayer );
}

void QgsLayerPropertiesWidget::mEnabledCheckBox_toggled( bool enabled )
{
  mLayer->setEnabled( enabled );
  emitSignalChanged();
}
