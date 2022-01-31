/***************************************************************************
    qgspointcloudclassifiedrendererwidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudclassifiedrendererwidget.h"
#include "qgscontrastenhancement.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgsdoublevalidator.h"
#include "qgsstyle.h"
#include "qgsguiutils.h"
#include "qgscompoundcolorwidget.h"
#include "qgscolordialog.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgspointcloudrendererregistry.h"

#include <QMimeData>

///@cond PRIVATE

QgsPointCloudClassifiedRendererModel::QgsPointCloudClassifiedRendererModel( QObject *parent )
  : QAbstractItemModel( parent )
  , mMimeFormat( QStringLiteral( "application/x-qgspointcloudclassifiedrenderermodel" ) )
{
}

void QgsPointCloudClassifiedRendererModel::setRendererCategories( const QgsPointCloudCategoryList &categories )
{
  if ( !mCategories.empty() )
  {
    beginRemoveRows( QModelIndex(), 0, std::max< int >( mCategories.size() - 1, 0 ) );
    mCategories.clear();
    endRemoveRows();
  }
  if ( categories.size() > 0 )
  {
    beginInsertRows( QModelIndex(), 0, categories.size() - 1 );
    mCategories = categories;
    endInsertRows();
  }
}

void QgsPointCloudClassifiedRendererModel::addCategory( const QgsPointCloudCategory &cat )
{
  const int idx = mCategories.size();
  beginInsertRows( QModelIndex(), idx, idx );
  mCategories.append( cat );
  endInsertRows();

  emit categoriesChanged();
}

QgsPointCloudCategory QgsPointCloudClassifiedRendererModel::category( const QModelIndex &index )
{
  const int row = index.row();
  if ( row >= mCategories.size() )
  {
    return QgsPointCloudCategory();
  }
  return mCategories.at( row );
}

Qt::ItemFlags QgsPointCloudClassifiedRendererModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || mCategories.empty() )
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;
  if ( index.column() == 1 )
  {
    flags |= Qt::ItemIsEditable;
  }
  else if ( index.column() == 2 )
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

Qt::DropActions QgsPointCloudClassifiedRendererModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QVariant QgsPointCloudClassifiedRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || mCategories.empty() )
    return QVariant();

  const QgsPointCloudCategory category = mCategories.value( index.row() );

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
          return QString::number( category.value() );
        }
        case 2:
          return category.label();
        case 3:
          const float value = mPercentages.value( category.value(), -1 );
          QString str;
          if ( value < 0 )
            str = tr( "N/A" );
          else if ( value != 0 && std::round( value * 10 ) < 1 )
            str = QStringLiteral( "< " ) + QLocale().toString( 0.1, 'f', 1 );
          else
            str = QLocale().toString( mPercentages.value( category.value() ), 'f', 1 );
          return str;
      }
      break;
    }

    case Qt::DecorationRole:
    {
      if ( index.column() == 0 )
      {
        const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
        QPixmap pix( iconSize, iconSize );
        pix.fill( category.color() );
        return QIcon( pix );
      }
      break;
    }

    case Qt::TextAlignmentRole:
    {
      if ( index.column() == 0 )
        return static_cast<Qt::Alignment::Int>( Qt::AlignHCenter );
      if ( index.column() == 3 )
        return static_cast<Qt::Alignment::Int>( Qt::AlignRight );
      return static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
    }

    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case 1:
        {
          return QString::number( category.value() );
        }

        case 2:
          return category.label();
      }
      break;
    }
  }

  return QVariant();
}

bool QgsPointCloudClassifiedRendererModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.column() == 0 && role == Qt::CheckStateRole )
  {
    if ( QGuiApplication::keyboardModifiers() == Qt::ShiftModifier || QGuiApplication::keyboardModifiers() == Qt::ControlModifier )
    {
      toggleSelectedSymbols( value == Qt::Checked );
    }
    else
      mCategories[ index.row() ].setRenderState( value == Qt::Checked );
    emit dataChanged( index, index );
    emit categoriesChanged();
    return true;
  }

  if ( role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case 1: // value
    {
      const int val = value.toInt();
      mCategories[ index.row() ].setValue( val );
      break;
    }
    case 2: // label
    {
      mCategories[ index.row() ].setLabel( value.toString() );
      break;
    }
    default:
      return false;
  }

  emit dataChanged( index, index );
  emit categoriesChanged();
  return true;
}

QVariant QgsPointCloudClassifiedRendererModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 4 )
  {
    QStringList lst;
    lst << tr( "Color" ) << tr( "Value" ) << tr( "Legend" ) << tr( "Percentage" );
    return lst.value( section );
  }
  return QVariant();
}

int QgsPointCloudClassifiedRendererModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return 0;
  }
  return mCategories.size();
}

int QgsPointCloudClassifiedRendererModel::columnCount( const QModelIndex &index ) const
{
  Q_UNUSED( index )
  return 4;
}

QModelIndex QgsPointCloudClassifiedRendererModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column );
  }
  return QModelIndex();
}

QModelIndex QgsPointCloudClassifiedRendererModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )
  return QModelIndex();
}

QStringList QgsPointCloudClassifiedRendererModel::mimeTypes() const
{
  QStringList types;
  types << mMimeFormat;
  return types;
}

QMimeData *QgsPointCloudClassifiedRendererModel::mimeData( const QModelIndexList &indexes ) const
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

bool QgsPointCloudClassifiedRendererModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( row )
  Q_UNUSED( column )
  if ( action != Qt::MoveAction )
    return true;

  if ( !data->hasFormat( mMimeFormat ) )
    return false;

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
  if ( to == -1 )
    to = mCategories.size(); // out of rang ok, will be decreased
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    int t = to;
    if ( rows[i] < t )
      t--;

    if ( !( rows[i] < 0 ||  rows[i] >= mCategories.size() || t < 0 || t >= mCategories.size() ) )
    {
      mCategories.move( rows[i], t );
    }

    // current moved under another, shift its index up
    for ( int j = 0; j < i; j++ )
    {
      if ( to < rows[j] && rows[i] > rows[j] )
        rows[j] += 1;
    }
    // removed under 'to' so the target shifted down
    if ( rows[i] < to )
      to--;
  }
  emit dataChanged( createIndex( 0, 0 ), createIndex( mCategories.size(), 0 ) );
  emit categoriesChanged();
  return false;
}

void QgsPointCloudClassifiedRendererModel::deleteRows( QList<int> rows )
{
  std::sort( rows.begin(), rows.end() ); // list might be unsorted, depending on how the user selected the rows
  for ( int i = rows.size() - 1; i >= 0; i-- )
  {
    beginRemoveRows( QModelIndex(), rows[i], rows[i] );
    mCategories.removeAt( rows[i] );
    endRemoveRows();
  }
  emit categoriesChanged();
}

void QgsPointCloudClassifiedRendererModel::removeAllRows()
{
  beginRemoveRows( QModelIndex(), 0, mCategories.size() - 1 );
  mCategories.clear();
  endRemoveRows();
  emit categoriesChanged();
}

void QgsPointCloudClassifiedRendererModel::setCategoryColor( int row, const QColor &color )
{
  mCategories[row].setColor( color );
  emit dataChanged( createIndex( row, 0 ), createIndex( row, 0 ) );
  emit categoriesChanged();
}

// ------------------------------ View style --------------------------------
QgsPointCloudClassifiedRendererViewStyle::QgsPointCloudClassifiedRendererViewStyle( QWidget *parent )
  : QgsProxyStyle( parent )
{}

void QgsPointCloudClassifiedRendererViewStyle::drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const
{
  if ( element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull() )
  {
    QStyleOption opt( *option );
    opt.rect.setLeft( 0 );
    // draw always as line above, because we move item to that index
    opt.rect.setHeight( 0 );
    if ( widget )
      opt.rect.setRight( widget->width() );
    QProxyStyle::drawPrimitive( element, &opt, painter, widget );
    return;
  }
  QProxyStyle::drawPrimitive( element, option, painter, widget );
}


QgsPointCloudClassifiedRendererWidget::QgsPointCloudClassifiedRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style )
  : QgsPointCloudRendererWidget( layer, style )
{
  setupUi( this );

  mAttributeComboBox->setAllowEmptyAttributeName( true );
  mAttributeComboBox->setFilters( QgsPointCloudAttributeProxyModel::Char | QgsPointCloudAttributeProxyModel::Int32 | QgsPointCloudAttributeProxyModel::Short );

  mModel = new QgsPointCloudClassifiedRendererModel( this );

  if ( layer )
  {
    mAttributeComboBox->setLayer( layer );

    setFromRenderer( layer->renderer() );
  }

  viewCategories->setModel( mModel );
  viewCategories->resizeColumnToContents( 0 );
  viewCategories->resizeColumnToContents( 1 );
  viewCategories->resizeColumnToContents( 2 );

  viewCategories->setStyle( new QgsPointCloudClassifiedRendererViewStyle( viewCategories ) );

  connect( mAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloudClassifiedRendererWidget::attributeChanged );
  connect( mModel, &QgsPointCloudClassifiedRendererModel::categoriesChanged, this, &QgsPointCloudClassifiedRendererWidget::emitWidgetChanged );

  connect( viewCategories, &QAbstractItemView::doubleClicked, this, &QgsPointCloudClassifiedRendererWidget::categoriesDoubleClicked );
  connect( btnAddCategories, &QAbstractButton::clicked, this, &QgsPointCloudClassifiedRendererWidget::addCategories );
  connect( btnDeleteCategories, &QAbstractButton::clicked, this, &QgsPointCloudClassifiedRendererWidget::deleteCategories );
  connect( btnDeleteAllCategories, &QAbstractButton::clicked, this, &QgsPointCloudClassifiedRendererWidget::deleteAllCategories );
  connect( btnAddCategory, &QAbstractButton::clicked, this, &QgsPointCloudClassifiedRendererWidget::addCategory );
  connect( mModel, &QgsPointCloudClassifiedRendererModel::toggleSelectedSymbols, this, &QgsPointCloudClassifiedRendererWidget::toggleSelectedSymbols );
}

QgsPointCloudRendererWidget *QgsPointCloudClassifiedRendererWidget::create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * )
{
  return new QgsPointCloudClassifiedRendererWidget( layer, style );
}

QgsPointCloudRenderer *QgsPointCloudClassifiedRendererWidget::renderer()
{
  if ( !mLayer )
  {
    return nullptr;
  }

  std::unique_ptr< QgsPointCloudClassifiedRenderer > renderer = std::make_unique< QgsPointCloudClassifiedRenderer >();
  renderer->setAttribute( mAttributeComboBox->currentAttribute() );
  renderer->setCategories( mModel->categories() );

  return renderer.release();
}

QgsPointCloudCategoryList QgsPointCloudClassifiedRendererWidget::categoriesList()
{
  return mModel->categories();
}

QString QgsPointCloudClassifiedRendererWidget::attribute()
{
  return mAttributeComboBox->currentAttribute();
}

void QgsPointCloudClassifiedRendererWidget::attributeChanged()
{
  if ( mBlockChangedSignal )
    return;

  mBlockChangedSignal = true;
  mModel->removeAllRows();
  mBlockChangedSignal = false;
  addCategories();
}

void QgsPointCloudClassifiedRendererWidget::emitWidgetChanged()
{
  if ( mBlockChangedSignal )
    return;

  updateCategoriesPercentages();
  emit widgetChanged();
}

void QgsPointCloudClassifiedRendererWidget::categoriesDoubleClicked( const QModelIndex &idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeCategorySymbol();
}

void QgsPointCloudClassifiedRendererWidget::addCategories()
{
  if ( !mLayer || !mLayer->dataProvider() )
    return;


  const QString currentAttribute = mAttributeComboBox->currentAttribute();
  const QgsPointCloudStatistics stats = mLayer->statistics();
  const QList<int> providerCategories = stats.classesOf( currentAttribute );

  const QgsPointCloudCategoryList currentCategories = mModel->categories();

  const bool isClassificationAttribute = ! currentAttribute.compare( QStringLiteral( "Classification" ), Qt::CaseInsensitive );
  const QgsPointCloudCategoryList defaultLayerCategories = isClassificationAttribute ? QgsPointCloudRendererRegistry::classificationAttributeCategories( mLayer ) : QgsPointCloudCategoryList();

  mBlockChangedSignal = true;
  for ( const int &providerCategory : providerCategories )
  {
    // does this category already exist?
    bool found = false;
    for ( const QgsPointCloudCategory &c : currentCategories )
    {
      if ( c.value() == providerCategory )
      {
        found = true;
        break;
      }
    }

    if ( found )
      continue;

    QgsPointCloudCategory category;
    if ( isClassificationAttribute )
    {
      for ( const QgsPointCloudCategory &c : defaultLayerCategories )
      {
        if ( c.value() == providerCategory )
          category = c;
      }
    }
    else
    {
      category = QgsPointCloudCategory( providerCategory, QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor(), QString::number( providerCategory ) );
    }
    mModel->addCategory( category );
  }
  mBlockChangedSignal = false;
  emitWidgetChanged();
}

void QgsPointCloudClassifiedRendererWidget::addCategory()
{
  if ( !mModel )
    return;

  const QgsPointCloudCategory cat( mModel->categories().size(), QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor(), QString(), true );
  mModel->addCategory( cat );
}

void QgsPointCloudClassifiedRendererWidget::deleteCategories()
{
  const QList<int> categoryIndexes = selectedCategories();
  mModel->deleteRows( categoryIndexes );
}

void QgsPointCloudClassifiedRendererWidget::deleteAllCategories()
{
  mModel->removeAllRows();
}

void QgsPointCloudClassifiedRendererWidget::setFromRenderer( const QgsPointCloudRenderer *r )
{
  mBlockChangedSignal = true;
  if ( const QgsPointCloudClassifiedRenderer *classifiedRenderer = dynamic_cast< const QgsPointCloudClassifiedRenderer *>( r ) )
  {
    mModel->setRendererCategories( classifiedRenderer->categories() );
    mAttributeComboBox->setAttribute( classifiedRenderer->attribute() );
  }
  else
  {
    initialize();
  }
  mBlockChangedSignal = false;
  emitWidgetChanged();
}

void QgsPointCloudClassifiedRendererWidget::setFromCategories( QgsPointCloudCategoryList categories, const QString &attribute )
{
  mBlockChangedSignal = true;
  mModel->setRendererCategories( categories );
  if ( !attribute.isEmpty() )
  {
    mAttributeComboBox->setAttribute( attribute );
  }
  else
  {
    initialize();
  }
  mBlockChangedSignal = false;
  emitWidgetChanged();
}

void QgsPointCloudClassifiedRendererWidget::initialize()
{
  if ( mAttributeComboBox->findText( QStringLiteral( "Classification" ) ) > -1 )
  {
    mAttributeComboBox->setAttribute( QStringLiteral( "Classification" ) );
  }
  else
  {
    mAttributeComboBox->setCurrentIndex( mAttributeComboBox->count() > 1 ? 1 : 0 );
  }
  mModel->removeAllRows();
  addCategories();
}

void QgsPointCloudClassifiedRendererWidget::changeCategorySymbol()
{
  const int row = currentCategoryRow();
  if ( row < 0 )
    return;

  const QgsPointCloudCategory category = mModel->categories().value( row );

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, category.color(), QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( category.label() );
    colorWidget->setAllowOpacity( true );
    colorWidget->setPreviousColor( category.color() );

    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, [ = ]( const QColor & newColor )
    {
      mModel->setCategoryColor( row, newColor );
    } );
    panel->openPanel( colorWidget );
  }
  else
  {
    const QColor newColor = QgsColorDialog::getColor( category.color(), this, category.label(), true );
    if ( newColor.isValid() )
    {
      mModel->setCategoryColor( row, newColor );
    }
  }
}

QList<int> QgsPointCloudClassifiedRendererWidget::selectedCategories()
{
  QList<int> rows;
  const QModelIndexList selectedRows = viewCategories->selectionModel()->selectedRows();
  for ( const QModelIndex &r : selectedRows )
  {
    if ( r.isValid() )
    {
      rows.append( r.row() );
    }
  }
  return rows;
}

int QgsPointCloudClassifiedRendererWidget::currentCategoryRow()
{
  const QModelIndex idx = viewCategories->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}

void QgsPointCloudClassifiedRendererWidget::updateCategoriesPercentages()
{
  QMap < int, float > percentages;

  const QgsPointCloudStatistics stats = mLayer->statistics();
  const QMap<int, int> classes = stats.availableClasses( attribute() );
  const int pointCount = stats.sampledPointsCount();
  const QgsPointCloudCategoryList currentCategories = mModel->categories();

  // when the stats are 100% accurate, we are sure that missing classes have a 0% of points
  const bool statsExact = stats.sampledPointsCount() == mLayer->pointCount();
  for ( const QgsPointCloudCategory &category : currentCategories )
  {
    if ( classes.contains( category.value() ) || statsExact )
      percentages.insert( category.value(), ( double ) classes.value( category.value() ) / pointCount * 100 );
  }
  mModel->updateCategoriesPercentages( percentages );
}

void QgsPointCloudClassifiedRendererWidget::toggleSelectedSymbols( const bool state )
{
  QModelIndexList selectedIndexes = viewCategories->selectionModel()->selectedRows();
  if ( !selectedIndexes.isEmpty() && mModel )
  {
    const auto constSelectedIndexes = selectedIndexes;
    for ( const QModelIndex &idx : constSelectedIndexes )
    {
      if ( idx.isValid() )
      {
        mModel->category( idx ).setRenderState( state );
        viewCategories->update( idx );
      }
    }
  emit widgetChanged();
  }
}


///@endcond
