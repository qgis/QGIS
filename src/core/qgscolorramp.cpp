/***************************************************************************
    qgscolorramp.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"

QgsColorRamp::~QgsColorRamp() = default;

QList<QPair<QString, QString> > QgsColorRamp::rampTypes()
{
  return QList<QPair<QString, QString> >
  {
    qMakePair( QgsGradientColorRamp::typeString(), QObject::tr( "Gradient" ) ),
    qMakePair( QgsPresetSchemeColorRamp::typeString(), QObject::tr( "Color Presets" ) ),
    qMakePair( QgsLimitedRandomColorRamp::typeString(), QObject::tr( "Random" ) ),
    qMakePair( QgsCptCityColorRamp::typeString(), QObject::tr( "Catalog: cpt-city" ) ),
    qMakePair( QgsColorBrewerColorRamp::typeString(), QObject::tr( "Catalog: ColorBrewer" ) )
  };
}
