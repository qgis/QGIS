/***************************************************************************
                          qgslegend.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgslogger.h"
#include "qgslegend.h"
#include "qgslegendgroup.h"
#include "qgslegendlayer.h"
#include "qgslegendpropertygroup.h"
#include "qgslegendsymbologyitem.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsgenericprojectionselector.h"

#include <QFont>
#include <QDomDocument>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QTreeWidgetItem>

const int AUTOSCROLL_MARGIN = 16;

QgsLegend::QgsLegend( QgsMapCanvas *canvas, QWidget * parent, const char *name )
    : QTreeWidget( parent )
    , mMousePressedFlag( false )
    , mMapCanvas( canvas )
    , mMinimumIconSize( 20, 20 )
    , mChanging( false )
{
  setObjectName( name );

  connect( this, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
           this, SLOT( handleItemChange( QTreeWidgetItem*, int ) ) );

  connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
           this, SLOT( handleCurrentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  // project handling
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );

  // connect map layer registry signal to legend
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ),
           this, SLOT( removeLayer( QString ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( removedAll() ),
           this, SLOT( removeAll() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ),
           this, SLOT( addLayer( QgsMapLayer * ) ) );

  connect( mMapCanvas, SIGNAL( layersChanged() ),
           this, SLOT( refreshCheckStates() ) );

  // Initialise the line indicator widget.
  mInsertionLine = new QWidget( viewport() );
  hideLine();
  mInsertionLine->setAutoFillBackground( true );
  QPalette pal = mInsertionLine->palette();
  pal.setColor( mInsertionLine->backgroundRole(), Qt::blue );
  mInsertionLine->setPalette( pal );

  setSortingEnabled( false );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setDragEnabled( false );
  setAutoScroll( true );
  QFont f( "Arial", 10, QFont::Normal );
  setFont( f );
  QPalette palette;
  palette.setColor( backgroundRole(), QColor( 192, 192, 192 ) );
  setPalette( palette );

  setColumnCount( 1 );
  header()->setHidden( 1 );
  setRootIsDecorated( true );
  initPixmaps();
}

QgsLegend::~QgsLegend()
{
  delete mInsertionLine;
}

#ifdef QGISDEBUG
void QgsLegend::showItem( QString msg, QTreeWidgetItem *item )
{
  if ( !item )
  {
    QgsDebugMsg( msg + ": no item?" );
    return;
  }

  QgsLegendItem  *litem = dynamic_cast<QgsLegendItem *>( item );
  QgsLegendGroup *group = dynamic_cast<QgsLegendGroup *>( item );
  QgsLegendLayer *layer = dynamic_cast<QgsLegendLayer *>( item );

  QString type;
  if ( group )
    type = "group";
  else if ( layer )
    type = "layer";
  else if ( litem )
    type = "litem";
  else
    type = "item";

  QgsDebugMsg( QString( "%1: %2 %3 row:%4" ).arg( msg ).arg( type ).arg( item->text( 0 ) ).arg( indexFromItem( item ).row() ) );
}
#else
#define showItem(msg, item)
#endif


void QgsLegend::handleCurrentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
  QgsMapLayer *layer = currentLayer();

  if ( mMapCanvas )
  {
    mMapCanvas->setCurrentLayer( layer );
  }

  emit currentLayerChanged( layer );
}

int QgsLegend::addGroupToCurrentItem( QString name, bool expand )
{
  QTreeWidgetItem *parent = currentItem();
  return addGroup( name, expand, parent );
}

int QgsLegend::addGroup( QString name, bool expand, QTreeWidgetItem* parent )
{
  //avoid multiple refreshes of map canvas because of itemChanged signal
  blockSignals( true );

  bool nameEmpty = name.isEmpty();
  if ( nameEmpty )
    name = tr( "group" ); // some default name if none specified

  QgsLegendGroup *parentGroup = dynamic_cast<QgsLegendGroup *>( parent );
  QgsLegendGroup *group;

  if ( parentGroup )
    group = new QgsLegendGroup( parentGroup, name );
  else
    group = new QgsLegendGroup( this, name );

  QModelIndex groupIndex = indexFromItem( group );
  setExpanded( groupIndex, expand );
  setCurrentItem( group );
  if ( nameEmpty )
    openEditor();

  blockSignals( false );
  return groupIndex.row();
}

int QgsLegend::addGroup( QString name, bool expand, int groupIndex )
{
  QgsLegendGroup * lg = dynamic_cast<QgsLegendGroup *>( topLevelItem( groupIndex ) );
  return addGroup( name, expand, lg );
}

void QgsLegend::removeAll()
{
  clear();
  mEmbeddedGroups.clear();
  mPixmapWidthValues.clear();
  mPixmapHeightValues.clear();
  updateMapCanvasLayerSet();
  setIconSize( mMinimumIconSize );
  mDropTarget = 0;
}

void QgsLegend::selectAll( bool select )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  // Turn off rendering to improve speed.
  bool renderFlagState = mMapCanvas->renderFlag();
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( false );

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem* litem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( litem && litem->type() == QgsLegendItem::LEGEND_LAYER )
    {
      theItem->setCheckState( 0, select ? Qt::Checked : Qt::Unchecked );
      handleItemChange( theItem, 0 );
    }
  }

  // Turn on rendering (if it was on previously)
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( true );

  QgsProject::instance()->dirty( true );
}

void QgsLegend::removeGroup( int groupIndex )
{
  QgsLegendGroup * lg = dynamic_cast<QgsLegendGroup *>( topLevelItem( groupIndex ) );
  if ( lg )
  {
    removeGroup( lg );
  }
}

void QgsLegend::removeLayer( QString layerId )
{
  QgsDebugMsg( "called." );

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem *li = dynamic_cast<QgsLegendItem *>( theItem );
    if ( li )
    {
      // save legend layer (parent of a legend layer file we're going to delete)
      QgsLegendLayer* ll = qobject_cast<QgsLegendLayer *>( li );

      if ( ll && ll->layer() && ll->layer()->id() == layerId )
      {
        removeItem( ll );
        delete ll;
        break;
      }
    }
  }

  updateMapCanvasLayerSet();
  adjustIconSize();
}

void QgsLegend::mousePressEvent( QMouseEvent * e )
{
  if ( e->button() == Qt::LeftButton )
  {
    mMousePressedFlag = true;
    mDropTarget = itemAt( e->pos() );
  }
  else if ( e->button() == Qt::RightButton )
  {
    QTreeWidgetItem* item = itemAt( e->pos() );
    if ( !item || item == currentItem() )
    {
      if ( !item )
        setCurrentItem( 0 );
      handleRightClickEvent( item, e->globalPos() );
    }
  }
  QTreeWidget::mousePressEvent( e );
}                               // contentsMousePressEvent

void QgsLegend::mouseMoveEvent( QMouseEvent * e )
{
  if ( !mMousePressedFlag )
  {
    QgsDebugMsg( "mouse not pressed" );
    return;
  }

  if ( mItemsBeingMoved.isEmpty() && !selectedItems().isEmpty() )
  {
    if ( mDropTarget == itemAt( e->pos() ) )
      return;

    mLayersPriorToMove = layerIDs();
    QgsDebugMsg( "layers prior to move: " + mLayersPriorToMove.join( ", " ) );

    // record which items were selected and hide them
    foreach( QTreeWidgetItem * item, selectedItems() )
    {
      //prevent to drag out content under groups that are embedded from other
      //project files.
      if ( parentGroupEmbedded( item ) )
      {
        continue;
      }
      item->setHidden( true );
      mItemsBeingMoved << item;
    }

    // remove and unhide items, whose parent is already to be moved
    foreach( QTreeWidgetItem * item, mItemsBeingMoved )
    {
      QTreeWidgetItem *parent = item->parent();

      bool parentHidden = false;
      while ( !parentHidden && parent )
      {
        parentHidden = parent->isHidden();
        parent = parent->parent();
      }

      if ( parentHidden )
      {
        mItemsBeingMoved.removeOne( item );
        item->setHidden( false );
      }
    }

    setCursor( Qt::SizeVerCursor );
  }

  if ( mItemsBeingMoved.isEmpty() )
  {
    QgsDebugMsg( "nothing to move" );
    setCursor( QCursor( Qt::ArrowCursor ) );
    return;
  }

  // change the cursor appropriate to if drop is allowed
  QTreeWidgetItem* item = itemAt( e->pos() );

  hideLine();
  updateLineWidget();
  scrollToItem( item );

  mDropTarget = 0;

  if ( item )
  {
    showItem( "moveMoveEvent" , item );

    QgsLegendItem  *litem = dynamic_cast<QgsLegendItem *>( item );
    QgsLegendGroup *group = dynamic_cast<QgsLegendGroup *>( item );
    QgsLegendLayer *layer = dynamic_cast<QgsLegendLayer *>( item );

    while ( item->parent() && !group && !layer )
    {
      item = item->parent();
      litem = dynamic_cast<QgsLegendItem *>( item );
      group = dynamic_cast<QgsLegendGroup *>( item );
      layer = dynamic_cast<QgsLegendLayer *>( item );
    }

    showItem( "layer/group" , item );

    int line_x = visualItemRect( item ).left();
    int line_y;
    if ( layer )
    {
      QTreeWidgetItem *lastItem = item->childCount() > 0 && item->isExpanded() ? item->child( item->childCount() - 1 ) : item;
      int y0 = visualItemRect( item ).top() + 1;
      int y1 = visualItemRect( lastItem ).bottom() - 2;

      mDropTarget = layer;

      //prevent inserting content into embedded groups
      if ( !parentGroupEmbedded( litem ) )
      {
        if ( e->y() < ( y0 + y1 ) / 2 )
        {
          QgsDebugMsg( "insert before layer" );
          mDropAction = BEFORE;
          line_y = y0;
        }
        else
        {
          QgsDebugMsg( "insert after layer" );
          mDropAction = AFTER;
          line_y = y1;
        }
      }
    }
    else if ( group )
    {
      if ( yCoordAboveCenter( litem, e->y() ) ) //over center of item
      {
        QgsDebugMsg( "insert before group" );

        //prevent inserting content into embedded groups
        if ( !parentGroupEmbedded( item ) )
        {
          line_y = visualItemRect( item ).top() + 1;
          mDropTarget = item;
          mDropAction = BEFORE;
        }
      }
      else // below center of item
      {
        if ( !groupEmbedded( item ) )
        {
          QgsDebugMsg( "insert into group" );

          line_y = visualItemRect( item ).bottom() - 2;
          mDropTarget = item;
          mDropAction = INSERT;
        }
      }
    }
    else
    {
      line_y = visualItemRect( item ).bottom() - 2;
    }

    showLine( line_y, line_x );
    setCursor( QCursor( Qt::SizeVerCursor ) );
  }
  else if ( !item
            && e->pos().y() >= 0 && e->pos().y() < viewport()->height()
            && e->pos().x() >= 0 && e->pos().x() < viewport()->width() )
  {
    // Outside the listed items, but check if we are in the empty area
    // of the viewport, so we can drop after the last top level item.
    mDropTarget = topLevelItem( topLevelItemCount() - 1 );
    mDropAction = AFTER;

    QgsDebugMsg( "insert after last layer/group" );
    showLine( visualItemRect( lastVisibleItem() ).bottom() + 1, 0 );
    setCursor( QCursor( Qt::SizeVerCursor ) );
  }
  else
  {
    QgsDebugMsg( "No item here" );
    setCursor( QCursor( Qt::ForbiddenCursor ) );
  }
}

void QgsLegend::updateGroupCheckStates( QTreeWidgetItem *item )
{
  QgsLegendGroup *lg = dynamic_cast< QgsLegendGroup * >( item );
  if ( !lg )
    return;

  for ( int i = 0; i < item->childCount(); i++ )
  {
    updateGroupCheckStates( item->child( i ) );
  }

  Qt::CheckState theState = lg->pendingCheckState();
  if ( theState != lg->checkState( 0 ) )
  {
    blockSignals( true );
    lg->setCheckState( 0, theState );
    blockSignals( false );
  }
}

void QgsLegend::mouseReleaseEvent( QMouseEvent * e )
{
  QTreeWidget::mouseReleaseEvent( e );
  mMousePressedFlag = false;

  if ( mItemsBeingMoved.isEmpty() )
    return;

  setCursor( QCursor( Qt::ArrowCursor ) );
  hideLine();

  // unhide
  foreach( QTreeWidgetItem * item, mItemsBeingMoved )
  {
    item->setHidden( false );
  }

  if ( mDropTarget )
  {
    if ( mDropAction == BEFORE )
    {
      showItem( "before => drop after", mDropTarget );

      QTreeWidgetItem *prev = previousSibling( mDropTarget );
      if ( prev )
      {
        mDropTarget = prev;

        showItem( "prev sibling", mDropTarget );

        foreach( QTreeWidgetItem * item, mItemsBeingMoved )
        {
          moveItem( item, mDropTarget );
          mDropTarget = item;
        }
      }
      else
      {
        mDropTarget = mDropTarget->parent();
        if ( mDropTarget )
        {
          showItem( "parent", mDropTarget );
          mDropAction = INSERT;
        }
        else
        {
          mDropTarget = invisibleRootItem();
          mDropAction = AFTER;
          showItem( "root", mDropTarget );
        }
      }
    }

    if ( mDropAction == AFTER )
    {
      showItem( "drop after", mDropTarget );

      foreach( QTreeWidgetItem * item, mItemsBeingMoved )
      {
        moveItem( item, mDropTarget );
        mDropTarget = item;
      }
    }

    if ( mDropAction == INSERT )
    {
      showItem( "insert into", mDropTarget );

      foreach( QTreeWidgetItem * item, mItemsBeingMoved )
      {
        insertItem( item, mDropTarget );
      }
    }

    mItemsBeingMoved.clear();

    for ( int i = 0; i < topLevelItemCount(); i++ )
    {
      updateGroupCheckStates( topLevelItem( i ) );
    }
  }
  else //no action
  {
    QgsDebugMsg( "Drop NO_ACTION" );
  }

  checkLayerOrderUpdate();
}

void QgsLegend::mouseDoubleClickEvent( QMouseEvent* e )
{
  QSettings settings;

  switch ( settings.value( "/qgis/legendDoubleClickAction", 0 ).toInt() )
  {
    case 0:
      QgisApp::instance()->layerProperties();
      break;
    case 1:
      QgisApp::instance()->attributeTable();
      break;
    default:
      break;
  }
}

void QgsLegend::handleRightClickEvent( QTreeWidgetItem* item, const QPoint& position )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  QMenu theMenu;

  QgsLegendItem* li = dynamic_cast<QgsLegendItem *>( item );
  if ( li )
  {
    if ( li->type() == QgsLegendItem::LEGEND_LAYER )
    {
      qobject_cast<QgsLegendLayer*>( li )->addToPopupMenu( theMenu );

      if ( li->parent() && !parentGroupEmbedded( li ) )
      {
        theMenu.addAction( tr( "&Make to toplevel item" ), this, SLOT( makeToTopLevelItem() ) );
      }
    }
    else if ( li->type() == QgsLegendItem::LEGEND_GROUP )
    {
      theMenu.addAction( QgisApp::getThemeIcon( "/mActionZoomToLayer.png" ),
                         tr( "Zoom to group" ), this, SLOT( legendLayerZoom() ) );

      theMenu.addAction( QgisApp::getThemeIcon( "/mActionRemoveLayer.png" ),
                         tr( "&Remove" ), this, SLOT( legendGroupRemove() ) );

      theMenu.addAction( QgisApp::getThemeIcon( "/mActionSetCRS.png" ),
                         tr( "&Set group CRS" ), this, SLOT( legendGroupSetCRS() ) );
    }

    if (( li->type() == QgsLegendItem::LEGEND_LAYER || li->type() == QgsLegendItem::LEGEND_GROUP ) && !groupEmbedded( li ) && !parentGroupEmbedded( li ) )
    {
      theMenu.addAction( tr( "Re&name" ), this, SLOT( openEditor() ) );
    }
  }

  theMenu.addAction( QgisApp::getThemeIcon( "/folder_new.png" ), tr( "&Add group" ), this, SLOT( addGroupToCurrentItem() ) );
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionExpandTree.png" ), tr( "&Expand all" ), this, SLOT( expandAll() ) );
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionCollapseTree.png" ), tr( "&Collapse all" ), this, SLOT( collapseAll() ) );

  theMenu.exec( position );
}

void QgsLegend::initPixmaps()
{
  mPixmaps.mOriginalPixmap = QgisApp::getThemePixmap( "/mActionFileSmall.png" );
  mPixmaps.mInOverviewPixmap = QgisApp::getThemePixmap( "/mActionInOverview.png" );
  mPixmaps.mEditablePixmap = QgisApp::getThemePixmap( "/mIconEditable.png" );
  mPixmaps.mProjectionErrorPixmap = QgisApp::getThemePixmap( "/mIconProjectionProblem.png" );
}

Qt::CheckState QgsLegend::layerCheckState( QgsMapLayer * layer )
{
  QgsLegendLayer * ll = findLegendLayer( layer );

  return ll ? ll->checkState( 0 ) : Qt::Unchecked;
}

QgsLegendGroup* QgsLegend::addEmbeddedGroup( const QString& groupName, const QString& projectFilePath, QgsLegendItem* parent )
{
  mEmbeddedGroups.insert( groupName, projectFilePath );

  //open project file, get layer ids in group, add the layers
  QFile projectFile( projectFilePath );
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return 0;
  }

  QDomDocument projectDocument;
  if ( !projectDocument.setContent( &projectFile ) )
  {
    return 0;
  }

  QDomElement legendElem = projectDocument.documentElement().firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return 0;
  }

  QList<QDomNode> brokenNodes;
  QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
  QSettings settings;

  QDomNodeList legendGroupList = legendElem.elementsByTagName( "legendgroup" );
  for ( int i = 0; i < legendGroupList.size(); ++i )
  {
    QDomElement legendElem = legendGroupList.at( i ).toElement();
    if ( legendElem.attribute( "name" ) == groupName )
    {
      //embedded groups cannot be embedded again
      if ( legendElem.attribute( "embedded" ) == "1" )
      {
        mEmbeddedGroups.remove( groupName );
        return 0;
      }

      QgsLegendGroup* group = 0;
      if ( parent )
      {
        group = new QgsLegendGroup( parent, groupName );
      }
      else
      {
        group = new QgsLegendGroup( this, groupName );
      }

      QFont groupFont;
      groupFont.setItalic( true );
      group->setFont( 0, groupFont );
      setCurrentItem( group );

      QDomNodeList groupChildren = legendElem.childNodes();
      for ( int j = 0; j < groupChildren.size(); ++j )
      {
        QDomElement childElem = groupChildren.at( j ).toElement();
        QString tagName = childElem.tagName();
        if ( tagName == "legendlayer" )
        {
          QString layerId = childElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" ).attribute( "layerid" );
          QgsProject::instance()->createEmbeddedLayer( layerId, projectFilePath, brokenNodes, vectorLayerList, false );
          QTreeWidgetItem* cItem = 0;
          if ( settings.value( "/qgis/addNewLayersToCurrentGroup", false ).toBool() )
          {
            cItem = group->takeChild( 0 );
          }
          else
          {
            cItem = currentItem();
            removeItem( cItem );
          }

          if( cItem )
          {
            group->insertChild( group->childCount(), cItem );
          }
        }
        else if ( tagName == "legendgroup" )
        {
          addEmbeddedGroup( childElem.attribute( "name" ), projectFilePath, group );
        }
      }
      checkLayerOrderUpdate();
      return group;
    }
  }
  return 0;
}

int QgsLegend::getItemPos( QTreeWidgetItem* item )
{
  int counter = 1;

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    if ( theItem == item )
    {
      return counter;
    }

    ++counter;
  }

  return -1;
}

void QgsLegend::addLayer( QgsMapLayer * layer )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsLegendLayer* llayer = new QgsLegendLayer( layer );
  if ( !QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
  {
    QFont itemFont;
    itemFont.setItalic( true );
    llayer->setFont( 0, itemFont );
  }

  //set the correct check states
  blockSignals( true );
  llayer->setCheckState( 0, llayer->isVisible() ? Qt::Checked : Qt::Unchecked );
  blockSignals( false );

  QgsLegendGroup *lg = dynamic_cast<QgsLegendGroup *>( currentItem() );
  QSettings settings;
  if ( lg && settings.value( "/qgis/addNewLayersToCurrentGroup", false ).toBool() )
  {
    lg->insertChild( 0, llayer );
  }
  else
  {
    insertTopLevelItem( 0, llayer );
    setCurrentItem( llayer );
  }

  setItemExpanded( llayer, true );
  //don't expand raster items by default, there could be too many
  refreshLayerSymbology( layer->id(), layer->type() != QgsMapLayer::RasterLayer );

  updateMapCanvasLayerSet();

  // first layer?
  if ( layers().count() == 1 )
  {
    mMapCanvas->zoomToFullExtent();
    mMapCanvas->clearExtentHistory();
  }
  //make the QTreeWidget item up-to-date
  doItemsLayout();
}

void QgsLegend::setLayerVisible( QgsMapLayer * layer, bool visible )
{
  QgsLegendLayer * ll = findLegendLayer( layer );
  if ( ll )
  {
    ll->setCheckState( 0, visible ? Qt::Checked : Qt::Unchecked );
  }
}

QgsLegendLayer* QgsLegend::currentLegendLayer()
{
  QgsLegendItem* citem = dynamic_cast<QgsLegendItem *>( currentItem() );

  if ( citem )
  {
    QgsLegendLayer* ll = qobject_cast<QgsLegendLayer *>( citem );
    if ( ll )
    {
      return ll; //the current item is a legend layer, so return it
    }

    QgsLegendLayer* lpl = dynamic_cast<QgsLegendLayer *>( citem->parent() );
    if ( lpl )
    {
      return lpl; //the parent of the current item is a legend layer, return it
    }
  }

  return 0;
}

QgsMapLayer* QgsLegend::currentLayer()
{
  QgsLegendLayer* ll = currentLegendLayer();
  if ( ll )
  {
    return ll->layer();
  }
  else
  {
    return 0;
  }
}

QList<QgsMapLayer *> QgsLegend::selectedLayers()
{
  QList<QgsMapLayer *> layers;

  foreach( QTreeWidgetItem * item, selectedItems() )
  {
    QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( item );
    if ( ll )
      layers << ll->layer();
  }

  return layers;
}

QList<QgsMapLayer *> QgsLegend::layers()
{
  QList< QgsMapLayer * > items;
  QTreeWidgetItemIterator it( this );
  while ( *it )
  {
    QgsLegendLayer *llayer = dynamic_cast<QgsLegendLayer *>( *it );
    if ( llayer )
      items.append( llayer->layer() );

    ++it;
  }

  return items;
}

bool QgsLegend::setCurrentLayer( QgsMapLayer *layer )
{
  QgsLegendLayer *ll = findLegendLayer( layer );
  if ( !ll )
    return false;

  setCurrentItem( ll );
  clearSelection();
  return true;
}

void QgsLegend::legendGroupRemove()
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsLegendGroup* lg = dynamic_cast<QgsLegendGroup *>( currentItem() );
  if ( lg )
  {
    removeGroup( lg );
  }
}

void QgsLegend::legendGroupSetCRS()
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem crs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );

    QgsLegendGroup* lg = dynamic_cast<QgsLegendGroup *>( currentItem() );
    setGroupCRS( lg, crs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  delete mySelector;
}

void QgsLegend::removeGroup( QgsLegendGroup *lg )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  //delete the legend layers first
  QTreeWidgetItem * child = lg->child( 0 );
  while ( child )
  {
    QgsLegendLayer *cl = dynamic_cast<QgsLegendLayer *>( child );
    QgsLegendGroup *cg = dynamic_cast<QgsLegendGroup *>( child );

    if ( cl )
      QgsMapLayerRegistry::instance()->removeMapLayer( cl->layer()->id() );
    else if ( cg )
      removeGroup( cg );

    child = lg->child( 0 );
  }

  delete lg;

  adjustIconSize();
}

void QgsLegend::setGroupCRS( QgsLegendGroup *lg, const QgsCoordinateReferenceSystem &crs )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  foreach( QgsLegendLayer *cl, lg->legendLayers() )
  {
    if ( cl )
    {
      cl->layer()->setCrs( crs );
    }
  }
}

void QgsLegend::moveLayer( QgsMapLayer *ml, int groupIndex )
{
  if ( !ml )
    return;

  QgsLegendLayer *layer = findLegendLayer( ml->id() );
  if ( !layer )
    return;

  QgsLegendGroup *group = dynamic_cast<QgsLegendGroup*>( topLevelItem( groupIndex ) );
  if ( !group )
    return;

  insertItem( layer, group );
}

void QgsLegend::legendLayerShowInOverview()
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsLegendItem* li = dynamic_cast<QgsLegendItem *>( currentItem() );
  if ( !li )
    return;

  if ( li->type() == QgsLegendItem::LEGEND_LAYER )
    return;

  QgsLegendLayer* ll = qobject_cast<QgsLegendLayer *>( li );
  if ( !ll )
    return;
  ll->showInOverview();
}

void QgsLegend::expandAll()
{
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    setExpanded( indexFromItem( theItem ), true );
  }
}

void QgsLegend::collapseAll()
{
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    setExpanded( indexFromItem( theItem ), false );
  }
}

bool QgsLegend::writeXML( QDomNode &legendnode, QDomDocument &document )
{
  QList<QTreeWidgetItem*> items;
  for ( int i = 0; i < topLevelItemCount(); i++ )
  {
    items << topLevelItem( i );
  }

  return writeXML( items, legendnode, document );
}

bool QgsLegend::writeXML( QList<QTreeWidgetItem *> items, QDomNode &node, QDomDocument &document )
{
  foreach( QTreeWidgetItem * currentItem, items )
  {
    QgsLegendItem *item = dynamic_cast<QgsLegendItem *>( currentItem );
    if ( !item )
      continue;

    if ( item->type() == QgsLegendItem::LEGEND_GROUP )
    {
      QDomElement legendgroupnode = document.createElement( "legendgroup" );
      legendgroupnode.setAttribute( "open", isItemExpanded( item ) ? "true" : "false" );
      legendgroupnode.setAttribute( "name", item->text( 0 ) );
      Qt::CheckState cstate = item->checkState( 0 );
      if ( cstate == Qt::Checked )
      {
        legendgroupnode.setAttribute( "checked", "Qt::Checked" );
      }
      else if ( cstate == Qt::Unchecked )
      {
        legendgroupnode.setAttribute( "checked", "Qt::Unchecked" );
      }
      else if ( cstate == Qt::PartiallyChecked )
      {
        legendgroupnode.setAttribute( "checked", "Qt::PartiallyChecked" );
      }

      QHash< QString, QString >::const_iterator embedIt = mEmbeddedGroups.find( item->text( 0 ) );
      if ( embedIt != mEmbeddedGroups.constEnd() )
      {
        legendgroupnode.setAttribute( "embedded", 1 );
        legendgroupnode.setAttribute( "project", embedIt.value() );
      }
      else
      {
        QList<QTreeWidgetItem *> children;
        for ( int i = 0; i < currentItem->childCount(); i++ )
        {
          children << currentItem->child( i );
        }

        writeXML( children, legendgroupnode, document );
      }
      node.appendChild( legendgroupnode );
    }
    else if ( item->type() == QgsLegendItem::LEGEND_LAYER )
    {
      QDomElement legendlayernode = document.createElement( "legendlayer" );
      legendlayernode.setAttribute( "open", isItemExpanded( item ) ? "true" : "false" );

      Qt::CheckState cstate = item->checkState( 0 );
      if ( cstate == Qt::Checked )
      {
        legendlayernode.setAttribute( "checked", "Qt::Checked" );
      }
      else if ( cstate == Qt::Unchecked )
      {
        legendlayernode.setAttribute( "checked", "Qt::Unchecked" );
      }
      else if ( cstate == Qt::PartiallyChecked )
      {
        legendlayernode.setAttribute( "checked", "Qt::PartiallyChecked" );
      }
      legendlayernode.setAttribute( "name", item->text( 0 ) );

      for ( int i = 0; i < item->childCount(); i++ )
      {
        QTreeWidgetItem *child = item->child( i );
        QgsLegendItem *litem = dynamic_cast<QgsLegendItem *>( child );

        if ( !litem )
        {
          QgsDebugMsg( "tree widget item not a legend item" );
          continue;
        }

        if ( litem->type() == QgsLegendItem::LEGEND_PROPERTY_GROUP )
        {
          QDomElement legendpropertynode = document.createElement( "propertygroup" );
          legendpropertynode.setAttribute( "open", isItemExpanded( item ) ? "true" : "false" );
          legendlayernode.appendChild( legendpropertynode );
        }
        else if ( litem->type() == QgsLegendItem::LEGEND_SYMBOL_GROUP )
        {
          QDomElement legendsymbolnode = document.createElement( "symbolgroup" );
          legendsymbolnode.setAttribute( "open", isItemExpanded( item ) ? "true" : "false" );
          legendlayernode.appendChild( legendsymbolnode );
        }
        else
        {
          QgsDebugMsg( "unexpected legend item type " + QString::number( litem->type() ) );
        }
      }

      node.appendChild( legendlayernode );

      // save the information about layer
      // emulate a legend layer file group and a legend layer file
      // to keep it compatible with older projects
      QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( item );
      QgsMapLayer* layer = ll->layer();
      legendlayernode.setAttribute( "showFeatureCount", ll->showFeatureCount() );

      QDomElement layerfilegroupnode = document.createElement( "filegroup" );
      layerfilegroupnode.setAttribute( "open", isItemExpanded( item ) ? "true" : "false" );
      layerfilegroupnode.setAttribute( "hidden", isItemHidden( item ) ? "true" : "false" );
      legendlayernode.appendChild( layerfilegroupnode );

      QDomElement legendlayerfilenode = document.createElement( "legendlayerfile" );

      // layer id
      legendlayerfilenode.setAttribute( "layerid", layer->id() );
      layerfilegroupnode.appendChild( legendlayerfilenode );

      //embedded layer?
      if ( !QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
      {
        legendlayerfilenode.setAttribute( "embedded", "1" );
      }

      // visible flag
      legendlayerfilenode.setAttribute( "visible", ll->isVisible() );

      // show in overview flag
      legendlayerfilenode.setAttribute( "isInOverview", ll->isInOverview() );
    }
    else
    {
      QgsDebugMsg( "unexpected legend item type " + QString::number( item->type() ) );
    }
  }

  return true;
}

bool QgsLegend::readXML( QgsLegendGroup *parent, const QDomNode &node )
{
  const QDomNodeList &l = node.childNodes();
  for ( int i = 0; i < l.count(); i++ )
  {
    QDomNode child = l.at( i );
    QDomElement childelem = child.toElement();
    QString name = childelem.attribute( "name" );

    //test every possibility of element...
    if ( childelem.tagName() == "legendgroup" )
    {
      QgsLegendGroup* theGroup = 0;
      if ( childelem.attribute( "embedded" ) == "1" )
      {
        theGroup = addEmbeddedGroup( name, childelem.attribute( "project" ) );
      }
      else
      {
        if ( parent )
          theGroup = new QgsLegendGroup( parent, name );
        else
          theGroup = new QgsLegendGroup( this, name );
      }

      if ( !theGroup )
      {
        continue;
      }

      //set the checkbox of the legend group to the right state
      blockSignals( true );
      QString checked = childelem.attribute( "checked" );
      if ( checked == "Qt::Checked" )
      {
        theGroup->setCheckState( 0, Qt::Checked );
      }
      else if ( checked == "Qt::Unchecked" )
      {
        theGroup->setCheckState( 0, Qt::Unchecked );
      }
      else if ( checked == "Qt::PartiallyChecked" )
      {
        theGroup->setCheckState( 0, Qt::PartiallyChecked );
      }
      blockSignals( false );

      readXML( theGroup, child );

      if ( childelem.attribute( "open" ) == "true" )
      {
        expandItem( theGroup );
      }
      else
      {
        collapseItem( theGroup );
      }
    }
    else if ( childelem.tagName() == "legendlayer" )
    {
      bool isOpen;
      QgsLegendLayer* currentLayer = readLayerFromXML( childelem, isOpen );
      if ( !currentLayer )
      {
        continue;
      }

      // add to tree - either as a top-level node or a child of a group
      if ( parent )
      {
        parent->addChild( currentLayer );
      }
      else
      {
        addTopLevelItem( currentLayer );
      }

      const QDomNodeList &cnl = child.childNodes();
      for ( int j = 0; j < cnl.count(); j++ )
      {
        const QDomElement &childelem = cnl.at( j ).toElement();

        if ( childelem.tagName() == "legendlayerfile" )
        {
          // do nothing, this has been handled in readLayerFromXML()
        }
        else if ( childelem.tagName() == "filegroup" )
        {
          // do nothing, this has been handled in readLayerFromXML()
        }
        else if ( childelem.tagName() == "propertygroup" )
        {
          QgsLegendPropertyGroup* thePropertyGroup = new QgsLegendPropertyGroup( currentLayer, "Properties" );
          setItemExpanded( thePropertyGroup, childelem.attribute( "open" ) == "true" );
        }
        else
        {
          QgsDebugMsg( "unexpected legendlayer child " + childelem.tagName() );
        }
      }

      // load symbology
      refreshLayerSymbology( currentLayer->layer()->id() );

      if ( isOpen )
      {
        expandItem( currentLayer );
      }
      else
      {
        collapseItem( currentLayer );
      }
    }
    else
    {
      QgsDebugMsg( "unexpected legend child " + childelem.tagName() );
    }
  }

  return true;
}

bool QgsLegend::readXML( QDomNode& legendnode )
{
  clear(); //remove all items first
  mEmbeddedGroups.clear();
  return readXML( 0, legendnode );
}

QgsLegendLayer* QgsLegend::readLayerFromXML( QDomElement& childelem, bool& isOpen )
{
  QDomElement fileGroupElem = childelem.firstChildElement( "filegroup" );
  if ( fileGroupElem.isNull() )
    return NULL; // we need a file group!

  QDomElement fileElem = fileGroupElem.firstChildElement( "legendlayerfile" );
  if ( fileElem.isNull() ) // we need a file element!
    return NULL;

  QgsMapLayer* theMapLayer = QgsMapLayerRegistry::instance()->mapLayer( fileElem.attribute( "layerid" ) );
  if ( theMapLayer == NULL ) //the layer cannot be found (e.g. the file has been moved)
  {
    return NULL;
  }

  // create the item
  QgsLegendLayer* ll = new QgsLegendLayer( theMapLayer );
  ll->setShowFeatureCount( childelem.attribute( "showFeatureCount", "0" ).toInt(), false );

  // load layer's visibility and 'show in overview' flag
  ll->setInOverview( atoi( fileElem.attribute( "isInOverview" ).toUtf8() ) );

  // expanded or collapsed
  isOpen = childelem.attribute( "open" ) == "true";

  //set the checkbox of the legend layer to the right state
  blockSignals( true );
  QString checked = childelem.attribute( "checked", "Qt::Checked" ); // Default is to show
  if ( checked == "Qt::Checked" )
  {
    ll->setVisible( true );
    ll->setCheckState( 0, Qt::Checked );
  }
  else if ( checked == "Qt::Unchecked" )
  {
    ll->setVisible( false );
    ll->setCheckState( 0, Qt::Unchecked );
  }
  blockSignals( false );

  return ll;
}


QgsLegendLayer* QgsLegend::findLegendLayer( const QString& layerKey )
{
  QgsLegendLayer* theLegendLayer = 0;
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    theLegendLayer = dynamic_cast<QgsLegendLayer *>( theItem );
    if ( theLegendLayer ) //item is a legend layer
    {
      if ( theLegendLayer->layer()->id() == layerKey )
      {
        return theLegendLayer;
      }
    }
  }

  return 0;
}

QgsLegendLayer* QgsLegend::findLegendLayer( const QgsMapLayer *layer )
{
  QgsLegendLayer* theLegendLayer = 0;
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    theLegendLayer = dynamic_cast<QgsLegendLayer *>( theItem );
    if ( theLegendLayer ) //item is a legend layer
    {
      if ( theLegendLayer->layer() == layer )
      {
        return theLegendLayer;
      }
    }
  }

  return 0;
}


void QgsLegend::adjustIconSize()
{
  if ( mPixmapWidthValues.size() > 0 && mPixmapHeightValues.size() > 0 )
  {
    std::multiset<int>::const_reverse_iterator width_it = mPixmapWidthValues.rbegin();
    std::multiset<int>::const_reverse_iterator height_it = mPixmapHeightValues.rbegin();
    int maxWidth = *width_it;
    int maxHeight = *height_it;

    QSize currentIconSize = iconSize();
    if ( maxWidth == currentIconSize.width() && maxHeight == currentIconSize.height() )
    {
      //no resizing necessary
      return;
    }

    //keep the minimum size
    if ( maxWidth < mMinimumIconSize.width() )
    {
      maxWidth = mMinimumIconSize.width();
    }
    if ( maxHeight < mMinimumIconSize.height() )
    {
      maxHeight = mMinimumIconSize.height();
    }

    setIconSize( QSize( maxWidth, maxHeight ) );
  }
}

bool QgsLegend::yCoordAboveCenter( QgsLegendItem* it, int ycoord )
{
  QRect rect = visualItemRect( it );
  int height = rect.height();
  int top = rect.top();
  int mid = top + ( height / 2 );
  if ( ycoord > mid ) //bottom, remember the y-coordinate increases downwards
  {
    return false;
  }
  else//top
  {
    return true;
  }
}

bool QgsLegend::isLegendGroup( const QModelIndex &index )
{
  return dynamic_cast<QgsLegendGroup *>( itemFromIndex( index ) );
}

QStringList QgsLegend::groups()
{
  QStringList groupList;
  QTreeWidgetItem *current = firstItem();

  while ( current )
  {
    QgsLegendGroup *group = dynamic_cast<QgsLegendGroup *>( current );
    if ( group )
    {
      groupList.append( group->text( 0 ) );
    }

    current = nextItem( current );
  }

  return groupList;
}

QList< GroupLayerInfo > QgsLegend::groupLayerRelationship()
{
  QList< GroupLayerInfo > groupLayerList;

  QList< QTreeWidgetItem * > items;

  for ( int i = 0; i < topLevelItemCount(); i++ )
  {
    items << topLevelItem( i );
  }

  while ( !items.isEmpty() )
  {
    QTreeWidgetItem *currentItem = items.takeFirst();

    QgsLegendLayer* lLayer = dynamic_cast<QgsLegendLayer*>( currentItem );
    if ( lLayer )
    {
      if ( lLayer->layer() )
      {
        QList<QString> layerList;
        layerList.push_back( lLayer->layer()->id() );
        groupLayerList.push_back( qMakePair( QString(), layerList ) );
      }
    }

    QgsLegendGroup* lGroup = dynamic_cast<QgsLegendGroup*>( currentItem );
    if ( lGroup )
    {
      int nLayers = lGroup->childCount();
      QList<QString> layerList;
      for ( int i = 0; i < nLayers; ++i )
      {
        QTreeWidgetItem *gItem = lGroup->child( i );

        QgsLegendLayer* lLayer = dynamic_cast<QgsLegendLayer*>( gItem );
        if ( lLayer )
        {
          if ( lLayer->layer() )
          {
            layerList.push_back( lLayer->layer()->id() );
          }
        }

        QgsLegendGroup* lGroup = dynamic_cast<QgsLegendGroup*>( gItem );
        if ( lGroup )
        {
          layerList << lGroup->text( 0 );
          items << lGroup;
        }
      }

      groupLayerList.push_back( qMakePair( lGroup->text( 0 ), layerList ) );
    }
  }

  return groupLayerList;
}

QTreeWidgetItem* QgsLegend::firstItem()
{
  return topLevelItem( 0 );
}

/**Returns the next item (next sibling or next item on level above)*/
QTreeWidgetItem* QgsLegend::nextItem( QTreeWidgetItem* item )
{
  QgsLegendItem* litem = dynamic_cast<QgsLegendItem *>( item );
  if ( !litem )
  {
    return 0;
  }
  else if ( litem->childCount() > 0 )
  {
    return litem->child( 0 );
  }
  else if ( litem->nextSibling() )
  {
    return litem->nextSibling();
  }
  else if ( litem->parent() )
  {
    QTreeWidgetItem *parent = litem->parent();

    while ( parent )
    {
      QgsLegendItem *sibling = dynamic_cast<QgsLegendItem *>( parent )->nextSibling();

      if ( sibling )
        return sibling;

      parent = parent->parent();
    }
  }

  return 0;
}

