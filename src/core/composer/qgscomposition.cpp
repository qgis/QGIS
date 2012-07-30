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
#include "qgscomposerarrow.h"
#include "qgscomposerframe.h"
#include "qgscomposerhtml.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgscomposerattributetable.h"
#include "qgslogger.h"
#include "qgspaintenginehack.h"
#include "qgspaperitem.h"
#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QPrinter>
#include <QSettings>

QgsComposition::QgsComposition( QgsMapRenderer* mapRenderer ):
    QGraphicsScene( 0 ), mMapRenderer( mapRenderer ), mPlotStyle( QgsComposition::Preview ), mPageWidth( 297 ), mPageHeight( 210 ), mSpaceBetweenPages( 10 ), mPrintAsRaster( false ), mSelectionTolerance( 0.0 ),
    mSnapToGrid( false ), mSnapGridResolution( 0.0 ), mSnapGridOffsetX( 0.0 ), mSnapGridOffsetY( 0.0 ), mActiveCommand( 0 )
{
  setBackgroundBrush( Qt::gray );
  addPaperItem();

  mPrintResolution = 300; //hardcoded default
  loadSettings();
}

QgsComposition::QgsComposition():
    QGraphicsScene( 0 ), mMapRenderer( 0 ), mPlotStyle( QgsComposition::Preview ),  mPageWidth( 297 ), mPageHeight( 210 ), mSpaceBetweenPages( 10 ), mPrintAsRaster( false ),
    mSelectionTolerance( 0.0 ), mSnapToGrid( false ), mSnapGridResolution( 0.0 ), mSnapGridOffsetX( 0.0 ), mSnapGridOffsetY( 0.0 ), mActiveCommand( 0 )
{
  loadSettings();
}

QgsComposition::~QgsComposition()
{
  removePaperItems();
  // make sure that all composer items are removed before
  // this class is deconstructed - to avoid segfaults
  // when composer items access in destructor composition that isn't valid anymore
  clear();
}

void QgsComposition::setPaperSize( double width, double height )
{
  mPageWidth = width;
  mPageHeight = height;
  double currentY = 0;
  for ( int i = 0; i < mPages.size(); ++i )
  {
    mPages.at( i )->setSceneRect( QRectF( 0, currentY, width, height ) );
    currentY += ( height + mSpaceBetweenPages );
  }
}

double QgsComposition::paperHeight() const
{
  return mPageHeight;
}

double QgsComposition::paperWidth() const
{
  return mPageWidth;
}

void QgsComposition::setNumPages( int pages )
{
  int currentPages = numPages();
  int diff = pages - currentPages;
  if ( diff >= 0 )
  {
    for ( int i = 0; i < diff; ++i )
    {
      addPaperItem();
    }
  }
  else
  {
    diff = -diff;
    for ( int i = 0; i < diff; ++i )
    {
      delete mPages.last();
      mPages.removeLast();
    }
  }
}

int QgsComposition::numPages() const
{
  return mPages.size();
}

