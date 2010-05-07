#include "qgsresidualplotitem.h"
#include "qgsgeorefdatapoint.h"
#include <QPainter>
#include <cfloat>
#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif

QgsResidualPlotItem::QgsResidualPlotItem( QgsComposition* c ): QgsComposerItem( c ), mConvertScaleToMapUnits( false ), mPixelToMapUnits( 1.0 )
{

}

QgsResidualPlotItem::~QgsResidualPlotItem()
{

}

void QgsResidualPlotItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( mGCPList.size() < 1 || !painter )
  {
    return;
  }

  double widthMM = rect().width();
  double heightMM = rect().height();

  QPen enabledPen( QColor( 0, 0, 255, 255 ) );
  enabledPen.setWidthF( 0.4 );
  QPen disabledPen( QColor( 0, 0, 255, 127 ) );
  disabledPen.setWidthF( 0.3 );
  QBrush enabledBrush( QColor( 255, 0, 0, 255 ) );
  QBrush disabledBrush( QColor( 255, 0, 0, 127 ) );

  //draw all points and collect minimal mm/pixel ratio
  double minMMPixelRatio = DBL_MAX;
  double mmPixelRatio = 1;

  painter->setRenderHint( QPainter::Antialiasing, true );

  QgsGCPList::const_iterator gcpIt = mGCPList.constBegin();
  for ( ; gcpIt != mGCPList.constEnd(); ++gcpIt )
  {
    QgsPoint gcpCoords = ( *gcpIt )->pixelCoords();
    double gcpItemMMX = ( gcpCoords.x() - mExtent.xMinimum() ) / mExtent.width() * widthMM;
    double gcpItemMMY = ( 1 - ( gcpCoords.y() - mExtent.yMinimum() ) / mExtent.height() ) * heightMM;

    if (( *gcpIt )->isEnabled() )
    {
      painter->setPen( enabledPen );
      painter->setBrush( enabledBrush );
    }
    else
    {
      painter->setPen( disabledPen );
      painter->setBrush( disabledBrush );
    }
    painter->drawRect( QRectF( gcpItemMMX - 1, gcpItemMMY - 1, 2, 2 ) );
    drawText( painter, gcpItemMMX + 2, gcpItemMMY + 2, QString::number(( *gcpIt )->id() ), QFont() );

    mmPixelRatio = maxMMToPixelRatioForGCP( *gcpIt, gcpItemMMX, gcpItemMMY );
    if ( mmPixelRatio < minMMPixelRatio )
    {
      minMMPixelRatio = mmPixelRatio;
    }
  }

  //draw residual arrows
  gcpIt = mGCPList.constBegin();
  for ( ; gcpIt != mGCPList.constEnd(); ++gcpIt )
  {
    QgsPoint gcpCoords = ( *gcpIt )->pixelCoords();
    double gcpItemMMX = ( gcpCoords.x() - mExtent.xMinimum() ) / mExtent.width() * widthMM;
    double gcpItemMMY = ( 1 - ( gcpCoords.y() - mExtent.yMinimum() ) / mExtent.height() ) * heightMM;
    if (( *gcpIt )->isEnabled() )
    {
      painter->setPen( enabledPen );
    }
    else
    {
      painter->setPen( disabledPen );
    }

    QPointF p1( gcpItemMMX, gcpItemMMY );
    QPointF p2( gcpItemMMX + ( *gcpIt )->residual().x() * minMMPixelRatio, gcpItemMMY + ( *gcpIt )->residual().y() * minMMPixelRatio );
    painter->drawLine( p1, p2 );
    painter->setBrush( QBrush( painter->pen().color() ) );
    drawArrowHead( painter, p2.x(), p2.y(), angle( p1, p2 ), 1 );
  }

  //draw scale bar
  double initialScaleBarWidth = rect().width() / 5;
  int nUnits;
  double scaleBarWidth;
  if ( mConvertScaleToMapUnits ) //map units
  {
    nUnits = initialScaleBarWidth / minMMPixelRatio * mPixelToMapUnits;
    scaleBarWidth = nUnits * minMMPixelRatio / mPixelToMapUnits;
  }
  else //pixels
  {
    nUnits = initialScaleBarWidth / minMMPixelRatio;
    scaleBarWidth = nUnits * minMMPixelRatio;
  }

  painter->setPen( QColor( 0, 0, 0 ) );
  painter->drawLine( QPointF( 5, rect().height() - 5 ), QPointF( 5 + scaleBarWidth, rect().height() - 5 ) );
  painter->drawLine( QPointF( 5, rect().height() - 5 ), QPointF( 5, rect().height() - 7 ) );
  painter->drawLine( QPointF( 5 + scaleBarWidth, rect().height() - 5 ), QPointF( 5 + scaleBarWidth, rect().height() - 7 ) );
  QFont scaleBarFont;
  if ( mConvertScaleToMapUnits )
  {
    drawText( painter, 5, rect().height() - 4 + fontAscentMillimeters( scaleBarFont ), QString( "%1 map units" ).arg( nUnits ), QFont() );
  }
  else
  {
    drawText( painter, 5, rect().height() - 4 + fontAscentMillimeters( scaleBarFont ), QString( "%1 pixels" ).arg( nUnits ), QFont() );
  }

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

