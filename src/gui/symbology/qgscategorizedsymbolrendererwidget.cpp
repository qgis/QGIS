/***************************************************************************
    qgscategorizedsymbolrendererwidget.cpp
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

#include "qgscategorizedsymbolrendererwidget.h"
#include "qgspanelwidget.h"

#include "qgscategorizedsymbolrenderer.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgsdatadefinedsizelegendwidget.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgsstyle.h"
#include "qgslogger.h"

#include "qgssymbolselectordialog.h"
#include "qgsexpressionbuilderdialog.h"

#include "qgsvectorlayer.h"
#include "qgsfeatureiterator.h"

#include "qgsproject.h"
#include "qgsexpression.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPen>
#include <QPainter>
#include <QFileDialog>

///@cond PRIVATE

QgsCategorizedSymbolRendererModel::QgsCategorizedSymbolRendererModel( QObject *parent ) : QAbstractItemModel( parent )
  , mMimeFormat( QStringLiteral( "application/x-qgscategorizedsymbolrendererv2model" ) )
{
}

void QgsCategorizedSymbolRendererModel::setRenderer( QgsCategorizedSymbolRenderer *renderer )
{
  if ( mRenderer )
  {
    beginRemoveRows( QModelIndex(), 0, std::max( mRenderer->categories().size() - 1, 0 ) );
    mRenderer = nullptr;
    endRemoveRows();
  }
  if ( renderer )
  {
    mRenderer = renderer;
    if ( renderer->categories().size() > 0 )
    {
      beginInsertRows( QModelIndex(), 0, renderer->categories().size() - 1 );
      endInsertRows();
    }
  }
}

void QgsCategorizedSymbolRendererModel::addCategory( const QgsRendererCategory &cat )
{
  if ( !mRenderer ) return;
  int idx = mRenderer->categories().size();
  beginInsertRows( QModelIndex(), idx, idx );
  mRenderer->addCategory( cat );
  endInsertRows();
}

QgsRendererCategory QgsCategorizedSymbolRendererModel::category( const QModelIndex &index )
{
  if ( !mRenderer )
  {
    return QgsRendererCategory();
  }
  const QgsCategoryList &catList = mRenderer->categories();
  int row = index.row();
  if ( row >= catList.size() )
  {
    return QgsRendererCategory();
  }
  return catList.at( row );
}


Qt::ItemFlags QgsCategorizedSymbolRendererModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || !mRenderer )
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;
  if ( index.column() == 1 )
  {
    const QgsRendererCategory category = mRenderer->categories().value( index.row() );
    if ( category.value().type() != QVariant::List )
    {
      flags |= Qt::ItemIsEditable;
    }
  }
  else if ( index.column() == 2 )
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

Qt::DropActions QgsCategorizedSymbolRendererModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QVariant QgsCategorizedSymbolRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mRenderer )
    return QVariant();

  const QgsRendererCategory category = mRenderer->categories().value( index.row() );

  switch ( role )
  {
    case Qt::CheckStateRole:
    {
      if ( index.column() == 0 )
      {
        return category.renderState() ? Qt::Checked : Qt::Unchecked;
      }
      break;
    }

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case 1:
        {
          if ( category.value().type() == QVariant::List )
          {
            QStringList res;
            const QVariantList list = category.value().toList();
            res.reserve( list.size() );
            for ( const QVariant &v : list )
              res << v.toString();

            if ( role == Qt::DisplayRole )
              return res.join( ';' );
            else // tooltip
              return res.join( '\n' );
          }
          else if ( !category.value().isValid() || category.value().isNull() || category.value().toString().isEmpty() )
          {
            return tr( "all other values" );
          }
          else
          {
            return category.value().toString();
          }
        }
        case 2:
          return category.label();
      }
      break;
    }

    case Qt::FontRole:
    {
      if ( index.column() == 1 && category.value().type() != QVariant::List && ( !category.value().isValid() || category.value().isNull() || category.value().toString().isEmpty() ) )
      {
        QFont italicFont;
        italicFont.setItalic( true );
        return italicFont;
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( index.column() == 0 && category.symbol() )
      {
        return QgsSymbolLayerUtils::symbolPreviewIcon( category.symbol(), QSize( 16, 16 ) );
      }
      break;
    }

    case Qt::ForegroundRole:
    {
      QBrush brush( qApp->palette().color( QPalette::Text ), Qt::SolidPattern );
      if ( index.column() == 1 && ( category.value().type() == QVariant::List
                                    || !category.value().isValid() || category.value().isNull() || category.value().toString().isEmpty() ) )
      {
        QColor fadedTextColor = brush.color();
        fadedTextColor.setAlpha( 128 );
        brush.setColor( fadedTextColor );
      }
      return brush;
    }

    case Qt::TextAlignmentRole:
    {
      return ( index.column() == 0 ) ? Qt::AlignHCenter : Qt::AlignLeft;
    }

    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case 1:
        {
          if ( category.value().type() == QVariant::List )
          {
            QStringList res;
            const QVariantList list = category.value().toList();
            res.reserve( list.size() );
            for ( const QVariant &v : list )
              res << v.toString();

            return res.join( ';' );
          }
          else
          {
            return category.value();
          }
        }

        case 2:
          return category.label();
      }
      break;
    }
  }

  return QVariant();
}

bool QgsCategorizedSymbolRendererModel::setData( const QModelIndex &index, const QVariant &value, int role )
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
        case QVariant::List:
        {
          const QStringList parts = value.toString().split( ';' );
          QVariantList list;
          list.reserve( parts.count() );
          for ( const QString &p : parts )
            list << p;

          if ( list.count() == 1 )
            val = list.at( 0 );
          else
            val = list;
          break;
        }
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

QVariant QgsCategorizedSymbolRendererModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 3 )
  {
    QStringList lst;
    lst << tr( "Symbol" ) << tr( "Value" ) << tr( "Legend" );
    return lst.value( section );
  }
  return QVariant();
}

int QgsCategorizedSymbolRendererModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() || !mRenderer )
  {
    return 0;
  }
  return mRenderer->categories().size();
}

int QgsCategorizedSymbolRendererModel::columnCount( const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return 3;
}

QModelIndex QgsCategorizedSymbolRendererModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column );
  }
  return QModelIndex();
}

QModelIndex QgsCategorizedSymbolRendererModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );
  return QModelIndex();
}

QStringList QgsCategorizedSymbolRendererModel::mimeTypes() const
{
  QStringList types;
  types << mMimeFormat;
  return types;
}

QMimeData *QgsCategorizedSymbolRendererModel::mimeData( const QModelIndexList &indexes ) const
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

bool QgsCategorizedSymbolRendererModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
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
    QgsDebugMsg( QStringLiteral( "move %1 to %2" ).arg( rows[i] ).arg( to ) );
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

void QgsCategorizedSymbolRendererModel::deleteRows( QList<int> rows )
{
  std::sort( rows.begin(), rows.end() ); // list might be unsorted, depending on how the user selected the rows
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    beginRemoveRows( QModelIndex(), rows[i], rows[i] );
    mRenderer->deleteCategory( rows[i] );
    endRemoveRows();
  }
}

void QgsCategorizedSymbolRendererModel::removeAllRows()
{
  beginRemoveRows( QModelIndex(), 0, mRenderer->categories().size() - 1 );
  mRenderer->deleteAllCategories();
  endRemoveRows();
}

void QgsCategorizedSymbolRendererModel::sort( int column, Qt::SortOrder order )
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
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->categories().size(), 0 ) );
}

void QgsCategorizedSymbolRendererModel::updateSymbology()
{
  emit dataChanged( createIndex( 0, 0 ), createIndex( mRenderer->categories().size(), 0 ) );
}

// ------------------------------ View style --------------------------------
QgsCategorizedSymbolRendererViewStyle::QgsCategorizedSymbolRendererViewStyle( QWidget *parent )
  : QgsProxyStyle( parent )
{}

void QgsCategorizedSymbolRendererViewStyle::drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const
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
QgsRendererWidget *QgsCategorizedSymbolRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsCategorizedSymbolRendererWidget( layer, style, renderer );
}

QgsCategorizedSymbolRendererWidget::QgsCategorizedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
  , mContextMenu( new QMenu( this ) )
{

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( renderer )
  {
    mRenderer.reset( QgsCategorizedSymbolRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = qgis::make_unique< QgsCategorizedSymbolRenderer >( QString(), QgsCategoryList() );
  }

  QString attrName = mRenderer->classAttribute();
  mOldClassificationAttribute = attrName;

  // setup user interface
  setupUi( this );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mExpressionWidget->setLayer( mLayer );

  // initiate color ramp button to random
  btnColorRamp->setShowRandomColorRamp( true );

  // set project default color ramp
  QString defaultColorRamp = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), QString() );
  if ( !defaultColorRamp.isEmpty() )
  {
    btnColorRamp->setColorRampFromName( defaultColorRamp );
  }
  else
  {
    btnColorRamp->setRandomColorRamp();
  }

  mCategorizedSymbol.reset( QgsSymbol::defaultSymbol( mLayer->geometryType() ) );

  mModel = new QgsCategorizedSymbolRendererModel( this );
  mModel->setRenderer( mRenderer.get() );

  // update GUI from renderer
  updateUiFromRenderer();

  viewCategories->setModel( mModel );
  viewCategories->resizeColumnToContents( 0 );
  viewCategories->resizeColumnToContents( 1 );
  viewCategories->resizeColumnToContents( 2 );

  viewCategories->setStyle( new QgsCategorizedSymbolRendererViewStyle( viewCategories ) );

  connect( mModel, &QgsCategorizedSymbolRendererModel::rowsMoved, this, &QgsCategorizedSymbolRendererWidget::rowsMoved );
  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsPanelWidget::widgetChanged );

  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsCategorizedSymbolRendererWidget::categoryColumnChanged );

  connect( viewCategories, &QAbstractItemView::doubleClicked, this, &QgsCategorizedSymbolRendererWidget::categoriesDoubleClicked );
  connect( viewCategories, &QTreeView::customContextMenuRequested, this, &QgsCategorizedSymbolRendererWidget::showContextMenu );

  connect( btnChangeCategorizedSymbol, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::changeCategorizedSymbol );
  connect( btnAddCategories, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::addCategories );
  connect( btnDeleteCategories, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::deleteCategories );
  connect( btnDeleteAllCategories, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::deleteAllCategories );
  connect( btnAddCategory, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::addCategory );

  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsCategorizedSymbolRendererWidget::applyColorRamp );

  // menus for data-defined rotation/size
  QMenu *advMenu = new QMenu;

  advMenu->addAction( tr( "Match to Saved Symbols" ), this, SLOT( matchToSymbolsFromLibrary() ) );
  advMenu->addAction( tr( "Match to Symbols from File…" ), this, SLOT( matchToSymbolsFromXml() ) );
  advMenu->addAction( tr( "Symbol Levels…" ), this, SLOT( showSymbolLevels() ) );
  if ( mCategorizedSymbol->type() == QgsSymbol::Marker )
  {
    QAction *actionDdsLegend = advMenu->addAction( tr( "Data-defined Size Legend…" ) );
    // only from Qt 5.6 there is convenience addAction() with new style connection
    connect( actionDdsLegend, &QAction::triggered, this, &QgsCategorizedSymbolRendererWidget::dataDefinedSizeLegend );
  }

  btnAdvanced->setMenu( advMenu );

  mExpressionWidget->registerExpressionContextGenerator( this );

  mMergeCategoriesAction = new QAction( tr( "Merge Categories" ), this );
  connect( mMergeCategoriesAction, &QAction::triggered, this, &QgsCategorizedSymbolRendererWidget::mergeSelectedCategories );
  mUnmergeCategoriesAction = new QAction( tr( "Unmerge Categories" ), this );
  connect( mUnmergeCategoriesAction, &QAction::triggered, this, &QgsCategorizedSymbolRendererWidget::unmergeSelectedCategories );
}

QgsCategorizedSymbolRendererWidget::~QgsCategorizedSymbolRendererWidget()
{
  delete mModel;
}

void QgsCategorizedSymbolRendererWidget::updateUiFromRenderer()
{
  // Note: This assumes that the signals for UI element changes have not
  // yet been connected, so that the updates to color ramp, symbol, etc
  // don't override existing customizations.

  updateCategorizedSymbolIcon();

  //mModel->setRenderer ( mRenderer ); // necessary?

  // set column
  QString attrName = mRenderer->classAttribute();
  mExpressionWidget->setField( attrName );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    mCategorizedSymbol.reset( mRenderer->sourceSymbol()->clone() );
    updateCategorizedSymbolIcon();
  }

  // if a color ramp attached to the renderer, enable the color ramp button
  if ( mRenderer->sourceColorRamp() )
  {
    btnColorRamp->setColorRamp( mRenderer->sourceColorRamp() );
  }
}

QgsFeatureRenderer *QgsCategorizedSymbolRendererWidget::renderer()
{
  return mRenderer.get();
}

void QgsCategorizedSymbolRendererWidget::changeSelectedSymbols()
{
  QList<int> selectedCats = selectedCategories();

  if ( !selectedCats.isEmpty() )
  {
    QgsSymbol *newSymbol = mCategorizedSymbol->clone();
    QgsSymbolSelectorDialog dlg( newSymbol, mStyle, mLayer, this );
    dlg.setContext( context() );
    if ( !dlg.exec() )
    {
      delete newSymbol;
      return;
    }

    Q_FOREACH ( int idx, selectedCats )
    {
      QgsRendererCategory category = mRenderer->categories().value( idx );

      QgsSymbol *newCatSymbol = newSymbol->clone();
      newCatSymbol->setColor( mRenderer->categories()[idx].symbol()->color() );
      mRenderer->updateCategorySymbol( idx, newCatSymbol );
    }
  }
}

void QgsCategorizedSymbolRendererWidget::changeCategorizedSymbol()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  std::unique_ptr<QgsSymbol> newSymbol( mCategorizedSymbol->clone() );
  if ( panel && panel->dockMode() )
  {
    // bit tricky here - the widget doesn't take ownership of the symbol. So we need it to last for the duration of the
    // panel's existence. Accordingly, just kinda give it ownership here, and clean up in cleanUpSymbolSelector
    QgsSymbolSelectorWidget *dlg = new QgsSymbolSelectorWidget( newSymbol.release(), mStyle, mLayer, panel );
    dlg->setContext( mContext );
    connect( dlg, &QgsPanelWidget::widgetChanged, this, &QgsCategorizedSymbolRendererWidget::updateSymbolsFromWidget );
    connect( dlg, &QgsPanelWidget::panelAccepted, this, &QgsCategorizedSymbolRendererWidget::cleanUpSymbolSelector );
    connect( dlg, &QgsPanelWidget::panelAccepted, this, &QgsCategorizedSymbolRendererWidget::updateCategorizedSymbolIcon );
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

    mCategorizedSymbol = std::move( newSymbol );
    updateCategorizedSymbolIcon();
    applyChangeToSymbol();
  }
}

void QgsCategorizedSymbolRendererWidget::updateCategorizedSymbolIcon()
{
  if ( !mCategorizedSymbol )
    return;

  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mCategorizedSymbol.get(), btnChangeCategorizedSymbol->iconSize() );
  btnChangeCategorizedSymbol->setIcon( icon );
}

void QgsCategorizedSymbolRendererWidget::populateCategories()
{
}

void QgsCategorizedSymbolRendererWidget::categoryColumnChanged( const QString &field )
{
  mRenderer->setClassAttribute( field );
  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::categoriesDoubleClicked( const QModelIndex &idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeCategorySymbol();
}

void QgsCategorizedSymbolRendererWidget::changeCategorySymbol()
{
  QgsRendererCategory category = mRenderer->categories().value( currentCategoryRow() );

  std::unique_ptr< QgsSymbol > symbol;

  if ( category.symbol() )
  {
    symbol.reset( category.symbol()->clone() );
  }
  else
  {
    symbol.reset( QgsSymbol::defaultSymbol( mLayer->geometryType() ) );
  }

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSymbolSelectorWidget *dlg = new QgsSymbolSelectorWidget( symbol.release(), mStyle, mLayer, panel );
    dlg->setContext( mContext );
    dlg->setPanelTitle( category.label() );
    connect( dlg, &QgsPanelWidget::widgetChanged, this, &QgsCategorizedSymbolRendererWidget::updateSymbolsFromWidget );
    connect( dlg, &QgsPanelWidget::panelAccepted, this, &QgsCategorizedSymbolRendererWidget::cleanUpSymbolSelector );
    openPanel( dlg );
  }
  else
  {
    QgsSymbolSelectorDialog dlg( symbol.get(), mStyle, mLayer, panel );
    dlg.setContext( mContext );
    if ( !dlg.exec() || !symbol )
    {
      return;
    }

    mCategorizedSymbol = std::move( symbol );
    applyChangeToSymbol();
  }
}


void QgsCategorizedSymbolRendererWidget::addCategories()
{
  QString attrName = mExpressionWidget->currentField();
  int idx = mLayer->fields().lookupField( attrName );
  QList<QVariant> uniqueValues;
  if ( idx == -1 )
  {
    // Lets assume it's an expression
    QgsExpression *expression = new QgsExpression( attrName );
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
            << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
            << QgsExpressionContextUtils::atlasScope( nullptr )
            << QgsExpressionContextUtils::layerScope( mLayer );

    expression->prepare( &context );
    QgsFeatureIterator fit = mLayer->getFeatures();
    QgsFeature feature;
    while ( fit.nextFeature( feature ) )
    {
      context.setFeature( feature );
      QVariant value = expression->evaluate( &context );
      if ( uniqueValues.contains( value ) )
        continue;
      uniqueValues << value;
    }
  }
  else
  {
    uniqueValues = mLayer->uniqueValues( idx ).toList();
  }

  // ask to abort if too many classes
  if ( uniqueValues.size() >= 1000 )
  {
    int res = QMessageBox::warning( nullptr, tr( "Classify Categories" ),
                                    tr( "High number of classes. Classification would yield %1 entries which might not be expected. Continue?" ).arg( uniqueValues.size() ),
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

  QgsCategoryList cats = QgsCategorizedSymbolRenderer::createCategories( uniqueValues, mCategorizedSymbol.get(), mLayer, attrName );
  bool deleteExisting = false;

  if ( !mOldClassificationAttribute.isEmpty() &&
       attrName != mOldClassificationAttribute &&
       !mRenderer->categories().isEmpty() )
  {
    int res = QMessageBox::question( this,
                                     tr( "Delete Classification" ),
                                     tr( "The classification field was changed from '%1' to '%2'.\n"
                                         "Should the existing classes be deleted before classification?" )
                                     .arg( mOldClassificationAttribute, attrName ),
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
    keepExistingColors = !prevCats.isEmpty();
    for ( int i = 0; i < cats.size(); ++i )
    {
      bool contains = false;
      QVariant value = cats.at( i ).value();
      for ( int j = 0; j < prevCats.size() && !contains; ++j )
      {
        const QVariant prevCatValue = prevCats.at( j ).value();
        if ( prevCatValue.type() == QVariant::List )
        {
          const QVariantList list = prevCatValue.toList();
          for ( const QVariant &v : list )
          {
            if ( v == value )
            {
              contains = true;
              break;
            }
          }
        }
        else
        {
          if ( prevCats.at( j ).value() == value )
          {
            contains = true;
          }
        }
        if ( contains )
          break;
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
  std::unique_ptr< QgsCategorizedSymbolRenderer > r = qgis::make_unique< QgsCategorizedSymbolRenderer >( attrName, cats );
  r->setSourceSymbol( mCategorizedSymbol->clone() );
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( ramp )
    r->setSourceColorRamp( ramp->clone() );

  if ( mModel )
  {
    mModel->setRenderer( r.get() );
  }
  mRenderer = std::move( r );
  if ( ! keepExistingColors && ramp )
    applyColorRamp();
  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::applyColorRamp()
{
  if ( !btnColorRamp->isNull() )
  {
    mRenderer->updateColorRamp( btnColorRamp->colorRamp() );
  }
  mModel->updateSymbology();
}

int QgsCategorizedSymbolRendererWidget::currentCategoryRow()
{
  QModelIndex idx = viewCategories->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}

QList<int> QgsCategorizedSymbolRendererWidget::selectedCategories()
{
  QList<int> rows;
  QModelIndexList selectedRows = viewCategories->selectionModel()->selectedRows();

  Q_FOREACH ( const QModelIndex &r, selectedRows )
  {
    if ( r.isValid() )
    {
      rows.append( r.row() );
    }
  }
  return rows;
}

void QgsCategorizedSymbolRendererWidget::deleteCategories()
{
  QList<int> categoryIndexes = selectedCategories();
  mModel->deleteRows( categoryIndexes );
  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::deleteAllCategories()
{
  mModel->removeAllRows();
  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::addCategory()
{
  if ( !mModel ) return;
  QgsSymbol *symbol = QgsSymbol::defaultSymbol( mLayer->geometryType() );
  QgsRendererCategory cat( QString(), symbol, QString(), true );
  mModel->addCategory( cat );
  emit widgetChanged();
}

QList<QgsSymbol *> QgsCategorizedSymbolRendererWidget::selectedSymbols()
{
  QList<QgsSymbol *> selectedSymbols;

  QItemSelectionModel *m = viewCategories->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );

  if ( m && !selectedIndexes.isEmpty() )
  {
    const QgsCategoryList &categories = mRenderer->categories();
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      int row = ( *indexIt ).row();
      QgsSymbol *s = categories[row].symbol();
      if ( s )
      {
        selectedSymbols.append( s );
      }
    }
  }
  return selectedSymbols;
}

QgsCategoryList QgsCategorizedSymbolRendererWidget::selectedCategoryList()
{
  QgsCategoryList cl;

  QItemSelectionModel *m = viewCategories->selectionModel();
  QModelIndexList selectedIndexes = m->selectedRows( 1 );

  if ( m && !selectedIndexes.isEmpty() )
  {
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      cl.append( mModel->category( *indexIt ) );
    }
  }
  return cl;
}

void QgsCategorizedSymbolRendererWidget::refreshSymbolView()
{
  populateCategories();
  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer.get() );
}

void QgsCategorizedSymbolRendererWidget::rowsMoved()
{
  viewCategories->selectionModel()->clear();
}

void QgsCategorizedSymbolRendererWidget::matchToSymbolsFromLibrary()
{
  int matched = matchToSymbols( QgsStyle::defaultStyle() );
  if ( matched > 0 )
  {
    QMessageBox::information( this, tr( "Matched Symbols" ),
                              tr( "Matched %1 categories to symbols." ).arg( matched ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Matched Symbols" ),
                          tr( "No categories could be matched to symbols in library." ) );
  }
}

int QgsCategorizedSymbolRendererWidget::matchToSymbols( QgsStyle *style )
{
  if ( !mLayer || !style )
    return 0;

  const QgsSymbol::SymbolType type = mLayer->geometryType() == QgsWkbTypes::PointGeometry ? QgsSymbol::Marker
                                     : mLayer->geometryType() == QgsWkbTypes::LineGeometry ? QgsSymbol::Line
                                     : QgsSymbol::Fill;

  QVariantList unmatchedCategories;
  QStringList unmatchedSymbols;
  const int matched = mRenderer->matchToSymbols( style, type, unmatchedCategories, unmatchedSymbols );

  mModel->updateSymbology();
  return matched;
}

void QgsCategorizedSymbolRendererWidget::matchToSymbolsFromXml()
{
  QgsSettings settings;
  QString openFileDir = settings.value( QStringLiteral( "UI/lastMatchToSymbolsDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName( this, tr( "Match to Symbols from File" ), openFileDir,
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QFileInfo openFileInfo( fileName );
  settings.setValue( QStringLiteral( "UI/lastMatchToSymbolsDir" ), openFileInfo.absolutePath() );

  QgsStyle importedStyle;
  if ( !importedStyle.importXml( fileName ) )
  {
    QMessageBox::warning( this, tr( "Match to Symbols from File" ),
                          tr( "An error occurred while reading file:\n%1" ).arg( importedStyle.errorString() ) );
    return;
  }

  int matched = matchToSymbols( &importedStyle );
  if ( matched > 0 )
  {
    QMessageBox::information( this, tr( "Match to Symbols from File" ),
                              tr( "Matched %1 categories to symbols from file." ).arg( matched ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Match to Symbols from File" ),
                          tr( "No categories could be matched to symbols in file." ) );
  }
}

void QgsCategorizedSymbolRendererWidget::cleanUpSymbolSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *dlg = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !dlg )
    return;

  delete dlg->symbol();
}

void QgsCategorizedSymbolRendererWidget::updateSymbolsFromWidget()
{
  QgsSymbolSelectorWidget *dlg = qobject_cast<QgsSymbolSelectorWidget *>( sender() );
  mCategorizedSymbol.reset( dlg->symbol()->clone() );

  applyChangeToSymbol();
}

void QgsCategorizedSymbolRendererWidget::applyChangeToSymbol()
{
  // When there is a selection, change the selected symbols only
  QItemSelectionModel *m = viewCategories->selectionModel();
  QModelIndexList i = m->selectedRows();

  if ( m && !i.isEmpty() )
  {
    QList<int> selectedCats = selectedCategories();

    if ( !selectedCats.isEmpty() )
    {
      Q_FOREACH ( int idx, selectedCats )
      {
        QgsSymbol *newCatSymbol = mCategorizedSymbol->clone();
        if ( selectedCats.count() > 1 )
        {
          //if updating multiple categories, retain the existing category colors
          newCatSymbol->setColor( mRenderer->categories().at( idx ).symbol()->color() );
        }
        mRenderer->updateCategorySymbol( idx, newCatSymbol );
      }
      emit widgetChanged();
    }
  }
  else
  {
    mRenderer->updateSymbols( mCategorizedSymbol.get() );
  }

  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::keyPressEvent( QKeyEvent *event )
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

QgsExpressionContext QgsCategorizedSymbolRendererWidget::createExpressionContext() const
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

void QgsCategorizedSymbolRendererWidget::dataDefinedSizeLegend()
{
  QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( mCategorizedSymbol.get() ); // this should be only enabled for marker symbols
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

void QgsCategorizedSymbolRendererWidget::mergeSelectedCategories()
{
  const QgsCategoryList &categories = mRenderer->categories();

  QList<int> selectedCategoryIndexes = selectedCategories();
  QList< int > categoryIndexes;

  // filter out "" entry
  for ( int i : selectedCategoryIndexes )
  {
    QVariant v = categories.at( i ).value();

    if ( !v.isValid() || v == "" )
    {
      continue;
    }

    categoryIndexes.append( i );
  }

  if ( categoryIndexes.count() < 2 )
    return;

  QStringList labels;
  QVariantList values;
  values.reserve( categoryIndexes.count() );
  labels.reserve( categoryIndexes.count() );
  for ( int i : categoryIndexes )
  {
    QVariant v = categories.at( i ).value();

    if ( v.type() == QVariant::List )
    {
      values.append( v.toList() );
    }
    else
      values << v;

    labels << categories.at( i ).label();
  }

  // modify first category (basically we "merge up" into the first selected category)
  mRenderer->updateCategoryLabel( categoryIndexes.at( 0 ), labels.join( ',' ) );
  mRenderer->updateCategoryValue( categoryIndexes.at( 0 ), values );

  categoryIndexes.pop_front();
  mModel->deleteRows( categoryIndexes );

  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::unmergeSelectedCategories()
{
  const QList<int> categoryIndexes = selectedCategories();
  if ( categoryIndexes.isEmpty() )
    return;

  const QgsCategoryList &categories = mRenderer->categories();
  for ( int i : categoryIndexes )
  {
    const QVariant v = categories.at( i ).value();
    if ( v.type() != QVariant::List )
      continue;

    const QVariantList list = v.toList();
    for ( int j = 1; j < list.count(); ++j )
    {
      mModel->addCategory( QgsRendererCategory( list.at( j ), categories.at( i ).symbol()->clone(), list.at( j ).toString(), categories.at( i ).renderState() ) );
    }
    mRenderer->updateCategoryValue( i, list.at( 0 ) );
    mRenderer->updateCategoryLabel( i, list.at( 0 ).toString() );
  }

  emit widgetChanged();
}

void QgsCategorizedSymbolRendererWidget::showContextMenu( QPoint )
{
  mContextMenu->clear();
  const QList< QAction * > actions = contextMenu->actions();
  for ( QAction *act : actions )
  {
    mContextMenu->addAction( act );
  }

  mContextMenu->addSeparator();

  if ( viewCategories->selectionModel()->selectedRows().count() > 1 )
  {
    mContextMenu->addAction( mMergeCategoriesAction );
  }
  if ( viewCategories->selectionModel()->selectedRows().count() == 1 )
  {
    const QList<int> categoryIndexes = selectedCategories();
    const QgsCategoryList &categories = mRenderer->categories();
    const QVariant v = categories.at( categoryIndexes.at( 0 ) ).value();
    if ( v.type() == QVariant::List )
      mContextMenu->addAction( mUnmergeCategoriesAction );
  }
  else if ( viewCategories->selectionModel()->selectedRows().count() > 1 )
  {
    mContextMenu->addAction( mUnmergeCategoriesAction );
  }

  mContextMenu->exec( QCursor::pos() );
}
