/***************************************************************************
    qgscategorizedsymbolrendererv2widget.cpp
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

#include "qgscategorizedsymbolrendererv2widget.h"

#include "qgscategorizedsymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"

#include "qgssymbolv2selectordialog.h"
#include "qgsexpressionbuilderdialog.h"

#include "qgsvectorlayer.h"

#include "qgsproject.h"
#include "qgsexpression.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPen>
#include <QPainter>
#include <QFileDialog>

QgsCategorizedSymbolRendererV2Model::QgsCategorizedSymbolRendererV2Model( QObject * parent ) : QAbstractItemModel( parent )
    , mRenderer( 0 )
    , mMimeFormat( "application/x-qgscategorizedsymbolrendererv2model" )
{
}

void QgsCategorizedSymbolRendererV2Model::setRenderer( QgsCategorizedSymbolRendererV2* renderer )
{
  if ( mRenderer )
  {
    beginRemoveRows( QModelIndex(), 0, mRenderer->categories().size() - 1 );
    mRenderer = 0;
    endRemoveRows();
  }
  if ( renderer )
  {
    beginInsertRows( QModelIndex(), 0, renderer->categories().size() - 1 );
    mRenderer = renderer;
    endInsertRows();
  }
}

void QgsCategorizedSymbolRendererV2Model::addCategory( const QgsRendererCategoryV2 &cat )
{
  if ( !mRenderer ) return;
  int idx = mRenderer->categories().size();
  beginInsertRows( QModelIndex(), idx, idx );
  mRenderer->addCategory( cat );
  endInsertRows();
}

QgsRendererCategoryV2 QgsCategorizedSymbolRendererV2Model::category( const QModelIndex &index )
{
  if ( !mRenderer )
  {
    return QgsRendererCategoryV2();
  }
  const QgsCategoryList& catList = mRenderer->categories();
  int row = index.row();
  if ( row >= catList.size() )
  {
    return QgsRendererCategoryV2();
  }
  return catList.at( row );
}


Qt::ItemFlags QgsCategorizedSymbolRendererV2Model::flags( const QModelIndex & index ) const
{
  if ( !index.isValid() )
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;
  if ( index.column() == 1 || index.column() == 2 )
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

Qt::DropActions QgsCategorizedSymbolRendererV2Model::supportedDropActions() const
{
  return Qt::MoveAction;
}

QVariant QgsCategorizedSymbolRendererV2Model::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mRenderer )
    return QVariant();

  const QgsRendererCategoryV2 category = mRenderer->categories().value( index.row() );

  if ( role == Qt::CheckStateRole && index.column() == 0 )
  {
    return category.renderState() ? Qt::Checked : Qt::Unchecked;
  }
  else if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
  {
    switch ( index.column() )
    {
      case 1: return category.value().toString();
      case 2: return category.label();
      default: return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && category.symbol() )
  {
    return QgsSymbolLayerV2Utils::symbolPreviewIcon( category.symbol(), QSize( 16, 16 ) );
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return ( index.column() == 0 ) ? Qt::AlignHCenter : Qt::AlignLeft;
  }
  else if ( role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case 1: return category.value();
      case 2: return category.label();
      default: return QVariant();
    }
  }

  return QVariant();
}

bool QgsCategorizedSymbolRendererV2Model::setData( const QModelIndex & index, const QVariant & value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.column() == 0 && role == Qt::CheckStateRole )
  {
    mRenderer->updateCategoryRenderState( index.row(), value == Qt::Checked );
    emit dataChanged( index, index );
    return true;
  }

  if ( role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case 1: // value
    {
      // try to preserve variant type for this value
      QVariant val;
      switch ( mRenderer->categories().value( index.row() ).value().type() )
      {
        case QVariant::Int:
          val = value.toInt();
          break;
        case QVariant::Double:
          val = value.toDouble();
          break;
        default:
          val = value.toString();
          break;
      }
      mRenderer->updateCategoryValue( index.row(), val );
      break;
    }
    case 2: // label
      mRenderer->updateCategoryLabel( index.row(), value.toString() );
      break;
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

QVariant QgsCategorizedSymbolRendererV2Model::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 3 )
  {
    QStringList lst; lst << tr( "Symbol" ) << tr( "Value" ) << tr( "Legend" );
    return lst.value( section );
  }
  return QVariant();
}

int QgsCategorizedSymbolRendererV2Model::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() || !mRenderer )
  {
    return 0;
  }
  return mRenderer->categories().size();
}

int QgsCategorizedSymbolRendererV2Model::columnCount( const QModelIndex & index ) const
{
  Q_UNUSED( index );
  return 3;
}

QModelIndex QgsCategorizedSymbolRendererV2Model::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column );
  }
  return QModelIndex();
}

QModelIndex QgsCategorizedSymbolRendererV2Model::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return QModelIndex();
}

QStringList QgsCategorizedSymbolRendererV2Model::mimeTypes() const
{
  QStringList types;
  types << mMimeFormat;
  return types;
}

QMimeData *QgsCategorizedSymbolRendererV2Model::mimeData( const QModelIndexList &indexes ) const
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

bool QgsCategorizedSymbolRendererV2Model::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
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
  if ( to == -1 ) to = mRenderer->categories().size(); // out of rang ok, will be decreased
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    QgsDebugMsg( QString( "move %1 to %2" ).arg( rows[i] ).arg( to ) );
    int t = to;
    // moveCategory first removes and then inserts
    if ( rows[i] < t ) t--;
    mRenderer->moveCategory( rows[i], t );
    // current moved under another, shift its index up
    for ( int j = 0; j < i; j++ )
    {
      if ( to < rows[j] && rows[i] > rows[j] ) rows[j] += 1;
    }
    // removed under 'to' so the target shifted down
    if ( rows[i] < to ) to--;
  }
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->categories().size(), 0 ) );
  emit rowsMoved();
  return false;
}

void QgsCategorizedSymbolRendererV2Model::deleteRows( QList<int> rows )
{
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    beginRemoveRows( QModelIndex(), rows[i], rows[i] );
    mRenderer->deleteCategory( rows[i] );
    endRemoveRows();
  }
}

void QgsCategorizedSymbolRendererV2Model::removeAllRows()
{
  beginRemoveRows( QModelIndex(), 0, mRenderer->categories().size() - 1 );
  mRenderer->deleteAllCategories();
  endRemoveRows();
}

void QgsCategorizedSymbolRendererV2Model::sort( int column, Qt::SortOrder order )
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
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->categories().size(), 0 ) );
  QgsDebugMsg( "Done" );
}

void QgsCategorizedSymbolRendererV2Model::updateSymbology()
{
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->categories().size(), 0 ) );
}

// ------------------------------ View style --------------------------------
QgsCategorizedSymbolRendererV2ViewStyle::QgsCategorizedSymbolRendererV2ViewStyle( QStyle* style )
    : QProxyStyle( style )
{}

void QgsCategorizedSymbolRendererV2ViewStyle::drawPrimitive( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget ) const
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
QgsRendererV2Widget* QgsCategorizedSymbolRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsCategorizedSymbolRendererV2Widget( layer, style, renderer );
}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( 0 )
  //TODO - use actual map canvas settings
  << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );

  const QgsVectorLayer* layer = ( const QgsVectorLayer* ) context;
  if ( layer )
    expContext << QgsExpressionContextUtils::layerScope( layer );

  return expContext;
}

QgsCategorizedSymbolRendererV2Widget::QgsCategorizedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( 0 )
    , mModel( 0 )
{

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( renderer )
  {
    mRenderer = QgsCategorizedSymbolRendererV2::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsCategorizedSymbolRendererV2( "", QgsCategoryList() );
  }

  QString attrName = mRenderer->classAttribute();
  mOldClassificationAttribute = attrName;

  // setup user interface
  setupUi( this );

  mExpressionWidget->setLayer( mLayer );

  cboCategorizedColorRamp->populate( mStyle );
  int randomIndex = cboCategorizedColorRamp->findText( tr( "Random colors" ) );
  if ( randomIndex != -1 )
  {
    cboCategorizedColorRamp->setCurrentIndex( randomIndex );
  }

  // set project default color ramp
  QString defaultColorRamp = QgsProject::instance()->readEntry( "DefaultStyles", "/ColorRamp", "" );
  if ( defaultColorRamp != "" )
  {
    int index = cboCategorizedColorRamp->findText( defaultColorRamp, Qt::MatchCaseSensitive );
    if ( index >= 0 )
      cboCategorizedColorRamp->setCurrentIndex( index );
  }

  mCategorizedSymbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

  mModel = new QgsCategorizedSymbolRendererV2Model( this );
  mModel->setRenderer( mRenderer );

  // update GUI from renderer
  updateUiFromRenderer();

  viewCategories->setModel( mModel );
  viewCategories->resizeColumnToContents( 0 );
  viewCategories->resizeColumnToContents( 1 );
  viewCategories->resizeColumnToContents( 2 );

  viewCategories->setStyle( new QgsCategorizedSymbolRendererV2ViewStyle( viewCategories->style() ) );

  connect( mModel, SIGNAL( rowsMoved() ), this, SLOT( rowsMoved() ) );

  connect( mExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( categoryColumnChanged( QString ) ) );

  connect( viewCategories, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( categoriesDoubleClicked( const QModelIndex & ) ) );
  connect( viewCategories, SIGNAL( customContextMenuRequested( const QPoint& ) ),  this, SLOT( contextMenuViewCategories( const QPoint& ) ) );

  connect( btnChangeCategorizedSymbol, SIGNAL( clicked() ), this, SLOT( changeCategorizedSymbol() ) );
  connect( btnAddCategories, SIGNAL( clicked() ), this, SLOT( addCategories() ) );
  connect( btnDeleteCategories, SIGNAL( clicked() ), this, SLOT( deleteCategories() ) );
  connect( btnDeleteAllCategories, SIGNAL( clicked() ), this, SLOT( deleteAllCategories() ) );
  connect( btnAddCategory, SIGNAL( clicked() ), this, SLOT( addCategory() ) );
  connect( cbxInvertedColorRamp, SIGNAL( toggled( bool ) ), this, SLOT( applyColorRamp() ) );
  connect( cboCategorizedColorRamp, SIGNAL( currentIndexChanged( int ) ), this, SLOT( applyColorRamp() ) );
  connect( cboCategorizedColorRamp, SIGNAL( sourceRampEdited() ), this, SLOT( applyColorRamp() ) );
  connect( mButtonEditRamp, SIGNAL( clicked() ), cboCategorizedColorRamp, SLOT( editSourceRamp() ) );

  // menus for data-defined rotation/size
  QMenu* advMenu = new QMenu;

  advMenu->addAction( tr( "Match to saved symbols" ), this, SLOT( matchToSymbolsFromLibrary() ) );
  advMenu->addAction( tr( "Match to symbols from file..." ), this, SLOT( matchToSymbolsFromXml() ) );
  advMenu->addAction( tr( "Symbol levels..." ), this, SLOT( showSymbolLevels() ) );

  btnAdvanced->setMenu( advMenu );

  mExpressionWidget->registerGetExpressionContextCallback( &_getExpressionContext, layer );
}

QgsCategorizedSymbolRendererV2Widget::~QgsCategorizedSymbolRendererV2Widget()
{
  if ( mRenderer ) delete mRenderer;
  if ( mModel ) delete mModel;
  delete mCategorizedSymbol;
}

void QgsCategorizedSymbolRendererV2Widget::updateUiFromRenderer()
{
  // Note: This assumes that the signals for UI element changes have not
  // yet been connected, so that the updates to color ramp, symbol, etc
  // don't override existing customisations.

  updateCategorizedSymbolIcon();

  //mModel->setRenderer ( mRenderer ); // necessary?

  // set column
  QString attrName = mRenderer->classAttribute();
  mExpressionWidget->setField( attrName );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    delete mCategorizedSymbol;
    mCategorizedSymbol = mRenderer->sourceSymbol()->clone();
    updateCategorizedSymbolIcon();
  }

  // set source color ramp
  if ( mRenderer->sourceColorRamp() )
  {
    cboCategorizedColorRamp->setSourceColorRamp( mRenderer->sourceColorRamp() );
    cbxInvertedColorRamp->setChecked( mRenderer->invertedColorRamp() );
  }

}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}

void QgsCategorizedSymbolRendererV2Widget::changeSelectedSymbols()
{
  QList<int> selectedCats = selectedCategories();

  if ( selectedCats.size() > 0 )
  {
    QgsSymbolV2* newSymbol = mCategorizedSymbol->clone();
    QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, mLayer, this );
    if ( !dlg.exec() )
    {
      delete newSymbol;
      return;
    }

    Q_FOREACH ( int idx, selectedCats )
    {
      QgsRendererCategoryV2 category = mRenderer->categories().value( idx );

      QgsSymbolV2* newCatSymbol = newSymbol->clone();
      newCatSymbol->setColor( mRenderer->categories()[idx].symbol()->color() );
      mRenderer->updateCategorySymbol( idx, newCatSymbol );
    }
  }
}

void QgsCategorizedSymbolRendererV2Widget::changeCategorizedSymbol()
{
  // When there is a slection, change the selected symbols alone
  QItemSelectionModel* m = viewCategories->selectionModel();
  QModelIndexList i = m->selectedRows();

  if ( m && i.size() > 0 )
  {
    changeSelectedSymbols();
    return;
  }

  // When there is no selection, change the base mCategorizedSymbol
  QgsSymbolV2* newSymbol = mCategorizedSymbol->clone();

  QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, mLayer, this );
  if ( !dlg.exec() )
  {
    delete newSymbol;
    return;
  }

  delete mCategorizedSymbol;
  mCategorizedSymbol = newSymbol;
  updateCategorizedSymbolIcon();

  mRenderer->updateSymbols( mCategorizedSymbol );
}

void QgsCategorizedSymbolRendererV2Widget::updateCategorizedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mCategorizedSymbol, btnChangeCategorizedSymbol->iconSize() );
  btnChangeCategorizedSymbol->setIcon( icon );
}

void QgsCategorizedSymbolRendererV2Widget::populateCategories()
{
}

void QgsCategorizedSymbolRendererV2Widget::categoryColumnChanged( QString field )
{
  mRenderer->setClassAttribute( field );
}

void QgsCategorizedSymbolRendererV2Widget::categoriesDoubleClicked( const QModelIndex & idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeCategorySymbol();
}

void QgsCategorizedSymbolRendererV2Widget::changeCategorySymbol()
{
  int catIdx = currentCategoryRow();
  QgsRendererCategoryV2 category = mRenderer->categories().value( currentCategoryRow() );

  QgsSymbolV2 *symbol = category.symbol();
  if ( symbol )
  {
    symbol = symbol->clone();
  }
  else
  {
    symbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );
  }

  QgsSymbolV2SelectorDialog dlg( symbol, mStyle, mLayer, this );
  if ( !dlg.exec() )
  {
    delete symbol;
    return;
  }

  mRenderer->updateCategorySymbol( catIdx, symbol );
}

static void _createCategories( QgsCategoryList& cats, QList<QVariant>& values, QgsSymbolV2* symbol )
{
  // sort the categories first
  QgsSymbolLayerV2Utils::sortVariantList( values, Qt::AscendingOrder );

  int num = values.count();

  bool hasNull = false;

  for ( int i = 0; i < num; i++ )
  {
    QVariant value = values[i];
    if ( value.toString().isNull() )
    {
      hasNull = true;
    }
    QgsSymbolV2* newSymbol = symbol->clone();

    cats.append( QgsRendererCategoryV2( value, newSymbol, value.toString(), true ) );
  }

  // add null (default) value if not exists
  if ( !hasNull )
  {
    QgsSymbolV2* newSymbol = symbol->clone();
    cats.append( QgsRendererCategoryV2( QVariant( "" ), newSymbol, QString(), true ) );
  }
}

QgsVectorColorRampV2* QgsCategorizedSymbolRendererV2Widget::getColorRamp()
{
  QgsVectorColorRampV2* ramp = cboCategorizedColorRamp->currentColorRamp();
  if ( ramp == NULL )
  {
    if ( cboCategorizedColorRamp->count() == 0 )
      QMessageBox::critical( this, tr( "Error" ), tr( "There are no available color ramps. You can add them in Style Manager." ) );
    else if ( !cboCategorizedColorRamp->createNewColorRampSelected() )
      QMessageBox::critical( this, tr( "Error" ), tr( "The selected color ramp is not available." ) );
  }
  return ramp;
}


void QgsCategorizedSymbolRendererV2Widget::addCategories()
{
  QString attrName = mExpressionWidget->currentField();
  int idx = mLayer->fieldNameIndex( attrName );
  QList<QVariant> unique_vals;
  if ( idx == -1 )
  {
    // Lets assume it's an expression
    QgsExpression* expression = new QgsExpression( attrName );
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::atlasScope( 0 )
    << QgsExpressionContextUtils::layerScope( mLayer );

    expression->prepare( &context );
    QgsFeatureIterator fit = mLayer->getFeatures();
    QgsFeature feature;
    while ( fit.nextFeature( feature ) )
    {
      context.setFeature( feature );
      QVariant value = expression->evaluate( &context );
      if ( unique_vals.contains( value ) )
        continue;
      unique_vals << value;
    }
  }
  else
  {
    mLayer->uniqueValues( idx, unique_vals );
  }

  // ask to abort if too many classes
  if ( unique_vals.size() >= 1000 )
  {
    int res = QMessageBox::warning( 0, tr( "High number of classes!" ),
                                    tr( "Classification would yield %1 entries which might not be expected. Continue?" ).arg( unique_vals.size() ),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Cancel );
    if ( res == QMessageBox::Cancel )
    {
      return;
    }
  }

#if 0
  DlgAddCategories dlg( mStyle, createDefaultSymbol(), unique_vals, this );
  if ( !dlg.exec() )
    return;
#endif

  QgsCategoryList cats;
  _createCategories( cats, unique_vals, mCategorizedSymbol );
  bool deleteExisting = false;

  if ( !mOldClassificationAttribute.isEmpty() &&
       attrName != mOldClassificationAttribute &&
       mRenderer->categories().count() > 0 )
  {
    int res = QMessageBox::question( this,
                                     tr( "Confirm Delete" ),
                                     tr( "The classification field was changed from '%1' to '%2'.\n"
                                         "Should the existing classes be deleted before classification?" )
                                     .arg( mOldClassificationAttribute ).arg( attrName ),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
    if ( res == QMessageBox::Cancel )
    {
      return;
    }

    deleteExisting = ( res == QMessageBox::Yes );
  }

  // First element to apply coloring to
  bool keepExistingColors = false;
  if ( !deleteExisting )
  {
    QgsCategoryList prevCats = mRenderer->categories();
    keepExistingColors = prevCats.size() > 0;
    for ( int i = 0; i < cats.size(); ++i )
    {
      bool contains = false;
      QVariant value = cats.at( i ).value();
      for ( int j = 0; j < prevCats.size() && !contains; ++j )
      {
        if ( prevCats.at( j ).value() == value )
        {
          contains = true;
          break;
        }
      }

      if ( !contains )
        prevCats.append( cats.at( i ) );
    }
    cats = prevCats;
  }

  mOldClassificationAttribute = attrName;

  // TODO: if not all categories are desired, delete some!
  /*
  if (not dlg.readAllCats.isChecked())
  {
    cats2 = {}
    for item in dlg.listCategories.selectedItems():
      for k,c in cats.iteritems():
        if item.text() == k.toString():
          break
      cats2[k] = c
    cats = cats2
  }
  */

  // recreate renderer
  QgsCategorizedSymbolRendererV2 *r = new QgsCategorizedSymbolRendererV2( attrName, cats );
  r->setSourceSymbol( mCategorizedSymbol->clone() );
  r->setScaleMethod( mRenderer->scaleMethod() );
  r->setSizeScaleField( mRenderer->sizeScaleField() );
  r->setInvertedColorRamp( cbxInvertedColorRamp->isChecked() );
  QgsVectorColorRampV2* ramp = getColorRamp();
  if ( ramp ) r->setSourceColorRamp( ramp->clone() );

  if ( mModel )
  {
    mModel->setRenderer( r );
  }
  delete mRenderer;
  mRenderer = r;
  if ( ! keepExistingColors && ramp ) applyColorRamp();
  delete ramp;
}

