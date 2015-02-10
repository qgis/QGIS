#include "qgscomposerruler.h"
#include "qgscomposition.h"
#include "qgis.h"
#include <QDragEnterEvent>
#include <QGraphicsLineItem>
#include <QPainter>
#include <cmath>

const int RULER_FONT_SIZE = 8;
const unsigned int COUNT_VALID_MULTIPLES = 3;
const unsigned int COUNT_VALID_MAGNITUDES = 5;
const int QgsComposerRuler::validScaleMultiples[] = {1, 2, 5};
const int QgsComposerRuler::validScaleMagnitudes[] = {1, 10, 100, 1000, 10000};

QgsComposerRuler::QgsComposerRuler( QgsComposerRuler::Direction d ) : QWidget( 0 ),
    mDirection( d ),
    mComposition( 0 ),
    mLineSnapItem( 0 ),
    mScaleMinPixelsWidth( 0 )
{
  setMouseTracking( true );

  //calculate minimum size required for ruler text
  mRulerFont = new QFont();
  mRulerFont->setPointSize( RULER_FONT_SIZE );
  mRulerFontMetrics = new QFontMetrics( *mRulerFont );

  //calculate ruler sizes and marker seperations

  //minimum gap required between major ticks is 3 digits * 250%, based on appearance
  mScaleMinPixelsWidth = mRulerFontMetrics->width( "000" ) * 2.5;
  //minimum ruler height is twice the font height in pixels
  mRulerMinSize = mRulerFontMetrics->height() * 1.5;

  mMinPixelsPerDivision = mRulerMinSize / 4;
  //each small division must be at least 2 pixels apart
  if ( mMinPixelsPerDivision < 2 )
    mMinPixelsPerDivision = 2;

  mPixelsBetweenLineAndText = mRulerMinSize / 10;
  mTextBaseline = mRulerMinSize / 1.667;
  mMinSpacingVerticalLabels = mRulerMinSize / 5;
}

QgsComposerRuler::~QgsComposerRuler()
{
  delete mRulerFontMetrics;
  delete mRulerFont;
}

QSize QgsComposerRuler::minimumSizeHint() const
{
  return QSize( mRulerMinSize, mRulerMinSize );
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

  p.setFont( *mRulerFont );

  //find optimum scale for ruler (size of numbered divisions)
  int magnitude = 1;
  int multiple = 1;
  int mmDisplay;
  mmDisplay = optimumScale( mScaleMinPixelsWidth, magnitude, multiple );

  //find optimum number of small divisions
  int numSmallDivisions = optimumNumberDivisions( mmDisplay, multiple );

  if ( mDirection == Horizontal )
  {
    if ( qgsDoubleNear( width(), 0 ) )
    {
      return;
    }

    //start x-coordinate
    double startX = t.map( QPointF( 0, 0 ) ).x();
    double endX = t.map( QPointF( width(), 0 ) ).x();

    //start marker position in mm
    double markerPos = ( floor( startX / mmDisplay ) + 1 ) * mmDisplay;

    //draw minor ticks marks which occur before first major tick
    drawSmallDivisions( &p, markerPos, numSmallDivisions, -mmDisplay );

    while ( markerPos <= endX )
    {
      double pixelCoord = mTransform.map( QPointF( markerPos, 0 ) ).x();

      //draw large division and text
      p.drawLine( pixelCoord, 0, pixelCoord, mRulerMinSize );
      p.drawText( QPointF( pixelCoord + mPixelsBetweenLineAndText, mTextBaseline ), QString::number( markerPos ) );

      //draw small divisions
      drawSmallDivisions( &p, markerPos, numSmallDivisions, mmDisplay, endX );

      markerPos += mmDisplay;
    }
  }
  else //vertical
  {
    if ( qgsDoubleNear( height(), 0 ) )
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

    if ( startY < 0 )
    {
      double beforePageCoord = -mmDisplay;
      double firstPageY = mTransform.map( QPointF( 0, 0 ) ).y();

      //draw negative rulers which fall before first page
      while ( beforePageCoord > startY )
      {
        double pixelCoord = mTransform.map( QPointF( 0, beforePageCoord ) ).y();
        p.drawLine( 0, pixelCoord, mRulerMinSize, pixelCoord );
        //calc size of label
        QString label = QString::number( beforePageCoord );
        int labelSize = mRulerFontMetrics->width( label );

        //draw label only if it fits in before start of next page
        if ( pixelCoord + labelSize + 8 < firstPageY )
        {
          drawRotatedText( &p, QPointF( mTextBaseline, pixelCoord + mMinSpacingVerticalLabels + labelSize ), label );
        }

        //draw small divisions
        drawSmallDivisions( &p, beforePageCoord, numSmallDivisions, mmDisplay );

        beforePageCoord -= mmDisplay;
      }

      //draw minor ticks marks which occur before first major tick
      drawSmallDivisions( &p, beforePageCoord + mmDisplay, numSmallDivisions, -mmDisplay, startY );
    }

    int endPage = ( int )( endY / ( mComposition->paperHeight() + mComposition->spaceBetweenPages() ) );
    if ( endPage > ( mComposition->numPages() - 1 ) )
    {
      endPage = mComposition->numPages() - 1;
    }

    double nextPageStartPos = 0;
    int nextPageStartPixel = 0;

    for ( int i = startPage; i <= endPage; ++i )
    {
      double pageCoord = 0; //page coordinate in mm
      //total (composition) coordinate in mm, including space between pages
      double totalCoord = i * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );

      //position of next page
      if ( i < endPage )
      {
        //not the last page
        nextPageStartPos = ( i + 1 ) * ( mComposition->paperHeight() + mComposition->spaceBetweenPages() );
        nextPageStartPixel = mTransform.map( QPointF( 0, nextPageStartPos ) ).y();
      }
      else
      {
        //is the last page
        nextPageStartPos = 0;
        nextPageStartPixel = 0;
      }

      while (( totalCoord < nextPageStartPos ) || (( nextPageStartPos == 0 ) && ( totalCoord <= endY ) ) )
      {
        double pixelCoord = mTransform.map( QPointF( 0, totalCoord ) ).y();
        p.drawLine( 0, pixelCoord, mRulerMinSize, pixelCoord );
        //calc size of label
        QString label = QString::number( pageCoord );
        int labelSize = mRulerFontMetrics->width( label );

        //draw label only if it fits in before start of next page
        if (( pixelCoord + labelSize + 8 < nextPageStartPixel )
            || ( nextPageStartPixel == 0 ) )
        {
          drawRotatedText( &p, QPointF( mTextBaseline, pixelCoord + mMinSpacingVerticalLabels + labelSize ), label );
        }

        //draw small divisions
        drawSmallDivisions( &p, totalCoord, numSmallDivisions, mmDisplay, nextPageStartPos );

        pageCoord += mmDisplay;
        totalCoord += mmDisplay;
      }
    }
  }

  //draw current marker pos
  drawMarkerPos( &p );
}

