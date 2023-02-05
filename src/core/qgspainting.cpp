/***************************************************************************
    qgspainting.cpp
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspainting.h"

#include "qgslogger.h"

QPainter::CompositionMode QgsPainting::getCompositionMode( Qgis::BlendMode blendMode )
{
  // Map Qgis::BlendMode::Normal to QPainter::CompositionMode
  switch ( blendMode )
  {
    case Qgis::BlendMode::Normal:
      return QPainter::CompositionMode_SourceOver;
    case Qgis::BlendMode::Lighten:
      return QPainter::CompositionMode_Lighten;
    case Qgis::BlendMode::Screen:
      return QPainter::CompositionMode_Screen;
    case Qgis::BlendMode::Dodge:
      return QPainter::CompositionMode_ColorDodge;
    case Qgis::BlendMode::Addition:
      return QPainter::CompositionMode_Plus;
    case Qgis::BlendMode::Darken:
      return QPainter::CompositionMode_Darken;
    case Qgis::BlendMode::Multiply:
      return QPainter::CompositionMode_Multiply;
    case Qgis::BlendMode::Burn:
      return QPainter::CompositionMode_ColorBurn;
    case Qgis::BlendMode::Overlay:
      return QPainter::CompositionMode_Overlay;
    case Qgis::BlendMode::SoftLight:
      return QPainter::CompositionMode_SoftLight;
    case Qgis::BlendMode::HardLight:
      return QPainter::CompositionMode_HardLight;
    case Qgis::BlendMode::Difference:
      return QPainter::CompositionMode_Difference;
    case Qgis::BlendMode::Subtract:
      return QPainter::CompositionMode_Exclusion;
    case Qgis::BlendMode::Source:
      return QPainter::CompositionMode_Source;
    case Qgis::BlendMode::DestinationOver:
      return QPainter::CompositionMode_DestinationOver;
    case Qgis::BlendMode::Clear:
      return QPainter::CompositionMode_Clear;
    case Qgis::BlendMode::Destination:
      return QPainter::CompositionMode_Destination;
    case Qgis::BlendMode::SourceIn:
      return QPainter::CompositionMode_SourceIn;
    case Qgis::BlendMode::DestinationIn:
      return QPainter::CompositionMode_DestinationIn;
    case Qgis::BlendMode::SourceOut:
      return QPainter::CompositionMode_SourceOut;
    case Qgis::BlendMode::DestinationOut:
      return QPainter::CompositionMode_DestinationOut;
    case Qgis::BlendMode::SourceAtop:
      return QPainter::CompositionMode_SourceAtop;
    case Qgis::BlendMode::DestinationAtop:
      return QPainter::CompositionMode_DestinationAtop;
    case Qgis::BlendMode::Xor:
      return QPainter::CompositionMode_Xor;
    default:
      QgsDebugMsg( QStringLiteral( "Blend mode %1 mapped to SourceOver" ).arg( qgsEnumValueToKey( blendMode ) ) );
      return QPainter::CompositionMode_SourceOver;
  }
}


Qgis::BlendMode QgsPainting::getBlendModeEnum( QPainter::CompositionMode blendMode )
{
  // Map QPainter::CompositionMode to Qgis::BlendMode::Normal
  switch ( blendMode )
  {
    case QPainter::CompositionMode_SourceOver:
      return Qgis::BlendMode::Normal;
    case QPainter::CompositionMode_Lighten:
      return Qgis::BlendMode::Lighten;
    case QPainter::CompositionMode_Screen:
      return Qgis::BlendMode::Screen;
    case QPainter::CompositionMode_ColorDodge:
      return Qgis::BlendMode::Dodge;
    case QPainter::CompositionMode_Plus:
      return Qgis::BlendMode::Addition;
    case QPainter::CompositionMode_Darken:
      return Qgis::BlendMode::Darken;
    case QPainter::CompositionMode_Multiply:
      return Qgis::BlendMode::Multiply;
    case QPainter::CompositionMode_ColorBurn:
      return Qgis::BlendMode::Burn;
    case QPainter::CompositionMode_Overlay:
      return Qgis::BlendMode::Overlay;
    case QPainter::CompositionMode_SoftLight:
      return Qgis::BlendMode::SoftLight;
    case QPainter::CompositionMode_HardLight:
      return Qgis::BlendMode::HardLight;
    case QPainter::CompositionMode_Difference:
      return Qgis::BlendMode::Difference;
    case QPainter::CompositionMode_Exclusion:
      return Qgis::BlendMode::Subtract;
    case QPainter::CompositionMode_Source:
      return Qgis::BlendMode::Source;
    case QPainter::CompositionMode_DestinationOver:
      return Qgis::BlendMode::DestinationOver;
    case QPainter::CompositionMode_Clear:
      return Qgis::BlendMode::Clear;
    case QPainter::CompositionMode_Destination:
      return Qgis::BlendMode::Destination;
    case QPainter::CompositionMode_SourceIn:
      return Qgis::BlendMode::SourceIn;
    case QPainter::CompositionMode_DestinationIn:
      return Qgis::BlendMode::DestinationIn;
    case QPainter::CompositionMode_SourceOut:
      return Qgis::BlendMode::SourceOut;
    case QPainter::CompositionMode_DestinationOut:
      return Qgis::BlendMode::DestinationOut;
    case QPainter::CompositionMode_SourceAtop:
      return Qgis::BlendMode::SourceAtop;
    case QPainter::CompositionMode_DestinationAtop:
      return Qgis::BlendMode::DestinationAtop;
    case QPainter::CompositionMode_Xor:
      return Qgis::BlendMode::Xor;
    default:
      QgsDebugMsg( QStringLiteral( "Composition mode %1 mapped to Normal" ).arg( blendMode ) );
      return Qgis::BlendMode::Normal;
  }
}

bool QgsPainting::isClippingMode( Qgis::BlendMode mode )
{
  switch ( mode )
  {
    case Qgis::BlendMode::Normal:
    case Qgis::BlendMode::Lighten:
    case Qgis::BlendMode::Screen:
    case Qgis::BlendMode::Dodge:
    case Qgis::BlendMode::Addition:
    case Qgis::BlendMode::Darken:
    case Qgis::BlendMode::Multiply:
    case Qgis::BlendMode::Burn:
    case Qgis::BlendMode::Overlay:
    case Qgis::BlendMode::SoftLight:
    case Qgis::BlendMode::HardLight:
    case Qgis::BlendMode::Difference:
    case Qgis::BlendMode::Subtract:
    case Qgis::BlendMode::Source:
    case Qgis::BlendMode::DestinationOver:
    case Qgis::BlendMode::Clear:
    case Qgis::BlendMode::Destination:
      return false;

    case Qgis::BlendMode::SourceIn:
    case Qgis::BlendMode::DestinationIn:
    case Qgis::BlendMode::SourceOut:
    case Qgis::BlendMode::DestinationOut:
    case Qgis::BlendMode::SourceAtop:
    case Qgis::BlendMode::DestinationAtop:
    case Qgis::BlendMode::Xor:
      return true;
  }
  return false;
}