void QgsCategorizedSymbolRendererV2Widget::applyColorRamp()
{
  QgsVectorColorRampV2* ramp = getColorRamp();
  if ( ramp )
  {
    mRenderer->updateColorRamp( ramp, cbxInvertedColorRamp->isChecked() );
  }
  mModel->updateSymbology();
}

int QgsCategorizedSymbolRendererV2Widget::currentCategoryRow()
{
  QModelIndex idx = viewCategories->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}

QList<int> QgsCategorizedSymbolRendererV2Widget::selectedCategories()
{
  QList<int> rows;
  QModelIndexList selectedRows = viewCategories->selectionModel()->selectedRows();

  Q_FOREACH ( const QModelIndex& r, selectedRows )
  {
    if ( r.isValid() )
    {
      rows.append( r.row() );
    }
  }
  return rows;
}

void QgsCategorizedSymbolRendererV2Widget::deleteCategories()
{
  QList<int> categoryIndexes = selectedCategories();
  mModel->deleteRows( categoryIndexes );
}

void QgsCategorizedSymbolRendererV2Widget::deleteAllCategories()
{
  mModel->removeAllRows();
}

void QgsCategorizedSymbolRendererV2Widget::addCategory()
{
  if ( !mModel ) return;
  QgsSymbolV2 *symbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );
  QgsRendererCategoryV2 cat( QString(), symbol, QString(), true );
  mModel->addCategory( cat );
}