double QgsResidualPlotItem::maxMMToPixelRatioForGCP( const QgsGeorefDataPoint* p, double pixelXMM, double pixelYMM )
{
  if ( !p )
  {
    return 0;
  }

  //calculate intersections with upper / lower frame edge depending on the residual y sign
  double upDownDist = DBL_MAX; //distance to frame intersection with lower or upper frame
  double leftRightDist = DBL_MAX; //distance to frame intersection with left or right frame

  QPointF residual = p->residual();
  QLineF residualLine( pixelXMM, pixelYMM, pixelXMM + residual.x(), pixelYMM + residual.y() );
  QPointF intersectionPoint;
  double dx, dy;

  if ( residual.y() > 0 )
  {
    QLineF lowerFrameLine( 0, rect().height(), rect().width(), rect().height() );
    if ( residualLine.intersect( lowerFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
    {
      upDownDist = dist( QPointF( pixelXMM, pixelYMM ) , intersectionPoint );
    }
  }
  else if ( residual.y() < 0 )
  {
    QLineF upperFrameLine( 0, 0, mExtent.xMaximum(), 0 );
    if ( residualLine.intersect( upperFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
    {
      upDownDist = dist( QPointF( pixelXMM, pixelYMM ) , intersectionPoint );
    }
  }

  //calculate intersection with left / right frame edge depending on the residual x sign
  if ( residual.x() > 0 )
  {
    QLineF rightFrameLine( rect().width(), 0, rect().width(), rect().height() );
    if ( residualLine.intersect( rightFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
    {
      leftRightDist = dist( QPointF( pixelXMM, pixelYMM ) , intersectionPoint );
    }
  }
  else if ( residual.x() < 0 )
  {
    QLineF leftFrameLine( 0, 0 , 0, rect().height() );
    if ( residualLine.intersect( leftFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
    {
      leftRightDist = dist( QPointF( pixelXMM, pixelYMM ) , intersectionPoint );
    }
  }

  double resTot = sqrt( residual.x() * residual.x() + residual.y() * residual.y() );
  if ( leftRightDist <= upDownDist )
  {
    return leftRightDist / resTot;
  }
  else
  {
    return upDownDist / resTot;
  }
}

bool QgsResidualPlotItem::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  return false;
}

bool QgsResidualPlotItem::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  return false;
}

double QgsResidualPlotItem::dist( const QPointF& p1, const QPointF& p2 ) const
{
  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  return sqrt( dx * dx + dy * dy );
}
