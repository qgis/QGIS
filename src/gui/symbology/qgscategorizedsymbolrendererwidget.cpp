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
#include "qgscolorrampimpl.h"
#include "qgscolorrampbutton.h"
#include "qgsstyle.h"
#include "qgslogger.h"
#include "qgsexpressioncontextutils.h"
#include "qgstemporalcontroller.h"

#include "qgssymbolselectordialog.h"
#include "qgsexpressionbuilderdialog.h"

#include "qgsvectorlayer.h"
#include "qgsfeatureiterator.h"

#include "qgsproject.h"
#include "qgsexpression.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsguiutils.h"
#include "qgsmarkersymbol.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPen>
#include <QPainter>
#include <QFileDialog>
#include <QClipboard>

///@cond PRIVATE

QgsCategorizedSymbolRendererModel::QgsCategorizedSymbolRendererModel( QObject *parent ) : QAbstractItemModel( parent )
  , mMimeFormat( QStringLiteral( "application/x-qgscategorizedsymbolrendererv2model" ) )
{
}

void QgsCategorizedSymbolRendererModel::setRenderer( QgsCategorizedSymbolRenderer *renderer )
{
  if ( mRenderer )
  {
    beginRemoveRows( QModelIndex(), 0, std::max< int >( mRenderer->categories().size() - 1, 0 ) );
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
  const int idx = mRenderer->categories().size();
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
  const int row = index.row();
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
              res << QgsCategorizedSymbolRenderer::displayString( v );

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
            return QgsCategorizedSymbolRenderer::displayString( category.value() );
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
        const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
        return QgsSymbolLayerUtils::symbolPreviewIcon( category.symbol(), QSize( iconSize, iconSize ) );
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
      return ( index.column() == 0 ) ? static_cast<Qt::Alignment::Int>( Qt::AlignHCenter ) : static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
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
    case QgsCategorizedSymbolRendererWidget::CustomRoles::ValueRole:
    {
      if ( index.column() == 1 )
        return category.value();
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
      // try to preserve variant type for this value, unless it was an empty string (other values)
      QVariant val = value;
      const QVariant previousValue = mRenderer->categories().value( index.row() ).value();
      if ( previousValue.type() != QVariant::String && ! previousValue.toString().isEmpty() )
      {
        switch ( previousValue.type() )
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
  Q_UNUSED( index )
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
  Q_UNUSED( index )
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

bool QgsCategorizedSymbolRendererModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
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


QgsCategorizedRendererViewItemDelegate::QgsCategorizedRendererViewItemDelegate( QgsFieldExpressionWidget *expressionWidget, QObject *parent )
  : QStyledItemDelegate( parent )
  , mFieldExpressionWidget( expressionWidget )
{
}

QWidget *QgsCategorizedRendererViewItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QVariant::Type userType { index.data( QgsCategorizedSymbolRendererWidget::CustomRoles::ValueRole ).type() };

  // In case of new values the type is not known
  if ( userType == QVariant::String && index.data( QgsCategorizedSymbolRendererWidget::CustomRoles::ValueRole ).isNull() )
  {
    bool isExpression;
    bool isValid;
    const QString fieldName { mFieldExpressionWidget->currentField( &isExpression, &isValid ) };
    if ( ! fieldName.isEmpty() && mFieldExpressionWidget->layer() && mFieldExpressionWidget->layer()->fields().lookupField( fieldName ) != -1 )
    {
      userType = mFieldExpressionWidget->layer()->fields().field( fieldName ).type();
    }
    else if ( isExpression && isValid )
    {
      // Try to guess the type from the expression return value
      QgsFeature feat;
      if ( mFieldExpressionWidget->layer()->getFeatures().nextFeature( feat ) )
      {
        QgsExpressionContext expressionContext;
        expressionContext.appendScope( QgsExpressionContextUtils::globalScope() );
        expressionContext.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
        expressionContext.appendScope( mFieldExpressionWidget->layer()->createExpressionContextScope() );
        expressionContext.setFeature( feat );
        QgsExpression exp { mFieldExpressionWidget->expression() };
        const QVariant value = exp.evaluate( &expressionContext );
        if ( !exp.hasEvalError() )
        {
          userType = value.type();
        }
      }
    }
  }

  QgsDoubleSpinBox *editor = nullptr;
  switch ( userType )
  {
    case QVariant::Type::Double:
    {
      editor = new QgsDoubleSpinBox( parent );
      bool ok;
      const QVariant value = index.data( QgsCategorizedSymbolRendererWidget::CustomRoles::ValueRole );
      int decimals {2};
      if ( value.toDouble( &ok ); ok )
      {
        const QString strVal { value.toString() };
        const int dotPosition( strVal.indexOf( '.' ) );
        if ( dotPosition >= 0 )
        {
          decimals = std::max<int>( 2, strVal.length() - dotPosition - 1 );
        }
      }
      editor->setDecimals( decimals );
      editor->setClearValue( 0 );
      editor->setMaximum( std::numeric_limits<double>::max() );
      editor->setMinimum( std::numeric_limits<double>::lowest() );
      break;
    }
    case QVariant::Type::Int:
    {
      editor = new QgsDoubleSpinBox( parent );
      editor->setDecimals( 0 );
      editor->setClearValue( 0 );
      editor->setMaximum( std::numeric_limits<int>::max() );
      editor->setMinimum( std::numeric_limits<int>::min() );
      break;
    }
    case QVariant::Type::Char:
    {
      editor = new QgsDoubleSpinBox( parent );
      editor->setDecimals( 0 );
      editor->setClearValue( 0 );
      editor->setMaximum( std::numeric_limits<char>::max() );
      editor->setMinimum( std::numeric_limits<char>::min() );
      break;
    }
    case QVariant::Type::UInt:
    {
      editor = new QgsDoubleSpinBox( parent );
      editor->setDecimals( 0 );
      editor->setClearValue( 0 );
      editor->setMaximum( std::numeric_limits<unsigned int>::max() );
      editor->setMinimum( 0 );
      break;
    }
    case QVariant::Type::LongLong:
    {
      editor = new QgsDoubleSpinBox( parent );
      editor->setDecimals( 0 );
      editor->setClearValue( 0 );
      editor->setMaximum( static_cast<double>( std::numeric_limits<qlonglong>::max() ) );
      editor->setMinimum( std::numeric_limits<qlonglong>::min() );
      break;
    }
    case QVariant::Type::ULongLong:
    {
      editor = new QgsDoubleSpinBox( parent );
      editor->setDecimals( 0 );
      editor->setClearValue( 0 );
      editor->setMaximum( static_cast<double>( std::numeric_limits<unsigned long long>::max() ) );
      editor->setMinimum( 0 );
      break;
    }
    default:
      break;
  }
  return editor ? editor : QStyledItemDelegate::createEditor( parent, option, index );
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
    mRenderer.reset( QgsCategorizedSymbolRenderer::convertFromRenderer( renderer, layer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = std::make_unique< QgsCategorizedSymbolRenderer >( QString(), QgsCategoryList() );
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  const QString attrName = mRenderer->classAttribute();
  mOldClassificationAttribute = attrName;

  // setup user interface
  setupUi( this );
  layout()->setContentsMargins( 0, 0, 0, 0 );

  mExpressionWidget->setLayer( mLayer );
  btnChangeCategorizedSymbol->setLayer( mLayer );
  btnChangeCategorizedSymbol->registerExpressionContextGenerator( this );

  // initiate color ramp button to random
  btnColorRamp->setShowRandomColorRamp( true );

  // set project default color ramp
  const QString defaultColorRamp = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), QString() );
  if ( !defaultColorRamp.isEmpty() )
  {
    btnColorRamp->setColorRampFromName( defaultColorRamp );
  }
  else
  {
    btnColorRamp->setRandomColorRamp();
  }

  mCategorizedSymbol.reset( QgsSymbol::defaultSymbol( mLayer->geometryType() ) );
  if ( mCategorizedSymbol )
  {
    btnChangeCategorizedSymbol->setSymbolType( mCategorizedSymbol->type() );
    btnChangeCategorizedSymbol->setSymbol( mCategorizedSymbol->clone() );
  }

  mModel = new QgsCategorizedSymbolRendererModel( this );
  mModel->setRenderer( mRenderer.get() );

  // update GUI from renderer
  updateUiFromRenderer();

  viewCategories->setModel( mModel );
  viewCategories->resizeColumnToContents( 0 );
  viewCategories->resizeColumnToContents( 1 );
  viewCategories->resizeColumnToContents( 2 );
  viewCategories->setItemDelegateForColumn( 1, new QgsCategorizedRendererViewItemDelegate( mExpressionWidget, viewCategories ) );

  viewCategories->setStyle( new QgsCategorizedSymbolRendererViewStyle( viewCategories ) );
  connect( viewCategories->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsCategorizedSymbolRendererWidget::selectionChanged );

  connect( mModel, &QgsCategorizedSymbolRendererModel::rowsMoved, this, &QgsCategorizedSymbolRendererWidget::rowsMoved );
  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsPanelWidget::widgetChanged );

  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsCategorizedSymbolRendererWidget::categoryColumnChanged );

  connect( viewCategories, &QAbstractItemView::doubleClicked, this, &QgsCategorizedSymbolRendererWidget::categoriesDoubleClicked );
  connect( viewCategories, &QTreeView::customContextMenuRequested, this, &QgsCategorizedSymbolRendererWidget::showContextMenu );

  connect( btnChangeCategorizedSymbol, &QgsSymbolButton::changed, this, &QgsCategorizedSymbolRendererWidget::updateSymbolsFromButton );

  connect( btnAddCategories, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::addCategories );
  connect( btnDeleteCategories, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::deleteCategories );
  connect( btnDeleteAllCategories, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::deleteAllCategories );
  connect( btnAddCategory, &QAbstractButton::clicked, this, &QgsCategorizedSymbolRendererWidget::addCategory );

  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsCategorizedSymbolRendererWidget::applyColorRamp );

  // menus for data-defined rotation/size
  QMenu *advMenu = new QMenu;

  advMenu->addAction( tr( "Match to Saved Symbols" ), this, &QgsCategorizedSymbolRendererWidget::matchToSymbolsFromLibrary );
  advMenu->addAction( tr( "Match to Symbols from File…" ), this, &QgsCategorizedSymbolRendererWidget::matchToSymbolsFromXml );
  mActionLevels = advMenu->addAction( tr( "Symbol Levels…" ), this, &QgsCategorizedSymbolRendererWidget::showSymbolLevels );
  if ( mCategorizedSymbol && mCategorizedSymbol->type() == Qgis::SymbolType::Marker )
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

  connect( mContextMenu, &QMenu::aboutToShow, this, [ = ]
  {
    const std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
    mPasteSymbolAction->setEnabled( static_cast< bool >( tempSymbol ) );
  } );
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

  //mModel->setRenderer ( mRenderer ); // necessary?

  // set column
  const QString attrName = mRenderer->classAttribute();
  mExpressionWidget->setField( attrName );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    mCategorizedSymbol.reset( mRenderer->sourceSymbol()->clone() );
    whileBlocking( btnChangeCategorizedSymbol )->setSymbol( mCategorizedSymbol->clone() );
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

void QgsCategorizedSymbolRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  btnChangeCategorizedSymbol->setMapCanvas( context.mapCanvas() );
  btnChangeCategorizedSymbol->setMessageBar( context.messageBar() );
}

