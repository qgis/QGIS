/***************************************************************************
                         qgsrasterlayertemporalpropertieswidget.cpp
                         ------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayertemporalpropertieswidget.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsdatetimeedit.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include <QMenu>
#include <QAction>

QgsRasterLayerTemporalPropertiesWidget::QgsRasterLayerTemporalPropertiesWidget( QWidget *parent, QgsRasterLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  Q_ASSERT( mLayer );
  setupUi( this );

  // make a useful default expression for per band ranges, just to give users some hints about how to do this...
  mFixedRangeLowerExpression = QStringLiteral( "make_datetime(%1,1,1,0,0,0) + make_interval(days:=@band)" ).arg( QDate::currentDate().year() );
  mFixedRangeUpperExpression = QStringLiteral( "make_datetime(%1,1,1,23,59,59) + make_interval(days:=@band)" ).arg( QDate::currentDate().year() );

  mExtraWidgetLayout = new QVBoxLayout();
  mExtraWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mExtraWidgetLayout->addStretch();
  mExtraWidgetContainer->setLayout( mExtraWidgetLayout );

  if ( mLayer->dataProvider() && mLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    mModeComboBox->addItem( tr( "Automatic" ), QVariant::fromValue( Qgis::RasterTemporalMode::TemporalRangeFromDataProvider ) );
  }
  mModeComboBox->addItem( tr( "Fixed Time Range" ), QVariant::fromValue( Qgis::RasterTemporalMode::FixedTemporalRange ) );
  mModeComboBox->addItem( tr( "Fixed Time Range Per Band" ), QVariant::fromValue( Qgis::RasterTemporalMode::FixedRangePerBand ) );
  mModeComboBox->addItem( tr( "Redraw Layer Only" ), QVariant::fromValue( Qgis::RasterTemporalMode::RedrawLayerOnly ) );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mFixedRangePerBandModel = new QgsRasterBandFixedTemporalRangeModel( this );
  mBandRangesTable->verticalHeader()->setVisible( false );
  mBandRangesTable->setModel( mFixedRangePerBandModel );
  QgsFixedTemporalRangeDelegate *tableDelegate = new QgsFixedTemporalRangeDelegate( mBandRangesTable );
  mBandRangesTable->setItemDelegateForColumn( 1, tableDelegate );
  mBandRangesTable->setItemDelegateForColumn( 2, tableDelegate );

  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterLayerTemporalPropertiesWidget::modeChanged );

  connect( mTemporalGroupBox, &QGroupBox::toggled, this, &QgsRasterLayerTemporalPropertiesWidget::temporalGroupBoxChecked );

  mStartTemporalDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );
  mEndTemporalDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );

  QMenu *calculateFixedRangePerBandMenu = new QMenu( mCalculateFixedRangePerBandButton );
  mCalculateFixedRangePerBandButton->setMenu( calculateFixedRangePerBandMenu );
  mCalculateFixedRangePerBandButton->setPopupMode( QToolButton::InstantPopup );
  QAction *calculateLowerAction = new QAction( "Calculate Beginning by Expression…", calculateFixedRangePerBandMenu );
  calculateFixedRangePerBandMenu->addAction( calculateLowerAction );
  connect( calculateLowerAction, &QAction::triggered, this, [this]
  {
    calculateRangeByExpression( false );
  } );
  QAction *calculateUpperAction = new QAction( "Calculate End by Expression…", calculateFixedRangePerBandMenu );
  calculateFixedRangePerBandMenu->addAction( calculateUpperAction );
  connect( calculateUpperAction, &QAction::triggered, this, [this]
  {
    calculateRangeByExpression( true );
  } );


  syncToLayer();
}

void QgsRasterLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  mLayer->temporalProperties()->setIsActive( mTemporalGroupBox->isChecked() );

  QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< QgsRasterLayerTemporalProperties * >( mLayer->temporalProperties() );

  temporalProperties->setMode( mModeComboBox->currentData().value< Qgis::RasterTemporalMode >() );

  const QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                       mEndTemporalDateTimeEdit->dateTime() );
  temporalProperties->setFixedTemporalRange( normalRange );

  temporalProperties->setFixedRangePerBand( mFixedRangePerBandModel->rangeData() );

  for ( QgsMapLayerConfigWidget *widget : std::as_const( mExtraWidgets ) )
  {
    widget->apply();
  }
}

void QgsRasterLayerTemporalPropertiesWidget::syncToLayer()
{
  const QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< const QgsRasterLayerTemporalProperties * >( mLayer->temporalProperties() );
  mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( temporalProperties->mode() ) ) );
  switch ( temporalProperties->mode() )
  {
    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
      mStackedWidget->setCurrentWidget( mPageAutomatic );
      break;
    case Qgis::RasterTemporalMode::FixedTemporalRange:
      mStackedWidget->setCurrentWidget( mPageFixedRange );
      break;
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
      mStackedWidget->setCurrentWidget( mPageRedrawOnly );
      break;
    case Qgis::RasterTemporalMode::FixedRangePerBand:
      mStackedWidget->setCurrentWidget( mPageFixedRangePerBand );
      break;
  }

  mStartTemporalDateTimeEdit->setDateTime( temporalProperties->fixedTemporalRange().begin() );
  mEndTemporalDateTimeEdit->setDateTime( temporalProperties->fixedTemporalRange().end() );

  mFixedRangePerBandModel->setLayerData( mLayer, temporalProperties->fixedRangePerBand() );
  mBandRangesTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mBandRangesTable->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mBandRangesTable->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );

  mTemporalGroupBox->setChecked( temporalProperties->isActive() );

  for ( QgsMapLayerConfigWidget *widget : std::as_const( mExtraWidgets ) )
  {
    widget->syncToLayer( mLayer );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::addWidget( QgsMapLayerConfigWidget *widget )
{
  mExtraWidgets << widget;
  mExtraWidgetLayout->insertWidget( mExtraWidgetLayout->count() - 1, widget );
}

void QgsRasterLayerTemporalPropertiesWidget::temporalGroupBoxChecked( bool checked )
{
  for ( QgsMapLayerConfigWidget *widget : std::as_const( mExtraWidgets ) )
  {
    widget->emit dynamicTemporalControlToggled( checked );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::modeChanged()
{
  if ( mModeComboBox->currentData().isValid() )
  {
    switch ( mModeComboBox->currentData().value< Qgis::RasterTemporalMode >() )
    {
      case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
        mStackedWidget->setCurrentWidget( mPageAutomatic );
        break;
      case Qgis::RasterTemporalMode::FixedTemporalRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::RasterTemporalMode::RedrawLayerOnly:
        mStackedWidget->setCurrentWidget( mPageRedrawOnly );
        break;
      case Qgis::RasterTemporalMode::FixedRangePerBand:
        mStackedWidget->setCurrentWidget( mPageFixedRangePerBand );
        break;
    }
  }
}

void QgsRasterLayerTemporalPropertiesWidget::calculateRangeByExpression( bool isUpper )
{
  QgsExpressionContext expressionContext;
  QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band" ), 1, true, false, tr( "Band number" ) ) );
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_name" ), mLayer->dataProvider()->displayBandName( 1 ), true, false, tr( "Band name" ) ) );
  bandScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "band_description" ), mLayer->dataProvider()->bandDescription( 1 ), true, false, tr( "Band description" ) ) );

  expressionContext.appendScope( bandScope );
  expressionContext.setHighlightedVariables( { QStringLiteral( "band" ), QStringLiteral( "band_name" ), QStringLiteral( "band_description" )} );

  QgsExpressionBuilderDialog dlg = QgsExpressionBuilderDialog( nullptr, isUpper ? mFixedRangeUpperExpression : mFixedRangeLowerExpression, this, QStringLiteral( "generic" ), expressionContext );
  dlg.setExpectedOutputFormat( !isUpper ? tr( "Temporal range start date / time" ) : tr( "Temporal range end date / time" ) );

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

QgsExpressionContext QgsRasterLayerTemporalPropertiesWidget::createExpressionContextForBand( int band ) const
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

///@cond PRIVATE

//
// QgsRasterBandFixedTemporalRangeModel
//

QgsRasterBandFixedTemporalRangeModel::QgsRasterBandFixedTemporalRangeModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

int QgsRasterBandFixedTemporalRangeModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

int QgsRasterBandFixedTemporalRangeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mBandCount;
}

QModelIndex QgsRasterBandFixedTemporalRangeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsRasterBandFixedTemporalRangeModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

Qt::ItemFlags QgsRasterBandFixedTemporalRangeModel::flags( const QModelIndex &index ) const
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

QVariant QgsRasterBandFixedTemporalRangeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= mBandCount || index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  const int band = index.row() + 1;
  const QgsDateTimeRange range = mRanges.value( band );

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
          return range.begin().isValid() ? range.begin() : QVariant();

        case 2:
          return range.end().isValid() ? range.end() : QVariant();

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

QVariant QgsRasterBandFixedTemporalRangeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
  {
    switch ( section )
    {
      case 0:
        return tr( "Band" );
      case 1:
        return tr( "Begin" );
      case 2:
        return tr( "End" );
      default:
        break;
    }
  }
  return QAbstractItemModel::headerData( section, orientation, role );
}

bool QgsRasterBandFixedTemporalRangeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.row() > mBandCount || index.row() < 0 )
    return false;

  const int band = index.row() + 1;
  const QgsDateTimeRange range = mRanges.value( band );

  switch ( role )
  {
    case Qt::EditRole:
    {
      const QDateTime newValue = value.toDateTime();
      if ( !newValue.isValid() )
        return false;

      switch ( index.column() )
      {
        case 1:
        {
          mRanges[band] = QgsDateTimeRange( newValue, range.end(), range.includeBeginning(), range.includeEnd() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        case 2:
          mRanges[band] = QgsDateTimeRange( range.begin(), newValue, range.includeBeginning(), range.includeEnd() );
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

void QgsRasterBandFixedTemporalRangeModel::setLayerData( QgsRasterLayer *layer, const QMap<int, QgsDateTimeRange> &ranges )
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
// QgsFixedTemporalRangeDelegate
//

QgsFixedTemporalRangeDelegate::QgsFixedTemporalRangeDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

}

QWidget *QgsFixedTemporalRangeDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsDateTimeEdit *editor = new QgsDateTimeEdit( parent );
  editor->setAllowNull( true );
  return editor;
}

void QgsFixedTemporalRangeDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( QgsDateTimeEdit *dateTimeEdit = qobject_cast< QgsDateTimeEdit * >( editor ) )
  {
    model->setData( index, dateTimeEdit->dateTime() );
  }
}
///@endcond PRIVATE
