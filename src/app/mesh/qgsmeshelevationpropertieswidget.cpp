/***************************************************************************
    qgsmeshelevationpropertieswidget.cpp
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

#include "qgsmeshelevationpropertieswidget.h"
#include "moc_qgsmeshelevationpropertieswidget.cpp"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include <QMenu>
#include <QAction>

QgsMeshElevationPropertiesWidget::QgsMeshElevationPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mModeComboBox->addItem( tr( "From Vertices" ), QVariant::fromValue( Qgis::MeshElevationMode::FromVertices ) );
  mModeComboBox->addItem( tr( "Fixed Elevation Range" ), QVariant::fromValue( Qgis::MeshElevationMode::FixedElevationRange ) );
  mModeComboBox->addItem( tr( "Fixed Elevation Range Per Group" ), QVariant::fromValue( Qgis::MeshElevationMode::FixedRangePerGroup ) );

  mLimitsComboBox->addItem( tr( "Include Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeBoth ) );
  mLimitsComboBox->addItem( tr( "Include Lower, Exclude Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeLowerExcludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower, Include Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeLowerIncludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeBoth ) );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );
  mSymbologyStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast<int>( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast<int>( Qgis::ProfileSurfaceSymbology::FillBelow ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillAbove.svg" ) ), tr( "Fill Above" ), static_cast<int>( Qgis::ProfileSurfaceSymbology::FillAbove ) );
  mElevationLimitSpinBox->setClearValue( mElevationLimitSpinBox->minimum(), tr( "Not set" ) );

  mFixedLowerSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedUpperSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedLowerSpinBox->clear();
  mFixedUpperSpinBox->clear();

  mFixedRangePerGroupModel = new QgsMeshGroupFixedElevationRangeModel( this );
  mGroupElevationTable->verticalHeader()->setVisible( false );
  mGroupElevationTable->setModel( mFixedRangePerGroupModel );
  QgsMeshFixedElevationRangeDelegate *tableDelegate = new QgsMeshFixedElevationRangeDelegate( mGroupElevationTable );
  mGroupElevationTable->setItemDelegateForColumn( 1, tableDelegate );
  mGroupElevationTable->setItemDelegateForColumn( 2, tableDelegate );

  QMenu *calculateFixedRangePerGroupMenu = new QMenu( mCalculateFixedRangePerGroupButton );
  mCalculateFixedRangePerGroupButton->setMenu( calculateFixedRangePerGroupMenu );
  mCalculateFixedRangePerGroupButton->setPopupMode( QToolButton::InstantPopup );
  QAction *calculateLowerAction = new QAction( "Calculate Lower by Expression…", calculateFixedRangePerGroupMenu );
  calculateFixedRangePerGroupMenu->addAction( calculateLowerAction );
  connect( calculateLowerAction, &QAction::triggered, this, [this] {
    calculateRangeByExpression( false );
  } );
  QAction *calculateUpperAction = new QAction( "Calculate Upper by Expression…", calculateFixedRangePerGroupMenu );
  calculateFixedRangePerGroupMenu->addAction( calculateUpperAction );
  connect( calculateUpperAction, &QAction::triggered, this, [this] {
    calculateRangeByExpression( true );
  } );

  syncToLayer( layer );

  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshElevationPropertiesWidget::modeChanged );
  connect( mOffsetZSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mElevationLimitSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mStyleComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=] {
    switch ( static_cast<Qgis::ProfileSurfaceSymbology>( mStyleComboBox->currentData().toInt() ) )
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

  setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#elevation-properties" ) );
}

void QgsMeshElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast<QgsMeshLayer *>( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  const QgsMeshLayerElevationProperties *props = qgis::down_cast<const QgsMeshLayerElevationProperties *>( mLayer->elevationProperties() );

  mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( props->mode() ) ) );
  switch ( props->mode() )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      mStackedWidget->setCurrentWidget( mPageFixedRange );
      break;
    case Qgis::MeshElevationMode::FromVertices:
      mStackedWidget->setCurrentWidget( mPageFromVertices );
      break;
    case Qgis::MeshElevationMode::FixedRangePerGroup:
      mStackedWidget->setCurrentWidget( mPageFixedRangePerGroup );
      break;
  }

  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  if ( std::isnan( props->elevationLimit() ) )
    mElevationLimitSpinBox->clear();
  else
    mElevationLimitSpinBox->setValue( props->elevationLimit() );

  if ( props->fixedRange().lower() != std::numeric_limits<double>::lowest() )
    mFixedLowerSpinBox->setValue( props->fixedRange().lower() );
  else
    mFixedLowerSpinBox->clear();
  if ( props->fixedRange().upper() != std::numeric_limits<double>::max() )
    mFixedUpperSpinBox->setValue( props->fixedRange().upper() );
  else
    mFixedUpperSpinBox->clear();
  mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( props->fixedRange().rangeLimits() ) ) );

  mFixedRangePerGroupModel->setLayerData( mLayer, props->fixedRangePerGroup() );
  mGroupElevationTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mGroupElevationTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mGroupElevationTable->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );

  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );

  mStyleComboBox->setCurrentIndex( mStyleComboBox->findData( static_cast<int>( props->profileSymbology() ) ) );
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

  mBlockUpdates = false;
}

void QgsMeshElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsMeshLayerElevationProperties *props = qgis::down_cast<QgsMeshLayerElevationProperties *>( mLayer->elevationProperties() );
  props->setMode( mModeComboBox->currentData().value<Qgis::MeshElevationMode>() );

  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  if ( mElevationLimitSpinBox->value() != mElevationLimitSpinBox->clearValue() )
    props->setElevationLimit( mElevationLimitSpinBox->value() );
  else
    props->setElevationLimit( std::numeric_limits<double>::quiet_NaN() );

  double fixedLower = std::numeric_limits<double>::lowest();
  double fixedUpper = std::numeric_limits<double>::max();
  if ( mFixedLowerSpinBox->value() != mFixedLowerSpinBox->clearValue() )
    fixedLower = mFixedLowerSpinBox->value();
  if ( mFixedUpperSpinBox->value() != mFixedUpperSpinBox->clearValue() )
    fixedUpper = mFixedUpperSpinBox->value();

  props->setFixedRange( QgsDoubleRange( fixedLower, fixedUpper, mLimitsComboBox->currentData().value<Qgis::RangeLimits>() ) );
  props->setFixedRangePerGroup( mFixedRangePerGroupModel->rangeData() );

  props->setProfileLineSymbol( mLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  props->setProfileFillSymbol( mFillStyleButton->clonedSymbol<QgsFillSymbol>() );
  props->setProfileSymbology( static_cast<Qgis::ProfileSurfaceSymbology>( mStyleComboBox->currentData().toInt() ) );
  mLayer->trigger3DUpdate();
}

void QgsMeshElevationPropertiesWidget::modeChanged()
{
  if ( mModeComboBox->currentData().isValid() )
  {
    switch ( mModeComboBox->currentData().value<Qgis::MeshElevationMode>() )
    {
      case Qgis::MeshElevationMode::FixedElevationRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::MeshElevationMode::FromVertices:
        mStackedWidget->setCurrentWidget( mPageFromVertices );
        break;
      case Qgis::MeshElevationMode::FixedRangePerGroup:
        mStackedWidget->setCurrentWidget( mPageFixedRangePerGroup );
        break;
    }
  }

  onChanged();
}

void QgsMeshElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

void QgsMeshElevationPropertiesWidget::calculateRangeByExpression( bool isUpper )
{
  QgsExpressionContext expressionContext;
  QgsExpressionContextScope *groupScope = new QgsExpressionContextScope();
  groupScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "group" ), 1, true, false, tr( "Group number" ) ) );
  const int groupIndex = mLayer->datasetGroupsIndexes().at( 0 );
  const QgsMeshDatasetGroupMetadata meta = mLayer->datasetGroupMetadata( groupIndex );
  groupScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "group_name" ), meta.name(), true, false, tr( "Group name" ) ) );

  expressionContext.appendScope( groupScope );
  expressionContext.setHighlightedVariables( { QStringLiteral( "group" ), QStringLiteral( "group_name" ) } );

  QgsExpressionBuilderDialog dlg = QgsExpressionBuilderDialog( nullptr, isUpper ? mFixedRangeUpperExpression : mFixedRangeLowerExpression, this, QStringLiteral( "generic" ), expressionContext );

  QList<QPair<QString, QVariant>> groupChoices;
  for ( int group = 0; group < mLayer->datasetGroupCount(); ++group )
  {
    const int groupIndex = mLayer->datasetGroupsIndexes().at( group );
    const QgsMeshDatasetGroupMetadata meta = mLayer->datasetGroupMetadata( groupIndex );
    groupChoices << qMakePair( meta.name(), group );
  }
  dlg.expressionBuilder()->setCustomPreviewGenerator( tr( "Group" ), groupChoices, [this]( const QVariant &value ) -> QgsExpressionContext {
    return createExpressionContextForGroup( value.toInt() );
  } );

  if ( dlg.exec() )
  {
    if ( isUpper )
      mFixedRangeUpperExpression = dlg.expressionText();
    else
      mFixedRangeLowerExpression = dlg.expressionText();

    QgsExpression exp( dlg.expressionText() );
    exp.prepare( &expressionContext );
    for ( int group = 0; group < mLayer->datasetGroupCount(); ++group )
    {
      groupScope->setVariable( QStringLiteral( "group" ), group + 1 );
      const int groupIndex = mLayer->datasetGroupsIndexes().at( group );
      const QgsMeshDatasetGroupMetadata meta = mLayer->datasetGroupMetadata( groupIndex );
      groupScope->setVariable( QStringLiteral( "group_name" ), meta.name() );

      const QVariant res = exp.evaluate( &expressionContext );
      mFixedRangePerGroupModel->setData( mFixedRangePerGroupModel->index( group, isUpper ? 2 : 1 ), res, Qt::EditRole );
    }
  }
}

QgsExpressionContext QgsMeshElevationPropertiesWidget::createExpressionContextForGroup( int group ) const
{
  QgsExpressionContext context;
  context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  QgsExpressionContextScope *groupScope = new QgsExpressionContextScope();
  groupScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "group" ), group + 1, true, false, tr( "Group number" ) ) );
  const int groupIndex = mLayer->datasetGroupsIndexes().at( group );
  const QgsMeshDatasetGroupMetadata meta = mLayer->datasetGroupMetadata( groupIndex );
  groupScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "group_name" ), meta.name(), true, false, tr( "Group name" ) ) );
  context.appendScope( groupScope );
  context.setHighlightedVariables( { QStringLiteral( "group" ), QStringLiteral( "group_name" ) } );
  return context;
}


//
// QgsMeshElevationPropertiesWidgetFactory
//

QgsMeshElevationPropertiesWidgetFactory::QgsMeshElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsMeshElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsMeshElevationPropertiesWidget( qobject_cast<QgsMeshLayer *>( layer ), canvas, parent );
}

bool QgsMeshElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsMeshElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsMeshElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Mesh;
}

QString QgsMeshElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}


//
// QgsMeshGroupFixedElevationRangeModel
//

QgsMeshGroupFixedElevationRangeModel::QgsMeshGroupFixedElevationRangeModel( QObject *parent )
  : QAbstractItemModel( parent )
{
}

int QgsMeshGroupFixedElevationRangeModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

int QgsMeshGroupFixedElevationRangeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mGroupCount;
}

QModelIndex QgsMeshGroupFixedElevationRangeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsMeshGroupFixedElevationRangeModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

Qt::ItemFlags QgsMeshGroupFixedElevationRangeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  if ( index.row() < 0 || index.row() >= mGroupCount || index.column() < 0 || index.column() >= columnCount() )
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

QVariant QgsMeshGroupFixedElevationRangeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= mGroupCount || index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  const int group = index.row();
  const QgsDoubleRange range = mRanges.value( group );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case 0:
          return mGroupNames.value( group, QString::number( group ) );

        case 1:
          return range.lower() > std::numeric_limits<double>::lowest() ? range.lower() : QVariant();

        case 2:
          return range.upper() < std::numeric_limits<double>::max() ? range.upper() : QVariant();

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

QVariant QgsMeshGroupFixedElevationRangeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
  {
    switch ( section )
    {
      case 0:
        return tr( "Group" );
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

bool QgsMeshGroupFixedElevationRangeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.row() >= mGroupCount || index.row() < 0 )
    return false;

  const int group = index.row();
  const QgsDoubleRange range = mRanges.value( group );

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
          mRanges[group] = QgsDoubleRange( newValue, range.upper(), range.includeLower(), range.includeUpper() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        case 2:
          mRanges[group] = QgsDoubleRange( range.lower(), newValue, range.includeLower(), range.includeUpper() );
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

void QgsMeshGroupFixedElevationRangeModel::setLayerData( QgsMeshLayer *layer, const QMap<int, QgsDoubleRange> &ranges )
{
  beginResetModel();

  mGroupCount = layer->datasetGroupCount();
  mRanges = ranges;

  mGroupNames.clear();
  for ( int group = 0; group < mGroupCount; ++group )
  {
    const int groupIndex = layer->datasetGroupsIndexes().at( group );
    const QgsMeshDatasetGroupMetadata meta = layer->datasetGroupMetadata( groupIndex );
    mGroupNames[group] = meta.name();
  }

  endResetModel();
}


//
// QgsFixedElevationRangeDelegate
//

QgsMeshFixedElevationRangeDelegate::QgsMeshFixedElevationRangeDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsMeshFixedElevationRangeDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsDoubleSpinBox *spin = new QgsDoubleSpinBox( parent );
  spin->setDecimals( 4 );
  spin->setMinimum( -9999999998.0 );
  spin->setMaximum( 9999999999.0 );
  spin->setShowClearButton( false );
  return spin;
}

void QgsMeshFixedElevationRangeDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( QgsDoubleSpinBox *spin = qobject_cast<QgsDoubleSpinBox *>( editor ) )
  {
    model->setData( index, spin->value() );
  }
}
