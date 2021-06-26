/***************************************************************************
                             qgspainterswapper.cpp
                             ---------------------
    begin                : July 2019
    copyright            : (C) 2019 Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspainterswapper.h"
#include "qgsrendercontext.h"
#include <QPainter>

QgsPainterSwapper::QgsPainterSwapper( QgsRenderContext &renderContext, QPainter *newPainter )
  : mRenderContext( renderContext )
  , mOriginalPainter( renderContext.painter() )
{
  mRenderContext.setPainter( newPainter );
}

QgsPainterSwapper::~QgsPainterSwapper()
{
  mRenderContext.setPainter( mOriginalPainter );
}