QgsComposerItem* QgsComposition::composerItemAt( const QPointF & position )
{
  QList<QGraphicsItem*> itemList;
  if ( mSelectionTolerance <= 0.0 )
  {
    itemList = items( position );
  }
  else
  {
    itemList = items( QRectF( position.x() - mSelectionTolerance, position.y() - mSelectionTolerance, 2 * mSelectionTolerance, 2 * mSelectionTolerance ),
                      Qt::IntersectsItemShape, Qt::DescendingOrder );
  }
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();

  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    QgsPaperItem* paperItem = dynamic_cast<QgsPaperItem*>( *itemIt );
    if ( composerItem && !paperItem )
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
  compositionElem.setAttribute( "paperWidth", QString::number( mPageWidth ) );
  compositionElem.setAttribute( "paperHeight", QString::number( mPageHeight ) );
  compositionElem.setAttribute( "numPages", mPages.size() );

  //snapping
  if ( mSnapToGrid )
  {
    compositionElem.setAttribute( "snapping", "1" );
  }
  else
  {
    compositionElem.setAttribute( "snapping", "0" );
  }
  compositionElem.setAttribute( "snapGridResolution", QString::number( mSnapGridResolution ) );
  compositionElem.setAttribute( "snapGridOffsetX", QString::number( mSnapGridOffsetX ) );
  compositionElem.setAttribute( "snapGridOffsetY", QString::number( mSnapGridOffsetY ) );

  compositionElem.setAttribute( "printResolution", mPrintResolution );
  compositionElem.setAttribute( "printAsRaster", mPrintAsRaster );

  //save items except paper items and frame items (they are saved with the corresponding multiframe)
  QList<QGraphicsItem*> itemList = items();
  QList<QGraphicsItem*>::const_iterator itemIt = itemList.constBegin();
  for ( ; itemIt != itemList.constEnd(); ++itemIt )
  {
    const QgsComposerItem* composerItem = dynamic_cast<const QgsComposerItem*>( *itemIt );
    if ( composerItem )
    {
      if ( composerItem->type() != QgsComposerItem::ComposerPaper &&  composerItem->type() != QgsComposerItem::ComposerFrame )
      {
        composerItem->writeXML( compositionElem, doc );
      }
    }
  }

  //save multiframes
  QSet<QgsComposerMultiFrame*>::const_iterator multiFrameIt = mMultiFrames.constBegin();
  for ( ; multiFrameIt != mMultiFrames.constEnd(); ++multiFrameIt )
  {
    ( *multiFrameIt )->writeXML( compositionElem, doc );
  }

  composerElem.appendChild( compositionElem );

  return true;
}

