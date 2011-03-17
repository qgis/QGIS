#include "qgsdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgsrendercontext.h"

#include <QPainter>

void QgsDiagram::setPenWidth( QPen& pen, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    pen.setWidthF( s.penWidth * c.scaleFactor() );
  }
  else
  {
    pen.setWidthF( s.penWidth / c.mapToPixel().mapUnitsPerPixel() );
  }
}

QSizeF QgsDiagram::sizePainterUnits( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    return QSizeF( s.size.width() * c.scaleFactor(), s.size.height() * c.scaleFactor() );
  }
  else
  {
    return QSizeF( s.size.width() / c.mapToPixel().mapUnitsPerPixel(), s.size.height() / c.mapToPixel().mapUnitsPerPixel() );
  }
}

QFont QgsDiagram::scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  QFont f = s.font;
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    f.setPixelSize( s.font.pointSizeF() * 0.376 * c.scaleFactor() );
  }
  else
  {
    f.setPixelSize( s.font.pointSizeF() / c.mapToPixel().mapUnitsPerPixel() );
  }

  return f;
}

QgsTextDiagram::QgsTextDiagram(): mOrientation( Vertical ), mShape( Circle )
{
  mPen.setWidthF( 2.0 );
  mPen.setColor( QColor( 0, 0, 0 ) );
  mPen.setCapStyle( Qt::FlatCap );
  mBrush.setStyle( Qt::SolidPattern );
}

QgsTextDiagram::~QgsTextDiagram()
{
}

void QgsTextDiagram::renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
{
  QPainter* p = c.painter();
  if ( !p )
  {
    return;
  }

  double scaleDenominator = c.rendererScale();
  if (( s.minScaleDenominator != -1 && scaleDenominator < s.minScaleDenominator )
      || ( s.maxScaleDenominator != -1 && scaleDenominator > s.maxScaleDenominator ) )
  {
    return;
  }

  //convert from mm / map units to painter units
  QSizeF spu = sizePainterUnits( s.size, s, c );
  double w = spu.width();
  double h = spu.height();

  double baseX = position.x();
  double baseY = position.y() - h;

  QList<QPointF> textPositions; //midpoints for text placement
  int nCategories = s.categoryIndices.size();
  for ( int i = 0; i < nCategories; ++i )
  {
    if ( mOrientation == Horizontal )
    {
      textPositions.push_back( QPointF( baseX + ( w / nCategories ) * i + w / nCategories / 2.0 , baseY + h / 2.0 ) );
    }
    else //vertical
    {
      textPositions.push_back( QPointF( baseX + w / 2.0, baseY + h / nCategories * i + w / nCategories / 2.0 ) );
    }
  }

  mPen.setColor( s.penColor );
  setPenWidth( mPen, s, c );
  p->setPen( mPen );
  mBrush.setColor( s.backgroundColor );
  p->setBrush( mBrush );

  //draw shapes and separator lines first
  if ( mShape == Circle )
  {
    p->drawEllipse( baseX, baseY, w, h );

    //draw separator lines
    QList<QPointF> intersect; //intersections between shape and separation lines
    QPointF center( baseX + w / 2.0, baseY + h / 2.0 );
    double r1 = w / 2.0; double r2 = h / 2.0;

    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        lineEllipseIntersection( QPointF( baseX + w / nCategories * i, baseY ), QPointF( baseX + w / nCategories * i, baseY + h ), center, r1, r2, intersect );
      }
      else //vertical
      {
        lineEllipseIntersection( QPointF( baseX, baseY + h / nCategories * i ), QPointF( baseX + w, baseY + h / nCategories * i ), center, r1, r2, intersect );
      }
      if ( intersect.size() > 1 )
      {
        p->drawLine( intersect.at( 0 ), intersect.at( 1 ) );
      }
    }
  }
  else if ( mShape == Rectangle )
  {
    p->drawRect( QRectF( baseX, baseY, w, h ) );
    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        p->drawLine( QPointF( baseX + w / nCategories * i , baseY ), QPointF( baseX + w / nCategories * i, baseY + h ) );
      }
      else
      {
        p->drawLine( QPointF( baseX, baseY + h / nCategories * i ) , QPointF( baseX + w,  baseY + h / nCategories * i ) );
      }
    }
  }
  else //triangle
  {
    QPolygonF triangle;
    triangle << QPointF( baseX, baseY + h ) << QPointF( baseX + w, baseY + h ) << QPointF( baseX + w / 2.0, baseY );
    p->drawPolygon( triangle );

    QLineF triangleEdgeLeft( baseX + w / 2.0, baseY, baseX, baseY + h );
    QLineF triangleEdgeRight( baseX + w, baseY + h, baseX + w / 2.0, baseY );
    QPointF intersectionPoint1, intersectionPoint2;

    for ( int i = 1; i < nCategories; ++i )
    {
      if ( mOrientation == Horizontal )
      {
        QLineF verticalLine( baseX + w / nCategories * i, baseY + h, baseX + w / nCategories * i, baseY );
        if ( baseX + w / nCategories * i < baseX + w / 2.0 )
        {
          verticalLine.intersect( triangleEdgeLeft, &intersectionPoint1 );
        }
        else
        {
          verticalLine.intersect( triangleEdgeRight, &intersectionPoint1 );
        }
        p->drawLine( QPointF( baseX + w / nCategories * i, baseY + h ), intersectionPoint1 );
      }
      else //vertical
      {
        QLineF horizontalLine( baseX, baseY + h / nCategories * i, baseX + w, baseY + h / nCategories * i );
        horizontalLine.intersect( triangleEdgeLeft, &intersectionPoint1 );
        horizontalLine.intersect( triangleEdgeRight, &intersectionPoint2 );
        p->drawLine( intersectionPoint1, intersectionPoint2 );
      }
    }
  }

  //draw text
  QFont sFont = scaledFont( s, c );
  QFontMetricsF fontMetrics( sFont );
  p->setFont( sFont );

  for ( int i = 0; i < textPositions.size(); ++i )
  {
    QString val = att[ s.categoryIndices.at( i )].toString();
    //find out dimensions
    double textHeight = fontMetrics.height();
    double textWidth = fontMetrics.width( val );
    mPen.setColor( s.categoryColors.at( i ) );
    p->setPen( mPen );
    QPointF position = textPositions.at( i );
    p->drawText( QPointF( position.x() - textWidth / 2.0, position.y() + textHeight / 2.0 ), val );
  }
}

