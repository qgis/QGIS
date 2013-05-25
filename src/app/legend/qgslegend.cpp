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
#include "qgsclipboard.h"
#include "qgsmessagelog.h"

#include <QFont>
#include <QDomDocument>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QTreeWidgetItem>
#include <QClipboard>

const int AUTOSCROLL_MARGIN = 16;

// This function finds a unique group name [prefix1, prefix2, ...] by adding an
// incremental integer to prefix. It is necessary because group names are the
// only way of identifying groups in QgsLegendInterface.
// Could add a "parent" argument and use that instead, or pass it as prefix
static QString getUniqueGroupName( QString prefix, QStringList groups )
{
  QString suffix;
  if ( groups.size() == 0 )
  {
    suffix = "1";
  }
  else
  {
    // get a list of strings that match prefix, and keep the suffix
    QStringList match = groups.filter( QRegExp( QString( "^" + prefix ) ) );
    match.replaceInStrings( prefix, QString( "" ) );
    // find the maximum
    int max = 0;
    foreach ( QString m, match )
    {
      if ( m.toInt() > max )
        max = m.toInt();
    }
    suffix = QString( "%1" ).arg( max + 1 );
  }
  return prefix + suffix;
}

QgsLegend::QgsLegend( QgsMapCanvas *canvas, QWidget * parent, const char *name )
    : QTreeWidget( parent )
    , mMousePressedFlag( false )
    , mMapCanvas( canvas )
    , mMinimumIconSize( 20, 20 )
    , mChanging( false )
    , mUpdateDrawingOrder( true )
{
  setObjectName( name );

  connect( this, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
           this, SLOT( handleItemChange( QTreeWidgetItem*, int ) ) );

  connect( itemDelegate(), SIGNAL( closeEditor( QWidget *, QAbstractItemDelegate::EndEditHint ) ),
           this, SLOT( handleCloseEditor( QWidget *, QAbstractItemDelegate::EndEditHint ) ) );

  connect( this, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
           this, SLOT( handleCurrentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  // project handling
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );

  // connect map layer registry signal to legend
  connect( QgsMapLayerRegistry::instance(),
           SIGNAL( layersWillBeRemoved( QStringList ) ),
           this, SLOT( removeLayers( QStringList ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( removeAll() ),
           this, SLOT( removeAll() ) );
  connect( QgsMapLayerRegistry::instance(),
           SIGNAL( legendLayersAdded( QList<QgsMapLayer*> ) ),
           this, SLOT( addLayers( QList<QgsMapLayer *> ) ) );

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

  QgsDebugMsgLevel( QString( "%1: %2 %3 row:%4" ).arg( msg ).arg( type ).arg( item->text( 0 ) ).arg( indexFromItem( item ).row() ), 3 );
}
#else
#define showItem(msg, item)
#endif


void QgsLegend::handleCurrentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
  Q_UNUSED( current );
  Q_UNUSED( previous );
  QgsMapLayer *layer = currentLayer();

  if ( mMapCanvas )
  {
    mMapCanvas->setCurrentLayer( layer );
  }

  blockSignals( true ); // or itemChanged() emitted
  foreach ( QgsLegendLayer *ll, legendLayers() )
  {
    QFont itemFont = ll->font( 0 );
    itemFont.setUnderline( ll->layer() == layer );
    ll->setFont( 0, itemFont );
  }
  blockSignals( false );

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

  QgsLegendGroup *parentGroup = dynamic_cast<QgsLegendGroup *>( parent );
  QgsLegendGroup *group;

  if ( parentGroup )
  {
    if ( nameEmpty )
      name = getUniqueGroupName( tr( "sub-group" ), groups() );
    group = new QgsLegendGroup( parentGroup, name );
  }
  else
  {
    if ( nameEmpty )
      name = getUniqueGroupName( tr( "group" ), groups() );
    group = new QgsLegendGroup( this, name );
    if ( parent ) moveItem( group, parent );
    // TODO: warn if parent != NULL or invisibleRootItem ?
  }

  QModelIndex groupIndex = indexFromItem( group );
  setExpanded( groupIndex, expand );
  setCurrentItem( group );
  if ( nameEmpty )
    openEditor();

  blockSignals( false );

  emit itemAdded( groupIndex );

  // TODO: use QModelIndex::iternalId for an identifier instead of this mess ?
  int itemCount = 0;
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem* legendItem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( legendItem->type() == QgsLegendItem::LEGEND_GROUP )
    {
      if ( legendItem == group ) return itemCount;
      else itemCount++;
    }
  }
  return itemCount; // bogus return
}

int QgsLegend::addGroup( QString name, bool expand, int groupIndex )
{
  QTreeWidgetItem * parentItem = invisibleRootItem();

  int itemCount = 0;
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem* legendItem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( legendItem->type() == QgsLegendItem::LEGEND_GROUP )
    {
      if ( itemCount == groupIndex )
      {
        // this is the matching group
        parentItem = legendItem;
        break;
      }
      else
      {
        itemCount = itemCount + 1;
      }
    }
  }

  return addGroup( name, expand, parentItem );
}

void QgsLegend::removeAll()
{
  QgsDebugMsg( "Entering." );

  clear();
  mEmbeddedGroups.clear();
  mPixmapWidthValues.clear();
  mPixmapHeightValues.clear();
  updateMapCanvasLayerSet();
  setIconSize( mMinimumIconSize );
  mDropTarget = 0;
}

void QgsLegend::setLayersVisible( bool visible )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  // Turn off rendering to improve speed.
  mMapCanvas->freeze();

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem* litem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( litem && litem->type() == QgsLegendItem::LEGEND_LAYER )
    {
      theItem->setCheckState( 0, visible ? Qt::Checked : Qt::Unchecked );
      handleItemChange( theItem, 0 );
    }
  }

  // Turn on rendering (if it was on previously)
  mMapCanvas->freeze( false );

  QgsProject::instance()->dirty( true );
}

