#include "qgscomposerruler.h"
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
    //start x-coordinate
    double startX = t.map( QPointF( 0, 0 ) ).x();//-mTransform.dx() / mTransform.m11();
    double endX = t.map( QPointF( width(), 0 ) ).x();//( -mTransform.dx() + width() ) / mTransform.m11();

    double markerPos = ( floor( startX / 10.0 ) + 1 ) * 10.0 - RULER_MIN_SIZE; //marker position in mm
    while ( markerPos <= endX )
    {
      if ( markerPos >= 0 && markerPos <= 296 ) //todo: need to know paper size
      {
        double pixelCoord = mTransform.map( QPointF( markerPos, 0 ) ).x();
        p.drawLine( pixelCoord, 0, pixelCoord, RULER_MIN_SIZE );
      }
      markerPos += 10.0;
    }

    qWarning( QString::number( startX ).toLocal8Bit().data() );
    qWarning( QString::number( endX ).toLocal8Bit().data() );
  }
  else //vertical
  {
    //p.fillRect( rect(), QColor( 0, 0, 255 ) );
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
