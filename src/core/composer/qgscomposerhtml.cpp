/***************************************************************************
                              qgscomposerhtml.cpp
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"
#include "qgsaddremovemultiframecommand.h"
#include "qgsnetworkaccessmanager.h"

#include <QCoreApplication>
#include <QPainter>
#include <QWebFrame>
#include <QWebPage>
#include <QImage>

QgsComposerHtml::QgsComposerHtml( QgsComposition* c, bool createUndoCommands ): QgsComposerMultiFrame( c, createUndoCommands ),
    mWebPage( 0 ),
    mLoaded( false ),
    mHtmlUnitsToMM( 1.0 ),
    mRenderedPage( 0 ),
    mUseSmartBreaks( true ),
    mMaxBreakDistance( 10 )
{
  mHtmlUnitsToMM = htmlUnitsToMM();
  mWebPage = new QWebPage();
  mWebPage->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
  QObject::connect( mWebPage, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );
  if ( mComposition )
  {
    QObject::connect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( handleFrameRemoval( QgsComposerItem* ) ) );
    connect( mComposition, SIGNAL( refreshItemsTriggered() ), this, SLOT( loadHtml() ) );
  }
}

QgsComposerHtml::QgsComposerHtml(): QgsComposerMultiFrame( 0, false ),
    mWebPage( 0 ),
    mLoaded( false ),
    mHtmlUnitsToMM( 1.0 ),
    mRenderedPage( 0 ),
    mUseSmartBreaks( true ),
    mMaxBreakDistance( 10 )
{
}

QgsComposerHtml::~QgsComposerHtml()
{
  delete mWebPage;
  delete mRenderedPage;
}

void QgsComposerHtml::setUrl( const QUrl& url )
{
  if ( !mWebPage )
  {
    return;
  }

  mUrl = url;
  loadHtml();
}

void QgsComposerHtml::loadHtml()
{
  if ( !mWebPage || mUrl.isEmpty() )
  {
    return;
  }

  mLoaded = false;
  mWebPage->mainFrame()->load( mUrl );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }

  if ( frameCount() < 1 )  return;

  QSize contentsSize = mWebPage->mainFrame()->contentsSize();
  contentsSize.setWidth( mFrameItems.at( 0 )->boundingRect().width() * mHtmlUnitsToMM );
  mWebPage->setViewportSize( contentsSize );
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
  mSize.setWidth( contentsSize.width() / mHtmlUnitsToMM );
  mSize.setHeight( contentsSize.height() / mHtmlUnitsToMM );

  renderCachedImage();

  recalculateFrameSizes();
  emit changed();
}

void QgsComposerHtml::frameLoaded( bool ok )
{
  Q_UNUSED( ok );
  mLoaded = true;
}

void QgsComposerHtml::renderCachedImage()
{
  //render page to cache image
  if ( mRenderedPage )
  {
    delete mRenderedPage;
  }
  mRenderedPage = new QImage( mWebPage->viewportSize(), QImage::Format_ARGB32 );
  QPainter painter;
  painter.begin( mRenderedPage );
  mWebPage->mainFrame()->render( &painter );
  painter.end();
}

QSizeF QgsComposerHtml::totalSize() const
{
  return mSize;
}

void QgsComposerHtml::render( QPainter* p, const QRectF& renderExtent )
{
  if ( !mWebPage )
  {
    return;
  }

  p->save();
  p->scale( 1.0 / mHtmlUnitsToMM, 1.0 / mHtmlUnitsToMM );
  p->translate( 0.0, -renderExtent.top() * mHtmlUnitsToMM );
  mWebPage->mainFrame()->render( p, QRegion( renderExtent.left(), renderExtent.top() * mHtmlUnitsToMM, renderExtent.width() * mHtmlUnitsToMM, renderExtent.height() * mHtmlUnitsToMM ) );
  p->restore();
}

double QgsComposerHtml::htmlUnitsToMM()
{
  if ( !mComposition )
  {
    return 1.0;
  }

  return ( mComposition->printResolution() / 72.0 ); //webkit seems to assume a standard dpi of 96
}

void QgsComposerHtml::addFrame( QgsComposerFrame* frame, bool recalcFrameSizes )
{
  mFrameItems.push_back( frame );
  QObject::connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );
  if ( mComposition )
  {
    mComposition->addComposerHtmlFrame( this, frame );
  }

  if ( recalcFrameSizes )
  {
    recalculateFrameSizes();
  }
}

bool candidateSort( const QPair<int, int> &c1, const QPair<int, int> &c2 )
{
  if ( c1.second < c2.second )
    return true;
  else if ( c1.second > c2.second )
    return false;
  else if ( c1.first > c2.first )
    return true;
  else
    return false;
}

double QgsComposerHtml::findNearbyPageBreak( double yPos )
{
  if ( !mWebPage || !mRenderedPage || !mUseSmartBreaks )
  {
    return yPos;
  }

  //convert yPos to pixels
  int idealPos = yPos * htmlUnitsToMM();

  //if ideal break pos is past end of page, there's nothing we need to do
  if ( idealPos >= mRenderedPage->height() )
  {
    return yPos;
  }

  int maxSearchDistance = mMaxBreakDistance * htmlUnitsToMM();

  //loop through all lines just before ideal break location, up to max distance
  //of maxSearchDistance
  int changes = 0;
  QRgb currentColor;
  QRgb pixelColor;
  QList< QPair<int, int> > candidates;
  int minRow = qMax( idealPos - maxSearchDistance, 0 );
  for ( int candidateRow = idealPos; candidateRow >= minRow; --candidateRow )
  {
    changes = 0;
    currentColor = qRgba( 0, 0, 0, 0 );
    //check all pixels in this line
    for ( int col = 0; col < mRenderedPage->width(); ++col )
    {
      //count how many times the pixels change color in this row
      //eventually, we select a row to break at with the minimum number of color changes
      //since this is likely a line break, or gap between table cells, etc
      //but very unlikely to be midway through a text line or picture
      pixelColor = mRenderedPage->pixel( col, candidateRow );
      if ( pixelColor != currentColor )
      {
        //color has changed
        currentColor = pixelColor;
        changes++;
      }
    }
    candidates.append( qMakePair( candidateRow, changes ) );
  }

  //sort candidate rows by number of changes ascending, row number descending
  qSort( candidates.begin(), candidates.end(), candidateSort );
  //first candidate is now the largest row with smallest number of changes

  //ok, now take the mid point of the best candidate position
  //we do this so that the spacing between text lines is likely to be split in half
  //otherwise the html will be broken immediately above a line of text, which
  //looks a little messy
  int maxCandidateRow = candidates[0].first;
  int minCandidateRow = maxCandidateRow + 1;
  int minCandidateChanges = candidates[0].second;

  QList< QPair<int, int> >::iterator it;
  for ( it = candidates.begin(); it != candidates.end(); ++it )
  {
    if (( *it ).second != minCandidateChanges || ( *it ).first != minCandidateRow - 1 )
    {
      //no longer in a consecutive block of rows of minimum pixel color changes
      //so return the row mid-way through the block
      //first converting back to mm
      return ( minCandidateRow + ( maxCandidateRow - minCandidateRow ) / 2 ) / htmlUnitsToMM();
    }
    minCandidateRow = ( *it ).first;
  }

  //above loop didn't work for some reason
  //return first candidate converted to mm
  return candidates[0].first / htmlUnitsToMM();
}

void QgsComposerHtml::setUseSmartBreaks( bool useSmartBreaks )
{
  mUseSmartBreaks = useSmartBreaks;
  recalculateFrameSizes();
  emit changed();
}

void QgsComposerHtml::setMaxBreakDistance( double maxBreakDistance )
{
  mMaxBreakDistance = maxBreakDistance;
  recalculateFrameSizes();
  emit changed();
}

bool QgsComposerHtml::writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames ) const
{
  QDomElement htmlElem = doc.createElement( "ComposerHtml" );
  htmlElem.setAttribute( "url", mUrl.toString() );
  htmlElem.setAttribute( "useSmartBreaks", mUseSmartBreaks ? "true" : "false" );
  htmlElem.setAttribute( "maxBreakDistance", QString::number( mMaxBreakDistance ) );

  bool state = _writeXML( htmlElem, doc, ignoreFrames );
  elem.appendChild( htmlElem );
  return state;
}

bool QgsComposerHtml::readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames )
{
  deleteFrames();

  //first create the frames
  if ( !_readXML( itemElem, doc, ignoreFrames ) )
  {
    return false;
  }

  mUseSmartBreaks = itemElem.attribute( "useSmartBreaks", "true" ) == "true" ? true : false;
  mMaxBreakDistance = itemElem.attribute( "maxBreakDistance", "10" ).toDouble();

  //finally load the set url
  QString urlString = itemElem.attribute( "url" );
  if ( !urlString.isEmpty() )
  {
    setUrl( QUrl( urlString ) );
  }
  return true;
}
