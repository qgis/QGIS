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
#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QWebFrame>
#include <QWebPage>

QgsComposerHtml::QgsComposerHtml( QgsComposition* c, qreal x, qreal y, qreal width, qreal height, bool addCommands ): QgsComposerMultiFrame( c ), mWebPage( 0 ),
    mLoaded( false ), mHtmlUnitsToMM( 1.0 )
{
  mHtmlUnitsToMM = htmlUnitsToMM();
  mWebPage = new QWebPage();
  QObject::connect( mWebPage, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );

  if ( mComposition && width > 0 && height > 0 )
  {
    if ( addCommands )
    {
      QgsAddRemoveMultiFrameCommand* c = new QgsAddRemoveMultiFrameCommand( QgsAddRemoveMultiFrameCommand::Added, this, mComposition, tr( "HTML added" ), 0 );
      mComposition->undoStack()->push( c );
    }
    QgsComposerFrame* frame = new QgsComposerFrame( c, this, x, y, width, height );
    addFrame( frame, addCommands );
    QObject::connect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( handleFrameRemoval( QgsComposerItem* ) ) );
    recalculateFrameSizes();
  }
}

QgsComposerHtml::QgsComposerHtml(): QgsComposerMultiFrame( 0 ), mWebPage( 0 ), mLoaded( false ), mHtmlUnitsToMM( 1.0 )
{
}

QgsComposerHtml::~QgsComposerHtml()
{
  delete mWebPage;
}

void QgsComposerHtml::setUrl( const QUrl& url )
{
  if ( !mWebPage )
  {
    return;
  }

  mUrl = url;
  mWebPage->mainFrame()->load( mUrl );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }
  QSize contentsSize = mWebPage->mainFrame()->contentsSize();
  mWebPage->setViewportSize( contentsSize );

  mSize.setWidth( contentsSize.width() / mHtmlUnitsToMM );
  mSize.setHeight( contentsSize.height() / mHtmlUnitsToMM );
  recalculateFrameSizes();
  emit changed();
}

void QgsComposerHtml::frameLoaded( bool ok )
{
  Q_UNUSED( ok );
  mLoaded = true;
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

  QImage img( 1, 1, QImage::Format_ARGB32_Premultiplied );
  double pixelPerMM = mComposition->printResolution() / 25.4;
  return ( pixelPerMM / ( img.dotsPerMeterX() / 1000.0 ) );
}

void QgsComposerHtml::addFrame( QgsComposerFrame* frame, bool addCommand )
{
  mFrameItems.push_back( frame );
  QObject::connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );
  if ( mComposition )
  {
    mComposition->addComposerHtmlFrame( this, frame );
    if ( addCommand )
    {
      mComposition->pushAddRemoveCommand( frame, tr( "Add Html frame" ) );
    }
  }
}

bool QgsComposerHtml::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement htmlElem = doc.createElement( "ComposerHtml" );
  htmlElem.setAttribute( "url", mUrl.toString() );
  bool state = _writeXML( htmlElem, doc );
  elem.appendChild( htmlElem );
  return state;
}

bool QgsComposerHtml::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  QString urlString = itemElem.attribute( "url" );
  if ( !urlString.isEmpty() )
  {
    setUrl( QUrl( urlString ) );
  }
  return _readXML( itemElem, doc );
}
