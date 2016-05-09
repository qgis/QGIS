/***************************************************************************

               ----------------------------------------------------
              date                 : 7.9.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspixmaplabel.h"


QgsPixmapLabel::QgsPixmapLabel( QWidget *parent )
    : QLabel( parent )
{
  this->setMinimumSize( 1, 1 );
}

void QgsPixmapLabel::setPixmap( const QPixmap & p )
{
  bool sizeChanged = ( p.size() != mPixmap.size() );
  mPixmap = p;

  if ( sizeChanged )
  {
    updateGeometry();
  }

  QLabel::setPixmap( mPixmap.scaled( this->size(),
                                     Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
}

int QgsPixmapLabel::heightForWidth( int width ) const
{
  if ( mPixmap.isNull() )
    return 0;

  return (( qreal )mPixmap.height()*width ) / mPixmap.width();
}

QSize QgsPixmapLabel::sizeHint() const
{
  if ( mPixmap.isNull() )
    return QSize( 0, 0 );

  int w = this->width();
  return QSize( w, heightForWidth( w ) );
}

void QgsPixmapLabel::resizeEvent( QResizeEvent * e )
{
  QLabel::resizeEvent( e );
  QLabel::setPixmap( mPixmap.scaled( this->size(),
                                     Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
}
