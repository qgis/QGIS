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
