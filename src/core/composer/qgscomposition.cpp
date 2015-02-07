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
#include "qgscomposerutils.h"
#include "qgscomposerarrow.h"
#include "qgscomposerframe.h"
#include "qgscomposerhtml.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposermapoverview.h"
#include "qgscomposermousehandles.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgscomposerlabel.h"
#include "qgscomposermodel.h"
#include "qgscomposerattributetable.h"
#include "qgscomposerattributetablev2.h"
#include "qgsaddremovemultiframecommand.h"
#include "qgscomposermultiframecommand.h"
#include "qgspaintenginehack.h"
#include "qgspaperitem.h"
#include "qgsproject.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsdatadefined.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPainter>
#include <QPrinter>
#include <QSettings>
#include <QDir>

#include <limits>

QgsComposition::QgsComposition( QgsMapRenderer* mapRenderer )
    : QGraphicsScene( 0 )
    , mMapRenderer( mapRenderer )
    , mMapSettings( mapRenderer->mapSettings() )
    , mAtlasComposition( this )
{
  init();
}

QgsComposition::QgsComposition( const QgsMapSettings& mapSettings )
    : QGraphicsScene( 0 )
    , mMapRenderer( 0 )
    , mMapSettings( mapSettings )
    , mAtlasComposition( this )
{
  init();
}

void QgsComposition::init()
{
  // these members should be ideally in constructor's initialization list, but now we have two constructors...
  mPlotStyle = QgsComposition::Preview;
  mPageWidth = 297;
  mPageHeight = 210;
  mSpaceBetweenPages = 10;
  mPageStyleSymbol = 0;
  mPrintAsRaster = false;
  mGenerateWorldFile = false;
  mWorldFileMap = 0;
  mUseAdvancedEffects = true;
  mSnapToGrid = false;
  mGridVisible = false;
  mSnapGridResolution = 0;
  mSnapGridOffsetX = 0;
  mSnapGridOffsetY = 0;
  mAlignmentSnap = true;
  mGuidesVisible = true;
  mSmartGuides = true;
  mSnapTolerance = 0;
  mBoundingBoxesVisible = true;
  mSelectionHandles = 0;
  mActiveItemCommand = 0;
  mActiveMultiFrameCommand = 0;
  mAtlasMode = QgsComposition::AtlasOff;
  mPreventCursorChange = false;
  mItemsModel = 0;
  mUndoStack = new QUndoStack();

  //data defined strings
  mDataDefinedNames.insert( QgsComposerObject::PresetPaperSize, QString( "dataDefinedPaperSize" ) );
  mDataDefinedNames.insert( QgsComposerObject::PaperWidth, QString( "dataDefinedPaperWidth" ) );
  mDataDefinedNames.insert( QgsComposerObject::PaperHeight, QString( "dataDefinedPaperHeight" ) );
  mDataDefinedNames.insert( QgsComposerObject::NumPages, QString( "dataDefinedNumPages" ) );
  mDataDefinedNames.insert( QgsComposerObject::PaperOrientation, QString( "dataDefinedPaperOrientation" ) );

  //connect to atlas toggling on/off and coverage layer and feature changes
  //to update data defined values
  connect( &mAtlasComposition, SIGNAL( toggled( bool ) ), this, SLOT( refreshDataDefinedProperty() ) );
  connect( &mAtlasComposition, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ), this, SLOT( refreshDataDefinedProperty() ) );
  connect( &mAtlasComposition, SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( refreshDataDefinedProperty() ) );
  //also, refreshing composition triggers a recalculation of data defined properties
  connect( this, SIGNAL( refreshItemsTriggered() ), this, SLOT( refreshDataDefinedProperty() ) );
  //toggling atlas or changing coverage layer requires data defined expressions to be reprepared
  connect( &mAtlasComposition, SIGNAL( toggled( bool ) ), this, SLOT( prepareAllDataDefinedExpressions() ) );
  connect( &mAtlasComposition, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ), this, SLOT( prepareAllDataDefinedExpressions() ) );

  setBackgroundBrush( QColor( 215, 215, 215 ) );
  createDefaultPageStyleSymbol();

  addPaperItem();

  updateBounds();

  //add mouse selection handles to composition, and initially hide
  mSelectionHandles = new QgsComposerMouseHandles( this );
  addItem( mSelectionHandles );
  mSelectionHandles->hide();
  mSelectionHandles->setZValue( 500 );

  mPrintResolution = 300; //hardcoded default

  //load default composition settings
  loadDefaults();
  loadSettings();

  mItemsModel = new QgsComposerModel( this );
}


/*
QgsComposition::QgsComposition()
    : QGraphicsScene( 0 ),
    mMapRenderer( 0 ),
    mPlotStyle( QgsComposition::Preview ),
    mPageWidth( 297 ),
    mPageHeight( 210 ),
    mSpaceBetweenPages( 10 ),
    mPageStyleSymbol( 0 ),
    mPrintAsRaster( false ),
    mGenerateWorldFile( false ),
    mWorldFileMap( 0 ),
    mUseAdvancedEffects( true ),
    mSnapToGrid( false ),
    mGridVisible( false ),
    mSnapGridResolution( 0 ),
    mSnapGridTolerance( 0 ),
    mSnapGridOffsetX( 0 ),
    mSnapGridOffsetY( 0 ),
    mAlignmentSnap( true ),
    mGuidesVisible( true ),
    mSmartGuides( true ),
    mAlignmentSnapTolerance( 0 ),
    mSelectionHandles( 0 ),
    mActiveItemCommand( 0 ),
    mActiveMultiFrameCommand( 0 ),
    mAtlasComposition( this ),
    mAtlasMode( QgsComposition::AtlasOff ),
    mPreventCursorChange( false ),
    mItemsModel( 0 )
{
  //load default composition settings
  loadDefaults();
  loadSettings();
  mItemsModel = new QgsComposerModel( this );
}*/

QgsComposition::~QgsComposition()
{
  removePaperItems();
  deleteAndRemoveMultiFrames();

  // make sure that all composer items are removed before
  // this class is deconstructed - to avoid segfaults
  // when composer items access in destructor composition that isn't valid anymore
  QList<QGraphicsItem*> itemList = items();
  qDeleteAll( itemList );

  // clear pointers to QgsDataDefined objects
  qDeleteAll( mDataDefinedProperties );
  mDataDefinedProperties.clear();

  //order is important here - we need to delete model last so that all items have already
  //been deleted. Deleting the undo stack will also delete any items which have been
  //removed from the scene, so this needs to be done before deleting the model
  delete mUndoStack;

  delete mActiveItemCommand;
  delete mActiveMultiFrameCommand;
  delete mPageStyleSymbol;
  delete mItemsModel;
}

void QgsComposition::loadDefaults()
{
  QSettings settings;
  mSnapGridResolution = settings.value( "/Composer/defaultSnapGridResolution", 10.0 ).toDouble();
  mSnapGridOffsetX = settings.value( "/Composer/defaultSnapGridOffsetX", 0 ).toDouble();
  mSnapGridOffsetY = settings.value( "/Composer/defaultSnapGridOffsetY", 0 ).toDouble();
  mSnapTolerance = settings.value( "/Composer/defaultSnapTolerancePixels", 5 ).toInt();
}

void QgsComposition::updateBounds()
{
  setSceneRect( compositionBounds() );
}

void QgsComposition::refreshItems()
{
  emit refreshItemsTriggered();
  //force a redraw on all maps
  QList<QgsComposerMap*> maps;
  composerItems( maps );
  QList<QgsComposerMap*>::iterator mapIt = maps.begin();
  for ( ; mapIt != maps.end(); ++mapIt )
  {
    ( *mapIt )->cache();
    ( *mapIt )->update();
  }
}

void QgsComposition::setSelectedItem( QgsComposerItem *item )
{
  setAllUnselected();
  if ( item )
  {
    item->setSelected( true );
    emit selectedItemChanged( item );
  }
}

void QgsComposition::setAllUnselected()
{
  //we can't use QGraphicsScene::clearSelection, as that emits no signals
  //and we don't know which items are being unselected
  //accordingly, we can't inform the composition model of selection changes
  //instead, do the clear selection manually...
  QList<QGraphicsItem *> selectedItemList = selectedItems();
  QList<QGraphicsItem *>::iterator itemIter = selectedItemList.begin();

  for ( ; itemIter != selectedItemList.end(); ++itemIter )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIter );
    if ( composerItem )
    {
      composerItem->setSelected( false );
    }
  }
}

void QgsComposition::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property )
{
  //updates data defined properties and redraws composition to match
  if ( property == QgsComposerObject::NumPages || property == QgsComposerObject::AllProperties )
  {
    setNumPages( numPages() );
  }
  if ( property == QgsComposerObject::PaperWidth || property == QgsComposerObject::PaperHeight ||
       property == QgsComposerObject::PaperOrientation || property == QgsComposerObject::PresetPaperSize ||
       property == QgsComposerObject::AllProperties )
  {
    refreshPageSize();
  }
}

QRectF QgsComposition::compositionBounds() const
{
  //start with an empty rectangle
  QRectF bounds = QRectF( 0, 0, 0, 0 );

  //add all QgsComposerItems and QgsPaperItems which are in the composition
  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    const QgsComposerItem* composerItem = dynamic_cast<const QgsComposerItem *>( *itemIt );
    const QgsPaperItem* paperItem = dynamic_cast<const QgsPaperItem*>( *itemIt );
    if (( composerItem || paperItem ) )
    {
      //expand bounds with current item's bounds
      bounds = bounds.united(( *itemIt )->sceneBoundingRect() );
    }
  }

  //finally, expand bounds out by 5% page size to give a bit of a margin
  bounds.adjust( -mPageWidth * 0.05, -mPageWidth * 0.05, mPageWidth * 0.05, mPageWidth * 0.05 );

  return bounds;
}