QTreeWidgetItem* QgsLegend::nextSibling( QTreeWidgetItem* item )
{
  QModelIndex thisidx = indexFromItem( item );
  QModelIndex nextsidx = thisidx.sibling( thisidx.row() + 1, thisidx.column() );
  if ( nextsidx.isValid() )
  {
    return dynamic_cast<QgsLegendItem *>( itemFromIndex( nextsidx ) );
  }
  else
  {
    return 0;
  }
}

QTreeWidgetItem* QgsLegend::previousSibling( QTreeWidgetItem* item )
{
  QModelIndex thisidx = indexFromItem( item );
  QgsDebugMsg( "prev sibling for row: " + QString::number( thisidx.row() ) );
  QModelIndex prevsidx = thisidx.sibling( thisidx.row() - 1, thisidx.column() );
  QgsDebugMsg( "row: " + QString::number( prevsidx.row() ) );
  if ( prevsidx.isValid() )
  {
    return dynamic_cast<QgsLegendItem *>( itemFromIndex( prevsidx ) );
  }
  else
  {
    return 0;
  }
}

void QgsLegend::insertItem( QTreeWidgetItem* move, QTreeWidgetItem* into )
{
  QgsLegendItem* movedItem = dynamic_cast<QgsLegendItem *>( move );
  QgsLegendItem* intoItem = dynamic_cast<QgsLegendItem *>( into );

  if ( movedItem && intoItem )
  {
    QgsLegendItem* parentItem = dynamic_cast<QgsLegendItem *>( movedItem->parent() );
    movedItem->storeAppearanceSettings();//store settings in the moved item and its children
    removeItem( movedItem );
    intoItem->insert( movedItem );
    if ( parentItem )
    {
      parentItem->release( movedItem ); //give the former parent item the possibility to do cleanups
    }
    intoItem->receive( movedItem );
    movedItem->restoreAppearanceSettings();//apply the settings again
  }
}