bool QgsComposition::readXML( const QDomElement& compositionElem, const QDomDocument& doc )
{
  Q_UNUSED( doc );
  if ( compositionElem.isNull() )
  {
    return false;
  }

  //create pages
  bool widthConversionOk, heightConversionOk;
  mPageWidth = compositionElem.attribute( "paperWidth" ).toDouble( &widthConversionOk );
  mPageHeight = compositionElem.attribute( "paperHeight" ).toDouble( &heightConversionOk );
  int numPages = compositionElem.attribute( "numPages", "1" ).toInt();

  if ( widthConversionOk && heightConversionOk )
  {
    removePaperItems();
    for ( int i = 0; i < numPages; ++i )
    {
      addPaperItem();
    }
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
  mPrintAsRaster = compositionElem.attribute( "printAsRaster" ).toInt();

  mPrintResolution = compositionElem.attribute( "printResolution", "300" ).toInt();
  updatePaperItems();
  return true;
}

void QgsComposition::addItemsFromXML( const QDomElement& elem, const QDomDocument& doc, QMap< QgsComposerMap*, int >* mapsToRestore,
                                      bool addUndoCommands, QPointF* pos )
{
  QDomNodeList composerLabelList = elem.elementsByTagName( "ComposerLabel" );
  for ( int i = 0; i < composerLabelList.size(); ++i )
  {
    QDomElement currentComposerLabelElem = composerLabelList.at( i ).toElement();
    QgsComposerLabel* newLabel = new QgsComposerLabel( this );
    newLabel->readXML( currentComposerLabelElem, doc );
    if ( pos )
    {
      newLabel->setItemPosition( pos->x(), pos->y() );
    }
    addComposerLabel( newLabel );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newLabel, tr( "Label added" ) );
    }
  }
  // map
  QDomNodeList composerMapList = elem.elementsByTagName( "ComposerMap" );
  for ( int i = 0; i < composerMapList.size(); ++i )
  {
    QDomElement currentComposerMapElem = composerMapList.at( i ).toElement();
    QgsComposerMap* newMap = new QgsComposerMap( this );
    newMap->readXML( currentComposerMapElem, doc );
    newMap->assignFreeId();

    if ( mapsToRestore )
    {
      mapsToRestore->insert( newMap, ( int )( newMap->previewMode() ) );
      newMap->setPreviewMode( QgsComposerMap::Rectangle );
    }
    addComposerMap( newMap, false );

    if ( pos )
    {
      newMap->setItemPosition( pos->x(), pos->y() );
    }

    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newMap, tr( "Map added" ) );
    }
  }
  // arrow
  QDomNodeList composerArrowList = elem.elementsByTagName( "ComposerArrow" );
  for ( int i = 0; i < composerArrowList.size(); ++i )
  {
    QDomElement currentComposerArrowElem = composerArrowList.at( i ).toElement();
    QgsComposerArrow* newArrow = new QgsComposerArrow( this );
    newArrow->readXML( currentComposerArrowElem, doc );
    if ( pos )
    {
      newArrow->setItemPosition( pos->x(), pos->y() );
    }
    addComposerArrow( newArrow );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newArrow, tr( "Arrow added" ) );
    }
  }
  // scalebar
  QDomNodeList composerScaleBarList = elem.elementsByTagName( "ComposerScaleBar" );
  for ( int i = 0; i < composerScaleBarList.size(); ++i )
  {
    QDomElement currentComposerScaleBarElem = composerScaleBarList.at( i ).toElement();
    QgsComposerScaleBar* newScaleBar = new QgsComposerScaleBar( this );
    newScaleBar->readXML( currentComposerScaleBarElem, doc );
    if ( pos )
    {
      newScaleBar->setItemPosition( pos->x(), pos->y() );
    }
    addComposerScaleBar( newScaleBar );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newScaleBar, tr( "Scale bar added" ) );
    }
  }
  // shape
  QDomNodeList composerShapeList = elem.elementsByTagName( "ComposerShape" );
  for ( int i = 0; i < composerShapeList.size(); ++i )
  {
    QDomElement currentComposerShapeElem = composerShapeList.at( i ).toElement();
    QgsComposerShape* newShape = new QgsComposerShape( this );
    newShape->readXML( currentComposerShapeElem, doc );
    if ( pos )
    {
      newShape->setItemPosition( pos->x(), pos->y() );
    }
    addComposerShape( newShape );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newShape, tr( "Shape added" ) );
    }
  }
  // picture
  QDomNodeList composerPictureList = elem.elementsByTagName( "ComposerPicture" );
  for ( int i = 0; i < composerPictureList.size(); ++i )
  {
    QDomElement currentComposerPictureElem = composerPictureList.at( i ).toElement();
    QgsComposerPicture* newPicture = new QgsComposerPicture( this );
    newPicture->readXML( currentComposerPictureElem, doc );
    if ( pos )
    {
      newPicture->setItemPosition( pos->x(), pos->y() );
    }
    addComposerPicture( newPicture );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newPicture, tr( "Picture added" ) );
    }
  }
  // legend
  QDomNodeList composerLegendList = elem.elementsByTagName( "ComposerLegend" );
  for ( int i = 0; i < composerLegendList.size(); ++i )
  {
    QDomElement currentComposerLegendElem = composerLegendList.at( i ).toElement();
    QgsComposerLegend* newLegend = new QgsComposerLegend( this );
    newLegend->readXML( currentComposerLegendElem, doc );
    if ( pos )
    {
      newLegend->setItemPosition( pos->x(), pos->y() );
    }
    addComposerLegend( newLegend );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newLegend, tr( "Legend added" ) );
    }
  }
  // table
  QDomNodeList composerTableList = elem.elementsByTagName( "ComposerAttributeTable" );
  for ( int i = 0; i < composerTableList.size(); ++i )
  {
    QDomElement currentComposerTableElem = composerTableList.at( i ).toElement();
    QgsComposerAttributeTable* newTable = new QgsComposerAttributeTable( this );
    newTable->readXML( currentComposerTableElem, doc );
    if ( pos )
    {
      newTable->setItemPosition( pos->x(), pos->y() );
    }
    addComposerTable( newTable );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newTable, tr( "Table added" ) );
    }
  }
  //html
  QDomNodeList composerHtmlList = elem.elementsByTagName( "ComposerHtml" );
  for ( int i = 0; i < composerHtmlList.size(); ++i )
  {
    QDomElement currentHtmlElem = composerHtmlList.at( i ).toElement();
    QgsComposerHtml* newHtml = new QgsComposerHtml( this, 0, 0, 0, 0 );
    newHtml->readXML( currentHtmlElem, doc );
    this->addMultiFrame( newHtml );
  }
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

  //y offset to current page
  int pageNr = ( int )( scenePoint.y() / ( mPageHeight + mSpaceBetweenPages ) );
  double yOffset = pageNr * ( mPageHeight + mSpaceBetweenPages );
  double yPage = scenePoint.y() - yOffset; //y-coordinate relative to current page

  //snap x coordinate
  int xRatio = ( int )(( scenePoint.x() - mSnapGridOffsetX ) / mSnapGridResolution + 0.5 );
  int yRatio = ( int )(( yPage - mSnapGridOffsetY ) / mSnapGridResolution + 0.5 );

  return QPointF( xRatio * mSnapGridResolution + mSnapGridOffsetX, yRatio * mSnapGridResolution + mSnapGridOffsetY + yOffset );
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
  updatePaperItems();
  saveSettings();
}

