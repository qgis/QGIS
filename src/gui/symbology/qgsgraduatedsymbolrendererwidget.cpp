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

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPen>
#include <QPainter>
#include <QClipboard>
#include <QCompleter>

#include "qgsgraduatedsymbolrendererwidget.h"
#include "qgspanelwidget.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgsdatadefinedsizelegendwidget.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscolorrampimpl.h"
#include "qgscolorrampbutton.h"
#include "qgsstyle.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayer.h"
#include "qgssymbolselectordialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgslogger.h"
#include "qgsludialog.h"
#include "qgsproject.h"
#include "qgsprojectstylesettings.h"
#include "qgsmapcanvas.h"
#include "qgsclassificationmethod.h"
#include "qgsapplication.h"
#include "qgsclassificationmethodregistry.h"
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationstandarddeviation.h"
#include "qgsgui.h"
#include "qgsprocessinggui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgstemporalcontroller.h"
#include "qgsdoublevalidator.h"
#include "qgsmarkersymbol.h"


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
    if ( !mRenderer->ranges().isEmpty() )
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
    if ( !renderer->ranges().isEmpty() )
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
        int decimalPlaces = mRenderer->classificationMethod()->labelPrecision() + 2;
        if ( decimalPlaces < 0 ) decimalPlaces = 0;
        return QString( QLocale().toString( range.lowerValue(), 'f', decimalPlaces ) + " - " + QLocale().toString( range.upperValue(), 'f', decimalPlaces ) );
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
    return ( index.column() == 0 ) ? static_cast<Qt::Alignment::Int>( Qt::AlignHCenter ) : static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
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
  Q_UNUSED( index )
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
  Q_UNUSED( index )
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
  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
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
  Q_UNUSED( row )
  Q_UNUSED( column )
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

