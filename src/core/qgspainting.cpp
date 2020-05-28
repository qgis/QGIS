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

QPainter::CompositionMode QgsPainting::getCompositionMode( QgsPainting::BlendMode blendMode )
{
  // Map QgsPainting::BlendNormal to QPainter::CompositionMode
  switch ( blendMode )
  {
    case QgsPainting::BlendNormal:
      return QPainter::CompositionMode_SourceOver;
    case QgsPainting::BlendLighten:
      return QPainter::CompositionMode_Lighten;
    case QgsPainting::BlendScreen:
      return QPainter::CompositionMode_Screen;
    case QgsPainting::BlendDodge:
      return QPainter::CompositionMode_ColorDodge;
    case QgsPainting::BlendAddition:
      return QPainter::CompositionMode_Plus;
    case QgsPainting::BlendDarken:
      return QPainter::CompositionMode_Darken;
    case QgsPainting::BlendMultiply:
      return QPainter::CompositionMode_Multiply;
    case QgsPainting::BlendBurn:
      return QPainter::CompositionMode_ColorBurn;
    case QgsPainting::BlendOverlay:
      return QPainter::CompositionMode_Overlay;
    case QgsPainting::BlendSoftLight:
      return QPainter::CompositionMode_SoftLight;
    case QgsPainting::BlendHardLight:
      return QPainter::CompositionMode_HardLight;
    case QgsPainting::BlendDifference:
      return QPainter::CompositionMode_Difference;
    case QgsPainting::BlendSubtract:
      return QPainter::CompositionMode_Exclusion;
    case QgsPainting::BlendSource:
      return QPainter::CompositionMode_Source;
    case QgsPainting::BlendDestinationOver:
      return QPainter::CompositionMode_DestinationOver;
    case QgsPainting::BlendClear:
      return QPainter::CompositionMode_Clear;
    case QgsPainting::BlendDestination:
      return QPainter::CompositionMode_Destination;
    case QgsPainting::BlendSourceIn:
      return QPainter::CompositionMode_SourceIn;
    case QgsPainting::BlendDestinationIn:
      return QPainter::CompositionMode_DestinationIn;
    case QgsPainting::BlendSourceOut:
      return QPainter::CompositionMode_SourceOut;
    case QgsPainting::BlendDestinationOut:
      return QPainter::CompositionMode_DestinationOut;
    case QgsPainting::BlendSourceAtop:
      return QPainter::CompositionMode_SourceAtop;
    case QgsPainting::BlendDestinationAtop:
      return QPainter::CompositionMode_DestinationAtop;
    case QgsPainting::BlendXor:
      return QPainter::CompositionMode_Xor;
    default:
      QgsDebugMsg( QStringLiteral( "Blend mode %1 mapped to SourceOver" ).arg( blendMode ) );
      return QPainter::CompositionMode_SourceOver;
  }
}


QgsPainting::BlendMode QgsPainting::getBlendModeEnum( QPainter::CompositionMode blendMode )
{
  // Map QPainter::CompositionMode to QgsPainting::BlendNormal
  switch ( blendMode )
  {
    case QPainter::CompositionMode_SourceOver:
      return QgsPainting::BlendNormal;
    case QPainter::CompositionMode_Lighten:
      return QgsPainting::BlendLighten;
    case QPainter::CompositionMode_Screen:
      return QgsPainting::BlendScreen;
    case QPainter::CompositionMode_ColorDodge:
      return QgsPainting::BlendDodge;
    case QPainter::CompositionMode_Plus:
      return QgsPainting::BlendAddition;
    case QPainter::CompositionMode_Darken:
      return QgsPainting::BlendDarken;
    case QPainter::CompositionMode_Multiply:
      return QgsPainting::BlendMultiply;
    case QPainter::CompositionMode_ColorBurn:
      return QgsPainting::BlendBurn;
    case QPainter::CompositionMode_Overlay:
      return QgsPainting::BlendOverlay;
    case QPainter::CompositionMode_SoftLight:
      return QgsPainting::BlendSoftLight;
    case QPainter::CompositionMode_HardLight:
      return QgsPainting::BlendHardLight;
    case QPainter::CompositionMode_Difference:
      return QgsPainting::BlendDifference;
    case QPainter::CompositionMode_Exclusion:
      return QgsPainting::BlendSubtract;
    case QPainter::CompositionMode_Source:
      return QgsPainting::BlendSource;
    case QPainter::CompositionMode_DestinationOver:
      return QgsPainting::BlendDestinationOver;
    case QPainter::CompositionMode_Clear:
      return QgsPainting::BlendClear;
    case QPainter::CompositionMode_Destination:
      return QgsPainting::BlendDestination;
    case QPainter::CompositionMode_SourceIn:
      return QgsPainting::BlendSourceIn;
    case QPainter::CompositionMode_DestinationIn:
      return QgsPainting::BlendDestinationIn;
    case QPainter::CompositionMode_SourceOut:
      return QgsPainting::BlendSourceOut;
    case QPainter::CompositionMode_DestinationOut:
      return QgsPainting::BlendDestinationOut;
    case QPainter::CompositionMode_SourceAtop:
      return QgsPainting::BlendSourceAtop;
    case QPainter::CompositionMode_DestinationAtop:
      return QgsPainting::BlendDestinationAtop;
    case QPainter::CompositionMode_Xor:
      return QgsPainting::BlendXor;
    default:
      QgsDebugMsg( QStringLiteral( "Composition mode %1 mapped to Normal" ).arg( blendMode ) );
      return QgsPainting::BlendNormal;
  }
}
