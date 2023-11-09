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
}

void QgsPixmapLabel::setPixmap( const QPixmap &p )
{
  const bool sizeChanged = ( p.size() != mPixmap.size() );
  mPixmap = p;

  if ( mPixmap.isNull() )
    this->setMinimumHeight( 0 );
  else
    this->setMinimumHeight( PIXMAP_MINIMUM_HEIGHT );

  if ( sizeChanged )
  {
    updateGeometry();
  }

  QLabel::setPixmap( mPixmap.scaled( this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
}

int QgsPixmapLabel::heightForWidth( int width ) const
{
  if ( mPixmap.isNull() )
    return 0;

  return ( ( qreal )mPixmap.height() * width ) / mPixmap.width();
}

QSize QgsPixmapLabel::sizeHint() const
{
  if ( mPixmap.isNull() )
    return QSize( 0, 0 );

  const int w = this->width();
  return QSize( w, heightForWidth( w ) );
}

void QgsPixmapLabel::resizeEvent( QResizeEvent *e )
{
  QLabel::resizeEvent( e );
  if ( !mPixmap.isNull() )
  {
    // Avoid infinite resize loop by setting a pixmap that'll always have a width and height less or equal to the label size
    QLabel::setPixmap( mPixmap.scaled( this->size() -= QSize( 1, 1 ), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
  }
}

void QgsPixmapLabel::clear()
{
  mPixmap = QPixmap();
  QLabel::clear();
  this->setMinimumHeight( 0 );
}