void QgsGraduatedSymbolRendererModel::updateSymbology()
{
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->ranges().size(), 0 ) );
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

  if ( auto *lMapCanvas = mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( lMapCanvas->mapSettings() )
               << new QgsExpressionContextScope( lMapCanvas->expressionContextScope() );
    if ( const QgsExpressionContextScopeGenerator *generator = dynamic_cast< const QgsExpressionContextScopeGenerator * >( lMapCanvas->temporalController() ) )
    {
      expContext << generator->createExpressionContextScope();
    }
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( auto *lVectorLayer = vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( lVectorLayer );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
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
    mRenderer = std::make_unique< QgsGraduatedSymbolRenderer >( QString(), QgsRangeList() );
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  // setup user interface
  setupUi( this );

  mSymmetryPointValidator = new QgsDoubleValidator( this );
  cboSymmetryPoint->setEditable( true );
  cboSymmetryPoint->setValidator( mSymmetryPointValidator );

  const QMap<QString, QString> methods = QgsApplication::classificationMethodRegistry()->methodNames();
  for ( QMap<QString, QString>::const_iterator it = methods.constBegin(); it != methods.constEnd(); ++it )
  {
    QIcon icon = QgsApplication::classificationMethodRegistry()->icon( it.value() );
    cboGraduatedMode->addItem( icon, it.key(), it.value() );
  }

  connect( methodComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::methodComboBox_currentIndexChanged );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mModel = new QgsGraduatedSymbolRendererModel( this );

  mExpressionWidget->setFilters( QgsFieldProxyModel::Numeric | QgsFieldProxyModel::Date );
  mExpressionWidget->setLayer( mLayer );

  btnChangeGraduatedSymbol->setLayer( mLayer );
  btnChangeGraduatedSymbol->registerExpressionContextGenerator( this );

  mSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                             << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  spinPrecision->setMinimum( QgsClassificationMethod::MIN_PRECISION );
  spinPrecision->setMaximum( QgsClassificationMethod::MAX_PRECISION );
  spinPrecision->setClearValue( 4 );

  spinGraduatedClasses->setShowClearButton( false );

  btnColorRamp->setShowRandomColorRamp( true );

  // set project default color ramp
  std::unique_ptr< QgsColorRamp > colorRamp( QgsProject::instance()->styleSettings()->defaultColorRamp() );
  if ( colorRamp )
  {
    btnColorRamp->setColorRamp( colorRamp.get() );
  }
  else
  {
    QgsColorRamp *ramp = new QgsGradientColorRamp( QColor( 255, 255, 255 ), QColor( 255, 0, 0 ) );
    btnColorRamp->setColorRamp( ramp );
    delete ramp;
  }


  viewGraduated->setStyle( new QgsGraduatedSymbolRendererViewStyle( viewGraduated ) );

  mGraduatedSymbol.reset( QgsSymbol::defaultSymbol( mLayer->geometryType() ) );
  if ( mGraduatedSymbol )
  {
    btnChangeGraduatedSymbol->setSymbolType( mGraduatedSymbol->type() );
    btnChangeGraduatedSymbol->setSymbol( mGraduatedSymbol->clone() );

    methodComboBox->blockSignals( true );
    methodComboBox->addItem( tr( "Color" ), ColorMode );
    switch ( mGraduatedSymbol->type() )
    {
      case Qgis::SymbolType::Marker:
      {
        methodComboBox->addItem( tr( "Size" ), SizeMode );
        minSizeSpinBox->setValue( 1 );
        maxSizeSpinBox->setValue( 8 );
        break;
      }
      case Qgis::SymbolType::Line:
      {
        methodComboBox->addItem( tr( "Size" ), SizeMode );
        minSizeSpinBox->setValue( .1 );
        maxSizeSpinBox->setValue( 2 );
        break;
      }
      case Qgis::SymbolType::Fill:
      {
        //set button and label invisible to avoid display of a single item combobox
        methodComboBox->hide();
        labelMethod->hide();
        break;
      }
      case Qgis::SymbolType::Hybrid:
        break;
    }
    methodComboBox->blockSignals( false );
  }

  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsGraduatedSymbolRendererWidget::graduatedColumnChanged );
  connect( viewGraduated, &QAbstractItemView::doubleClicked, this, &QgsGraduatedSymbolRendererWidget::rangesDoubleClicked );
  connect( viewGraduated, &QAbstractItemView::clicked, this, &QgsGraduatedSymbolRendererWidget::rangesClicked );
  connect( viewGraduated, &QTreeView::customContextMenuRequested,  this, &QgsGraduatedSymbolRendererWidget::contextMenuViewCategories );

  connect( btnGraduatedClassify, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( btnChangeGraduatedSymbol, &QgsSymbolButton::changed, this, &QgsGraduatedSymbolRendererWidget::changeGraduatedSymbol );
  connect( btnGraduatedDelete, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::deleteClasses );
  connect( btnDeleteAllClasses, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::deleteAllClasses );
  connect( btnGraduatedAdd, &QAbstractButton::clicked, this, &QgsGraduatedSymbolRendererWidget::addClass );
  connect( cbxLinkBoundaries, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::toggleBoundariesLink );
  connect( mSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsGraduatedSymbolRendererWidget::mSizeUnitWidget_changed );

  connect( cboGraduatedMode, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::updateMethodParameters );

  connectUpdateHandlers();

  // initialize from previously set renderer
  updateUiFromRenderer();

  // default to collapsed symmetric group for ui simplicity
  mGroupBoxSymmetric->setCollapsed( true ); //

  // menus for data-defined rotation/size
  QMenu *advMenu = new QMenu( this );

  mActionLevels = advMenu->addAction( tr( "Symbol Levels…" ), this, &QgsGraduatedSymbolRendererWidget::showSymbolLevels );
  if ( mGraduatedSymbol && mGraduatedSymbol->type() == Qgis::SymbolType::Marker )
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
  if ( !mGraduatedSymbol )
    return;
  mGraduatedSymbol->setOutputUnit( mSizeUnitWidget->unit() );
  mGraduatedSymbol->setMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
  mRenderer->updateSymbols( mGraduatedSymbol.get() );
  refreshSymbolView();
}

QgsGraduatedSymbolRendererWidget::~QgsGraduatedSymbolRendererWidget()
{
  delete mModel;
  mParameterWidgetWrappers.clear();
}

QgsFeatureRenderer *QgsGraduatedSymbolRendererWidget::renderer()
{
  return mRenderer.get();
}

void QgsGraduatedSymbolRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  btnChangeGraduatedSymbol->setMapCanvas( context.mapCanvas() );
  btnChangeGraduatedSymbol->setMessageBar( context.messageBar() );
}

