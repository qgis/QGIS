#include "qgscomposerruler.h"
#include "qgis.h"
#include <QPainter>
#include <cmath>

const int RULER_MIN_SIZE = 20;

QgsComposerRuler::QgsComposerRuler( QgsComposerRuler::Direction d ): QWidget( 0 ), mDirection( d )
{
}

QgsComposerRuler::~QgsComposerRuler()
{
}

QSize QgsComposerRuler::minimumSizeHint() const
{
  return QSize( RULER_MIN_SIZE, RULER_MIN_SIZE );
}

void QgsComposerRuler::paintEvent( QPaintEvent* event )
{
  Q_UNUSED( event );
  QPainter p( this );

  QTransform t = mTransform.inverted();

  if ( mDirection == Horizontal )
  {
    if ( doubleNear( width(), 0 ) )
    {
      return;
    }

    //start x-coordinate
    double startX = t.map( QPointF( 0, 0 ) ).x();
    double endX = t.map( QPointF( width(), 0 ) ).x();

    double markerPos = ( floor( startX / 10.0 ) + 1 ) * 10.0; //marker position in mm
    while ( markerPos <= endX )
    {
      if ( markerPos >= 0 && markerPos <= 297 ) //todo: need to know paper size
      {
        double pixelCoord = mTransform.map( QPointF( markerPos, 0 ) ).x();
        p.drawLine( pixelCoord, 0, pixelCoord, RULER_MIN_SIZE );
        p.drawText( QPointF( pixelCoord + 2, RULER_MIN_SIZE / 2.0 ), QString::number(( int )( markerPos ) ) );
      }
      markerPos += 10.0;
    }

    qWarning( QString::number( startX ).toLocal8Bit().data() );
    qWarning( QString::number( endX ).toLocal8Bit().data() );
  }
  else //vertical
  {
    if ( doubleNear( height(), 0 ) )
    {
      return;
    }

    double startY = t.map( QPointF( 0, 0 ) ).y();
    double endY = t.map( QPointF( 0, height() ) ).y();

    double markerPos = ( floor( startY / 10.0 ) + 1 ) * 10.0; //marker position in mm
    while ( markerPos <= endY )
    {
      if ( markerPos >= 0 && markerPos <= 210 )
      {
        double pixelCoord = mTransform.map( QPointF( 0, markerPos ) ).y();
        p.drawLine( 0, pixelCoord, RULER_MIN_SIZE, pixelCoord );
        p.drawText( QPointF( 0, pixelCoord - 2.0 ), QString::number( markerPos ) );
      }
      markerPos += 10.0;
    }
  }
}

void QgsComposerRuler::setSceneTransform( const QTransform& transform )
{
  QString debug = QString::number( transform.dx() ) + "," + QString::number( transform.dy() ) + ","
                  + QString::number( transform.m11() ) + "," + QString::number( transform.m22() );
  qWarning( debug.toLocal8Bit().data() );
  mTransform = transform;
  update();
}
