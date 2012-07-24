/***************************************************************************
                              qgscomposerhtml.cpp
    ------------------------------------------------------------
    begin                : Julli 2012
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
#include <QCoreApplication>
#include <QWebFrame>
#include <QWebPage>

QgsComposerHtml::QgsComposerHtml( QgsComposition* c ): QgsComposerMultiFrame( c ), mWebPage( 0 ), mLoaded( false )
{
  mWebPage = new QWebPage();
  QObject::connect( mWebPage, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );
}

QgsComposerHtml::QgsComposerHtml(): QgsComposerMultiFrame( 0 ), mWebPage( 0 ), mLoaded( false )
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
}

void QgsComposerHtml::frameLoaded( bool ok )
{
  mLoaded = true;
}

QSizeF QgsComposerHtml::totalSize() const
{
  return QSizeF(); //soon...
}

void QgsComposerHtml::render( QPainter* p, const QRectF& renderExtent )
{
  //soon...
}
