#include "qgscomposerruler.h"
#include "qgscomposition.h"
#include "qgis.h"
#include <QDragEnterEvent>
#include <QGraphicsLineItem>
#include <QPainter>
#include <cmath>

const int RULER_MIN_SIZE = 20;

QgsComposerRuler::QgsComposerRuler( QgsComposerRuler::Direction d ): QWidget( 0 ), mDirection( d ), mComposition( 0 ), mLineSnapItem( 0 )
{
  setMouseTracking( true );
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

void QgsComposerRuler::mouseMoveEvent( QMouseEvent* event )
{
  //qWarning( "QgsComposerRuler::mouseMoveEvent" );
  updateMarker( event->posF() );
  setSnapLinePosition( event->posF() );
}

void QgsComposerRuler::mouseReleaseEvent( QMouseEvent* event )
{
  Q_UNUSED( event );

  //remove snap line if coordinate under 0
  QPointF pos = mTransform.inverted().map( event->pos() );
  bool removeItem = false;
  if ( mDirection == Horizontal )
  {
    removeItem = pos.x() < 0 ? true : false;
  }
  else
  {
    removeItem = pos.y() < 0 ? true : false;
  }

  if ( removeItem )
  {
    mComposition->removeSnapLine( mLineSnapItem );
    mSnappedItems.clear();
  }
  mLineSnapItem = 0;
}

void QgsComposerRuler::mousePressEvent( QMouseEvent* event )
{
  double x = 0;
  double y = 0;
  if ( mDirection == Horizontal )
  {
    x = mTransform.inverted().map( event->pos() ).x();
  }
  else //vertical
  {
    y = mTransform.inverted().map( event->pos() ).y();
  }

  //horizontal ruler means vertical snap line
  QGraphicsLineItem* line = mComposition->nearestSnapLine( mDirection != Horizontal, x, y, 10.0, mSnappedItems );
  if ( !line )
  {
    //create new snap line
    mLineSnapItem = mComposition->addSnapLine();
  }
  else
  {
    mLineSnapItem = line;
  }
}

void QgsComposerRuler::setSnapLinePosition( const QPointF& pos )
{
  if ( !mLineSnapItem || !mComposition )
  {
    return;
  }

  QPointF transformedPt = mTransform.inverted().map( pos );
  if ( mDirection == Horizontal )
  {
    int numPages = mComposition->numPages();
    double lineHeight = numPages * mComposition->paperHeight();
    if ( numPages > 1 )
    {
      lineHeight += ( numPages - 1 ) * mComposition->spaceBetweenPages();
    }
    mLineSnapItem->setLine( QLineF( transformedPt.x(), 0, transformedPt.x(), lineHeight ) );
  }
  else //vertical
  {
    mLineSnapItem->setLine( QLineF( 0, transformedPt.y(), mComposition->paperWidth(), transformedPt.y() ) );
  }

  //move snapped items together with the snap line
  QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode > >::iterator itemIt = mSnappedItems.begin();
  for ( ; itemIt != mSnappedItems.end(); ++itemIt )
  {
    if ( mDirection == Horizontal )
    {
      if ( itemIt->second == QgsComposerItem::MiddleLeft )
      {
        itemIt->first->setItemPosition( transformedPt.x(), itemIt->first->transform().dy(), QgsComposerItem::UpperLeft );
      }
      else if ( itemIt->second == QgsComposerItem::Middle )
      {
        itemIt->first->setItemPosition( transformedPt.x(), itemIt->first->transform().dy(), QgsComposerItem::UpperMiddle );
      }
      else
      {
        itemIt->first->setItemPosition( transformedPt.x(), itemIt->first->transform().dy(), QgsComposerItem::UpperRight );
      }
    }
    else
    {
      if ( itemIt->second == QgsComposerItem::UpperMiddle )
      {
        itemIt->first->setItemPosition( itemIt->first->transform().dx(), transformedPt.y(), QgsComposerItem::UpperLeft );
      }
      else if ( itemIt->second == QgsComposerItem::Middle )
      {
        itemIt->first->setItemPosition( itemIt->first->transform().dx(), transformedPt.y(), QgsComposerItem::MiddleLeft );
      }
      else
      {
        itemIt->first->setItemPosition( itemIt->first->transform().dx(), transformedPt.y(), QgsComposerItem::LowerLeft );
      }
    }
  }
}