void QgsGraduatedSymbolRendererWidget::disableSymbolLevels()
{
  delete mActionLevels;
  mActionLevels = nullptr;
}

// Connect/disconnect event handlers which trigger updating renderer
void QgsGraduatedSymbolRendererWidget::connectUpdateHandlers()
{
  connect( spinGraduatedClasses, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cboGraduatedMode, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsGraduatedSymbolRendererWidget::reapplyColorRamp );
  connect( spinPrecision, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  connect( cbxTrimTrailingZeroes, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  connect( txtLegendFormat, &QLineEdit::textChanged, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  connect( minSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );
  connect( maxSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );

  connect( mModel, &QgsGraduatedSymbolRendererModel::rowsMoved, this, &QgsGraduatedSymbolRendererWidget::rowsMoved );
  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsGraduatedSymbolRendererWidget::modelDataChanged );

  connect( mGroupBoxSymmetric, &QGroupBox::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cbxAstride, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cboSymmetryPoint, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  connect( cboSymmetryPoint->lineEdit(), &QLineEdit::editingFinished, this, &QgsGraduatedSymbolRendererWidget::symmetryPointEditingFinished );

  for ( const auto &ppww : std::as_const( mParameterWidgetWrappers ) )
  {
    connect( ppww.get(), &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  }
}

// Connect/disconnect event handlers which trigger updating renderer
void QgsGraduatedSymbolRendererWidget::disconnectUpdateHandlers()
{
  disconnect( spinGraduatedClasses, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cboGraduatedMode, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsGraduatedSymbolRendererWidget::reapplyColorRamp );
  disconnect( spinPrecision, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  disconnect( cbxTrimTrailingZeroes, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  disconnect( txtLegendFormat, &QLineEdit::textChanged, this, &QgsGraduatedSymbolRendererWidget::labelFormatChanged );
  disconnect( minSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );
  disconnect( maxSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsGraduatedSymbolRendererWidget::reapplySizes );

  disconnect( mModel, &QgsGraduatedSymbolRendererModel::rowsMoved, this, &QgsGraduatedSymbolRendererWidget::rowsMoved );
  disconnect( mModel, &QAbstractItemModel::dataChanged, this, &QgsGraduatedSymbolRendererWidget::modelDataChanged );

  disconnect( mGroupBoxSymmetric, &QGroupBox::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cbxAstride, &QAbstractButton::toggled, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cboSymmetryPoint, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  disconnect( cboSymmetryPoint->lineEdit(), &QLineEdit::editingFinished, this, &QgsGraduatedSymbolRendererWidget::symmetryPointEditingFinished );

  for ( const auto &ppww : std::as_const( mParameterWidgetWrappers ) )
  {
    disconnect( ppww.get(), &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );
  }
}

void QgsGraduatedSymbolRendererWidget::updateUiFromRenderer( bool updateCount )
{
  disconnectUpdateHandlers();
  mBlockUpdates++;

  const QgsClassificationMethod *method = mRenderer->classificationMethod();

  const QgsRangeList ranges = mRenderer->ranges();

  // use the breaks for symmetry point
  int precision = spinPrecision->value() + 2;
  while ( cboSymmetryPoint->count() )
    cboSymmetryPoint->removeItem( 0 );
  for ( int i = 0; i < ranges.count() - 1; i++ )
    cboSymmetryPoint->addItem( QLocale().toString( ranges.at( i ).upperValue(), 'f', precision ), ranges.at( i ).upperValue() );

  if ( method )
  {
    int idx = cboGraduatedMode->findData( method->id() );
    if ( idx >= 0 )
      cboGraduatedMode->setCurrentIndex( idx );

    mGroupBoxSymmetric->setVisible( method->symmetricModeAvailable() );
    mGroupBoxSymmetric->setChecked( method->symmetricModeEnabled() );
    cbxAstride->setChecked( method->symmetryAstride() );
    if ( method->symmetricModeEnabled() )
      cboSymmetryPoint->setItemText( cboSymmetryPoint->currentIndex(), QLocale().toString( method->symmetryPoint(), 'f', method->labelPrecision() + 2 ) );

    txtLegendFormat->setText( method->labelFormat() );
    spinPrecision->setValue( method->labelPrecision() );
    cbxTrimTrailingZeroes->setChecked( method->labelTrimTrailingZeroes() );

    QgsProcessingContext context;
    for ( const auto &ppww : std::as_const( mParameterWidgetWrappers ) )
    {
      const QgsProcessingParameterDefinition *def = ppww->parameterDefinition();
      QVariant value = method->parameterValues().value( def->name(), def->defaultValueForGui() );
      ppww->setParameterValue( value, context );
    }
  }

  // Only update class count if different - otherwise typing value gets very messy
  int nclasses = ranges.count();
  if ( nclasses && ( updateCount || ( method && ( method->flags() & QgsClassificationMethod::MethodProperty::IgnoresClassCount ) ) ) )
  {
    spinGraduatedClasses->setValue( ranges.count() );
  }
  if ( method )
  {
    spinGraduatedClasses->setEnabled( !( method->flags() & QgsClassificationMethod::MethodProperty::IgnoresClassCount ) );
  }
  else
  {
    spinGraduatedClasses->setEnabled( true );
  }

  // set column
  QString attrName = mRenderer->classAttribute();
  mExpressionWidget->setField( attrName );
  mHistogramWidget->setSourceFieldExp( attrName );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    mGraduatedSymbol.reset( mRenderer->sourceSymbol()->clone() );
    whileBlocking( btnChangeGraduatedSymbol )->setSymbol( mGraduatedSymbol->clone() );
  }

  mModel->setRenderer( mRenderer.get() );
  viewGraduated->setModel( mModel );

  connect( viewGraduated->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsGraduatedSymbolRendererWidget::selectionChanged );

  if ( mGraduatedSymbol )
  {
    mSizeUnitWidget->blockSignals( true );
    mSizeUnitWidget->setUnit( mGraduatedSymbol->outputUnit() );
    mSizeUnitWidget->setMapUnitScale( mGraduatedSymbol->mapUnitScale() );
    mSizeUnitWidget->blockSignals( false );
  }

  // set source color ramp
  methodComboBox->blockSignals( true );
  switch ( mRenderer->graduatedMethod() )
  {
    case Qgis::GraduatedMethod::Color:
    {
      methodComboBox->setCurrentIndex( methodComboBox->findData( ColorMode ) );
      if ( mRenderer->sourceColorRamp() )
      {
        btnColorRamp->setColorRamp( mRenderer->sourceColorRamp() );
      }
      break;
    }
    case Qgis::GraduatedMethod::Size:
    {
      methodComboBox->setCurrentIndex( methodComboBox->findData( SizeMode ) );
      if ( !mRenderer->ranges().isEmpty() ) // avoid overriding default size with zeros
      {
        minSizeSpinBox->setValue( mRenderer->minSymbolSize() );
        maxSizeSpinBox->setValue( mRenderer->maxSymbolSize() );
      }
      break;
    }
  }
  toggleMethodWidgets( static_cast< MethodMode>( methodComboBox->currentData().toInt() ) );
  methodComboBox->blockSignals( false );

  viewGraduated->resizeColumnToContents( 0 );
  viewGraduated->resizeColumnToContents( 1 );
  viewGraduated->resizeColumnToContents( 2 );

  mHistogramWidget->refresh();

  connectUpdateHandlers();
  mBlockUpdates--;

  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::graduatedColumnChanged( const QString &field )
{
  mRenderer->setClassAttribute( field );
}

void QgsGraduatedSymbolRendererWidget::methodComboBox_currentIndexChanged( int )
{
  const MethodMode newMethod = static_cast< MethodMode >( methodComboBox->currentData().toInt() );
  toggleMethodWidgets( newMethod );
  switch ( newMethod )
  {
    case ColorMode:
    {
      mRenderer->setGraduatedMethod( Qgis::GraduatedMethod::Color );
      QgsColorRamp *ramp = btnColorRamp->colorRamp();

      if ( !ramp )
      {
        QMessageBox::critical( this, tr( "Select Method" ), tr( "No color ramp defined." ) );
        return;
      }
      mRenderer->setSourceColorRamp( ramp );
      reapplyColorRamp();
      break;
    }

    case SizeMode:
    {
      lblColorRamp->setVisible( false );
      btnColorRamp->setVisible( false );
      lblSize->setVisible( true );
      minSizeSpinBox->setVisible( true );
      lblSize->setVisible( true );
      maxSizeSpinBox->setVisible( true );
      mSizeUnitWidget->setVisible( true );

      mRenderer->setGraduatedMethod( Qgis::GraduatedMethod::Size );
      reapplySizes();
      break;
    }
  }
}

void QgsGraduatedSymbolRendererWidget::updateMethodParameters()
{
  clearParameterWidgets();

  const QString methodId = cboGraduatedMode->currentData().toString();
  QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method( methodId );
  Q_ASSERT( method );

  // need more context?
  QgsProcessingContext context;

  for ( const QgsProcessingParameterDefinition *def : method->parameterDefinitions() )
  {
    QgsAbstractProcessingParameterWidgetWrapper *ppww = QgsGui::processingGuiRegistry()->createParameterWidgetWrapper( def, QgsProcessingGui::Standard );
    mParametersLayout->addRow( ppww->createWrappedLabel(), ppww->createWrappedWidget( context ) );

    QVariant value = method->parameterValues().value( def->name(), def->defaultValueForGui() );
    ppww->setParameterValue( value, context );

    connect( ppww, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, &QgsGraduatedSymbolRendererWidget::classifyGraduated );

    mParameterWidgetWrappers.push_back( std::unique_ptr<QgsAbstractProcessingParameterWidgetWrapper>( ppww ) );
  }

  spinGraduatedClasses->setEnabled( !( method->flags() & QgsClassificationMethod::MethodProperty::IgnoresClassCount ) );
}

void QgsGraduatedSymbolRendererWidget::toggleMethodWidgets( MethodMode mode )
{
  switch ( mode )
  {
    case ColorMode:
    {
      lblColorRamp->setVisible( true );
      btnColorRamp->setVisible( true );
      lblSize->setVisible( false );
      minSizeSpinBox->setVisible( false );
      lblSizeTo->setVisible( false );
      maxSizeSpinBox->setVisible( false );
      mSizeUnitWidget->setVisible( false );
      break;
    }

    case SizeMode:
    {
      lblColorRamp->setVisible( false );
      btnColorRamp->setVisible( false );
      lblSize->setVisible( true );
      minSizeSpinBox->setVisible( true );
      lblSizeTo->setVisible( true );
      maxSizeSpinBox->setVisible( true );
      mSizeUnitWidget->setVisible( true );
      break;
    }
  }
}

void QgsGraduatedSymbolRendererWidget::clearParameterWidgets()
{
  while ( mParametersLayout->rowCount() )
  {
    QFormLayout::TakeRowResult row = mParametersLayout->takeRow( 0 );
    for ( QLayoutItem *item : QList<QLayoutItem *>( {row.labelItem, row.fieldItem} ) )
      if ( item )
      {
        if ( item->widget() )
          item->widget()->deleteLater();
        delete item;
      }
  }
  mParameterWidgetWrappers.clear();
}

void QgsGraduatedSymbolRendererWidget::refreshRanges( bool )
{
  if ( !mModel )
    return;

  mModel->updateSymbology();

  disconnectUpdateHandlers();
  spinGraduatedClasses->setValue( mRenderer->ranges().count() );
  connectUpdateHandlers();

  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::setSymbolLevels( const QgsLegendSymbolList &levels, bool enabled )
{
  for ( const QgsLegendSymbolItem &legendSymbol : levels )
  {
    QgsSymbol *sym = legendSymbol.symbol();
    for ( int layer = 0; layer < sym->symbolLayerCount(); layer++ )
    {
      mRenderer->setLegendSymbolItem( legendSymbol.ruleKey(), sym->clone() );
    }
  }
  mRenderer->setUsingSymbolLevels( enabled );
  mModel->updateSymbology();
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
  if ( !selectedIndexes.isEmpty() )
  {
    const auto constSelectedIndexes = selectedIndexes;
    for ( const QModelIndex &idx : constSelectedIndexes )
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

void QgsGraduatedSymbolRendererWidget::symmetryPointEditingFinished( )
{
  const QString text = cboSymmetryPoint->lineEdit()->text();
  int index = cboSymmetryPoint->findText( text );
  if ( index != -1 )
  {
    cboSymmetryPoint->setCurrentIndex( index );
  }
  else
  {
    cboSymmetryPoint->setItemText( cboSymmetryPoint->currentIndex(), text );
    classifyGraduated();
  }
}


void QgsGraduatedSymbolRendererWidget::classifyGraduated()
{
  if ( mBlockUpdates )
    return;

  QgsTemporaryCursorOverride override( Qt::WaitCursor );
  QString attrName = mExpressionWidget->currentField();
  int nclasses = spinGraduatedClasses->value();

  const QString methodId = cboGraduatedMode->currentData().toString();
  QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method( methodId );
  Q_ASSERT( method );

  int attrNum = mLayer->fields().lookupField( attrName );

  QVariant minVal;
  QVariant maxVal;
  mLayer->minimumAndMaximumValue( attrNum, minVal, maxVal );

  double minimum = minVal.toDouble();
  double maximum = maxVal.toDouble();
  mSymmetryPointValidator->setBottom( minimum );
  mSymmetryPointValidator->setTop( maximum );
  mSymmetryPointValidator->setMaxDecimals( spinPrecision->value() );

  if ( method->id() == QgsClassificationEqualInterval::METHOD_ID ||
       method->id() == QgsClassificationStandardDeviation::METHOD_ID )
  {
    // knowing that spinSymmetryPointForOtherMethods->value() is automatically put at minimum when out of min-max
    // using "(maximum-minimum)/100)" to avoid direct comparison of doubles
    double currentValue = QgsDoubleValidator::toDouble( cboSymmetryPoint->currentText() );
    if ( currentValue < ( minimum + ( maximum - minimum ) / 100. ) || currentValue > ( maximum - ( maximum - minimum ) / 100. ) )
      cboSymmetryPoint->setItemText( cboSymmetryPoint->currentIndex(), QLocale().toString( minimum + ( maximum - minimum ) / 2., 'f', method->labelPrecision() + 2 ) );
  }

  if ( mGroupBoxSymmetric->isChecked() )
  {
    double symmetryPoint = QgsDoubleValidator::toDouble( cboSymmetryPoint->currentText() );
    bool astride = cbxAstride->isChecked();
    method->setSymmetricMode( true, symmetryPoint, astride );
  }

  QVariantMap parameterValues;
  for ( const auto &ppww : std::as_const( mParameterWidgetWrappers ) )
    parameterValues.insert( ppww->parameterDefinition()->name(), ppww->parameterValue() );
  method->setParameterValues( parameterValues );

  // set method to renderer
  mRenderer->setClassificationMethod( method );

  // create and set new renderer
  mRenderer->setClassAttribute( attrName );

  // If complexity >= oN^2, warn for big dataset (more than 50k records)
  // and give the user the chance to cancel
  if ( method->codeComplexity() > 1 && mLayer->featureCount() > 50000 )
  {
    if ( QMessageBox::Cancel == QMessageBox::question( this, tr( "Apply Classification" ), tr( "Natural break classification (Jenks) is O(n2) complexity, your classification may take a long time.\nPress cancel to abort breaks calculation or OK to continue." ), QMessageBox::Cancel, QMessageBox::Ok ) )
    {
      return;
    }
  }

  if ( methodComboBox->currentData() == ColorMode )
  {
    std::unique_ptr<QgsColorRamp> ramp( btnColorRamp->colorRamp() );
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

  mRenderer->updateClasses( mLayer, nclasses );

  if ( methodComboBox->currentData() == SizeMode )
    mRenderer->setSymbolSizes( minSizeSpinBox->value(), maxSizeSpinBox->value() );

  mRenderer->calculateLabelPrecision();
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

  const auto constSelectedRows = selectedRows;
  for ( const QModelIndex &r : constSelectedRows )
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
    whileBlocking( btnChangeGraduatedSymbol )->setSymbol( mGraduatedSymbol->clone() );
    applyChangeToSymbol();
  }
}

void QgsGraduatedSymbolRendererWidget::changeRange( int rangeIdx )
{
  QgsLUDialog dialog( this );

  const QgsRendererRange &range = mRenderer->ranges()[rangeIdx];
  // Add arbitrary 2 to number of decimal places to retain a bit extra.
  // Ensures users can see if legend is not completely honest!
  int decimalPlaces = mRenderer->classificationMethod()->labelPrecision() + 2;
  if ( decimalPlaces < 0 ) decimalPlaces = 0;
  dialog.setLowerValue( QLocale().toString( range.lowerValue(), 'f', decimalPlaces ) );
  dialog.setUpperValue( QLocale().toString( range.upperValue(), 'f', decimalPlaces ) );

  if ( dialog.exec() == QDialog::Accepted )
  {
    mRenderer->updateRangeUpperValue( rangeIdx, dialog.upperValueDouble() );
    mRenderer->updateRangeLowerValue( rangeIdx, dialog.lowerValueDouble() );

    //If the boundaries have to stay linked, we update the ranges above and below, as well as their label if needed
    if ( cbxLinkBoundaries->isChecked() )
    {
      if ( rangeIdx > 0 )
      {
        mRenderer->updateRangeUpperValue( rangeIdx - 1, dialog.lowerValueDouble() );
      }

      if ( rangeIdx < mRenderer->ranges().size() - 1 )
      {
        mRenderer->updateRangeLowerValue( rangeIdx + 1, dialog.upperValueDouble() );
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
  emit widgetChanged();

}

void QgsGraduatedSymbolRendererWidget::deleteClasses()
{
  QList<int> classIndexes = selectedClasses();
  mModel->deleteRows( classIndexes );
  mHistogramWidget->refresh();
  emit widgetChanged();
}

void QgsGraduatedSymbolRendererWidget::deleteAllClasses()
{
  mModel->removeAllRows();
  mHistogramWidget->refresh();
  emit widgetChanged();
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
  mRenderer->classificationMethod()->setLabelFormat( txtLegendFormat->text() );
  mRenderer->classificationMethod()->setLabelPrecision( spinPrecision->value() );
  mRenderer->classificationMethod()->setLabelTrimTrailingZeroes( cbxTrimTrailingZeroes->isChecked() );
  mRenderer->updateRangeLabels();
  mModel->updateLabels();
}


QList<QgsSymbol *> QgsGraduatedSymbolRendererWidget::selectedSymbols()
{
  QList<QgsSymbol *> selectedSymbols;

  QItemSelectionModel *m = viewGraduated->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );
  if ( !selectedIndexes.isEmpty() )
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
  int decimalPlaces = mRenderer->classificationMethod()->labelPrecision() + 2;
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

void QgsGraduatedSymbolRendererWidget::selectionChanged( const QItemSelection &, const QItemSelection & )
{
  const QgsRangeList ranges = selectedRanges();
  if ( !ranges.isEmpty() )
  {
    whileBlocking( btnChangeGraduatedSymbol )->setSymbol( ranges.at( 0 ).symbol()->clone() );
  }
  else if ( mRenderer->sourceSymbol() )
  {
    whileBlocking( btnChangeGraduatedSymbol )->setSymbol( mRenderer->sourceSymbol()->clone() );
  }
  btnChangeGraduatedSymbol->setDialogTitle( ranges.size() == 1 ? ranges.at( 0 ).label() : tr( "Symbol Settings" ) );
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

void QgsGraduatedSymbolRendererWidget::changeGraduatedSymbol()
{
  mGraduatedSymbol.reset( btnChangeGraduatedSymbol->symbol()->clone() );
  applyChangeToSymbol();
}

void QgsGraduatedSymbolRendererWidget::pasteSymbolToSelection()
{
  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( !tempSymbol )
    return;

  const QModelIndexList selectedRows = viewGraduated->selectionModel()->selectedRows();
  for ( const QModelIndex &index : selectedRows )
  {
    if ( !index.isValid() )
      continue;

    const int row = index.row();
    if ( !mRenderer || mRenderer->ranges().size() <= row )
      continue;

    if ( mRenderer->ranges().at( row ).symbol()->type() != tempSymbol->type() )
      continue;

    std::unique_ptr< QgsSymbol > newCatSymbol( tempSymbol->clone() );
    if ( selectedRows.count() > 1 )
    {
      //if updating multiple ranges, retain the existing category colors
      newCatSymbol->setColor( mRenderer->ranges().at( row ).symbol()->color() );
    }

    mRenderer->updateRangeSymbol( row, newCatSymbol.release() );
  }
  emit widgetChanged();
}
