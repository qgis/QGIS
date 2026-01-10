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

#include "qgslayout.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgsreadwritecontext.h"
#include "qgssettings.h"
#include "qgsunittypes.h"

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
  gridStyleString = s.value( u"LayoutDesigner/gridStyle"_s, "Dots", QgsSettings::Gui ).toString();

  int gridRed, gridGreen, gridBlue, gridAlpha;
  gridRed = s.value( u"LayoutDesigner/gridRed"_s, 190, QgsSettings::Gui ).toInt();
  gridGreen = s.value( u"LayoutDesigner/gridGreen"_s, 190, QgsSettings::Gui ).toInt();
  gridBlue = s.value( u"LayoutDesigner/gridBlue"_s, 190, QgsSettings::Gui ).toInt();
  gridAlpha = s.value( u"LayoutDesigner/gridAlpha"_s, 100, QgsSettings::Gui ).toInt();
  const QColor gridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );

  mGridPen.setColor( gridColor );
  mGridPen.setWidthF( 0 );
  mGridPen.setCosmetic( true );

  if ( gridStyleString == "Dots"_L1 )
  {
    mGridStyle = StyleDots;
  }
  else if ( gridStyleString == "Crosses"_L1 )
  {
    mGridStyle = StyleCrosses;
  }
  else
  {
    mGridStyle = StyleLines;
  }

  mGridResolution = QgsLayoutMeasurement( s.value( u"LayoutDesigner/defaultSnapGridResolution"_s, 10.0, QgsSettings::Gui ).toDouble(), Qgis::LayoutUnit::Millimeters );
//  mSnapToleranceSpinBox->setValue( mSettings->value( u"LayoutDesigner/defaultSnapTolerancePixels"_s, 5, QgsSettings::Gui ).toInt() );
  mGridOffset = QgsLayoutPoint( s.value( u"LayoutDesigner/defaultSnapGridOffsetX"_s, 0, QgsSettings::Gui ).toDouble(),
                                s.value( u"LayoutDesigner/defaultSnapGridOffsetY"_s, 0, QgsSettings::Gui ).toDouble(), Qgis::LayoutUnit::Millimeters );
}

bool QgsLayoutGridSettings::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( u"Grid"_s );

  element.setAttribute( u"resolution"_s, mGridResolution.length() );
  element.setAttribute( u"resUnits"_s, QgsUnitTypes::encodeUnit( mGridResolution.units() ) );

  element.setAttribute( u"offsetX"_s, mGridOffset.x() );
  element.setAttribute( u"offsetY"_s, mGridOffset.y() );
  element.setAttribute( u"offsetUnits"_s, QgsUnitTypes::encodeUnit( mGridOffset.units() ) );

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutGridSettings::readXml( const QDomElement &e, const QDomDocument &, const QgsReadWriteContext & )
{
  QDomElement element = e;
  if ( element.nodeName() != "Grid"_L1 )
  {
    element = element.firstChildElement( u"Grid"_s );
  }

  if ( element.nodeName() != "Grid"_L1 )
  {
    return false;
  }

  const double res = element.attribute( u"resolution"_s, u"10"_s ).toDouble();
  const Qgis::LayoutUnit resUnit = QgsUnitTypes::decodeLayoutUnit( element.attribute( u"resUnits"_s ) );
  mGridResolution = QgsLayoutMeasurement( res, resUnit );

  const double offsetX = element.attribute( u"offsetX"_s ).toDouble();
  const double offsetY = element.attribute( u"offsetY"_s ).toDouble();
  const Qgis::LayoutUnit offsetUnit = QgsUnitTypes::decodeLayoutUnit( element.attribute( u"offsetUnits"_s ) );
  mGridOffset = QgsLayoutPoint( offsetX, offsetY, offsetUnit );

  mLayout->pageCollection()->redraw();

  return true;
}

