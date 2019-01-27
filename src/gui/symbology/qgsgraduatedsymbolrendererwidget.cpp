/***************************************************************************
    qgsgraduatedsymbolrendererwidget.cpp
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
#include "qgsgraduatedsymbolrendererwidget.h"
#include "qgspanelwidget.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgsdatadefinedsizelegendwidget.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgsstyle.h"

#include "qgsvectorlayer.h"

#include "qgssymbolselectordialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgslogger.h"

#include "qgsludialog.h"

#include "qgsproject.h"
#include "qgsmapcanvas.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPen>
#include <QPainter>

// ------------------------------ Model ------------------------------------

///@cond PRIVATE

QgsGraduatedSymbolRendererModel::QgsGraduatedSymbolRendererModel( QObject *parent ) : QAbstractItemModel( parent )
  , mMimeFormat( QStringLiteral( "application/x-qgsgraduatedsymbolrendererv2model" ) )
{
}

void QgsGraduatedSymbolRendererModel::setRenderer( QgsGraduatedSymbolRenderer *renderer )
{
  if ( mRenderer )
  {
    if ( mRenderer->ranges().size() )
    {
      beginRemoveRows( QModelIndex(), 0, mRenderer->ranges().size() - 1 );
      mRenderer = nullptr;
      endRemoveRows();
    }
    else
    {
      mRenderer = nullptr;
    }
  }
  if ( renderer )
  {
    if ( renderer->ranges().size() )
    {
      beginInsertRows( QModelIndex(), 0, renderer->ranges().size() - 1 );
      mRenderer = renderer;
      endInsertRows();
    }
    else
    {
      mRenderer = renderer;
    }
  }
}

void QgsGraduatedSymbolRendererModel::addClass( QgsSymbol *symbol )
{
  if ( !mRenderer ) return;
  int idx = mRenderer->ranges().size();
  beginInsertRows( QModelIndex(), idx, idx );
  mRenderer->addClass( symbol );
  endInsertRows();
}

void QgsGraduatedSymbolRendererModel::addClass( const QgsRendererRange &range )
{
  if ( !mRenderer )
  {
    return;
  }
  int idx = mRenderer->ranges().size();
  beginInsertRows( QModelIndex(), idx, idx );
  mRenderer->addClass( range );
  endInsertRows();
}

QgsRendererRange QgsGraduatedSymbolRendererModel::rendererRange( const QModelIndex &index )
{
  if ( !index.isValid() || !mRenderer || mRenderer->ranges().size() <= index.row() )
  {
    return QgsRendererRange();
  }

  return mRenderer->ranges().value( index.row() );
}

Qt::ItemFlags QgsGraduatedSymbolRendererModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;

  if ( index.column() == 2 )
  {
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

Qt::DropActions QgsGraduatedSymbolRendererModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QVariant QgsGraduatedSymbolRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mRenderer ) return QVariant();

  const QgsRendererRange range = mRenderer->ranges().value( index.row() );

  if ( role == Qt::CheckStateRole && index.column() == 0 )
  {
    return range.renderState() ? Qt::Checked : Qt::Unchecked;
  }
  else if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
  {
    switch ( index.column() )
    {
      case 1:
      {
        int decimalPlaces = mRenderer->labelFormat().precision() + 2;
        if ( decimalPlaces < 0 ) decimalPlaces = 0;
        return QLocale().toString( range.lowerValue(), 'f', decimalPlaces ) + " - " + QLocale().toString( range.upperValue(), 'f', decimalPlaces );
      }
      case 2:
        return range.label();
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && range.symbol() )
  {
    const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
    return QgsSymbolLayerUtils::symbolPreviewIcon( range.symbol(), QSize( iconSize, iconSize ) );
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return ( index.column() == 0 ) ? Qt::AlignHCenter : Qt::AlignLeft;
  }
  else if ( role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      // case 1: return rangeStr;
      case 2:
        return range.label();
      default:
        return QVariant();
    }
  }

  return QVariant();
}

bool QgsGraduatedSymbolRendererModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.column() == 0 && role == Qt::CheckStateRole )
  {
    mRenderer->updateRangeRenderState( index.row(), value == Qt::Checked );
    emit dataChanged( index, index );
    return true;
  }

  if ( role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case 1: // range
      return false; // range is edited in popup dialog
    case 2: // label
      mRenderer->updateRangeLabel( index.row(), value.toString() );
      break;
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

QVariant QgsGraduatedSymbolRendererModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 3 )
  {
    QStringList lst;
    lst << tr( "Symbol" ) << tr( "Values" ) << tr( "Legend" );
    return lst.value( section );
  }
  return QVariant();
}

int QgsGraduatedSymbolRendererModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() || !mRenderer )
  {
    return 0;
  }
  return mRenderer->ranges().size();
}

int QgsGraduatedSymbolRendererModel::columnCount( const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return 3;
}

QModelIndex QgsGraduatedSymbolRendererModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column );
  }
  return QModelIndex();
}

QModelIndex QgsGraduatedSymbolRendererModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return QModelIndex();
}

QStringList QgsGraduatedSymbolRendererModel::mimeTypes() const
{
  QStringList types;
  types << mMimeFormat;
  return types;
}

QMimeData *QgsGraduatedSymbolRendererModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  // Create list of rows
  Q_FOREACH ( const QModelIndex &index, indexes )
  {
    if ( !index.isValid() || index.column() != 0 )
      continue;

    stream << index.row();
  }
  mimeData->setData( mMimeFormat, encodedData );
  return mimeData;
}

bool QgsGraduatedSymbolRendererModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( row );
  Q_UNUSED( column );
  if ( action != Qt::MoveAction ) return true;

  if ( !data->hasFormat( mMimeFormat ) ) return false;

  QByteArray encodedData = data->data( mMimeFormat );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );

  QVector<int> rows;
  while ( !stream.atEnd() )
  {
    int r;
    stream >> r;
    rows.append( r );
  }

  int to = parent.row();
  // to is -1 if dragged outside items, i.e. below any item,
  // then move to the last position
  if ( to == -1 ) to = mRenderer->ranges().size(); // out of rang ok, will be decreased
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    QgsDebugMsg( QStringLiteral( "move %1 to %2" ).arg( rows[i] ).arg( to ) );
    int t = to;
    // moveCategory first removes and then inserts
    if ( rows[i] < t ) t--;
    mRenderer->moveClass( rows[i], t );
    // current moved under another, shift its index up
    for ( int j = 0; j < i; j++ )
    {
      if ( to < rows[j] && rows[i] > rows[j] ) rows[j] += 1;
    }
    // removed under 'to' so the target shifted down
    if ( rows[i] < to ) to--;
  }
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->ranges().size(), 0 ) );
  emit rowsMoved();
  return false;
}

void QgsGraduatedSymbolRendererModel::deleteRows( QList<int> rows )
{
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    beginRemoveRows( QModelIndex(), rows[i], rows[i] );
    mRenderer->deleteClass( rows[i] );
    endRemoveRows();
  }
}

void QgsGraduatedSymbolRendererModel::removeAllRows()
{
  beginRemoveRows( QModelIndex(), 0, mRenderer->ranges().size() - 1 );
  mRenderer->deleteAllClasses();
  endRemoveRows();
}

void QgsGraduatedSymbolRendererModel::sort( int column, Qt::SortOrder order )
{
  if ( column == 0 )
  {
    return;
  }
  if ( column == 1 )
  {
    mRenderer->sortByValue( order );
  }
  else if ( column == 2 )
  {
    mRenderer->sortByLabel( order );
  }
  emit rowsMoved();
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->ranges().size(), 0 ) );
}

void QgsGraduatedSymbolRendererModel::updateSymbology( bool resetModel )
{
  if ( resetModel )
  {
    reset();
  }
  else
  {
    emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->ranges().size(), 0 ) );
  }
}

void QgsGraduatedSymbolRendererModel::updateLabels()
{
  emit dataChanged( createIndex( 0, 2 ), createIndex( mRenderer->ranges().size(), 2 ) );
}

// ------------------------------ View style --------------------------------
QgsGraduatedSymbolRendererViewStyle::QgsGraduatedSymbolRendererViewStyle( QWidget *parent )
  : QgsProxyStyle( parent )
{}

void QgsGraduatedSymbolRendererViewStyle::drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const
{
  if ( element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull() )
  {
    QStyleOption opt( *option );
    opt.rect.setLeft( 0 );
    // draw always as line above, because we move item to that index
    opt.rect.setHeight( 0 );
    if ( widget ) opt.rect.setRight( widget->width() );
    QProxyStyle::drawPrimitive( element, &opt, painter, widget );
    return;
  }
  QProxyStyle::drawPrimitive( element, option, painter, widget );
}

///@endcond

// ------------------------------ Widget ------------------------------------

QgsRendererWidget *QgsGraduatedSymbolRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsGraduatedSymbolRendererWidget( layer, style, renderer );
}

QgsExpressionContext QgsGraduatedSymbolRendererWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( mContext.mapCanvas()->mapSettings() )
               << new QgsExpressionContextScope( mContext.mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( vectorLayer() );

  // additional scopes
  Q_FOREACH ( const QgsExpressionContextScope &scope, mContext.additionalExpressionContextScopes() )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  return expContext;
}

QgsGraduatedSymbolRendererWidget::QgsGraduatedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
{
  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( renderer )
  {
    mRenderer.reset( QgsGraduatedSymbolRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = qgis::make_unique< QgsGraduatedSymbolRenderer >( QString(), QgsRangeList() );
  }

  // setup user interface
  setupUi( this );

  cboGraduatedMode->addItem( tr( "Equal Interval" ), QgsGraduatedSymbolRenderer::EqualInterval );
  cboGraduatedMode->addItem( tr( "Quantile (Equal Count)" ), QgsGraduatedSymbolRenderer::Quantile );
  cboGraduatedMode->addItem( tr( "Natural Breaks (Jenks)" ), QgsGraduatedSymbolRenderer::Jenks );
  cboGraduatedMode->addItem( tr( "Standard Deviation" ), QgsGraduatedSymbolRenderer::StdDev );
  cboGraduatedMode->addItem( tr( "Pretty Breaks" ), QgsGraduatedSymbolRenderer::Pretty );

  connect( methodComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::methodComboBox_currentIndexChanged );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mModel = new QgsGraduatedSymbolRendererModel( this );

  mExpressionWidget->setFilters( QgsFieldProxyModel::Numeric | QgsFieldProxyModel::Date );
  mExpressionWidget->setLayer( mLayer );

  mSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                             << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  spinPrecision->setMinimum( QgsRendererRangeLabelFormat::MIN_PRECISION );
  spinPrecision->setMaximum( QgsRendererRangeLabelFormat::MAX_PRECISION );

  btnColorRamp->setShowRandomColorRamp( true );

  // set project default color ramp
  QString defaultColorRamp = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), QString() );
  if ( !defaultColorRamp.isEmpty() )
  {
    btnColorRamp->setColorRampFromName( defaultColorRamp );
  }
  else
  {
    QgsColorRamp *ramp = new QgsGradientColorRamp( QColor( 255, 255, 255 ), QColor( 255, 0, 0 ) );
    btnColorRamp->setColorRamp( ramp );
    delete ramp;
  }


  viewGraduated->setStyle( new QgsGraduatedSymbolRendererViewStyle( viewGraduated ) );

  mGraduatedSymbol.reset( QgsSymbol::defaultSymbol( mLayer->geometryType() ) );

  methodComboBox->blockSignals( true );
  methodComboBox->addItem( QStringLiteral( "Color" ) );
  if ( mGraduatedSymbol->type() == QgsSymbol::Marker )
  {
    methodComboBox->addItem( QStringLiteral( "Size" ) );
    minSizeSpinBox->setValue( 1 );
    maxSizeSpinBox->setValue( 8 );
  }
  else if ( mGraduatedSymbol->type() == QgsSymbol::Line )
  {
    methodComboBox->addItem( QStringLiteral( "Size" ) );
    minSizeSpinBox->setValue( .1 );
    maxSizeSpinBox->setValue( 2 );
  }
  methodComboBox->blockSignals( false );

  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsGraduatedSymbolRendererWidget::graduatedColumnChanged );
  connect( viewGraduated, &QAbstractItemView::doubleClicked, this, &QgsGraduatedSymbolRendererWidget::rangesDoubleClicked );
  connect( viewGraduated, &QAbstractItemView::clicked, this, &QgsGraduatedSymbolRendererWidget::rangesClicked );
  connect( viewGraduated, &QTreeView::customContextMenuRequested,  this, &QgsGraduatedSymbolRendererWidget::contextMenuViewCategories );

  connect( btnGraduatedClassify, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( btnChangeGraduatedSymbol, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::changeGraduatedSymbol );
  connect( btnGraduatedDelete, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::deleteClasses );
  connect( btnDeleteAllClasses, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::deleteAllClasses );
  connect( btnGraduatedAdd, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::addClass );
  connect( cbxLinkBoundaries, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::toggleBoundariesLink );
  connect( mSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsGraduatedSymbolRendererWidget::mSizeUnitWidget_changed );

  connectUpdateHandlers();

  // initialize from previously set renderer
  updateUiFromRenderer();

  // default to collapsed symmetric group for ui simplicity
  mGroupBoxSymmetric->setCollapsed( true ); //

  // menus for data-defined rotation/size
  QMenu *advMenu = new QMenu( this );

  advMenu->addAction( tr( "Symbol Levels…" ), this, SLOT( showSymbolLevels() ) );
  if ( mGraduatedSymbol->type() == QgsSymbol::Marker )
  {
    QAction *actionDdsLegend = advMenu->addAction( tr( "Data-defined Size Legend…" ) );
    // only from Qt 5.6 there is convenience addAction() with new style connection
    connect( actionDdsLegend, &QAction::triggered, this, &QgsGraduatedSymbolRendererWidget::dataDefinedSizeLegend );
  }

  btnAdvanced->setMenu( advMenu );

  mHistogramWidget->setLayer( mLayer );
  mHistogramWidget->setRenderer( mRenderer.get() );
  connect( mHistogramWidget, &QgsGraduatedHistogramWidget::rangesModified, this, &QgsGraduatedSymbolRendererWidget::refreshRanges );
  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), mHistogramWidget, &QgsHistogramWidget::setSourceFieldExp );

  mExpressionWidget->registerExpressionContextGenerator( this );
}

void QgsGraduatedSymbolRendererWidget::mSizeUnitWidget_changed()
{
  if ( !mGraduatedSymbol ) return;
  mGraduatedSymbol->setOutputUnit( mSizeUnitWidget->unit() );
  mGraduatedSymbol->setMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
  updateGraduatedSymbolIcon();
  mRenderer->updateSymbols( mGraduatedSymbol.get() );
  refreshSymbolView();
}

QgsGraduatedSymbolRendererWidget::~QgsGraduatedSymbolRendererWidget()
{
  delete mModel;
}

QgsFeatureRenderer *QgsGraduatedSymbolRendererWidget::renderer()
{
  return mRenderer.get();
}

// Connect/disconnect event handlers which trigger updating renderer
void QgsGraduatedSymbolRendererWidget::connectUpdateHandlers()
{
  connect( spinGraduatedClasses, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cboGraduatedMode, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsGraduatedSymbolRendererWidget::reapplyColorRamp );
  connect( spinPrecision, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  connect( cbxTrimTrailingZeroes, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  connect( txtLegendFormat, &QLineEdit::textChanged, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  connect( minSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );
  connect( maxSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );

  connect( mModel, &QgsGraduatedSymbolRendererModel::rowsMoved, this, &QgsGraduatedSymbolRendererWidget::rowsMoved );
  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsGraduatedSymbolRendererWidget::modelDataChanged );

  connect( mGroupBoxSymmetric, &QGroupBox::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cbxAstride, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cboSymmetryPointForPretty, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( spinSymmetryPointForOtherMethods, static_cast<void( QgsDoubleSpinBox::* )()>( &QgsDoubleSpinBox::editingFinished ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
}

// Connect/disconnect event handlers which trigger updating renderer
void QgsGraduatedSymbolRendererWidget::disconnectUpdateHandlers()
{
  disconnect( spinGraduatedClasses, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cboGraduatedMode, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsGraduatedSymbolRendererWidget::reapplyColorRamp );
  disconnect( spinPrecision, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  disconnect( cbxTrimTrailingZeroes, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  disconnect( txtLegendFormat, &QLineEdit::textChanged, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  disconnect( minSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );
  disconnect( maxSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );

  disconnect( mModel, &QgsGraduatedSymbolRendererModel::rowsMoved, this, &QgsGraduatedSymbolRendererWidget::rowsMoved );
  disconnect( mModel, &QAbstractItemModel::dataChanged, this, &QgsGraduatedSymbolRendererWidget::modelDataChanged );

  disconnect( mGroupBoxSymmetric, &QGroupBox::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cbxAstride, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cboSymmetryPointForPretty, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( spinSymmetryPointForOtherMethods, static_cast<void( QgsDoubleSpinBox::* )()>( &QgsDoubleSpinBox::editingFinished ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
}

void QgsGraduatedSymbolRendererWidget::updateUiFromRenderer( bool updateCount )
{
  disconnectUpdateHandlers();
  updateGraduatedSymbolIcon();
  spinSymmetryPointForOtherMethods->setShowClearButton( false );

  // update UI from the graduated renderer (update combo boxes, view)
  if ( cboGraduatedMode->findData( mRenderer->mode() ) >= 0 )
  {
    cboGraduatedMode->setCurrentIndex( cboGraduatedMode->findData( mRenderer->mode() ) );
  }

  // symmetric classification
  const QgsGraduatedSymbolRenderer::Mode cboMode = static_cast< QgsGraduatedSymbolRenderer::Mode >( cboGraduatedMode->currentData().toInt() );
  switch ( cboMode )
  {
    case QgsGraduatedSymbolRenderer::EqualInterval:
    case QgsGraduatedSymbolRenderer::StdDev:
    {
      mGroupBoxSymmetric->setVisible( true );
      cbxAstride->setVisible( true );
      cboSymmetryPointForPretty->setVisible( false );
      spinSymmetryPointForOtherMethods->setVisible( true );
      spinSymmetryPointForOtherMethods->setValue( mRenderer->symmetryPoint() );
      break;
    }

    case QgsGraduatedSymbolRenderer::Pretty:
    {
      mGroupBoxSymmetric->setVisible( true );
      cbxAstride->setVisible( true );
      spinSymmetryPointForOtherMethods->setVisible( false );
      cboSymmetryPointForPretty->setVisible( true );
      cboSymmetryPointForPretty->clear();
      cboSymmetryPointForPretty->addItems( mRenderer->listForCboPrettyBreaks() );
      // replace the combobox on the good old value
      cboSymmetryPointForPretty->setCurrentText( QString::number( mRenderer->symmetryPoint(), 'f', 2 ) );
      break;
    }

    case QgsGraduatedSymbolRenderer::Quantile:
    case QgsGraduatedSymbolRenderer::Jenks:
    case QgsGraduatedSymbolRenderer::Custom:
    {
      mGroupBoxSymmetric->setVisible( false );
      cbxAstride->setVisible( false );
      cboSymmetryPointForPretty->setVisible( false );
      spinSymmetryPointForOtherMethods->setVisible( false );
      spinSymmetryPointForOtherMethods->setValue( mRenderer->symmetryPoint() );
      break;
    }
  }

  if ( mRenderer->useSymmetricMode() )
  {
    mGroupBoxSymmetric->setChecked( true );
    spinSymmetryPointForOtherMethods->setEnabled( true );
    cbxAstride->setEnabled( true );
    cboSymmetryPointForPretty->setEnabled( true );
  }
  else
  {
    mGroupBoxSymmetric->setChecked( false );
    spinSymmetryPointForOtherMethods->setEnabled( false );
    cbxAstride->setEnabled( false );
    cboSymmetryPointForPretty->setEnabled( false );
  }

  if ( mRenderer->astride() )
    cbxAstride->setChecked( true );
  else
    cbxAstride->setChecked( false );

  // Only update class count if different - otherwise typing value gets very messy
  int nclasses = mRenderer->ranges().count();
  if ( nclasses && updateCount )
    spinGraduatedClasses->setValue( mRenderer->ranges().count() );

  // set column
  QString attrName = mRenderer->classAttribute();
  mExpressionWidget->setField( attrName );
  mHistogramWidget->setSourceFieldExp( attrName );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    mGraduatedSymbol.reset( mRenderer->sourceSymbol()->clone() );
    updateGraduatedSymbolIcon();
  }

  mModel->setRenderer( mRenderer.get() );
  viewGraduated->setModel( mModel );

  if ( mGraduatedSymbol )
  {
    mSizeUnitWidget->blockSignals( true );
    mSizeUnitWidget->setUnit( mGraduatedSymbol->outputUnit() );
    mSizeUnitWidget->setMapUnitScale( mGraduatedSymbol->mapUnitScale() );
    mSizeUnitWidget->blockSignals( false );
  }

  // set source color ramp
  methodComboBox->blockSignals( true );
  if ( mRenderer->graduatedMethod() == QgsGraduatedSymbolRenderer::GraduatedColor )
  {
    methodComboBox->setCurrentIndex( 0 );
    if ( mRenderer->sourceColorRamp() )
    {
      btnColorRamp->setColorRamp( mRenderer->sourceColorRamp() );
    }
  }
  else
  {
    methodComboBox->setCurrentIndex( 1 );
    if ( !mRenderer->ranges().isEmpty() ) // avoid overriding default size with zeros
    {
      minSizeSpinBox->setValue( mRenderer->minSymbolSize() );
      maxSizeSpinBox->setValue( mRenderer->maxSymbolSize() );
    }
  }
  toggleMethodWidgets( methodComboBox->currentIndex() );
  methodComboBox->blockSignals( false );

  QgsRendererRangeLabelFormat labelFormat = mRenderer->labelFormat();
  txtLegendFormat->setText( labelFormat.format() );
  spinPrecision->setValue( labelFormat.precision() );
  cbxTrimTrailingZeroes->setChecked( labelFormat.trimTrailingZeroes() );

  viewGraduated->resizeColumnToContents( 0 );
  viewGraduated->resizeColumnToContents( 1 );
  viewGraduated->resizeColumnToContents( 2 );

  mHistogramWidget->refresh();

  connectUpdateHandlers();
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::graduatedColumnChanged( const QString &field )
{
  mRenderer->setClassAttribute( field );
}

void QgsGraduatedSymbolRendererWidget::methodComboBox_currentIndexChanged( int idx )
{
  toggleMethodWidgets( idx );
  if ( idx == 0 )
  {
    mRenderer->setGraduatedMethod( QgsGraduatedSymbolRenderer::GraduatedColor );
    QgsColorRamp *ramp = btnColorRamp->colorRamp();

    if ( !ramp )
    {
      QMessageBox::critical( this, tr( "Select Method" ), tr( "No color ramp defined." ) );
      return;
    }
    mRenderer->setSourceColorRamp( ramp );
    reapplyColorRamp();
  }
  else
  {
    lblColorRamp->setVisible( false );
    btnColorRamp->setVisible( false );
    lblSize->setVisible( true );
    minSizeSpinBox->setVisible( true );
    lblSize->setVisible( true );
    maxSizeSpinBox->setVisible( true );
    mSizeUnitWidget->setVisible( true );

    mRenderer->setGraduatedMethod( QgsGraduatedSymbolRenderer::GraduatedSize );
    reapplySizes();
  }
}

void QgsGraduatedSymbolRendererWidget::toggleMethodWidgets( int idx )
{
  if ( idx == 0 )
  {
    lblColorRamp->setVisible( true );
    btnColorRamp->setVisible( true );
    lblSize->setVisible( false );
    minSizeSpinBox->setVisible( false );
    lblSizeTo->setVisible( false );
    maxSizeSpinBox->setVisible( false );
    mSizeUnitWidget->setVisible( false );
  }
  else
  {
    lblColorRamp->setVisible( false );
    btnColorRamp->setVisible( false );
    lblSize->setVisible( true );
    minSizeSpinBox->setVisible( true );
    lblSizeTo->setVisible( true );
    maxSizeSpinBox->setVisible( true );
    mSizeUnitWidget->setVisible( true );
  }
}

void QgsGraduatedSymbolRendererWidget::refreshRanges( bool reset )
{
  if ( !mModel )
    return;

  mModel->updateSymbology( reset );
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::cleanUpSymbolSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *dlg = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !dlg )
    return;

  delete dlg->symbol();
}

void QgsGraduatedSymbolRendererWidget::updateSymbolsFromWidget()
{
  QgsSymbolSelectorWidget *dlg = qobject_cast<QgsSymbolSelectorWidget *>( sender() );
  mGraduatedSymbol.reset( dlg->symbol()->clone() );

  applyChangeToSymbol();
}

void QgsGraduatedSymbolRendererWidget::applyChangeToSymbol()
{
  mSizeUnitWidget->blockSignals( true );
  mSizeUnitWidget->setUnit( mGraduatedSymbol->outputUnit() );
  mSizeUnitWidget->setMapUnitScale( mGraduatedSymbol->mapUnitScale() );
  mSizeUnitWidget->blockSignals( false );

  QItemSelectionModel *m = viewGraduated->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );
  if ( m && !selectedIndexes.isEmpty() )
  {
    Q_FOREACH ( const QModelIndex &idx, selectedIndexes )
    {
      if ( idx.isValid() )
      {
        int rangeIdx = idx.row();
        QgsSymbol *newRangeSymbol = mGraduatedSymbol->clone();
        if ( selectedIndexes.count() > 1 )
        {
          //if updating multiple ranges, retain the existing range colors
          newRangeSymbol->setColor( mRenderer->ranges().at( rangeIdx ).symbol()->color() );
        }
        mRenderer->updateRangeSymbol( rangeIdx, newRangeSymbol );
      }
    }
  }
  else
  {
    mRenderer->updateSymbols( mGraduatedSymbol.get() );
  }

  refreshSymbolView();
  emit widgetChanged();
}


void QgsGraduatedSymbolRendererWidget::classifyGraduated()
{
  QString attrName = mExpressionWidget->currentField();
  int nclasses = spinGraduatedClasses->value();

  std::unique_ptr<QgsColorRamp> ramp( btnColorRamp->colorRamp() );
  if ( !ramp )
  {
    QMessageBox::critical( this, tr( "Apply Classification" ), tr( "No color ramp defined." ) );
    return;
  }

  QgsGraduatedSymbolRenderer::Mode mode;
  bool useSymmetricMode = false;
  bool astride = false;

  int attrNum = mLayer->fields().lookupField( attrName );
  double minimum = mLayer->minimumValue( attrNum ).toDouble();
  double maximum = mLayer->maximumValue( attrNum ).toDouble();
  spinSymmetryPointForOtherMethods->setMinimum( minimum );
  spinSymmetryPointForOtherMethods->setMaximum( maximum );
  spinSymmetryPointForOtherMethods->setDecimals( spinPrecision->value() );

  double symmetryPoint = spinSymmetryPointForOtherMethods->value();

  const QgsGraduatedSymbolRenderer::Mode cboMode = static_cast< QgsGraduatedSymbolRenderer::Mode >( cboGraduatedMode->currentData().toInt() );
  switch ( cboMode )
  {
    case QgsGraduatedSymbolRenderer::EqualInterval:
    {
      mode = QgsGraduatedSymbolRenderer::EqualInterval;
      // knowing that spinSymmetryPointForOtherMethods->value() is automatically put at minimum when out of min-max
      // using "(maximum-minimum)/100)" to avoid direct comparison of doubles
      if ( spinSymmetryPointForOtherMethods->value() < ( minimum + ( maximum - minimum ) / 100. ) || spinSymmetryPointForOtherMethods->value() > ( maximum - ( maximum - minimum ) / 100. ) )
        spinSymmetryPointForOtherMethods->setValue( minimum + ( maximum - minimum ) / 2. );

      if ( mGroupBoxSymmetric->isChecked() )
      {
        useSymmetricMode = true;
        symmetryPoint = spinSymmetryPointForOtherMethods->value();
        astride = cbxAstride->isChecked();
      }
      break;
    }

    case QgsGraduatedSymbolRenderer::Jenks:
    {
      mode = QgsGraduatedSymbolRenderer::Jenks;
      break;
    }

    case QgsGraduatedSymbolRenderer::StdDev:
    {
      mode = QgsGraduatedSymbolRenderer::StdDev;
      // knowing that spinSymmetryPointForOtherMethods->value() is automatically put at minimum when out of min-max
      // using "(maximum-minimum)/100)" to avoid direct comparison of doubles
      if ( spinSymmetryPointForOtherMethods->value() < ( minimum + ( maximum - minimum ) / 100. ) || spinSymmetryPointForOtherMethods->value() > ( maximum - ( maximum - minimum ) / 100. ) )
        spinSymmetryPointForOtherMethods->setValue( minimum + ( maximum - minimum ) / 2. );

      if ( mGroupBoxSymmetric->isChecked() )
      {
        useSymmetricMode = true;
        symmetryPoint = spinSymmetryPointForOtherMethods->value();
        astride = cbxAstride->isChecked();
      }
      break;
    }

    case QgsGraduatedSymbolRenderer::Pretty:
    {
      mode = QgsGraduatedSymbolRenderer::Pretty;
      if ( mGroupBoxSymmetric->isChecked() )
      {
        useSymmetricMode = true;
        astride = cbxAstride->isChecked();
        symmetryPoint = cboSymmetryPointForPretty->currentText().toDouble(); //selected number
      }
      break;
    }

    case QgsGraduatedSymbolRenderer::Quantile:
    case QgsGraduatedSymbolRenderer::Custom:
    {
      // default should be quantile for now
      mode = QgsGraduatedSymbolRenderer::Quantile; // Quantile
      break;
    }
  }

  // Jenks is n^2 complexity, warn for big dataset (more than 50k records)
  // and give the user the chance to cancel
  if ( QgsGraduatedSymbolRenderer::Jenks == mode && mLayer->featureCount() > 50000 )
  {
    if ( QMessageBox::Cancel == QMessageBox::question( this, tr( "Apply Classification" ), tr( "Natural break classification (Jenks) is O(n2) complexity, your classification may take a long time.\nPress cancel to abort breaks calculation or OK to continue." ), QMessageBox::Cancel, QMessageBox::Ok ) )
      return;
  }
  // create and set new renderer
  mRenderer->setClassAttribute( attrName );
  mRenderer->setMode( mode );
  mRenderer->setUseSymmetricMode( useSymmetricMode );
  mRenderer->setSymmetryPoint( symmetryPoint );
  mRenderer->setAstride( astride );

  if ( methodComboBox->currentIndex() == 0 )
  {
    if ( !ramp )
    {
      QMessageBox::critical( this, tr( "Apply Classification" ), tr( "No color ramp defined." ) );
      return;
    }
    mRenderer->setSourceColorRamp( ramp.release() );
  }
  else
  {
    mRenderer->setSourceColorRamp( nullptr );
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  mRenderer->updateClasses( mLayer, mode, nclasses, useSymmetricMode, symmetryPoint, astride );

  if ( methodComboBox->currentIndex() == 1 )
    mRenderer->setSymbolSizes( minSizeSpinBox->value(), maxSizeSpinBox->value() );

  mRenderer->calculateLabelPrecision();
  QApplication::restoreOverrideCursor();
  // PrettyBreaks and StdDev calculation don't generate exact
  // number of classes - leave user interface unchanged for these
  updateUiFromRenderer( false );
}

void QgsGraduatedSymbolRendererWidget::reapplyColorRamp()
{
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( !ramp )
    return;

  mRenderer->updateColorRamp( ramp.release() );
  mRenderer->updateSymbols( mGraduatedSymbol.get() );
  refreshSymbolView();
}

void QgsGraduatedSymbolRendererWidget::reapplySizes()
{
  mRenderer->setSymbolSizes( minSizeSpinBox->value(), maxSizeSpinBox->value() );
  mRenderer->updateSymbols( mGraduatedSymbol.get() );
  refreshSymbolView();
}

void QgsGraduatedSymbolRendererWidget::changeGraduatedSymbol()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  std::unique_ptr< QgsSymbol > newSymbol( mGraduatedSymbol->clone() );
  if ( panel && panel->dockMode() )
  {
    QgsSymbolSelectorWidget *dlg = new QgsSymbolSelectorWidget( newSymbol.release(), mStyle, mLayer, panel );
    dlg->setContext( mContext );

    connect( dlg, &QgsPanelWidget::widgetChanged, this, &QgsGraduatedSymbolRendererWidget::updateSymbolsFromWidget );
    connect( dlg, &QgsPanelWidget::panelAccepted, this, &QgsGraduatedSymbolRendererWidget::cleanUpSymbolSelector );
    connect( dlg, &QgsPanelWidget::panelAccepted, this, &QgsGraduatedSymbolRendererWidget::updateGraduatedSymbolIcon );
    panel->openPanel( dlg );
  }
  else
  {
    QgsSymbolSelectorDialog dlg( newSymbol.get(), mStyle, mLayer, panel );
    if ( !dlg.exec() || !newSymbol )
    {
      return;
    }

    mGraduatedSymbol = std::move( newSymbol );
    updateGraduatedSymbolIcon();
    applyChangeToSymbol();
  }
}

void QgsGraduatedSymbolRendererWidget::updateGraduatedSymbolIcon()
{
  if ( !mGraduatedSymbol )
    return;

  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mGraduatedSymbol.get(), btnChangeGraduatedSymbol->iconSize() );
  btnChangeGraduatedSymbol->setIcon( icon );
}

#if 0
int QgsRendererPropertiesDialog::currentRangeRow()
{
  QModelIndex idx = viewGraduated->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}
#endif

QList<int> QgsGraduatedSymbolRendererWidget::selectedClasses()
{
  QList<int> rows;
  QModelIndexList selectedRows = viewGraduated->selectionModel()->selectedRows();

  Q_FOREACH ( const QModelIndex &r, selectedRows )
  {
    if ( r.isValid() )
    {
      rows.append( r.row() );
    }
  }
  return rows;
}

QgsRangeList QgsGraduatedSymbolRendererWidget::selectedRanges()
{
  QgsRangeList selectedRanges;
  QModelIndexList selectedRows = viewGraduated->selectionModel()->selectedRows();
  QModelIndexList::const_iterator sIt = selectedRows.constBegin();

  for ( ; sIt != selectedRows.constEnd(); ++sIt )
  {
    selectedRanges.append( mModel->rendererRange( *sIt ) );
  }
  return selectedRanges;
}

void QgsGraduatedSymbolRendererWidget::rangesDoubleClicked( const QModelIndex &idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeRangeSymbol( idx.row() );
  if ( idx.isValid() && idx.column() == 1 )
    changeRange( idx.row() );
}

void QgsGraduatedSymbolRendererWidget::rangesClicked( const QModelIndex &idx )
{
  if ( !idx.isValid() )
    mRowSelected = -1;
  else
    mRowSelected = idx.row();
}

void QgsGraduatedSymbolRendererWidget::changeSelectedSymbols()
{
}

void QgsGraduatedSymbolRendererWidget::changeRangeSymbol( int rangeIdx )
{
  const QgsRendererRange &range = mRenderer->ranges()[rangeIdx];
  std::unique_ptr< QgsSymbol > newSymbol( range.symbol()->clone() );
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    // bit tricky here - the widget doesn't take ownership of the symbol. So we need it to last for the duration of the
    // panel's existence. Accordingly, just kinda give it ownership here, and clean up in cleanUpSymbolSelector
    QgsSymbolSelectorWidget *dlg = new QgsSymbolSelectorWidget( newSymbol.release(), mStyle, mLayer, panel );
    dlg->setContext( mContext );
    dlg->setPanelTitle( range.label() );
    connect( dlg, &QgsPanelWidget::widgetChanged, this, &QgsGraduatedSymbolRendererWidget::updateSymbolsFromWidget );
    connect( dlg, &QgsPanelWidget::panelAccepted, this, &QgsGraduatedSymbolRendererWidget::cleanUpSymbolSelector );
    openPanel( dlg );
  }
  else
  {
    QgsSymbolSelectorDialog dlg( newSymbol.get(), mStyle, mLayer, panel );
    dlg.setContext( mContext );
    if ( !dlg.exec() || !newSymbol )
    {
      return;
    }

    mGraduatedSymbol = std::move( newSymbol );
    applyChangeToSymbol();
  }
}

void QgsGraduatedSymbolRendererWidget::changeRange( int rangeIdx )
{
  QgsLUDialog dialog( this );

  const QgsRendererRange &range = mRenderer->ranges()[rangeIdx];
  // Add arbitrary 2 to number of decimal places to retain a bit extra.
  // Ensures users can see if legend is not completely honest!
  int decimalPlaces = mRenderer->labelFormat().precision() + 2;
  if ( decimalPlaces < 0 ) decimalPlaces = 0;
  dialog.setLowerValue( QLocale().toString( range.lowerValue(), 'f', decimalPlaces ) );
  dialog.setUpperValue( QLocale().toString( range.upperValue(), 'f', decimalPlaces ) );

  if ( dialog.exec() == QDialog::Accepted )
  {
    bool ok = false;
    double lowerValue = qgsPermissiveToDouble( dialog.lowerValue(), ok );
    if ( ! ok )
      lowerValue = 0.0;
    double upperValue = qgsPermissiveToDouble( dialog.upperValue(), ok );
    if ( ! ok )
      upperValue = 0.0;
    mRenderer->updateRangeUpperValue( rangeIdx, upperValue );
    mRenderer->updateRangeLowerValue( rangeIdx, lowerValue );

    //If the boundaries have to stay linked, we update the ranges above and below, as well as their label if needed
    if ( cbxLinkBoundaries->isChecked() )
    {
      if ( rangeIdx > 0 )
      {
        mRenderer->updateRangeUpperValue( rangeIdx - 1, lowerValue );
      }

      if ( rangeIdx < mRenderer->ranges().size() - 1 )
      {
        mRenderer->updateRangeLowerValue( rangeIdx + 1, upperValue );
      }
    }
  }
  mHistogramWidget->refresh();
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::addClass()
{
  mModel->addClass( mGraduatedSymbol.get() );
  mHistogramWidget->refresh();
}

void QgsGraduatedSymbolRendererWidget::deleteClasses()
{
  QList<int> classIndexes = selectedClasses();
  mModel->deleteRows( classIndexes );
  mHistogramWidget->refresh();
}

void QgsGraduatedSymbolRendererWidget::deleteAllClasses()
{
  mModel->removeAllRows();
  mHistogramWidget->refresh();
}

bool QgsGraduatedSymbolRendererWidget::rowsOrdered()
{
  const QgsRangeList &ranges = mRenderer->ranges();
  bool ordered = true;
  for ( int i = 1; i < ranges.size(); ++i )
  {
    if ( ranges[i] < ranges[i - 1] )
    {
      ordered = false;
      break;
    }
  }
  return ordered;
}

void QgsGraduatedSymbolRendererWidget::toggleBoundariesLink( bool linked )
{
  //If the checkbox controlling the link between boundaries was unchecked and we check it, we have to link the boundaries
  //This is done by updating all lower ranges to the upper value of the range above
  if ( linked )
  {
    if ( ! rowsOrdered() )
    {
      int result = QMessageBox::warning(
                     this,
                     tr( "Link Class Boundaries" ),
                     tr( "Rows will be reordered before linking boundaries. Continue?" ),
                     QMessageBox::Ok | QMessageBox::Cancel );
      if ( result != QMessageBox::Ok )
      {
        cbxLinkBoundaries->setChecked( false );
        return;
      }
      mRenderer->sortByValue();
    }

    // Ok to proceed
    for ( int i = 1; i < mRenderer->ranges().size(); ++i )
    {
      mRenderer->updateRangeLowerValue( i, mRenderer->ranges()[i - 1].upperValue() );
    }
    refreshSymbolView();
  }
}

void QgsGraduatedSymbolRendererWidget::changeCurrentValue( QStandardItem *item )
{
  if ( item->column() == 2 )
  {
    QString label = item->text();
    int idx = item->row();
    mRenderer->updateRangeLabel( idx, label );
  }
}

void QgsGraduatedSymbolRendererWidget::labelFormatChanged()
{
  QgsRendererRangeLabelFormat labelFormat = QgsRendererRangeLabelFormat(
        txtLegendFormat->text(),
        spinPrecision->value(),
        cbxTrimTrailingZeroes->isChecked() );
  mRenderer->setLabelFormat( labelFormat, true );
  mModel->updateLabels();
}


QList<QgsSymbol *> QgsGraduatedSymbolRendererWidget::selectedSymbols()
{
  QList<QgsSymbol *> selectedSymbols;

  QItemSelectionModel *m = viewGraduated->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );
  if ( m && !selectedIndexes.isEmpty() )
  {
    const QgsRangeList &ranges = mRenderer->ranges();
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      QStringList list = m->model()->data( *indexIt ).toString().split( ' ' );
      if ( list.size() < 3 )
      {
        continue;
      }
      // Not strictly necessary because the range should have been sanitized already
      // after user input, but being permissive never hurts
      bool ok = false;
      double lowerBound = qgsPermissiveToDouble( list.at( 0 ), ok );
      if ( ! ok )
        lowerBound = 0.0;
      double upperBound = qgsPermissiveToDouble( list.at( 2 ), ok );
      if ( ! ok )
        upperBound = 0.0;
      QgsSymbol *s = findSymbolForRange( lowerBound, upperBound, ranges );
      if ( s )
      {
        selectedSymbols.append( s );
      }
    }
  }
  return selectedSymbols;
}

QgsSymbol *QgsGraduatedSymbolRendererWidget::findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList &ranges ) const
{
  int decimalPlaces = mRenderer->labelFormat().precision() + 2;
  if ( decimalPlaces < 0 )
    decimalPlaces = 0;
  double precision = 1.0 / std::pow( 10, decimalPlaces );

  for ( QgsRangeList::const_iterator it = ranges.begin(); it != ranges.end(); ++it )
  {
    if ( qgsDoubleNear( lowerBound, it->lowerValue(), precision ) && qgsDoubleNear( upperBound, it->upperValue(), precision ) )
    {
      return it->symbol();
    }
  }
  return nullptr;
}

void QgsGraduatedSymbolRendererWidget::refreshSymbolView()
{
  if ( mModel )
  {
    mModel->updateSymbology();
  }
  mHistogramWidget->refresh();
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer.get() );
}

void QgsGraduatedSymbolRendererWidget::rowsMoved()
{
  viewGraduated->selectionModel()->clear();
  if ( ! rowsOrdered() )
  {
    cbxLinkBoundaries->setChecked( false );
  }
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::modelDataChanged()
{
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::keyPressEvent( QKeyEvent *event )
{
  if ( !event )
  {
    return;
  }

  if ( event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier )
  {
    mCopyBuffer.clear();
    mCopyBuffer = selectedRanges();
  }
  else if ( event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier )
  {
    QgsRangeList::const_iterator rIt = mCopyBuffer.constBegin();
    for ( ; rIt != mCopyBuffer.constEnd(); ++rIt )
    {
      mModel->addClass( *rIt );
    }
    emit widgetChanged();
  }
}

void QgsGraduatedSymbolRendererWidget::dataDefinedSizeLegend()
{
  QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( mGraduatedSymbol.get() ); // this should be only enabled for marker symbols
  QgsDataDefinedSizeLegendWidget *panel = createDataDefinedSizeLegendWidget( s, mRenderer->dataDefinedSizeLegend() );
  if ( panel )
  {
    connect( panel, &QgsPanelWidget::widgetChanged, this, [ = ]
    {
      mRenderer->setDataDefinedSizeLegend( panel->dataDefinedSizeLegend() );
      emit widgetChanged();
    } );
    openPanel( panel );  // takes ownership of the panel
  }
}
