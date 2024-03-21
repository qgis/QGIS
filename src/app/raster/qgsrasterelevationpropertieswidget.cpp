/***************************************************************************
    qgsrasterelevationpropertieswidget.cpp
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterelevationpropertieswidget.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterrenderer.h"
#include "qgsexpressioncontextutils.h"
#include <QMenu>
#include <QAction>

QgsRasterElevationPropertiesWidget::QgsRasterElevationPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mModeComboBox->addItem( tr( "Disabled" ) );
  mModeComboBox->addItem( tr( "Represents Elevation Surface" ), QVariant::fromValue( Qgis::RasterElevationMode::RepresentsElevationSurface ) );
  mModeComboBox->addItem( tr( "Fixed Elevation Range" ), QVariant::fromValue( Qgis::RasterElevationMode::FixedElevationRange ) );
  mModeComboBox->addItem( tr( "Fixed Elevation Range Per Band" ), QVariant::fromValue( Qgis::RasterElevationMode::FixedRangePerBand ) );
  mModeComboBox->addItem( tr( "Dynamic Elevation Range Per Band" ), QVariant::fromValue( Qgis::RasterElevationMode::DynamicRangePerBand ) );

  mLimitsComboBox->addItem( tr( "Include Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeBoth ) );
  mLimitsComboBox->addItem( tr( "Include Lower, Exclude Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeLowerExcludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower, Include Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeLowerIncludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeBoth ) );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );
  mSymbologyStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mFixedLowerSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedUpperSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedLowerSpinBox->clear();
  mFixedUpperSpinBox->clear();

  mLowerExpressionWidget->registerExpressionContextGenerator( this );
  mUpperExpressionWidget->registerExpressionContextGenerator( this );

  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillBelow ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillAbove.svg" ) ), tr( "Fill Above" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillAbove ) );
  mElevationLimitSpinBox->setClearValue( mElevationLimitSpinBox->minimum(), tr( "Not set" ) );

  // NOTE -- this doesn't work, there's something broken in QgsStackedWidget which breaks the height calculations
  mWidgetFixedRangePerBand->setFixedHeight( QFontMetrics( font() ).height() * 15 );

  mFixedRangePerBandModel = new QgsRasterBandFixedElevationRangeModel( this );
  mBandElevationTable->verticalHeader()->setVisible( false );
  mBandElevationTable->setModel( mFixedRangePerBandModel );
  QgsFixedElevationRangeDelegate *tableDelegate = new QgsFixedElevationRangeDelegate( mBandElevationTable );
  mBandElevationTable->setItemDelegateForColumn( 1, tableDelegate );
  mBandElevationTable->setItemDelegateForColumn( 2, tableDelegate );

  mDynamicRangePerBandModel = new QgsRasterBandDynamicElevationRangeModel( this );
  mBandDynamicElevationTable->verticalHeader()->setVisible( false );
  mBandDynamicElevationTable->setModel( mDynamicRangePerBandModel );

  QMenu *calculateFixedRangePerBandMenu = new QMenu( mCalculateFixedRangePerBandButton );
  mCalculateFixedRangePerBandButton->setMenu( calculateFixedRangePerBandMenu );
  mCalculateFixedRangePerBandButton->setPopupMode( QToolButton::InstantPopup );
  QAction *calculateLowerAction = new QAction( "Calculate Lower by Expression…", calculateFixedRangePerBandMenu );
  calculateFixedRangePerBandMenu->addAction( calculateLowerAction );
  connect( calculateLowerAction, &QAction::triggered, this, [this]
  {
    calculateRangeByExpression( false );
  } );
  QAction *calculateUpperAction = new QAction( "Calculate Upper by Expression…", calculateFixedRangePerBandMenu );
  calculateFixedRangePerBandMenu->addAction( calculateUpperAction );
  connect( calculateUpperAction, &QAction::triggered, this, [this]
  {
    calculateRangeByExpression( true );
  } );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mElevationLimitSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterElevationPropertiesWidget::modeChanged );
  connect( mLimitsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mStyleComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    switch ( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) )
    {
      case Qgis::ProfileSurfaceSymbology::Line:
        mSymbologyStackedWidget->setCurrentWidget( mPageLine );
        break;
      case Qgis::ProfileSurfaceSymbology::FillBelow:
      case Qgis::ProfileSurfaceSymbology::FillAbove:
        mSymbologyStackedWidget->setCurrentWidget( mPageFill );
        break;
    }

    onChanged();
  } );
  connect( mLowerExpressionWidget, qOverload< const QString &, bool >( &QgsFieldExpressionWidget::fieldChanged ), this, [this]( const QString & expression, bool isValid )
  {
    if ( isValid )
      mDynamicRangePerBandModel->setLowerExpression( expression );
  } );
  connect( mUpperExpressionWidget, qOverload< const QString &, bool >( &QgsFieldExpressionWidget::fieldChanged ), this, [this]( const QString & expression, bool isValid )
  {
    if ( isValid )
      mDynamicRangePerBandModel->setUpperExpression( expression );
  } );

  setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#elevation-properties" ) );
}

void QgsRasterElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;

  const QgsRasterLayerElevationProperties *props = qgis::down_cast< const QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );
  if ( !props->isEnabled() )
  {
    mModeComboBox->setCurrentIndex( 0 );
    mStackedWidget->setCurrentWidget( mPageDisabled );
    mProfileChartGroupBox->hide();
  }
  else
  {
    mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( props->mode() ) ) );
    switch ( props->mode() )
    {
      case Qgis::RasterElevationMode::FixedElevationRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::RasterElevationMode::RepresentsElevationSurface:
        mStackedWidget->setCurrentWidget( mPageSurface );
        break;
      case Qgis::RasterElevationMode::FixedRangePerBand:
        mStackedWidget->setCurrentWidget( mPageFixedRangePerBand );
        break;
      case Qgis::RasterElevationMode::DynamicRangePerBand:
        mStackedWidget->setCurrentWidget( mPageDynamicPerBand );
        break;
    }
    mProfileChartGroupBox->show();
  }

  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  if ( std::isnan( props->elevationLimit() ) )
    mElevationLimitSpinBox->clear();
  else
    mElevationLimitSpinBox->setValue( props->elevationLimit() );
  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );
  mBandComboBox->setLayer( mLayer );
  mBandComboBox->setBand( props->bandNumber() );

  if ( props->fixedRange().lower() != std::numeric_limits< double >::lowest() )
    mFixedLowerSpinBox->setValue( props->fixedRange().lower() );
  else
    mFixedLowerSpinBox->clear();
  if ( props->fixedRange().upper() != std::numeric_limits< double >::max() )
    mFixedUpperSpinBox->setValue( props->fixedRange().upper() );
  else
    mFixedUpperSpinBox->clear();
  mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( props->fixedRange().rangeLimits() ) ) );

  mFixedRangePerBandModel->setLayerData( mLayer, props->fixedRangePerBand() );
  mBandElevationTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mBandElevationTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mBandElevationTable->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );

  mDynamicRangePerBandModel->setLayer( mLayer );
  mBandDynamicElevationTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mBandDynamicElevationTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mBandDynamicElevationTable->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );

  if ( QgsApplication::rasterRendererRegistry()->rendererCapabilities( mLayer->renderer()->type() ) & Qgis::RasterRendererCapability::UsesMultipleBands )
  {
    mWidgetFixedRangePerBand->hide();
    mFixedRangePerBandLabel->setText( tr( "This mode cannot be used with a multi-band renderer." ) );
  }

  mStyleComboBox->setCurrentIndex( mStyleComboBox->findData( static_cast <int >( props->profileSymbology() ) ) );
  switch ( props->profileSymbology() )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mSymbologyStackedWidget->setCurrentWidget( mPageLine );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
    case Qgis::ProfileSurfaceSymbology::FillAbove:
      mSymbologyStackedWidget->setCurrentWidget( mPageFill );
      break;
  }

  mLowerExpressionWidget->setExpression(
    props->dataDefinedProperties().property( QgsMapLayerElevationProperties::Property::RasterPerBandLowerElevation ).asExpression() );
  mUpperExpressionWidget->setExpression(
    props->dataDefinedProperties().property( QgsMapLayerElevationProperties::Property::RasterPerBandUpperElevation ).asExpression() );

  mDynamicRangePerBandModel->setLowerExpression( mLowerExpressionWidget->expression() );
  mDynamicRangePerBandModel->setUpperExpression( mUpperExpressionWidget->expression() );

  QList<QPair<QString, QVariant> > bandChoices;
  for ( int band = 1; band <= mLayer->bandCount(); ++band )
  {
    bandChoices << qMakePair( mLayer->dataProvider()->displayBandName( band ), band );
  }
  mLowerExpressionWidget->setCustomPreviewGenerator( tr( "Band" ), bandChoices, [this]( const QVariant & value )-> QgsExpressionContext
  {
    return createExpressionContextForBand( value.toInt() );
  } );
  mUpperExpressionWidget->setCustomPreviewGenerator( tr( "Band" ), bandChoices, [this]( const QVariant & value )-> QgsExpressionContext
  {
    return createExpressionContextForBand( value.toInt() );
  } );

  mBlockUpdates = false;
}

QgsExpressionContext QgsRasterElevationPropertiesWidget::createExpressionContext() const
{
  return createExpressionContextForBand( 1 );
}

void QgsRasterElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsRasterLayerElevationProperties *props = qgis::down_cast< QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );

  if ( !mModeComboBox->currentData().isValid() )
  {
    props->setEnabled( false );
  }
  else
  {
    props->setEnabled( true );
    props->setMode( mModeComboBox->currentData().value< Qgis::RasterElevationMode >() );
  }

  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  if ( mElevationLimitSpinBox->value() != mElevationLimitSpinBox->clearValue() )
    props->setElevationLimit( mElevationLimitSpinBox->value() );
  else
    props->setElevationLimit( std::numeric_limits< double >::quiet_NaN() );
  props->setProfileLineSymbol( mLineStyleButton->clonedSymbol< QgsLineSymbol >() );
  props->setProfileFillSymbol( mFillStyleButton->clonedSymbol< QgsFillSymbol >() );
  props->setProfileSymbology( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) );
  props->setBandNumber( mBandComboBox->currentBand() );

  double fixedLower = std::numeric_limits< double >::lowest();
  double fixedUpper = std::numeric_limits< double >::max();
  if ( mFixedLowerSpinBox->value() != mFixedLowerSpinBox->clearValue() )
    fixedLower = mFixedLowerSpinBox->value();
  if ( mFixedUpperSpinBox->value() != mFixedUpperSpinBox->clearValue() )
    fixedUpper = mFixedUpperSpinBox->value();

  props->setFixedRange( QgsDoubleRange( fixedLower, fixedUpper, mLimitsComboBox->currentData().value< Qgis::RangeLimits >() ) );

  props->setFixedRangePerBand( mFixedRangePerBandModel->rangeData() );

  QgsPropertyCollection properties;
  properties.setProperty( QgsMapLayerElevationProperties::Property::RasterPerBandLowerElevation, QgsProperty::fromExpression( mLowerExpressionWidget->asExpression() ) );
  properties.setProperty( QgsMapLayerElevationProperties::Property::RasterPerBandUpperElevation, QgsProperty::fromExpression( mUpperExpressionWidget->asExpression() ) );
  props->setDataDefinedProperties( properties );

  mLayer->trigger3DUpdate();
}

void QgsRasterElevationPropertiesWidget::modeChanged()
{
  if ( mModeComboBox->currentData().isValid() )
  {
    switch ( mModeComboBox->currentData().value< Qgis::RasterElevationMode >() )
    {
      case Qgis::RasterElevationMode::FixedElevationRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::RasterElevationMode::RepresentsElevationSurface:
        mStackedWidget->setCurrentWidget( mPageSurface );
        break;
      case Qgis::RasterElevationMode::FixedRangePerBand:
        mStackedWidget->setCurrentWidget( mPageFixedRangePerBand );
        break;
      case Qgis::RasterElevationMode::DynamicRangePerBand:
        mStackedWidget->setCurrentWidget( mPageDynamicPerBand );
        break;
    }
    mProfileChartGroupBox->show();
  }
  else
  {
    mStackedWidget->setCurrentWidget( mPageDisabled );
    mProfileChartGroupBox->hide();
  }

  onChanged();
}

void QgsRasterElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

void QgsRasterElevationPropertiesWidget::calculateRangeByExpression( bool isUpper )
{
  QgsExpressionContext expressionContext;
  QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), 1, true, false, tr( "Band number" ) ) );
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), mLayer->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), mLayer->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );

  expressionContext.appendScope( bandScope );
  expressionContext.setHighlightedVariables( { QStringLiteral( "band" ), QStringLiteral( "band_name" ), QStringLiteral( "band_description" )} );

  QgsExpressionBuilderDialog dlg = QgsExpressionBuilderDialog( nullptr, isUpper ? mFixedRangeUpperExpression : mFixedRangeLowerExpression, this, QStringLiteral( "generic" ), expressionContext );

  QList<QPair<QString, QVariant> > bandChoices;
  for ( int band = 1; band <= mLayer->bandCount(); ++band )
  {
    bandChoices << qMakePair( mLayer->dataProvider()->displayBandName( band ), band );
  }
  dlg.expressionBuilder()->setCustomPreviewGenerator( tr( "Band" ), bandChoices, [this]( const QVariant & value )-> QgsExpressionContext
  {
    return createExpressionContextForBand( value.toInt() );
  } );

  if ( dlg.exec() )
  {
    if ( isUpper )
      mFixedRangeUpperExpression = dlg.expressionText();
    else
      mFixedRangeLowerExpression = dlg.expressionText();

    QgsExpression exp( dlg.expressionText() );
    exp.prepare( &expressionContext );
    for ( int band = 1; band <= mLayer->bandCount(); ++band )
    {
      bandScope->setVariable( QStringLiteral( "band" ), band );
      bandScope->setVariable( QStringLiteral( "band_name" ), mLayer->dataProvider()->displayBandName( band ) );
      bandScope->setVariable( QStringLiteral( "band_description" ), mLayer->dataProvider()->bandDescription( band ) );

      const QVariant res = exp.evaluate( &expressionContext );
      mFixedRangePerBandModel->setData( mFixedRangePerBandModel->index( band - 1, isUpper ? 2 : 1 ), res, Qt::EditRole );
    }
  }
}

QgsExpressionContext QgsRasterElevationPropertiesWidget::createExpressionContextForBand( int band ) const
{
  QgsExpressionContext context;
  context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), band, true, false, tr( "Band number" ) ) );
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), ( mLayer && mLayer->dataProvider() ) ? mLayer->dataProvider()->displayBandName( band ) : QString(), true, false, tr( "Band name" ) ) );
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), ( mLayer && mLayer->dataProvider() ) ? mLayer->dataProvider()->bandDescription( band ) : QString(), true, false, tr( "Band description" ) ) );
  context.appendScope( bandScope );
  context.setHighlightedVariables( { QStringLiteral( "band" ), QStringLiteral( "band_name" ), QStringLiteral( "band_description" )} );
  return context;
}


//
// QgsRasterElevationPropertiesWidgetFactory
//

QgsRasterElevationPropertiesWidgetFactory::QgsRasterElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsRasterElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsRasterElevationPropertiesWidget( qobject_cast< QgsRasterLayer * >( layer ), canvas, parent );
}

bool QgsRasterElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Raster;
}

QString QgsRasterElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

//
// QgsRasterBandFixedElevationRangeModel
//

QgsRasterBandFixedElevationRangeModel::QgsRasterBandFixedElevationRangeModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

int QgsRasterBandFixedElevationRangeModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

int QgsRasterBandFixedElevationRangeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mBandCount;
}

QModelIndex QgsRasterBandFixedElevationRangeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsRasterBandFixedElevationRangeModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

Qt::ItemFlags QgsRasterBandFixedElevationRangeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  if ( index.row() < 0 || index.row() >= mBandCount || index.column() < 0 || index.column() >= columnCount() )
    return Qt::ItemFlags();

  switch ( index.column() )
  {
    case 0:
      return Qt::ItemFlag::ItemIsEnabled;
    case 1:
    case 2:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable;
    default:
      break;
  }

  return Qt::ItemFlags();
}

QVariant QgsRasterBandFixedElevationRangeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= mBandCount || index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  const int band = index.row() + 1;
  const QgsDoubleRange range = mRanges.value( band );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case 0:
          return mBandNames.value( band, QString::number( band ) );

        case 1:
          return range.lower() > std::numeric_limits< double >::lowest() ? range.lower() : QVariant();

        case 2:
          return range.upper() < std::numeric_limits< double >::max() ? range.upper() : QVariant();

        default:
          break;
      }
      break;
    }

    case Qt::TextAlignmentRole:
    {
      switch ( index.column() )
      {
        case 0:
          return static_cast<Qt::Alignment::Int>( Qt::AlignLeft | Qt::AlignVCenter );

        case 1:
        case 2:
          return static_cast<Qt::Alignment::Int>( Qt::AlignRight | Qt::AlignVCenter );
        default:
          break;
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}

QVariant QgsRasterBandFixedElevationRangeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
  {
    switch ( section )
    {
      case 0:
        return tr( "Band" );
      case 1:
        return tr( "Lower" );
      case 2:
        return tr( "Upper" );
      default:
        break;
    }
  }
  return QAbstractItemModel::headerData( section, orientation, role );
}

bool QgsRasterBandFixedElevationRangeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.row() > mBandCount || index.row() < 0 )
    return false;

  const int band = index.row() + 1;
  const QgsDoubleRange range = mRanges.value( band );

  switch ( role )
  {
    case Qt::EditRole:
    {
      bool ok = false;
      double newValue = value.toDouble( &ok );
      if ( !ok )
        return false;

      switch ( index.column() )
      {
        case 1:
        {
          mRanges[band] = QgsDoubleRange( newValue, range.upper(), range.includeLower(), range.includeUpper() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        case 2:
          mRanges[band] = QgsDoubleRange( range.lower(), newValue, range.includeLower(), range.includeUpper() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;

        default:
          break;
      }
      return true;
    }

    default:
      break;
  }

  return false;
}

void QgsRasterBandFixedElevationRangeModel::setLayerData( QgsRasterLayer *layer, const QMap<int, QgsDoubleRange> &ranges )
{
  beginResetModel();

  mBandCount = layer->bandCount();
  mRanges = ranges;

  mBandNames.clear();
  for ( int band = 1; band <= mBandCount; ++band )
  {
    mBandNames[band] = layer->dataProvider()->displayBandName( band );
  }

  endResetModel();
}


//
// QgsRasterBandDynamicElevationRangeModel
//

QgsRasterBandDynamicElevationRangeModel::QgsRasterBandDynamicElevationRangeModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

int QgsRasterBandDynamicElevationRangeModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

int QgsRasterBandDynamicElevationRangeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() || !mLayer )
    return 0;
  return mLayer->bandCount();
}

QModelIndex QgsRasterBandDynamicElevationRangeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsRasterBandDynamicElevationRangeModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

Qt::ItemFlags QgsRasterBandDynamicElevationRangeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || !mLayer )
    return Qt::ItemFlags();

  if ( index.row() < 0 || index.row() >= mLayer->bandCount() || index.column() < 0 || index.column() >= columnCount() )
    return Qt::ItemFlags();

  switch ( index.column() )
  {
    case 0:
    case 1:
    case 2:

      return Qt::ItemFlag::ItemIsEnabled  | Qt::ItemFlag::ItemIsSelectable;;
    default:
      break;
  }

  return Qt::ItemFlags();
}

QVariant QgsRasterBandDynamicElevationRangeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mLayer )
    return QVariant();

  if ( index.row() < 0 || index.row() >= mLayer->bandCount() || index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  const int band = index.row() + 1;
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case 0:
          return mLayer->dataProvider() ? QVariant( mLayer->dataProvider()->displayBandName( band ) ) : QVariant( band );

        case 1:
        case 2:
        {
          const QString expressionString = index.column() == 1 ? mLowerExpression : mUpperExpression;
          QgsExpression expression( expressionString );

          QgsExpressionContext context;
          context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
          QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
          bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), band, true, false, tr( "Band number" ) ) );
          bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), ( mLayer && mLayer->dataProvider() ) ? mLayer->dataProvider()->displayBandName( band ) : QString(), true, false, tr( "Band name" ) ) );
          bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), ( mLayer && mLayer->dataProvider() ) ? mLayer->dataProvider()->bandDescription( band ) : QString(), true, false, tr( "Band description" ) ) );
          context.appendScope( bandScope );

          return expression.evaluate( &context );
        }

        default:
          break;
      }
      break;
    }

    case Qt::TextAlignmentRole:
    {
      switch ( index.column() )
      {
        case 0:
          return static_cast<Qt::Alignment::Int>( Qt::AlignLeft | Qt::AlignVCenter );

        case 1:
        case 2:
          return static_cast<Qt::Alignment::Int>( Qt::AlignRight | Qt::AlignVCenter );
        default:
          break;
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}

QVariant QgsRasterBandDynamicElevationRangeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
  {
    switch ( section )
    {
      case 0:
        return tr( "Band" );
      case 1:
        return tr( "Lower" );
      case 2:
        return tr( "Upper" );
      default:
        break;
    }
  }
  return QAbstractItemModel::headerData( section, orientation, role );
}

void QgsRasterBandDynamicElevationRangeModel::setLayer( QgsRasterLayer *layer )
{
  beginResetModel();
  mLayer = layer;
  endResetModel();
}

void QgsRasterBandDynamicElevationRangeModel::setLowerExpression( const QString &expression )
{
  mLowerExpression = expression;
  emit dataChanged( index( 0, 1 ), index( rowCount() - 1, 1 ) );
}

void QgsRasterBandDynamicElevationRangeModel::setUpperExpression( const QString &expression )
{
  mUpperExpression = expression;
  emit dataChanged( index( 0, 2 ), index( rowCount() - 1, 2 ) );
}


//
// QgsFixedElevationRangeDelegate
//

QgsFixedElevationRangeDelegate::QgsFixedElevationRangeDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

}

QWidget *QgsFixedElevationRangeDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsDoubleSpinBox *spin = new QgsDoubleSpinBox( parent );
  spin->setDecimals( 4 );
  spin->setMinimum( -9999999998.0 );
  spin->setMaximum( 9999999999.0 );
  spin->setShowClearButton( false );
  return spin;
}

void QgsFixedElevationRangeDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( QgsDoubleSpinBox *spin = qobject_cast< QgsDoubleSpinBox * >( editor ) )
  {
    model->setData( index, spin->value() );
  }
}
