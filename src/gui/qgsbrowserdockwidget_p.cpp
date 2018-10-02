/***************************************************************************
    qgsbrowserdockwidget_p.cpp

    Private classes for QgsBrowserDockWidget

    ---------------------
    begin                : May 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    real work done by    : (C) 2011 by Martin Dobias
    email                : a dot pasotti at itopen dot it
    ---------------------
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsbrowserdockwidget_p.h"

#include <memory>

#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QPlainTextDocumentLayout>
#include <QSortFilterProxyModel>

#include "qgsbrowsermodel.h"
#include "qgsbrowsertreeview.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsmeshlayer.h"

#include <QDragEnterEvent>


/// @cond PRIVATE


QgsBrowserPropertiesWrapLabel::QgsBrowserPropertiesWrapLabel( const QString &text, QWidget *parent )
  : QTextEdit( text, parent )
{
  setReadOnly( true );
  setFrameStyle( QFrame::NoFrame );
  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
  QPalette pal = palette();
  pal.setColor( QPalette::Base, Qt::transparent );
  setPalette( pal );
  setLineWrapMode( QTextEdit::WidgetWidth );
  setWordWrapMode( QTextOption::WrapAnywhere );
  connect( qobject_cast<QAbstractTextDocumentLayout *>( document()->documentLayout() ), &QAbstractTextDocumentLayout::documentSizeChanged,
           this, &QgsBrowserPropertiesWrapLabel::adjustHeight );
  setMaximumHeight( 20 );
}

void QgsBrowserPropertiesWrapLabel::adjustHeight( QSizeF size )
{
  int height = size.height() + 2 * frameWidth();
  setMinimumHeight( height );
  setMaximumHeight( height );
}

QgsBrowserPropertiesWidget::QgsBrowserPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
}

void QgsBrowserPropertiesWidget::setWidget( QWidget *paramWidget )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  paramWidget->setParent( this );
  layout->addWidget( paramWidget );
}

QgsBrowserPropertiesWidget *QgsBrowserPropertiesWidget::createWidget( QgsDataItem *item, QWidget *parent )
{
  QgsBrowserPropertiesWidget *propertiesWidget = nullptr;
  // In general, we would like to show all items' paramWidget, but top level items like
  // WMS etc. have currently too large widgets which do not fit well to browser properties widget
  if ( item->type() == QgsDataItem::Directory )
  {
    propertiesWidget = new QgsBrowserDirectoryProperties( parent );
    propertiesWidget->setItem( item );
  }
  else if ( item->type() == QgsDataItem::Layer )
  {
    // prefer item's widget over standard layer widget
    QWidget *paramWidget = item->paramWidget();
    if ( paramWidget )
    {
      propertiesWidget = new QgsBrowserPropertiesWidget( parent );
      propertiesWidget->setWidget( paramWidget );
    }
    else
    {
      propertiesWidget = new QgsBrowserLayerProperties( parent );
      propertiesWidget->setItem( item );
    }
  }
  return propertiesWidget;
}

QgsBrowserLayerProperties::QgsBrowserLayerProperties( QWidget *parent )
  : QgsBrowserPropertiesWidget( parent )
{
  setupUi( this );

  mUriLabel = new QgsBrowserPropertiesWrapLabel( QString(), this );
  mHeaderGridLayout->addItem( new QWidgetItem( mUriLabel ), 1, 1 );
}

class ProjectionSettingRestorer
{
  public:

    ProjectionSettingRestorer()
    {
      QgsSettings settings;
      previousSetting = settings.value( QStringLiteral( "/Projections/defaultBehavior" ) ).toString();
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), QStringLiteral( "useProject" ) );
    }

    ~ProjectionSettingRestorer()
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), previousSetting );
    }

    QString previousSetting;
};

void QgsBrowserLayerProperties::setItem( QgsDataItem *item )
{
  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
  if ( !layerItem )
    return;

  mNoticeLabel->clear();

  QgsMapLayer::LayerType type = layerItem->mapLayerType();
  QString layerMetadata = tr( "Error" );
  QgsCoordinateReferenceSystem layerCrs;

  QString defaultProjectionOption = QgsSettings().value( QStringLiteral( "Projections/defaultBehavior" ), "prompt" ).toString();
  // temporarily override /Projections/defaultBehavior to avoid dialog prompt
  // TODO - remove when there is a cleaner way to block the unknown projection dialog!
  ProjectionSettingRestorer restorer;
  ( void )restorer; // no warnings

  // find root item
  // we need to create a temporary layer to get metadata
  // we could use a provider but the metadata is not as complete and "pretty"  and this is easier
  QgsDebugMsg( QString( "creating temporary layer using path %1" ).arg( layerItem->path() ) );
  if ( type == QgsMapLayer::RasterLayer )
  {
    QgsDebugMsg( "creating raster layer" );
    // should copy code from addLayer() to split uri ?
    std::unique_ptr<QgsRasterLayer> layer( new QgsRasterLayer( layerItem->uri(), layerItem->uri(), layerItem->providerKey() ) );
    if ( layer )
    {
      if ( layer->isValid() )
      {
        bool ok = false;
        layer->loadDefaultMetadata( ok );
        layerCrs = layer->crs();
        layerMetadata = layer->htmlMetadata();
      }
    }
  }
  else if ( type == QgsMapLayer::MeshLayer )
  {
    QgsDebugMsg( "creating mesh layer" );
    std::unique_ptr<QgsMeshLayer> layer( new QgsMeshLayer( layerItem->uri(), layerItem->uri(), layerItem->providerKey() ) );
    if ( layer )
    {
      if ( layer->isValid() )
      {
        bool ok = false;
        layer->loadDefaultMetadata( ok );
        layerCrs = layer->crs();
        layerMetadata = layer->htmlMetadata();
      }
    }
  }
  else if ( type == QgsMapLayer::VectorLayer )
  {
    QgsDebugMsg( "creating vector layer" );
    std::unique_ptr<QgsVectorLayer> layer( new QgsVectorLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey() ) );
    if ( layer )
    {
      if ( layer->isValid() )
      {
        bool ok = false;
        layer->loadDefaultMetadata( ok );
        layerCrs = layer->crs();
        layerMetadata = layer->htmlMetadata();
      }
    }
  }
  else if ( type == QgsMapLayer::PluginLayer )
  {
    // TODO: support display of properties for plugin layers
    return;
  }

  mNameLabel->setText( layerItem->name() );
  mUriLabel->setText( layerItem->uri() );
  mProviderLabel->setText( layerItem->providerKey() );
  QString myStyle = QgsApplication::reportStyleSheet();
  mMetadataTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mMetadataTextBrowser->setHtml( layerMetadata );

  // report if layer was set to to project crs without prompt (may give a false positive)
  if ( defaultProjectionOption == QLatin1String( "prompt" ) )
  {
    QgsCoordinateReferenceSystem defaultCrs =
      QgsProject::instance()->crs();
    if ( layerCrs == defaultCrs )
      mNoticeLabel->setText( "NOTICE: Layer CRS set from project (" + defaultCrs.authid() + ')' );
  }

  if ( mNoticeLabel->text().isEmpty() )
  {
    mNoticeLabel->hide();
  }
}

void QgsBrowserLayerProperties::setCondensedMode( bool condensedMode )
{
  if ( condensedMode )
  {
    mUriLabel->setLineWrapMode( QTextEdit::NoWrap );
    mUriLabel->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    mUriLabel->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  }
  else
  {
    mUriLabel->setLineWrapMode( QTextEdit::WidgetWidth );
    mUriLabel->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    mUriLabel->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  }
}

QgsBrowserDirectoryProperties::QgsBrowserDirectoryProperties( QWidget *parent )
  : QgsBrowserPropertiesWidget( parent )

{
  setupUi( this );

  mPathLabel = new QgsBrowserPropertiesWrapLabel( QString(), mHeaderWidget );
  mHeaderGridLayout->addItem( new QWidgetItem( mPathLabel ), 0, 1 );
}

void QgsBrowserDirectoryProperties::setItem( QgsDataItem *item )
{
  QgsDirectoryItem *directoryItem = qobject_cast<QgsDirectoryItem *>( item );
  if ( !item )
    return;

  mPathLabel->setText( QDir::toNativeSeparators( directoryItem->dirPath() ) );
  mDirectoryWidget = new QgsDirectoryParamWidget( directoryItem->dirPath(), this );
  mLayout->addWidget( mDirectoryWidget );
}

QgsBrowserPropertiesDialog::QgsBrowserPropertiesDialog( const QString &settingsSection, QWidget *parent )
  : QDialog( parent )
  , mSettingsSection( settingsSection )
{
  setupUi( this );
  QgsSettings settings;
  restoreGeometry( settings.value( mSettingsSection + "/propertiesDialog/geometry" ).toByteArray() );
}

QgsBrowserPropertiesDialog::~QgsBrowserPropertiesDialog()
{
  QgsSettings settings;
  settings.setValue( mSettingsSection + "/propertiesDialog/geometry", saveGeometry() );
}

void QgsBrowserPropertiesDialog::setItem( QgsDataItem *item )
{
  if ( !item )
    return;

  mPropertiesWidget = QgsBrowserPropertiesWidget::createWidget( item, this );
  mLayout->addWidget( mPropertiesWidget );
  setWindowTitle( item->type() == QgsDataItem::Layer ? tr( "Layer Properties" ) : tr( "Directory Properties" ) );
}


//
// QgsDockBrowserTreeView
//

QgsDockBrowserTreeView::QgsDockBrowserTreeView( QWidget *parent ) : QgsBrowserTreeView( parent )
{
  setDragDropMode( QTreeView::DragDrop ); // sets also acceptDrops + dragEnabled
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setContextMenuPolicy( Qt::CustomContextMenu );
  setHeaderHidden( true );
  setDropIndicatorShown( true );

}

void QgsDockBrowserTreeView::setAction( QDropEvent *e )
{
  // if this mime data come from layer tree, the proposed action will be MoveAction
  // but for browser we really need CopyAction
  if ( e->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) &&
       e->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    e->setDropAction( Qt::CopyAction );
  }
}

void QgsDockBrowserTreeView::dragEnterEvent( QDragEnterEvent *e )
{
  setAction( e );

  // accept drag enter so that our widget will not get ignored
  // and drag events will not get passed to QgisApp
  e->accept();
}

void QgsDockBrowserTreeView::dragMoveEvent( QDragMoveEvent *e )
{
  // do not accept drops above/below items
  /*if ( dropIndicatorPosition() != QAbstractItemView::OnItem )
      {
        QgsDebugMsg("drag not on item");
        e->ignore();
        return;
      }*/

  setAction( e );
  QTreeView::dragMoveEvent( e );
  // reset action because QTreeView::dragMoveEvent() accepts proposed action
  setAction( e );

  if ( !e->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    e->ignore();
    return;
  }
}

