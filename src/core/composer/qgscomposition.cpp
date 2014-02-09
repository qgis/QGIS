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
#include "qgscomposermousehandles.h"
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
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QPrinter>
#include <QSettings>
#include <QDir>

#include <limits>

QgsComposition::QgsComposition( QgsMapRenderer* mapRenderer )
    : QGraphicsScene( 0 )
    , mMapRenderer( mapRenderer )
    , mPlotStyle( QgsComposition::Preview )
    , mPageWidth( 297 )
    , mPageHeight( 210 )
    , mSpaceBetweenPages( 10 )
    , mPageStyleSymbol( 0 )
    , mPrintAsRaster( false )
    , mGenerateWorldFile( false )
    , mWorldFileMap( 0 )
    , mUseAdvancedEffects( true )
    , mSnapToGrid( false )
    , mGridVisible( false )
    , mSnapGridResolution( 0 )
    , mSnapGridTolerance( 0 )
    , mSnapGridOffsetX( 0 )
    , mSnapGridOffsetY( 0 )
    , mAlignmentSnap( true )
    , mGuidesVisible( true )
    , mSmartGuides( true )
    , mAlignmentSnapTolerance( 0 )
    , mSelectionHandles( 0 )
    , mActiveItemCommand( 0 )
    , mActiveMultiFrameCommand( 0 )
    , mAtlasComposition( this )
    , mAtlasMode( QgsComposition::AtlasOff )
    , mPreventCursorChange( false )
{
  setBackgroundBrush( QColor( 215, 215, 215 ) );
  createDefaultPageStyleSymbol();
  addPaperItem();

  //add mouse selection handles to composition, and initially hide
  mSelectionHandles = new QgsComposerMouseHandles( this );
  addItem( mSelectionHandles );
  mSelectionHandles->hide();
  mSelectionHandles->setZValue( 500 );

  mPrintResolution = 300; //hardcoded default

  //load default composition settings
  loadDefaults();
  loadSettings();
}

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
    mPreventCursorChange( false )
{
  //load default composition settings
  loadDefaults();
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
  delete mPageStyleSymbol;
}