void QgsLegend::moveItem( QTreeWidgetItem* move, QTreeWidgetItem* after )
{
  QgsDebugMsg( QString( "Moving layer : %1 (%2)" ).arg( move->text( 0 ) ).arg( move->type() ) );
  if ( after )
    QgsDebugMsg( QString( "after layer  : %1 (%2)" ).arg( after->text( 0 ) ).arg( after->type() ) );
  else
    QgsDebugMsg( "as toplevel item" );

  static_cast<QgsLegendItem*>( move )->storeAppearanceSettings();//store settings in the moved item and its childern

  if ( move->parent() )
  {
    move->parent()->takeChild( move->parent()->indexOfChild( move ) );
  }
  else //move is toplevel item
  {
    takeTopLevelItem( indexOfTopLevelItem( move ) );
  }

  if ( after )
  {
    if ( after->parent() )
    {
      after->parent()->insertChild( after->parent()->indexOfChild( after ) + 1, move );
    }
    else //toplevel item
    {
      insertTopLevelItem( indexOfTopLevelItem( after ) + 1, move );
    }
  }
  else
  {
    insertTopLevelItem( 0, move );
  }

  static_cast<QgsLegendItem*>( move )->restoreAppearanceSettings();//apply the settings again
}

void QgsLegend::removeItem( QTreeWidgetItem* item )
{
  if ( item->parent() )
  {
    item->parent()->takeChild( item->parent()->indexOfChild( item ) );
  }
  else
  {
    takeTopLevelItem( indexOfTopLevelItem( item ) );
  }
}

