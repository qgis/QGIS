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
#include "qgscomposerlabel.h"
#include "qgscomposerattributetable.h"
#include "qgsaddremovemultiframecommand.h"
#include "qgscomposermultiframecommand.h"
#include "qgspaintenginehack.h"
#include "qgspaperitem.h"
#include "qgsproject.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QPrinter>
#include <QSettings>
#include <QDir>


QgsComposition::QgsComposition( QgsMapRenderer* mapRenderer )
    : QGraphicsScene( 0 )
    , mMapRenderer( mapRenderer )
    , mPlotStyle( QgsComposition::Preview )
    , mPageWidth( 297 )
    , mPageHeight( 210 )
    , mSpaceBetweenPages( 10 )
    , mPrintAsRaster( false )
    , mGenerateWorldFile( false )
    , mWorldFileMap( 0 )
    , mUseAdvancedEffects( true )
    , mSelectionTolerance( 0.0 )
    , mSnapToGrid( false )
    , mSnapGridResolution( 10.0 )
    , mSnapGridOffsetX( 0.0 )
    , mSnapGridOffsetY( 0.0 )
    , mAlignmentSnap( true )
    , mAlignmentSnapTolerance( 2 )
    , mActiveItemCommand( 0 )
    , mActiveMultiFrameCommand( 0 )
    , mAtlasComposition( this )
{
  setBackgroundBrush( Qt::gray );
  addPaperItem();

  mPrintResolution = 300; //hardcoded default
  loadSettings();
}

QgsComposition::QgsComposition()
    : QGraphicsScene( 0 ),
    mMapRenderer( 0 ),
    mPlotStyle( QgsComposition::Preview ),
    mPageWidth( 297 ),
    mPageHeight( 210 ),
    mSpaceBetweenPages( 10 ),
    mPrintAsRaster( false ),
    mGenerateWorldFile( false ),
    mWorldFileMap( 0 ),
    mUseAdvancedEffects( true ),
    mSelectionTolerance( 0.0 ),
    mSnapToGrid( false ),
    mSnapGridResolution( 10.0 ),
    mSnapGridOffsetX( 0.0 ),
    mSnapGridOffsetY( 0.0 ),
    mAlignmentSnap( true ),
    mAlignmentSnapTolerance( 2 ),
    mActiveItemCommand( 0 ),
    mActiveMultiFrameCommand( 0 ),
    mAtlasComposition( this )
{
  loadSettings();

}