void QgsComposerRuler::drawMarkerPos( QPainter *painter )
{
  //draw current marker pos in red
  painter->setPen( QColor( Qt::red ) );
  if ( mDirection == Horizontal )
  {
    painter->drawLine( mMarkerPos.x(), 0, mMarkerPos.x(), mRulerMinSize );
  }
  else
  {
    painter->drawLine( 0, mMarkerPos.y(), mRulerMinSize, mMarkerPos.y() );
  }
}

void QgsComposerRuler::drawRotatedText( QPainter *painter, QPointF pos, const QString &text )
{
  painter->save();
  painter->translate( pos.x(), pos.y() );
  painter->rotate( 270 );
  painter->drawText( 0, 0, text );
  painter->restore();
}

void QgsComposerRuler::drawSmallDivisions( QPainter *painter, double startPos, int numDivisions, double rulerScale, double maxPos )
{
  if ( numDivisions == 0 )
    return;

  //draw small divisions starting at startPos (in mm)
  double smallMarkerPos = startPos;
  double smallDivisionSpacing = rulerScale / numDivisions;

  double pixelCoord;

  //draw numDivisions small divisions
  for ( int i = 0; i < numDivisions; ++i )
  {
    smallMarkerPos += smallDivisionSpacing;

    if ( maxPos > 0 && smallMarkerPos > maxPos )
    {
      //stop drawing current division position is past maxPos
      return;
    }

    //calculate pixelCoordinate of the current division
    if ( mDirection == Horizontal )
    {
      pixelCoord = mTransform.map( QPointF( smallMarkerPos, 0 ) ).x();
    }
    else
    {
      pixelCoord = mTransform.map( QPointF( 0, smallMarkerPos ) ).y();
    }

    //calculate height of small division line
    double lineSize;
    if (( numDivisions == 10 && i == 4 ) || ( numDivisions == 4 && i == 1 ) )
    {
      //if drawing the 5th line of 10 or drawing the 2nd line of 4, then draw it slightly longer
      lineSize = mRulerMinSize / 1.5;
    }
    else
    {
      lineSize = mRulerMinSize / 1.25;
    }

    //draw either horizontal or vertical line depending on ruler direction
    if ( mDirection == Horizontal )
    {
      painter->drawLine( pixelCoord, lineSize, pixelCoord, mRulerMinSize );
    }
    else
    {
      painter->drawLine( lineSize, pixelCoord, mRulerMinSize, pixelCoord );
    }
  }
}