void QgsCategorizedSymbolRendererV2Widget::sizeScaleFieldChanged( QString fldName )
{
  mRenderer->setSizeScaleField( fldName );
}

void QgsCategorizedSymbolRendererV2Widget::scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mRenderer->setScaleMethod( scaleMethod );
}

QList<QgsSymbolV2*> QgsCategorizedSymbolRendererV2Widget::selectedSymbols()
{
  QList<QgsSymbolV2*> selectedSymbols;

  QItemSelectionModel* m = viewCategories->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );

  if ( m && selectedIndexes.size() > 0 )
  {
    const QgsCategoryList& categories = mRenderer->categories();
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      int row = ( *indexIt ).row();
      QgsSymbolV2* s = categories[row].symbol();
      if ( s )
      {
        selectedSymbols.append( s );
      }
    }
  }
  return selectedSymbols;
}

QgsCategoryList QgsCategorizedSymbolRendererV2Widget::selectedCategoryList()
{
  QgsCategoryList cl;

  QItemSelectionModel* m = viewCategories->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );

  if ( m && selectedIndexes.size() > 0 )
  {
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      cl.append( mModel->category( *indexIt ) );
    }
  }
  return cl;
}

void QgsCategorizedSymbolRendererV2Widget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer );
}

void QgsCategorizedSymbolRendererV2Widget::rowsMoved()
{
  viewCategories->selectionModel()->clear();
}