void QgsComposition::setPaperSize( const double width, const double height )
{
  if ( width == mPageWidth && height == mPageHeight )
  {
    return;
  }

  //update item positions
  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    if ( composerItem )
    {
      composerItem->updatePagePos( width, height );
    }
  }
  //update guide positions and size
  QList< QGraphicsLineItem* >* guides = snapLines();
  QList< QGraphicsLineItem* >::iterator guideIt = guides->begin();
  double totalHeight = ( height + spaceBetweenPages() ) * ( numPages() - 1 ) + height;
  for ( ; guideIt != guides->end(); ++guideIt )
  {
    QLineF line = ( *guideIt )->line();
    if ( line.dx() == 0 )
    {
      //vertical line, change height of line
      ( *guideIt )->setLine( line.x1(), 0, line.x1(), totalHeight );
    }
    else
    {
      //horizontal line
      //move to new vertical position and change width of line
      QPointF curPagePos = positionOnPage( line.p1() );
      int curPage = pageNumberForPoint( line.p1() ) - 1;
      double newY = curPage * ( height + spaceBetweenPages() ) + curPagePos.y();
      ( *guideIt )->setLine( 0, newY, width, newY );
    }
  }

  mPageWidth = width;
  mPageHeight = height;
  double currentY = 0;
  for ( int i = 0; i < mPages.size(); ++i )
  {
    mPages.at( i )->setSceneRect( QRectF( 0, currentY, width, height ) );
    currentY += ( height + mSpaceBetweenPages );
  }
  QgsProject::instance()->dirty( true );
  updateBounds();
  emit paperSizeChanged();
}

double QgsComposition::paperHeight() const
{
  return mPageHeight;
}

double QgsComposition::paperWidth() const
{
  return mPageWidth;
}

void QgsComposition::setNumPages( const int pages )
{
  int currentPages = numPages();
  int desiredPages = pages;

  //data defined num pages set?
  QVariant exprVal;
  if ( dataDefinedEvaluate( QgsComposerObject::NumPages, exprVal, &mDataDefinedProperties ) )
  {
    bool ok = false;
    int pagesD = exprVal.toInt( &ok );
    QgsDebugMsg( QString( "exprVal NumPages:%1" ).arg( pagesD ) );
    if ( ok )
    {
      desiredPages = pagesD;
    }
  }

  int diff = desiredPages - currentPages;
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

  //update vertical guide height
  QList< QGraphicsLineItem* >* guides = snapLines();
  QList< QGraphicsLineItem* >::iterator guideIt = guides->begin();
  double totalHeight = ( mPageHeight + spaceBetweenPages() ) * ( pages - 1 ) + mPageHeight;
  for ( ; guideIt != guides->end(); ++guideIt )
  {
    QLineF line = ( *guideIt )->line();
    if ( line.dx() == 0 )
    {
      //vertical line, change height of line
      ( *guideIt )->setLine( line.x1(), 0, line.x1(), totalHeight );
    }
  }

  //update the corresponding variable
  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )numPages() ) );

  QgsProject::instance()->dirty( true );
  updateBounds();

  emit nPagesChanged();
}

int QgsComposition::numPages() const
{
  return mPages.size();
}

bool QgsComposition::pageIsEmpty( const int page ) const
{
  //get all items on page
  QList<QgsComposerItem*> items;
  //composerItemsOnPage uses 0-based page numbering
  composerItemsOnPage( items, page - 1 );

  //loop through and check for non-paper items
  QList<QgsComposerItem*>::const_iterator itemIt = items.constBegin();
  for ( ; itemIt != items.constEnd(); ++itemIt )
  {
    //is item a paper item?
    QgsPaperItem* paper = dynamic_cast<QgsPaperItem*>( *itemIt );
    if ( !paper )
    {
      //item is not a paper item, so we have other items on the page
      return false;
    }
  }
  //no non-paper items
  return true;
}

bool QgsComposition::shouldExportPage( const int page ) const
{
  if ( page > numPages() || page < 1 )
  {
    //page number out of range
    return false;
  }

  //check all frame items on page
  QList<QgsComposerFrame*> frames;
  //composerItemsOnPage uses 0 based page numbering
  composerItemsOnPage( frames, page - 1 );
  QList<QgsComposerFrame*>::const_iterator frameIt = frames.constBegin();
  for ( ; frameIt != frames.constEnd(); ++frameIt )
  {
    if (( *frameIt )->hidePageIfEmpty() && ( *frameIt )->isEmpty() )
    {
      //frame is set to hide page if empty, and frame is empty, so we don't want to export this page
      return false;
    }
  }
  return true;
}

void QgsComposition::setPageStyleSymbol( QgsFillSymbolV2* symbol )
{
  delete mPageStyleSymbol;
  mPageStyleSymbol = symbol;
  QgsProject::instance()->dirty( true );
}

void QgsComposition::createDefaultPageStyleSymbol()
{
  delete mPageStyleSymbol;
  QgsStringMap properties;
  properties.insert( "color", "white" );
  properties.insert( "style", "solid" );
  properties.insert( "style_border", "no" );
  properties.insert( "joinstyle", "miter" );
  mPageStyleSymbol = QgsFillSymbolV2::createSimple( properties );
}

QPointF QgsComposition::positionOnPage( const QPointF & position ) const
{
  double y;
  if ( position.y() > ( mPages.size() - 1 ) * ( paperHeight() + spaceBetweenPages() ) )
  {
    //y coordinate is greater then the end of the last page, so return distance between
    //top of last page and y coordinate
    y = position.y() - ( mPages.size() - 1 ) * ( paperHeight() + spaceBetweenPages() );
  }
  else
  {
    //y coordinate is less then the end of the last page
    y = fmod( position.y(), ( paperHeight() + spaceBetweenPages() ) );
  }
  return QPointF( position.x(), y );
}

int QgsComposition::pageNumberForPoint( const QPointF & position ) const
{
  int pageNumber = qFloor( position.y() / ( paperHeight() + spaceBetweenPages() ) ) + 1;
  pageNumber = pageNumber < 1 ? 1 : pageNumber;
  pageNumber = pageNumber > mPages.size() ? mPages.size() : pageNumber;
  return pageNumber;
}

void QgsComposition::setStatusMessage( const QString & message )
{
  emit statusMsgChanged( message );
}

QgsComposerItem* QgsComposition::composerItemAt( const QPointF & position, const bool ignoreLocked ) const
{
  return composerItemAt( position, 0, ignoreLocked );
}

QgsComposerItem* QgsComposition::composerItemAt( const QPointF & position, const QgsComposerItem* belowItem, const bool ignoreLocked ) const
{
  //get a list of items which intersect the specified position, in descending z order
  QList<QGraphicsItem*> itemList;
  itemList = items( position, Qt::IntersectsItemShape, Qt::DescendingOrder );
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();

  bool foundBelowItem = false;
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    QgsPaperItem* paperItem = dynamic_cast<QgsPaperItem*>( *itemIt );
    if ( composerItem && !paperItem )
    {
      // If we are not checking for a an item below a specified item, or if we've
      // already found that item, then we've found our target
      if (( ! belowItem || foundBelowItem ) && ( !ignoreLocked || !composerItem->positionLock() ) )
      {
        return composerItem;
      }
      else
      {
        if ( composerItem == belowItem )
        {
          //Target item is next in list
          foundBelowItem = true;
        }
      }
    }
  }
  return 0;
}

int QgsComposition::pageNumberAt( const QPointF& position ) const
{
  return position.y() / ( paperHeight() + spaceBetweenPages() );
}

int QgsComposition::itemPageNumber( const QgsComposerItem* item ) const
{
  return pageNumberAt( QPointF( item->pos().x(), item->pos().y() ) );
}

QList<QgsComposerItem*> QgsComposition::selectedComposerItems( const bool includeLockedItems )
{
  QList<QgsComposerItem*> composerItemList;

  QList<QGraphicsItem *> graphicsItemList = selectedItems();
  QList<QGraphicsItem *>::iterator itemIter = graphicsItemList.begin();

  for ( ; itemIter != graphicsItemList.end(); ++itemIter )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem *>( *itemIter );
    if ( composerItem && ( includeLockedItems || !composerItem->positionLock() ) )
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

const QgsComposerMap* QgsComposition::getComposerMapById( const int id ) const
{
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

const QgsComposerHtml* QgsComposition::getComposerHtmlByItem( QgsComposerItem *item ) const
{
  // an html item will be a composer frame and if it is we can try to get
  // its multiframe parent and then try to cast that to a composer html
  const QgsComposerFrame* composerFrame =
    dynamic_cast<const QgsComposerFrame *>( item );
  if ( composerFrame )
  {
    const QgsComposerMultiFrame * mypMultiFrame = composerFrame->multiFrame();
    const QgsComposerHtml* composerHtml =
      dynamic_cast<const QgsComposerHtml *>( mypMultiFrame );
    if ( composerHtml )
    {
      return composerHtml;
    }
  }
  return 0;
}

const QgsComposerItem* QgsComposition::getComposerItemById( const QString theId ) const
{
  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    const QgsComposerItem* mypItem = dynamic_cast<const QgsComposerItem *>( *itemIt );
    if ( mypItem )
    {
      if ( mypItem->id() == theId )
      {
        return mypItem;
      }
    }
  }
  return 0;
}

#if 0
const QgsComposerItem* QgsComposition::getComposerItemByUuid( QString theUuid, bool inAllComposers ) const
{
  //This does not work since it seems impossible to get the QgisApp::instance() from here... Is there a workaround ?
  QSet<QgsComposer*> composers = QSet<QgsComposer*>();

  if ( inAllComposers )
  {
    composers = QgisApp::instance()->printComposers();
  }
  else
  {
    composers.insert( this )
  }

  QSet<QgsComposer*>::const_iterator it = composers.constBegin();
  for ( ; it != composers.constEnd(); ++it )
  {
    QList<QGraphicsItem *> itemList = ( *it )->items();
    QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
    for ( ; itemIt != itemList.end(); ++itemIt )
    {
      const QgsComposerItem* mypItem = dynamic_cast<const QgsComposerItem *>( *itemIt );
      if ( mypItem )
      {
        if ( mypItem->uuid() == theUuid )
        {
          return mypItem;
        }
      }
    }
  }

  return 0;
}
#endif

const QgsComposerItem* QgsComposition::getComposerItemByUuid( const QString theUuid ) const
{
  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    const QgsComposerItem* mypItem = dynamic_cast<const QgsComposerItem *>( *itemIt );
    if ( mypItem )
    {
      if ( mypItem->uuid() == theUuid )
      {
        return mypItem;
      }
    }
  }

  return 0;
}