void QgsComposition::setSnapGridResolution( double r )
{
  mSnapGridResolution = r;
  updatePaperItems();
  saveSettings();
}

void QgsComposition::setSnapGridOffsetX( double offset )
{
  mSnapGridOffsetX = offset;
  updatePaperItems();
  saveSettings();
}

void QgsComposition::setSnapGridOffsetY( double offset )
{
  mSnapGridOffsetY = offset;
  updatePaperItems();
  saveSettings();
}

void QgsComposition::setGridPen( const QPen& p )
{
  mGridPen = p;
  updatePaperItems();
  saveSettings();
}

void QgsComposition::setGridStyle( GridStyle s )
{
  mGridStyle = s;
  updatePaperItems();
  saveSettings();
}

void QgsComposition::setSelectionTolerance( double tol )
{
  mSelectionTolerance = tol;
  saveSettings();
}

void QgsComposition::loadSettings()
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

  mSelectionTolerance = s.value( "/qgis/composerSelectionTolerance", 0.0 ).toDouble();
}

void QgsComposition::saveSettings()
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

  //store also selection tolerance
  s.setValue( "/qgis/composerSelectionTolerance", mSelectionTolerance );
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

void QgsComposition::removeMultiFrame( QgsComposerMultiFrame* multiFrame )
{
  mMultiFrames.remove( multiFrame );
  delete multiFrame; //e.v. use deleteLater() in case of stability problems
}

void QgsComposition::addComposerArrow( QgsComposerArrow* arrow )
{
  addItem( arrow );
  emit composerArrowAdded( arrow );
  clearSelection();
  arrow->setSelected( true );
  emit selectedItemChanged( arrow );
}

void QgsComposition::addComposerLabel( QgsComposerLabel* label )
{
  addItem( label );
  emit composerLabelAdded( label );
  clearSelection();
  label->setSelected( true );
  emit selectedItemChanged( label );
}

void QgsComposition::addComposerMap( QgsComposerMap* map, bool setDefaultPreviewStyle )
{
  addItem( map );
  if ( setDefaultPreviewStyle )
  {
    //set default preview mode to cache. Must be done here between adding composer map to scene and emiting signal
    map->setPreviewMode( QgsComposerMap::Cache );
  }

  if ( map->previewMode() != QgsComposerMap::Rectangle )
  {
    map->cache();
  }

  emit composerMapAdded( map );
  clearSelection();
  map->setSelected( true );
  emit selectedItemChanged( map );
}

void QgsComposition::addComposerScaleBar( QgsComposerScaleBar* scaleBar )
{
  //take first available map
  QList<const QgsComposerMap*> mapItemList = composerMapItems();
  if ( mapItemList.size() > 0 )
  {
    scaleBar->setComposerMap( mapItemList.at( 0 ) );
  }
  addItem( scaleBar );
  emit composerScaleBarAdded( scaleBar );
  clearSelection();
  scaleBar->setSelected( true );
  emit selectedItemChanged( scaleBar );
}

void QgsComposition::addComposerLegend( QgsComposerLegend* legend )
{
  //take first available map
  QList<const QgsComposerMap*> mapItemList = composerMapItems();
  if ( mapItemList.size() > 0 )
  {
    legend->setComposerMap( mapItemList.at( 0 ) );
  }
  addItem( legend );
  emit composerLegendAdded( legend );
  clearSelection();
  legend->setSelected( true );
  emit selectedItemChanged( legend );
}

void QgsComposition::addComposerPicture( QgsComposerPicture* picture )
{
  addItem( picture );
  emit composerPictureAdded( picture );
  clearSelection();
  picture->setSelected( true );
  emit selectedItemChanged( picture );
}

