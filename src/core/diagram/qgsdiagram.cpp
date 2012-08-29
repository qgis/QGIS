/***************************************************************************
    qgsdiagram.cpp
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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

QSizeF QgsDiagram::sizeForPAL( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    return QSizeF( size.width() / c.rasterScaleFactor(), size.height() / c.rasterScaleFactor() );
  }
  else
  {
    return QSizeF( size );
  }
}

QSizeF QgsDiagram::sizePainterUnits( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  Q_UNUSED( size );
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    return QSizeF( s.size.width() * c.scaleFactor() * c.rasterScaleFactor(), s.size.height() * c.scaleFactor() * c.rasterScaleFactor());
  }
  else
  {
    return QSizeF( s.size.width() / c.mapToPixel().mapUnitsPerPixel(), s.size.height() / c.mapToPixel().mapUnitsPerPixel() );
  }
}

float QgsDiagram::sizePainterUnits( float l, const QgsDiagramSettings& s, const QgsRenderContext& c )
{
  if ( s.sizeType == QgsDiagramSettings::MM )
  {
    return l * c.scaleFactor();
  }
  else
  {
    return l / c.mapToPixel().mapUnitsPerPixel();
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