void QgsLegend::updateMapCanvasLayerSet()
{
  QList<QgsMapCanvasLayer> layers;

  // create list of the layers
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem *li = dynamic_cast<QgsLegendItem *>( theItem );
    QgsLegendLayer *ll = qobject_cast<QgsLegendLayer *>( li );
    if ( ll )
    {
      QgsMapCanvasLayer& lyr = ll->canvasLayer();
      layers.append( lyr );
    }
  }

  // set layers in canvas
  mMapCanvas->setLayerSet( layers );
}

void QgsLegend::updateOverview()
{
  mMapCanvas->updateOverview();
}

void QgsLegend::enableOverviewModeAllLayers( bool isInOverview )
{
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer *>( theItem );
    if ( ll )
    {
      ll->setInOverview( isInOverview );
    }

  }
  updateMapCanvasLayerSet();
  updateOverview();
}

QStringList QgsLegend::layerIDs()
{
  QStringList layers;

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem *li = dynamic_cast<QgsLegendItem *>( theItem );
    QgsLegendLayer* ll = qobject_cast<QgsLegendLayer *>( li );
    if ( ll )
    {
      QgsMapLayer *lyr = ll->layer();
      layers.push_front( lyr->id() );
    }
  }

#ifdef QGISDEBUG
  QgsDebugMsg( "QgsLegend::layerIDs()" );
  foreach( QString id, layers )
  {
    QgsDebugMsg( id );
  }
