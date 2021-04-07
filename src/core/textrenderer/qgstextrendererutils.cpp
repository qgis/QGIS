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

  QString cleaned = name.toLower().trimmed();

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
  int r = layer->customProperty( property + 'R', QVariant( defaultColor.red() ) ).toInt();
  int g = layer->customProperty( property + 'G', QVariant( defaultColor.green() ) ).toInt();
  int b = layer->customProperty( property + 'B', QVariant( defaultColor.blue() ) ).toInt();
  int a = withAlpha ? layer->customProperty( property + 'A', QVariant( defaultColor.alpha() ) ).toInt() : 255;
  return QColor( r, g, b, a );
}

QgsTextRendererUtils::CurvePlacementProperties *QgsTextRendererUtils::generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const QgsLineString *line, double offsetAlongLine, int orientation, double maxConcaveAngle, double maxConvexAngle, bool uprightOnly )
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

  return generateCurvedTextPlacement( metrics, line->xData(), line->yData(), numPoints, pathDistances, offsetAlongLine, orientation, maxConcaveAngle, maxConvexAngle, uprightOnly );
}

QgsTextRendererUtils::CurvePlacementProperties *QgsTextRendererUtils::generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const double *x, const double *y, int numPoints, const std::vector<double> &pathDistances, double offsetAlongLine, const int orientation, double maxConcaveAngle, double maxConvexAngle, bool uprightOnly )
{
  std::unique_ptr< CurvePlacementProperties > output = std::make_unique< CurvePlacementProperties >();
  output->graphemePlacement.reserve( metrics.count() );
  output->orientation = orientation;

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
    return nullptr;
  }

  const double characterHeight = metrics.characterHeight();

  const double segmentLength = pathDistances[index];
  if ( qgsDoubleNear( segmentLength, 0.0 ) )
  {
    // Not allowed to place across on 0 length segments or discontinuities
    return nullptr;
  }

  const int characterCount = metrics.count();

  if ( orientation == 0 )       // Must be map orientation
  {
    // Calculate the orientation based on the angle of the path segment under consideration

    double _distance = offsetAlongSegment;
    int endindex = index;

    double startLabelX = 0;
    double startLabelY = 0;
    double endLabelX = 0;
    double endLabelY = 0;
    for ( int i = 0; i < characterCount; i++ )
    {
      const double characterWidth = metrics.characterWidth( i );
      double characterStartX, characterStartY;
      if ( !nextCharPosition( characterWidth, pathDistances[endindex], x, y, numPoints, endindex, _distance, characterStartX, characterStartY, endLabelX, endLabelY ) )
      {
        return nullptr;
      }
      if ( i == 0 )
      {
        startLabelX = characterStartX;
        startLabelY = characterStartY;
      }
    }

    // Determine the angle of the path segment under consideration
    double dx = endLabelX - startLabelX;
    double dy = endLabelY - startLabelY;
    const double lineAngle = std::atan2( -dy, dx ) * 180 / M_PI;

    bool isRightToLeft = ( lineAngle > 90 || lineAngle < -90 );
    output->reversed = isRightToLeft;
    output->orientation = isRightToLeft ? -1 : 1;
  }

  if ( !uprightOnly )
  {
    if ( orientation < 0 )
    {
      output->flip = true;   // Report to the caller, that the orientation is flipped
      output->reversed = !output->reversed;
      output->orientation = 1;
    }
  }

  double old_x = x[index - 1];
  double old_y = y[index - 1];

  double new_x = x[index];
  double new_y = y[index];

  double dx = new_x - old_x;
  double dy = new_y - old_y;

  double angle = std::atan2( -dy, dx );

  for ( int i = 0; i < characterCount; i++ )
  {
    double last_character_angle = angle;

    // grab the next character according to the orientation
    const double characterWidth = ( orientation > 0 ? metrics.characterWidth( i ) : metrics.characterWidth( characterCount - i - 1 ) );
    if ( qgsDoubleNear( characterWidth, 0.0 ) )
      // Certain scripts rely on zero-width character, skip those to prevent failure (see #15801)
      continue;

    double start_x, start_y, end_x, end_y;
    if ( !nextCharPosition( characterWidth, pathDistances[index], x, y, numPoints, index, offsetAlongSegment, start_x, start_y, end_x, end_y ) )
    {
      return nullptr;
    }

    // Calculate angle from the start of the character to the end based on start_/end_ position
    angle = std::atan2( start_y - end_y, end_x - start_x );

    // Test last_character_angle vs angle
    // since our rendering angle has changed then check against our
    // max allowable angle change.
    double angleDelta = last_character_angle - angle;
    // normalise between -180 and 180
    while ( angleDelta > M_PI )
      angleDelta -= 2 * M_PI;
    while ( angleDelta < -M_PI )
      angleDelta += 2 * M_PI;
    if ( ( maxConcaveAngle > 0 && angleDelta > 0 && angleDelta > maxConcaveAngle ) || ( maxConvexAngle > 0 && angleDelta < 0 && angleDelta < -maxConvexAngle ) )
    {
      return nullptr;
    }

    // Shift the character downwards since the draw position is specified at the baseline
    // and we're calculating the mean line here
    double dist = 0.9 * metrics.characterHeight() / 2;
    if ( orientation < 0 )
    {
      dist = -dist;
      output->flip = true;
    }
    start_x += dist * std::cos( angle + M_PI_2 );
    start_y -= dist * std::sin( angle + M_PI_2 );

    double render_angle = angle;

    double render_x = start_x;
    double render_y = start_y;

    // Center the text on the line
    //render_x -= ((string_height/2.0) - 1.0)*math.cos(render_angle+math.pi/2)
    //render_y += ((string_height/2.0) - 1.0)*math.sin(render_angle+math.pi/2)

    if ( orientation < 0 )
    {
      // rotate in place
      render_x += characterWidth * std::cos( render_angle ); //- (string_height-2)*sin(render_angle);
      render_y -= characterWidth * std::sin( render_angle ); //+ (string_height-2)*cos(render_angle);
      render_angle += M_PI;
    }

    CurvedGraphemePlacement placement;
    placement.x = render_x;
    placement.y = render_y;
    placement.width = characterWidth;
    placement.height = characterHeight;
    placement.angle = -render_angle;
    placement.partId = orientation > 0 ? i : characterCount - i - 1;
    output->graphemePlacement.push_back( placement );

    // Normalise to 0 <= angle < 2PI
    while ( render_angle >= 2 * M_PI )
      render_angle -= 2 * M_PI;
    while ( render_angle < 0 )
      render_angle += 2 * M_PI;

    if ( render_angle > M_PI_2 && render_angle < 1.5 * M_PI )
      output->upsideDownCharCount++;
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

  double segmentDx = segmentEndX - segmentStartX;
  double segmentDy = segmentEndY - segmentStartY;

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
    while ( std::sqrt( std::pow( characterStartX - segmentEndX, 2 ) + std::pow( characterStartY - segmentEndY, 2 ) ) < charWidth ); // Distance from start_ to new_

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

  double dx = x2 - x1;
  double dy = y2 - y1;

  double A = dx * dx + dy * dy;
  double B = 2 * ( dx * ( x1 - cx ) + dy * ( y1 - cy ) );
  double C = ( x1 - cx ) * ( x1 - cx ) + ( y1 - cy ) * ( y1 - cy ) - radius * radius;

  double det = B * B - 4 * A * C;
  if ( A <= 0.000000000001 || det < 0 )
    // Should never happen, No real solutions.
    return;

  if ( qgsDoubleNear( det, 0.0 ) )
  {
    // Could potentially happen.... One solution.
    double t = -B / ( 2 * A );
    xRes = x1 + t * dx;
    yRes = y1 + t * dy;
  }
  else
  {
    // Two solutions.
    // Always use the 1st one
    // We only really have one solution here, as we know the line segment will start in the circle and end outside
    double t = ( -B + std::sqrt( det ) ) / ( 2 * A );
    xRes = x1 + t * dx;
    yRes = y1 + t * dy;
  }

  if ( multiplier != 1 )
  {
    xRes /= multiplier;
    yRes /= multiplier;
  }
}
