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
  if ( flags == mFlags )
    return;

  mFlags = flags;
  emit flagsChanged( mFlags );
}

void QgsLayoutContext::setFlag( const QgsLayoutContext::Flag flag, const bool on )
{
  Flags newFlags = mFlags;
  if ( on )
    newFlags |= flag;
  else
    newFlags &= ~flag;

  if ( newFlags == mFlags )
    return;

  mFlags = newFlags;
  emit flagsChanged( mFlags );
}

QgsLayoutContext::Flags QgsLayoutContext::flags() const
{
  return mFlags;
}

bool QgsLayoutContext::testFlag( const QgsLayoutContext::Flag flag ) const
{
  return mFlags.testFlag( flag );
}

QgsRenderContext::Flags QgsLayoutContext::renderContextFlags() const
{
  QgsRenderContext::Flags flags = 0;
  if ( mFlags & FlagAntialiasing )
    flags = flags | QgsRenderContext::Antialiasing;
  if ( mFlags & FlagUseAdvancedEffects )
    flags = flags | QgsRenderContext::UseAdvancedEffects;

  // TODO - expose as layout context flag?
  flags |= QgsRenderContext::ForceVectorOutput;
  return flags;
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

bool QgsLayoutContext::gridVisible() const
{
  return mGridVisible;
}

void QgsLayoutContext::setGridVisible( bool visible )
{
  mGridVisible = visible;
}

bool QgsLayoutContext::boundingBoxesVisible() const
{
  return mBoundingBoxesVisible;
}

void QgsLayoutContext::setBoundingBoxesVisible( bool visible )
{
  mBoundingBoxesVisible = visible;
}

void QgsLayoutContext::setPagesVisible( bool visible )
{
  mPagesVisible = visible;
}
