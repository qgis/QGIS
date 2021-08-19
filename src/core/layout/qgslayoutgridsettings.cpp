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
#include "qgslayoutundostack.h"
#include "qgslayoutpagecollection.h"
#include "qgssettings.h"

QgsLayoutGridSettings::QgsLayoutGridSettings( QgsLayout *layout )
  : mGridResolution( QgsLayoutMeasurement( 10 ) )
  , mLayout( layout )
{
  mGridPen = QPen( QColor( 190, 190, 190, 100 ), 0 );
  mGridPen.setCosmetic( true );
  loadFromSettings();
}

QgsLayout *QgsLayoutGridSettings::layout()
{
  return mLayout;
}

void QgsLayoutGridSettings::setResolution( QgsLayoutMeasurement resolution )
{
  mLayout->undoStack()->beginCommand( this, QObject::tr( "Change Grid Resolution" ), UndoGridResolution );
  mGridResolution = resolution;
  mLayout->undoStack()->endCommand();
}

void QgsLayoutGridSettings::setOffset( const QgsLayoutPoint &offset )
{
  mLayout->undoStack()->beginCommand( this, QObject::tr( "Change Grid Offset" ), UndoGridOffset );
  mGridOffset = offset;
  mLayout->undoStack()->endCommand();
}

void QgsLayoutGridSettings::loadFromSettings()
{
  //read grid style, grid color and pen width from settings
  const QgsSettings s;

  QString gridStyleString;
  gridStyleString = s.value( QStringLiteral( "LayoutDesigner/gridStyle" ), "Dots", QgsSettings::Gui ).toString();

  int gridRed, gridGreen, gridBlue, gridAlpha;
  gridRed = s.value( QStringLiteral( "LayoutDesigner/gridRed" ), 190, QgsSettings::Gui ).toInt();
  gridGreen = s.value( QStringLiteral( "LayoutDesigner/gridGreen" ), 190, QgsSettings::Gui ).toInt();
  gridBlue = s.value( QStringLiteral( "LayoutDesigner/gridBlue" ), 190, QgsSettings::Gui ).toInt();
  gridAlpha = s.value( QStringLiteral( "LayoutDesigner/gridAlpha" ), 100, QgsSettings::Gui ).toInt();
  const QColor gridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );

  mGridPen.setColor( gridColor );
  mGridPen.setWidthF( 0 );
  mGridPen.setCosmetic( true );

  if ( gridStyleString == QLatin1String( "Dots" ) )
  {
    mGridStyle = StyleDots;
  }
  else if ( gridStyleString == QLatin1String( "Crosses" ) )
  {
    mGridStyle = StyleCrosses;
  }
  else
  {
    mGridStyle = StyleLines;
  }

  mGridResolution = QgsLayoutMeasurement( s.value( QStringLiteral( "LayoutDesigner/defaultSnapGridResolution" ), 10.0, QgsSettings::Gui ).toDouble(), QgsUnitTypes::LayoutMillimeters );
//  mSnapToleranceSpinBox->setValue( mSettings->value( QStringLiteral( "LayoutDesigner/defaultSnapTolerancePixels" ), 5, QgsSettings::Gui ).toInt() );
  mGridOffset = QgsLayoutPoint( s.value( QStringLiteral( "LayoutDesigner/defaultSnapGridOffsetX" ), 0, QgsSettings::Gui ).toDouble(),
                                s.value( QStringLiteral( "LayoutDesigner/defaultSnapGridOffsetY" ), 0, QgsSettings::Gui ).toDouble(), QgsUnitTypes::LayoutMillimeters );
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
  if ( element.nodeName() != QLatin1String( "Grid" ) )
  {
    element = element.firstChildElement( QStringLiteral( "Grid" ) );
  }

  if ( element.nodeName() != QLatin1String( "Grid" ) )
  {
    return false;
  }

  const double res = element.attribute( QStringLiteral( "resolution" ), QStringLiteral( "10" ) ).toDouble();
  const QgsUnitTypes::LayoutUnit resUnit = QgsUnitTypes::decodeLayoutUnit( element.attribute( QStringLiteral( "resUnits" ) ) );
  mGridResolution = QgsLayoutMeasurement( res, resUnit );

  const double offsetX = element.attribute( QStringLiteral( "offsetX" ) ).toDouble();
  const double offsetY = element.attribute( QStringLiteral( "offsetY" ) ).toDouble();
  const QgsUnitTypes::LayoutUnit offsetUnit = QgsUnitTypes::decodeLayoutUnit( element.attribute( QStringLiteral( "offsetUnits" ) ) );
  mGridOffset = QgsLayoutPoint( offsetX, offsetY, offsetUnit );

  mLayout->pageCollection()->redraw();

  return true;
}