void QgsCategorizedSymbolRendererV2Widget::matchToSymbolsFromLibrary()
{
  int matched = matchToSymbols( QgsStyleV2::defaultStyle() );
  if ( matched > 0 )
  {
    QMessageBox::information( this, tr( "Matched symbols" ),
                              tr( "Matched %1 categories to symbols." ).arg( matched ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Matched symbols" ),
                          tr( "No categories could be matched to symbols in library." ) );
  }
}

int QgsCategorizedSymbolRendererV2Widget::matchToSymbols( QgsStyleV2* style )
{
  if ( !mLayer || !style )
    return 0;

  int matched = 0;
  for ( int catIdx = 0; catIdx < mRenderer->categories().count(); ++catIdx )
  {
    QString val = mRenderer->categories().at( catIdx ).value().toString();
    QgsSymbolV2* symbol = style->symbol( val );
    if ( symbol &&
         (( symbol->type() == QgsSymbolV2::Marker && mLayer->geometryType() == QGis::Point )
          || ( symbol->type() == QgsSymbolV2::Line && mLayer->geometryType() == QGis::Line )
          || ( symbol->type() == QgsSymbolV2::Fill && mLayer->geometryType() == QGis::Polygon ) ) )
    {
      matched++;
      mRenderer->updateCategorySymbol( catIdx, symbol->clone() );
    }
  }
  mModel->updateSymbology();
  return matched;
}

void QgsCategorizedSymbolRendererV2Widget::matchToSymbolsFromXml()
{
  QSettings settings;
  QString openFileDir = settings.value( "UI/lastMatchToSymbolsDir", "" ).toString();

  QString fileName = QFileDialog::getOpenFileName( this, tr( "Match to symbols from file" ), openFileDir,
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QFileInfo openFileInfo( fileName );
  settings.setValue( "UI/lastMatchToSymbolsDir", openFileInfo.absolutePath() );

  QgsStyleV2 importedStyle;
  if ( !importedStyle.importXML( fileName ) )
  {
    QMessageBox::warning( this, tr( "Matching error" ),
                          tr( "An error occured reading file:\n%1" ).arg( importedStyle.errorString() ) );
    return;
  }

  int matched = matchToSymbols( &importedStyle );
  if ( matched > 0 )
  {
    QMessageBox::information( this, tr( "Matched symbols" ),
                              tr( "Matched %1 categories to symbols from file." ).arg( matched ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Matched symbols" ),
                          tr( "No categories could be matched to symbols in file." ) );
  }
}

void QgsCategorizedSymbolRendererV2Widget::keyPressEvent( QKeyEvent* event )
{
  if ( !event )
  {
    return;
  }

  if ( event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier )
  {
    mCopyBuffer.clear();
    mCopyBuffer = selectedCategoryList();
  }
  else if ( event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier )
  {
    QgsCategoryList::const_iterator rIt = mCopyBuffer.constBegin();
    for ( ; rIt != mCopyBuffer.constEnd(); ++rIt )
    {
      mModel->addCategory( *rIt );
    }
  }
}
