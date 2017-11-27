/***************************************************************************
                             qgslayoutgridsettings.cpp
                             -------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutgridsettings.h"
#include "qgsreadwritecontext.h"
#include "qgslayout.h"
#include "qgsproject.h"

QgsLayoutGridSettings::QgsLayoutGridSettings( QgsLayout *layout )
  : mGridResolution( QgsLayoutMeasurement( 10 ) )
  , mLayout( layout )
{
  mGridPen = QPen( QColor( 190, 190, 190, 100 ), 0 );
  mGridPen.setCosmetic( true );
}

QgsLayout *QgsLayoutGridSettings::layout()
{
  return mLayout;
}

void QgsLayoutGridSettings::setResolution( const QgsLayoutMeasurement &resolution )
{
  mLayout->undoStack()->beginCommand( this, QObject::tr( "Change Grid Resolution" ), UndoGridResolution );
  mGridResolution = resolution;
  mLayout->undoStack()->endCommand();
}

void QgsLayoutGridSettings::setOffset( const QgsLayoutPoint offset )
{
  mLayout->undoStack()->beginCommand( this, QObject::tr( "Change Grid Offset" ), UndoGridOffset );
  mGridOffset = offset;
  mLayout->undoStack()->endCommand();
}

bool QgsLayoutGridSettings::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "Grid" ) );

  element.setAttribute( QStringLiteral( "resolution" ), mGridResolution.length() );
  element.setAttribute( QStringLiteral( "resUnits" ), QgsUnitTypes::encodeUnit( mGridResolution.units() ) );

  element.setAttribute( QStringLiteral( "offsetX" ), mGridOffset.x() );
  element.setAttribute( QStringLiteral( "offsetY" ), mGridOffset.y() );
  element.setAttribute( QStringLiteral( "offsetUnits" ), QgsUnitTypes::encodeUnit( mGridOffset.units() ) );

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutGridSettings::readXml( const QDomElement &e, const QDomDocument &, const QgsReadWriteContext & )
{
  QDomElement element = e;
  if ( element.nodeName() != QStringLiteral( "Grid" ) )
  {
    element = element.firstChildElement( QStringLiteral( "Grid" ) );
  }

  if ( element.nodeName() != QStringLiteral( "Grid" ) )
  {
    return false;
  }

  double res = element.attribute( QStringLiteral( "resolution" ), QStringLiteral( "10" ) ).toDouble();
  QgsUnitTypes::LayoutUnit resUnit = QgsUnitTypes::decodeLayoutUnit( element.attribute( QStringLiteral( "resUnits" ) ) );
  mGridResolution = QgsLayoutMeasurement( res, resUnit );

  double offsetX = element.attribute( QStringLiteral( "offsetX" ) ).toDouble();
  double offsetY = element.attribute( QStringLiteral( "offsetY" ) ).toDouble();
  QgsUnitTypes::LayoutUnit offsetUnit = QgsUnitTypes::decodeLayoutUnit( element.attribute( QStringLiteral( "offsetUnits" ) ) );
  mGridOffset = QgsLayoutPoint( offsetX, offsetY, offsetUnit );

  mLayout->pageCollection()->redraw();

  return true;
}