QgsComposition::~QgsComposition()
{
  removePaperItems();
  deleteAndRemoveMultiFrames();

  // make sure that all composer items are removed before
  // this class is deconstructed - to avoid segfaults
  // when composer items access in destructor composition that isn't valid anymore
  clear();
  delete mActiveItemCommand;
  delete mActiveMultiFrameCommand;
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

  // update the corresponding variable
  QgsExpression::setSpecialColumn( "$numpages", QVariant(( int )numPages() ) );

  emit nPagesChanged();
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

int QgsComposition::pageNumberAt( const QPointF& position ) const
{
  return position.y() / ( paperHeight() + spaceBetweenPages() );
}

int QgsComposition::itemPageNumber( const QgsComposerItem* item ) const
{
  return pageNumberAt( QPointF( item->transform().dx(), item->transform().dy() ) );
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

const QgsComposerItem* QgsComposition::getComposerItemById( QString theId ) const
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

const QgsComposerItem* QgsComposition::getComposerItemByUuid( QString theUuid ) const
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


void QgsComposition::setUseAdvancedEffects( bool effectsEnabled )
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
  //in QgsComposition, one unit = one mm
  double sizeMillimeters = pointSize * 0.3527;
  return qRound( sizeMillimeters ); //round to nearest mm
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
  compositionElem.setAttribute( "alignmentSnapTolerance", mAlignmentSnapTolerance );

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
  emit paperSizeChanged();
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

  mAlignmentSnap = compositionElem.attribute( "alignmentSnap", "1" ).toInt() == 0 ? false : true;
  mAlignmentSnapTolerance = compositionElem.attribute( "alignmentSnapTolerance", "2.0" ).toDouble();

  mPrintAsRaster = compositionElem.attribute( "printAsRaster" ).toInt();
  mPrintResolution = compositionElem.attribute( "printResolution", "300" ).toInt();

  mGenerateWorldFile = compositionElem.attribute( "generateWorldFile", "0" ).toInt() == 1 ? true : false;

  updatePaperItems();

  return true;
}

bool QgsComposition::loadFromTemplate( const QDomDocument& doc, QMap<QString, QString>* substitutionMap, bool addUndoCommands )
{
  deleteAndRemoveMultiFrames();

  //delete all items and emit itemRemoved signal
  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIter = itemList.begin();
  for ( ; itemIter != itemList.end(); ++itemIter )
  {
    QgsComposerItem* cItem = dynamic_cast<QgsComposerItem*>( *itemIter );
    if ( cItem )
    {
      removeItem( cItem );
      emit itemRemoved( cItem );
      delete cItem;
    }
  }
  mItemZList.clear();

  mPages.clear();
  mUndoStack.clear();

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

  // read atlas parameters
  QDomElement atlasElem = importDoc.documentElement().firstChildElement( "Atlas" );
  atlasComposition().readXML( atlasElem, importDoc );
  return true;
}

void QgsComposition::addItemsFromXML( const QDomElement& elem, const QDomDocument& doc, QMap< QgsComposerMap*, int >* mapsToRestore,
                                      bool addUndoCommands, QPointF* pos, bool pasteInPlace )
{
  QPointF* pasteInPlacePt = 0;
  if ( pasteInPlace )
  {
    pasteInPlacePt = new QPointF( 0, pageNumberAt( *pos ) * ( mPageHeight + mSpaceBetweenPages ) );
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
        newLabel->setItemPosition( newLabel->transform().dx(), fmod( newLabel->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newLabel->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newLabel->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newMap->setItemPosition( newMap->transform().dx(), fmod( newMap->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newMap->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newMap->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newArrow->setItemPosition( newArrow->transform().dx(), fmod( newArrow->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newArrow->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newArrow->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newScaleBar->setItemPosition( newScaleBar->transform().dx(), fmod( newScaleBar->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newScaleBar->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newScaleBar->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newShape->setItemPosition( newShape->transform().dx(), fmod( newShape->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newShape->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newShape->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newPicture->setItemPosition( newPicture->transform().dx(), fmod( newPicture->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newPicture->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newPicture->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newLegend->setItemPosition( newLegend->transform().dx(), fmod( newLegend->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newLegend->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newLegend->setItemPosition( pos->x(), pos->y() );
      }
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
      if ( pasteInPlace )
      {
        newTable->setItemPosition( newTable->transform().dx(), fmod( newTable->transform().dy(), ( paperHeight() + spaceBetweenPages() ) ) );
        newTable->move( pasteInPlacePt->x(), pasteInPlacePt->y() );
      }
      else
      {
        newTable->setItemPosition( pos->x(), pos->y() );
      }
    }
    addComposerTable( newTable );
    if ( addUndoCommands )
    {
      pushAddRemoveCommand( newTable, tr( "Table added" ) );
    }
  }
  // html
  QDomNodeList composerHtmlList = elem.elementsByTagName( "ComposerHtml" );
  for ( int i = 0; i < composerHtmlList.size(); ++i )
  {
    QDomElement currentHtmlElem = composerHtmlList.at( i ).toElement();
    QgsComposerHtml* newHtml = new QgsComposerHtml( this, false );
    newHtml->readXML( currentHtmlElem, doc );
    newHtml->setCreateUndoCommands( true );
    this->addMultiFrame( newHtml );
  }


  // groups (must be last as it references uuids of above items)
  QDomNodeList groupList = elem.elementsByTagName( "ComposerItemGroup" );
  for ( int i = 0; i < groupList.size(); ++i )
  {
    QDomElement groupElem = groupList.at( i ).toElement();
    QgsComposerItemGroup *newGroup = new QgsComposerItemGroup( this );
    newGroup->readXML( groupElem, doc );
    addItem( newGroup );
  }
}

void QgsComposition::addItemToZList( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }
  mItemZList.push_back( item );
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
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items horizontal center" ) );
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
  QUndoCommand* parentCommand = new QUndoCommand( tr( "Aligned items vertical center" ) );
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

QPointF QgsComposition::alignItem( const QgsComposerItem* item, double& alignX, double& alignY, double dx, double dy )
{
  if ( !item )
  {
    return QPointF();
  }

  double left = item->transform().dx() + dx;
  double right = left + item->rect().width();
  double midH = ( left + right ) / 2.0;
  double top = item->transform().dy() + dy;
  double bottom = top + item->rect().height();
  double midV = ( top + bottom ) / 2.0;

  QMap<double, const QgsComposerItem* > xAlignCoordinates;
  QMap<double, const QgsComposerItem* > yAlignCoordinates;
  collectAlignCoordinates( xAlignCoordinates, yAlignCoordinates, item );

  //find nearest matches x
  double xItemLeft = left; //new left coordinate of the item
  double xAlignCoord = 0;
  double smallestDiffX = DBL_MAX;

  checkNearestItem( left, xAlignCoordinates, smallestDiffX, 0, xItemLeft, xAlignCoord );
  checkNearestItem( midH, xAlignCoordinates, smallestDiffX, ( left - right ) / 2.0, xItemLeft, xAlignCoord );
  checkNearestItem( right, xAlignCoordinates, smallestDiffX, left - right, xItemLeft, xAlignCoord );

  //find nearest matches y
  double yItemTop = top; //new top coordinate of the item
  double yAlignCoord = 0;
  double smallestDiffY = DBL_MAX;

  checkNearestItem( top, yAlignCoordinates, smallestDiffY, 0, yItemTop, yAlignCoord );
  checkNearestItem( midV, yAlignCoordinates, smallestDiffY, ( top - bottom ) / 2.0, yItemTop, yAlignCoord );
  checkNearestItem( bottom, yAlignCoordinates, smallestDiffY, top - bottom, yItemTop, yAlignCoord );

  double xCoord = ( smallestDiffX < 5 ) ? xItemLeft : item->transform().dx() + dx;
  alignX = ( smallestDiffX < 5 ) ? xAlignCoord : -1;
  double yCoord = ( smallestDiffY < 5 ) ? yItemTop : item->transform().dy() + dy;
  alignY = ( smallestDiffY < 5 ) ? yAlignCoord : -1;
  return QPointF( xCoord, yCoord );
}

QPointF QgsComposition::alignPos( const QPointF& pos, const QgsComposerItem* excludeItem, double& alignX, double& alignY )
{
  QMap<double, const QgsComposerItem* > xAlignCoordinates;
  QMap<double, const QgsComposerItem* > yAlignCoordinates;
  collectAlignCoordinates( xAlignCoordinates, yAlignCoordinates, excludeItem );

  double nearestX = pos.x();
  double nearestY = pos.y();
  if ( !nearestItem( xAlignCoordinates, pos.x(), nearestX )
       || !nearestItem( yAlignCoordinates, pos.y(), nearestY ) )
  {
    alignX = -1;
    alignY = -1;
    return pos;
  }

  QPointF result( pos.x(), pos.y() );
  if ( abs( nearestX - pos.x() ) < mAlignmentSnapTolerance )
  {
    result.setX( nearestX );
    alignX = nearestX;
  }
  else
  {
    alignX = -1;
  }

  if ( abs( nearestY - pos.y() ) < mAlignmentSnapTolerance )
  {
    result.setY( nearestY );
    alignY = nearestY;
  }
  else
  {
    alignY = -1;
  }
  return result;
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

void QgsComposition::setSnapLinesVisible( bool visible )
{
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

QGraphicsLineItem* QgsComposition::nearestSnapLine( bool horizontal, double x, double y, double tolerance,
    QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode> >& snappedItems )
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
    else if ( !itemHorizontal )
    {
      currentXCoord = ( *it )->line().x1();
      currentSqrDist = ( x - currentXCoord ) * ( x - currentXCoord );
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
        if ( qgsDoubleNear( currentYCoord, currentItem->transform().dy() + currentItem->rect().top(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::UpperMiddle ) );
        }
        else if ( qgsDoubleNear( currentYCoord, currentItem->transform().dy() + currentItem->rect().center().y(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::Middle ) );
        }
        else if ( qgsDoubleNear( currentYCoord, currentItem->transform().dy() + currentItem->rect().bottom(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::LowerMiddle ) );
        }
      }
      else
      {
        if ( qgsDoubleNear( currentXCoord, currentItem->transform().dx(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::MiddleLeft ) );
        }
        else if ( qgsDoubleNear( currentXCoord, currentItem->transform().dx() + currentItem->rect().center().x(), itemTolerance ) )
        {
          snappedItems.append( qMakePair( currentItem, QgsComposerItem::Middle ) );
        }
        else if ( qgsDoubleNear( currentXCoord, currentItem->transform().dx() + currentItem->rect().width(), itemTolerance ) )
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
      mUndoStack.push( mActiveItemCommand );
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

void QgsComposition::beginMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text )
{
  delete mActiveMultiFrameCommand;
  mActiveMultiFrameCommand = new QgsComposerMultiFrameCommand( multiFrame, text );
  mActiveMultiFrameCommand->savePreviousState();
}

void QgsComposition::endMultiFrameCommand()
{
  if ( mActiveMultiFrameCommand )
  {
    mActiveMultiFrameCommand->saveAfterState();
    if ( mActiveMultiFrameCommand->containsChange() )
    {
      mUndoStack.push( mActiveMultiFrameCommand );
      QgsProject::instance()->dirty( true );
    }
    else
    {
      delete mActiveMultiFrameCommand;
    }
    mActiveMultiFrameCommand = 0;
  }
}

void QgsComposition::addMultiFrame( QgsComposerMultiFrame* multiFrame )
{
  mMultiFrames.insert( multiFrame );
}

void QgsComposition::removeMultiFrame( QgsComposerMultiFrame* multiFrame )
{
  mMultiFrames.remove( multiFrame );
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

void QgsComposition::removeComposerItem( QgsComposerItem* item, bool createCommand )
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
}

void QgsComposition::pushAddRemoveCommand( QgsComposerItem* item, const QString& text, QgsAddRemoveItemCommand::State state )
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
  for ( int i = 0; i < mPages.size(); ++i )
  {
    delete mPages.at( i );
  }
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
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( file );
  printer.setPaperSize( QSizeF( paperWidth(), paperHeight() ), QPrinter::Millimeter );

  QgsPaintEngineHack::fixEngineFlags( printer.paintEngine() );
}

void QgsComposition::exportAsPDF( const QString& file )
{
  QPrinter printer;
  beginPrintAsPDF( printer, file );
  print( printer );
}

void QgsComposition::doPrint( QPrinter& printer, QPainter& p )
{
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

void QgsComposition::beginPrint( QPrinter &printer )
{
  //set resolution based on composer setting
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );

  //set user-defined resolution
  printer.setResolution( printResolution() );
}

void QgsComposition::print( QPrinter &printer )
{
  beginPrint( printer );
  QPainter p( &printer );
  doPrint( printer, p );
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

  QRectF paperRect = QRectF( paperItem->transform().dx(), paperItem->transform().dy(), paperItem->rect().width(), paperItem->rect().height() );

  QgsComposition::PlotStyle savedPlotStyle = mPlotStyle;
  mPlotStyle = QgsComposition::Print;

  setSnapLinesVisible( false );
  render( p, QRectF( 0, 0, paintDevice->width(), paintDevice->height() ), paperRect );
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

void QgsComposition::collectAlignCoordinates( QMap< double, const QgsComposerItem* >& alignCoordsX, QMap< double, const QgsComposerItem* >& alignCoordsY,
    const QgsComposerItem* excludeItem )
{
  alignCoordsX.clear();
  alignCoordsY.clear();

  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    const QgsComposerItem* currentItem = dynamic_cast<const QgsComposerItem *>( *itemIt );
    if ( excludeItem )
    {
      if ( !currentItem || currentItem == excludeItem )
      {
        continue;
      }
      alignCoordsX.insert( currentItem->transform().dx(), currentItem );
      alignCoordsX.insert( currentItem->transform().dx() + currentItem->rect().width(), currentItem );
      alignCoordsX.insert( currentItem->transform().dx() + currentItem->rect().center().x(), currentItem );
      alignCoordsY.insert( currentItem->transform().dy() + currentItem->rect().top(), currentItem );
      alignCoordsY.insert( currentItem->transform().dy() + currentItem->rect().center().y(), currentItem );
      alignCoordsY.insert( currentItem->transform().dy() + currentItem->rect().bottom(), currentItem );
    }
  }

  //arbitrary snap lines
  QList< QGraphicsLineItem* >::const_iterator sIt = mSnapLines.constBegin();
  for ( ; sIt != mSnapLines.constEnd(); ++sIt )
  {
    double x = ( *sIt )->line().x1();
    double y = ( *sIt )->line().y1();
    if ( qgsDoubleNear( y, 0.0 ) )
    {
      alignCoordsX.insert( x, 0 );
    }
    else
    {
      alignCoordsY.insert( y, 0 );
    }
  }
}

void QgsComposition::checkNearestItem( double checkCoord, const QMap< double, const QgsComposerItem* >& alignCoords, double& smallestDiff,
                                       double itemCoordOffset, double& itemCoord, double& alignCoord ) const
{
  double currentCoord = 0;
  if ( !nearestItem( alignCoords, checkCoord, currentCoord ) )
  {
    return;
  }

  double currentDiff = abs( checkCoord - currentCoord );
  if ( currentDiff < mAlignmentSnapTolerance )
  {
    itemCoord = currentCoord + itemCoordOffset;
    alignCoord = currentCoord;
    smallestDiff = currentDiff;
  }
}

bool QgsComposition::nearestItem( const QMap< double, const QgsComposerItem* >& coords, double value, double& nearestValue )
{
  if ( coords.size() < 1 )
  {
    return false;
  }

  QMap< double, const QgsComposerItem* >::const_iterator it = coords.lowerBound( value );
  if ( it == coords.constBegin() ) //value smaller than first map value
  {
    nearestValue = it.key();
    return true;
  }
  else if ( it == coords.constEnd() ) //value larger than last map value
  {
    --it;
    nearestValue = it.key();
    return true;
  }
  else
  {
    //get smaller value and larger value and return the closer one
    double upperVal = it.key();
    --it;
    double lowerVal = it.key();

    double lowerDiff = value - lowerVal;
    double upperDiff = upperVal - value;
    if ( lowerDiff < upperDiff )
    {
      nearestValue = lowerVal;
      return true;
    }
    else
    {
      nearestValue = upperVal;
      return true;
    }
  }
}

void QgsComposition::computeWorldFileParameters( double& a, double& b, double& c, double& d, double& e, double& f ) const
{
  //
  // Word file parameters : affine transformation parameters from pixel coordinates to map coordinates

  if ( !mWorldFileMap )
  {
    return;
  }

  QRectF brect = mWorldFileMap->boundingRect();
  QgsRectangle extent = mWorldFileMap->extent();

  double alpha = mWorldFileMap->rotation() / 180 * M_PI;

  double xr = extent.width() / brect.width();
  double yr = extent.height() / brect.height();

  double XC = extent.center().x();
  double YC = extent.center().y();

  // get the extent for the page
  double xmin = extent.xMinimum() - mWorldFileMap->transform().dx() * xr;
  double ymax = extent.yMaximum() + mWorldFileMap->transform().dy() * yr;
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
