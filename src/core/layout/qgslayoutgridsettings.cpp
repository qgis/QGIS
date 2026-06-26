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
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsunittypes.h"

#include <QString>

using namespace Qt::StringLiterals;

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

const QgsSettingsEntryString *QgsLayoutGridSettings::settingsGridStyle
  = new QgsSettingsEntryString( u"grid-style"_s, QgsSettingsTree::sTreeLayout, u"Dots"_s, u"Default rendering style for the layout designer grid. Accepted values are \"Solid\", \"Dots\" or \"Crosses\"."_s );
const QgsSettingsEntryColor *QgsLayoutGridSettings::settingsGridColor
  = new QgsSettingsEntryColor( u"grid-color"_s, QgsSettingsTree::sTreeLayout, QColor( 190, 190, 190, 100 ), u"Default color used to draw the layout designer grid."_s );
const QgsSettingsEntryDouble *QgsLayoutGridSettings::settingsGridResolution
  = new QgsSettingsEntryDouble( u"resolution"_s, QgsSettingsTree::sTreeLayoutGrid, 10.0, u"Default grid resolution (in millimeters) for newly created layouts."_s );
const QgsSettingsEntryDouble *QgsLayoutGridSettings::settingsGridOffsetX
  = new QgsSettingsEntryDouble( u"offset-x"_s, QgsSettingsTree::sTreeLayoutGrid, 0, u"Default grid horizontal offset (in millimeters) for newly created layouts."_s );
const QgsSettingsEntryDouble *QgsLayoutGridSettings::settingsGridOffsetY
  = new QgsSettingsEntryDouble( u"offset-y"_s, QgsSettingsTree::sTreeLayoutGrid, 0, u"Default grid vertical offset (in millimeters) for newly created layouts."_s );

void QgsLayoutGridSettings::loadFromSettings()
{
  //read grid style, grid color and pen width from settings
  const QString gridStyleString = settingsGridStyle->value();
  const QColor gridColor = settingsGridColor->value();

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

  mGridResolution = QgsLayoutMeasurement( settingsGridResolution->value(), Qgis::LayoutUnit::Millimeters );
  mGridOffset = QgsLayoutPoint( settingsGridOffsetX->value(), settingsGridOffsetY->value(), Qgis::LayoutUnit::Millimeters );
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