#endif

  return layers;
}


void QgsLegend::refreshLayerSymbology( QString key, bool expandItem )
{
  QgsLegendLayer* theLegendLayer = findLegendLayer( key );
  if ( !theLegendLayer )
  {
    return;
  }

  //store the current item
  QModelIndex currentItemIndex( currentIndex() );

  double widthScale = 1.0;
  if ( mMapCanvas && mMapCanvas->map() )
  {
    widthScale = mMapCanvas->map()->paintDevice().logicalDpiX() / 25.4;
  }

  theLegendLayer->refreshSymbology( key, widthScale );

  //restore the current item again
  setCurrentIndex( currentItemIndex );
  adjustIconSize();
  setItemExpanded( theLegendLayer, expandItem );//make sure the symbology items are visible
}


void QgsLegend::addPixmapWidthValue( int width )
{
  mPixmapWidthValues.insert( width );
}

void QgsLegend::addPixmapHeightValue( int height )
{
  mPixmapHeightValues.insert( height );
}

void QgsLegend::removePixmapWidthValue( int width )
{
  std::multiset<int>::iterator it = mPixmapWidthValues.find( width );
  if ( it != mPixmapWidthValues.end() )
  {
    mPixmapWidthValues.erase( it );
  }
  //todo: adapt the icon size if width is the largest value and the size of the next element is higher than the minimum
}

