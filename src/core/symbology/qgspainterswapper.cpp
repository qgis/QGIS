/***************************************************************************
                             qgspainterswapper.cpp
                             ---------------------
    begin                : July 2019
    copyright            : (C) 2019 Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
