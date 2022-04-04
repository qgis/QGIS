/***************************************************************************
                             qgslayoutruler.cpp
                             ------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayoutruler.h"
#include "qgslayout.h"
#include "qgis.h"
#include "qgslayoutview.h"
#include "qgslogger.h"
#include "qgslayoutpagecollection.h"
#include <QDragEnterEvent>
#include <QGraphicsLineItem>
#include <QPainter>
#include <QMenu>
#include <cmath>

const int RULER_FONT_SIZE = 8;
const unsigned int COUNT_VALID_MULTIPLES = 3;
const unsigned int COUNT_VALID_MAGNITUDES = 5;
const int QgsLayoutRuler::VALID_SCALE_MULTIPLES[] = {1, 2, 5};
const int QgsLayoutRuler::VALID_SCALE_MAGNITUDES[] = {1, 10, 100, 1000, 10000};

QgsLayoutRuler::QgsLayoutRuler( QWidget *parent, Qt::Orientation orientation )
  : QWidget( parent )
  , mOrientation( orientation )
{
  setMouseTracking( true );

  //calculate minimum size required for ruler text
  mRulerFont.setPointSize( RULER_FONT_SIZE );
  mRulerFontMetrics.reset( new QFontMetrics( mRulerFont ) );

  //calculate ruler sizes and marker separations

  //minimum gap required between major ticks is 3 digits * 250%, based on appearance
  mScaleMinPixelsWidth = mRulerFontMetrics->boundingRect( QStringLiteral( "000" ) ).width() * 2.5;
  //minimum ruler height is twice the font height in pixels
  mRulerMinSize = mRulerFontMetrics->height() * 1.5;

  mMinPixelsPerDivision = mRulerMinSize / 4;
  //each small division must be at least 2 pixels apart
  if ( mMinPixelsPerDivision < 2 )
    mMinPixelsPerDivision = 2;

  mPixelsBetweenLineAndText = mRulerMinSize / 10;
  mTextBaseline = mRulerMinSize / 1.667;
  mMinSpacingVerticalLabels = mRulerMinSize / 5;

  const double guideMarkerSize = mRulerFontMetrics->horizontalAdvance( '*' );
  mDragGuideTolerance = guideMarkerSize;
  switch ( mOrientation )
  {
    case Qt::Horizontal:
      mGuideMarker << QPoint( -guideMarkerSize / 2, mRulerMinSize - guideMarkerSize ) << QPoint( 0, mRulerMinSize ) <<
                   QPoint( guideMarkerSize / 2, mRulerMinSize - guideMarkerSize );
      break;

    case Qt::Vertical:
      mGuideMarker << QPoint( mRulerMinSize - guideMarkerSize, -guideMarkerSize / 2 ) << QPoint( mRulerMinSize, 0 ) <<
                   QPoint( mRulerMinSize - guideMarkerSize, guideMarkerSize / 2 );
      break;
  }
}

QSize QgsLayoutRuler::minimumSizeHint() const
{
  return QSize( mRulerMinSize, mRulerMinSize );
}

void QgsLayoutRuler::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event )
  if ( !mView || !mView->currentLayout() )
  {
    return;
  }

  QgsLayout *layout = mView->currentLayout();
  QPainter p( this );

  drawGuideMarkers( &p, layout );

  const QTransform t = mTransform.inverted();
  p.setFont( mRulerFont );
  // keep same default color, but lower opacity a tad
  QBrush brush = p.brush();
  QColor color = brush.color();
  color.setAlphaF( 0.7 );
  brush.setColor( color );
  p.setBrush( brush );
  QPen pen = p.pen();
  color = pen.color();
  color.setAlphaF( 0.7 );
  pen.setColor( color );
  p.setPen( pen );

  //find optimum scale for ruler (size of numbered divisions)
  int magnitude = 1;
  int multiple = 1;
  const int mmDisplay = optimumScale( mScaleMinPixelsWidth, magnitude, multiple );

  //find optimum number of small divisions
  const int numSmallDivisions = optimumNumberDivisions( mmDisplay, multiple );

  switch ( mOrientation )
  {
    case Qt::Horizontal:
    {
      if ( qgsDoubleNear( width(), 0 ) )
      {
        return;
      }

      //start x-coordinate
      const double startX = t.map( QPointF( 0, 0 ) ).x();
      const double endX = t.map( QPointF( width(), 0 ) ).x();

      //start marker position in mm
      double markerPos = ( std::floor( startX / mmDisplay ) + 1 ) * mmDisplay;

      //draw minor ticks marks which occur before first major tick
      drawSmallDivisions( &p, markerPos, numSmallDivisions, -mmDisplay );

      while ( markerPos <= endX )
      {
        const double pixelCoord = mTransform.map( QPointF( markerPos, 0 ) ).x();

        //draw large division and text
        p.drawLine( pixelCoord, 0, pixelCoord, mRulerMinSize );
        p.drawText( QPointF( pixelCoord + mPixelsBetweenLineAndText, mTextBaseline ), QLocale().toString( markerPos ) );

        //draw small divisions
        drawSmallDivisions( &p, markerPos, numSmallDivisions, mmDisplay, endX );

        markerPos += mmDisplay;
      }
      break;
    }
    case Qt::Vertical:
    {
      if ( qgsDoubleNear( height(), 0 ) )
      {
        return;
      }

      const double startY = t.map( QPointF( 0, 0 ) ).y(); //start position in mm (total including space between pages)
      const double endY = t.map( QPointF( 0, height() ) ).y(); //stop position in mm (total including space between pages)

      // work out start page
      int startPage = 0;
      int endPage = 0;
      double currentY = 0;
      double currentPageY = 0;
      for ( int page = 0; page < layout->pageCollection()->pageCount(); ++page )
      {
        if ( currentY < startY )
        {
          startPage = page;
          currentPageY = currentY;
        }
        endPage = page;

        currentY += layout->pageCollection()->page( startPage )->rect().height() + layout->pageCollection()->spaceBetweenPages();
        if ( currentY > endY )
          break;
      }

      if ( startY < 0 )
      {
        double beforePageCoord = -mmDisplay;
        const double firstPageY = mTransform.map( QPointF( 0, 0 ) ).y();

        //draw negative rulers which fall before first page
        while ( beforePageCoord > startY )
        {
          const double pixelCoord = mTransform.map( QPointF( 0, beforePageCoord ) ).y();
          p.drawLine( 0, pixelCoord, mRulerMinSize, pixelCoord );
          //calc size of label
          const QString label = QLocale().toString( beforePageCoord );
          const int labelSize = mRulerFontMetrics->boundingRect( label ).width();

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

      double nextPageStartPos = 0;
      int nextPageStartPixel = 0;

      for ( int i = startPage; i <= endPage; ++i )
      {
        double pageCoord = 0; //page coordinate in mm
        //total (composition) coordinate in mm, including space between pages

        double totalCoord = currentPageY;

        //position of next page
        if ( i < endPage )
        {
          //not the last page
          nextPageStartPos = currentPageY + layout->pageCollection()->page( i )->rect().height() + layout->pageCollection()->spaceBetweenPages();
          nextPageStartPixel = mTransform.map( QPointF( 0, nextPageStartPos ) ).y();
        }
        else
        {
          //is the last page
          nextPageStartPos = 0;
          nextPageStartPixel = 0;
        }

        while ( ( totalCoord < nextPageStartPos ) || ( ( nextPageStartPos == 0 ) && ( totalCoord <= endY ) ) )
        {
          const double pixelCoord = mTransform.map( QPointF( 0, totalCoord ) ).y();
          p.drawLine( 0, pixelCoord, mRulerMinSize, pixelCoord );
          //calc size of label
          const QString label = QLocale().toString( pageCoord );
          const int labelSize = mRulerFontMetrics->boundingRect( label ).width();

          //draw label only if it fits in before start of next page
          if ( ( pixelCoord + labelSize + 8 < nextPageStartPixel )
               || ( nextPageStartPixel == 0 ) )
          {
            drawRotatedText( &p, QPointF( mTextBaseline, pixelCoord + mMinSpacingVerticalLabels + labelSize ), label );
          }

          //draw small divisions
          drawSmallDivisions( &p, totalCoord, numSmallDivisions, mmDisplay, nextPageStartPos );

          pageCoord += mmDisplay;
          totalCoord += mmDisplay;
        }
        if ( i < endPage )
          currentPageY += layout->pageCollection()->page( i )->rect().height() + layout->pageCollection()->spaceBetweenPages();
      }
      break;
    }
  }

  //draw current marker pos
  drawMarkerPos( &p );
}

void QgsLayoutRuler::drawMarkerPos( QPainter *painter )
{
  //draw current marker pos in red
  painter->setPen( QColor( Qt::red ) );
  switch ( mOrientation )
  {
    case Qt::Horizontal:
    {
      painter->drawLine( mMarkerPos.x(), 0, mMarkerPos.x(), mRulerMinSize );
      break;
    }
    case Qt::Vertical:
    {
      painter->drawLine( 0, mMarkerPos.y(), mRulerMinSize, mMarkerPos.y() );
      break;
    }
  }
}

void QgsLayoutRuler::drawGuideMarkers( QPainter *p, QgsLayout *layout )
{
  const QList< QgsLayoutItemPage * > visiblePages = mView->visiblePages();
  const QList< QgsLayoutGuide * > guides = layout->guides().guides( mOrientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal );
  const QgsScopedQPainterState painterState( p );
  p->setRenderHint( QPainter::Antialiasing, true );
  p->setPen( Qt::NoPen );
  const auto constGuides = guides;
  for ( QgsLayoutGuide *guide : constGuides )
  {
    if ( visiblePages.contains( guide->page() ) )
    {
      if ( guide == mHoverGuide )
      {
        p->setBrush( QBrush( QColor( 255, 0, 0, 225 ) ) );
      }
      else
      {
        p->setBrush( QBrush( QColor( 255, 0, 0, 150 ) ) );
      }
      QPointF point;
      switch ( mOrientation )
      {
        case Qt::Horizontal:
          point = QPointF( guide->layoutPosition(), 0 );
          break;

        case Qt::Vertical:
          point = QPointF( 0, guide->layoutPosition() );
          break;
      }
      drawGuideAtPos( p, convertLayoutPointToLocal( point ) );
    }
  }
}

void QgsLayoutRuler::drawGuideAtPos( QPainter *painter, QPoint pos )
{
  switch ( mOrientation )
  {
    case Qt::Horizontal:
    {
      painter->translate( pos.x(), 0 );
      painter->drawPolygon( mGuideMarker );
      painter->translate( -pos.x(), 0 );
      break;
    }
    case Qt::Vertical:
    {
      painter->translate( 0, pos.y() );
      painter->drawPolygon( mGuideMarker );
      painter->translate( 0, -pos.y() );
      break;
    }
  }
}

void QgsLayoutRuler::createTemporaryGuideItem()
{
  if ( !mView->currentLayout() )
    return;

  delete mGuideItem;
  mGuideItem = new QGraphicsLineItem();

  mGuideItem->setZValue( QgsLayout::ZGuide );
  QPen linePen( Qt::DotLine );
  linePen.setColor( QColor( 255, 0, 0, 150 ) );
  linePen.setWidthF( 0 );
  mGuideItem->setPen( linePen );

  mView->currentLayout()->addItem( mGuideItem );
}

QPointF QgsLayoutRuler::convertLocalPointToLayout( QPoint localPoint ) const
{
  const QPoint viewPoint = mView->mapFromGlobal( mapToGlobal( localPoint ) );
  return  mView->mapToScene( viewPoint );
}

QPoint QgsLayoutRuler::convertLayoutPointToLocal( QPointF layoutPoint ) const
{
  const QPoint viewPoint = mView->mapFromScene( layoutPoint );
  return mapFromGlobal( mView->mapToGlobal( viewPoint ) );
}

QgsLayoutGuide *QgsLayoutRuler::guideAtPoint( QPoint localPoint ) const
{
  if ( !mView->currentLayout() )
    return nullptr;

  const QPointF layoutPoint = convertLocalPointToLayout( localPoint );
  const QList< QgsLayoutItemPage * > visiblePages = mView->visiblePages();
  const QList< QgsLayoutGuide * > guides = mView->currentLayout()->guides().guides( mOrientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal );
  QgsLayoutGuide *closestGuide = nullptr;
  double minDelta = std::numeric_limits<double>::max();
  const auto constGuides = guides;
  for ( QgsLayoutGuide *guide : constGuides )
  {
    if ( visiblePages.contains( guide->page() ) )
    {
      double currentDelta = 0;
      switch ( mOrientation )
      {
        case Qt::Horizontal:
          currentDelta = std::fabs( layoutPoint.x() - guide->layoutPosition() );
          break;

        case Qt::Vertical:
          currentDelta = std::fabs( layoutPoint.y() - guide->layoutPosition() );
          break;
      }
      if ( currentDelta < minDelta )
      {
        minDelta = currentDelta;
        closestGuide = guide;
      }
    }
  }

  if ( minDelta * mView->transform().m11() <= mDragGuideTolerance )
  {
    return closestGuide;
  }
  else
  {
    return nullptr;
  }
}

void QgsLayoutRuler::drawRotatedText( QPainter *painter, QPointF pos, const QString &text )
{
  const QgsScopedQPainterState painterState( painter );
  painter->translate( pos.x(), pos.y() );
  painter->rotate( 270 );
  painter->drawText( 0, 0, text );
}

void QgsLayoutRuler::drawSmallDivisions( QPainter *painter, double startPos, int numDivisions, double rulerScale, double maxPos )
{
  if ( numDivisions == 0 )
    return;

  //draw small divisions starting at startPos (in mm)
  double smallMarkerPos = startPos;
  const double smallDivisionSpacing = rulerScale / numDivisions;

  double pixelCoord = 0.0;

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
    switch ( mOrientation )
    {
      case Qt::Horizontal:
      {
        pixelCoord = mTransform.map( QPointF( smallMarkerPos, 0 ) ).x();
        break;
      }
      case Qt::Vertical:
      {
        pixelCoord = mTransform.map( QPointF( 0, smallMarkerPos ) ).y();
        break;
      }
    }

    //calculate height of small division line
    double lineSize;
    if ( ( numDivisions == 10 && i == 4 ) || ( numDivisions == 4 && i == 1 ) )
    {
      //if drawing the 5th line of 10 or drawing the 2nd line of 4, then draw it slightly longer
      lineSize = mRulerMinSize / 1.5;
    }
    else
    {
      lineSize = mRulerMinSize / 1.25;
    }

    //draw either horizontal or vertical line depending on ruler direction
    switch ( mOrientation )
    {
      case Qt::Horizontal:
      {
        painter->drawLine( pixelCoord, lineSize, pixelCoord, mRulerMinSize );
        break;
      }
      case Qt::Vertical:
      {
        painter->drawLine( lineSize, pixelCoord, mRulerMinSize, pixelCoord );
        break;
      }
    }
  }
}

int QgsLayoutRuler::optimumScale( double minPixelDiff, int &magnitude, int &multiple )
{
  //find optimal ruler display scale

  //loop through magnitudes and multiples to find optimum scale
  for ( unsigned int magnitudeCandidate = 0; magnitudeCandidate < COUNT_VALID_MAGNITUDES; ++magnitudeCandidate )
  {
    for ( unsigned int multipleCandidate = 0; multipleCandidate < COUNT_VALID_MULTIPLES; ++multipleCandidate )
    {
      const int candidateScale = VALID_SCALE_MULTIPLES[multipleCandidate] * VALID_SCALE_MAGNITUDES[magnitudeCandidate];
      //find pixel size for each step using this candidate scale
      const double pixelDiff = mTransform.map( QPointF( candidateScale, 0 ) ).x() - mTransform.map( QPointF( 0, 0 ) ).x();
      if ( pixelDiff > minPixelDiff )
      {
        //found the optimum major scale
        magnitude = VALID_SCALE_MAGNITUDES[magnitudeCandidate];
        multiple = VALID_SCALE_MULTIPLES[multipleCandidate];
        return candidateScale;
      }
    }
  }

  return 100000;
}

int QgsLayoutRuler::optimumNumberDivisions( double rulerScale, int scaleMultiple )
{
  //calculate size in pixels of each marked ruler unit
  const double largeDivisionSize = mTransform.map( QPointF( rulerScale, 0 ) ).x() - mTransform.map( QPointF( 0, 0 ) ).x();

  //now calculate optimum small tick scale, depending on marked ruler units
  QList<int> validSmallDivisions;
  switch ( scaleMultiple )
  {
    case 1:
      //numbers increase by 1 increment each time, e.g., 1, 2, 3 or 10, 20, 30
      //so we can draw either 10, 5 or 2 small ticks and have each fall on a nice value
      validSmallDivisions << 10 << 5 << 2;
      break;
    case 2:
      //numbers increase by 2 increments each time, e.g., 2, 4, 6 or 20, 40, 60
      //so we can draw either 10, 4 or 2 small ticks and have each fall on a nice value
      validSmallDivisions << 10 << 4 << 2;
      break;
    case 5:
      //numbers increase by 5 increments each time, e.g., 5, 10, 15 or 100, 500, 1000
      //so we can draw either 10 or 5 small ticks and have each fall on a nice value
      validSmallDivisions << 10 << 5;
      break;
  }

  //calculate the most number of small divisions we can draw without them being too close to each other
  QList<int>::iterator divisions_it;
  for ( divisions_it = validSmallDivisions.begin(); divisions_it != validSmallDivisions.end(); ++divisions_it )
  {
    //find pixel size for this small division
    const double candidateSize = largeDivisionSize / ( *divisions_it );
    //check if this separation is more then allowed min separation
    if ( candidateSize >= mMinPixelsPerDivision )
    {
      //found a good candidate, return it
      return ( *divisions_it );
    }
  }

  //unable to find a good candidate
  return 0;
}


void QgsLayoutRuler::setSceneTransform( const QTransform &transform )
{
  mTransform = transform;
  update();
}

void QgsLayoutRuler::setLayoutView( QgsLayoutView *view )
{
  mView = view;
  connect( mView, &QgsLayoutView::cursorPosChanged, this, &QgsLayoutRuler::setCursorPosition );
}

void QgsLayoutRuler::setContextMenu( QMenu *menu )
{
  mMenu = menu;
}

void QgsLayoutRuler::setCursorPosition( QPointF position )
{
  mMarkerPos = mView->mapFromScene( position );
  update();
}

void QgsLayoutRuler::mouseMoveEvent( QMouseEvent *event )
{
  mMarkerPos = event->pos();
  update();

  if ( !mView->currentLayout() )
    return;

  QPointF displayPos;
  if ( mCreatingGuide || mDraggingGuide )
  {
    // event -> layout coordinates
    displayPos = convertLocalPointToLayout( event->pos() );

    if ( mCreatingGuide )
    {
      QgsLayout *layout = mView->currentLayout();
      const int pageNo = layout->pageCollection()->pageNumberForPoint( displayPos );
      QgsLayoutItemPage *page = layout->pageCollection()->page( pageNo );
      if ( !page )
        return;

      QPen linePen = mGuideItem->pen();
      // if guide preview is outside a page draw it a lot fainter, to indicate it's invalid
      if ( !layout->pageCollection()->pageAtPoint( displayPos ) )
      {
        linePen.setColor( QColor( 255, 0, 0, 150 ) );
      }
      else
      {
        linePen.setColor( QColor( 255, 0, 0, 225 ) );
      }
      mGuideItem->setPen( linePen );
      switch ( mOrientation )
      {
        case Qt::Horizontal:
        {
          //mouse is creating a horizontal ruler, so don't show x coordinate
          mGuideItem->setLine( page->scenePos().x(), displayPos.y(), page->scenePos().x() + page->rect().width(), displayPos.y() );
          displayPos.setX( 0 );
          break;
        }
        case Qt::Vertical:
        {
          //mouse is creating a vertical ruler, so don't show a y coordinate
          mGuideItem->setLine( displayPos.x(), page->scenePos().y(), displayPos.x(), page->scenePos().y() + page->rect().height() );
          displayPos.setY( 0 );
          break;
        }
      }
    }
    else
    {
      // dragging guide
      switch ( mOrientation )
      {
        case Qt::Horizontal:
        {
          mView->currentLayout()->guides().setGuideLayoutPosition( mDraggingGuide, displayPos.x() );
          displayPos.setY( 0 );
          break;
        }
        case Qt::Vertical:
        {
          mView->currentLayout()->guides().setGuideLayoutPosition( mDraggingGuide, displayPos.y() );
          displayPos.setX( 0 );
          break;
        }
      }
    }
  }
  else
  {
    // is cursor over a guide marker?
    mHoverGuide = guideAtPoint( event->pos() );
    if ( mHoverGuide )
    {
      setCursor( mOrientation == Qt::Vertical ? Qt::SplitVCursor : Qt::SplitHCursor );
    }
    else
    {
      setCursor( Qt::ArrowCursor );
    }

    //update cursor position in status bar
    displayPos = mTransform.inverted().map( event->pos() );
    switch ( mOrientation )
    {
      case Qt::Horizontal:
      {
        //mouse is over a horizontal ruler, so don't show a y coordinate
        displayPos.setY( 0 );
        break;
      }
      case Qt::Vertical:
      {
        //mouse is over a vertical ruler, so don't show an x coordinate
        displayPos.setX( 0 );
        break;
      }
    }
  }
  emit cursorPosChanged( displayPos );
}

void QgsLayoutRuler::mousePressEvent( QMouseEvent *event )
{
  if ( !mView->currentLayout() )
    return;

  if ( event->button() == Qt::LeftButton )
  {
    mDraggingGuide = guideAtPoint( event->pos() );
    if ( !mDraggingGuide )
    {
      // if no guide at the point, then we're creating one
      if ( mView->currentLayout()->pageCollection()->pageCount() > 0 )
      {
        mCreatingGuide = true;
        createTemporaryGuideItem();
      }
    }
    else
    {
      mDraggingGuideOldPosition = mDraggingGuide->layoutPosition();
    }
    switch ( mOrientation )
    {
      case Qt::Horizontal:
      {
        QApplication::setOverrideCursor( mDraggingGuide ? Qt::SplitHCursor : Qt::SplitVCursor );
        break;
      }
      case Qt::Vertical:
        QApplication::setOverrideCursor( mDraggingGuide ? Qt::SplitVCursor : Qt::SplitHCursor );
        break;
    }
  }
}

void QgsLayoutRuler::mouseReleaseEvent( QMouseEvent *event )
{
  if ( !mView->currentLayout() )
    return;

  if ( event->button() == Qt::LeftButton )
  {
    if ( mDraggingGuide )
    {
      QApplication::restoreOverrideCursor();

      const QPointF layoutPoint = convertLocalPointToLayout( event->pos() );

      // delete guide if it ends outside of page
      QgsLayoutItemPage *page = mDraggingGuide->page();
      bool deleteGuide = false;
      switch ( mDraggingGuide->orientation() )
      {
        case Qt::Horizontal:
          if ( layoutPoint.y() < page->scenePos().y() || layoutPoint.y() > page->scenePos().y() + page->rect().height() )
            deleteGuide = true;
          break;

        case Qt::Vertical:
          if ( layoutPoint.x() < page->scenePos().x() || layoutPoint.x() > page->scenePos().x() + page->rect().width() )
            deleteGuide = true;
          break;
      }

      if ( deleteGuide )
      {
        mView->currentLayout()->guides().removeGuide( mDraggingGuide );
      }
      mDraggingGuide = nullptr;
    }
    else
    {
      mCreatingGuide = false;
      QApplication::restoreOverrideCursor();
      delete mGuideItem;
      mGuideItem = nullptr;

      // check that cursor left the ruler
      switch ( mOrientation )
      {
        case Qt::Horizontal:
        {
          if ( event->pos().y() <= height() )
            return;
          break;
        }
        case Qt::Vertical:
        {
          if ( event->pos().x() <= width() )
            return;
          break;
        }
      }

      QgsLayout *layout = mView->currentLayout();

      // create guide
      const QPointF scenePos = convertLocalPointToLayout( event->pos() );
      QgsLayoutItemPage *page = layout->pageCollection()->pageAtPoint( scenePos );
      if ( !page )
        return; // dragged outside of a page

      std::unique_ptr< QgsLayoutGuide > guide;
      switch ( mOrientation )
      {
        case Qt::Horizontal:
        {
          //mouse is creating a horizontal guide
          const double posOnPage = layout->pageCollection()->positionOnPage( scenePos ).y();
          guide.reset( new QgsLayoutGuide( Qt::Horizontal, QgsLayoutMeasurement( posOnPage, layout->units() ), page ) );
          break;
        }
        case Qt::Vertical:
        {
          //mouse is creating a vertical guide
          guide.reset( new QgsLayoutGuide( Qt::Vertical, QgsLayoutMeasurement( scenePos.x(), layout->units() ), page ) );
          break;
        }
      }
      mView->currentLayout()->guides().addGuide( guide.release() );
    }
  }
  else if ( event->button() == Qt::RightButton )
  {
    if ( mCreatingGuide || mDraggingGuide )
    {
      QApplication::restoreOverrideCursor();
      delete mGuideItem;
      mGuideItem = nullptr;
      mCreatingGuide = false;
      if ( mDraggingGuide )
      {
        mDraggingGuide->setLayoutPosition( mDraggingGuideOldPosition );
      }
      mDraggingGuide = nullptr;
    }
    if ( mMenu )
      mMenu->popup( event->globalPos() );
  }
}
