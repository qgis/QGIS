#include "qgscomposerruler.h"
#include "qgscomposition.h"
#include "qgis.h"
#include <QPainter>
#include <cmath>

const int RULER_MIN_SIZE = 20;

QgsComposerRuler::QgsComposerRuler( QgsComposerRuler::Direction d ): QWidget( 0 ), mDirection( d ), mComposition( 0 )
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
  if ( !mComposition )
  {
    return;
  }

  QPainter p( this );

  QTransform t = mTransform.inverted();

  //find optimal ruler display scale (steps of 1, 10 or 50)
  double pixelDiff1 = mTransform.map( QPointF( 1, 0 ) ).x() - mTransform.map( QPointF( 0, 0 ) ).x();
  double pixelDiff10 = mTransform.map( QPointF( 10, 0 ) ).x() - mTransform.map( QPointF( 0, 0 ) ).x();
  //double pixelDiff50 = mTransform.map( QPointF( 50, 0 ) ).x() - mTransform.map( QPointF( 5, 0 ) ).x();

  double mmDisplay = 50.0;
  if ( pixelDiff1 > 25 )
  {
    mmDisplay = 1.0;
  }
  else if ( pixelDiff10 > 25 )
  {
    mmDisplay = 10.0;
  }

  if ( mDirection == Horizontal )
  {
    if ( doubleNear( width(), 0 ) )
    {
      return;
    }

    //start x-coordinate
    double startX = t.map( QPointF( 0, 0 ) ).x();
    double endX = t.map( QPointF( width(), 0 ) ).x();

    double markerPos = ( floor( startX / mmDisplay ) + 1 ) * mmDisplay; //marker position in mm
    while ( markerPos <= endX )
    {
      if ( markerPos >= 0 && markerPos <= mComposition->paperWidth() ) //todo: need to know paper size
      {
        double pixelCoord = mTransform.map( QPointF( markerPos, 0 ) ).x();
        p.drawLine( pixelCoord, 0, pixelCoord, RULER_MIN_SIZE );
        p.drawText( QPointF( pixelCoord + 2, RULER_MIN_SIZE / 2.0 ), QString::number(( int )( markerPos ) ) );
      }
      markerPos += mmDisplay;
    }

    p.setPen( QColor( Qt::red ) );
    p.drawLine( mMarkerPos.x(), 0, mMarkerPos.x(), RULER_MIN_SIZE );
  }
  else //vertical
  {
    if ( doubleNear( height(), 0 ) )
    {
      return;
    }

    double startY = t.map( QPointF( 0, 0 ) ).y(); //start position in mm (total including space between pages)
    double endY = t.map( QPointF( 0, height() ) ).y(); //stop position in mm (total including space between pages)
    int startPage = ( int )( startY / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() ) );
    if ( startPage < 0 )
    {
      startPage = 0;
    }
    int endPage = ( int )( endY / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() ) );
    if ( endPage > ( mComposition->numPages() - 1 ) )
    {
      endPage = mComposition->numPages() - 1;
    }

    for ( int i = startPage; i <= endPage; ++i )
    {
      double pageCoord = 0; //page coordinate in mm
      //total (composition) coordinate in mm, including space between pages
      double totalCoord = i * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
      while ( pageCoord < mComposition->paperHeight() )
      {
        if ( totalCoord > endY )
        {
          break;
        }
        double pixelCoord = mTransform.map( QPointF( 0, totalCoord ) ).y();
        p.drawLine( 0, pixelCoord, RULER_MIN_SIZE, pixelCoord );
        p.drawText( QPointF( 0, pixelCoord - 2.0 ), QString::number( pageCoord ) );
        pageCoord += mmDisplay;
        totalCoord += mmDisplay;
      }
    }

#if 0
    double markerPos = ( floor( startY / mmDisplay ) + 1 ) * mmDisplay; //marker position in mm
    while ( markerPos <= endY )
    {
      int page = ( int )( markerPos / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() ) );
      double pageCoord = markerPos - page * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
      if ( page >= mComposition->numPages() )
      {
        break;
      }

      if ( pageCoord < 0 || pageCoord > mComposition->paperHeight() ) //marker is in a page gap
      {
        markerPos += mmDisplay;
        continue;
      }
      double pixelCoord = mTransform.map( QPointF( 0, markerPos ) ).y();
      p.drawLine( 0, pixelCoord, RULER_MIN_SIZE, pixelCoord );
      p.drawText( QPointF( 0, pixelCoord - 2.0 ), QString::number( pageCoord ) );
      markerPos += mmDisplay;
    }
#endif //0

    p.setPen( QColor( Qt::red ) );
    p.drawLine( 0, mMarkerPos.y(), RULER_MIN_SIZE, mMarkerPos.y() );
  }
}

void QgsComposerRuler::setSceneTransform( const QTransform& transform )
{
  QString debug = QString::number( transform.dx() ) + "," + QString::number( transform.dy() ) + ","
                  + QString::number( transform.m11() ) + "," + QString::number( transform.m22() );
  mTransform = transform;
  update();
}
