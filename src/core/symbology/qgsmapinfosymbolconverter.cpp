/***************************************************************************
  qgsmapinfosymbolconverter.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapinfosymbolconverter.h"
#include "qgslogger.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgssymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

//
// QgsMapInfoSymbolConversionContext
//
void QgsMapInfoSymbolConversionContext::pushWarning( const QString &warning )
{
  QgsDebugMsg( warning );
  mWarnings << warning;
}


QgsLineSymbol *QgsMapInfoSymbolConverter::convertLineSymbol( int identifier, QgsMapInfoSymbolConversionContext &context, const QColor &foreColor, double size, QgsUnitTypes::RenderUnit sizeUnit, bool interleaved )
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, size );
  simpleLine->setWidthUnit( sizeUnit );
  simpleLine->setPenCapStyle( Qt::RoundCap );
  simpleLine->setPenJoinStyle( Qt::RoundJoin );

  QVector< qreal > dashPattern;
  double patternOffset = 0;
  switch ( identifier )
  {
    case 1:
    case 57:
    case 58:
    case 82:
    case 90:
    case 98:
    case 106:
      // no pen
      simpleLine->setPenStyle( Qt::NoPen );
      break;

    case 2:
    case 38:
      // solid line
      break;

    case 3:
      dashPattern << 1 << 2.2;
      break;

    case 4:
      dashPattern << 2 << 2;
      break;

    case 5:
      dashPattern << 4 << 2;
      break;

    case 6:
      dashPattern << 8 << 2;
      break;

    case 7:
      dashPattern << 16 << 4;
      break;

    case 8:
      dashPattern << 32 << 8;
      break;

    case 9:
      dashPattern << 10.5 << 4.5;
      break;

    case 10:
      dashPattern << 1 << 13.5 / 2;
      break;

    case 11:
      dashPattern << 4 << 8;
      break;

    case 12:
      dashPattern << 8 << 8;
      break;

    case 13:
      dashPattern << 16 << 16;
      break;

    case 14:
      dashPattern << 10 << 5 << 1 << 5;
      break;

    case 15:
      dashPattern << 18 << 3 << 1 << 3;
      break;

    case 16:
      dashPattern << 20 << 3 << 4 << 3;
      break;

    case 17:
      dashPattern << 32 << 12 << 6 << 12;
      break;

    case 18:
      dashPattern << 32 << 6 << 4 << 6 << 4 << 6;
      break;

    case 19:
      dashPattern << 32 << 6 << 4 << 6 << 4 << 6 << 4 << 6;
      break;

    case 20:
      dashPattern << 11 << 5 << 1 << 5 << 1 << 5;
      break;

    case 21:
      dashPattern << 20 << 4 << 1 << 4 << 1 << 4;
      break;

    case 22:
      dashPattern << 20 << 4 << 1 << 4 << 1 << 4 << 1 << 4;
      break;

    case 23:
      dashPattern << 6 << 2 << 1 << 2;
      break;

    case 24:
      dashPattern << 6 << 2 << 1 << 2 << 1 << 2;
      break;

    case 25:
      dashPattern << 10.5 << 2 << 1 << 2 << 4 << 2 << 1 << 2;
      break;

    case 32:
    case 33:
    case 34:
    case 35:
      dashPattern << 18 << 4;
      break;

    case 36:
      dashPattern << 7 << 4;
      break;

    case 37:
      dashPattern << 16 << 6;
      break;

    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
      break;

    case 39:
    case 40:
      dashPattern << 20 << 15;
      break;

    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
      break;

    case 47:
      dashPattern << 4 << 8;
      break;

    case 48:
    case 49:
    case 50:
    case 51:
      break;

    case 52:
    case 53:
      dashPattern << 15 << 4;
      break;

    case 54:
    case 55:
    case 56:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
      break;

    case 68:
      dashPattern << 10 << 5;
      break;

    case 69:
    case 70:
      break;

    case 71:
      dashPattern << 12 << 20;
      break;

    case 72:
      dashPattern << 20 << 8;
      break;

    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
      break;

    case 83:
    case 91:
    case 99:
    case 107:
      dashPattern << 0 << 4 << 1 << 4;
      patternOffset = 2;
      break;

    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 108:
    case 109:
      break;

    case 110:
    case 111:
    case 112:
    case 113:
      // these four are zig-zaggy patterns which can't be reproduced in QGIS!
      context.pushWarning( QObject::tr( "The line style is not supported in QGIS" ) );
      return nullptr;

    case 114:
    case 115:
    case 116:
      simpleLine->setWidth( simpleLine->width() * 2 );
      break;

    case 117:
    case 118:
      break;

    default:
      QgsDebugMsg( QStringLiteral( "Unknown line symbol identifier %1" ).arg( identifier ) );
      return nullptr;
  }

  if ( !dashPattern.isEmpty() )
  {
    // scale dash pattern -- sizes above expect a 1 pt width line
    for ( int i = 0; i < dashPattern.size() ; ++i )
      dashPattern[ i ] *= size;

    simpleLine->setCustomDashVector( dashPattern );
    simpleLine->setUseCustomDashPattern( true );
    simpleLine->setCustomDashPatternUnit( sizeUnit );

    simpleLine->setDashPatternOffset( patternOffset * size );
    simpleLine->setDashPatternOffsetUnit( sizeUnit );
  }

  std::unique_ptr< QgsLineSymbol > symbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << simpleLine.release() );

  if ( ( identifier >= 26 && identifier < 29 ) || ( identifier >= 31 && identifier < 34 ) || ( identifier >= 36 && identifier < 38 ) || ( identifier >= 47 && identifier <= 53 ) || identifier == 118 )
  {
    std::unique_ptr< QgsHashedLineSymbolLayer > hash = std::make_unique< QgsHashedLineSymbolLayer >();

    double spacing = 1;
    double offset = 1;
    double lineOffset = 0;
    double length = 3.5;
    switch ( identifier )
    {
      case 26:
        spacing = 10;
        offset = 5;
        length = 3.5;
        break;

      case 27:
        spacing = 16;
        offset = 5;
        length = 3.5;
        break;

      case 28:
      case 31:
        spacing = 24;
        offset = 5;
        length = 3.5;
        break;

      case 32:
      case 33:
        spacing = 22;
        offset = 9;
        length = 3.5;
        break;

      case 36:
        spacing = 11;
        offset = 0;
        length = 2;
        break;

      case 37:
        spacing = 22;
        offset = 0;
        length = 2;
        break;

      case 47:
        spacing = 12;
        offset = 0;
        length = 2;
        break;

      case 48:
        spacing = 3;
        offset = 0;
        lineOffset = -1.5;
        length = 3;
        break;

      case 49:
        spacing = 3;
        offset = 0;
        lineOffset = 1.5;
        length = 3;
        break;

      case 50:
        spacing = 6;
        offset = 0;
        lineOffset = -1.5;
        length = 3;
        break;

      case 51:
        spacing = 6;
        offset = 0;
        lineOffset = 1.5;
        length = 3;
        break;

      case 52:
        spacing = 19;
        offset = 5;
        lineOffset = -1;
        length = 2;
        break;

      case 53:
        spacing = 19;
        offset = 5;
        lineOffset = 1;
        length = 2;
        break;

      case 118:
        spacing = 5;
        offset = 0;
        lineOffset = 0;
        length = 8;
        break;

      default:
        break;
    }

    hash->setInterval( spacing * size );
    hash->setIntervalUnit( sizeUnit );

    hash->setOffset( lineOffset * size );
    hash->setOffsetUnit( sizeUnit );

    hash->setOffsetAlongLine( offset * size );
    hash->setOffsetAlongLineUnit( sizeUnit );

    hash->setHashLength( length * size );
    hash->setHashLengthUnit( sizeUnit );

    std::unique_ptr< QgsSimpleLineSymbolLayer > subSimpleLine = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, size );
    subSimpleLine->setWidthUnit( sizeUnit );
    subSimpleLine->setPenCapStyle( Qt::RoundCap );
    subSimpleLine->setPenJoinStyle( Qt::RoundJoin );

    std::unique_ptr< QgsLineSymbol > subSymbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << subSimpleLine.release() );
    hash->setSubSymbol( subSymbol.release() );

    if ( identifier == 31 || identifier == 33 )
    {
      std::unique_ptr< QgsHashedLineSymbolLayer > hash2( hash->clone() );
      hash->setOffsetAlongLine( hash->offsetAlongLine() - size );
      hash2->setOffsetAlongLine( hash2->offsetAlongLine() + size );
      symbol->appendSymbolLayer( hash2.release() );
    }
    else if ( identifier == 36 || identifier == 37 )
    {
      std::unique_ptr< QgsHashedLineSymbolLayer > hash2( hash->clone() );
      hash2->setOffsetAlongLine( dashPattern.at( 0 ) );
      symbol->appendSymbolLayer( hash2.release() );
    }
    else if ( identifier == 52 || identifier == 53 )
    {
      std::unique_ptr< QgsHashedLineSymbolLayer > hash2( hash->clone() );
      hash2->setOffsetAlongLine( hash->offsetAlongLine() * 2 );
      symbol->appendSymbolLayer( hash2.release() );
    }
    else if ( identifier == 118 )
    {
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setColor( QColor( 0, 0, 0 ) );
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setWidth( 0 );
      symbol->symbolLayer( 0 )->setLocked( true );

      std::unique_ptr<QgsSimpleLineSymbolLayer > secondRail( qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->clone() );
      const double offset = 2 * size;
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setOffset( offset );
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setOffsetUnit( sizeUnit );
      secondRail->setOffset( -offset );
      secondRail->setOffsetUnit( sizeUnit );

      secondRail->setLocked( true );
      symbol->appendSymbolLayer( secondRail.release() );
    }
    symbol->appendSymbolLayer( hash.release() );
  }
  else if ( ( identifier >= 29 && identifier < 31 ) || ( identifier >= 34 && identifier < 36 ) )
  {
    double spacing = 1;
    double offset = 1;
    switch ( identifier )
    {
      case 29:
      case 30:
        spacing = 10;
        offset = 5;
        break;

      case 34:
      case 35:
        spacing = 22;
        offset = 9;
        break;

      default:
        break;
    }

    std::unique_ptr< QgsHashedLineSymbolLayer > hash = std::make_unique< QgsHashedLineSymbolLayer >();
    hash->setInterval( spacing * size * 2 );
    hash->setIntervalUnit( sizeUnit );

    hash->setOffsetAlongLine( offset * size );
    hash->setOffsetAlongLineUnit( sizeUnit );

    hash->setHashLength( 3.5 * size * 0.5 );
    hash->setHashLengthUnit( sizeUnit );

    std::unique_ptr< QgsSimpleLineSymbolLayer > subSimpleLine = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, size );
    subSimpleLine->setWidthUnit( sizeUnit );
    subSimpleLine->setPenCapStyle( Qt::RoundCap );
    subSimpleLine->setPenJoinStyle( Qt::RoundJoin );

    std::unique_ptr< QgsLineSymbol > subSymbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << subSimpleLine.release() );
    hash->setSubSymbol( subSymbol.release() );
    std::unique_ptr< QgsHashedLineSymbolLayer > hash2( hash->clone() );

    hash->setOffset( -hash->hashLength() );
    hash->setOffsetUnit( hash->hashLengthUnit() );
    hash2->setOffset( hash->hashLength() );
    hash2->setOffsetUnit( hash->hashLengthUnit() );
    hash2->setOffsetAlongLine( hash2->offsetAlongLine() + hash2->interval() * 0.5 );

    switch ( identifier )
    {
      case 29:
      case 34:
        symbol->appendSymbolLayer( hash.release() );
        symbol->appendSymbolLayer( hash2.release() );
        break;

      case 30:
      case 35:
      {
        std::unique_ptr< QgsHashedLineSymbolLayer > hash3( hash->clone() );
        std::unique_ptr< QgsHashedLineSymbolLayer > hash4( hash2->clone() );

        hash->setOffsetAlongLine( hash->offsetAlongLine() - size );
        hash3->setOffsetAlongLine( hash3->offsetAlongLine() + size );
        hash2->setOffsetAlongLine( hash2->offsetAlongLine() - size );
        hash4->setOffsetAlongLine( hash4->offsetAlongLine() + size );

        symbol->appendSymbolLayer( hash.release() );
        symbol->appendSymbolLayer( hash2.release() );
        symbol->appendSymbolLayer( hash3.release() );
        symbol->appendSymbolLayer( hash4.release() );
        break;
      }
    }
  }
  else if ( ( identifier >= 38 && identifier < 41 ) || ( identifier >= 54 && identifier <= 61 ) || ( identifier >= 78 && identifier <= 109 ) || ( identifier >= 114 && identifier <= 117 ) )
  {
    std::unique_ptr< QgsMarkerLineSymbolLayer > marker = std::make_unique< QgsMarkerLineSymbolLayer >();

    double spacing = 1;
    double offset = 1;
    double markerSize = 1;
    double angle = 0;
    double lineOffset = 0;
    Qgis::MarkerLinePlacement placement = Qgis::MarkerLinePlacement::Interval;
    Qgis::MarkerShape shape = Qgis::MarkerShape::Circle;
    switch ( identifier )
    {
      case 38:
        spacing = 35;
        offset = 25;
        markerSize = 3;
        shape = Qgis::MarkerShape::Cross2;
        break;

      case 39:
        spacing = 35;
        offset = 27.5;
        markerSize = 3;
        shape = Qgis::MarkerShape::Cross2;
        break;

      case 40:
        spacing = 35;
        offset = 27.5;
        markerSize = 3.2;
        shape = Qgis::MarkerShape::Cross;
        break;

      case 54:
        spacing = 12;
        offset = 4;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        break;

      case 55:
        spacing = 12;
        offset = 0;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        angle = 180;
        break;

      case 56:
        spacing = 31;
        offset = 4;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        angle = 180;
        break;

      case 57:
        spacing = 10;
        offset = 4;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        break;

      case 58:
        spacing = 10;
        offset = 0;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        angle = 180;
        break;

      case 59:
      case 61:
        offset = 0;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        placement = Qgis::MarkerLinePlacement::LastVertex;
        break;

      case 60:
        offset = 0;
        markerSize = 6;
        shape = Qgis::MarkerShape::ArrowHead;
        placement = Qgis::MarkerLinePlacement::FirstVertex;
        angle = 180;
        break;

      case 78:
      case 80:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Octagon;
        placement = Qgis::MarkerLinePlacement::FirstVertex;
        angle = 0;
        break;

      case 79:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Octagon;
        placement = Qgis::MarkerLinePlacement::LastVertex;
        angle = 0;
        break;

      case 81:
      case 82:
      case 83:
      case 84:
      case 85:
        spacing = 9;
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Octagon;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        break;

      case 86:
      case 88:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Square;
        placement = Qgis::MarkerLinePlacement::FirstVertex;
        angle = 0;
        break;

      case 87:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Square;
        placement = Qgis::MarkerLinePlacement::LastVertex;
        angle = 0;
        break;

      case 89:
      case 90:
      case 91:
      case 92:
      case 93:
        spacing = 9;
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Square;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        break;

      case 94:
      case 96:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::EquilateralTriangle;
        placement = Qgis::MarkerLinePlacement::FirstVertex;
        angle = 0;
        break;

      case 95:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::EquilateralTriangle;
        placement = Qgis::MarkerLinePlacement::LastVertex;
        angle = 180;
        break;

      case 97:
      case 98:
      case 99:
      case 100:
      case 101:
        spacing = 9;
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Diamond;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        break;

      case 102:
      case 104:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Diamond;
        placement = Qgis::MarkerLinePlacement::FirstVertex;
        angle = 0;
        break;

      case 103:
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Diamond;
        placement = Qgis::MarkerLinePlacement::LastVertex;
        angle = 180;
        break;

      case 105:
      case 106:
      case 107:
      case 108:
      case 109:
        spacing = 9;
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Diamond;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        break;

      case 114:
        spacing = 18;
        offset = 9;
        markerSize = 8;
        shape = Qgis::MarkerShape::SemiCircle;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        lineOffset = -0.8;
        break;

      case 115:
        spacing = 16;
        offset = 8;
        markerSize = 8;
        shape = Qgis::MarkerShape::EquilateralTriangle;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        lineOffset = -2;
        break;

      case 116:
        spacing = 23;
        offset = 8;
        markerSize = 8;
        shape = Qgis::MarkerShape::SemiCircle;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        lineOffset = -0.8;
        break;

      case 117:
        spacing = 9;
        offset = 2;
        markerSize = 4;
        shape = Qgis::MarkerShape::Square;
        placement = Qgis::MarkerLinePlacement::Interval;
        angle = 0;
        lineOffset = -2;
        break;

      default:
        break;
    }

    if ( identifier >= 78 && identifier <= 109 )
    {
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setColor( QColor( 0, 0, 0 ) );
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setWidth( 0 );
      symbol->symbolLayer( 0 )->setLocked( true );

      if ( ( identifier >= 84 && identifier <= 85 ) || ( identifier >= 92 && identifier <= 93 ) || ( identifier >= 100 && identifier <= 101 )  || ( identifier >= 108 && identifier <= 109 ) )
      {
        std::unique_ptr<QgsSimpleLineSymbolLayer > secondRail( qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->clone() );

        double offset = 2 * size;
        if ( identifier == 85 || identifier == 93 || identifier == 101 || identifier == 109 )
          offset = 3 * size;

        qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setOffset( offset );
        qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setOffsetUnit( sizeUnit );
        secondRail->setOffset( -offset );
        secondRail->setOffsetUnit( sizeUnit );

        secondRail->setLocked( true );
        symbol->appendSymbolLayer( secondRail.release() );
      }
    }

    marker->setPlacements( placement );
    marker->setInterval( spacing * size );
    marker->setIntervalUnit( sizeUnit );

    marker->setOffsetAlongLine( offset * size );
    marker->setOffsetAlongLineUnit( sizeUnit );

    marker->setOffset( lineOffset * size );
    marker->setOffsetUnit( sizeUnit );

    std::unique_ptr< QgsSimpleMarkerSymbolLayer > subSimpleMarker = std::make_unique< QgsSimpleMarkerSymbolLayer >( shape, markerSize * size );
    subSimpleMarker->setColor( foreColor );
    subSimpleMarker->setSizeUnit( sizeUnit );
    subSimpleMarker->setStrokeWidth( size );
    subSimpleMarker->setStrokeWidthUnit( sizeUnit );
    subSimpleMarker->setAngle( angle );

    subSimpleMarker->setPenJoinStyle( Qt::RoundJoin );
    subSimpleMarker->setPenCapStyle( Qt::RoundCap );

    if ( shape == Qgis::MarkerShape::Octagon
         || shape == Qgis::MarkerShape::Square
         || shape == Qgis::MarkerShape::EquilateralTriangle
         || shape == Qgis::MarkerShape::Diamond
         || shape == Qgis::MarkerShape::SemiCircle )
    {
      subSimpleMarker->setStrokeStyle( Qt::NoPen );
    }

    std::unique_ptr< QgsMarkerSymbol > subSymbol = std::make_unique< QgsMarkerSymbol >( QgsSymbolLayerList() << subSimpleMarker.release() );
    marker->setSubSymbol( subSymbol.release() );

    if ( identifier == 56 )
    {
      std::unique_ptr< QgsMarkerLineSymbolLayer > marker2( marker->clone() );
      marker2->setOffsetAlongLine( 19 * size );
      qgis::down_cast< QgsMarkerSymbol * >( marker2->subSymbol() )->setAngle( 0 );
      symbol->appendSymbolLayer( marker2.release() );
    }
    else if ( identifier == 61 )
    {
      std::unique_ptr< QgsMarkerLineSymbolLayer > marker2( marker->clone() );
      marker2->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
      qgis::down_cast< QgsMarkerSymbol * >( marker2->subSymbol() )->setAngle( 180 );
      symbol->appendSymbolLayer( marker2.release() );
    }
    else if ( identifier == 80 || identifier == 88 || identifier == 96 || identifier == 104 )
    {
      std::unique_ptr< QgsMarkerLineSymbolLayer > marker2( marker->clone() );
      marker2->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
      qgis::down_cast< QgsMarkerSymbol * >( marker2->subSymbol() )->setAngle( 180 );
      symbol->appendSymbolLayer( marker2.release() );
    }

    if ( identifier == 116 )
    {
      std::unique_ptr< QgsMarkerLineSymbolLayer > marker2( marker->clone() );

      qgis::down_cast< QgsSimpleMarkerSymbolLayer * >( marker2->subSymbol()->symbolLayer( 0 ) )->setShape( Qgis::MarkerShape::EquilateralTriangle );
      marker2->setOffsetAlongLine( 16 * size );
      marker2->setOffset( -1.5 * size );
      symbol->appendSymbolLayer( marker2.release() );
    }

    symbol->appendSymbolLayer( marker.release() );
  }
  else if ( identifier >= 41 && identifier < 45 )
  {
    const int count = identifier - 40;
    QgsSimpleLineSymbolLayer *simpleLine = dynamic_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) );
    simpleLine->setCustomDashVector( QVector< qreal >() <<  0 << 5.25 * size << 4 * size << ( 3.25 * size  + ( count - 1 ) * ( 7.25 * size ) ) );
    simpleLine->setCustomDashPatternUnit( sizeUnit );
    simpleLine->setUseCustomDashPattern( true );

    for ( int i = 1 ; i < count; ++i )
    {
      std::unique_ptr< QgsSimpleLineSymbolLayer > dashLine( simpleLine->clone() );

      dashLine->setCustomDashVector( QVector< qreal >() <<  0 << 5.25 * size + ( i * 7.25 * size ) << 4 * size << ( 3.25 * size  + ( count - 1 - i ) * ( 7.25 * size ) ) );
      symbol->appendSymbolLayer( dashLine.release() );
    }

    std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine2 = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, 1.6 * size );
    simpleLine2->setWidthUnit( sizeUnit );
    simpleLine2->setPenCapStyle( Qt::RoundCap );
    simpleLine2->setPenJoinStyle( Qt::RoundJoin );

    simpleLine2->setCustomDashVector( QVector< qreal >() << 2 * size << 10.5 * size + ( count - 1 ) * ( 7.25 * size ) );
    simpleLine2->setUseCustomDashPattern( true );
    simpleLine2->setCustomDashPatternUnit( sizeUnit );

    symbol->appendSymbolLayer( simpleLine2.release() );
  }
  else if ( identifier == 45 )
  {
    std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine2 = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, 1.6 * size );
    simpleLine2->setWidthUnit( sizeUnit );
    simpleLine2->setPenCapStyle( Qt::RoundCap );
    simpleLine2->setPenJoinStyle( Qt::RoundJoin );

    simpleLine2->setCustomDashVector( QVector< qreal >() << 0 << 2 * size << 1.25 * size << 6.5 * size );
    simpleLine2->setUseCustomDashPattern( true );
    simpleLine2->setCustomDashPatternUnit( sizeUnit );

    symbol->appendSymbolLayer( simpleLine2.release() );
  }
  else if ( identifier == 46 )
  {
    std::unique_ptr< QgsHashedLineSymbolLayer > hashLine = std::make_unique< QgsHashedLineSymbolLayer >();

    hashLine->setInterval( 4 * size );
    hashLine->setIntervalUnit( sizeUnit );
    hashLine->setOffsetAlongLine( 2 * size );
    hashLine->setOffsetAlongLineUnit( sizeUnit );
    hashLine->setHashLength( 3.8 * size );
    hashLine->setHashLengthUnit( sizeUnit );

    hashLine->setSubSymbol( symbol.release() );

    symbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << hashLine.release() );
  }
  else if ( identifier == 62 )
  {
    std::unique_ptr< QgsMarkerLineSymbolLayer > markerLine = std::make_unique< QgsMarkerLineSymbolLayer >();
    markerLine->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
    markerLine->setOffsetAlongLine( 2 * size );
    markerLine->setOffsetAlongLineUnit( sizeUnit );

    std::unique_ptr< QgsSimpleMarkerSymbolLayer > subSimpleMarker = std::make_unique< QgsSimpleMarkerSymbolLayer >( Qgis::MarkerShape::Line, size * 4 );
    subSimpleMarker->setColor( foreColor );
    subSimpleMarker->setSizeUnit( sizeUnit );
    subSimpleMarker->setStrokeWidth( 1.25 * size );
    subSimpleMarker->setStrokeWidthUnit( sizeUnit );
    subSimpleMarker->setAngle( 90 );

    subSimpleMarker->setPenJoinStyle( Qt::RoundJoin );
    subSimpleMarker->setPenCapStyle( Qt::RoundCap );

    std::unique_ptr< QgsMarkerSymbol > subSymbol = std::make_unique< QgsMarkerSymbol >( QgsSymbolLayerList() << subSimpleMarker.release() );
    markerLine->setSubSymbol( subSymbol.release() );

    symbol->appendSymbolLayer( markerLine.release() );
  }
  else if ( ( identifier >= 63 && identifier <= 69 ) || ( identifier >= 72 && identifier <= 77 ) )
  {
    std::unique_ptr< QgsSimpleLineSymbolLayer > upperLine( qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->clone() );
    upperLine->setUseCustomDashPattern( false );

    if ( identifier < 65  || ( identifier >= 68 && identifier <= 69 ) || identifier == 73 )
    {
      upperLine->setColor( QColor( 255, 255, 255 ) );
      upperLine->setLocked( true );
    }
    else if ( identifier < 67 || identifier == 72 || identifier == 75 || identifier == 76 )
    {
      qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setColor( QColor( 0, 0, 0 ) );
      symbol->symbolLayer( 0 )->setLocked( true );
    }
    else if ( identifier <= 69 || identifier == 77 )
    {
      upperLine->setColor( QColor( 0, 0, 0 ) );
      upperLine->setLocked( true );
    }
    upperLine->setWidth( upperLine->width() * 0.9 );
    if ( interleaved )
    {
      upperLine->setRenderingPass( 1 );
    }

    if ( identifier >= 73 && identifier <= 75 )
    {
      upperLine->setCustomDashVector( QVector< qreal >() << 0 << 10 * size << 12 * size << 2 * size );
      upperLine->setUseCustomDashPattern( true );
      upperLine->setCustomDashPatternUnit( sizeUnit );
    }
    else if ( identifier == 76 )
    {
      upperLine->setCustomDashVector( QVector< qreal >() << 0 << 10 * size << 24 * size << 14 * size );
      upperLine->setUseCustomDashPattern( true );
      upperLine->setCustomDashPatternUnit( sizeUnit );
    }

    if ( identifier == 75 || identifier == 76 )
    {
      std::unique_ptr< QgsSimpleLineSymbolLayer > middleLine = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 255, 255, 255 ), upperLine->width() );
      middleLine->setWidthUnit( sizeUnit );
      middleLine->setLocked( true );
      middleLine->setPenCapStyle( Qt::RoundCap );
      middleLine->setPenJoinStyle( Qt::RoundJoin );

      if ( interleaved )
      {
        middleLine->setRenderingPass( 1 );
        upperLine->setRenderingPass( 2 );
      }
      symbol->appendSymbolLayer( middleLine.release() );
    }

    symbol->appendSymbolLayer( upperLine.release() );

    if ( identifier == 64 || identifier == 66 )
    {
      std::unique_ptr< QgsSimpleLineSymbolLayer > middleLine = std::make_unique< QgsSimpleLineSymbolLayer >( identifier == 64 ? foreColor : QColor( 0, 0, 0 ), 0 );
      if ( identifier == 66 )
        middleLine->setLocked( true );

      if ( interleaved )
      {
        middleLine->setRenderingPass( 2 );
      }
      symbol->appendSymbolLayer( middleLine.release() );
    }

    else if ( identifier == 69 )
    {
      std::unique_ptr< QgsHashedLineSymbolLayer > hashedLine = std::make_unique< QgsHashedLineSymbolLayer >();

      std::unique_ptr< QgsSimpleLineSymbolLayer > middleLine = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, 0 );
      hashedLine->setSubSymbol( new QgsLineSymbol( { middleLine.release() } ) );
      hashedLine->setInterval( 18 * size );
      hashedLine->setIntervalUnit( sizeUnit );
      hashedLine->setOffsetAlongLine( 4 * size );
      hashedLine->setOffsetAlongLineUnit( sizeUnit );
      hashedLine->setHashLength( 8 * size );
      hashedLine->setHashLengthUnit( sizeUnit );

      if ( interleaved )
      {
        hashedLine->setRenderingPass( 2 );
      }
      symbol->appendSymbolLayer( hashedLine.release() );
    }
    else if ( identifier == 77 )
    {
      std::unique_ptr< QgsSimpleLineSymbolLayer > middleLine = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 255, 255, 255 ), qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 1 ) )->width() );
      middleLine->setWidthUnit( sizeUnit );
      middleLine->setLocked( true );
      middleLine->setPenCapStyle( Qt::RoundCap );
      middleLine->setPenJoinStyle( Qt::RoundJoin );
      middleLine->setCustomDashVector( QVector< qreal >() << 0 << 10 * size << 12 * size << 2 * size );
      middleLine->setUseCustomDashPattern( true );
      middleLine->setCustomDashPatternUnit( sizeUnit );
      if ( interleaved )
      {
        middleLine->setRenderingPass( 2 );
      }
      symbol->appendSymbolLayer( middleLine.release() );
    }
  }
  else if ( identifier >= 70 && identifier <= 71 )
  {
    qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setColor( QColor( 0, 0, 0 ) );
    qgis::down_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->setLocked( true );

    std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine2 = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, size );
    simpleLine2->setWidthUnit( sizeUnit );
    simpleLine2->setPenCapStyle( Qt::RoundCap );
    simpleLine2->setPenJoinStyle( Qt::RoundJoin );

    if ( identifier == 70 )
      simpleLine2->setCustomDashVector( QVector< qreal >() << 0 << 12 * size << 12 * size << 0 );
    else if ( identifier == 71 )
      simpleLine2->setCustomDashVector( QVector< qreal >() << 0 << 16 * size << 12 * size << 4 * size );

    simpleLine2->setUseCustomDashPattern( true );
    simpleLine2->setCustomDashPatternUnit( sizeUnit );

    symbol->appendSymbolLayer( simpleLine2.release() );
  }

  return symbol.release();
}

QgsFillSymbol *QgsMapInfoSymbolConverter::convertFillSymbol( int identifier, QgsMapInfoSymbolConversionContext &context, const QColor &foreColor, const QColor &backColor )
{
  Qt::BrushStyle style = Qt::SolidPattern;

  bool useLineFill = false;
  bool crossFill = false;
  double lineAngle = 0;
  double lineWidth = 0;
  double lineSpacing = 1;
  switch ( identifier )
  {
    case 0:
    case 1:
      style = Qt::NoBrush;
      break;

    case 2:
      style = Qt::SolidPattern;
      break;

    case 3:
    case 19:
      style = Qt::HorPattern;
      break;

    case 4:
    case 24:
      style = Qt::VerPattern;
      break;

    case 5:
    case 34:
      style = Qt::FDiagPattern;
      break;

    case 6:
    case 29:
      style = Qt::BDiagPattern;
      break;

    case 7:
    case 39:
      style = Qt::CrossPattern;
      break;

    case 8:
    case 44:
      style = Qt::DiagCrossPattern;
      break;

    case 12:
      style = Qt::Dense1Pattern;
      break;

    case 13:
      style = Qt::Dense2Pattern;
      break;

    case 14:
      style = Qt::Dense3Pattern;
      break;

    case 15:
      style = Qt::Dense4Pattern;
      break;

    case 16:
      style = Qt::Dense5Pattern;
      break;

    case 17:
      style = Qt::Dense6Pattern;
      break;

    case 18:
      style = Qt::Dense7Pattern;
      break;

    case 20:
      useLineFill = true;
      lineAngle = 0;
      lineSpacing = 6;
      lineWidth = 1.2;
      break;

    case 21:
      useLineFill = true;
      lineAngle = 0;
      lineSpacing = 4;
      lineWidth = 0.8;
      break;

    case 22:
      useLineFill = true;
      lineAngle = 0;
      lineSpacing = 3.4;
      lineWidth = 1.2;
      break;

    case 23:
      useLineFill = true;
      lineAngle = 0;
      lineSpacing = 3.0;
      lineWidth = 1.0;
      break;

    case 25:
      useLineFill = true;
      lineAngle = 90;
      lineSpacing = 6;
      lineWidth = 1.2;
      break;

    case 26:
      useLineFill = true;
      lineAngle = 90;
      lineSpacing = 4;
      lineWidth = 0.8;
      break;

    case 27:
      useLineFill = true;
      lineAngle = 90;
      lineSpacing = 3.4;
      lineWidth = 1.2;
      break;

    case 28:
      useLineFill = true;
      lineAngle = 90;
      lineSpacing = 3.0;
      lineWidth = 1.0;
      break;

    case 30:
      useLineFill = true;
      lineAngle = 45;
      lineSpacing = 6;
      lineWidth = 1.2;
      break;

    case 31:
      useLineFill = true;
      lineAngle = 45;
      lineSpacing = 4;
      lineWidth = 0.8;
      break;

    case 32:
      useLineFill = true;
      lineAngle = 45;
      lineSpacing = 3.4;
      lineWidth = 1.2;
      break;

    case 33:
      useLineFill = true;
      lineAngle = 45;
      lineSpacing = 3.0;
      lineWidth = 1.0;
      break;

    case 35:
      useLineFill = true;
      lineAngle = 135;
      lineSpacing = 6;
      lineWidth = 1.2;
      break;

    case 36:
      useLineFill = true;
      lineAngle = 135;
      lineSpacing = 4;
      lineWidth = 0.8;
      break;

    case 37:
      useLineFill = true;
      lineAngle = 135;
      lineSpacing = 3.4;
      lineWidth = 1.2;
      break;

    case 38:
      useLineFill = true;
      lineAngle = 135;
      lineSpacing = 3.0;
      lineWidth = 1.0;
      break;

    case 40:
      useLineFill = true;
      crossFill = true;
      lineAngle = 0;
      lineSpacing = 6;
      lineWidth = 1.2;
      break;

    case 41:
      useLineFill = true;
      crossFill = true;
      lineAngle = 0;
      lineSpacing = 4;
      lineWidth = 0.8;
      break;

    case 42:
      useLineFill = true;
      crossFill = true;
      lineAngle = 0;
      lineSpacing = 3.4;
      lineWidth = 1.2;
      break;

    case 43:
      useLineFill = true;
      crossFill = true;
      lineAngle = 0;
      lineSpacing = 3.0;
      lineWidth = 1.0;
      break;

    case 45:
      useLineFill = true;
      crossFill = true;
      lineAngle = 45;
      lineSpacing = 6;
      lineWidth = 1.2;
      break;

    case 46:
      useLineFill = true;
      crossFill = true;
      lineAngle = 45;
      lineSpacing = 4;
      lineWidth = 0.8;
      break;

    case 47:
      useLineFill = true;
      crossFill = true;
      lineAngle = 45;
      lineSpacing = 3.4;
      lineWidth = 1.2;
      break;

    default:
      context.pushWarning( QObject::tr( "The brush style is not supported in QGIS" ) );
      return nullptr;
  }

  QgsSymbolLayerList layers;
  if ( backColor.isValid() && style != Qt::SolidPattern && ( useLineFill || style != Qt::NoBrush ) )
  {
    std::unique_ptr< QgsSimpleFillSymbolLayer > backgroundFill = std::make_unique< QgsSimpleFillSymbolLayer >( backColor );
    backgroundFill->setLocked( true );
    backgroundFill->setStrokeStyle( Qt::NoPen );
    layers << backgroundFill.release();
  }

  if ( !useLineFill )
  {
    std::unique_ptr< QgsSimpleFillSymbolLayer > foregroundFill = std::make_unique< QgsSimpleFillSymbolLayer >( foreColor );
    foregroundFill->setBrushStyle( style );
    foregroundFill->setStrokeStyle( Qt::NoPen );
    layers << foregroundFill.release();
  }
  else
  {
    std::unique_ptr< QgsLinePatternFillSymbolLayer > lineFill = std::make_unique< QgsLinePatternFillSymbolLayer >();

    std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine = std::make_unique< QgsSimpleLineSymbolLayer >( foreColor, lineWidth );
    simpleLine->setWidthUnit( QgsUnitTypes::RenderPoints );
    lineFill->setSubSymbol( new QgsLineSymbol( QgsSymbolLayerList() << simpleLine.release() ) );

    lineFill->setDistance( lineSpacing );
    lineFill->setDistanceUnit( QgsUnitTypes::RenderPoints );
    lineFill->setLineAngle( lineAngle );

    if ( crossFill )
    {
      std::unique_ptr< QgsLinePatternFillSymbolLayer > lineFill2( lineFill->clone() );
      lineFill2->setLineAngle( lineFill->lineAngle() + 90 );
      layers << lineFill2.release();
    }

    layers << lineFill.release();
  }
  return new QgsFillSymbol( layers );
}

QgsMarkerSymbol *QgsMapInfoSymbolConverter::convertMarkerSymbol( int identifier, QgsMapInfoSymbolConversionContext &context, const QColor &color, double size, QgsUnitTypes::RenderUnit sizeUnit )
{
  Qgis::MarkerShape shape;
  bool isFilled = true;
  bool isNull = false;
  bool hasShadow = false;
  double angle = 0;
  QgsMarkerSymbolLayer::VerticalAnchorPoint vertAlign = QgsMarkerSymbolLayer::VCenter;
  QPointF shadowOffset;
  switch ( identifier )
  {
    case 31:
      // null symbol
      shape = Qgis::MarkerShape::Square; // to initialize the variable
      isNull = true;
      break;

    case 32:
      shape = Qgis::MarkerShape::Square;
      break;

    case 33:
      shape = Qgis::MarkerShape::Diamond;
      break;

    case 34:
      shape = Qgis::MarkerShape::Circle;
      break;

    case 35:
      shape = Qgis::MarkerShape::Star;
      break;

    case 36:
      shape = Qgis::MarkerShape::Triangle;
      break;

    case 37:
      shape = Qgis::MarkerShape::Triangle;
      angle = 180;
      break;

    case 38:
      shape = Qgis::MarkerShape::Square;
      isFilled = false;
      break;

    case 39:
      shape = Qgis::MarkerShape::Diamond;
      isFilled = false;
      break;

    case 40:
      shape = Qgis::MarkerShape::Circle;
      isFilled = false;
      break;

    case 41:
      shape = Qgis::MarkerShape::Star;
      isFilled = false;
      break;

    case 42:
      shape = Qgis::MarkerShape::Triangle;
      isFilled = false;
      break;

    case 43:
      shape = Qgis::MarkerShape::Triangle;
      angle = 180;
      isFilled = false;
      break;

    case 44:
      shape = Qgis::MarkerShape::Square;
      hasShadow = true;
      shadowOffset = QPointF( size * 0.1, size * 0.1 );
      break;

    case 45:
      shape = Qgis::MarkerShape::Triangle;
      shadowOffset = QPointF( size * 0.2, size * 0.1 );
      hasShadow = true;
      break;

    case 46:
      shape = Qgis::MarkerShape::Circle;
      shadowOffset = QPointF( size * 0.1, size * 0.1 );
      hasShadow = true;
      break;

    case 47:
      shape = Qgis::MarkerShape::Arrow;
      size *= 0.66666;
      angle = 45;
      vertAlign = QgsMarkerSymbolLayer::Top;
      break;

    case 48:
      shape = Qgis::MarkerShape::Arrow;
      size *= 0.66666;
      angle = 225;
      vertAlign = QgsMarkerSymbolLayer::Top;
      break;

    case 49:
      shape = Qgis::MarkerShape::Cross;
      break;

    case 50:
      shape = Qgis::MarkerShape::Cross2;
      break;

    case 51:
      shape = Qgis::MarkerShape::Cross;
      break;

    default:
      context.pushWarning( QObject::tr( "The symbol is not supported in QGIS" ) );
      return nullptr;
  }

  std::unique_ptr< QgsSimpleMarkerSymbolLayer > simpleMarker = std::make_unique< QgsSimpleMarkerSymbolLayer >( shape, size );
  simpleMarker->setSizeUnit( sizeUnit );
  simpleMarker->setAngle( angle );
  simpleMarker->setVerticalAnchorPoint( vertAlign );

  if ( isNull )
  {
    simpleMarker->setFillColor( QColor( 0, 0, 0, 0 ) );
    simpleMarker->setStrokeStyle( Qt::NoPen );
  }
  else if ( isFilled && QgsSimpleMarkerSymbolLayer::shapeIsFilled( shape ) )
  {
    simpleMarker->setColor( color );
    simpleMarker->setStrokeColor( QColor( 0, 0, 0 ) );
    simpleMarker->setStrokeWidth( 0 );
  }
  else
  {
    simpleMarker->setFillColor( QColor( 0, 0, 0, 0 ) );
    simpleMarker->setStrokeColor( color );
  }

  QgsSymbolLayerList symbols;
  if ( hasShadow )
  {
    std::unique_ptr< QgsSimpleMarkerSymbolLayer > shadow( simpleMarker->clone() );
    shadow->setColor( QColor( 0, 0, 0 ) );
    shadow->setLocked( true );
    shadow->setOffset( shadowOffset );
    shadow->setOffsetUnit( sizeUnit );

    symbols << shadow.release();
    symbols << simpleMarker.release();
  }
  else
  {
    if ( identifier == 51 )
    {
      std::unique_ptr< QgsSimpleMarkerSymbolLayer > second( simpleMarker->clone() );
      second->setShape( Qgis::MarkerShape::Cross2 );
      symbols << second.release();
    }
    symbols << simpleMarker.release();
  }

  return new QgsMarkerSymbol( symbols );
}