void QgsLegend::removeGroup( int groupIndex )
{
  QgsLegendGroup * lg = NULL;
  int itemCount = 0;

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    QgsLegendItem* legendItem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( legendItem->type() == QgsLegendItem::LEGEND_GROUP )
    {
      if ( itemCount == groupIndex )
      {
        // this is the matching group
        lg = dynamic_cast<QgsLegendGroup*>( legendItem );
        break;
      }
      else
      {
        itemCount = itemCount + 1;
      }
    }
  }

  if ( lg )
  {
    removeGroup( lg );
  }
}

void QgsLegend::removeLayers( QStringList theLayers )
{
  QgsDebugMsg( "Entering." );
  foreach ( const QString &myId, theLayers )
  {
    bool invLayerRemoved = false;

    for ( QTreeWidgetItem* theItem = firstItem();
          theItem; theItem = nextItem( theItem ) )
    {
      QgsLegendItem *li = dynamic_cast<QgsLegendItem *>( theItem );
      if ( li )
      {
        // save legend layer (parent of a legend layer file we're going to delete)
        QgsLegendLayer* ll = qobject_cast<QgsLegendLayer *>( li );

        if ( ll && ll->layer() && ll->layer()->id() == myId )
        {
          if ( !ll->isVisible() )
          {
            invLayerRemoved = true;
          }
          removeLegendLayerActionsForLayer( ll->layer() );
          removeItem( ll );
          delete ll;
          break;
        }
      }
    }
    emit itemRemoved();
    if ( invLayerRemoved )
      emit invisibleLayerRemoved();
  }
  updateMapCanvasLayerSet();
  adjustIconSize();
}

//deprecated delegates to removeLayers now
void QgsLegend::removeLayer( QString theLayer )
{
  QStringList myList;
  myList << theLayer;
  removeLayers( myList );
}

