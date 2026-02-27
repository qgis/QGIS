/***************************************************************************
    qgscategorized3drendererwidget.cpp
    ---------------------
    begin                : November 2025
    copyright            : (C) 2025 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscategorized3drendererwidget.h"

#include "qgs3dsymbolregistry.h"
#include "qgs3dsymbolutils.h"
#include "qgsabstract3dsymbol.h"
#include "qgsapplication.h"
#include "qgscategorized3drenderer.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgscategorizedsymbolrendererwidget.h"
#include "qgscolorrampbutton.h"
#include "qgscolorrampimpl.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"
#include "qgsguiutils.h"
#include "qgsmapcanvas.h"
#include "qgspanelwidget.h"
#include "qgsproject.h"
#include "qgsprojectstylesettings.h"
#include "qgsstyle.h"
#include "qgssymbol3dwidget.h"
#include "qgssymbollayerutils.h"
#include "qgssymbolselectordialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drendererwidget.h"
#include "qgsvectorlayerutils.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>

#include "moc_qgscategorized3drendererwidget.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsCategorized3DRendererModel::QgsCategorized3DRendererModel( QObject *parent, QScreen *screen )
  : QgsTemplatedCategorizedRendererModel<QgsCategorized3DRenderer>( parent, screen )
{
}

Qt::ItemFlags QgsCategorized3DRendererModel::flags( const QModelIndex &index ) const
{
  // Flat list, to ease drop handling valid indexes are not dropEnabled
  if ( !index.isValid() || !mRenderer )
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable;
  if ( index.column() == 1 )
  {
    const Qgs3DRendererCategory category = mRenderer->categories().value( index.row() );
    if ( category.value().userType() != QMetaType::Type::QVariantList )
    {
      flags |= Qt::ItemIsEditable;
    }
  }

  return flags;
}

QVariant QgsCategorized3DRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mRenderer )
  {
    return QVariant();
  }

  const Qgs3DRendererCategory category = mRenderer->categories().value( index.row() );

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
          if ( category.value().userType() == QMetaType::Type::QVariantList )
          {
            QStringList res;
            const QVariantList list = category.value().toList();
            res.reserve( list.size() );
            for ( const QVariant &v : list )
              res << QgsVariantUtils::displayString( v );

            if ( role == Qt::DisplayRole )
              return res.join( ';' );
            else // tooltip
              return res.join( '\n' );
          }
          else if ( QgsVariantUtils::isNull( category.value() ) || category.value().toString().isEmpty() )
          {
            return tr( "all other values" );
          }
          else
          {
            return QgsVariantUtils::displayString( category.value() );
          }
        }
        default:
          break;
      }
      break;
    }

    case Qt::FontRole:
    {
      if ( index.column() == 1 && category.value().userType() != QMetaType::Type::QVariantList && ( QgsVariantUtils::isNull( category.value() ) || category.value().toString().isEmpty() ) )
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
        return Qgs3DSymbolUtils::vectorSymbolPreviewIcon( category.symbol(), QSize( iconSize, iconSize ), QgsScreenProperties( mScreen.data() ), 0 );
      }
      break;
    }

    case Qt::ForegroundRole:
    {
      QBrush brush( qApp->palette().color( QPalette::Text ), Qt::SolidPattern );
      if ( index.column() == 1 && ( category.value().userType() == QMetaType::Type::QVariantList || QgsVariantUtils::isNull( category.value() ) || category.value().toString().isEmpty() ) )
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
          if ( category.value().userType() == QMetaType::Type::QVariantList )
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
        default:
          break;
      }
      break;
    }
    case static_cast<int>( QgsCategorizedSymbolRendererWidget::CustomRole::Value ):
    {
      if ( index.column() == 1 )
        return category.value();
      break;
    }
    default:
      break;
  }

  return QVariant();
}

bool QgsCategorized3DRendererModel::setData( const QModelIndex &index, const QVariant &value, int role )
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
      if ( previousValue.userType() != QMetaType::Type::QString && !previousValue.toString().isEmpty() )
      {
        switch ( previousValue.userType() )
        {
          case QMetaType::Type::Int:
            val = value.toInt();
            break;
          case QMetaType::Type::Double:
            val = value.toDouble();
            break;
          case QMetaType::Type::QVariantList:
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
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

QVariant QgsCategorized3DRendererModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 2 )
  {
    QStringList lst;
    lst << tr( "Symbol" ) << tr( "Value" );
    return lst.value( section );
  }
  return QVariant();
}

int QgsCategorized3DRendererModel::columnCount( const QModelIndex &index ) const
{
  Q_UNUSED( index )
  return 2;
}

void QgsCategorized3DRendererModel::sort( int column, Qt::SortOrder order )
{
  if ( column == 0 )
  {
    return;
  }
  if ( column == 1 )
  {
    mRenderer->sortByValue( order );
  }

  emit dataChanged( createIndex( 0, 0 ), createIndex( static_cast<int>( mRenderer->categories().size() ), 0 ) );
}

void QgsCategorized3DRendererModel::onRowsMoved()
{
  emit rowsMoved();
}

///@endcond

// ------------------------------ Widget ------------------------------------

QgsCategorized3DRendererWidget::QgsCategorized3DRendererWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  layout()->setContentsMargins( 0, 0, 0, 0 );

  mRenderer = std::make_unique<QgsCategorized3DRenderer>();

  // initiate color ramp button to random
  mBtnColorRamp->setShowRandomColorRamp( true );

  // set project default color ramp
  std::unique_ptr<QgsColorRamp> colorRamp( QgsProject::instance()->styleSettings()->defaultColorRamp() );
  if ( colorRamp )
  {
    mBtnColorRamp->setColorRamp( colorRamp.get() );
  }
  else
  {
    mBtnColorRamp->setRandomColorRamp();
  }

  mModel = new QgsCategorized3DRendererModel( this, screen() );
  mModel->setRenderer( mRenderer.get() );

  // update GUI from renderer
  updateUiFromRenderer();

  mViewCategories->setModel( mModel );
  mViewCategories->resizeColumnToContents( 0 );
  mViewCategories->resizeColumnToContents( 1 );
  mViewCategories->setItemDelegateForColumn( 1, new QgsCategorizedRendererViewItemDelegate( mExpressionWidget, mViewCategories ) );

  mViewCategories->setStyle( new QgsCategorizedSymbolRendererViewStyle( mViewCategories ) );
  connect( mViewCategories->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsCategorized3DRendererWidget::selectionChanged );

  connect( mModel, &QgsCategorized3DRendererModel::rowsMoved, this, &QgsCategorized3DRendererWidget::rowsMoved );
  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsPanelWidget::widgetChanged );

  connect( mExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsCategorized3DRendererWidget::categoryColumnChanged );

  connect( mViewCategories, &QAbstractItemView::doubleClicked, this, &QgsCategorized3DRendererWidget::categoriesDoubleClicked );

  mBtnChangeCategorizedSymbol->setDialogTitle( tr( "Symbol Settings" ) );
  connect( mBtnChangeCategorizedSymbol, &Qgs3DSymbolButton::changed, this, &QgsCategorized3DRendererWidget::updateSymbolsFromButton );

  connect( mBtnAddCategories, &QAbstractButton::clicked, this, &QgsCategorized3DRendererWidget::addCategories );
  connect( mBtnDeleteCategories, &QAbstractButton::clicked, this, &QgsCategorized3DRendererWidget::deleteCategories );
  connect( mBtnDeleteAllCategories, &QAbstractButton::clicked, this, &QgsCategorized3DRendererWidget::deleteAllCategories );
  connect( mBtnDeleteUnusedCategories, &QAbstractButton::clicked, this, &QgsCategorized3DRendererWidget::deleteUnusedCategories );
  connect( mBtnAddCategory, &QAbstractButton::clicked, this, &QgsCategorized3DRendererWidget::addCategory );

  connect( mBtnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsCategorized3DRendererWidget::applyColorRamp );
}

QgsCategorized3DRendererWidget::~QgsCategorized3DRendererWidget()
{
  delete mModel;
}

void QgsCategorized3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  mExpressionWidget->setLayer( mLayer );
  mBtnChangeCategorizedSymbol->setLayer( mLayer );

  QgsAbstract3DRenderer *renderer = layer->renderer3D();
  if ( renderer && renderer->type() == "categorized"_L1 )
  {
    // if the layer has a categorized renderer
    mRenderer.reset( dynamic_cast<QgsCategorized3DRenderer *>( renderer )->clone() );
    mCategorizedSymbol.reset( mRenderer->sourceSymbol()->clone() );
    mModel->setRenderer( mRenderer.get() );
  }
  else
  {
    // Create a default renderer
    mRenderer.reset( new QgsCategorized3DRenderer() );
    QgsAbstract3DSymbol *symbol = QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( mLayer->geometryType() );
    symbol->setDefaultPropertiesFromLayer( mLayer );
    mCategorizedSymbol.reset( symbol );
    mRenderer->setSourceSymbol( mCategorizedSymbol->clone() );
    mModel->setRenderer( mRenderer.get() );
  }

  // update GUI from renderer
  updateUiFromRenderer();
}

void QgsCategorized3DRendererWidget::updateUiFromRenderer()
{
  // Note: This assumes that the signals for UI element changes have not
  // yet been connected, so that the updates to color ramp, symbol, etc
  // don't override existing customizations.

  // set column
  const QString attributeName = mRenderer->classAttribute();
  whileBlocking( mExpressionWidget )->setField( attributeName );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    mCategorizedSymbol.reset( mRenderer->sourceSymbol()->clone() );
    if ( mCategorizedSymbol )
    {
      mBtnChangeCategorizedSymbol->setSymbol( std::unique_ptr<QgsAbstract3DSymbol>( mCategorizedSymbol->clone() ) );
      applyChangeToSymbol();
    }
  }

  // if a color ramp attached to the renderer, enable the color ramp button
  if ( mRenderer->sourceColorRamp() )
  {
    mBtnColorRamp->setColorRamp( mRenderer->sourceColorRamp() );
  }
}

void QgsCategorized3DRendererWidget::categoryColumnChanged( const QString &field )
{
  mRenderer->setClassAttribute( field );
  emit widgetChanged();
}

void QgsCategorized3DRendererWidget::categoriesDoubleClicked( const QModelIndex &idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeCategorySymbol();
}

void QgsCategorized3DRendererWidget::changeCategorySymbol()
{
  const Qgs3DRendererCategory category = mRenderer->categories().value( currentCategoryRow() );

  std::unique_ptr<QgsAbstract3DSymbol> symbol;
  if ( auto *lSymbol = category.symbol() )
  {
    symbol.reset( lSymbol->clone() );
  }
  else
  {
    QgsAbstract3DSymbol *defaultSymbol = QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( mLayer->geometryType() );
    defaultSymbol->setDefaultPropertiesFromLayer( mLayer );
    symbol.reset( defaultSymbol );
  }

  QgsSingleSymbol3DRendererWidget *widget = new QgsSingleSymbol3DRendererWidget( mLayer, this );
  widget->setSymbol( symbol.get() );
  widget->setPanelTitle( category.value().toString() );
  connect( widget, &QgsPanelWidget::widgetChanged, this, [this, widget] { updateSymbolsFromWidget( widget ); } );
  openPanel( widget );
}


void QgsCategorized3DRendererWidget::addCategories()
{
  const QString attributeName = mExpressionWidget->currentField();
  bool valuesRetrieved;
  const QList<QVariant> uniqueValues = QgsVectorLayerUtils::uniqueValues( mLayer, attributeName, valuesRetrieved );
  if ( !valuesRetrieved )
  {
    QgsDebugMsgLevel( u"Unable to retrieve values from layer %1 with expression %2"_s.arg( mLayer->name() ).arg( attributeName ), 2 );
    return;
  }

  // ask to abort if too many classes
  if ( uniqueValues.size() >= 1000 )
  {
    const int res = QMessageBox::warning( nullptr, tr( "Classify Categories" ), tr( "High number of classes. Classification would yield %n entries which might not be expected. Continue?", nullptr, static_cast<int>( uniqueValues.size() ) ), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel );
    if ( res == QMessageBox::Cancel )
    {
      return;
    }
  }

  Qgs3DCategoryList cats = QgsCategorized3DRenderer::createCategories( uniqueValues, mCategorizedSymbol.get(), mLayer, attributeName );
  bool deleteExisting = false;

  if ( !mOldClassificationAttribute.isEmpty() && attributeName != mOldClassificationAttribute && !mRenderer->categories().isEmpty() )
  {
    const int res = QMessageBox::question( this, tr( "Delete Classification" ), tr( "The classification field was changed from '%1' to '%2'.\n"
                                                                                    "Should the existing classes be deleted before classification?" )
                                                                                  .arg( mOldClassificationAttribute, attributeName ),
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
    Qgs3DCategoryList prevCats = mRenderer->categories();
    keepExistingColors = !prevCats.isEmpty();
    QgsRandomColorRamp randomColors;
    if ( keepExistingColors && mBtnColorRamp->isRandomColorRamp() )
    {
      randomColors.setTotalColorCount( static_cast<int>( cats.size() ) );
    }

    for ( int i = 0; i < cats.size(); ++i )
    {
      bool contains = false;
      const QVariant value = cats.at( i ).value();
      for ( int j = 0; j < prevCats.size() && !contains; ++j )
      {
        const QVariant prevCatValue = prevCats.at( j ).value();
        if ( prevCatValue.userType() == QMetaType::Type::QVariantList )
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
        if ( keepExistingColors && mBtnColorRamp->isRandomColorRamp() )
        {
          // insure that append symbols have random colors
          Qgs3DSymbolUtils::setVectorSymbolBaseColor( cats.at( i ).symbol(), randomColors.color( i ) );
        }
        prevCats.append( cats.at( i ) );
      }
    }
    cats = prevCats;
  }

  mOldClassificationAttribute = attributeName;

  // recreate renderer
  auto renderer = std::make_unique<QgsCategorized3DRenderer>( attributeName, cats );
  renderer->setSourceSymbol( mCategorizedSymbol->clone() );
  std::unique_ptr<QgsColorRamp> ramp( mBtnColorRamp->colorRamp() );
  if ( ramp )
  {
    renderer->setSourceColorRamp( ramp->clone() );
  }

  if ( mModel )
  {
    mModel->setRenderer( renderer.get() );
  }
  mRenderer = std::move( renderer );
  if ( !keepExistingColors && ramp )
  {
    applyColorRamp();
  }

  emit widgetChanged();
}

void QgsCategorized3DRendererWidget::applyColorRamp()
{
  if ( !mBtnColorRamp->isNull() )
  {
    mRenderer->updateColorRamp( mBtnColorRamp->colorRamp() );
  }
  mModel->updateSymbology();
}

int QgsCategorized3DRendererWidget::currentCategoryRow()
{
  const QModelIndex idx = mViewCategories->selectionModel()->currentIndex();
  if ( !idx.isValid() )
  {
    return -1;
  }

  return idx.row();
}

QList<int> QgsCategorized3DRendererWidget::selectedCategories()
{
  QList<int> rows;
  const QModelIndexList selectedRows = mViewCategories->selectionModel()->selectedRows();

  const auto constSelectedRows = selectedRows;
  for ( const QModelIndex &idx : constSelectedRows )
  {
    if ( idx.isValid() )
    {
      rows.append( idx.row() );
    }
  }
  return rows;
}

void QgsCategorized3DRendererWidget::deleteCategories()
{
  const QList<int> categoryIndexes = selectedCategories();
  mModel->deleteRows( categoryIndexes );
  emit widgetChanged();
}

void QgsCategorized3DRendererWidget::deleteAllCategories()
{
  mModel->removeAllRows();
  emit widgetChanged();
}

void QgsCategorized3DRendererWidget::deleteUnusedCategories()
{
  if ( !mRenderer )
  {
    return;
  }

  const QString attributeName = mExpressionWidget->currentField();
  bool valuesRetrieved;
  const QList<QVariant> uniqueValues = QgsVectorLayerUtils::uniqueValues( mLayer, attributeName, valuesRetrieved );
  if ( !valuesRetrieved )
  {
    QgsDebugMsgLevel( u"Unable to retrieve values from layer %1 with expression %2"_s.arg( mLayer->name() ).arg( attributeName ), 2 );
  }

  const Qgs3DCategoryList catList = mRenderer->categories();

  QList<int> unusedIndexes;

  for ( int i = 0; i < catList.size(); ++i )
  {
    const Qgs3DRendererCategory cat = catList.at( i );
    if ( !uniqueValues.contains( cat.value() ) )
    {
      unusedIndexes.append( i );
    }
  }
  mModel->deleteRows( unusedIndexes );
  emit widgetChanged();
}

void QgsCategorized3DRendererWidget::addCategory()
{
  if ( !mModel || !mLayer )
  {
    return;
  }

  const std::unique_ptr<QgsAbstract3DSymbol> symbol( QgsApplication::symbol3DRegistry()->defaultSymbolForGeometryType( mLayer->geometryType() ) );
  symbol->setDefaultPropertiesFromLayer( mLayer );
  const Qgs3DRendererCategory category( QVariant(), symbol->clone(), true );
  mModel->addCategory( category );
  emit widgetChanged();
}

Qgs3DCategoryList QgsCategorized3DRendererWidget::selectedCategoryList() const
{
  Qgs3DCategoryList categoryList;

  QItemSelectionModel *selectionModel = mViewCategories->selectionModel();
  const QModelIndexList selectedIndexes = selectionModel->selectedRows( 1 );

  if ( !selectedIndexes.isEmpty() )
  {
    QModelIndexList::const_iterator indexIt = selectedIndexes.constBegin();
    for ( ; indexIt != selectedIndexes.constEnd(); ++indexIt )
    {
      categoryList.append( mModel->category( *indexIt ) );
    }
  }
  return categoryList;
}

void QgsCategorized3DRendererWidget::rowsMoved()
{
  mViewCategories->selectionModel()->clear();
}

void QgsCategorized3DRendererWidget::updateSymbolsFromWidget( QgsSingleSymbol3DRendererWidget *widget )
{
  mCategorizedSymbol.reset( widget->symbol()->clone() );
  applyChangeToSymbol();
}

void QgsCategorized3DRendererWidget::updateSymbolsFromButton()
{
  mCategorizedSymbol.reset( mBtnChangeCategorizedSymbol->symbol()->clone() );
  applyChangeToSymbol();
}

void QgsCategorized3DRendererWidget::applyChangeToSymbol()
{
  // When there is a selection, change the selected symbols only
  QItemSelectionModel *selectionModel = mViewCategories->selectionModel();
  const QModelIndexList selectedIndexes = selectionModel->selectedRows();

  if ( !selectedIndexes.isEmpty() )
  {
    const QList<int> selectedCats = selectedCategories();
    if ( !selectedCats.isEmpty() )
    {
      const auto constSelectedCats = selectedCats;
      for ( const int idx : constSelectedCats )
      {
        QgsAbstract3DSymbol *newCatSymbol = mCategorizedSymbol->clone();

        if ( selectedCats.count() > 1 )
        {
          //if updating multiple categories, retain the existing category colors
          const QgsAbstract3DSymbol *existingCatSymbol = mRenderer->categories().at( idx ).symbol();
          Qgs3DSymbolUtils::copyVectorSymbolMaterial( existingCatSymbol, newCatSymbol );
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

void QgsCategorized3DRendererWidget::keyPressEvent( QKeyEvent *event )
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
    Qgs3DCategoryList::iterator rendererIt = mCopyBuffer.begin();
    for ( ; rendererIt != mCopyBuffer.end(); ++rendererIt )
    {
      mModel->addCategory( *rendererIt );
    }
  }
}

void QgsCategorized3DRendererWidget::selectionChanged( const QItemSelection &, const QItemSelection & )
{
  const QList<int> selectedCats = selectedCategories();
  const QgsAbstract3DSymbol *symbolToClone = nullptr;

  // NOLINTBEGIN(bugprone-branch-clone)
  if ( !selectedCats.isEmpty() )
  {
    symbolToClone = mRenderer->categories().at( selectedCats.at( 0 ) ).symbol();
  }
  else if ( mRenderer->sourceSymbol() )
  {
    symbolToClone = mRenderer->sourceSymbol();
  }
  // NOLINTEND(bugprone-branch-clone)

  if ( symbolToClone )
  {
    whileBlocking( mBtnChangeCategorizedSymbol )->setSymbol( std::unique_ptr<QgsAbstract3DSymbol>( symbolToClone->clone() ) );
  }

  const QString title = selectedCats.size() == 1 ? mRenderer->categories().at( selectedCats.at( 0 ) ).value().toString() : tr( "Symbol Settings" );
  mBtnChangeCategorizedSymbol->setDialogTitle( title );
}