void QgsTextDiagram::lineEllipseIntersection( const QPointF& lineStart, const QPointF& lineEnd, const QPointF& ellipseMid, double r1, double r2, QList<QPointF>& result ) const
{
  result.clear();

  double rrx = r1 * r1;
  double rry = r2 * r2;
  double x21 = lineEnd.x() - lineStart.x();
  double y21 = lineEnd.y() - lineStart.y();
  double x10 = lineStart.x() - ellipseMid.x();
  double y10 = lineStart.y() - ellipseMid.y();
  double a = x21 * x21 / rrx + y21 * y21 / rry;
  double b = x21 * x10 / rrx + y21 * y10 / rry;
  double c = x10 * x10 / rrx + y10 * y10 / rry;
  double d = b * b - a * ( c - 1 );
  if ( d > 0 )
  {
    double e = sqrt( d );
    double u1 = ( -b - e ) / a;
    double u2 = ( -b + e ) / a;
    //work with a tolerance of 0.00001 because of limited numerical precision
    if ( -0.00001 <= u1 && u1 < 1.00001 )
    {
      result.push_back( QPointF( lineStart.x() + x21 * u1, lineStart.y() + y21 * u1 ) );
    }
    if ( -0.00001 <= u2 && u2 <= 1.00001 )
    {
      result.push_back( QPointF( lineStart.x() + x21 * u2, lineStart.y() + y21 * u2 ) );
    }
  }
}

QgsPieDiagram::QgsPieDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
}

QgsPieDiagram::~QgsPieDiagram()
{
}

void QgsPieDiagram::renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )
{
  QPainter* p = c.painter();
  if ( !p )
  {
    return;
  }

  //get sum of values
  QList<double> values;
  double currentVal = 0;
  double valSum = 0;

  QList<int>::const_iterator catIt = s.categoryIndices.constBegin();
  for ( ; catIt != s.categoryIndices.constEnd(); ++catIt )
  {
    currentVal = att[*catIt].toDouble();
    values.push_back( currentVal );
    valSum += currentVal;
  }

  //draw the slices
  double totalAngle = 0;
  double currentAngle;

  //convert from mm / map units to painter units
  QSizeF spu = sizePainterUnits( s.size, s, c );
  double w = spu.width();
  double h = spu.height();

  double baseX = position.x();
  double baseY = position.y() - h;

  mPen.setColor( s.penColor );
  setPenWidth( mPen, s, c );
  p->setPen( mPen );

  QList<double>::const_iterator valIt = values.constBegin();
  QList< QColor >::const_iterator colIt = s.categoryColors.constBegin();
  for ( ; valIt != values.constEnd(); ++valIt, ++colIt )
  {
    currentAngle =  *valIt / valSum * 360 * 16;
    mCategoryBrush.setColor( *colIt );
    p->setBrush( mCategoryBrush );
    p->drawPie( baseX, baseY, w, h, totalAngle, currentAngle );
    totalAngle += currentAngle;
  }
}