void QgsLegend::removePixmapHeightValue( int height )
{
  std::multiset<int>::iterator it = mPixmapHeightValues.find( height );
  if ( it != mPixmapHeightValues.end() )
  {
    mPixmapHeightValues.erase( height );
  }
  //todo: adapt the icon size if height is the largest value and the size of the next element is higher than the minimum
}


void QgsLegend::handleItemChange( QTreeWidgetItem* item, int column )
{
  if ( !item )
  {
    return;
  }

  QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( item );
  QgsLegendGroup *lg = dynamic_cast<QgsLegendGroup *>( item );

  if ( !ll && !lg )
  {
    return;
  }

#ifdef QGISDEBUG
  if ( item->checkState( 0 ) == Qt::Checked )
    showItem( "handleItemChange[checked]", item );
  else if ( item->checkState( 0 ) == Qt::Unchecked )
    showItem( "handleItemChange[unchecked]", item );
  else if ( item->checkState( 0 ) == Qt::PartiallyChecked )
    showItem( "handleItemChange[partially]", item );
  else
    showItem( "handleItemChange[?]", item );
#endif

  if ( ll )
  {
    //if the text of a QgsLegendLayer has changed, change the display names of all its maplayers
    // TODO: is this still necessary?
    ll->layer()->setLayerName( ll->text( 0 ) );
  }

  bool renderFlagState = false;
  bool changing = mChanging;
  mChanging = true;

  if ( !changing )
  {
    renderFlagState = mMapCanvas->renderFlag();
    if ( renderFlagState )
      mMapCanvas->setRenderFlag( false );

    if ( item->isSelected() )
    {
      foreach( QTreeWidgetItem * i, selectedItems() )
      {
        if ( i != item )
        {
          i->setCheckState( 0, item->checkState( 0 ) );
        }
      }
    }
  }

  if ( ll )
  {
    ll->setVisible( ll->checkState( 0 ) == Qt::Checked );
  }

  if ( lg && lg->checkState( 0 ) != Qt::PartiallyChecked )
  {
    Qt::CheckState theState = lg->checkState( 0 );
    for ( int i = 0; i < item->childCount(); i++ )
    {
      QTreeWidgetItem *child = item->child( i );
      if ( child->checkState( 0 ) != item->checkState( 0 ) )
        child->setCheckState( 0, theState );
    }
  }

  // propagate updates to upper groups
  for (
    QgsLegendGroup *plg = dynamic_cast<QgsLegendGroup *>( item->parent() );
    plg;
    plg = dynamic_cast<QgsLegendGroup *>( plg->parent() )
  )
  {
    Qt::CheckState theState = plg->pendingCheckState();

    if ( theState != plg->checkState( 0 ) )
    {
      blockSignals( true );
      plg->setCheckState( 0, theState );
      blockSignals( false );
    }
  }

  if ( !changing )
  {
    // update layer set
    updateMapCanvasLayerSet();

    // If it was on, turn it back on, otherwise leave it
    // off, as turning it on causes a refresh.
    if ( renderFlagState )
      mMapCanvas->setRenderFlag( true );
  }

  mChanging = changing;
}

