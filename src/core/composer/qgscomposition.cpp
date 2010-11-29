/***************************************************************************
                              qgscomposition.cpp
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposition.h"
#include "qgscomposeritem.h"
#include "qgscomposermap.h"
#include "qgspaperitem.h"
#include "qgslogger.h"
#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsRectItem>
#include <QSettings>

QgsComposition::QgsComposition( QgsMapRenderer* mapRenderer ):
    QGraphicsScene( 0 ), mMapRenderer( mapRenderer ), mPlotStyle( QgsComposition::Preview ), mPaperItem( 0 ), mSnapToGrid( false ),
    mSnapGridResolution( 0.0 ), mSnapGridOffsetX( 0.0 ), mSnapGridOffsetY( 0.0 ), mActiveCommand( 0 )
{
  setBackgroundBrush( Qt::gray );

  //set paper item
  mPaperItem = new QgsPaperItem( 0, 0, 297, 210, this ); //default size A4
  mPaperItem->setBrush( Qt::white );
  addItem( mPaperItem );
  mPaperItem->setZValue( 0 );
  mPrintResolution = 300; //hardcoded default
  loadGridAppearanceSettings();

  //mPrintAsRaster
  QSettings s;
  mPrintAsRaster = s.value( "/qgis/composerPrintAsRaster", false ).toBool();
}

QgsComposition::QgsComposition():
    QGraphicsScene( 0 ), mMapRenderer( 0 ), mPlotStyle( QgsComposition::Preview ), mPaperItem( 0 ), mPrintAsRaster( false ),
    mSnapToGrid( false ), mSnapGridResolution( 0.0 ), mSnapGridOffsetX( 0.0 ), mSnapGridOffsetY( 0.0 ), mActiveCommand( 0 )
{
  loadGridAppearanceSettings();
  //mPrintAsRaster
  QSettings s;
  mPrintAsRaster = s.value( "/qgis/composerPrintAsRaster", false ).toBool();
}

QgsComposition::~QgsComposition()
{
  delete mPaperItem;

  // make sure that all composer items are removed before
  // this class is deconstructed - to avoid segfaults
  // when composer items access in destructor composition that isn't valid anymore
  clear();
}

void QgsComposition::setPaperSize( double width, double height )
{
  if ( mPaperItem )
  {
    mPaperItem->setRect( QRectF( 0, 0, width, height ) );
    emit paperSizeChanged();
  }
}

double QgsComposition::paperHeight() const
{
  return mPaperItem->rect().height();
}

double QgsComposition::paperWidth() const
{
  return mPaperItem->rect().width();
}

QgsComposerItem* QgsComposition::composerItemAt( const QPointF & position )
{
  QList<QGraphicsItem *> itemList = items( position );
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();

  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    if ( composerItem && composerItem != mPaperItem )
    {
      return composerItem;
    }
  }
  return 0;
}

QList<QgsComposerItem*> QgsComposition::selectedComposerItems()
{
  QList<QgsComposerItem*> composerItemList;

  QList<QGraphicsItem *> graphicsItemList = selectedItems();
  QList<QGraphicsItem *>::iterator itemIter = graphicsItemList.begin();

  for ( ; itemIter != graphicsItemList.end(); ++itemIter )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIter );
    if ( composerItem )
    {
      composerItemList.push_back( composerItem );
    }
  }

  return composerItemList;
}

QList<const QgsComposerMap*> QgsComposition::composerMapItems() const
{
  QList<const QgsComposerMap*> resultList;

  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    const QgsComposerMap* composerMap = dynamic_cast<const QgsComposerMap *>( *itemIt );
    if ( composerMap )
    {
      resultList.push_back( composerMap );
    }
  }

  return resultList;
}

const QgsComposerMap* QgsComposition::getComposerMapById( int id ) const
{
  QList<const QgsComposerMap*> resultList;

  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    const QgsComposerMap* composerMap = dynamic_cast<const QgsComposerMap *>( *itemIt );
    if ( composerMap )
    {
      if ( composerMap->id() == id )
      {
        return composerMap;
      }
    }
  }

  return 0;
}

int QgsComposition::pixelFontSize( double pointSize ) const
{
  //in QgsComposition, one unit = one mm
  double sizeMillimeters = pointSize * 0.3527;
  return ( sizeMillimeters + 0.5 ); //round to nearest mm
}

double QgsComposition::pointFontSize( int pixelSize ) const
{
  double sizePoint = pixelSize / 0.3527;
  return sizePoint;
}

bool QgsComposition::writeXML( QDomElement& composerElem, QDomDocument& doc )
{
  if ( composerElem.isNull() )
  {
    return false;
  }

  QDomElement compositionElem = doc.createElement( "Composition" );
  if ( mPaperItem )
  {
    compositionElem.setAttribute( "paperWidth", mPaperItem->rect().width() );
    compositionElem.setAttribute( "paperHeight", mPaperItem->rect().height() );
  }

  //snapping
  if ( mSnapToGrid )
  {
    compositionElem.setAttribute( "snapping", "1" );
  }
  else
  {
    compositionElem.setAttribute( "snapping", "0" );
  }
  compositionElem.setAttribute( "snapGridResolution", mSnapGridResolution );
  compositionElem.setAttribute( "snapGridOffsetX", mSnapGridOffsetX );
  compositionElem.setAttribute( "snapGridOffsetY", mSnapGridOffsetY );

  compositionElem.setAttribute( "printResolution", mPrintResolution );

  composerElem.appendChild( compositionElem );

  return true;
}

bool QgsComposition::readXML( const QDomElement& compositionElem, const QDomDocument& doc )
{
  if ( compositionElem.isNull() )
  {
    return false;
  }

  //create paper item
  bool widthConversionOk, heightConversionOk;
  double paperWidth = compositionElem.attribute( "paperWidth" ).toDouble( &widthConversionOk );
  double paperHeight = compositionElem.attribute( "paperHeight" ).toDouble( &heightConversionOk );

  if ( widthConversionOk && heightConversionOk )
  {
    delete mPaperItem;
    mPaperItem = new QgsPaperItem( 0, 0, paperWidth, paperHeight, this );
    mPaperItem->setBrush( Qt::white );
    addItem( mPaperItem );
    mPaperItem->setZValue( 0 );
  }

  //snapping
  if ( compositionElem.attribute( "snapping" ) == "0" )
  {
    mSnapToGrid = false;
  }
  else
  {
    mSnapToGrid = true;
  }
  mSnapGridResolution = compositionElem.attribute( "snapGridResolution" ).toDouble();
  mSnapGridOffsetX = compositionElem.attribute( "snapGridOffsetX" ).toDouble();
  mSnapGridOffsetY = compositionElem.attribute( "snapGridOffsetY" ).toDouble();

  mPrintResolution = compositionElem.attribute( "printResolution", "300" ).toInt();

  if ( mPaperItem )
  {
    mPaperItem->update();
  }

  return true;
}

void QgsComposition::addItemToZList( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }
  mItemZList.push_back( item );
  QgsDebugMsg( QString::number( mItemZList.size() ) );
  item->setZValue( mItemZList.size() );
}

void QgsComposition::removeItemFromZList( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }
  mItemZList.removeAll( item );
}

void QgsComposition::raiseSelectedItems()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  for ( ; it != selectedItems.end(); ++it )
  {
    raiseItem( *it );
  }

  //update all positions
  updateZValues();
  update();
}

void QgsComposition::raiseItem( QgsComposerItem* item )
{
  //search item
  QMutableLinkedListIterator<QgsComposerItem*> it( mItemZList );
  if ( it.findNext( item ) )
  {
    if ( it.hasNext() )
    {
      it.remove();
      it.next();
      it.insert( item );
    }
  }
}

void QgsComposition::lowerSelectedItems()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  for ( ; it != selectedItems.end(); ++it )
  {
    lowerItem( *it );
  }

  //update all positions
  updateZValues();
  update();
}

void QgsComposition::lowerItem( QgsComposerItem* item )
{
  //search item
  QMutableLinkedListIterator<QgsComposerItem*> it( mItemZList );
  if ( it.findNext( item ) )
  {
    it.previous();
    if ( it.hasPrevious() )
    {
      it.remove();
      it.previous();
      it.insert( item );
    }
  }
}

void QgsComposition::moveSelectedItemsToTop()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();

  for ( ; it != selectedItems.end(); ++it )
  {
    moveItemToTop( *it );
  }

  //update all positions
  updateZValues();
  update();
}

void QgsComposition::moveItemToTop( QgsComposerItem* item )
{
  //search item
  QMutableLinkedListIterator<QgsComposerItem*> it( mItemZList );
  if ( it.findNext( item ) )
  {
    it.remove();
  }
  mItemZList.push_back( item );
}

void QgsComposition::moveSelectedItemsToBottom()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  for ( ; it != selectedItems.end(); ++it )
  {
    moveItemToBottom( *it );
  }

  //update all positions
  updateZValues();
  update();
}

void QgsComposition::moveItemToBottom( QgsComposerItem* item )
{
  //search item
  QMutableLinkedListIterator<QgsComposerItem*> it( mItemZList );
  if ( it.findNext( item ) )
  {
    it.remove();
  }
  mItemZList.push_front( item );
}

void QgsComposition::alignSelectedItemsLeft()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 2 )
  {
    return;
  }

  QRectF selectedItemBBox;
  if ( boundingRectOfSelectedItems( selectedItemBBox ) != 0 )
  {
    return;
  }

  double minXCoordinate = selectedItemBBox.left();

  //align items left to minimum x coordinate
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items left" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    QTransform itemTransform = ( *align_it )->transform();
    itemTransform.translate( minXCoordinate - itemTransform.dx(), 0 );
    ( *align_it )->setTransform( itemTransform );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::alignSelectedItemsHCenter()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 2 )
  {
    return;
  }

  QRectF selectedItemBBox;
  if ( boundingRectOfSelectedItems( selectedItemBBox ) != 0 )
  {
    return;
  }

  double averageXCoord = ( selectedItemBBox.left() + selectedItemBBox.right() ) / 2.0;

  //place items
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items hcenter" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    QTransform itemTransform = ( *align_it )->transform();
    itemTransform.translate( averageXCoord - itemTransform.dx() - ( *align_it )->rect().width() / 2.0, 0 );
    ( *align_it )->setTransform( itemTransform );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::alignSelectedItemsRight()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 2 )
  {
    return;
  }

  QRectF selectedItemBBox;
  if ( boundingRectOfSelectedItems( selectedItemBBox ) != 0 )
  {
    return;
  }

  double maxXCoordinate = selectedItemBBox.right();

  //align items right to maximum x coordinate
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items right" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    QTransform itemTransform = ( *align_it )->transform();
    itemTransform.translate( maxXCoordinate - itemTransform.dx() - ( *align_it )->rect().width(), 0 );
    ( *align_it )->setTransform( itemTransform );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::alignSelectedItemsTop()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 2 )
  {
    return;
  }

  QRectF selectedItemBBox;
  if ( boundingRectOfSelectedItems( selectedItemBBox ) != 0 )
  {
    return;
  }

  double minYCoordinate = selectedItemBBox.top();

  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items top" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    QTransform itemTransform = ( *align_it )->transform();
    itemTransform.translate( 0, minYCoordinate - itemTransform.dy() );
    ( *align_it )->setTransform( itemTransform );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::alignSelectedItemsVCenter()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 2 )
  {
    return;
  }

  QRectF selectedItemBBox;
  if ( boundingRectOfSelectedItems( selectedItemBBox ) != 0 )
  {
    return;
  }

  double averageYCoord = ( selectedItemBBox.top() + selectedItemBBox.bottom() ) / 2.0;
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items vcenter" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    QTransform itemTransform = ( *align_it )->transform();
    itemTransform.translate( 0, averageYCoord - itemTransform.dy() - ( *align_it )->rect().height() / 2 );
    ( *align_it )->setTransform( itemTransform );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::alignSelectedItemsBottom()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 2 )
  {
    return;
  }

  QRectF selectedItemBBox;
  if ( boundingRectOfSelectedItems( selectedItemBBox ) != 0 )
  {
    return;
  }

  double maxYCoord = selectedItemBBox.bottom();
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items bottom" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    QTransform itemTransform = ( *align_it )->transform();
    itemTransform.translate( 0, maxYCoord - itemTransform.dy() - ( *align_it )->rect().height() );
    ( *align_it )->setTransform( itemTransform );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::updateZValues()
{
  int counter = 1;
  QLinkedList<QgsComposerItem*>::iterator it = mItemZList.begin();
  QgsComposerItem* currentItem = 0;

  QUndoCommand* parentCommand = new QUndoCommand( tr( "Item z-order changed" ) );
  for ( ; it != mItemZList.end(); ++it )
  {
    currentItem = *it;
    if ( currentItem )
    {
      QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *it, "", parentCommand );
      subcommand->savePreviousState();
      currentItem->setZValue( counter );
      subcommand->saveAfterState();
    }
    ++counter;
  }
  mUndoStack.push( parentCommand );
}

void QgsComposition::sortZList()
{
  if ( mItemZList.size() < 2 )
  {
    return;
  }

  QLinkedList<QgsComposerItem*>::const_iterator lIt = mItemZList.constBegin();
  QLinkedList<QgsComposerItem*> sortedList;

  for ( ; lIt != mItemZList.constEnd(); ++lIt )
  {
    QLinkedList<QgsComposerItem*>::iterator insertIt = sortedList.begin();
    for ( ; insertIt != sortedList.end(); ++insertIt )
    {
      if (( *lIt )->zValue() < ( *insertIt )->zValue() )
      {
        break;
      }
    }
    sortedList.insert( insertIt, ( *lIt ) );
  }

  mItemZList = sortedList;
}

QPointF QgsComposition::snapPointToGrid( const QPointF& scenePoint ) const
{
  if ( !mSnapToGrid || mSnapGridResolution <= 0 )
  {
    return scenePoint;
  }

  //snap x coordinate //todo: add support for x- and y- offset
  int xRatio = ( int )(( scenePoint.x() - mSnapGridOffsetX ) / mSnapGridResolution + 0.5 );
  int yRatio = ( int )(( scenePoint.y() - mSnapGridOffsetY ) / mSnapGridResolution + 0.5 );

  return QPointF( xRatio * mSnapGridResolution + mSnapGridOffsetX, yRatio * mSnapGridResolution + mSnapGridOffsetY );
}

int QgsComposition::boundingRectOfSelectedItems( QRectF& bRect )
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() < 1 )
  {
    return 1;
  }

  //set the box to the first item
  QgsComposerItem* currentItem = selectedItems.at( 0 );
  double minX = currentItem->transform().dx();
  double minY = currentItem->transform().dy();
  double maxX = minX + currentItem->rect().width();
  double maxY = minY + currentItem->rect().height();

  double currentMinX, currentMinY, currentMaxX, currentMaxY;

  for ( int i = 1; i < selectedItems.size(); ++i )
  {
    currentItem = selectedItems.at( i );
    currentMinX = currentItem->transform().dx();
    currentMinY = currentItem->transform().dy();
    currentMaxX = currentMinX + currentItem->rect().width();
    currentMaxY = currentMinY + currentItem->rect().height();

    if ( currentMinX < minX )
      minX = currentMinX;
    if ( currentMaxX > maxX )
      maxX = currentMaxX;
    if ( currentMinY < minY )
      minY = currentMinY;
    if ( currentMaxY > maxY )
      maxY = currentMaxY;
  }

  bRect.setTopLeft( QPointF( minX, minY ) );
  bRect.setBottomRight( QPointF( maxX, maxY ) );
  return 0;
}

void QgsComposition::setSnapToGridEnabled( bool b )
{
  mSnapToGrid = b;
  if ( mPaperItem )
  {
    mPaperItem->update();
  }
}

void QgsComposition::setSnapGridResolution( double r )
{
  mSnapGridResolution = r;
  if ( mPaperItem )
  {
    mPaperItem->update();
  }
}

void QgsComposition::setSnapGridOffsetX( double offset )
{
  mSnapGridOffsetX = offset;
  if ( mPaperItem )
  {
    mPaperItem->update();
  }
}

void QgsComposition::setSnapGridOffsetY( double offset )
{
  mSnapGridOffsetY = offset;
  if ( mPaperItem )
  {
    mPaperItem->update();
  }
}

void QgsComposition::setGridPen( const QPen& p )
{
  mGridPen = p;
  if ( mPaperItem )
  {
    mPaperItem->update();
  }
  saveGridAppearanceSettings();
}

void QgsComposition::setGridStyle( GridStyle s )
{
  mGridStyle = s;
  if ( mPaperItem )
  {
    mPaperItem->update();
  }
  saveGridAppearanceSettings();
}

void QgsComposition::setPrintAsRaster( bool enabled )
{
  mPrintAsRaster = enabled;
  QSettings s;
  s.setValue( "/qgis/composerPrintAsRaster", QVariant( mPrintAsRaster ) );
}

void QgsComposition::loadGridAppearanceSettings()
{
  //read grid style, grid color and pen width from settings
  QSettings s;

  QString gridStyleString;
  int red, green, blue;
  double penWidth;

  gridStyleString = s.value( "/qgis/composerGridStyle", "Dots" ).toString();
  penWidth = s.value( "/qgis/composerGridWidth", 0.5 ).toDouble();
  red = s.value( "/qgis/composerGridRed", 0 ).toInt();
  green = s.value( "/qgis/composerGridGreen", 0 ).toInt();
  blue = s.value( "/qgis/composerGridBlue", 0 ).toInt();

  mGridPen.setColor( QColor( red, green, blue ) );
  mGridPen.setWidthF( penWidth );

  if ( gridStyleString == "Dots" )
  {
    mGridStyle = Dots;
  }
  else if ( gridStyleString == "Crosses" )
  {
    mGridStyle = Crosses;
  }
  else
  {
    mGridStyle = Solid;
  }
}

void QgsComposition::saveGridAppearanceSettings()
{
  //store grid appearance settings
  QSettings s;
  s.setValue( "/qgis/composerGridWidth", mGridPen.widthF() );
  s.setValue( "/qgis/composerGridRed", mGridPen.color().red() );
  s.setValue( "/qgis/composerGridGreen", mGridPen.color().green() );
  s.setValue( "/qgis/composerGridBlue", mGridPen.color().blue() );

  if ( mGridStyle == Solid )
  {
    s.setValue( "/qgis/composerGridStyle", "Solid" );
  }
  else if ( mGridStyle == Dots )
  {
    s.setValue( "/qgis/composerGridStyle", "Dots" );
  }
  else if ( mGridStyle == Crosses )
  {
    s.setValue( "/qgis/composerGridStyle", "Crosses" );
  }
}

void QgsComposition::beginCommand( QgsComposerItem* item, const QString& commandText, QgsComposerMergeCommand::Context c )
{
  delete mActiveCommand;
  if ( !item )
  {
    mActiveCommand = 0;
    return;
  }

  if ( c == QgsComposerMergeCommand::Unknown )
  {
    mActiveCommand = new QgsComposerItemCommand( item, commandText );
  }
  else
  {
    mActiveCommand = new QgsComposerMergeCommand( c, item, commandText );
  }
  mActiveCommand->savePreviousState();
}

void QgsComposition::endCommand()
{
  if ( mActiveCommand )
  {
    mActiveCommand->saveAfterState();
    if ( mActiveCommand->containsChange() ) //protect against empty commands
    {
      mUndoStack.push( mActiveCommand );
    }
    else
    {
      delete mActiveCommand;
    }
    mActiveCommand = 0;
  }
}

void QgsComposition::cancelCommand()
{
  delete mActiveCommand;
  mActiveCommand = 0;
}