void QgsComposition::loadDefaults()
{
  QSettings settings;
  mSnapGridResolution = settings.value( "/Composer/defaultSnapGridResolution", 10.0 ).toDouble();
  mSnapGridTolerance = settings.value( "/Composer/defaultSnapGridTolerance", 2 ).toDouble();
  mSnapGridOffsetX = settings.value( "/Composer/defaultSnapGridOffsetX", 0 ).toDouble();
  mSnapGridOffsetY = settings.value( "/Composer/defaultSnapGridOffsetY", 0 ).toDouble();
  mAlignmentSnapTolerance = settings.value( "/Composer/defaultSnapGuideTolerance", 2 ).toDouble();
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

void QgsComposition::setPageStyleSymbol( QgsFillSymbolV2* symbol )
{
  delete mPageStyleSymbol;
  mPageStyleSymbol = symbol;
}

void QgsComposition::createDefaultPageStyleSymbol()
{
  delete mPageStyleSymbol;
  QgsStringMap properties;
  properties.insert( "color", "white" );
  properties.insert( "style", "solid" );
  properties.insert( "style_border", "no" );
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

QgsComposerItem* QgsComposition::composerItemAt( const QPointF & position )
{
  return composerItemAt( position, 0 );
}

QgsComposerItem* QgsComposition::composerItemAt( const QPointF & position, const QgsComposerItem* belowItem )
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
      if ( ! belowItem || foundBelowItem )
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
  compositionElem.setAttribute( "snapGridTolerance", QString::number( mSnapGridTolerance ) );
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

  QDomElement pageStyleSymbolElem = compositionElem.firstChildElement( "symbol" );
  if ( !pageStyleSymbolElem.isNull() )
  {
    delete mPageStyleSymbol;
    mPageStyleSymbol = dynamic_cast<QgsFillSymbolV2*>( QgsSymbolLayerV2Utils::loadSymbol( pageStyleSymbolElem ) );
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
  mSnapGridTolerance = compositionElem.attribute( "snapGridTolerance", "2.0" ).toDouble();
  mSnapGridOffsetX = compositionElem.attribute( "snapGridOffsetX" ).toDouble();
  mSnapGridOffsetY = compositionElem.attribute( "snapGridOffsetY" ).toDouble();

  mAlignmentSnap = compositionElem.attribute( "alignmentSnap", "1" ).toInt() == 0 ? false : true;
  mGuidesVisible = compositionElem.attribute( "guidesVisible", "1" ).toInt() == 0 ? false : true;
  mSmartGuides = compositionElem.attribute( "smartGuides", "1" ).toInt() == 0 ? false : true;
  mAlignmentSnapTolerance = compositionElem.attribute( "alignmentSnapTolerance", "2.0" ).toDouble();

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
  int zOrderOffset = mItemZList.size();

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
    clearSelection();
  }

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
    newMap->readXML( currentComposerMapElem, doc );
    newMap->assignFreeId();

    if ( mapsToRestore )
    {
      mapsToRestore->insert( newMap, ( int )( newMap->previewMode() ) );
      newMap->setPreviewMode( QgsComposerMap::Rectangle );
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
    if (( *mit )->overviewFrameMapId() != -1 )
    {
      const QgsComposerMap* overviewMap = getComposerMapById(( *mit )->overviewFrameMapId() );
      if ( overviewMap )
      {
        QObject::connect( overviewMap, SIGNAL( extentChanged() ), *mit, SLOT( overviewExtentChanged() ) );
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
  //TODO - fix this. pasting html items has no effect
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
  refreshZList();

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

QgsComposerItem* QgsComposition::getComposerItemAbove( QgsComposerItem* item )
{
  //search item z list for selected item
  QLinkedListIterator<QgsComposerItem*> it( mItemZList );
  if ( it.findNext( item ) )
  {
    //return next item (list is sorted from lowest->highest items)
    if ( it.hasNext() )
    {
      return it.next();
    }
  }
  return 0;
}

QgsComposerItem* QgsComposition::getComposerItemBelow( QgsComposerItem* item )
{
  //search item z list for selected item
  QLinkedListIterator<QgsComposerItem*> it( mItemZList );
  if ( it.findNext( item ) )
  {
    //move position to before selected item
    it.previous();
    //now find previous item, since list is sorted from lowest->highest items
    if ( it.hasPrevious() )
    {
      return it.previous();
    }
  }
  return 0;
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
  clearSelection();
  selectedItem->setSelected( true );
  emit selectedItemChanged( selectedItem );
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
    ( *align_it )->setPos( minXCoordinate, ( *align_it )->pos().y() );
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
    ( *align_it )->setPos( averageXCoord - ( *align_it )->rect().width() / 2.0, ( *align_it )->pos().y() );
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
    ( *align_it )->setPos( maxXCoordinate - ( *align_it )->rect().width(), ( *align_it )->pos().y() );
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
    ( *align_it )->setPos(( *align_it )->pos().x(), minYCoordinate );
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
    ( *align_it )->setPos(( *align_it )->pos().x(), averageYCoord - ( *align_it )->rect().height() / 2 );
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
    ( *align_it )->setPos(( *align_it )->pos().x(), maxYCoord - ( *align_it )->rect().height() );
    subcommand->saveAfterState();
  }
  mUndoStack.push( parentCommand );
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

  clearSelection();
  mUndoStack.push( parentCommand );
}

void QgsComposition::unlockAllItems()
{
  //unlock all items in composer

  QUndoCommand* parentCommand = new QUndoCommand( tr( "Items unlocked" ) );

  //first, clear the selection
  clearSelection();

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
  mUndoStack.push( parentCommand );
}

void QgsComposition::updateZValues( bool addUndoCommands )
{
  int counter = 1;
  QLinkedList<QgsComposerItem*>::iterator it = mItemZList.begin();
  QgsComposerItem* currentItem = 0;

  QUndoCommand* parentCommand;
  if ( addUndoCommands )
  {
    parentCommand = new QUndoCommand( tr( "Item z-order changed" ) );
  }
  for ( ; it != mItemZList.end(); ++it )
  {
    currentItem = *it;
    if ( currentItem )
    {
      QgsComposerItemCommand* subcommand;
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
    ++counter;
  }
  if ( addUndoCommands )
  {
    mUndoStack.push( parentCommand );
  }
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

void QgsComposition::refreshZList()
{
  QLinkedList<QgsComposerItem*> sortedList;

  //rebuild the item z order list based on the current zValues of items in the scene

  //get items in descending zValue order
  QList<QGraphicsItem*> itemList = items();
  QList<QGraphicsItem*>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem*>( *itemIt );
    if ( composerItem )
    {
      if ( composerItem->type() != QgsComposerItem::ComposerPaper && composerItem->type() != QgsComposerItem::ComposerFrame )
      {
        //since the z order list is in ascending zValue order (opposite order to itemList), we prepend each item
        sortedList.prepend( composerItem );
      }
    }
  }

  mItemZList = sortedList;

  //Finally, rebuild the zValue of all items to remove any duplicate zValues and make sure there's
  //no missing zValues.
  updateZValues( false );
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

  double xSnapped = xRatio * mSnapGridResolution + mSnapGridOffsetX;
  double ySnapped = yRatio * mSnapGridResolution + mSnapGridOffsetY + yOffset;

  if ( abs( xSnapped - scenePoint.x() ) > mSnapGridTolerance )
  {
    //snap distance is outside of tolerance
    xSnapped = scenePoint.x();
  }
  if ( abs( ySnapped - scenePoint.y() ) > mSnapGridTolerance )
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

void QgsComposition::setSnapLinesVisible( bool visible )
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

void QgsComposition::setSnapToGridEnabled( bool b )
{
  mSnapToGrid = b;
  updatePaperItems();
}

void QgsComposition::setGridVisible( bool b )
{
  mGridVisible = b;
  updatePaperItems();
}

void QgsComposition::setSnapGridResolution( double r )
{
  mSnapGridResolution = r;
  updatePaperItems();
}

void QgsComposition::setSnapGridTolerance( double tolerance )
{
  mSnapGridTolerance = tolerance;
}

void QgsComposition::setSnapGridOffsetX( double offset )
{
  mSnapGridOffsetX = offset;
  updatePaperItems();
}

void QgsComposition::setSnapGridOffsetY( double offset )
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

void QgsComposition::setGridStyle( GridStyle s )
{
  mGridStyle = s;
  updatePaperItems();
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
}

void QgsComposition::addComposerLabel( QgsComposerLabel* label )
{
  addItem( label );
  emit composerLabelAdded( label );
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
}

void QgsComposition::addComposerScaleBar( QgsComposerScaleBar* scaleBar )
{
  addItem( scaleBar );
  emit composerScaleBarAdded( scaleBar );
}

void QgsComposition::addComposerLegend( QgsComposerLegend* legend )
{
  addItem( legend );
  emit composerLegendAdded( legend );
}

void QgsComposition::addComposerPicture( QgsComposerPicture* picture )
{
  addItem( picture );
  emit composerPictureAdded( picture );
}

void QgsComposition::addComposerShape( QgsComposerShape* shape )
{
  addItem( shape );
  emit composerShapeAdded( shape );
}

void QgsComposition::addComposerTable( QgsComposerAttributeTable* table )
{
  addItem( table );
  emit composerTableAdded( table );
}

void QgsComposition::addComposerHtmlFrame( QgsComposerHtml* html, QgsComposerFrame* frame )
{
  addItem( frame );
  emit composerHtmlFrameAdded( html, frame );
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

bool QgsComposition::setAtlasMode( QgsComposition::AtlasMode mode )
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
      return false;
    }
  }

  QList<QgsComposerMap*> maps;
  composerItems( maps );
  for ( QList<QgsComposerMap*>::iterator mit = maps.begin(); mit != maps.end(); ++mit )
  {
    QgsComposerMap* currentMap = ( *mit );
    if ( !currentMap->atlasDriven() )
    {
      continue;
    }
    currentMap->toggleAtlasPreview();
  }

  update();
  return true;
}

void QgsComposition::relativeResizeRect( QRectF& rectToResize, const QRectF& boundsBefore, const QRectF& boundsAfter )
{
  //linearly scale rectToResize relative to the scaling from boundsBefore to boundsAfter
  double left = relativePosition( rectToResize.left(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double right = relativePosition( rectToResize.right(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double top = relativePosition( rectToResize.top(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );
  double bottom = relativePosition( rectToResize.bottom(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );

  rectToResize.setRect( left, top, right - left, bottom - top );
}

double QgsComposition::relativePosition( double position, double beforeMin, double beforeMax, double afterMin, double afterMax )
{
  //calculate parameters for linear scale between before and after ranges
  double m = ( afterMax - afterMin ) / ( beforeMax - beforeMin );
  double c = afterMin - ( beforeMin * m );

  //return linearly scaled position
  return m * position + c;
}
