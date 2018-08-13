/***************************************************************************
                             qgslayoutrendercontext.cpp
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

#include "qgslayoutrendercontext.h"
#include "qgsfeature.h"
#include "qgslayout.h"

QgsLayoutRenderContext::QgsLayoutRenderContext( QgsLayout *layout )
  : QObject( layout )
  , mFlags( FlagAntialiasing | FlagUseAdvancedEffects )
  , mLayout( layout )
{}

void QgsLayoutRenderContext::setFlags( const QgsLayoutRenderContext::Flags flags )
{
  if ( flags == mFlags )
    return;

  mFlags = flags;
  emit flagsChanged( mFlags );
}

void QgsLayoutRenderContext::setFlag( const QgsLayoutRenderContext::Flag flag, const bool on )
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

QgsLayoutRenderContext::Flags QgsLayoutRenderContext::flags() const
{
  return mFlags;
}

bool QgsLayoutRenderContext::testFlag( const QgsLayoutRenderContext::Flag flag ) const
{
  return mFlags.testFlag( flag );
}

QgsRenderContext::Flags QgsLayoutRenderContext::renderContextFlags() const
{
  QgsRenderContext::Flags flags = nullptr;
  if ( mFlags & FlagAntialiasing )
    flags = flags | QgsRenderContext::Antialiasing;
  if ( mFlags & FlagUseAdvancedEffects )
    flags = flags | QgsRenderContext::UseAdvancedEffects;

  // TODO - expose as layout context flag?
  flags |= QgsRenderContext::ForceVectorOutput;
  return flags;
}

void QgsLayoutRenderContext::setDpi( double dpi )
{
  if ( qgsDoubleNear( dpi, mMeasurementConverter.dpi() ) )
    return;

  mMeasurementConverter.setDpi( dpi );
  emit dpiChanged();
}

double QgsLayoutRenderContext::dpi() const
{
  return mMeasurementConverter.dpi();
}

bool QgsLayoutRenderContext::gridVisible() const
{
  return mGridVisible;
}

void QgsLayoutRenderContext::setGridVisible( bool visible )
{
  mGridVisible = visible;
}

bool QgsLayoutRenderContext::boundingBoxesVisible() const
{
  return mBoundingBoxesVisible;
}

void QgsLayoutRenderContext::setBoundingBoxesVisible( bool visible )
{
  mBoundingBoxesVisible = visible;
}

void QgsLayoutRenderContext::setPagesVisible( bool visible )
{
  mPagesVisible = visible;
}
