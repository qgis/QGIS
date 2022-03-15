/***************************************************************************
  qgsvectortilebasiclabelingwidget.cpp
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilebasiclabelingwidget.h"

#include "qgsvectortilebasiclabeling.h"
#include "qgsvectortilelayer.h"

#include "qgslabelinggui.h"
#include "qgsmapcanvas.h"
#include "qgsvectortileutils.h"

#include <QMenu>

///@cond PRIVATE

const double ICON_PADDING_FACTOR = 0.16;

QgsVectorTileBasicLabelingListModel::QgsVectorTileBasicLabelingListModel( QgsVectorTileBasicLabeling *l, QObject *parent )
  : QAbstractListModel( parent )
  , mLabeling( l )
{
}

int QgsVectorTileBasicLabelingListModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mLabeling->styles().count();
}

int QgsVectorTileBasicLabelingListModel::columnCount( const QModelIndex & ) const
{
  return 5;
}

QVariant QgsVectorTileBasicLabelingListModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= mLabeling->styles().count() )
    return QVariant();

  const QList<QgsVectorTileBasicLabelingStyle> styles = mLabeling->styles();
  const QgsVectorTileBasicLabelingStyle &style = styles[index.row()];

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      if ( index.column() == 0 )
        return style.styleName();
      else if ( index.column() == 1 )
        return style.layerName().isEmpty() ? tr( "(all layers)" ) : style.layerName();
      else if ( index.column() == 2 )
        return style.minZoomLevel() >= 0 ? style.minZoomLevel() : QVariant();
      else if ( index.column() == 3 )
        return style.maxZoomLevel() >= 0 ? style.maxZoomLevel() : QVariant();
      else if ( index.column() == 4 )
        return style.filterExpression().isEmpty() ? tr( "(no filter)" ) : style.filterExpression();

      break;
    }

    case Qt::EditRole:
    {
      if ( index.column() == 0 )
        return style.styleName();
      else if ( index.column() == 1 )
        return style.layerName();
      else if ( index.column() == 2 )
        return style.minZoomLevel();
      else if ( index.column() == 3 )
        return style.maxZoomLevel();
      else if ( index.column() == 4 )
        return style.filterExpression();

      break;
    }

    case Qt::CheckStateRole:
    {
      if ( index.column() != 0 )
        return QVariant();
      return style.isEnabled() ? Qt::Checked : Qt::Unchecked;
    }

    case Qt::DecorationRole:
    {
      if ( index.column() == 0 )
      {
        const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
        return QgsPalLayerSettings::labelSettingsPreviewPixmap( style.labelSettings(), QSize( iconSize, iconSize ), QString(),  static_cast< int >( iconSize * ICON_PADDING_FACTOR ) );
      }
      break;
    }

    case MinZoom:
      return style.minZoomLevel();

    case MaxZoom:
      return style.maxZoomLevel();

    case Label:
      return style.styleName();

    case Layer:
      return style.layerName();

    case Filter:
      return style.filterExpression();

  }
  return QVariant();
}

QVariant QgsVectorTileBasicLabelingListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 5 )
  {
    QStringList lst;
    lst << tr( "Label" ) << tr( "Layer" ) << tr( "Min. Zoom" ) << tr( "Max. Zoom" ) << tr( "Filter" );
    return lst[section];
  }

  return QVariant();
}

Qt::ItemFlags QgsVectorTileBasicLabelingListModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsDropEnabled;

  const Qt::ItemFlag checkable = ( index.column() == 0 ? Qt::ItemIsUserCheckable : Qt::NoItemFlags );

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
         Qt::ItemIsEditable | checkable |
         Qt::ItemIsDragEnabled;
}

bool QgsVectorTileBasicLabelingListModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsVectorTileBasicLabelingStyle style = mLabeling->style( index.row() );

  if ( role == Qt::CheckStateRole )
  {
    style.setEnabled( value.toInt() == Qt::Checked );
    mLabeling->setStyle( index.row(), style );
    emit dataChanged( index, index );
    return true;
  }

  if ( role == Qt::EditRole )
  {
    if ( index.column() == 0 )
      style.setStyleName( value.toString() );
    else if ( index.column() == 1 )
      style.setLayerName( value.toString() );
    else if ( index.column() == 2 )
      style.setMinZoomLevel( value.toInt() );
    else if ( index.column() == 3 )
      style.setMaxZoomLevel( value.toInt() );
    else if ( index.column() == 4 )
      style.setFilterExpression( value.toString() );

    mLabeling->setStyle( index.row(), style );
    emit dataChanged( index, index );
    return true;
  }

  return false;
}

bool QgsVectorTileBasicLabelingListModel::removeRows( int row, int count, const QModelIndex &parent )
{
  QList<QgsVectorTileBasicLabelingStyle> styles = mLabeling->styles();

  if ( row < 0 || row >= styles.count() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int i = 0; i < count; i++ )
  {
    if ( row < styles.count() )
    {
      styles.removeAt( row );
    }
  }

  mLabeling->setStyles( styles );

  endRemoveRows();
  return true;
}

void QgsVectorTileBasicLabelingListModel::insertStyle( int row, const QgsVectorTileBasicLabelingStyle &style )
{
  beginInsertRows( QModelIndex(), row, row );

  QList<QgsVectorTileBasicLabelingStyle> styles = mLabeling->styles();
  styles.insert( row, style );
  mLabeling->setStyles( styles );

  endInsertRows();
}

Qt::DropActions QgsVectorTileBasicLabelingListModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QStringList QgsVectorTileBasicLabelingListModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/vnd.text.list" );
  return types;
}

QMimeData *QgsVectorTileBasicLabelingListModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    // each item consists of several columns - let's add it with just first one
    if ( !index.isValid() || index.column() != 0 )
      continue;

    const QgsVectorTileBasicLabelingStyle style = mLabeling->style( index.row() );

    QDomDocument doc;
    QDomElement rootElem = doc.createElement( QStringLiteral( "vector_tile_basic_labeling_style_mime" ) );
    style.writeXml( rootElem, QgsReadWriteContext() );
    doc.appendChild( rootElem );

    stream << doc.toString( -1 );
  }

  mimeData->setData( QStringLiteral( "application/vnd.text.list" ), encodedData );
  return mimeData;
}

bool QgsVectorTileBasicLabelingListModel::dropMimeData( const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column )

  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/vnd.text.list" ) ) )
    return false;

  if ( parent.column() > 0 )
    return false;

  QByteArray encodedData = data->data( QStringLiteral( "application/vnd.text.list" ) );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  int rows = 0;

  if ( row == -1 )
  {
    // the item was dropped at a parent - we may decide where to put the items - let's append them
    row = rowCount( parent );
  }

  while ( !stream.atEnd() )
  {
    QString text;
    stream >> text;

    QDomDocument doc;
    if ( !doc.setContent( text ) )
      continue;
    const QDomElement rootElem = doc.documentElement();
    if ( rootElem.tagName() != QLatin1String( "vector_tile_basic_labeling_style_mime" ) )
      continue;

    QgsVectorTileBasicLabelingStyle style;
    style.readXml( rootElem, QgsReadWriteContext() );

    insertStyle( row + rows, style );
    ++rows;
  }
  return true;
}


//


QgsVectorTileBasicLabelingWidget::QgsVectorTileBasicLabelingWidget( QgsVectorTileLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mMapCanvas( canvas )
  , mMessageBar( messageBar )
{

  setupUi( this );
  layout()->setContentsMargins( 0, 0, 0, 0 );

  mFilterLineEdit->setShowClearButton( true );
  mFilterLineEdit->setShowSearchIcon( true );
  mFilterLineEdit->setPlaceholderText( tr( "Filter rules" ) );

  QMenu *menuAddRule = new QMenu( btnAddRule );
  menuAddRule->addAction( tr( "Marker" ), this, [this] { addStyle( QgsWkbTypes::PointGeometry ); } );
  menuAddRule->addAction( tr( "Line" ), this, [this] { addStyle( QgsWkbTypes::LineGeometry ); } );
  menuAddRule->addAction( tr( "Fill" ), this, [this] { addStyle( QgsWkbTypes::PolygonGeometry ); } );
  btnAddRule->setMenu( menuAddRule );

  //connect( btnAddRule, &QPushButton::clicked, this, &QgsVectorTileBasicLabelingWidget::addStyle );
  connect( btnEditRule, &QPushButton::clicked, this, &QgsVectorTileBasicLabelingWidget::editStyle );
  connect( btnRemoveRule, &QAbstractButton::clicked, this, &QgsVectorTileBasicLabelingWidget::removeStyle );

  connect( viewStyles, &QAbstractItemView::doubleClicked, this, &QgsVectorTileBasicLabelingWidget::editStyleAtIndex );

  if ( mMapCanvas )
  {
    connect( mMapCanvas, &QgsMapCanvas::scaleChanged, this, [ = ]( double scale )
    {
      const int zoom = mVTLayer ? mVTLayer->tileMatrixSet().scaleToZoomLevel( scale ) : QgsVectorTileUtils::scaleToZoomLevel( scale, 0, 99 );
      mLabelCurrentZoom->setText( tr( "Current zoom: %1" ).arg( zoom ) );
      if ( mProxyModel )
        mProxyModel->setCurrentZoom( zoom );
    } );
    mLabelCurrentZoom->setText( tr( "Current zoom: %1" ).arg( mVTLayer ? mVTLayer->tileMatrixSet().scaleToZoomLevel( mMapCanvas->scale() ) : QgsVectorTileUtils::scaleToZoomLevel( mMapCanvas->scale(), 0, 99 ) ) );
  }

  connect( mCheckVisibleOnly, &QCheckBox::toggled, this, [ = ]( bool filter )
  {
    mProxyModel->setFilterVisible( filter );
  } );

  connect( mFilterLineEdit, &QgsFilterLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    mProxyModel->setFilterString( text );
  } );

  setLayer( layer );
}

void QgsVectorTileBasicLabelingWidget::setLayer( QgsVectorTileLayer *layer )
{
  mVTLayer = layer;

  if ( layer && layer->labeling() && layer->labeling()->type() == QLatin1String( "basic" ) )
  {
    mLabeling.reset( static_cast<QgsVectorTileBasicLabeling *>( layer->labeling()->clone() ) );
  }
  else
  {
    mLabeling.reset( new QgsVectorTileBasicLabeling() );
  }

  mModel = new QgsVectorTileBasicLabelingListModel( mLabeling.get(), viewStyles );
  mProxyModel = new QgsVectorTileBasicLabelingProxyModel( mModel, viewStyles );
  viewStyles->setModel( mProxyModel );

  if ( mMapCanvas )
  {
    const int zoom = mVTLayer ? mVTLayer->tileMatrixSet().scaleToZoomLevel( mMapCanvas->scale() ) : QgsVectorTileUtils::scaleToZoomLevel( mMapCanvas->scale(), 0, 99 );
    mProxyModel->setCurrentZoom( zoom );
  }

  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mModel, &QAbstractItemModel::rowsInserted, this, &QgsPanelWidget::widgetChanged );
  connect( mModel, &QAbstractItemModel::rowsRemoved, this, &QgsPanelWidget::widgetChanged );
}

QgsVectorTileBasicLabelingWidget::~QgsVectorTileBasicLabelingWidget() = default;

void QgsVectorTileBasicLabelingWidget::apply()
{
  mVTLayer->setLabeling( mLabeling->clone() );
}

void QgsVectorTileBasicLabelingWidget::addStyle( QgsWkbTypes::GeometryType geomType )
{
  QgsVectorTileBasicLabelingStyle style;
  style.setGeometryType( geomType );
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      style.setFilterExpression( QStringLiteral( "geometry_type($geometry)='Point'" ) );
      break;
    case QgsWkbTypes::LineGeometry:
      style.setFilterExpression( QStringLiteral( "geometry_type($geometry)='Line'" ) );
      break;
    case QgsWkbTypes::PolygonGeometry:
      style.setFilterExpression( QStringLiteral( "geometry_type($geometry)='Polygon'" ) );
      break;
    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      break;
  }

  const int rows = mModel->rowCount();
  mModel->insertStyle( rows, style );
  viewStyles->selectionModel()->setCurrentIndex( mProxyModel->mapFromSource( mModel->index( rows, 0 ) ), QItemSelectionModel::ClearAndSelect );
}

void QgsVectorTileBasicLabelingWidget::editStyle()
{
  editStyleAtIndex( viewStyles->selectionModel()->currentIndex() );
}

void QgsVectorTileBasicLabelingWidget::editStyleAtIndex( const QModelIndex &proxyIndex )
{
  const QModelIndex index = mProxyModel->mapToSource( proxyIndex );
  if ( index.row() < 0 || index.row() >= mLabeling->styles().count() )
    return;

  const QgsVectorTileBasicLabelingStyle style = mLabeling->style( index.row() );

  QgsPalLayerSettings labelSettings = style.labelSettings();
  if ( labelSettings.layerType == QgsWkbTypes::UnknownGeometry )
    labelSettings.layerType = style.geometryType();

  QgsSymbolWidgetContext context;
  context.setMapCanvas( mMapCanvas );
  context.setMessageBar( mMessageBar );

  if ( mMapCanvas )
  {
    const int zoom = mVTLayer ? mVTLayer->tileMatrixSet().scaleToZoomLevel( mMapCanvas->scale() ) : QgsVectorTileUtils::scaleToZoomLevel( mMapCanvas->scale(), 0, 99 );
    QList<QgsExpressionContextScope> scopes = context.additionalExpressionContextScopes();
    QgsExpressionContextScope tileScope;
    tileScope.setVariable( "zoom_level", zoom, true );
    tileScope.setVariable( "vector_tile_zoom", mVTLayer ? mVTLayer->tileMatrixSet().scaleToZoom( mMapCanvas->scale() ) : QgsVectorTileUtils::scaleToZoom( mMapCanvas->scale() ), true );
    scopes << tileScope;
    context.setAdditionalExpressionContextScopes( scopes );
  }

  QgsVectorLayer *vectorLayer = nullptr;  // TODO: have a temporary vector layer with sub-layer's fields?

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsLabelingPanelWidget *widget = new QgsLabelingPanelWidget( labelSettings, vectorLayer, mMapCanvas, panel );
    widget->setContext( context );
    widget->setPanelTitle( style.styleName() );
    connect( widget, &QgsPanelWidget::widgetChanged, this, &QgsVectorTileBasicLabelingWidget::updateLabelingFromWidget );
    openPanel( widget );
  }
  else
  {
    QgsLabelSettingsDialog dlg( labelSettings, vectorLayer, mMapCanvas, this, labelSettings.layerType );
    if ( dlg.exec() )
    {
      QgsVectorTileBasicLabelingStyle style = mLabeling->style( index.row() );
      style.setLabelSettings( dlg.settings() );
      mLabeling->setStyle( index.row(), style );
      emit widgetChanged();
    }
  }
}

void QgsVectorTileBasicLabelingWidget::updateLabelingFromWidget()
{
  const int index = mProxyModel->mapToSource( viewStyles->selectionModel()->currentIndex() ).row();
  if ( index < 0 )
    return;

  QgsVectorTileBasicLabelingStyle style = mLabeling->style( index );

  QgsLabelingPanelWidget *widget = qobject_cast<QgsLabelingPanelWidget *>( sender() );
  style.setLabelSettings( widget->labelSettings() );

  mLabeling->setStyle( index, style );
  emit widgetChanged();
}

void QgsVectorTileBasicLabelingWidget::removeStyle()
{
  const QModelIndexList sel = viewStyles->selectionModel()->selectedIndexes();

  QList<int > res;
  for ( const QModelIndex &proxyIndex : sel )
  {
    const QModelIndex sourceIndex = mProxyModel->mapToSource( proxyIndex );
    if ( !res.contains( sourceIndex.row() ) )
      res << sourceIndex.row();
  }
  std::sort( res.begin(), res.end() );

  for ( int i = res.size() - 1; i >= 0; --i )
  {
    mModel->removeRow( res[ i ] );
  }
  // make sure that the selection is gone
  viewStyles->selectionModel()->clear();
}


//


QgsLabelingPanelWidget::QgsLabelingPanelWidget( const QgsPalLayerSettings &labelSettings, QgsVectorLayer *vectorLayer, QgsMapCanvas *mapCanvas, QWidget *parent )
  : QgsPanelWidget( parent )
{
  mLabelingGui = new QgsLabelingGui( vectorLayer, mapCanvas, labelSettings, this, labelSettings.layerType );
  mLabelingGui->setLabelMode( QgsLabelingGui::Labels );

  mLabelingGui->layout()->setContentsMargins( 0, 0, 0, 0 );
  QVBoxLayout *l = new QVBoxLayout;
  l->addWidget( mLabelingGui );
  setLayout( l );

  connect( mLabelingGui, &QgsTextFormatWidget::widgetChanged, this, &QgsLabelingPanelWidget::widgetChanged );
}

void QgsLabelingPanelWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mLabelingGui->setDockMode( dockMode );
}

void QgsLabelingPanelWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mLabelingGui->setContext( context );
}

QgsPalLayerSettings QgsLabelingPanelWidget::labelSettings()
{
  return mLabelingGui->layerSettings();
}


QgsVectorTileBasicLabelingProxyModel::QgsVectorTileBasicLabelingProxyModel( QgsVectorTileBasicLabelingListModel *source, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setSourceModel( source );
  setDynamicSortFilter( true );
}

void QgsVectorTileBasicLabelingProxyModel::setCurrentZoom( int zoom )
{
  mCurrentZoom = zoom;
  invalidateFilter();
}

void QgsVectorTileBasicLabelingProxyModel::setFilterVisible( bool enabled )
{
  mFilterVisible = enabled;
  invalidateFilter();
}

void QgsVectorTileBasicLabelingProxyModel::setFilterString( const QString &string )
{
  mFilterString = string;
  invalidateFilter();
}

bool QgsVectorTileBasicLabelingProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mCurrentZoom >= 0 && mFilterVisible )
  {
    const int rowMinZoom = sourceModel()->data( sourceModel()->index( source_row, 0, source_parent ), QgsVectorTileBasicLabelingListModel::MinZoom ).toInt();
    const int rowMaxZoom = sourceModel()->data( sourceModel()->index( source_row, 0, source_parent ), QgsVectorTileBasicLabelingListModel::MaxZoom ).toInt();

    if ( rowMinZoom >= 0 && rowMinZoom > mCurrentZoom )
      return false;

    if ( rowMaxZoom >= 0 && rowMaxZoom < mCurrentZoom )
      return false;
  }

  if ( !mFilterString.isEmpty() )
  {
    const QString name = sourceModel()->data( sourceModel()->index( source_row, 0, source_parent ), QgsVectorTileBasicLabelingListModel::Label ).toString();
    const QString layer = sourceModel()->data( sourceModel()->index( source_row, 0, source_parent ), QgsVectorTileBasicLabelingListModel::Layer ).toString();
    const QString filter = sourceModel()->data( sourceModel()->index( source_row, 0, source_parent ), QgsVectorTileBasicLabelingListModel::Filter ).toString();
    if ( !name.contains( mFilterString, Qt::CaseInsensitive )
         && !layer.contains( mFilterString, Qt::CaseInsensitive )
         && !filter.contains( mFilterString, Qt::CaseInsensitive ) )
    {
      return false;
    }
  }

  return true;
}

///@endcond