void QgsCategorizedSymbolRendererWidget::disableSymbolLevels()
{
  delete mActionLevels;
  mActionLevels = nullptr;
}

void QgsCategorizedSymbolRendererWidget::changeSelectedSymbols()
{
  const QList<int> selectedCats = selectedCategories();

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

    const auto constSelectedCats = selectedCats;
    for ( const int idx : constSelectedCats )
    {
      const QgsRendererCategory category = mRenderer->categories().value( idx );

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
    applyChangeToSymbol();
  }
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
  const QgsRendererCategory category = mRenderer->categories().value( currentCategoryRow() );

  std::unique_ptr< QgsSymbol > symbol;

  if ( auto *lSymbol = category.symbol() )
  {
    symbol.reset( lSymbol->clone() );
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
  const QString attrName = mExpressionWidget->currentField();
  const int idx = mLayer->fields().lookupField( attrName );
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
      const QVariant value = expression->evaluate( &context );
      if ( uniqueValues.contains( value ) )
        continue;
      uniqueValues << value;
    }
  }
  else
  {
    uniqueValues = qgis::setToList( mLayer->uniqueValues( idx ) );
  }

  // ask to abort if too many classes
  if ( uniqueValues.size() >= 1000 )
  {
    const int res = QMessageBox::warning( nullptr, tr( "Classify Categories" ),
                                          tr( "High number of classes. Classification would yield %n entries which might not be expected. Continue?", nullptr, uniqueValues.size() ),
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
    const int res = QMessageBox::question( this,
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
    QgsRandomColorRamp randomColors;
    if ( keepExistingColors && btnColorRamp->isRandomColorRamp() )
      randomColors.setTotalColorCount( cats.size() );
    for ( int i = 0; i < cats.size(); ++i )
    {
      bool contains = false;
      const QVariant value = cats.at( i ).value();
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
      {
        if ( keepExistingColors && btnColorRamp->isRandomColorRamp() )
        {
          // insure that append symbols have random colors
          cats.at( i ).symbol()->setColor( randomColors.color( i ) );
        }
        prevCats.append( cats.at( i ) );
      }
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
  std::unique_ptr< QgsCategorizedSymbolRenderer > r = std::make_unique< QgsCategorizedSymbolRenderer >( attrName, cats );
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
  const QModelIndex idx = viewCategories->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}

QList<int> QgsCategorizedSymbolRendererWidget::selectedCategories()
{
  QList<int> rows;
  const QModelIndexList selectedRows = viewCategories->selectionModel()->selectedRows();

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

void QgsCategorizedSymbolRendererWidget::deleteCategories()
{
  const QList<int> categoryIndexes = selectedCategories();
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
  const QgsRendererCategory cat( QString(), symbol, QString(), true );
  mModel->addCategory( cat );
  emit widgetChanged();
}

QList<QgsSymbol *> QgsCategorizedSymbolRendererWidget::selectedSymbols()
{
  QList<QgsSymbol *> selectedSymbols;

  QItemSelectionModel *m = viewCategories->selectionModel();
  const QModelIndexList selectedIndexes = m->selectedRows( 1 );

  if ( !selectedIndexes.isEmpty() )
  {
    const QgsCategoryList &categories = mRenderer->categories();
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      const int row = ( *indexIt ).row();
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
  const QModelIndexList selectedIndexes = m->selectedRows( 1 );

  if ( !selectedIndexes.isEmpty() )
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
  const int matched = matchToSymbols( QgsStyle::defaultStyle() );
  if ( matched > 0 )
  {
    QMessageBox::information( this, tr( "Matched Symbols" ),
                              tr( "Matched %n categories to symbols.", nullptr, matched ) );
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

  const Qgis::SymbolType type = mLayer->geometryType() == QgsWkbTypes::PointGeometry ? Qgis::SymbolType::Marker
                                : mLayer->geometryType() == QgsWkbTypes::LineGeometry ? Qgis::SymbolType::Line
                                : Qgis::SymbolType::Fill;

  QVariantList unmatchedCategories;
  QStringList unmatchedSymbols;
  const int matched = mRenderer->matchToSymbols( style, type, unmatchedCategories, unmatchedSymbols );

  mModel->updateSymbology();
  return matched;
}

void QgsCategorizedSymbolRendererWidget::matchToSymbolsFromXml()
{
  QgsSettings settings;
  const QString openFileDir = settings.value( QStringLiteral( "UI/lastMatchToSymbolsDir" ), QDir::homePath() ).toString();

  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Match to Symbols from File" ), openFileDir,
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  const QFileInfo openFileInfo( fileName );
  settings.setValue( QStringLiteral( "UI/lastMatchToSymbolsDir" ), openFileInfo.absolutePath() );

  QgsStyle importedStyle;
  if ( !importedStyle.importXml( fileName ) )
  {
    QMessageBox::warning( this, tr( "Match to Symbols from File" ),
                          tr( "An error occurred while reading file:\n%1" ).arg( importedStyle.errorString() ) );
    return;
  }

  const int matched = matchToSymbols( &importedStyle );
  if ( matched > 0 )
  {
    QMessageBox::information( this, tr( "Match to Symbols from File" ),
                              tr( "Matched %n categories to symbols from file.", nullptr, matched ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Match to Symbols from File" ),
                          tr( "No categories could be matched to symbols in file." ) );
  }
}

void QgsCategorizedSymbolRendererWidget::setSymbolLevels( const QgsLegendSymbolList &levels, bool enabled )
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

void QgsCategorizedSymbolRendererWidget::pasteSymbolToSelection()
{
  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( !tempSymbol )
    return;

  const QList<int> selectedCats = selectedCategories();
  if ( !selectedCats.isEmpty() )
  {
    for ( const int idx : selectedCats )
    {
      if ( mRenderer->categories().at( idx ).symbol()->type() != tempSymbol->type() )
        continue;

      std::unique_ptr< QgsSymbol > newCatSymbol( tempSymbol->clone() );
      if ( selectedCats.count() > 1 )
      {
        //if updating multiple categories, retain the existing category colors
        newCatSymbol->setColor( mRenderer->categories().at( idx ).symbol()->color() );
      }
      mRenderer->updateCategorySymbol( idx, newCatSymbol.release() );
    }
    emit widgetChanged();
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

void QgsCategorizedSymbolRendererWidget::updateSymbolsFromButton()
{
  mCategorizedSymbol.reset( btnChangeCategorizedSymbol->symbol()->clone() );

  applyChangeToSymbol();
}

void QgsCategorizedSymbolRendererWidget::applyChangeToSymbol()
{
  // When there is a selection, change the selected symbols only
  QItemSelectionModel *m = viewCategories->selectionModel();
  const QModelIndexList i = m->selectedRows();

  if ( !i.isEmpty() )
  {
    const QList<int> selectedCats = selectedCategories();

    if ( !selectedCats.isEmpty() )
    {
      const auto constSelectedCats = selectedCats;
      for ( const int idx : constSelectedCats )
      {
        QgsSymbol *newCatSymbol = mCategorizedSymbol->clone();
        if ( selectedCats.count() > 1 )
        {
          //if updating multiple categories, retain the existing category colors
          newCatSymbol->setColor( mRenderer->categories().at( idx ).symbol()->color() );
        }
        mRenderer->updateCategorySymbol( idx, newCatSymbol );
      }
    }
  }
  else
  {
    mRenderer->updateSymbols( mCategorizedSymbol.get() );
  }

  mModel->updateSymbology();
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

  const QList<int> selectedCategoryIndexes = selectedCategories();
  QList< int > categoryIndexes;

  // filter out "" entry
  for ( const int i : selectedCategoryIndexes )
  {
    const QVariant v = categories.at( i ).value();

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
  for ( const int i : categoryIndexes )
  {
    const QVariant v = categories.at( i ).value();

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
  for ( const int i : categoryIndexes )
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

void QgsCategorizedSymbolRendererWidget::selectionChanged( const QItemSelection &, const QItemSelection & )
{
  const QList<int> selectedCats = selectedCategories();
  if ( !selectedCats.isEmpty() )
  {
    whileBlocking( btnChangeCategorizedSymbol )->setSymbol( mRenderer->categories().at( selectedCats.at( 0 ) ).symbol()->clone() );
  }
  else if ( mRenderer->sourceSymbol() )
  {
    whileBlocking( btnChangeCategorizedSymbol )->setSymbol( mRenderer->sourceSymbol()->clone() );
  }
  btnChangeCategorizedSymbol->setDialogTitle( selectedCats.size() == 1 ? mRenderer->categories().at( selectedCats.at( 0 ) ).label() : tr( "Symbol Settings" ) );
}