void QgsLegend::openEditor()
{
  QTreeWidgetItem* theItem = currentItem();
  if ( theItem )
  {
    if ( !groupEmbedded( theItem ) && !parentGroupEmbedded( theItem ) )
    {
      editItem( theItem, 0 );
    }
  }
}

void QgsLegend::makeToTopLevelItem()
{
  QgsLegendItem* theItem = dynamic_cast<QgsLegendItem *>( currentItem() );
  if ( theItem )
  {
    if ( !parentGroupEmbedded( theItem ) )
    {
      theItem->storeAppearanceSettings();
      removeItem( theItem );
      addTopLevelItem( theItem );
      theItem->restoreAppearanceSettings();
    }
  }
}


void QgsLegend::legendLayerZoom()
{
  if ( !mMapCanvas )
  {
    return;
  }

  QgsRectangle extent;

  QgsLegendItem* li = dynamic_cast<QgsLegendItem *>( currentItem() );

  if ( !li )
    return;

  if ( li->type() == QgsLegendItem::LEGEND_LAYER )
  {
    QgsLegendLayer* currentLayer = dynamic_cast<QgsLegendLayer *>( currentItem() );
    if ( !currentLayer )
      return;

    QgsMapLayer* theLayer = currentLayer->layer();
    extent = theLayer->extent();

    //transform extent if otf-projection is on
    if ( mMapCanvas->hasCrsTransformEnabled() )
    {
      QgsMapRenderer* renderer = mMapCanvas->mapRenderer();
      if ( renderer )
      {
        extent = renderer->layerExtentToOutputExtent( theLayer, extent );
      }
    }
  }
  else if ( li->type() == QgsLegendItem::LEGEND_GROUP )
  {
    QgsLegendGroup* currentGroup = dynamic_cast<QgsLegendGroup *>( currentItem() );

    QgsRectangle layerExtent;

    QList<QgsLegendLayer*> layers = currentGroup->legendLayers();
    for ( int i = 0; i < layers.size(); ++i )
    {
      QgsMapLayer* theLayer = layers.at( i )->layer();
      layerExtent = theLayer->extent();

      //transform extent if otf-projection is on
      if ( mMapCanvas->hasCrsTransformEnabled() )
      {
        QgsMapRenderer* renderer = mMapCanvas->mapRenderer();
        if ( renderer )
        {
          layerExtent = renderer->layerExtentToOutputExtent( theLayer, layerExtent );
        }
      }

      if ( i == 0 )
      {
        extent = layerExtent;
      }
      else
      {
        extent.combineExtentWith( &layerExtent );
      }
    }
  }

  if ( extent.isEmpty() )
  {
    return;
  }

  // Increase bounding box with 5%, so that layer is a bit inside the borders
  extent.scale( 1.05 );

  //zoom to bounding box
  mMapCanvas->setExtent( extent );
  mMapCanvas->refresh();
}

