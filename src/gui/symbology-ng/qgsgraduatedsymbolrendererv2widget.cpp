/***************************************************************************
    qgsgraduatedsymbolrendererv2widget.cpp
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
#include "qgsgraduatedsymbolrendererv2widget.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"

#include "qgsvectorlayer.h"

#include "qgssymbolv2selectordialog.h"
#include "qgsexpressionbuilderdialog.h"

#include "qgsludialog.h"

#include "qgsproject.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPen>
#include <QPainter>

// ------------------------------ Model ------------------------------------

QgsGraduatedSymbolRendererV2Model::QgsGraduatedSymbolRendererV2Model( QObject * parent ) : QAbstractItemModel( parent )
    , mRenderer( 0 )
    , mMimeFormat( "application/x-qgsgraduatedsymbolrendererv2model" )
{
}

void QgsGraduatedSymbolRendererV2Model::setRenderer( QgsGraduatedSymbolRendererV2* renderer )
{
  if ( mRenderer )
  {
    beginRemoveRows( QModelIndex(), 0, mRenderer->ranges().size() - 1 );
    mRenderer = 0;
    endRemoveRows();
  }
  if ( renderer )
  {
    beginInsertRows( QModelIndex(), 0, renderer->ranges().size() - 1 );
    mRenderer = renderer;
    endInsertRows();
  }
}

void QgsGraduatedSymbolRendererV2Model::addClass( QgsSymbolV2* symbol )
{
  if ( !mRenderer ) return;
  int idx = mRenderer->ranges().size();
  beginInsertRows( QModelIndex(), idx, idx );
  mRenderer->addClass( symbol );
  endInsertRows();
}

void QgsGraduatedSymbolRendererV2Model::addClass( QgsRendererRangeV2 range )
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

QgsRendererRangeV2 QgsGraduatedSymbolRendererV2Model::rendererRange( const QModelIndex &index )
{
  if ( !index.isValid() || !mRenderer || mRenderer->ranges().size() <= index.row() )
  {
    return QgsRendererRangeV2();
  }

  return mRenderer->ranges().value( index.row() );
}

Qt::ItemFlags QgsGraduatedSymbolRendererV2Model::flags( const QModelIndex & index ) const
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

Qt::DropActions QgsGraduatedSymbolRendererV2Model::supportedDropActions() const
{
  return Qt::MoveAction;
}

QVariant QgsGraduatedSymbolRendererV2Model::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mRenderer ) return QVariant();

  const QgsRendererRangeV2 range = mRenderer->ranges().value( index.row() );

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
        return QString::number( range.lowerValue(), 'f', decimalPlaces ) + " - " + QString::number( range.upperValue(), 'f', decimalPlaces );
      }
      case 2: return range.label();
      default: return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && range.symbol() )
  {
    return QgsSymbolLayerV2Utils::symbolPreviewIcon( range.symbol(), QSize( 16, 16 ) );
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
      case 2: return range.label();
      default: return QVariant();
    }
  }

  return QVariant();
}

bool QgsGraduatedSymbolRendererV2Model::setData( const QModelIndex & index, const QVariant & value, int role )
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
      break;
    case 2: // label
      mRenderer->updateRangeLabel( index.row(), value.toString() );
      break;
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

QVariant QgsGraduatedSymbolRendererV2Model::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 3 )
  {
    QStringList lst; lst << tr( "Symbol" ) << tr( "Values" ) << tr( "Legend" );
    return lst.value( section );
  }
  return QVariant();
}

int QgsGraduatedSymbolRendererV2Model::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() || !mRenderer )
  {
    return 0;
  }
  return mRenderer->ranges().size();
}

int QgsGraduatedSymbolRendererV2Model::columnCount( const QModelIndex & index ) const
{
  Q_UNUSED( index );
  return 3;
}

QModelIndex QgsGraduatedSymbolRendererV2Model::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column );
  }
  return QModelIndex();
}

QModelIndex QgsGraduatedSymbolRendererV2Model::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return QModelIndex();
}

QStringList QgsGraduatedSymbolRendererV2Model::mimeTypes() const
{
  QStringList types;
  types << mMimeFormat;
  return types;
}

QMimeData *QgsGraduatedSymbolRendererV2Model::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  // Create list of rows
  foreach ( const QModelIndex &index, indexes )
  {
    if ( !index.isValid() || index.column() != 0 )
      continue;

    stream << index.row();
  }
  mimeData->setData( mMimeFormat, encodedData );
  return mimeData;
}

bool QgsGraduatedSymbolRendererV2Model::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
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
    QgsDebugMsg( QString( "move %1 to %2" ).arg( rows[i] ).arg( to ) );
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

void QgsGraduatedSymbolRendererV2Model::deleteRows( QList<int> rows )
{
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    beginRemoveRows( QModelIndex(), rows[i], rows[i] );
    mRenderer->deleteClass( rows[i] );
    endRemoveRows();
  }
}

void QgsGraduatedSymbolRendererV2Model::removeAllRows()
{
  beginRemoveRows( QModelIndex(), 0, mRenderer->ranges().size() - 1 );
  mRenderer->deleteAllClasses();
  endRemoveRows();
}

void QgsGraduatedSymbolRendererV2Model::sort( int column, Qt::SortOrder order )
{
  QgsDebugMsg( "Entered" );
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
  QgsDebugMsg( "Done" );
}

void QgsGraduatedSymbolRendererV2Model::updateSymbology( bool resetModel )
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

void QgsGraduatedSymbolRendererV2Model::updateLabels()
{
  emit dataChanged( createIndex( 0, 2 ), createIndex( mRenderer->ranges().size(), 2 ) );
}

// ------------------------------ View style --------------------------------
QgsGraduatedSymbolRendererV2ViewStyle::QgsGraduatedSymbolRendererV2ViewStyle( QStyle* style )
    : QProxyStyle( style )
{}

void QgsGraduatedSymbolRendererV2ViewStyle::drawPrimitive( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget ) const
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

// ------------------------------ Widget ------------------------------------

QgsRendererV2Widget* QgsGraduatedSymbolRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsGraduatedSymbolRendererV2Widget( layer, style, renderer );
}

QgsGraduatedSymbolRendererV2Widget::QgsGraduatedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( 0 )
    , mModel( 0 )
{


  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( renderer )
  {
    mRenderer = QgsGraduatedSymbolRendererV2::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsGraduatedSymbolRendererV2( "", QgsRangeList() );
  }

  // setup user interface
  setupUi( this );
  mModel = new QgsGraduatedSymbolRendererV2Model( this );

  mExpressionWidget->setFilters( QgsFieldProxyModel::Numeric | QgsFieldProxyModel::Date );
  mExpressionWidget->setLayer( mLayer );

  mSizeUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit );


  cboGraduatedColorRamp->populate( mStyle );

  spinPrecision->setMinimum( QgsRendererRangeV2LabelFormat::MinPrecision );
  spinPrecision->setMaximum( QgsRendererRangeV2LabelFormat::MaxPrecision );

  // set project default color ramp
  QString defaultColorRamp = QgsProject::instance()->readEntry( "DefaultStyles", "/ColorRamp", "" );
  if ( defaultColorRamp != "" )
  {
    int index = cboGraduatedColorRamp->findText( defaultColorRamp, Qt::MatchCaseSensitive );
    if ( index >= 0 )
      cboGraduatedColorRamp->setCurrentIndex( index );
  }


  viewGraduated->setStyle( new QgsGraduatedSymbolRendererV2ViewStyle( viewGraduated->style() ) );

  mGraduatedSymbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

  methodComboBox->blockSignals( true );
  methodComboBox->addItem( "Color" );
  if ( mGraduatedSymbol->type() == QgsSymbolV2::Marker )
  {
    methodComboBox->addItem( "Size" );
    minSizeSpinBox->setValue( 1 );
    maxSizeSpinBox->setValue( 8 );
  }
  else if ( mGraduatedSymbol->type() == QgsSymbolV2::Line )
  {
    methodComboBox->addItem( "Size" );
    minSizeSpinBox->setValue( .1 );
    maxSizeSpinBox->setValue( 2 );
  }
  methodComboBox->blockSignals( false );

  connect( mExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( graduatedColumnChanged( QString ) ) );
  connect( viewGraduated, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( rangesDoubleClicked( const QModelIndex & ) ) );
  connect( viewGraduated, SIGNAL( clicked( const QModelIndex & ) ), this, SLOT( rangesClicked( const QModelIndex & ) ) );
  connect( viewGraduated, SIGNAL( customContextMenuRequested( const QPoint& ) ),  this, SLOT( contextMenuViewCategories( const QPoint& ) ) );

  connect( btnGraduatedClassify, SIGNAL( clicked() ), this, SLOT( classifyGraduated() ) );
  connect( btnChangeGraduatedSymbol, SIGNAL( clicked() ), this, SLOT( changeGraduatedSymbol() ) );
  connect( btnGraduatedDelete, SIGNAL( clicked() ), this, SLOT( deleteClasses() ) );
  connect( btnDeleteAllClasses, SIGNAL( clicked() ), this, SLOT( deleteAllClasses() ) );
  connect( btnGraduatedAdd, SIGNAL( clicked() ), this, SLOT( addClass() ) );
  connect( cbxLinkBoundaries, SIGNAL( toggled( bool ) ), this, SLOT( toggleBoundariesLink( bool ) ) );

  connect( mSizeUnitWidget, SIGNAL( changed() ), this, SLOT( on_mSizeUnitWidget_changed() ) );

  connectUpdateHandlers();

  // initialize from previously set renderer
  updateUiFromRenderer();

  // menus for data-defined rotation/size
  QMenu* advMenu = new QMenu;

  advMenu->addAction( tr( "Symbol levels..." ), this, SLOT( showSymbolLevels() ) );

  mDataDefinedMenus = new QgsRendererV2DataDefinedMenus( advMenu, mLayer,
      mRenderer->rotationField(), mRenderer->sizeScaleField(), mRenderer->scaleMethod() );
  connect( mDataDefinedMenus, SIGNAL( rotationFieldChanged( QString ) ), this, SLOT( rotationFieldChanged( QString ) ) );
  connect( mDataDefinedMenus, SIGNAL( sizeScaleFieldChanged( QString ) ), this, SLOT( sizeScaleFieldChanged( QString ) ) );
  connect( mDataDefinedMenus, SIGNAL( scaleMethodChanged( QgsSymbolV2::ScaleMethod ) ), this, SLOT( scaleMethodChanged( QgsSymbolV2::ScaleMethod ) ) );
  btnAdvanced->setMenu( advMenu );

  mHistogramWidget->setLayer( mLayer );
  mHistogramWidget->setRenderer( mRenderer );
  connect( mHistogramWidget, SIGNAL( rangesModified( bool ) ), this, SLOT( refreshRanges( bool ) ) );
  connect( mExpressionWidget, SIGNAL( fieldChanged( QString ) ), mHistogramWidget, SLOT( setSourceFieldExp( QString ) ) );
}

void QgsGraduatedSymbolRendererV2Widget::on_mSizeUnitWidget_changed()
{
  if ( !mGraduatedSymbol ) return;
  mGraduatedSymbol->setOutputUnit( mSizeUnitWidget->unit() );
  mGraduatedSymbol->setMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
  updateGraduatedSymbolIcon();
  mRenderer->updateSymbols( mGraduatedSymbol );
  refreshSymbolView();
}

QgsGraduatedSymbolRendererV2Widget::~QgsGraduatedSymbolRendererV2Widget()
{
  delete mRenderer;
  delete mModel;
}

QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}

// Connect/disconnect event handlers which trigger updating renderer

void QgsGraduatedSymbolRendererV2Widget::connectUpdateHandlers()
{
  connect( spinGraduatedClasses, SIGNAL( valueChanged( int ) ), this, SLOT( classifyGraduated() ) );
  connect( cboGraduatedMode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( classifyGraduated() ) );
  connect( cboGraduatedColorRamp, SIGNAL( currentIndexChanged( int ) ), this, SLOT( reapplyColorRamp() ) );
  connect( cbxInvertedColorRamp, SIGNAL( toggled( bool ) ), this, SLOT( reapplyColorRamp() ) );
  connect( spinPrecision, SIGNAL( valueChanged( int ) ), this, SLOT( labelFormatChanged() ) );
  connect( cbxTrimTrailingZeroes, SIGNAL( toggled( bool ) ), this, SLOT( labelFormatChanged() ) );
  connect( txtLegendFormat, SIGNAL( textChanged( QString ) ), this, SLOT( labelFormatChanged() ) );
  connect( minSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( reapplySizes() ) );
  connect( maxSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( reapplySizes() ) );

  connect( mModel, SIGNAL( rowsMoved() ), this, SLOT( rowsMoved() ) );
  connect( mModel, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), this, SLOT( modelDataChanged() ) );
}

// Connect/disconnect event handlers which trigger updating renderer

void QgsGraduatedSymbolRendererV2Widget::disconnectUpdateHandlers()
{
  disconnect( spinGraduatedClasses, SIGNAL( valueChanged( int ) ), this, SLOT( classifyGraduated() ) );
  disconnect( cboGraduatedMode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( classifyGraduated() ) );
  disconnect( cboGraduatedColorRamp, SIGNAL( currentIndexChanged( int ) ), this, SLOT( reapplyColorRamp() ) );
  disconnect( cbxInvertedColorRamp, SIGNAL( toggled( bool ) ), this, SLOT( reapplyColorRamp() ) );
  disconnect( spinPrecision, SIGNAL( valueChanged( int ) ), this, SLOT( labelFormatChanged() ) );
  disconnect( cbxTrimTrailingZeroes, SIGNAL( toggled( bool ) ), this, SLOT( labelFormatChanged() ) );
  disconnect( txtLegendFormat, SIGNAL( textChanged( QString ) ), this, SLOT( labelFormatChanged() ) );
  disconnect( minSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( reapplySizes() ) );
  disconnect( maxSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( reapplySizes() ) );

  disconnect( mModel, SIGNAL( rowsMoved() ), this, SLOT( rowsMoved() ) );
  disconnect( mModel, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), this, SLOT( modelDataChanged() ) );
}

void QgsGraduatedSymbolRendererV2Widget::updateUiFromRenderer( bool updateCount )
{
  disconnectUpdateHandlers();

  updateGraduatedSymbolIcon();

  // update UI from the graduated renderer (update combo boxes, view)
  if ( mRenderer->mode() < cboGraduatedMode->count() )
    cboGraduatedMode->setCurrentIndex( mRenderer->mode() );

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
    delete mGraduatedSymbol;
    mGraduatedSymbol = mRenderer->sourceSymbol()->clone();
    updateGraduatedSymbolIcon();
  }

  mModel->setRenderer( mRenderer );
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
  if ( mRenderer->graduatedMethod() == QgsGraduatedSymbolRendererV2::GraduatedColor )
  {
    methodComboBox->setCurrentIndex( 0 );
    if ( mRenderer->sourceColorRamp() )
      cboGraduatedColorRamp->setSourceColorRamp( mRenderer->sourceColorRamp() );
    cbxInvertedColorRamp->setChecked( mRenderer->invertedColorRamp() );
  }
  else
  {
    methodComboBox->setCurrentIndex( 1 );
    if ( mRenderer->ranges().count() ) // avoid overiding default size with zeros
    {
      minSizeSpinBox->setValue( mRenderer->minSymbolSize() );
      maxSizeSpinBox->setValue( mRenderer->maxSymbolSize() );
    }
  }
  mMethodStackedWidget->setCurrentIndex( methodComboBox->currentIndex() );
  methodComboBox->blockSignals( false );

  QgsRendererRangeV2LabelFormat labelFormat = mRenderer->labelFormat();
  txtLegendFormat->setText( labelFormat.format() );
  spinPrecision->setValue( labelFormat.precision() );
  cbxTrimTrailingZeroes->setChecked( labelFormat.trimTrailingZeroes() );

  viewGraduated->resizeColumnToContents( 0 );
  viewGraduated->resizeColumnToContents( 1 );
  viewGraduated->resizeColumnToContents( 2 );

  mHistogramWidget->refreshAndRedraw();

  connectUpdateHandlers();
}

void QgsGraduatedSymbolRendererV2Widget::graduatedColumnChanged( QString field )
{
  mRenderer->setClassAttribute( field );
}

void QgsGraduatedSymbolRendererV2Widget::on_methodComboBox_currentIndexChanged( int idx )
{
  mMethodStackedWidget->setCurrentIndex( idx );
  if ( idx == 0 )
  {
    mRenderer->setGraduatedMethod( QgsGraduatedSymbolRendererV2::GraduatedColor );
    QgsVectorColorRampV2* ramp = cboGraduatedColorRamp->currentColorRamp();

    if ( ramp == NULL )
    {
      if ( cboGraduatedColorRamp->count() == 0 )
        QMessageBox::critical( this, tr( "Error" ), tr( "There are no available color ramps. You can add them in Style Manager." ) );
      else
        QMessageBox::critical( this, tr( "Error" ), tr( "The selected color ramp is not available." ) );
      return;
    }
    mRenderer->setSourceColorRamp( ramp->clone() );
    reapplyColorRamp();
  }
  else
  {
    mRenderer->setGraduatedMethod( QgsGraduatedSymbolRendererV2::GraduatedSize );
    reapplySizes();
  }
}

void QgsGraduatedSymbolRendererV2Widget::refreshRanges( bool reset )
{
  if ( !mModel )
    return;

  mModel->updateSymbology( reset );
}

void QgsGraduatedSymbolRendererV2Widget::classifyGraduated()
{
  QString attrName = mExpressionWidget->currentField();

  int nclasses = spinGraduatedClasses->value();

  QSharedPointer<QgsVectorColorRampV2> ramp( cboGraduatedColorRamp->currentColorRamp() );
  if ( !ramp )
  {
    if ( cboGraduatedColorRamp->count() == 0 )
      QMessageBox::critical( this, tr( "Error" ), tr( "There are no available color ramps. You can add them in Style Manager." ) );
    else
      QMessageBox::critical( this, tr( "Error" ), tr( "The selected color ramp is not available." ) );
    return;
  }

  QgsGraduatedSymbolRendererV2::Mode mode;
  if ( cboGraduatedMode->currentIndex() == 0 )
    mode = QgsGraduatedSymbolRendererV2::EqualInterval;
  else if ( cboGraduatedMode->currentIndex() == 2 )
    mode = QgsGraduatedSymbolRendererV2::Jenks;
  else if ( cboGraduatedMode->currentIndex() == 3 )
    mode = QgsGraduatedSymbolRendererV2::StdDev;
  else if ( cboGraduatedMode->currentIndex() == 4 )
    mode = QgsGraduatedSymbolRendererV2::Pretty;
  else // default should be quantile for now
    mode = QgsGraduatedSymbolRendererV2::Quantile;

  // Jenks is n^2 complexity, warn for big dataset (more than 50k records)
  // and give the user the chance to cancel
  if ( QgsGraduatedSymbolRendererV2::Jenks == mode && mLayer->featureCount() > 50000 )
  {
    if ( QMessageBox::Cancel == QMessageBox::question( this, tr( "Warning" ), tr( "Natural break classification (Jenks) is O(n2) complexity, your classification may take a long time.\nPress cancel to abort breaks calculation or OK to continue." ), QMessageBox::Cancel, QMessageBox::Ok ) )
      return;
  }

  // create and set new renderer

  mRenderer->setClassAttribute( attrName );
  mRenderer->setMode( mode );

  if ( methodComboBox->currentIndex() == 0 )
  {
    QgsVectorColorRampV2* ramp = cboGraduatedColorRamp->currentColorRamp();

    if ( ramp == NULL )
    {
      if ( cboGraduatedColorRamp->count() == 0 )
        QMessageBox::critical( this, tr( "Error" ), tr( "There are no available color ramps. You can add them in Style Manager." ) );
      else
        QMessageBox::critical( this, tr( "Error" ), tr( "The selected color ramp is not available." ) );
      return;
    }
    mRenderer->setSourceColorRamp( ramp->clone() );
  }
  else
  {
    mRenderer->setSourceColorRamp( NULL );
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mRenderer->updateClasses( mLayer, mode, nclasses );

  if ( methodComboBox->currentIndex() == 1 )
    mRenderer->setSymbolSizes( minSizeSpinBox->value(), maxSizeSpinBox->value() );

  mRenderer->calculateLabelPrecision();
  QApplication::restoreOverrideCursor();
  // PrettyBreaks and StdDev calculation don't generate exact
  // number of classes - leave user interface unchanged for these
  updateUiFromRenderer( false );
}

void QgsGraduatedSymbolRendererV2Widget::reapplyColorRamp()
{
  QgsVectorColorRampV2* ramp = cboGraduatedColorRamp->currentColorRamp();
  if ( ramp == NULL )
    return;

  mRenderer->updateColorRamp( ramp, cbxInvertedColorRamp->isChecked() );
  mRenderer->updateSymbols( mGraduatedSymbol );
  refreshSymbolView();
}

void QgsGraduatedSymbolRendererV2Widget::reapplySizes()
{
  mRenderer->setSymbolSizes( minSizeSpinBox->value(), maxSizeSpinBox->value() );
  mRenderer->updateSymbols( mGraduatedSymbol );
  refreshSymbolView();
}

void QgsGraduatedSymbolRendererV2Widget::changeGraduatedSymbol()
{
  // Change the selected symbols alone if anything is selected
  QItemSelectionModel* m = viewGraduated->selectionModel();
  QModelIndexList i = m->selectedRows();
  if ( m && i.size() > 0 )
  {
    changeSelectedSymbols();
    return;
  }

  // Otherwise change the base mGraduatedSymbol
  QgsSymbolV2* newSymbol = mGraduatedSymbol->clone();

  QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, mLayer, this );
  if ( !dlg.exec() )
  {
    delete newSymbol;
    return;
  }

  mGraduatedSymbol = newSymbol;

  mSizeUnitWidget->blockSignals( true );
  mSizeUnitWidget->setUnit( mGraduatedSymbol->outputUnit() );
  mSizeUnitWidget->setMapUnitScale( mGraduatedSymbol->mapUnitScale() );
  mSizeUnitWidget->blockSignals( false );

  updateGraduatedSymbolIcon();
  mRenderer->updateSymbols( mGraduatedSymbol );
  refreshSymbolView();
}

void QgsGraduatedSymbolRendererV2Widget::updateGraduatedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mGraduatedSymbol, btnChangeGraduatedSymbol->iconSize() );
  btnChangeGraduatedSymbol->setIcon( icon );
}

#if 0
int QgsRendererV2PropertiesDialog::currentRangeRow()
{
  QModelIndex idx = viewGraduated->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}
#endif

QList<int> QgsGraduatedSymbolRendererV2Widget::selectedClasses()
{
  QList<int> rows;
  QModelIndexList selectedRows = viewGraduated->selectionModel()->selectedRows();

  foreach ( QModelIndex r, selectedRows )
  {
    if ( r.isValid() )
    {
      rows.append( r.row() );
    }
  }
  return rows;
}

QgsRangeList QgsGraduatedSymbolRendererV2Widget::selectedRanges()
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

void QgsGraduatedSymbolRendererV2Widget::rangesDoubleClicked( const QModelIndex & idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeRangeSymbol( idx.row() );
  if ( idx.isValid() && idx.column() == 1 )
    changeRange( idx.row() );
}

void QgsGraduatedSymbolRendererV2Widget::rangesClicked( const QModelIndex & idx )
{
  if ( !idx.isValid() )
    mRowSelected = -1;
  else
    mRowSelected = idx.row();
}

void QgsGraduatedSymbolRendererV2Widget::changeSelectedSymbols()
{
  QItemSelectionModel* m = viewGraduated->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );
  if ( m && selectedIndexes.size() > 0 )
  {
    QgsSymbolV2* newSymbol = mGraduatedSymbol->clone();
    QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, mLayer, this );
    if ( !dlg.exec() )
    {
      delete newSymbol;
      return;
    }

    foreach ( QModelIndex idx, selectedIndexes )
    {
      if ( idx.isValid() )
      {
        int rangeIdx = idx.row();
        QgsSymbolV2* newRangeSymbol = newSymbol->clone();
        newRangeSymbol->setColor( mRenderer->ranges()[rangeIdx].symbol()->color() );
        mRenderer->updateRangeSymbol( rangeIdx, newRangeSymbol );
      }
    }
  }
  refreshSymbolView();
}

void QgsGraduatedSymbolRendererV2Widget::changeRangeSymbol( int rangeIdx )
{
  QgsSymbolV2* newSymbol = mRenderer->ranges()[rangeIdx].symbol()->clone();

  QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, mLayer, this );
  if ( !dlg.exec() )
  {
    delete newSymbol;
    return;
  }

  mRenderer->updateRangeSymbol( rangeIdx, newSymbol );
  mHistogramWidget->refreshHistogram();
}

void QgsGraduatedSymbolRendererV2Widget::changeRange( int rangeIdx )
{
  QgsLUDialog dialog( this );

  const QgsRendererRangeV2& range = mRenderer->ranges()[rangeIdx];
  // Add arbitrary 2 to number of decimal places to retain a bit extra.
  // Ensures users can see if legend is not completely honest!
  int decimalPlaces = mRenderer->labelFormat().precision() + 2;
  if ( decimalPlaces < 0 ) decimalPlaces = 0;
  dialog.setLowerValue( QString::number( range.lowerValue(), 'f', decimalPlaces ) );
  dialog.setUpperValue( QString::number( range.upperValue(), 'f', decimalPlaces ) );

  if ( dialog.exec() == QDialog::Accepted )
  {
    double lowerValue = dialog.lowerValue().toDouble();
    double upperValue = dialog.upperValue().toDouble();
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
  mHistogramWidget->refreshHistogram();
}

void QgsGraduatedSymbolRendererV2Widget::addClass()
{
  mModel->addClass( mGraduatedSymbol );
  mHistogramWidget->refreshHistogram();
}

void QgsGraduatedSymbolRendererV2Widget::deleteClasses()
{
  QList<int> classIndexes = selectedClasses();
  mModel->deleteRows( classIndexes );
  mHistogramWidget->refreshHistogram();
}

void QgsGraduatedSymbolRendererV2Widget::deleteAllClasses()
{
  mModel->removeAllRows();
  mHistogramWidget->refreshHistogram();
}

bool QgsGraduatedSymbolRendererV2Widget::rowsOrdered()
{
  const QgsRangeList &ranges = mRenderer->ranges();
  bool ordered = true;
  for ( int i = 1;i < ranges.size();++i )
  {
    if ( ranges[i] < ranges[i-1] )
    {
      ordered = false;
      break;
    }
  }
  return ordered;
}

void QgsGraduatedSymbolRendererV2Widget::toggleBoundariesLink( bool linked )
{
  //If the checkbox controlling the link between boundaries was unchecked and we check it, we have to link the boundaries
  //This is done by updating all lower ranges to the upper value of the range above
  if ( linked )
  {
    if ( ! rowsOrdered() )
    {
      int result = QMessageBox::warning(
                     this,
                     tr( "Linked range warning" ),
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
    for ( int i = 1;i < mRenderer->ranges().size();++i )
    {
      mRenderer->updateRangeLowerValue( i, mRenderer->ranges()[i-1].upperValue() );
    }
    refreshSymbolView();
  }
}

void QgsGraduatedSymbolRendererV2Widget::changeCurrentValue( QStandardItem * item )
{
  if ( item->column() == 2 )
  {
    QString label = item->text();
    int idx = item->row();
    mRenderer->updateRangeLabel( idx, label );
  }
}

void QgsGraduatedSymbolRendererV2Widget::rotationFieldChanged( QString fldName )
{
  mRenderer->setRotationField( fldName );
}

void QgsGraduatedSymbolRendererV2Widget::sizeScaleFieldChanged( QString fldName )
{
  mRenderer->setSizeScaleField( fldName );
}

void QgsGraduatedSymbolRendererV2Widget::scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mRenderer->setScaleMethod( scaleMethod );
}

void QgsGraduatedSymbolRendererV2Widget::labelFormatChanged()
{
  QgsRendererRangeV2LabelFormat labelFormat = QgsRendererRangeV2LabelFormat(
        txtLegendFormat->text(),
        spinPrecision->value(),
        cbxTrimTrailingZeroes->isChecked() );
  mRenderer->setLabelFormat( labelFormat, true );
  mModel->updateLabels();
}


QList<QgsSymbolV2*> QgsGraduatedSymbolRendererV2Widget::selectedSymbols()
{
  QList<QgsSymbolV2*> selectedSymbols;

  QItemSelectionModel* m = viewGraduated->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );
  if ( m && selectedIndexes.size() > 0 )
  {
    const QgsRangeList& ranges = mRenderer->ranges();
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      QStringList list = m->model()->data( *indexIt ).toString().split( " " );
      if ( list.size() < 3 )
      {
        continue;
      }

      double lowerBound = list.at( 0 ).toDouble();
      double upperBound = list.at( 2 ).toDouble();
      QgsSymbolV2* s = findSymbolForRange( lowerBound, upperBound, ranges );
      if ( s )
      {
        selectedSymbols.append( s );
      }
    }
  }
  return selectedSymbols;
}

QgsSymbolV2* QgsGraduatedSymbolRendererV2Widget::findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList& ranges ) const
{
  for ( QgsRangeList::const_iterator it = ranges.begin(); it != ranges.end(); ++it )
  {
    //range string has been created with option 'f',4
    if ( qgsDoubleNear( lowerBound, it->lowerValue(), 0.0001 ) && qgsDoubleNear( upperBound, it->upperValue(), 0.0001 ) )
    {
      return it->symbol();
    }
  }
  return 0;
}

void QgsGraduatedSymbolRendererV2Widget::refreshSymbolView()
{
  if ( mModel )
  {
    mModel->updateSymbology();
  }
}

void QgsGraduatedSymbolRendererV2Widget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer );
}

void QgsGraduatedSymbolRendererV2Widget::rowsMoved()
{
  viewGraduated->selectionModel()->clear();
  if ( ! rowsOrdered() )
  {
    cbxLinkBoundaries->setChecked( false );
  }
}

void QgsGraduatedSymbolRendererV2Widget::modelDataChanged()
{
}

void QgsGraduatedSymbolRendererV2Widget::keyPressEvent( QKeyEvent* event )
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
  }
}
