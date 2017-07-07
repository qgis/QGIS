/***************************************************************************
                             qgslayoutcontext.cpp
                             --------------------
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

#include "qgslayoutcontext.h"
#include "qgsfeature.h"


QgsLayoutContext::QgsLayoutContext()
  : mFlags( FlagAntialiasing | FlagUseAdvancedEffects )
{}

void QgsLayoutContext::setFlags( const QgsLayoutContext::Flags flags )
{
  mFlags = flags;
}

void QgsLayoutContext::setFlag( const QgsLayoutContext::Flag flag, const bool on )
{
  if ( on )
    mFlags |= flag;
  else
    mFlags &= ~flag;
}

QgsLayoutContext::Flags QgsLayoutContext::flags() const
{
  return mFlags;
}

bool QgsLayoutContext::testFlag( const QgsLayoutContext::Flag flag ) const
{
  return mFlags.testFlag( flag );
}

QgsVectorLayer *QgsLayoutContext::layer() const
{
  return mLayer;
}

void QgsLayoutContext::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsLayoutContext::setDpi( double dpi )
{
  mMeasurementConverter.setDpi( dpi );
}

double QgsLayoutContext::dpi() const
{
  return mMeasurementConverter.dpi();
}