void QgsLegend::mousePressEvent( QMouseEvent * e )
{
  if ( e->button() == Qt::LeftButton )
  {
    mMousePressedFlag = true;
    mDropTarget = itemAt( e->pos() );
    if ( !mDropTarget )
    {
      setCurrentItem( 0 );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    QTreeWidgetItem* item = itemAt( e->pos() );
    if ( !item )
    {
      setCurrentItem( 0 );
    }
    else if ( item != currentItem() )
    {
      if ( selectedItems().contains( item ) )
      {
        setCurrentItem( item, currentColumn(), QItemSelectionModel::NoUpdate );
      }
      else
      {
        clearSelection();
        setCurrentItem( item );
      }
    }
    else
    {
      // item is the current layer, but maybe previous selection was none
      if ( !item->isSelected() )
      {
        item->setSelected( true );
      }
    }
    handleRightClickEvent( item, e->globalPos() );
    e->ignore();
    return;
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
    foreach ( QTreeWidgetItem * item, selectedItems() )
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
    foreach ( QTreeWidgetItem * item, mItemsBeingMoved )
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
    int line_y = 0;
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
  QStringList layersPriorToEvent = layerIDs();
  QTreeWidget::mouseReleaseEvent( e );
  mMousePressedFlag = false;

  if ( mItemsBeingMoved.isEmpty() )
  {
    //Trigger refresh because of check states on layers.
    //If it comes from a check action on a group, it is not covered in handleItemChanges(),
    //so we do it here
    QgsLegendGroup *lg = dynamic_cast<QgsLegendGroup *>( currentItem() );
    if ( lg && ( layersPriorToEvent != layerIDs() ) )
    {
      mMapCanvas->refresh();
    }
    return;
  }

  setCursor( QCursor( Qt::ArrowCursor ) );
  hideLine();

  // unhide
  foreach ( QTreeWidgetItem * item, mItemsBeingMoved )
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

        foreach ( QTreeWidgetItem * item, mItemsBeingMoved )
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

      foreach ( QTreeWidgetItem * item, mItemsBeingMoved )
      {
        moveItem( item, mDropTarget );
        mDropTarget = item;
      }
    }

    if ( mDropAction == INSERT )
    {
      showItem( "insert into", mDropTarget );

      foreach ( QTreeWidgetItem * item, mItemsBeingMoved )
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

void QgsLegend::mouseDoubleClickEvent( QMouseEvent *e )
{
#ifdef Q_WS_MAC
  // fix for when quick left-then-right clicks (when legend is out of focus)
  //  register as left double click: show contextual menu as user intended
  if ( e->button() == Qt::RightButton )
  {
    mousePressEvent( e );
    return;
  }
#else
  Q_UNUSED( e );
#endif

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

  QMenu theMenu( tr( "Legend context" ), this );

  QgsLegendItem* li = dynamic_cast<QgsLegendItem *>( item );
  if ( li )
  {
    if ( li->type() == QgsLegendItem::LEGEND_LAYER )
    {
      QgsLegendLayer* lyr = qobject_cast<QgsLegendLayer*>( li );
      lyr->addToPopupMenu( theMenu );

      // add custom layer actions
      QList< LegendLayerAction > actions = legendLayerActions( lyr->layer()->type() );
      if ( ! actions.isEmpty() )
      {
        theMenu.addSeparator();
        QList<QMenu*> theMenus;
        for ( int i = 0; i < actions.count(); i++ )
        {
          if ( actions[i].allLayers || actions[i].layers.contains( lyr->layer() ) )
          {
            if ( actions[i].menu.isEmpty() )
            {
              theMenu.addAction( actions[i].action );
            }
            else
            {
              // find or create menu for given menu name
              // adapted from QgisApp::getPluginMenu( QString menuName )
              QString menuName = actions[i].menu;
#ifdef Q_WS_MAC
              // Mac doesn't have '&' keyboard shortcuts.
              menuName.remove( QChar( '&' ) );
#endif
              QAction* before = 0;
              QMenu* newMenu = 0;
              QString dst = menuName;
              dst.remove( QChar( '&' ) );
              foreach ( QMenu* menu, theMenus )
              {
                QString src = menu->title();
                src.remove( QChar( '&' ) );
                int comp = dst.localeAwareCompare( src );
                if ( comp < 0 )
                {
                  // Add item before this one
                  before = menu->menuAction();
                  break;
                }
                else if ( comp == 0 )
                {
                  // Plugin menu item already exists
                  newMenu = menu;
                  break;
                }
              }
              if ( ! newMenu )
              {
                // It doesn't exist, so create
                newMenu = new QMenu( menuName, this );
                theMenus.append( newMenu );
                // Where to put it? - we worked that out above...
                theMenu.insertMenu( before, newMenu );
              }
              // QMenu* menu = getMenu( actions[i].menu, &theBeforeSep, &theAfterSep, &theMenu );
              newMenu->addAction( actions[i].action );
            }
          }
        }
        theMenu.addSeparator();
      }

      // properties goes on bottom of menu for consistency with normal ui standards
      // e.g. kde stuff
      theMenu.addAction( tr( "&Properties" ), QgisApp::instance(), SLOT( layerProperties() ) );

      if ( li->parent() && !parentGroupEmbedded( li ) )
      {
        theMenu.addAction( tr( "&Make to Toplevel Item" ), this, SLOT( makeToTopLevelItem() ) );
      }
    }
    else if ( li->type() == QgsLegendItem::LEGEND_GROUP )
    {
      theMenu.addAction( QgsApplication::getThemeIcon( "/mActionZoomToLayer.png" ),
                         tr( "Zoom to Group" ), this, SLOT( legendLayerZoom() ) );

      // use QGisApp::removeLayer() to remove all selected layers+groups
      theMenu.addAction( QgsApplication::getThemeIcon( "/mActionRemoveLayer.png" ), tr( "&Remove" ), QgisApp::instance(), SLOT( removeLayer() ) );

      theMenu.addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ),
                         tr( "&Set Group CRS" ), this, SLOT( legendGroupSetCRS() ) );
    }

    if (( li->type() == QgsLegendItem::LEGEND_LAYER || li->type() == QgsLegendItem::LEGEND_GROUP ) && !groupEmbedded( li ) && !parentGroupEmbedded( li ) )
    {
      theMenu.addAction( tr( "Re&name" ), this, SLOT( openEditor() ) );
    }

    //
    // Option to group layers, if the selection is more than one
    //
    if ( selectedLayers().length() > 1 )
    {
      theMenu.addAction( tr( "&Group Selected" ), this, SLOT( groupSelectedLayers() ) );
    }
    // ends here
  }

  if ( selectedLayers().length() == 1 )
  {
    QgisApp* app = QgisApp::instance();
    theMenu.addAction( tr( "Copy Style" ), app, SLOT( copyStyle() ) );
    if ( app->clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
    {
      theMenu.addAction( tr( "Paste Style" ), app, SLOT( pasteStyle() ) );
    }
  }

  theMenu.addAction( QgsApplication::getThemeIcon( "/folder_new.png" ), tr( "&Add New Group" ), this, SLOT( addGroupToCurrentItem() ) );
  theMenu.addAction( QgsApplication::getThemeIcon( "/mActionExpandTree.png" ), tr( "&Expand All" ), this, SLOT( expandAll() ) );
  theMenu.addAction( QgsApplication::getThemeIcon( "/mActionCollapseTree.png" ), tr( "&Collapse All" ), this, SLOT( collapseAll() ) );

  QAction *updateDrawingOrderAction = theMenu.addAction( QgsApplication::getThemeIcon( "/mUpdateDrawingOrder.png" ), tr( "&Update Drawing Order" ), this, SLOT( toggleDrawingOrderUpdate() ) );
  updateDrawingOrderAction->setCheckable( true );
  updateDrawingOrderAction->setChecked( mUpdateDrawingOrder );

  theMenu.exec( position );
}

void QgsLegend::initPixmaps()
{
  mPixmaps.mOriginalPixmap = QgsApplication::getThemePixmap( "/mActionFileSmall.png" );
  mPixmaps.mInOverviewPixmap = QgsApplication::getThemePixmap( "/mActionInOverview.png" );
  mPixmaps.mEditablePixmap = QgsApplication::getThemePixmap( "/mIconEditable.png" );
  mPixmaps.mProjectionErrorPixmap = QgsApplication::getThemePixmap( "/mIconProjectionProblem.png" );
}

Qt::CheckState QgsLegend::layerCheckState( QgsMapLayer * layer )
{
  QgsLegendLayer * ll = findLegendLayer( layer );

  return ll ? ll->checkState( 0 ) : Qt::Unchecked;
}

bool QgsLegend::layerIsExpanded( QgsMapLayer * layer )
{
  QgsLegendLayer * ll = findLegendLayer( layer );

  return ll->isExpanded();
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

  //store identify disabled layers of the embedded project
  QSet<QString> embeddedIdentifyDisabledLayers;
  QDomElement disabledLayersElem = projectDocument.documentElement().firstChildElement( "properties" ).firstChildElement( "Identify" ).firstChildElement( "disabledLayers" );
  if ( !disabledLayersElem.isNull() )
  {
    QDomNodeList valueList = disabledLayersElem.elementsByTagName( "value" );
    for ( int i = 0; i < valueList.size(); ++i )
    {
      embeddedIdentifyDisabledLayers.insert( valueList.at( i ).toElement().text() );
    }
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

      group->setEmbedded( true );
      group->setProjectPath( projectFilePath );

      // start with already set font style
      QFont groupFont = group->font( 0 );
      groupFont.setItalic( true );
      group->setFont( 0, groupFont );
      setCurrentItem( group );

      QDomNodeList groupChildren = legendElem.childNodes();
      for ( int j = 0; j < groupChildren.size(); ++j )
      {
        QDomElement childElem = groupChildren.at( j ).toElement();
        bool visible = ( childElem.attribute( "checked" ).compare( "Qt::Checked", Qt::CaseInsensitive ) == 0 );
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

          if ( cItem )
          {
            group->insertChild( group->childCount(), cItem );
          }

          if ( !visible )
          {
            cItem->setCheckState( 0, Qt::Unchecked );
          }

          //consider the layer might be identify disabled in its project
          if ( embeddedIdentifyDisabledLayers.contains( layerId ) )
          {
            QStringList thisProjectIdentifyDisabledLayers = QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );
            thisProjectIdentifyDisabledLayers.append( layerId );
            QgsProject::instance()->writeEntry( "Identify", "/disabledLayers", thisProjectIdentifyDisabledLayers );
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

//introduced in QGIS 1.8 - add layers in a batch
void QgsLegend::addLayers( QList<QgsMapLayer *> theLayerList )
{
  QgsDebugMsg( "Entering." );
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  QSettings settings;

  //Note if the canvas was previously blank so we can
  //zoom to all layers at the end if neeeded
  bool myFirstLayerFlag = false;
  QgsCoordinateReferenceSystem myPreviousCrs;
  if ( layers().count() < 1 )
  {
    myFirstLayerFlag = true;
  }
  else
  {
    // remember CRS of present layer
    myPreviousCrs = layers().first()->crs();
  }

  //iteratively add the layers to the canvas
  for ( int i = 0; i < theLayerList.size(); ++i )
  {
    QgsMapLayer * layer = theLayerList.at( i );
    QgsLegendLayer* llayer = new QgsLegendLayer( layer );
    if ( !QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
    {
      // start with already set font style
      QFont itemFont = llayer->font( 0 );
      itemFont.setItalic( true );
      llayer->setFont( 0, itemFont );
    }

    //set the correct check states
    blockSignals( true );
    llayer->setCheckState( 0, llayer->isVisible() ? Qt::Checked : Qt::Unchecked );
    blockSignals( false );

    QgsLegendGroup *lg = dynamic_cast<QgsLegendGroup *>( currentItem() );
    if ( !lg && currentItem() )
    {
      lg = dynamic_cast<QgsLegendGroup *>( currentItem()->parent() );
    }

    int index = 0;
    if ( lg )
    {
      index = lg->indexOfChild( currentItem() );
    }
    else
    {
      index = indexOfTopLevelItem( currentItem() );
    }

    if ( index < 0 )
    {
      index = 0;
    }

    if ( lg && settings.value( "/qgis/addNewLayersToCurrentGroup", false ).toBool() )
    {
      lg->insertChild( index, llayer );
    }
    else
    {
      insertTopLevelItem( index, llayer );
      setCurrentItem( llayer );
    }

    setItemExpanded( llayer, true );
    //don't expand raster items by default, there could be too many
    refreshLayerSymbology( layer->id(), layer->type() != QgsMapLayer::RasterLayer );

    updateMapCanvasLayerSet();
    emit itemAdded( indexFromItem( llayer ) );
  }
  // first layer?
  if ( myFirstLayerFlag )
  {
    QgsMapLayer * myFirstLayer = theLayerList.at( 0 );
    if ( !mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
    {
      mMapCanvas->mapRenderer()->setDestinationCrs( myFirstLayer->crs() );
      mMapCanvas->mapRenderer()->setMapUnits( myFirstLayer->crs().mapUnits() );
    }
    mMapCanvas->zoomToFullExtent();
    mMapCanvas->clearExtentHistory();
  }
  else
  {
    if ( settings.value( "/Projections/otfTransformAutoEnable", true ).toBool() &&
         !mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
    {
      // Verify if all layers have the same CRS
      foreach ( QgsMapLayer *l, layers() )
      {
        if ( myPreviousCrs != l->crs() )
        {
          // Set to the previous de facto used so that extent does not change
          mMapCanvas->mapRenderer()->setDestinationCrs( myPreviousCrs );
          mMapCanvas->mapRenderer()->setProjectionsEnabled( true );
          break;
        }
      }
    }
  }
  //make the QTreeWidget item up-to-date
  doItemsLayout();
}

//deprecated since 1.8 - delegates to addLayers
void QgsLegend::addLayer( QgsMapLayer * layer )
{
  QList<QgsMapLayer *> myList;
  myList << layer;
  addLayers( myList );
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

QList<QgsMapLayer *> QgsLegend::selectedLayers( bool inDrawOrder )
{
  QList<QgsMapLayer *> layers;

  if ( inDrawOrder )
  {
    foreach ( QgsLegendLayer *ll, legendLayers() )
    {
      if ( ll->isSelected() )
        layers << ll->layer();
    }
  }
  else
  {
    foreach ( QTreeWidgetItem * item, selectedItems() )
    {
      QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( item );
      if ( ll )
        layers << ll->layer();
    }
  }

  return layers;
}

bool QgsLegend::selectedLayersEditable( bool modified )
{
  bool hasEditable = false;
  foreach ( QgsMapLayer * layer, selectedLayers() )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer*>( layer );
    if ( !vl )
    {
      continue;
    }
    if ( vl->isEditable() && ( !modified || ( modified && vl->isModified() ) ) )
    {
      hasEditable = true;
      break;
    }
  }
  return hasEditable;
}

QList<QgsLegendLayer *> QgsLegend::legendLayers()
{
  if ( mUpdateDrawingOrder )
  {
    QList< QgsLegendLayer * > items;
    QTreeWidgetItemIterator it( this );
    while ( *it )
    {
      QgsLegendLayer *llayer = dynamic_cast<QgsLegendLayer *>( *it );
      if ( llayer )
        items.append( llayer );

      ++it;
    }

    return items;
  }
  else
  {
    int n = 0;
    QMap< int, QgsLegendLayer * > items;
    QTreeWidgetItemIterator it( this );
    while ( *it )
    {
      QgsLegendLayer *llayer = dynamic_cast<QgsLegendLayer *>( *it );
      if ( llayer )
      {
        QgsDebugMsgLevel( QString( "n=%1 o=%2 name=%3" ).arg( n ).arg( llayer->drawingOrder() ).arg( llayer->layer()->name() ), 3 );
        items.insertMulti( llayer->drawingOrder(), llayer );
        n++;
      }

      ++it;
    }

    QList< QgsLegendLayer * > ls;

    foreach ( int o, items.uniqueKeys() )
    {
      QgsDebugMsgLevel( QString( "o=%1" ).arg( o ), 3 );
      QList< QgsLegendLayer *> values = items.values( o );
      for ( int i = values.size() - 1; i >= 0; i-- )
      {
        QgsDebugMsgLevel( QString( " layer=%1" ).arg( values[i]->layer()->name() ), 3 );
        ls << values[i];
      }
    }

    return ls;
  }
}

QList<DrawingOrderInfo> QgsLegend::drawingOrder()
{
  QMap<int, DrawingOrderInfo> drawingOrder;
  QSet<QgsLegendLayer*> embeddedGroupChildren;
  int nEntries = 0;

  QTreeWidgetItemIterator it( this );
  while ( *it )
  {
    QgsLegendLayer* llayer = dynamic_cast<QgsLegendLayer *>( *it );
    QgsLegendGroup* lgroup = dynamic_cast<QgsLegendGroup *>( *it );
    if ( llayer )
    {
      if ( !embeddedGroupChildren.contains( llayer ) )
      {
        DrawingOrderInfo dInfo;
        dInfo.name = llayer->layerName();
        dInfo.id = llayer->layer()->id();
        dInfo.checked = ( llayer->checkState( 0 ) == Qt::Checked );
        dInfo.embeddedGroup = false;
        if ( mUpdateDrawingOrder )
        {
          drawingOrder.insertMulti( nEntries, dInfo );
        }
        else
        {
          drawingOrder.insertMulti( llayer->drawingOrder(), dInfo );
        }
        ++nEntries;
      }
    }
    else if ( lgroup )
    {
      if ( lgroup->isEmbedded() && !( lgroup->parent() ) )
      {
        QList<QgsLegendLayer*> groupLayers = lgroup->legendLayers();
        QList<QgsLegendLayer*>::const_iterator groupLayerIt = groupLayers.constBegin();
        for ( ; groupLayerIt != groupLayers.constEnd(); ++groupLayerIt )
        {
          embeddedGroupChildren.insert( *groupLayerIt );
        }
        DrawingOrderInfo dInfo;
        dInfo.name = lgroup->text( 0 );
        dInfo.id = lgroup->projectPath();
        dInfo.checked = ( lgroup->checkState( 0 ) != Qt::Unchecked );
        dInfo.embeddedGroup = true;
        if ( mUpdateDrawingOrder )
        {
          drawingOrder.insertMulti( nEntries, dInfo );
        }
        else
        {
          drawingOrder.insertMulti( lgroup->drawingOrder(), dInfo );
        }
        ++nEntries;
      }
    }
    ++it;
  }
  return drawingOrder.values();
}

QList<QgsMapLayer *> QgsLegend::layers()
{
  QList<QgsMapLayer *> ls;

  foreach ( QgsLegendLayer *l, legendLayers() )
  {
    ls << l->layer();
  }

  return ls;
}

QList<QgsMapCanvasLayer> QgsLegend::canvasLayers()
{
  QMap<int, QgsMapCanvasLayer> layers;
  QSet<QgsLegendLayer*> embeddedGroupChildren;
  int nEntries = 0;

  QTreeWidgetItemIterator it( this );
  while ( *it )
  {
    QgsLegendLayer* llayer = dynamic_cast<QgsLegendLayer *>( *it );
    QgsLegendGroup* lgroup = dynamic_cast<QgsLegendGroup *>( *it );
    if ( llayer && !embeddedGroupChildren.contains( llayer ) )
    {
      QgsMapCanvasLayer canvasLayer = llayer->canvasLayer();
      if ( mUpdateDrawingOrder )
      {
        layers.insertMulti( nEntries + embeddedGroupChildren.size(), canvasLayer );
      }
      else
      {
        layers.insertMulti( llayer->drawingOrder(), canvasLayer );
      }
    }
    else if ( lgroup )
    {
      if ( lgroup->isEmbedded() )
      {
        int groupDrawingOrder = lgroup->drawingOrder();
        QList<QgsLegendLayer*> groupLayers = lgroup->legendLayers();
        for ( int i = groupLayers.size() - 1; i >= 0; --i )
        {
          QgsLegendLayer* ll = groupLayers.at( i );
          if ( !ll || embeddedGroupChildren.contains( ll ) )
          {
            continue;
          }

          if ( mUpdateDrawingOrder )
          {
            layers.insertMulti( nEntries, ll->canvasLayer() );
          }
          else
          {
            layers.insertMulti( groupDrawingOrder,  ll->canvasLayer() );
          }
          embeddedGroupChildren.insert( ll );
        }
      }
    }
    ++it;
    ++nEntries;
  }

  return layers.values();
}

void QgsLegend::setDrawingOrder( QList<QgsMapLayer *> layers )
{
  QgsDebugMsg( "Entering." );

  for ( int i = 0; i < layers.size(); i++ )
  {
    QgsLegendLayer *ll = findLegendLayer( layers[i] );
    if ( !ll )
      continue;
    QgsDebugMsgLevel( QString( "setting order=%1 name=%2." ).arg( i ).arg( layers[i]->name() ), 3 );
    ll->setDrawingOrder( i );
  }

  updateMapCanvasLayerSet();
}

void QgsLegend::setDrawingOrder( const QList<DrawingOrderInfo> &order )
{
  QList<QgsMapCanvasLayer> layers;

  QList<DrawingOrderInfo>::const_iterator orderIt = order.constBegin();
  int i = 0;
  for ( ; orderIt != order.constEnd(); ++orderIt )
  {
    if ( orderIt->embeddedGroup )
    {
      //find group
      QgsLegendGroup* group = findLegendGroup( orderIt->name, orderIt->id );
      if ( group )
      {
        group->setDrawingOrder( i );
        QList<QgsLegendLayer*> groupLayers = group->legendLayers();
        QList<QgsLegendLayer*>::iterator groupIt = groupLayers.begin();
        for ( ; groupIt != groupLayers.end(); ++groupIt )
        {
          layers.push_back(( *groupIt )->canvasLayer() );
        }
        ++i;
      }
    }
    else
    {
      QgsLegendLayer *ll = findLegendLayer( orderIt->id );
      if ( ll )
      {
        ll->setDrawingOrder( i );
        layers.push_back( ll->canvasLayer() );
        ++i;
      }
    }
  }
  mMapCanvas->setLayerSet( layers );
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

  // Turn off rendering to improve speed.
  mMapCanvas->freeze();

  QgsLegendGroup* lg = dynamic_cast<QgsLegendGroup *>( currentItem() );
  if ( lg )
  {
    removeGroup( lg );
  }

  // Turn on rendering (if it was on previously)
  mMapCanvas->freeze( false );
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
      QgsMapLayerRegistry::instance()->removeMapLayers(
        QStringList() << cl->layer()->id() );
    else if ( cg )
      removeGroup( cg );

    child = lg->child( 0 );
  }

  delete lg;

  emit itemRemoved();

  adjustIconSize();
}

void QgsLegend::setGroupCRS( QgsLegendGroup *lg, const QgsCoordinateReferenceSystem &crs )
{
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  foreach ( QgsLegendLayer *cl, lg->legendLayers() )
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

  int itemCount = 0;
  QgsLegendGroup *group = NULL;

  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {

    QgsLegendItem* legendItem = dynamic_cast<QgsLegendItem *>( theItem );
    if ( legendItem->type() == QgsLegendItem::LEGEND_GROUP )
    {
      if ( itemCount == groupIndex )
      {
        // this is the matching group
        group = dynamic_cast<QgsLegendGroup*>( legendItem );
        break;
      }
      else
      {
        itemCount = itemCount + 1;
      }
    }
  }

  if ( group == NULL )
    return;

  insertItem( layer, group );

  emit itemMovedGroup( dynamic_cast<QgsLegendItem*>( layer ), groupIndex );
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
  foreach ( QTreeWidgetItem *currentItem, items )
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
        legendgroupnode.setAttribute( "project", QgsProject::instance()->writePath( embedIt.value() ) );
        QgsLegendGroup* group = dynamic_cast<QgsLegendGroup*>( item );
        if ( group )
        {
          legendgroupnode.setAttribute( "drawingOrder", group->drawingOrder() );
        }
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
      legendlayernode.setAttribute( "drawingOrder", ll->drawingOrder() );
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
  QgsRectangle bkMapExtent;
  if ( mMapCanvas )
  {
    bkMapExtent = mMapCanvas->extent();
  }

  QString activeLayerId = node.toElement().attribute( "activeLayer" );
  QgsLegendLayer *activeLayer = 0;

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
        theGroup = addEmbeddedGroup( name, QgsProject::instance()->readPath( childelem.attribute( "project" ) ) );
        if ( childelem.hasAttribute( "drawingOrder" ) )
        {
          theGroup->setDrawingOrder( childelem.attribute( "drawingOrder" ).toInt() );
        }
        updateGroupCheckStates( theGroup );
      }
      else
      {
        blockSignals( true );
        if ( parent )
          theGroup = new QgsLegendGroup( parent, name );
        else
          theGroup = new QgsLegendGroup( this, name );
        blockSignals( false );
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

      if ( currentLayer->layer() && currentLayer->layer()->id() == activeLayerId )
        activeLayer = currentLayer;

      if ( currentLayer->layer() && !QgsProject::instance()->layerIsEmbedded( currentLayer->layer()->id() ).isEmpty() )
      {
        // start with already set font style
        QFont itemFont = currentLayer->font( 0 );
        itemFont.setItalic( true );
        currentLayer->setFont( 0, itemFont );
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

  if ( activeLayer )
    setCurrentItem( activeLayer );

  //restore canvas extent (could be changed by addLayer calls)
  if ( !bkMapExtent.isEmpty() )
  {
    mMapCanvas->setExtent( bkMapExtent );
  }

  return true;
}

bool QgsLegend::readXML( QDomNode& legendnode )
{
  clear(); //remove all items first
  mEmbeddedGroups.clear();

  mUpdateDrawingOrder = true;

  if( !readXML( 0, legendnode ) )
    return false;

  if( legendnode.toElement().attribute( "updateDrawingOrder", "true" ) != "true" )
  {
    if( !verifyDrawingOrder() )
      QgsMessageLog::logMessage( tr( "Not fully defined drawing order set to legend order." ), tr( "Legend" ), QgsMessageLog::WARNING );

    mUpdateDrawingOrder = false;
  }

  emit updateDrawingOrderChecked( mUpdateDrawingOrder );
  emit updateDrawingOrderUnchecked( !mUpdateDrawingOrder );

  return true;
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
  ll->setDrawingOrder( childelem.attribute( "drawingOrder", "-1" ).toInt() );
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

QgsLegendGroup* QgsLegend::findLegendGroup( const QString& name, const QString& projectPath )
{
  QgsLegendGroup* group = 0;
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    group = dynamic_cast<QgsLegendGroup*>( theItem );
    if ( group )
    {
      if ( group->text( 0 ) == name && group->projectPath() == projectPath )
      {
        return group;
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
    emit itemMovedGroup( movedItem, indexFromItem( intoItem ).row() );
  }
}

void QgsLegend::moveItem( QTreeWidgetItem* move, QTreeWidgetItem* after )
{
  QModelIndex oldIndex = indexFromItem( move );

  QgsDebugMsgLevel( QString( "Moving layer : %1 (%2)" ).arg( move->text( 0 ) ).arg( move->type() ), 3 );
  if ( after )
  {
    QgsDebugMsgLevel( QString( "after layer  : %1 (%2)" ).arg( after->text( 0 ) ).arg( after->type() ), 3 );
  }
  else
  {
    QgsDebugMsgLevel( "as toplevel item", 3 );
  }

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

  emit itemMoved( oldIndex, indexFromItem( move ) );
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
  QgsDebugMsg( "Entering." );
  QList<QgsMapCanvasLayer> layers = canvasLayers();
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
      if ( ll->checkState( 0 ) == Qt::Checked )
      {
        QgsMapLayer *lyr = ll->layer();
        layers.push_front( lyr->id() );
      }
    }
  }

#ifdef QGISDEBUG
  QgsDebugMsg( "QgsLegend::layerIDs()" );
  foreach ( QString id, layers )
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
  QTreeWidgetItem* current = currentItem();
  // in case the current item is a child of the layer, use the layer as current item
  // because otherwise we would set an invalid item as current item
  // (in refreshSymbology the symbology items are removed and new ones are added)
  if ( current && current->parent() == theLegendLayer )
    current = current->parent();

  theLegendLayer->refreshSymbology( key );

  //restore the current item again
  setCurrentItem( current );
  adjustIconSize();
  setItemExpanded( theLegendLayer, expandItem );//make sure the symbology items are visible
}

void QgsLegend::refreshLayerSymbology( QString key, QgsLegendItem::Expansion expandItem )
{
  bool expand = true;
  if ( expandItem == QgsLegendItem::DontChange )
  {
    QgsLegendLayer* theLegendLayer = findLegendLayer( key );
    if ( !theLegendLayer )
    {
      return;
    }
    expand = theLegendLayer->isExpanded();
  }
  else if ( expandItem == QgsLegendItem::Collapse )
  {
    expand = false;
  }
  return refreshLayerSymbology( key, expand );
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

void QgsLegend::handleCloseEditor( QWidget * editor, QAbstractItemDelegate::EndEditHint hint )
{
  Q_UNUSED( editor );
  Q_UNUSED( hint );
  QgsLegendItem *item = dynamic_cast<QgsLegendItem *>( currentItem() );
  if ( item )
  {
    item->afterEdit();
  }
}

void QgsLegend::handleItemChange( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column );

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
    if ( ll->layerName() != ll->layer()->name() )
    {
      ll->layer()->setLayerName( ll->layerName() );
    }
  }

  bool changing = mChanging;
  mChanging = true;
  bool mapCanvasFrozen = mMapCanvas->isFrozen(); //save freeze state

  if ( !changing )
  {
    mMapCanvas->freeze();

    if ( item->isSelected() )
    {
      foreach ( QTreeWidgetItem * i, selectedItems() )
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
    // If it was on, turn it back on, otherwise leave it
    // off, as turning it on causes a refresh.
    mMapCanvas->freeze( mapCanvasFrozen );

    // update layer set
    updateMapCanvasLayerSet();
  }

  mChanging = changing;
}

void QgsLegend::openEditor()
{
  QgsLegendItem* theItem = dynamic_cast<QgsLegendItem*>( currentItem() );
  if ( theItem )
  {
    if ( !groupEmbedded( theItem ) && !parentGroupEmbedded( theItem ) )
    {
      theItem->beforeEdit();
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
    QgsDebugMsg( "Raster units per pixel  : " + QString::number( layer->rasterUnitsPerPixelX() ) );
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
      mMapCanvas->zoomByFactor( qAbs( layer->rasterUnitsPerPixelX() / width ) );
    }
    else
    {
      mMapCanvas->zoomByFactor( qAbs( layer->rasterUnitsPerPixelX() / mMapCanvas->mapUnitsPerPixel() ) );
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
    QgsContrastEnhancement::ContrastEnhancementAlgorithm contrastEnhancementAlgorithm = QgsContrastEnhancement::StretchToMinimumMaximum;

    QgsRectangle myRectangle;
    myRectangle = mMapCanvas->mapRenderer()->outputExtentToLayerExtent( layer, mMapCanvas->extent() );
    layer->setContrastEnhancement( contrastEnhancementAlgorithm, QgsRaster::ContrastEnhancementMinMax, myRectangle );

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
  mapcanvasNode.setAttribute( "updateDrawingOrder", mUpdateDrawingOrder ? "true" : "false" );
  if ( currentLayer() )
    mapcanvasNode.setAttribute( "activeLayer", currentLayer()->id() );
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
  mMapCanvas->freeze();

  foreach ( QTreeWidgetItem * item, selectedItems() )
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
      QgsMapLayerRegistry::instance()->removeMapLayers(
        QStringList() << ll->layer()->id() );
      continue;
    }
  }

  // Turn on rendering (if it was on previously)
  mMapCanvas->freeze( false );
}

void QgsLegend::setCRSForSelectedLayers( const QgsCoordinateReferenceSystem &crs )
{
  // Turn off rendering to improve speed.
  mMapCanvas->freeze();

  foreach ( QTreeWidgetItem * item, selectedItems() )
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
  mMapCanvas->freeze( false );
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

bool QgsLegend::verifyDrawingOrder()
{
  Q_ASSERT( mUpdateDrawingOrder );

  // check if the drawing order wasn't already initially set
  bool hasUndefinedOrder = false;
  for ( QTreeWidgetItemIterator it( this ); *it && !hasUndefinedOrder; it++ )
  {
    QgsLegendLayer *ll = dynamic_cast<QgsLegendLayer *>( *it );
    hasUndefinedOrder |= ll && ll->drawingOrder() < 0;
  }

  if( !hasUndefinedOrder )
    return true;

  int i = 0;
  foreach ( QgsLegendLayer *ll, legendLayers() )
  {
    ll->setDrawingOrder( i++ );
  }

  return false;
}

void QgsLegend::setUpdateDrawingOrder( bool updateDrawingOrder )
{
  if ( mUpdateDrawingOrder == updateDrawingOrder )
    return;

  if ( !updateDrawingOrder )
  {
    verifyDrawingOrder();
  }

  mUpdateDrawingOrder = updateDrawingOrder;

  QgsProject::instance()->dirty( true );

  updateMapCanvasLayerSet();

  emit updateDrawingOrderChecked( mUpdateDrawingOrder );
  emit updateDrawingOrderUnchecked( !mUpdateDrawingOrder );
}

bool QgsLegend::updateDrawingOrder()
{
  return mUpdateDrawingOrder;
}

void QgsLegend::toggleDrawingOrderUpdate()
{
  setUpdateDrawingOrder( !mUpdateDrawingOrder );
}

void QgsLegend::groupSelectedLayers()
{
  //avoid multiple refreshes of map canvas because of itemChanged signal
  blockSignals( true );

  QTreeWidgetItem * parent = 0;
  foreach ( QTreeWidgetItem* item, selectedItems() )
  {
    parent = item->parent();
  }
  QgsLegendGroup *group;

  if ( parent )
  {
    group = new QgsLegendGroup( parent,
                                getUniqueGroupName( tr( "sub-group" ), groups() ) );
  }
  else
  {
    group = new QgsLegendGroup( this,
                                getUniqueGroupName( tr( "group" ), groups() ) );
  }

  // save old indexes so we can notify changes
  QList< QModelIndex > oldIndexes;
  QList< QTreeWidgetItem* > selected;

  foreach ( QTreeWidgetItem * item, selectedItems() )
  {
    QgsLegendLayer* layer = dynamic_cast<QgsLegendLayer *>( item );
    if ( layer )
    {
      oldIndexes.append( indexFromItem( item ) );
      selected.append( item );
    }
  }
  foreach ( QTreeWidgetItem * item, selected )
  {
    insertItem( item, group );
  }

  editItem( group, 0 );

  blockSignals( false );

  // notify that group was added and that items were moved
  emit itemAdded( indexFromItem( group ) );
  for ( int i = 0; i < selected.size(); i++ )
  {
    emit itemMoved( oldIndexes[i], indexFromItem( selected[i] ) );
  }
}

void QgsLegend::addLegendLayerAction( QAction* action, QString menu, QString id,
                                      QgsMapLayer::LayerType type, bool allLayers )
{
  mLegendLayerActionMap[type].append( LegendLayerAction( action, menu, id, allLayers ) );
}

bool QgsLegend::removeLegendLayerAction( QAction* action )
{
  QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > >::iterator it;
  for ( it = mLegendLayerActionMap.begin();
        it != mLegendLayerActionMap.end(); ++it )
  {
    for ( int i = 0; i < it->count(); i++ )
    {
      if (( *it )[i].action == action )
      {
        ( *it ).removeAt( i );
        return true;
      }
    }
  }
  return false;
}

void QgsLegend::addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer )
{
  QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > >::iterator it;
  for ( it = mLegendLayerActionMap.begin();
        it != mLegendLayerActionMap.end(); ++it )
  {
    for ( int i = 0; i < it->count(); i++ )
    {
      if (( *it )[i].action == action )
      {
        ( *it )[i].layers.append( layer );
        return;
      }
    }
  }
}

void QgsLegend::removeLegendLayerActionsForLayer( QgsMapLayer* layer )
{
  QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > >::iterator it;
  for ( it = mLegendLayerActionMap.begin();
        it != mLegendLayerActionMap.end(); ++it )
  {
    for ( int i = 0; i < it->count(); i++ )
    {
      ( *it )[i].layers.removeAll( layer );
    }
  }
}

QList< LegendLayerAction > QgsLegend::legendLayerActions( QgsMapLayer::LayerType type ) const
{
  return mLegendLayerActionMap.contains( type ) ? mLegendLayerActionMap.value( type ) : QList< LegendLayerAction >() ;
}

void QgsLegend::updateLegendItemStyles()
{
  QgsLegendGroup* lg = 0;
  QgsLegendLayer* ll = 0;
  for ( QTreeWidgetItem* theItem = firstItem(); theItem; theItem = nextItem( theItem ) )
  {
    ll = dynamic_cast<QgsLegendLayer *>( theItem );
    if ( ll )
    {
      ll->setupFont();
      // map layer name capitalize option may have changed
      ll->layer()->setLayerName( ll->layer()->originalName() );
      continue;
    }

    lg = dynamic_cast<QgsLegendGroup *>( theItem );
    if ( lg )
    {
      lg->setupFont();
    }
  }
  update();
}

void QgsLegend::updateLegendItemSymbologies()
{
  foreach ( QgsLegendLayer* ll, legendLayers() )
  {
    ll->refreshSymbology( ll->layer()->id() );
  }
}