void QgsLegend::legendLayerZoomNative()
{
  //find current Layer
  QgsLegendLayer* currentLayer = dynamic_cast<QgsLegendLayer *>( currentItem() );
  if ( !currentLayer )
    return;

  QgsRasterLayer *layer =  qobject_cast<QgsRasterLayer *>( currentLayer->layer() );
  if ( layer )
  {
    QgsDebugMsg( "Raster units per pixel  : " + QString::number( layer->rasterUnitsPerPixel() ) );
    QgsDebugMsg( "MapUnitsPerPixel before : " + QString::number( mMapCanvas->mapUnitsPerPixel() ) );

    layer->setCacheImage( NULL );
    if ( mMapCanvas->hasCrsTransformEnabled() )
    {
      // get legth of central canvas pixel width in source raster crs
      QgsRectangle e = mMapCanvas->extent();
      QgsMapRenderer* r = mMapCanvas->mapRenderer();
      QgsPoint p1( e.center().x(), e.center().y() );
      QgsPoint p2( e.center().x() + e.width() / r->width(), e.center().y() + e.height() / r->height() );
      QgsCoordinateTransform ct( r->destinationCrs(), layer->crs() );
      p1 = ct.transform( p1 );
      p2 = ct.transform( p2 );
      double width = sqrt( p1.sqrDist( p2 ) ); // width of reprojected pixel
      // This is not perfect of course, we use the resolution in just one direction
      mMapCanvas->zoomByFactor( qAbs( layer->rasterUnitsPerPixel() / width ) );
    }
    else
    {
      mMapCanvas->zoomByFactor( qAbs( layer->rasterUnitsPerPixel() / mMapCanvas->mapUnitsPerPixel() ) );
    }
    mMapCanvas->refresh();
    QgsDebugMsg( "MapUnitsPerPixel after  : " + QString::number( mMapCanvas->mapUnitsPerPixel() ) );
  }
}

void QgsLegend::legendLayerStretchUsingCurrentExtent()
{
  //find current Layer
  QgsLegendLayer* currentLayer = dynamic_cast<QgsLegendLayer *>( currentItem() );
  if ( !currentLayer )
    return;

  QgsRasterLayer *layer =  qobject_cast<QgsRasterLayer *>( currentLayer->layer() );
  if ( layer )
  {
    // Note: Do we really want to do these next clauses? The user will get a surprise when the
    // drawing style they are using suddenly changes....! TS
    if ( layer->drawingStyle() == QgsRasterLayer::SingleBandPseudoColor )
    {
      layer->setDrawingStyle( QgsRasterLayer::SingleBandGray );
    }
    else if ( layer->drawingStyle() == QgsRasterLayer::MultiBandSingleBandPseudoColor )
    {
      layer->setDrawingStyle( QgsRasterLayer::MultiBandSingleBandGray );
    }

    if ( layer->contrastEnhancementAlgorithmAsString() == "NoEnhancement" )
    {
      layer->setContrastEnhancementAlgorithm( "StretchToMinimumMaximum" );
    }

    layer->setMinimumMaximumUsingLastExtent();
    layer->setCacheImage( NULL );
    refreshLayerSymbology( layer->id() );
    mMapCanvas->refresh();
  }
}

void QgsLegend::readProject( const QDomDocument & doc )
{
  QDomNodeList nodes = doc.elementsByTagName( "legend" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    readXML( node );

    // update canvas layers to match the order defined by legend
    updateMapCanvasLayerSet();
  }
  else
  {
    QgsDebugMsg( "Couldn't read legend information from project" );
  }
}

void QgsLegend::writeProject( QDomDocument & doc )
{
  QDomNodeList nl = doc.elementsByTagName( "qgis" );
  if ( !nl.count() )
  {
    QgsDebugMsg( "Unable to find qgis element in project file" );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  QDomElement mapcanvasNode = doc.createElement( "legend" );
  qgisNode.appendChild( mapcanvasNode );
  writeXML( mapcanvasNode, doc );
}


bool QgsLegend::checkLayerOrderUpdate()
{
  QStringList layersAfterRelease = layerIDs(); //test if canvas redraw is really necessary
  if ( layersAfterRelease != mLayersPriorToMove )
  {
    // z-order has changed - update layer set
    updateMapCanvasLayerSet();
    emit zOrderChanged();
    return true;
  }
  return false;
}

void QgsLegend::hideLine()
{
  mInsertionLine->setGeometry( 0, -100, 1, 1 );
}

void QgsLegend::showLine( int y, int left )
{
  mInsertionLine->setGeometry( left, y, viewport()->width(), 2 );
}

void QgsLegend::updateLineWidget()
{
  mInsertionLine->repaint();
}


QTreeWidgetItem * QgsLegend::lastVisibleItem()
{
  QTreeWidgetItem *current;
  QTreeWidgetItem *next;

  current = topLevelItem( topLevelItemCount() - 1 );
  while (( next = itemBelow( current ) ) )
  {
    current = next;
  }
  return current;
}

void QgsLegend::refreshCheckStates()
{
  if ( !mMapCanvas )
  {
    return;
  }

  QList<QgsMapLayer*> lst = mMapCanvas->layers();
  for ( QTreeWidgetItem* item = firstItem(); item; item = nextItem( item ) )
  {
    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer *>( item );
    if ( ll )
    {
      ll->setCheckState( 0, lst.contains( ll->layer() ) ? Qt::Checked : Qt::Unchecked );
    }
  }
}

void QgsLegend::removeSelectedLayers()
{
  // Turn off rendering to improve speed.
  bool renderFlagState = mMapCanvas->renderFlag();
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( false );

  foreach( QTreeWidgetItem * item, selectedItems() )
  {
    QgsLegendGroup* lg = dynamic_cast<QgsLegendGroup *>( item );
    if ( lg )
    {
      removeGroup( lg );
      continue;
    }

    QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( item );
    if ( ll && ll->layer() )
    {
      QgsMapLayerRegistry::instance()->removeMapLayer( ll->layer()->id() );
      continue;
    }
  }

  // Turn on rendering (if it was on previously)
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( true );
}

void QgsLegend::setCRSForSelectedLayers( const QgsCoordinateReferenceSystem &crs )
{
  // Turn off rendering to improve speed.
  bool renderFlagState = mMapCanvas->renderFlag();
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( false );

  foreach( QTreeWidgetItem * item, selectedItems() )
  {
    QgsLegendGroup* lg = dynamic_cast<QgsLegendGroup *>( item );
    if ( lg )
    {
      setGroupCRS( lg, crs );
      continue;
    }

    QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( item );
    if ( ll && ll->layer() )
    {
      ll->layer()->setCrs( crs );
      continue;
    }
  }

  // Turn on rendering (if it was on previously)
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( true );
}

bool QgsLegend::parentGroupEmbedded( QTreeWidgetItem* item ) const
{
  if ( !item )
  {
    return false;
  }

  QgsLegendItem* lItem = dynamic_cast<QgsLegendItem*>( item );
  if ( lItem && lItem->parent() )
  {
    QgsLegendGroup* parentGroup = dynamic_cast<QgsLegendGroup*>( lItem->parent() );
    if ( parentGroup && parentGroup->type() == QgsLegendItem::LEGEND_GROUP
         && mEmbeddedGroups.contains( parentGroup->text( 0 ) ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsLegend::groupEmbedded( QTreeWidgetItem* item ) const
{
  if ( !item )
  {
    return false;
  }

  QgsLegendGroup* gItem = dynamic_cast<QgsLegendGroup*>( item );
  if ( !gItem )
  {
    return false;
  }

  return mEmbeddedGroups.contains( gItem->text( 0 ) );
}
