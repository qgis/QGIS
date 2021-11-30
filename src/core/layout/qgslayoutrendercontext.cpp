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
#include "qgslayout.h"

QgsLayoutRenderContext::QgsLayoutRenderContext( QgsLayout *layout )
  : QObject( layout )
  , mFlags( FlagAntialiasing | FlagUseAdvancedEffects )
  , mLayout( layout )
{
  mSimplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
}

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

Qgis::RenderContextFlags QgsLayoutRenderContext::renderContextFlags() const
{
  Qgis::RenderContextFlags flags = Qgis::RenderContextFlags();
  if ( mFlags & FlagAntialiasing )
  {
    flags = flags | Qgis::RenderContextFlag::Antialiasing;
    flags = flags | Qgis::RenderContextFlag::HighQualityImageTransforms;
  }
  if ( mFlags & FlagUseAdvancedEffects )
    flags = flags | Qgis::RenderContextFlag::UseAdvancedEffects;
  if ( mFlags & FlagLosslessImageRendering )
    flags = flags | Qgis::RenderContextFlag::LosslessImageRendering;

  // TODO - expose as layout context flag?
  flags |= Qgis::RenderContextFlag::ForceVectorOutput;
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

QStringList QgsLayoutRenderContext::exportThemes() const
{
  return mExportThemes;
}

void QgsLayoutRenderContext::setExportThemes( const QStringList &exportThemes )
{
  mExportThemes = exportThemes;
}

void QgsLayoutRenderContext::setPredefinedScales( const QVector<qreal> &scales )
{
  if ( scales == mPredefinedScales )
    return;

  mPredefinedScales = scales;
  // make sure the list is sorted
  std::sort( mPredefinedScales.begin(), mPredefinedScales.end() ); // clazy:exclude=detaching-member
  emit predefinedScalesChanged();
}

QgsFeatureFilterProvider *QgsLayoutRenderContext::featureFilterProvider() const
{
  return mFeatureFilterProvider;
}

void QgsLayoutRenderContext::setFeatureFilterProvider( QgsFeatureFilterProvider *featureFilterProvider )
{
  mFeatureFilterProvider = featureFilterProvider;
}
