/***************************************************************************
                             qgsemfrenderer.cpp
                             ------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsemfrenderer.h"
#include "QEmfRenderer.h"


QgsEmfRenderer::QgsEmfRenderer( const QString &filename )
  : mFilename( filename )
{

}

QgsEmfRenderer::QgsEmfRenderer( const QByteArray &contents )
  : mContents( contents )
{

}

bool QgsEmfRenderer::render( QPainter *painter, QSizeF size, bool keepAspectRatio )
{
  if ( !painter )
    return false;

  QSize s( size.width(), size.height() );
  QEmf::QEmfRenderer renderer( *painter, s, keepAspectRatio );

  if ( !mFilename.isEmpty() )
    return renderer.load( mFilename );
  else
    return renderer.load( mContents );
}