void QgsComposition::setPrintResolution( const int dpi )
{
  mPrintResolution = dpi;
  emit printResolutionChanged();
  QgsProject::instance()->dirty( true );
}

void QgsComposition::setUseAdvancedEffects( const bool effectsEnabled )
{
  mUseAdvancedEffects = effectsEnabled;

  //toggle effects for all composer items
  QList<QGraphicsItem*> itemList = items();
  QList<QGraphicsItem*>::const_iterator itemIt = itemList.constBegin();
  for ( ; itemIt != itemList.constEnd(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem*>( *itemIt );
    if ( composerItem )
    {
      composerItem->setEffectsEnabled( effectsEnabled );
    }
  }
}

int QgsComposition::pixelFontSize( double pointSize ) const
{
  return qRound( QgsComposerUtils::pointsToMM( pointSize ) ); //round to nearest mm
}

double QgsComposition::pointFontSize( int pixelSize ) const
{
  return QgsComposerUtils::mmToPoints( pixelSize );
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

  QDomElement pageStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mPageStyleSymbol, doc );
  compositionElem.appendChild( pageStyleElem );

  //snapping
  if ( mSnapToGrid )
  {
    compositionElem.setAttribute( "snapping", "1" );
  }
  else
  {
    compositionElem.setAttribute( "snapping", "0" );
  }
  if ( mGridVisible )
  {
    compositionElem.setAttribute( "gridVisible", "1" );
  }
  else
  {
    compositionElem.setAttribute( "gridVisible", "0" );
  }
  compositionElem.setAttribute( "snapGridResolution", QString::number( mSnapGridResolution ) );
  compositionElem.setAttribute( "snapGridOffsetX", QString::number( mSnapGridOffsetX ) );
  compositionElem.setAttribute( "snapGridOffsetY", QString::number( mSnapGridOffsetY ) );

  //custom snap lines
  QList< QGraphicsLineItem* >::const_iterator snapLineIt = mSnapLines.constBegin();
  for ( ; snapLineIt != mSnapLines.constEnd(); ++snapLineIt )
  {
    QDomElement snapLineElem = doc.createElement( "SnapLine" );
    QLineF line = ( *snapLineIt )->line();
    snapLineElem.setAttribute( "x1", QString::number( line.x1() ) );
    snapLineElem.setAttribute( "y1", QString::number( line.y1() ) );
    snapLineElem.setAttribute( "x2", QString::number( line.x2() ) );
    snapLineElem.setAttribute( "y2", QString::number( line.y2() ) );
    compositionElem.appendChild( snapLineElem );
  }

  compositionElem.setAttribute( "printResolution", mPrintResolution );
  compositionElem.setAttribute( "printAsRaster", mPrintAsRaster );

  compositionElem.setAttribute( "generateWorldFile", mGenerateWorldFile ? 1 : 0 );
  if ( mGenerateWorldFile && mWorldFileMap )
  {
    compositionElem.setAttribute( "worldFileMap", mWorldFileMap->id() );
  }

  compositionElem.setAttribute( "alignmentSnap", mAlignmentSnap ? 1 : 0 );
  compositionElem.setAttribute( "guidesVisible", mGuidesVisible ? 1 : 0 );
  compositionElem.setAttribute( "smartGuides", mSmartGuides ? 1 : 0 );
  compositionElem.setAttribute( "snapTolerancePixels", mSnapTolerance );

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

  //data defined properties
  QgsComposerUtils::writeDataDefinedPropertyMap( compositionElem, doc, &mDataDefinedNames, &mDataDefinedProperties );

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
  emit paperSizeChanged();
  int numPages = compositionElem.attribute( "numPages", "1" ).toInt();

  QDomElement pageStyleSymbolElem = compositionElem.firstChildElement( "symbol" );
  if ( !pageStyleSymbolElem.isNull() )
  {
    delete mPageStyleSymbol;
    mPageStyleSymbol = QgsSymbolLayerV2Utils::loadSymbol<QgsFillSymbolV2>( pageStyleSymbolElem );
  }

  if ( widthConversionOk && heightConversionOk )
  {
    removePaperItems();
    for ( int i = 0; i < numPages; ++i )
    {
      addPaperItem();
    }
  }

  //snapping
  mSnapToGrid = compositionElem.attribute( "snapping", "0" ).toInt() == 0 ? false : true;
  mGridVisible = compositionElem.attribute( "gridVisible", "0" ).toInt() == 0 ? false : true;

  mSnapGridResolution = compositionElem.attribute( "snapGridResolution" ).toDouble();
  mSnapGridOffsetX = compositionElem.attribute( "snapGridOffsetX" ).toDouble();
  mSnapGridOffsetY = compositionElem.attribute( "snapGridOffsetY" ).toDouble();

  mAlignmentSnap = compositionElem.attribute( "alignmentSnap", "1" ).toInt() == 0 ? false : true;
  mGuidesVisible = compositionElem.attribute( "guidesVisible", "1" ).toInt() == 0 ? false : true;
  mSmartGuides = compositionElem.attribute( "smartGuides", "1" ).toInt() == 0 ? false : true;
  mSnapTolerance = compositionElem.attribute( "snapTolerancePixels", "10" ).toInt();

  //custom snap lines
  QDomNodeList snapLineNodes = compositionElem.elementsByTagName( "SnapLine" );
  for ( int i = 0; i < snapLineNodes.size(); ++i )
  {
    QDomElement snapLineElem = snapLineNodes.at( i ).toElement();
    QGraphicsLineItem* snapItem = addSnapLine();
    double x1 = snapLineElem.attribute( "x1" ).toDouble();
    double y1 = snapLineElem.attribute( "y1" ).toDouble();
    double x2 = snapLineElem.attribute( "x2" ).toDouble();
    double y2 = snapLineElem.attribute( "y2" ).toDouble();
    snapItem->setLine( x1, y1, x2, y2 );
  }

  mPrintAsRaster = compositionElem.attribute( "printAsRaster" ).toInt();
  mPrintResolution = compositionElem.attribute( "printResolution", "300" ).toInt();

  mGenerateWorldFile = compositionElem.attribute( "generateWorldFile", "0" ).toInt() == 1 ? true : false;

  //data defined properties
  QgsComposerUtils::readDataDefinedPropertyMap( compositionElem, &mDataDefinedNames, &mDataDefinedProperties );

  updatePaperItems();

  updateBounds();

  return true;
}

bool QgsComposition::loadFromTemplate( const QDomDocument& doc, QMap<QString, QString>* substitutionMap, bool addUndoCommands, const bool clearComposition )
{
  if ( clearComposition )
  {
    deleteAndRemoveMultiFrames();

    //delete all non paper items and emit itemRemoved signal
    QList<QGraphicsItem *> itemList = items();
    QList<QGraphicsItem *>::iterator itemIter = itemList.begin();
    for ( ; itemIter != itemList.end(); ++itemIter )
    {
      QgsComposerItem* cItem = dynamic_cast<QgsComposerItem*>( *itemIter );
      QgsPaperItem* pItem = dynamic_cast<QgsPaperItem*>( *itemIter );
      if ( cItem && !pItem )
      {
        removeItem( cItem );
        emit itemRemoved( cItem );
        delete cItem;
      }
    }
    mItemsModel->clear();

    removePaperItems();
    mUndoStack->clear();
  }

  QDomDocument importDoc;
  if ( substitutionMap )
  {
    QString xmlString = doc.toString();
    QMap<QString, QString>::const_iterator sIt = substitutionMap->constBegin();
    for ( ; sIt != substitutionMap->constEnd(); ++sIt )
    {
      xmlString = xmlString.replace( "[" + sIt.key() + "]", encodeStringForXML( sIt.value() ) );
    }

    QString errorMsg;
    int errorLine, errorColumn;
    if ( !importDoc.setContent( xmlString, &errorMsg, &errorLine, &errorColumn ) )
    {
      return false;
    }
  }
  else
  {
    importDoc = doc;
  }

  //read general settings
  QDomElement atlasElem;
  if ( clearComposition )
  {
    QDomElement compositionElem = importDoc.documentElement().firstChildElement( "Composition" );
    if ( compositionElem.isNull() )
    {
      return false;
    }

    bool ok = readXML( compositionElem, importDoc );
    if ( !ok )
    {
      return false;
    }

    // read atlas parameters - must be done before adding items
    atlasElem = importDoc.documentElement().firstChildElement( "Atlas" );
    atlasComposition().readXML( atlasElem, importDoc );
  }

  // remove all uuid attributes since we don't want duplicates UUIDS
  QDomNodeList composerItemsNodes = importDoc.elementsByTagName( "ComposerItem" );
  for ( int i = 0; i < composerItemsNodes.count(); ++i )
  {
    QDomNode composerItemNode = composerItemsNodes.at( i );
    if ( composerItemNode.isElement() )
    {
      composerItemNode.toElement().setAttribute( "templateUuid", composerItemNode.toElement().attribute( "uuid" ) );
      composerItemNode.toElement().removeAttribute( "uuid" );
    }
  }

  //addItemsFromXML
  addItemsFromXML( importDoc.documentElement(), importDoc, 0, addUndoCommands, 0 );

  //read atlas map parameters (for pre 2.2 templates)
  //this can only be done after items have been added
  if ( clearComposition )
  {
    atlasComposition().readXMLMapSettings( atlasElem, importDoc );
  }
  return true;
}

