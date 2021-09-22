/***************************************************************************
  qgstextrendererutils.h
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextrendererutils.h"
#include "qgsvectorlayer.h"
#include "qgslinestring.h"

QgsTextBackgroundSettings::ShapeType QgsTextRendererUtils::decodeShapeType( const QString &string )
{
  QgsTextBackgroundSettings::ShapeType shpkind = QgsTextBackgroundSettings::ShapeRectangle;
  const QString skind = string.trimmed();

  if ( skind.compare( QLatin1String( "Square" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeSquare;
  }
  else if ( skind.compare( QLatin1String( "Ellipse" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeEllipse;
  }
  else if ( skind.compare( QLatin1String( "Circle" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeCircle;
  }
  else if ( skind.compare( QLatin1String( "SVG" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeSVG;
  }
  else if ( skind.compare( QLatin1String( "marker" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeMarkerSymbol;
  }
  return shpkind;
}

QgsTextBackgroundSettings::SizeType QgsTextRendererUtils::decodeBackgroundSizeType( const QString &string )
{
  const QString stype = string.trimmed();
  // "Buffer"
  QgsTextBackgroundSettings::SizeType sizType = QgsTextBackgroundSettings::SizeBuffer;

  if ( stype.compare( QLatin1String( "Fixed" ), Qt::CaseInsensitive ) == 0 )
  {
    sizType = QgsTextBackgroundSettings::SizeFixed;
  }
  return sizType;
}

QgsTextBackgroundSettings::RotationType QgsTextRendererUtils::decodeBackgroundRotationType( const QString &string )
{
  const QString rotstr = string.trimmed();
  // "Sync"
  QgsTextBackgroundSettings::RotationType rottype = QgsTextBackgroundSettings::RotationSync;

  if ( rotstr.compare( QLatin1String( "Offset" ), Qt::CaseInsensitive ) == 0 )
  {
    rottype = QgsTextBackgroundSettings::RotationOffset;
  }
  else if ( rotstr.compare( QLatin1String( "Fixed" ), Qt::CaseInsensitive ) == 0 )
  {
    rottype = QgsTextBackgroundSettings::RotationFixed;
  }
  return rottype;
}

QgsTextShadowSettings::ShadowPlacement QgsTextRendererUtils::decodeShadowPlacementType( const QString &string )
{
  const QString str = string.trimmed();
  // "Lowest"
  QgsTextShadowSettings::ShadowPlacement shdwtype = QgsTextShadowSettings::ShadowLowest;

  if ( str.compare( QLatin1String( "Text" ), Qt::CaseInsensitive ) == 0 )
  {
    shdwtype = QgsTextShadowSettings::ShadowText;
  }
  else if ( str.compare( QLatin1String( "Buffer" ), Qt::CaseInsensitive ) == 0 )
  {
    shdwtype = QgsTextShadowSettings::ShadowBuffer;
  }
  else if ( str.compare( QLatin1String( "Background" ), Qt::CaseInsensitive ) == 0 )
  {
    shdwtype = QgsTextShadowSettings::ShadowShape;
  }
  return shdwtype;
}

QString QgsTextRendererUtils::encodeTextOrientation( QgsTextFormat::TextOrientation orientation )
{
  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
      return QStringLiteral( "horizontal" );
    case QgsTextFormat::VerticalOrientation:
      return QStringLiteral( "vertical" );
    case QgsTextFormat::RotationBasedOrientation:
      return QStringLiteral( "rotation-based" );
  }
  return QString();
}

QgsTextFormat::TextOrientation QgsTextRendererUtils::decodeTextOrientation( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;

  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "horizontal" ) )
    return QgsTextFormat::HorizontalOrientation;
  else if ( cleaned == QLatin1String( "vertical" ) )
    return QgsTextFormat::VerticalOrientation;
  else if ( cleaned == QLatin1String( "rotation-based" ) )
    return QgsTextFormat::RotationBasedOrientation;

  if ( ok )
    *ok = false;
  return QgsTextFormat::HorizontalOrientation;
}

QgsUnitTypes::RenderUnit QgsTextRendererUtils::convertFromOldLabelUnit( int val )
{
  if ( val == 0 )
    return QgsUnitTypes::RenderPoints;
  else if ( val == 1 )
    return QgsUnitTypes::RenderMillimeters;
  else if ( val == 2 )
    return QgsUnitTypes::RenderMapUnits;
  else if ( val == 3 )
    return QgsUnitTypes::RenderPercentage;
  else
    return QgsUnitTypes::RenderMillimeters;
}

QColor QgsTextRendererUtils::readColor( QgsVectorLayer *layer, const QString &property, const QColor &defaultColor, bool withAlpha )
{
  const int r = layer->customProperty( property + 'R', QVariant( defaultColor.red() ) ).toInt();
  const int g = layer->customProperty( property + 'G', QVariant( defaultColor.green() ) ).toInt();
  const int b = layer->customProperty( property + 'B', QVariant( defaultColor.blue() ) ).toInt();
  const int a = withAlpha ? layer->customProperty( property + 'A', QVariant( defaultColor.alpha() ) ).toInt() : 255;
  return QColor( r, g, b, a );
}

#if 0
QgsTextRendererUtils::CurvePlacementProperties *QgsTextRendererUtils::generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const QgsLineString *line, double offsetAlongLine, LabelLineDirection direction, double maxConcaveAngle, double maxConvexAngle, bool uprightOnly )
{
  const int numPoints = line->numPoints();
  std::vector<double> pathDistances( numPoints );

  const double *x = line->xData();
  const double *y = line->yData();
  double dx, dy;

  pathDistances[0] = 0;
  double prevX = *x++;
  double prevY = *y++;

  for ( int i = 1; i < numPoints; ++i )
  {
    dx = *x - prevX;
    dy = *y - prevY;
    pathDistances[i] = std::sqrt( dx * dx + dy * dy );

    prevX = *x++;
    prevY = *y++;
  }

  return generateCurvedTextPlacementPrivate( metrics, line->xData(), line->yData(), numPoints, pathDistances, offsetAlongLine, direction, maxConcaveAngle, maxConvexAngle, uprightOnly );
}
#endif

QgsTextRendererUtils::CurvePlacementProperties *QgsTextRendererUtils::generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const double *x, const double *y, int numPoints, const std::vector<double> &pathDistances, double offsetAlongLine, LabelLineDirection direction, double maxConcaveAngle, double maxConvexAngle, bool uprightOnly )
{
  return generateCurvedTextPlacementPrivate( metrics, x, y, numPoints, pathDistances, offsetAlongLine, direction, maxConcaveAngle, maxConvexAngle, uprightOnly );
}

QgsTextRendererUtils::CurvePlacementProperties *QgsTextRendererUtils::generateCurvedTextPlacementPrivate( const QgsPrecalculatedTextMetrics &metrics, const double *x, const double *y, int numPoints, const std::vector<double> &pathDistances, double offsetAlongLine, LabelLineDirection direction, double maxConcaveAngle, double maxConvexAngle, bool uprightOnly, bool isSecondAttempt )
{
  std::unique_ptr< CurvePlacementProperties > output = std::make_unique< CurvePlacementProperties >();
  output->graphemePlacement.reserve( metrics.count() );

  double offsetAlongSegment = offsetAlongLine;
  int index = 1;
  // Find index of segment corresponding to starting offset
  while ( index < numPoints && offsetAlongSegment > pathDistances[index] )
  {
    offsetAlongSegment -= pathDistances[index];
    index += 1;
  }
  if ( index >= numPoints )
  {
    return output.release();
  }

  const double characterHeight = metrics.characterHeight();

  const double segmentLength = pathDistances[index];
  if ( qgsDoubleNear( segmentLength, 0.0 ) )
  {
    // Not allowed to place across on 0 length segments or discontinuities
    return output.release();
  }

  const int characterCount = metrics.count();

  if ( direction == RespectPainterOrientation && !isSecondAttempt )
  {
    // Calculate the orientation based on the angle of the path segment under consideration

    double distance = offsetAlongSegment;
    int endindex = index;

    double startLabelX = 0;
    double startLabelY = 0;
    double endLabelX = 0;
    double endLabelY = 0;
    for ( int i = 0; i < characterCount; i++ )
    {
      const double characterWidth = metrics.characterWidth( i );
      double characterStartX, characterStartY;
      if ( !nextCharPosition( characterWidth, pathDistances[endindex], x, y, numPoints, endindex, distance, characterStartX, characterStartY, endLabelX, endLabelY ) )
      {
        return output.release();
      }
      if ( i == 0 )
      {
        startLabelX = characterStartX;
        startLabelY = characterStartY;
      }
    }

    // Determine the angle of the path segment under consideration
    const double dx = endLabelX - startLabelX;
    const double dy = endLabelY - startLabelY;
    const double lineAngle = std::atan2( -dy, dx ) * 180 / M_PI;

    if ( lineAngle > 90 || lineAngle < -90 )
    {
      output->labeledLineSegmentIsRightToLeft = true;
    }
  }

  if ( isSecondAttempt )
  {
    // we know that treating the segment as running from right to left gave too many upside down characters, so try again treating the
    // segment as left to right
    output->labeledLineSegmentIsRightToLeft = false;
    output->flippedCharacterPlacementToGetUprightLabels = true;
  }

  const double dx = x[index] - x[index - 1];
  const double dy = y[index] - y[index - 1];

  double angle = std::atan2( -dy, dx );

  for ( int i = 0; i < characterCount; i++ )
  {
    const double lastCharacterAngle = angle;

    // grab the next character according to the orientation
    const double characterWidth = !output->flippedCharacterPlacementToGetUprightLabels ? metrics.characterWidth( i ) : metrics.characterWidth( characterCount - i - 1 );
    if ( qgsDoubleNear( characterWidth, 0.0 ) )
      // Certain scripts rely on zero-width character, skip those to prevent failure (see #15801)
      continue;

    double characterStartX = 0;
    double characterStartY = 0;
    double characterEndX = 0;
    double characterEndY = 0;
    if ( !nextCharPosition( characterWidth, pathDistances[index], x, y, numPoints, index, offsetAlongSegment, characterStartX, characterStartY, characterEndX, characterEndY ) )
    {
      output->graphemePlacement.clear();
      return output.release();
    }

    // Calculate angle from the start of the character to the end based on start/end of character
    angle = std::atan2( characterStartY - characterEndY, characterEndX - characterStartX );

    if ( maxConcaveAngle >= 0 || maxConvexAngle >= 0 )
    {
      // Test lastCharacterAngle vs angle
      // since our rendering angle has changed then check against our
      // max allowable angle change.
      double angleDelta = lastCharacterAngle - angle;
      // normalise between -180 and 180
      while ( angleDelta > M_PI )
        angleDelta -= 2 * M_PI;
      while ( angleDelta < -M_PI )
        angleDelta += 2 * M_PI;
      if ( ( maxConcaveAngle >= 0 && angleDelta > 0 && angleDelta > maxConcaveAngle ) || ( maxConvexAngle >= 0 && angleDelta < 0 && angleDelta < -maxConvexAngle ) )
      {
        output->graphemePlacement.clear();
        return output.release();
      }
    }

    // Shift the character downwards since the draw position is specified at the baseline
    // and we're calculating the mean line here
    double dist = 0.9 * metrics.characterHeight() / 2;
    if ( output->flippedCharacterPlacementToGetUprightLabels )
    {
      dist = -dist;
    }
    characterStartX += dist * std::cos( angle + M_PI_2 );
    characterStartY -= dist * std::sin( angle + M_PI_2 );

    double renderAngle = angle;
    CurvedGraphemePlacement placement;
    placement.graphemeIndex = !output->flippedCharacterPlacementToGetUprightLabels ? i : characterCount - i - 1;
    placement.x = characterStartX;
    placement.y = characterStartY;
    placement.width = characterWidth;
    placement.height = characterHeight;
    if ( output->flippedCharacterPlacementToGetUprightLabels )
    {
      // rotate in place
      placement.x += characterWidth * std::cos( renderAngle );
      placement.y -= characterWidth * std::sin( renderAngle );
      renderAngle += M_PI;
    }
    placement.angle = -renderAngle;
    output->graphemePlacement.push_back( placement );

    // Normalise to 0 <= angle < 2PI
    while ( renderAngle >= 2 * M_PI )
      renderAngle -= 2 * M_PI;
    while ( renderAngle < 0 )
      renderAngle += 2 * M_PI;

    if ( renderAngle > M_PI_2 && renderAngle < 1.5 * M_PI )
      output->upsideDownCharCount++;
  }

  if ( !isSecondAttempt && uprightOnly && output->upsideDownCharCount >= characterCount / 2.0 )
  {
    // more of text is upside down then right side up...
    // if text should be shown upright then retry with the opposite orientation
    return generateCurvedTextPlacementPrivate( metrics, x, y, numPoints, pathDistances, offsetAlongLine, direction, maxConcaveAngle, maxConvexAngle, uprightOnly, true );
  }

  return output.release();
}

bool QgsTextRendererUtils::nextCharPosition( double charWidth, double segmentLength, const double *x, const double *y, int numPoints, int &index, double &currentDistanceAlongSegment, double &characterStartX, double &characterStartY, double &characterEndX, double &characterEndY )
{
  // Coordinates this character will start at
  if ( qgsDoubleNear( segmentLength, 0.0 ) )
  {
    // Not allowed to place across on 0 length segments or discontinuities
    return false;
  }

  double segmentStartX = x[index - 1];
  double segmentStartY = y[index - 1];

  double segmentEndX = x[index];
  double segmentEndY = y[index];

  const double segmentDx = segmentEndX - segmentStartX;
  const double segmentDy = segmentEndY - segmentStartY;

  characterStartX = segmentStartX + segmentDx * currentDistanceAlongSegment / segmentLength;
  characterStartY = segmentStartY + segmentDy * currentDistanceAlongSegment / segmentLength;

  // Coordinates this character ends at, calculated below
  characterEndX = 0;
  characterEndY = 0;

  if ( segmentLength - currentDistanceAlongSegment >= charWidth )
  {
    // if the distance remaining in this segment is enough, we just go further along the segment
    currentDistanceAlongSegment += charWidth;
    characterEndX = segmentStartX + segmentDx * currentDistanceAlongSegment / segmentLength;
    characterEndY = segmentStartY + segmentDy * currentDistanceAlongSegment / segmentLength;
  }
  else
  {
    // If there isn't enough distance left on this segment
    // then we need to search until we find the line segment that ends further than ci.width away
    do
    {
      segmentStartX = segmentEndX;
      segmentStartY = segmentEndY;
      index++;
      if ( index >= numPoints ) // Bail out if we run off the end of the shape
      {
        return false;
      }
      segmentEndX = x[index];
      segmentEndY = y[index];
    }
    while ( std::sqrt( std::pow( characterStartX - segmentEndX, 2 ) + std::pow( characterStartY - segmentEndY, 2 ) ) < charWidth ); // Distance from character start to end

    // Calculate the position to place the end of the character on
    findLineCircleIntersection( characterStartX, characterStartY, charWidth, segmentStartX, segmentStartY, segmentEndX, segmentEndY, characterEndX, characterEndY );

    // Need to calculate distance on the new segment
    currentDistanceAlongSegment = std::sqrt( std::pow( segmentStartX - characterEndX, 2 ) + std::pow( segmentStartY - characterEndY, 2 ) );
  }
  return true;
}

void QgsTextRendererUtils::findLineCircleIntersection( double cx, double cy, double radius, double x1, double y1, double x2, double y2, double &xRes, double &yRes )
{
  double multiplier = 1;
  if ( radius < 10 )
  {
    // these calculations get unstable for small coordinates differences, e.g. as a result of map labeling in a geographic
    // CRS
    multiplier = 10000;
    x1 *= multiplier;
    y1 *= multiplier;
    x2 *= multiplier;
    y2 *= multiplier;
    cx *= multiplier;
    cy *= multiplier;
    radius *= multiplier;
  }

  const double dx = x2 - x1;
  const double dy = y2 - y1;

  const double A = dx * dx + dy * dy;
  const double B = 2 * ( dx * ( x1 - cx ) + dy * ( y1 - cy ) );
  const double C = ( x1 - cx ) * ( x1 - cx ) + ( y1 - cy ) * ( y1 - cy ) - radius * radius;

  const double det = B * B - 4 * A * C;
  if ( A <= 0.000000000001 || det < 0 )
    // Should never happen, No real solutions.
    return;

  if ( qgsDoubleNear( det, 0.0 ) )
  {
    // Could potentially happen.... One solution.
    const double t = -B / ( 2 * A );
    xRes = x1 + t * dx;
    yRes = y1 + t * dy;
  }
  else
  {
    // Two solutions.
    // Always use the 1st one
    // We only really have one solution here, as we know the line segment will start in the circle and end outside
    const double t = ( -B + std::sqrt( det ) ) / ( 2 * A );
    xRes = x1 + t * dx;
    yRes = y1 + t * dy;
  }

  if ( multiplier != 1 )
  {
    xRes /= multiplier;
    yRes /= multiplier;
  }
}