void QgsDockBrowserTreeView::dropEvent( QDropEvent *e )
{
  setAction( e );
  QTreeView::dropEvent( e );
  // reset action because QTreeView::dropEvent() accepts proposed action
  setAction( e );
}

//
// QgsBrowserTreeFilterProxyModel
//

QgsBrowserTreeFilterProxyModel::QgsBrowserTreeFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mPatternSyntax( QStringLiteral( "normal" ) )
  , mCaseSensitivity( Qt::CaseInsensitive )
{
  setDynamicSortFilter( true );
  setSortRole( QgsBrowserModel::SortRole );
  setSortCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

void QgsBrowserTreeFilterProxyModel::setBrowserModel( QgsBrowserModel *model )
{
  mModel = model;
  setSourceModel( model );
}

void QgsBrowserTreeFilterProxyModel::setFilterSyntax( const QString &syntax )
{
  QgsDebugMsg( QString( "syntax = %1" ).arg( syntax ) );
  if ( mPatternSyntax == syntax )
    return;
  mPatternSyntax = syntax;
  updateFilter();
}

void QgsBrowserTreeFilterProxyModel::setFilter( const QString &filter )
{
  QgsDebugMsg( QString( "filter = %1" ).arg( mFilter ) );
  if ( mFilter == filter )
    return;
  mFilter = filter;
  updateFilter();
}

void QgsBrowserTreeFilterProxyModel::setCaseSensitive( bool caseSensitive )
{
  mCaseSensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  updateFilter();
}

void QgsBrowserTreeFilterProxyModel::updateFilter()
{
  QgsDebugMsg( QString( "filter = %1 syntax = %2" ).arg( mFilter, mPatternSyntax ) );
  mREList.clear();
  if ( mPatternSyntax == QLatin1String( "normal" ) )
  {
    Q_FOREACH ( const QString &f, mFilter.split( '|' ) )
    {
      QRegExp rx( QString( "*%1*" ).arg( f.trimmed() ) );
      rx.setPatternSyntax( QRegExp::Wildcard );
      rx.setCaseSensitivity( mCaseSensitivity );
      mREList.append( rx );
    }
  }
  else if ( mPatternSyntax == QLatin1String( "wildcard" ) )
  {
    Q_FOREACH ( const QString &f, mFilter.split( '|' ) )
    {
      QRegExp rx( f.trimmed() );
      rx.setPatternSyntax( QRegExp::Wildcard );
      rx.setCaseSensitivity( mCaseSensitivity );
      mREList.append( rx );
    }
  }
  else
  {
    QRegExp rx( mFilter.trimmed() );
    rx.setPatternSyntax( QRegExp::RegExp );
    rx.setCaseSensitivity( mCaseSensitivity );
    mREList.append( rx );
  }
  invalidateFilter();
}

bool QgsBrowserTreeFilterProxyModel::filterAcceptsString( const QString &value ) const
{
  if ( mPatternSyntax == QLatin1String( "normal" ) || mPatternSyntax == QLatin1String( "wildcard" ) )
  {
    Q_FOREACH ( const QRegExp &rx, mREList )
    {
      QgsDebugMsg( QString( "value: [%1] rx: [%2] match: %3" ).arg( value, rx.pattern() ).arg( rx.exactMatch( value ) ) );
      if ( rx.exactMatch( value ) )
        return true;
    }
  }
  else
  {
    Q_FOREACH ( const QRegExp &rx, mREList )
    {
      QgsDebugMsg( QString( "value: [%1] rx: [%2] match: %3" ).arg( value, rx.pattern() ).arg( rx.indexIn( value ) ) );
      if ( rx.indexIn( value ) != -1 )
        return true;
    }
  }
  return false;
}

bool QgsBrowserTreeFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( mFilter.isEmpty() || !mModel )
    return true;

  QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  return filterAcceptsItem( sourceIndex ) || filterAcceptsAncestor( sourceIndex ) || filterAcceptsDescendant( sourceIndex );
}