QPointF QgsComposition::minPointFromXml( const QDomElement& elem ) const
{
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  QDomNodeList composerItemList = elem.elementsByTagName( "ComposerItem" );
  for ( int i = 0; i < composerItemList.size(); ++i )
  {
    QDomElement currentComposerItemElem = composerItemList.at( i ).toElement();
    double x, y;
    bool xOk, yOk;
    x = currentComposerItemElem.attribute( "x" ).toDouble( &xOk );
    y = currentComposerItemElem.attribute( "y" ).toDouble( &yOk );
    if ( !xOk || !yOk )
    {
      continue;
    }
    minX = qMin( minX, x );
    minY = qMin( minY, y );
  }
  if ( minX < std::numeric_limits<double>::max() )
  {
    return QPointF( minX, minY );
  }
  else
  {
    return QPointF( 0, 0 );
  }
}

void QgsComposition::addItemsFromXML( const QDomElement& elem, const QDomDocument& doc, QMap< QgsComposerMap*, int >* mapsToRestore,
                                      bool addUndoCommands, QPointF* pos, bool pasteInPlace )
{
  QPointF* pasteInPlacePt = 0;

  //if we are adding items to a composition which already contains items, we need to make sure
  //these items are placed at the top of the composition and that zValues are not duplicated
  //so, calculate an offset which needs to be added to the zValue of created items
  int zOrderOffset = mItemsModel->zOrderListSize();

  QPointF pasteShiftPos;
  QgsComposerItem* lastPastedItem = 0;
  if ( pos )
  {
    //If we are placing items relative to a certain point, then calculate how much we need
    //to shift the items by so that they are placed at this point
    //First, calculate the minimum position from the xml
    QPointF minItemPos = minPointFromXml( elem );
    //next, calculate how much each item needs to be shifted from its original position
    //so that it's placed at the correct relative position
    pasteShiftPos = *pos - minItemPos;

    //since we are pasting items, clear the existing selection
    setAllUnselected();

    if ( pasteInPlace )
    {
      pasteInPlacePt = new QPointF( 0, pageNumberAt( *pos ) * ( mPageHeight + mSpaceBetweenPages ) );
    }
  }
  QDomNodeList composerLabelList = elem.elementsByTagName( "ComposerLabel" );
  for ( int i = 0; i < composerLabelList.size(); ++i )
  {
    QDomElement currentComposerLabelElem = composerLabelList.at( i ).toElement();
    QgsComposerLabel* newLabel = new QgsComposerLabel( this );
    newLabel->readXML( currentComposerLabelElem, doc );
    if ( pos )
    {
      if ( pasteInPlacePt )
      {
        newLabel->setItemPosition( newLabel->pos().x(), fmod( newLabel->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newLabel->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newLabel->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newLabel->setSelected( true );
      lastPastedItem = newLabel;
    }
    addComposerLabel( newLabel );
    newLabel->setZValue( newLabel->zValue() + zOrderOffset );
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

    if ( mapsToRestore )
    {
      newMap->setUpdatesEnabled( false );
    }

    newMap->readXML( currentComposerMapElem, doc );
    newMap->assignFreeId();

    if ( mapsToRestore )
    {
      mapsToRestore->insert( newMap, ( int )( newMap->previewMode() ) );
      newMap->setPreviewMode( QgsComposerMap::Rectangle );
      newMap->setUpdatesEnabled( true );
    }
    addComposerMap( newMap, false );
    newMap->setZValue( newMap->zValue() + zOrderOffset );
    if ( pos )
    {
      if ( pasteInPlace )
      {
        newMap->setItemPosition( newMap->pos().x(), fmod( newMap->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newMap->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newMap->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newMap->setSelected( true );
      lastPastedItem = newMap;
    }

    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newMap, tr( "Map added" ) );
    }
  }
  //now that all map items have been created, re-connect overview map signals
  QList<QgsComposerMap*> maps;
  composerItems( maps );
  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    QgsComposerMap* map = ( *mit );
    if ( map )
    {
      QList<QgsComposerMapOverview* > overviews = map->overviews()->asList();
      QList<QgsComposerMapOverview* >::iterator overviewIt = overviews.begin();
      for ( ; overviewIt != overviews.end(); ++overviewIt )
      {
        ( *overviewIt )->connectSignals();
      }
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
      if ( pasteInPlace )
      {
        newArrow->setItemPosition( newArrow->pos().x(), fmod( newArrow->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newArrow->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newArrow->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newArrow->setSelected( true );
      lastPastedItem = newArrow;
    }
    addComposerArrow( newArrow );
    newArrow->setZValue( newArrow->zValue() + zOrderOffset );
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
      if ( pasteInPlace )
      {
        newScaleBar->setItemPosition( newScaleBar->pos().x(), fmod( newScaleBar->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newScaleBar->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newScaleBar->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newScaleBar->setSelected( true );
      lastPastedItem = newScaleBar;
    }
    addComposerScaleBar( newScaleBar );
    newScaleBar->setZValue( newScaleBar->zValue() + zOrderOffset );
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
    //new shapes should default to symbol v2
    newShape->setUseSymbolV2( true );
    if ( pos )
    {
      if ( pasteInPlace )
      {
        newShape->setItemPosition( newShape->pos().x(), fmod( newShape->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newShape->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newShape->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newShape->setSelected( true );
      lastPastedItem = newShape;
    }
    addComposerShape( newShape );
    newShape->setZValue( newShape->zValue() + zOrderOffset );
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
      if ( pasteInPlace )
      {
        newPicture->setItemPosition( newPicture->pos().x(), fmod( newPicture->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newPicture->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newPicture->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newPicture->setSelected( true );
      lastPastedItem = newPicture;
    }
    addComposerPicture( newPicture );
    newPicture->setZValue( newPicture->zValue() + zOrderOffset );
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
      if ( pasteInPlace )
      {
        newLegend->setItemPosition( newLegend->pos().x(), fmod( newLegend->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newLegend->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newLegend->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newLegend->setSelected( true );
      lastPastedItem = newLegend;
    }
    addComposerLegend( newLegend );
    newLegend->setZValue( newLegend->zValue() + zOrderOffset );
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
      if ( pasteInPlace )
      {
        newTable->setItemPosition( newTable->pos().x(), fmod( newTable->pos().y(), ( paperHeight() + spaceBetweenPages() ) ) );
        newTable->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newTable->move( pasteShiftPos.x(), pasteShiftPos.y() );
      }
      newTable->setSelected( true );
      lastPastedItem = newTable;
    }
    addComposerTable( newTable );
    newTable->setZValue( newTable->zValue() + zOrderOffset );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newTable, tr( "Table added" ) );
    }
  }
  // html
  //TODO - fix this. pasting multiframe frame items has no effect
  QDomNodeList composerHtmlList = elem.elementsByTagName( "ComposerHtml" );
  for ( int i = 0; i < composerHtmlList.size(); ++i )
  {
    QDomElement currentHtmlElem = composerHtmlList.at( i ).toElement();
    QgsComposerHtml* newHtml = new QgsComposerHtml( this, false );
    newHtml->readXML( currentHtmlElem, doc );
    newHtml->setCreateUndoCommands( true );
    this->addMultiFrame( newHtml );

    //offset z values for frames
    //TODO - fix this after fixing html item paste
    /*for ( int frameIdx = 0; frameIdx < newHtml->frameCount(); ++frameIdx )
    {
      QgsComposerFrame * frame = newHtml->frame( frameIdx );
      frame->setZValue( frame->zValue() + zOrderOffset );
    }*/
  }
  QDomNodeList composerAttributeTableV2List = elem.elementsByTagName( "ComposerAttributeTableV2" );
  for ( int i = 0; i < composerAttributeTableV2List.size(); ++i )
  {
    QDomElement currentTableElem = composerAttributeTableV2List.at( i ).toElement();
    QgsComposerAttributeTableV2* newTable = new QgsComposerAttributeTableV2( this, false );
    newTable->readXML( currentTableElem, doc );
    newTable->setCreateUndoCommands( true );
    this->addMultiFrame( newTable );

    //offset z values for frames
    //TODO - fix this after fixing html item paste
    /*for ( int frameIdx = 0; frameIdx < newHtml->frameCount(); ++frameIdx )
    {
      QgsComposerFrame * frame = newHtml->frame( frameIdx );
      frame->setZValue( frame->zValue() + zOrderOffset );
    }*/
  }

  // groups (must be last as it references uuids of above items)
  //TODO - pasted groups lose group properties, since the uuids of group items
  //changes
  QDomNodeList groupList = elem.elementsByTagName( "ComposerItemGroup" );
  for ( int i = 0; i < groupList.size(); ++i )
  {
    QDomElement groupElem = groupList.at( i ).toElement();
    QgsComposerItemGroup *newGroup = new QgsComposerItemGroup( this );
    newGroup->readXML( groupElem, doc );
    addItem( newGroup );
  }

  //Since this function adds items grouped by type, and each item is added to end of
  //z order list in turn, it will now be inconsistent with the actual order of items in the scene.
  //Make sure z order list matches the actual order of items in the scene.
  mItemsModel->rebuildZList();

  if ( lastPastedItem )
  {
    emit selectedItemChanged( lastPastedItem );
  }

  delete pasteInPlacePt;
  pasteInPlacePt = 0;

}

void QgsComposition::addItemToZList( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }

  //model handles changes to z list
  mItemsModel->addItemAtTop( item );
}

void QgsComposition::removeItemFromZList( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }

  //model handles changes to z list
  mItemsModel->removeItem( item );
}

void QgsComposition::raiseSelectedItems()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  bool itemsRaised = false;
  for ( ; it != selectedItems.end(); ++it )
  {
    itemsRaised = itemsRaised | raiseItem( *it );
  }

  if ( !itemsRaised )
  {
    //no change
    return;
  }

  //update all positions
  updateZValues();
  update();
}

bool QgsComposition::raiseItem( QgsComposerItem* item )
{
  //model handles reordering items
  return mItemsModel->reorderItemUp( item );
}

QgsComposerItem* QgsComposition::getComposerItemAbove( QgsComposerItem* item ) const
{
  return mItemsModel->getComposerItemAbove( item );
}

QgsComposerItem* QgsComposition::getComposerItemBelow( QgsComposerItem* item ) const
{
  return mItemsModel->getComposerItemBelow( item );
}

void QgsComposition::selectNextByZOrder( ZValueDirection direction )
{
  QgsComposerItem* previousSelectedItem = 0;
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  if ( selectedItems.size() > 0 )
  {
    previousSelectedItem = selectedItems.at( 0 );
  }

  if ( !previousSelectedItem )
  {
    return;
  }

  //select item with target z value
  QgsComposerItem* selectedItem = 0;
  switch ( direction )
  {
    case QgsComposition::ZValueBelow:
      selectedItem = getComposerItemBelow( previousSelectedItem );
      break;
    case QgsComposition::ZValueAbove:
      selectedItem = getComposerItemAbove( previousSelectedItem );
      break;
  }

  if ( !selectedItem )
  {
    return;
  }

  //ok, found a good target item
  setAllUnselected();
  selectedItem->setSelected( true );
  emit selectedItemChanged( selectedItem );
}

void QgsComposition::lowerSelectedItems()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  bool itemsLowered = false;
  for ( ; it != selectedItems.end(); ++it )
  {
    itemsLowered = itemsLowered | lowerItem( *it );
  }

  if ( !itemsLowered )
  {
    //no change
    return;
  }

  //update all positions
  updateZValues();
  update();
}

bool QgsComposition::lowerItem( QgsComposerItem* item )
{
  //model handles reordering items
  return mItemsModel->reorderItemDown( item );
}

void QgsComposition::moveSelectedItemsToTop()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  bool itemsRaised = false;
  for ( ; it != selectedItems.end(); ++it )
  {
    itemsRaised = itemsRaised | moveItemToTop( *it );
  }

  if ( !itemsRaised )
  {
    //no change
    return;
  }

  //update all positions
  updateZValues();
  update();
}

bool QgsComposition::moveItemToTop( QgsComposerItem* item )
{
  //model handles reordering items
  return mItemsModel->reorderItemToTop( item );
}

void QgsComposition::moveSelectedItemsToBottom()
{
  QList<QgsComposerItem*> selectedItems = selectedComposerItems();
  QList<QgsComposerItem*>::iterator it = selectedItems.begin();
  bool itemsLowered = false;
  for ( ; it != selectedItems.end(); ++it )
  {
    itemsLowered = itemsLowered | moveItemToBottom( *it );
  }

  if ( !itemsLowered )
  {
    //no change
    return;
  }

  //update all positions
  updateZValues();
  update();
}

bool QgsComposition::moveItemToBottom( QgsComposerItem* item )
{
  //model handles reordering items
  return mItemsModel->reorderItemToBottom( item );
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
    ( *align_it )->setPos( minXCoordinate, ( *align_it )->pos().y() );
    subcommand->saveAfterState();
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
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
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items horizontal center" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    ( *align_it )->setPos( averageXCoord - ( *align_it )->rect().width() / 2.0, ( *align_it )->pos().y() );
    subcommand->saveAfterState();
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
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
    ( *align_it )->setPos( maxXCoordinate - ( *align_it )->rect().width(), ( *align_it )->pos().y() );
    subcommand->saveAfterState();
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
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
    ( *align_it )->setPos(( *align_it )->pos().x(), minYCoordinate );
    subcommand->saveAfterState();
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
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
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items vertical center" ) );
  QList<QgsComposerItem*>::iterator align_it = selectedItems.begin();
  for ( ; align_it != selectedItems.end(); ++align_it )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *align_it, "", parentCommand );
    subcommand->savePreviousState();
    ( *align_it )->setPos(( *align_it )->pos().x(), averageYCoord - ( *align_it )->rect().height() / 2 );
    subcommand->saveAfterState();
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
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
    ( *align_it )->setPos(( *align_it )->pos().x(), maxYCoord - ( *align_it )->rect().height() );
    subcommand->saveAfterState();
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
}

void QgsComposition::lockSelectedItems()
{
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Items locked" ) );
  QList<QgsComposerItem*> selectionList = selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIter = selectionList.begin();
  for ( ; itemIter != selectionList.end(); ++itemIter )
  {
    QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( *itemIter, "", parentCommand );
    subcommand->savePreviousState();
    ( *itemIter )->setPositionLock( true );
    subcommand->saveAfterState();
  }

  setAllUnselected();
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
}

void QgsComposition::unlockAllItems()
{
  //unlock all items in composer

  QUndoCommand* parentCommand = new QUndoCommand( tr( "Items unlocked" ) );

  //first, clear the selection
  setAllUnselected();

  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* mypItem = dynamic_cast<QgsComposerItem *>( *itemIt );
    if ( mypItem && mypItem->positionLock() )
    {
      QgsComposerItemCommand* subcommand = new QgsComposerItemCommand( mypItem, "", parentCommand );
      subcommand->savePreviousState();
      mypItem->setPositionLock( false );
      //select unlocked items, same behaviour as illustrator
      mypItem->setSelected( true );
      emit selectedItemChanged( mypItem );
      subcommand->saveAfterState();
    }
  }
  mUndoStack->push( parentCommand );
  QgsProject::instance()->dirty( true );
}

QgsComposerItemGroup *QgsComposition::groupItems( QList<QgsComposerItem *> items )
{
  if ( items.size() < 2 )
  {
    //not enough items for a group
    return 0;
  }

  QgsComposerItemGroup* itemGroup = new QgsComposerItemGroup( this );

  QList<QgsComposerItem*>::iterator itemIter = items.begin();
  for ( ; itemIter != items.end(); ++itemIter )
  {
    itemGroup->addItem( *itemIter );
  }

  addItem( itemGroup );
  return itemGroup;
}

QList<QgsComposerItem *> QgsComposition::ungroupItems( QgsComposerItemGroup* group )
{
  QList<QgsComposerItem *> ungroupedItems;
  if ( !group )
  {
    return ungroupedItems;
  }

  QSet<QgsComposerItem*> groupedItems = group->items();
  QSet<QgsComposerItem*>::iterator itemIt = groupedItems.begin();
  for ( ; itemIt != groupedItems.end(); ++itemIt )
  {
    ungroupedItems << ( *itemIt );
  }

  group->removeItems();
  removeComposerItem( group, false, false );

  emit itemRemoved( group );
  delete( group );

  return ungroupedItems;
}

void QgsComposition::updateZValues( const bool addUndoCommands )
{
  int counter = mItemsModel->zOrderListSize();
  QList<QgsComposerItem*>::const_iterator it = mItemsModel->zOrderList()->constBegin();
  QgsComposerItem* currentItem = 0;

  QUndoCommand* parentCommand = 0;
  if ( addUndoCommands )
  {
    parentCommand = new QUndoCommand( tr( "Item z-order changed" ) );
  }
  for ( ; it != mItemsModel->zOrderList()->constEnd(); ++it )
  {
    currentItem = *it;
    if ( currentItem )
    {
      QgsComposerItemCommand* subcommand = 0;
      if ( addUndoCommands )
      {
        subcommand = new QgsComposerItemCommand( *it, "", parentCommand );
        subcommand->savePreviousState();
      }
      currentItem->setZValue( counter );
      if ( addUndoCommands )
      {
        subcommand->saveAfterState();
      }
    }
    --counter;
  }
  if ( addUndoCommands )
  {
    mUndoStack->push( parentCommand );
    QgsProject::instance()->dirty( true );
  }
}

void QgsComposition::refreshZList()
{
  //model handles changes to item z order list
  mItemsModel->rebuildZList();

  //Finally, rebuild the zValue of all items to remove any duplicate zValues and make sure there's
  //no missing zValues.
  updateZValues( false );
}

QPointF QgsComposition::snapPointToGrid( const QPointF& scenePoint ) const
{
  if ( !mSnapToGrid || mSnapGridResolution <= 0 || !graphicsView() )
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

  double xSnapped = xRatio * mSnapGridResolution + mSnapGridOffsetX;
  double ySnapped = yRatio * mSnapGridResolution + mSnapGridOffsetY + yOffset;

  //convert snap tolerance from pixels to mm
  double viewScaleFactor = graphicsView()->transform().m11();
  double alignThreshold = mSnapTolerance / viewScaleFactor;

  if ( fabs( xSnapped - scenePoint.x() ) > alignThreshold )
  {
    //snap distance is outside of tolerance
    xSnapped = scenePoint.x();
  }
  if ( fabs( ySnapped - scenePoint.y() ) > alignThreshold )
  {
    //snap distance is outside of tolerance
    ySnapped = scenePoint.y();
  }

  return QPointF( xSnapped, ySnapped );
}

QGraphicsLineItem* QgsComposition::addSnapLine()
{
  QGraphicsLineItem* item = new QGraphicsLineItem();
  QPen linePen( Qt::SolidLine );
  linePen.setColor( Qt::red );
  // use a pen width of 0, since this activates a cosmetic pen
  // which doesn't scale with the composer and keeps a constant size
  linePen.setWidthF( 0 );
  item->setPen( linePen );
  item->setZValue( 100 );
  item->setVisible( mGuidesVisible );
  addItem( item );
  mSnapLines.push_back( item );
  return item;
}

void QgsComposition::removeSnapLine( QGraphicsLineItem* line )
{
  removeItem( line );
  mSnapLines.removeAll( line );
  delete line;
}

void QgsComposition::clearSnapLines()
{
  QList< QGraphicsLineItem* >::iterator it = mSnapLines.begin();
  for ( ; it != mSnapLines.end(); ++it )
  {
    removeItem(( *it ) );
    delete( *it );
  }
  mSnapLines.clear();
}

void QgsComposition::setSnapLinesVisible( const bool visible )
{
  mGuidesVisible = visible;
  QList< QGraphicsLineItem* >::iterator it = mSnapLines.begin();
  for ( ; it != mSnapLines.end(); ++it )
  {
    if ( visible )
    {
      ( *it )->show();
    }
    else
    {
      ( *it )->hide();
    }
  }
}

QGraphicsLineItem* QgsComposition::nearestSnapLine( const bool horizontal, const double x, const double y, const double tolerance,
    QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode> >& snappedItems ) const
{
  double minSqrDist = DBL_MAX;
  QGraphicsLineItem* item = 0;
  double currentXCoord = 0;
  double currentYCoord = 0;
  double currentSqrDist = 0;
  double sqrTolerance = tolerance * tolerance;

  snappedItems.clear();

  QList< QGraphicsLineItem* >::const_iterator it = mSnapLines.constBegin();
  for ( ; it != mSnapLines.constEnd(); ++it )
  {
    bool itemHorizontal = qgsDoubleNear(( *it )->line().y2() - ( *it )->line().y1(), 0 );
    if ( horizontal && itemHorizontal )
    {
      currentYCoord = ( *it )->line().y1();
      currentSqrDist = ( y - currentYCoord ) * ( y - currentYCoord );
    }
    else if ( !horizontal && !itemHorizontal )
    {
      currentXCoord = ( *it )->line().x1();
      currentSqrDist = ( x - currentXCoord ) * ( x - currentXCoord );
    }
    else
    {
      continue;
    }

    if ( currentSqrDist < minSqrDist && currentSqrDist < sqrTolerance )
    {
      item = *it;
      minSqrDist = currentSqrDist;
    }
  }

  double itemTolerance = 0.0000001;
  if ( item )
  {
    //go through all the items to find items snapped to this snap line
    QList<QGraphicsItem *> itemList = items();
    QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
    for ( ; itemIt != itemList.end(); ++itemIt )
    {
      QgsComposerItem* currentItem = dynamic_cast<QgsComposerItem*>( *itemIt );
      if ( !currentItem || currentItem->type() == QgsComposerItem::ComposerPaper )
      {
        continue;
      }

      if ( horizontal )
      {
        if ( qgsDoubleNear( currentYCoord, currentItem->pos().y() + currentItem->rect().top(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::UpperMiddle ) );
        }
        else if ( qgsDoubleNear( currentYCoord, currentItem->pos().y() + currentItem->rect().center().y(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::Middle ) );
        }
        else if ( qgsDoubleNear( currentYCoord, currentItem->pos().y() + currentItem->rect().bottom(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::LowerMiddle ) );
        }
      }
      else
      {
        if ( qgsDoubleNear( currentXCoord, currentItem->pos().x(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::MiddleLeft ) );
        }
        else if ( qgsDoubleNear( currentXCoord, currentItem->pos().x() + currentItem->rect().center().x(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::Middle ) );
        }
        else if ( qgsDoubleNear( currentXCoord, currentItem->pos().x() + currentItem->rect().width(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::MiddleRight ) );
        }
      }
    }
  }

  return item;
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
  double minX = currentItem->pos().x();
  double minY = currentItem->pos().y();
  double maxX = minX + currentItem->rect().width();
  double maxY = minY + currentItem->rect().height();

  double currentMinX, currentMinY, currentMaxX, currentMaxY;

  for ( int i = 1; i < selectedItems.size(); ++i )
  {
    currentItem = selectedItems.at( i );
    currentMinX = currentItem->pos().x();
    currentMinY = currentItem->pos().y();
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

void QgsComposition::setSnapToGridEnabled( const bool b )
{
  mSnapToGrid = b;
  updatePaperItems();
}

void QgsComposition::setGridVisible( const bool b )
{
  mGridVisible = b;
  updatePaperItems();
}

void QgsComposition::setSnapGridResolution( const double r )
{
  mSnapGridResolution = r;
  updatePaperItems();
}

void QgsComposition::setSnapGridOffsetX( const double offset )
{
  mSnapGridOffsetX = offset;
  updatePaperItems();
}

void QgsComposition::setSnapGridOffsetY( const double offset )
{
  mSnapGridOffsetY = offset;
  updatePaperItems();
}

void QgsComposition::setGridPen( const QPen& p )
{
  mGridPen = p;
  //make sure grid is drawn using a zero-width cosmetic pen
  mGridPen.setWidthF( 0 );
  updatePaperItems();
}

void QgsComposition::setGridStyle( const GridStyle s )
{
  mGridStyle = s;
  updatePaperItems();
}

void QgsComposition::setBoundingBoxesVisible( const bool boundsVisible )
{
  mBoundingBoxesVisible = boundsVisible;

  if ( mSelectionHandles )
  {
    mSelectionHandles->update();
  }
}

void QgsComposition::updateSettings()
{
  //load new composer setting values
  loadSettings();
  //update any paper items to reflect new settings
  updatePaperItems();
}

void QgsComposition::loadSettings()
{
  //read grid style, grid color and pen width from settings
  QSettings s;

  QString gridStyleString;
  gridStyleString = s.value( "/Composer/gridStyle", "Dots" ).toString();

  int gridRed, gridGreen, gridBlue, gridAlpha;
  gridRed = s.value( "/Composer/gridRed", 190 ).toInt();
  gridGreen = s.value( "/Composer/gridGreen", 190 ).toInt();
  gridBlue = s.value( "/Composer/gridBlue", 190 ).toInt();
  gridAlpha = s.value( "/Composer/gridAlpha", 100 ).toInt();
  QColor gridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );

  mGridPen.setColor( gridColor );
  mGridPen.setWidthF( 0 );

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

void QgsComposition::beginCommand( QgsComposerItem* item, const QString& commandText, const QgsComposerMergeCommand::Context c )
{
  delete mActiveItemCommand;
  if ( !item )
  {
    mActiveItemCommand = 0;
    return;
  }

  if ( c == QgsComposerMergeCommand::Unknown )
  {
    mActiveItemCommand = new QgsComposerItemCommand( item, commandText );
  }
  else
  {
    mActiveItemCommand = new QgsComposerMergeCommand( c, item, commandText );
  }
  mActiveItemCommand->savePreviousState();
}

void QgsComposition::endCommand()
{
  if ( mActiveItemCommand )
  {
    mActiveItemCommand->saveAfterState();
    if ( mActiveItemCommand->containsChange() ) //protect against empty commands
    {
      mUndoStack->push( mActiveItemCommand );
      QgsProject::instance()->dirty( true );
    }
    else
    {
      delete mActiveItemCommand;
    }
    mActiveItemCommand = 0;
  }
}

void QgsComposition::cancelCommand()
{
  delete mActiveItemCommand;
  mActiveItemCommand = 0;
}

void QgsComposition::beginMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, const QgsComposerMultiFrameMergeCommand::Context c )
{
  delete mActiveMultiFrameCommand;

  if ( !multiFrame )
  {
    mActiveMultiFrameCommand = 0;
    return;
  }

  if ( c == QgsComposerMultiFrameMergeCommand::Unknown )
  {
    mActiveMultiFrameCommand = new QgsComposerMultiFrameCommand( multiFrame, text );
  }
  else
  {
    mActiveMultiFrameCommand = new QgsComposerMultiFrameMergeCommand( c, multiFrame, text );
  }
  mActiveMultiFrameCommand->savePreviousState();
}

void QgsComposition::endMultiFrameCommand()
{
  if ( mActiveMultiFrameCommand )
  {
    mActiveMultiFrameCommand->saveAfterState();
    if ( mActiveMultiFrameCommand->containsChange() )
    {
      mUndoStack->push( mActiveMultiFrameCommand );
      QgsProject::instance()->dirty( true );
    }
    else
    {
      delete mActiveMultiFrameCommand;
    }
    mActiveMultiFrameCommand = 0;
  }
}

void QgsComposition::cancelMultiFrameCommand()
{
  delete mActiveMultiFrameCommand;
  mActiveMultiFrameCommand = 0;
}

void QgsComposition::addMultiFrame( QgsComposerMultiFrame* multiFrame )
{
  mMultiFrames.insert( multiFrame );

  updateBounds();
}

void QgsComposition::removeMultiFrame( QgsComposerMultiFrame* multiFrame )
{
  mMultiFrames.remove( multiFrame );

  updateBounds();
}

void QgsComposition::addComposerArrow( QgsComposerArrow* arrow )
{
  addItem( arrow );

  updateBounds();
  connect( arrow, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerArrowAdded( arrow );
}

void QgsComposition::addComposerLabel( QgsComposerLabel* label )
{
  addItem( label );

  updateBounds();
  connect( label, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerLabelAdded( label );
}

void QgsComposition::addComposerMap( QgsComposerMap* map, const bool setDefaultPreviewStyle )
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

  updateBounds();
  connect( map, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerMapAdded( map );
}

void QgsComposition::addComposerScaleBar( QgsComposerScaleBar* scaleBar )
{
  addItem( scaleBar );

  updateBounds();
  connect( scaleBar, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerScaleBarAdded( scaleBar );
}

void QgsComposition::addComposerLegend( QgsComposerLegend* legend )
{
  addItem( legend );

  updateBounds();
  connect( legend, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerLegendAdded( legend );
}

void QgsComposition::addComposerPicture( QgsComposerPicture* picture )
{
  addItem( picture );

  updateBounds();
  connect( picture, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerPictureAdded( picture );
}

void QgsComposition::addComposerShape( QgsComposerShape* shape )
{
  addItem( shape );

  updateBounds();
  connect( shape, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerShapeAdded( shape );
}

void QgsComposition::addComposerTable( QgsComposerAttributeTable* table )
{
  addItem( table );

  updateBounds();
  connect( table, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerTableAdded( table );
}

void QgsComposition::addComposerHtmlFrame( QgsComposerHtml* html, QgsComposerFrame* frame )
{
  addItem( frame );

  updateBounds();
  connect( frame, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerHtmlFrameAdded( html, frame );
}

void QgsComposition::addComposerTableFrame( QgsComposerAttributeTableV2 *table, QgsComposerFrame *frame )
{
  addItem( frame );

  updateBounds();
  connect( frame, SIGNAL( sizeChanged() ), this, SLOT( updateBounds() ) );

  emit composerTableFrameAdded( table, frame );
}

void QgsComposition::removeComposerItem( QgsComposerItem* item, const bool createCommand, const bool removeGroupItems )
{
  QgsComposerMap* map = dynamic_cast<QgsComposerMap *>( item );

  if ( !map || !map->isDrawing() ) //don't delete a composer map while it draws
  {
    mItemsModel->setItemRemoved( item );
    removeItem( item );

    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup*>( item );
    if ( itemGroup && removeGroupItems )
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
      emit itemRemoved( itemGroup );
      delete itemGroup;
    }
    else
    {
      bool frameItem = ( item->type() == QgsComposerItem::ComposerFrame );
      QgsComposerMultiFrame* multiFrame = 0;
      if ( createCommand )
      {
        if ( frameItem ) //multiframe tracks item changes
        {
          multiFrame = static_cast<QgsComposerFrame*>( item )->multiFrame();
          item->beginItemCommand( tr( "Frame deleted" ) );
          emit itemRemoved( item );
          item->endItemCommand();
        }
        else
        {
          emit itemRemoved( item );
          pushAddRemoveCommand( item, tr( "Item deleted" ), QgsAddRemoveItemCommand::Removed );
        }
      }
      else
      {
        emit itemRemoved( item );
      }

      //check if there are frames left. If not, remove the multi frame
      if ( frameItem && multiFrame )
      {
        if ( multiFrame->frameCount() < 1 )
        {
          removeMultiFrame( multiFrame );
          if ( createCommand )
          {
            QgsAddRemoveMultiFrameCommand* command = new QgsAddRemoveMultiFrameCommand( QgsAddRemoveMultiFrameCommand::Removed,
                multiFrame, this, tr( "Multiframe removed" ) );
            undoStack()->push( command );
          }
          else
          {
            delete multiFrame;
          }
        }
      }
    }
  }

  updateBounds();
}

void QgsComposition::pushAddRemoveCommand( QgsComposerItem* item, const QString& text, const QgsAddRemoveItemCommand::State state )
{
  QgsAddRemoveItemCommand* c = new QgsAddRemoveItemCommand( state, item, this, text );
  connectAddRemoveCommandSignals( c );
  undoStack()->push( c );
  QgsProject::instance()->dirty( true );
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
  QgsComposerFrame* frame = dynamic_cast<QgsComposerFrame*>( item );
  if ( frame )
  {
    //emit composerFrameAdded( multiframe, frame, );
    QgsComposerMultiFrame* mf = frame->multiFrame();
    QgsComposerHtml* html = dynamic_cast<QgsComposerHtml*>( mf );
    if ( html )
    {
      emit composerHtmlFrameAdded( html, frame );
    }
    QgsComposerAttributeTableV2* table = dynamic_cast<QgsComposerAttributeTableV2*>( mf );
    if ( table )
    {
      emit composerTableFrameAdded( table, frame );
    }
    emit selectedItemChanged( frame );
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

  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )mPages.size() ) );
}

void QgsComposition::removePaperItems()
{
  qDeleteAll( mPages );
  mPages.clear();
  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )0 ) );
}

void QgsComposition::deleteAndRemoveMultiFrames()
{
  QSet<QgsComposerMultiFrame*>::iterator multiFrameIt = mMultiFrames.begin();
  for ( ; multiFrameIt != mMultiFrames.end(); ++multiFrameIt )
  {
    delete *multiFrameIt;
  }
  mMultiFrames.clear();
}

void QgsComposition::beginPrintAsPDF( QPrinter& printer, const QString& file )
{
  printer.setOutputFileName( file );
  // setOutputFormat should come after setOutputFileName, which auto-sets format to QPrinter::PdfFormat.
  // [LS] This should be QPrinter::NativeFormat for Mac, otherwise fonts are not embed-able
  // and text is not searchable; however, there are several bugs with <= Qt 4.8.5, 5.1.1, 5.2.0:
  // https://bugreports.qt-project.org/browse/QTBUG-10094 - PDF font embedding fails
  // https://bugreports.qt-project.org/browse/QTBUG-33583 - PDF output converts text to outline
  // Also an issue with PDF paper size using QPrinter::NativeFormat on Mac (always outputs portrait letter-size)
  printer.setOutputFormat( QPrinter::PdfFormat );

  refreshPageSize();
  //must set orientation to portrait before setting paper size, otherwise size will be flipped
  //for landscape sized outputs (#11352)
  printer.setOrientation( QPrinter::Portrait );
  printer.setPaperSize( QSizeF( paperWidth(), paperHeight() ), QPrinter::Millimeter );

  // TODO: add option for this in Composer
  // May not work on Windows or non-X11 Linux. Works fine on Mac using QPrinter::NativeFormat
  //printer.setFontEmbeddingEnabled( true );

  QgsPaintEngineHack::fixEngineFlags( printer.paintEngine() );
}

bool QgsComposition::exportAsPDF( const QString& file )
{
  QPrinter printer;
  beginPrintAsPDF( printer, file );
  return print( printer );
}

void QgsComposition::doPrint( QPrinter& printer, QPainter& p, bool startNewPage )
{
  if ( ddPageSizeActive() )
  {
    //set the page size again so that data defined page size takes effect
    refreshPageSize();
    //must set orientation to portrait before setting paper size, otherwise size will be flipped
    //for landscape sized outputs (#11352)
    printer.setOrientation( QPrinter::Portrait );
    printer.setPaperSize( QSizeF( paperWidth(), paperHeight() ), QPrinter::Millimeter );
  }

  //QgsComposition starts page numbering at 0
  int fromPage = ( printer.fromPage() < 1 ) ? 0 : printer.fromPage() - 1;
  int toPage = ( printer.toPage() < 1 ) ? numPages() - 1 : printer.toPage() - 1;

  bool pageExported = false;
  if ( mPrintAsRaster )
  {
    for ( int i = fromPage; i <= toPage; ++i )
    {
      if ( !shouldExportPage( i + 1 ) )
      {
        continue;
      }
      if (( pageExported && i > fromPage ) || startNewPage )
      {
        printer.newPage();
      }

      QImage image = printPageAsRaster( i );
      if ( !image.isNull() )
      {
        QRectF targetArea( 0, 0, image.width(), image.height() );
        p.drawImage( targetArea, image, targetArea );
      }
      pageExported = true;
    }
  }

  if ( !mPrintAsRaster )
  {
    for ( int i = fromPage; i <= toPage; ++i )
    {
      if ( !shouldExportPage( i + 1 ) )
      {
        continue;
      }
      if (( pageExported && i > fromPage ) || startNewPage )
      {
        printer.newPage();
      }
      renderPage( &p, i );
      pageExported = true;
    }
  }
}

void QgsComposition::beginPrint( QPrinter &printer, const bool evaluateDDPageSize )
{
  //set resolution based on composer setting
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );

  //set user-defined resolution
  printer.setResolution( printResolution() );

  if ( evaluateDDPageSize && ddPageSizeActive() )
  {
    //set data defined page size
    refreshPageSize();
    //must set orientation to portrait before setting paper size, otherwise size will be flipped
    //for landscape sized outputs (#11352)
    printer.setOrientation( QPrinter::Portrait );
    printer.setPaperSize( QSizeF( paperWidth(), paperHeight() ), QPrinter::Millimeter );
  }
}

bool QgsComposition::print( QPrinter &printer, const bool evaluateDDPageSize )
{
  beginPrint( printer, evaluateDDPageSize );
  QPainter p;
  bool ready = p.begin( &printer );
  if ( !ready )
  {
    //error beginning print
    return false;
  }
  doPrint( printer, p );
  p.end();
  return true;
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
    if ( !imagePainter.isActive() ) return QImage();
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

  QRectF paperRect = QRectF( paperItem->pos().x(), paperItem->pos().y(), paperItem->rect().width(), paperItem->rect().height() );

  QgsComposition::PlotStyle savedPlotStyle = mPlotStyle;
  mPlotStyle = QgsComposition::Print;

  setSnapLinesVisible( false );
  //hide background before rendering
  setBackgroundBrush( Qt::NoBrush );
  render( p, QRectF( 0, 0, paintDevice->width(), paintDevice->height() ), paperRect );
  //show background after rendering
  setBackgroundBrush( QColor( 215, 215, 215 ) );
  setSnapLinesVisible( true );

  mPlotStyle = savedPlotStyle;
}

QString QgsComposition::encodeStringForXML( const QString& str )
{
  QString modifiedStr( str );
  modifiedStr.replace( "&", "&amp;" );
  modifiedStr.replace( "\"", "&quot;" );
  modifiedStr.replace( "'", "&apos;" );
  modifiedStr.replace( "<", "&lt;" );
  modifiedStr.replace( ">", "&gt;" );
  return modifiedStr;
}

QGraphicsView *QgsComposition::graphicsView() const
{
  //try to find current view attached to composition
  QList<QGraphicsView*> viewList = views();
  if ( viewList.size() > 0 )
  {
    return viewList.at( 0 );
  }

  //no view attached to composition
  return 0;
}

void QgsComposition::computeWorldFileParameters( double& a, double& b, double& c, double& d, double& e, double& f ) const
{
  //
  // Word file parameters : affine transformation parameters from pixel coordinates to map coordinates

  if ( !mWorldFileMap )
  {
    return;
  }

  QRectF brect = mWorldFileMap->mapRectToScene( mWorldFileMap->rect() );
  QgsRectangle extent = *mWorldFileMap->currentMapExtent();

  double alpha = mWorldFileMap->mapRotation() / 180 * M_PI;

  double xr = extent.width() / brect.width();
  double yr = extent.height() / brect.height();

  double XC = extent.center().x();
  double YC = extent.center().y();

  // get the extent for the page
  double xmin = extent.xMinimum() - mWorldFileMap->pos().x() * xr;
  double ymax = extent.yMaximum() + mWorldFileMap->pos().y() * yr;
  QgsRectangle paperExtent( xmin, ymax - paperHeight() * yr, xmin + paperWidth() * xr, ymax );

  double X0 = paperExtent.xMinimum();
  double Y0 = paperExtent.yMinimum();

  int widthPx = ( int )( printResolution() * paperWidth() / 25.4 );
  int heightPx = ( int )( printResolution() * paperHeight() / 25.4 );

  double Ww = paperExtent.width() / widthPx;
  double Hh = paperExtent.height() / heightPx;

  // scaling matrix
  double s[6];
  s[0] = Ww;
  s[1] = 0;
  s[2] = X0;
  s[3] = 0;
  s[4] = -Hh;
  s[5] = Y0 + paperExtent.height();

  // rotation matrix
  double r[6];
  r[0] = cos( alpha );
  r[1] = -sin( alpha );
  r[2] = XC * ( 1 - cos( alpha ) ) + YC * sin( alpha );
  r[3] = sin( alpha );
  r[4] = cos( alpha );
  r[5] = - XC * sin( alpha ) + YC * ( 1 - cos( alpha ) );

  // result = rotation x scaling = rotation(scaling(X))
  a = r[0] * s[0] + r[1] * s[3];
  b = r[0] * s[1] + r[1] * s[4];
  c = r[0] * s[2] + r[1] * s[5] + r[2];
  d = r[3] * s[0] + r[4] * s[3];
  e = r[3] * s[1] + r[4] * s[4];
  f = r[3] * s[2] + r[4] * s[5] + r[5];
}

bool QgsComposition::setAtlasMode( const AtlasMode mode )
{
  mAtlasMode = mode;

  if ( mode == QgsComposition::AtlasOff )
  {
    mAtlasComposition.endRender();
  }
  else
  {
    bool atlasHasFeatures = mAtlasComposition.beginRender();
    if ( ! atlasHasFeatures )
    {
      mAtlasMode = QgsComposition::AtlasOff;
      mAtlasComposition.endRender();
      return false;
    }
  }

  update();
  return true;
}

bool QgsComposition::ddPageSizeActive() const
{
  //check if any data defined page settings are active
  return dataDefinedActive( QgsComposerObject::PresetPaperSize, &mDataDefinedProperties ) ||
         dataDefinedActive( QgsComposerObject::PaperWidth, &mDataDefinedProperties ) ||
         dataDefinedActive( QgsComposerObject::PaperHeight, &mDataDefinedProperties ) ||
         dataDefinedActive( QgsComposerObject::PaperOrientation, &mDataDefinedProperties );
}

void QgsComposition::refreshPageSize()
{
  double pageWidth = mPageWidth;
  double pageHeight = mPageHeight;

  QVariant exprVal;
  //in order of precedence - first consider predefined page size
  if ( dataDefinedEvaluate( QgsComposerObject::PresetPaperSize, exprVal, &mDataDefinedProperties ) )
  {
    QString presetString = exprVal.toString().trimmed();
    QgsDebugMsg( QString( "exprVal Paper Preset size :%1" ).arg( presetString ) );
    double widthD = 0;
    double heightD = 0;
    if ( QgsComposerUtils::decodePresetPaperSize( presetString, widthD, heightD ) )
    {
      pageWidth = widthD;
      pageHeight = heightD;
    }
  }

  //which is overwritten by data defined width/height
  if ( dataDefinedEvaluate( QgsComposerObject::PaperWidth, exprVal, &mDataDefinedProperties ) )
  {
    bool ok;
    double widthD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Paper Width:%1" ).arg( widthD ) );
    if ( ok )
    {
      pageWidth = widthD;
    }
  }
  if ( dataDefinedEvaluate( QgsComposerObject::PaperHeight, exprVal, &mDataDefinedProperties ) )
  {
    bool ok;
    double heightD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Paper Height:%1" ).arg( heightD ) );
    if ( ok )
    {
      pageHeight = heightD;
    }
  }

  //which is finally overwritten by data defined orientation
  if ( dataDefinedEvaluate( QgsComposerObject::PaperOrientation, exprVal, &mDataDefinedProperties ) )
  {
    bool ok;
    QString orientationString = exprVal.toString().trimmed();
    QgsComposition::PaperOrientation orientation = QgsComposerUtils::decodePaperOrientation( orientationString, ok );
    QgsDebugMsg( QString( "exprVal Paper Orientation:%1" ).arg( orientationString ) );
    if ( ok )
    {
      double heightD, widthD;
      if ( orientation == QgsComposition::Portrait )
      {
        heightD = qMax( pageHeight, pageWidth );
        widthD = qMin( pageHeight, pageWidth );
      }
      else
      {
        heightD = qMin( pageHeight, pageWidth );
        widthD = qMax( pageHeight, pageWidth );
      }
      pageWidth = widthD;
      pageHeight = heightD;
    }
  }

  setPaperSize( pageWidth, pageHeight );
}

QgsDataDefined *QgsComposition::dataDefinedProperty( const QgsComposerObject::DataDefinedProperty property )
{
  if ( property == QgsComposerObject::AllProperties || property == QgsComposerObject::NoProperty )
  {
    //invalid property
    return 0;
  }

  //find matching QgsDataDefined for property
  QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.constEnd() )
  {
    return it.value();
  }

  //not found
  return 0;
}

void QgsComposition::setDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, bool active, bool useExpression, const QString &expression, const QString &field )
{
  if ( property == QgsComposerObject::AllProperties || property == QgsComposerObject::NoProperty )
  {
    //invalid property
    return;
  }

  bool defaultVals = ( !active && !useExpression && expression.isEmpty() && field.isEmpty() );

  if ( mDataDefinedProperties.contains( property ) )
  {
    QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = mDataDefinedProperties.find( property );
    if ( it != mDataDefinedProperties.constEnd() )
    {
      QgsDataDefined* dd = it.value();
      dd->setActive( active );
      dd->setUseExpression( useExpression );
      dd->setExpressionString( expression );
      dd->setField( field );
    }
  }
  else if ( !defaultVals )
  {
    QgsDataDefined* dd = new QgsDataDefined( active, useExpression, expression, field );
    mDataDefinedProperties.insert( property, dd );
  }
}

bool QgsComposition::dataDefinedEvaluate( QgsComposerObject::DataDefinedProperty property, QVariant &expressionValue, QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties )
{
  if ( property == QgsComposerObject::NoProperty || property == QgsComposerObject::AllProperties )
  {
    //invalid property
    return false;
  }

  //null passed-around QVariant
  expressionValue.clear();

  //get fields and feature from atlas
  const QgsFeature* currentFeature = 0;
  const QgsFields* layerFields = 0;
  if ( mAtlasComposition.enabled() )
  {
    QgsVectorLayer* atlasLayer = mAtlasComposition.coverageLayer();
    if ( atlasLayer )
    {
      layerFields = &atlasLayer->pendingFields();
    }
    if ( mAtlasMode != QgsComposition::AtlasOff )
    {
      currentFeature = mAtlasComposition.currentFeature();
    }
  }

  //evaluate data defined property using current atlas context
  QVariant result = dataDefinedValue( property, currentFeature, layerFields, dataDefinedProperties );

  if ( result.isValid() )
  {
    expressionValue = result;
    return true;
  }

  return false;
}

bool QgsComposition::dataDefinedActive( const QgsComposerObject::DataDefinedProperty property, const QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties ) const
{
  if ( property == QgsComposerObject::AllProperties || property == QgsComposerObject::NoProperty )
  {
    //invalid property
    return false;
  }
  if ( !dataDefinedProperties->contains( property ) )
  {
    //missing property
    return false;
  }

  QgsDataDefined* dd = 0;
  QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = dataDefinedProperties->find( property );
  if ( it != dataDefinedProperties->constEnd() )
  {
    dd = it.value();
  }

  if ( !dd )
  {
    return false;
  }

  //found the data defined property, return whether it is active
  return dd->isActive();
}

QVariant QgsComposition::dataDefinedValue( QgsComposerObject::DataDefinedProperty property, const QgsFeature *feature, const QgsFields *fields, QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties ) const
{
  if ( property == QgsComposerObject::AllProperties || property == QgsComposerObject::NoProperty )
  {
    //invalid property
    return QVariant();
  }
  if ( !dataDefinedProperties->contains( property ) )
  {
    //missing property
    return QVariant();
  }

  QgsDataDefined* dd = 0;
  QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = dataDefinedProperties->find( property );
  if ( it != dataDefinedProperties->constEnd() )
  {
    dd = it.value();
  }

  if ( !dd )
  {
    return QVariant();
  }

  if ( !dd->isActive() )
  {
    return QVariant();
  }

  QVariant result = QVariant();
  bool useExpression = dd->useExpression();
  QString field = dd->field();

  if ( !dd->expressionIsPrepared() )
  {
    prepareDataDefinedExpression( dd, dataDefinedProperties );
  }

  if ( useExpression && dd->expressionIsPrepared() )
  {
    QgsExpression* expr = dd->expression();

    result = expr->evaluate( feature );
    if ( expr->hasEvalError() )
    {
      QgsDebugMsgLevel( QString( "Evaluate error:" ) + expr->evalErrorString(), 4 );
      return QVariant();
    }
  }
  else if ( !useExpression && !field.isEmpty() && fields )
  {
    if ( !feature )
    {
      return QVariant();
    }
    // use direct attribute access instead of evaluating "field" expression (much faster)
    int indx = fields->indexFromName( field );
    if ( indx != -1 )
    {
      result = feature->attribute( indx );
    }
  }
  return result;
}

void QgsComposition::prepareDataDefinedExpression( QgsDataDefined *dd, QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties ) const
{
  QgsVectorLayer* atlasLayer = 0;

  if ( mAtlasComposition.enabled() )
  {
    atlasLayer = mAtlasComposition.coverageLayer();
  }

  //if specific QgsDataDefined passed, prepare it
  //otherwise prepare all QgsDataDefineds
  if ( dd )
  {
    dd->prepareExpression( atlasLayer );
  }
  else
  {
    QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >::const_iterator it = dataDefinedProperties->constBegin();
    for ( ; it != dataDefinedProperties->constEnd();  ++it )
    {
      it.value()->prepareExpression( atlasLayer );
    }
  }
}

void QgsComposition::prepareAllDataDefinedExpressions()
{
  prepareDataDefinedExpression( 0, &mDataDefinedProperties );
}

void QgsComposition::relativeResizeRect( QRectF& rectToResize, const QRectF& boundsBefore, const QRectF& boundsAfter )
{
  QgsComposerUtils::relativeResizeRect( rectToResize, boundsBefore, boundsAfter );
}

double QgsComposition::relativePosition( double position, double beforeMin, double beforeMax, double afterMin, double afterMax )
{
  return QgsComposerUtils::relativePosition( position, beforeMin, beforeMax, afterMin, afterMax );
}