void QgsComposition::addComposerShape( QgsComposerShape* shape )
{
  addItem( shape );
  emit composerShapeAdded( shape );
  clearSelection();
  shape->setSelected( true );
  emit selectedItemChanged( shape );
}

void QgsComposition::addComposerTable( QgsComposerAttributeTable* table )
{
  addItem( table );
  emit composerTableAdded( table );
  clearSelection();
  table->setSelected( true );
  emit selectedItemChanged( table );
}

void QgsComposition::addComposerHtmlFrame( QgsComposerHtml* html, QgsComposerFrame* frame )
{
  addItem( frame );
  emit composerHtmlFrameAdded( html, frame );
  clearSelection();
  frame->setSelected( true );
  emit selectedItemChanged( frame );
}

void QgsComposition::removeComposerItem( QgsComposerItem* item )
{
  QgsComposerMap* map = dynamic_cast<QgsComposerMap *>( item );
  if ( !map || !map->isDrawing() ) //don't delete a composer map while it draws
  {
    removeItem( item );
    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup*>( item );
    if ( itemGroup )
    {
      //add add/remove item command for every item in the group
      QUndoCommand* parentCommand = new QUndoCommand( tr( "Remove item group" ) );

      QSet<QgsComposerItem*> groupedItems = itemGroup->items();
      QSet<QgsComposerItem*>::iterator it = groupedItems.begin();
      for ( ; it != groupedItems.end(); ++it )
      {
        QgsAddRemoveItemCommand* subcommand = new QgsAddRemoveItemCommand( QgsAddRemoveItemCommand::Removed, *it, this, "", parentCommand );
        connectAddRemoveCommandSignals( subcommand );
        emit itemRemoved( *it );
      }

      undoStack()->push( parentCommand );
      delete itemGroup;
      emit itemRemoved( itemGroup );
    }
    else
    {
      emit itemRemoved( item );
      pushAddRemoveCommand( item, tr( "Item deleted" ), QgsAddRemoveItemCommand::Removed );
    }
  }
}

void QgsComposition::pushAddRemoveCommand( QgsComposerItem* item, const QString& text, QgsAddRemoveItemCommand::State state )
{
  QgsAddRemoveItemCommand* c = new QgsAddRemoveItemCommand( state, item, this, text );
  connectAddRemoveCommandSignals( c );
  undoStack()->push( c );
}

void QgsComposition::connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c )
{
  if ( !c )
  {
    return;
  }

  QObject::connect( c, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SIGNAL( itemRemoved( QgsComposerItem* ) ) );
  QObject::connect( c, SIGNAL( itemAdded( QgsComposerItem* ) ), this, SLOT( sendItemAddedSignal( QgsComposerItem* ) ) );
}

void QgsComposition::sendItemAddedSignal( QgsComposerItem* item )
{
  //cast and send proper signal
  item->setSelected( true );
  QgsComposerArrow* arrow = dynamic_cast<QgsComposerArrow*>( item );
  if ( arrow )
  {
    emit composerArrowAdded( arrow );
    emit selectedItemChanged( arrow );
    return;
  }
  QgsComposerLabel* label = dynamic_cast<QgsComposerLabel*>( item );
  if ( label )
  {
    emit composerLabelAdded( label );
    emit selectedItemChanged( label );
    return;
  }
  QgsComposerMap* map = dynamic_cast<QgsComposerMap*>( item );
  if ( map )
  {
    emit composerMapAdded( map );
    emit selectedItemChanged( map );
    return;
  }
  QgsComposerScaleBar* scalebar = dynamic_cast<QgsComposerScaleBar*>( item );
  if ( scalebar )
  {
    emit composerScaleBarAdded( scalebar );
    emit selectedItemChanged( scalebar );
    return;
  }
  QgsComposerLegend* legend = dynamic_cast<QgsComposerLegend*>( item );
  if ( legend )
  {
    emit composerLegendAdded( legend );
    emit selectedItemChanged( legend );
    return;
  }
  QgsComposerPicture* picture = dynamic_cast<QgsComposerPicture*>( item );
  if ( picture )
  {
    emit composerPictureAdded( picture );
    emit selectedItemChanged( picture );
    return;
  }
  QgsComposerShape* shape = dynamic_cast<QgsComposerShape*>( item );
  if ( shape )
  {
    emit composerShapeAdded( shape );
    emit selectedItemChanged( shape );
    return;
  }
  QgsComposerAttributeTable* table = dynamic_cast<QgsComposerAttributeTable*>( item );
  if ( table )
  {
    emit composerTableAdded( table );
    emit selectedItemChanged( table );
    return;
  }
}