bool QgsBrowserTreeFilterProxyModel::filterAcceptsAncestor( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  QModelIndex sourceParentIndex = mModel->parent( sourceIndex );
  if ( !sourceParentIndex.isValid() )
    return false;
  if ( filterAcceptsItem( sourceParentIndex ) )
    return true;

  return filterAcceptsAncestor( sourceParentIndex );
}

bool QgsBrowserTreeFilterProxyModel::filterAcceptsDescendant( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;

  for ( int i = 0; i < mModel->rowCount( sourceIndex ); i++ )
  {
    QgsDebugMsg( QString( "i = %1" ).arg( i ) );
    QModelIndex sourceChildIndex = mModel->index( i, 0, sourceIndex );
    if ( filterAcceptsItem( sourceChildIndex ) )
      return true;
    if ( filterAcceptsDescendant( sourceChildIndex ) )
      return true;
  }
  return false;
}

bool QgsBrowserTreeFilterProxyModel::filterAcceptsItem( const QModelIndex &sourceIndex ) const
{
  if ( !mModel )
    return true;
  //accept item if either displayed text or comment role matches string
  QString comment = mModel->data( sourceIndex, QgsBrowserModel::CommentRole ).toString();
  return ( filterAcceptsString( mModel->data( sourceIndex, Qt::DisplayRole ).toString() )
           || ( !comment.isEmpty() && filterAcceptsString( comment ) ) );
}

///@endcond
