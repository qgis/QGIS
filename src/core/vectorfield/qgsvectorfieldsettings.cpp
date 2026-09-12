/***************************************************************************
    qgsvectorfieldsettings.cpp
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldsettings.h"

#include "qgscolorutils.h"

#include <QString>

using namespace Qt::StringLiterals;

double QgsVectorFieldSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsVectorFieldSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsVectorFieldSettings::color() const
{
  return mColor;
}

void QgsVectorFieldSettings::setColor( const QColor &vectorColor )
{
  mColor = vectorColor;
}

double QgsVectorFieldSettings::filterMin() const
{
  return mFilterMin;
}

void QgsVectorFieldSettings::setFilterMin( double vectorFilterMin )
{
  mFilterMin = vectorFilterMin;
}

double QgsVectorFieldSettings::filterMax() const
{
  return mFilterMax;
}

void QgsVectorFieldSettings::setFilterMax( double vectorFilterMax )
{
  mFilterMax = vectorFilterMax;
}

bool QgsVectorFieldSettings::isOnUserDefinedGrid() const
{
  return mOnUserDefinedGrid;
}

void QgsVectorFieldSettings::setOnUserDefinedGrid( bool enabled )
{
  mOnUserDefinedGrid = enabled;
}

int QgsVectorFieldSettings::userGridCellWidth() const
{
  return mUserGridCellWidth;
}

void QgsVectorFieldSettings::setUserGridCellWidth( int width )
{
  mUserGridCellWidth = width;
}

int QgsVectorFieldSettings::userGridCellHeight() const
{
  return mUserGridCellHeight;
}

void QgsVectorFieldSettings::setUserGridCellHeight( int height )
{
  mUserGridCellHeight = height;
}

QgsVectorFieldSettings::Symbology QgsVectorFieldSettings::symbology() const
{
  return mDisplayingMethod;
}

void QgsVectorFieldSettings::setSymbology( const Symbology &displayingMethod )
{
  mDisplayingMethod = displayingMethod;
}

QgsVectorFieldArrowSettings QgsVectorFieldSettings::arrowSettings() const
{
  return mArrowsSettings;
}

void QgsVectorFieldSettings::setArrowsSettings( const QgsVectorFieldArrowSettings &arrowSettings )
{
  mArrowsSettings = arrowSettings;
}

QgsVectorFieldStreamlineSettings QgsVectorFieldSettings::streamLinesSettings() const
{
  return mStreamLinesSettings;
}

void QgsVectorFieldSettings::setStreamLinesSettings( const QgsVectorFieldStreamlineSettings &streamLinesSettings )
{
  mStreamLinesSettings = streamLinesSettings;
}

QDomElement QgsVectorFieldSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( u"vector-settings"_s );
  elem.setAttribute( u"symbology"_s, static_cast< int >( mDisplayingMethod ) );

  elem.setAttribute( u"line-width"_s, mLineWidth );
  elem.setAttribute( u"coloring-method"_s, coloringMethod() );
  elem.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );
  const QDomElement elemShader = mColorRampShader.writeXml( doc, context );
  elem.appendChild( elemShader );
  elem.setAttribute( u"filter-min"_s, mFilterMin );
  elem.setAttribute( u"filter-max"_s, mFilterMax );

  elem.setAttribute( u"user-grid-enabled"_s, mOnUserDefinedGrid ? u"1"_s : u"0"_s );
  elem.setAttribute( u"user-grid-width"_s, mUserGridCellWidth );
  elem.setAttribute( u"user-grid-height"_s, mUserGridCellHeight );

  elem.appendChild( mArrowsSettings.writeXml( doc ) );
  elem.appendChild( mStreamLinesSettings.writeXml( doc ) );
  elem.appendChild( mTracesSettings.writeXml( doc ) );
  elem.appendChild( mWindBarbSettings.writeXml( doc ) );

  return elem;
}

void QgsVectorFieldSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mDisplayingMethod = static_cast<QgsVectorFieldSettings::Symbology>( elem.attribute( u"symbology"_s ).toInt() );

  mLineWidth = elem.attribute( u"line-width"_s ).toDouble();
  mColoringMethod = static_cast<QgsInterpolatedLineColor::ColoringMethod>( elem.attribute( u"coloring-method"_s ).toInt() );
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s ) );
  mColorRampShader.readXml( elem.firstChildElement( "colorrampshader" ), context );
  mFilterMin = elem.attribute( u"filter-min"_s ).toDouble();
  mFilterMax = elem.attribute( u"filter-max"_s ).toDouble();

  mOnUserDefinedGrid = elem.attribute( u"user-grid-enabled"_s ).toInt(); //bool
  mUserGridCellWidth = elem.attribute( u"user-grid-width"_s ).toInt();
  mUserGridCellHeight = elem.attribute( u"user-grid-height"_s ).toInt();

  const QDomElement elemVector = elem.firstChildElement( u"vector-arrow-settings"_s );
  if ( !elemVector.isNull() )
    mArrowsSettings.readXml( elemVector );

  const QDomElement elemStreamLine = elem.firstChildElement( u"vector-streamline-settings"_s );
  if ( !elemStreamLine.isNull() )
    mStreamLinesSettings.readXml( elemStreamLine );

  const QDomElement elemTraces = elem.firstChildElement( u"vector-traces-settings"_s );
  if ( !elemTraces.isNull() )
    mTracesSettings.readXml( elemTraces );

  const QDomElement elemWindBarb = elem.firstChildElement( u"vector-windbarb-settings"_s );
  if ( !elemWindBarb.isNull() )
    mWindBarbSettings.readXml( elemWindBarb );
}

QgsInterpolatedLineColor::ColoringMethod QgsVectorFieldSettings::coloringMethod() const
{
  return mColoringMethod;
}

void QgsVectorFieldSettings::setColoringMethod( const QgsInterpolatedLineColor::ColoringMethod &coloringMethod )
{
  mColoringMethod = coloringMethod;
}

QgsColorRampShader QgsVectorFieldSettings::colorRampShader() const
{
  return mColorRampShader;
}

void QgsVectorFieldSettings::setColorRampShader( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
}

QgsInterpolatedLineColor QgsVectorFieldSettings::vectorStrokeColoring() const
{
  QgsInterpolatedLineColor strokeColoring;
  switch ( mColoringMethod )
  {
    case QgsInterpolatedLineColor::SingleColor:
      strokeColoring = QgsInterpolatedLineColor( mColor );
      break;
    case QgsInterpolatedLineColor::ColorRamp:
      strokeColoring = QgsInterpolatedLineColor( mColorRampShader );
      break;
  }

  return strokeColoring;
}

QgsVectorFieldTracesSettings QgsVectorFieldSettings::tracesSettings() const
{
  return mTracesSettings;
}

void QgsVectorFieldSettings::setTracesSettings( const QgsVectorFieldTracesSettings &tracesSettings )
{
  mTracesSettings = tracesSettings;
}

QgsVectorFieldWindBarbSettings QgsVectorFieldSettings::windBarbSettings() const
{
  return mWindBarbSettings;
}

void QgsVectorFieldSettings::setWindBarbSettings( const QgsVectorFieldWindBarbSettings &windBarbSettings )
{
  mWindBarbSettings = windBarbSettings;
}