void QgsComposition::updatePaperItems()
{
  QList< QgsPaperItem* >::iterator paperIt = mPages.begin();
  for ( ; paperIt != mPages.end(); ++paperIt )
  {
    ( *paperIt )->update();
  }
}

void QgsComposition::addPaperItem()
{
  double paperHeight = this->paperHeight();
  double paperWidth = this->paperWidth();
  double currentY = paperHeight * mPages.size() + mPages.size() * mSpaceBetweenPages; //add 10mm visible space between pages
  QgsPaperItem* paperItem = new QgsPaperItem( 0, currentY, paperWidth, paperHeight, this ); //default size A4
  paperItem->setBrush( Qt::white );
  addItem( paperItem );
  paperItem->setZValue( 0 );
  mPages.push_back( paperItem );
}

void QgsComposition::removePaperItems()
{
  for ( int i = 0; i < mPages.size(); ++i )
  {
    delete mPages.at( i );
  }
  mPages.clear();
}

void QgsComposition::exportAsPDF( const QString& file )
{
  QPrinter printer;
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( file );
  printer.setPaperSize( QSizeF( paperWidth(), paperHeight() ), QPrinter::Millimeter );

  QgsPaintEngineHack::fixEngineFlags( printer.paintEngine() );
  print( printer );
}

void QgsComposition::print( QPrinter &printer )
{
  //set resolution based on composer setting
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );

  //set user-defined resolution
  printer.setResolution( printResolution() );

  QPainter p( &printer );

  //QgsComposition starts page numbering at 0
  int fromPage = ( printer.fromPage() < 1 ) ? 0 : printer.fromPage() - 1 ;
  int toPage = ( printer.toPage() < 1 ) ? numPages() - 1 : printer.toPage() - 1;

  if ( mPrintAsRaster )
  {
    for ( int i = fromPage; i <= toPage; ++i )
    {
      if ( i > fromPage )
      {
        printer.newPage();
      }

      QImage image = printPageAsRaster( i );
      if ( !image.isNull() )
      {
        QRectF targetArea( 0, 0, image.width(), image.height() );
        p.drawImage( targetArea, image, targetArea );
      }
    }
  }

  if ( !mPrintAsRaster )
  {
    for ( int i = fromPage; i <= toPage; ++i )
    {
      if ( i > fromPage )
      {
        printer.newPage();
      }
      renderPage( &p, i );
    }
  }
}

QImage QgsComposition::printPageAsRaster( int page )
{
  //print out via QImage, code copied from on_mActionExportAsImage_activated
  int width = ( int )( printResolution() * paperWidth() / 25.4 );
  int height = ( int )( printResolution() * paperHeight() / 25.4 );
  QImage image( QSize( width, height ), QImage::Format_ARGB32 );
  if ( !image.isNull() )
  {
    image.setDotsPerMeterX( printResolution() / 25.4 * 1000 );
    image.setDotsPerMeterY( printResolution() / 25.4 * 1000 );
    image.fill( 0 );
    QPainter imagePainter( &image );
    renderPage( &imagePainter, page );
  }
  return image;
}

void QgsComposition::renderPage( QPainter* p, int page )
{
  if ( mPages.size() <= page )
  {
    return;
  }

  QgsPaperItem* paperItem = mPages[page];
  if ( !paperItem )
  {
    return;
  }

  QPaintDevice* paintDevice = p->device();
  if ( !paintDevice )
  {
    return;
  }

  QRectF paperRect = QRectF( paperItem->transform().dx(), paperItem->transform().dy(), paperItem->rect().width(), paperItem->rect().height() );

  QgsComposition::PlotStyle savedPlotStyle = mPlotStyle;
  mPlotStyle = QgsComposition::Print;

  render( p, QRectF( 0, 0, paintDevice->width(), paintDevice->height() ), paperRect );

  mPlotStyle = savedPlotStyle;
}