int QgsComposerRuler::optimumScale( double minPixelDiff, int &magnitude, int &multiple )
{
  //find optimal ruler display scale

  //loop through magnitudes and multiples to find optimum scale
  for ( unsigned int magnitudeCandidate = 0; magnitudeCandidate < COUNT_VALID_MAGNITUDES; ++magnitudeCandidate )
  {
    for ( unsigned int multipleCandidate = 0; multipleCandidate < COUNT_VALID_MULTIPLES; ++multipleCandidate )
    {
      int candidateScale = validScaleMultiples[multipleCandidate] * validScaleMagnitudes[magnitudeCandidate];
      //find pixel size for each step using this candidate scale
      double pixelDiff = mTransform.map( QPointF( candidateScale, 0 ) ).x() - mTransform.map( QPointF( 0, 0 ) ).x();
      if ( pixelDiff > minPixelDiff )
      {
        //found the optimum major scale
        magnitude = validScaleMagnitudes[magnitudeCandidate];
        multiple = validScaleMultiples[multipleCandidate];
        return candidateScale;
      }
    }
  }

  return 100000;
}

int QgsComposerRuler::optimumNumberDivisions( double rulerScale, int scaleMultiple )
{
  //calculate size in pixels of each marked ruler unit
  double largeDivisionSize = mTransform.map( QPointF( rulerScale, 0 ) ).x() - mTransform.map( QPointF( 0, 0 ) ).x();

  //now calculate optimum small tick scale, depending on marked ruler units
  QList<int> validSmallDivisions;
  switch ( scaleMultiple )
  {
    case 1:
      //numbers increase by 1 increment each time, eg 1, 2, 3 or 10, 20, 30
      //so we can draw either 10, 5 or 2 small ticks and have each fall on a nice value
      validSmallDivisions << 10 << 5 << 2;
      break;
    case 2:
      //numbers increase by 2 increments each time, eg 2, 4, 6 or 20, 40, 60
      //so we can draw either 10, 4 or 2 small ticks and have each fall on a nice value
      validSmallDivisions << 10 << 4 << 2;
      break;
    case 5:
      //numbers increase by 5 increments each time, eg 5, 10, 15 or 100, 500, 1000
      //so we can draw either 10 or 5 small ticks and have each fall on a nice value
      validSmallDivisions << 10 << 5;
      break;
  }

  //calculate the most number of small divisions we can draw without them being too close to each other
  QList<int>::iterator divisions_it;
  for ( divisions_it = validSmallDivisions.begin(); divisions_it != validSmallDivisions.end(); ++divisions_it )
  {
    //find pixel size for this small division
    double candidateSize = largeDivisionSize / ( *divisions_it );
    //check if this seperation is more then allowed min seperation
    if ( candidateSize >= mMinPixelsPerDivision )
    {
      //found a good candidate, return it
      return ( *divisions_it );
    }
  }

  //unable to find a good candidate
  return 0;
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

  //update cursor position in status bar
  QPointF displayPos = mTransform.inverted().map( event->posF() );
  if ( mDirection == Horizontal )
  {
    //mouse is over a horizontal ruler, so don't show a y coordinate
    displayPos.setY( 0 );
  }
  else
  {
    //mouse is over a vertical ruler, so don't show an x coordinate
    displayPos.setX( 0 );
  }
  emit cursorPosChanged( displayPos );
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
        itemIt->first->setItemPosition( transformedPt.x(), itemIt->first->pos().y(), QgsComposerItem::UpperLeft );
      }
      else if ( itemIt->second == QgsComposerItem::Middle )
      {
        itemIt->first->setItemPosition( transformedPt.x(), itemIt->first->pos().y(), QgsComposerItem::UpperMiddle );
      }
      else
      {
        itemIt->first->setItemPosition( transformedPt.x(), itemIt->first->pos().y(), QgsComposerItem::UpperRight );
      }
    }
    else
    {
      if ( itemIt->second == QgsComposerItem::UpperMiddle )
      {
        itemIt->first->setItemPosition( itemIt->first->pos().x(), transformedPt.y(), QgsComposerItem::UpperLeft );
      }
      else if ( itemIt->second == QgsComposerItem::Middle )
      {
        itemIt->first->setItemPosition( itemIt->first->pos().x(), transformedPt.y(), QgsComposerItem::MiddleLeft );
      }
      else
      {
        itemIt->first->setItemPosition( itemIt->first->pos().x(), transformedPt.y(), QgsComposerItem::LowerLeft );
      }
    }
  }
}
