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
#include "moc_qgslayoutrendercontext.cpp"
#include "qgslayout.h"

QgsLayoutRenderContext::QgsLayoutRenderContext( QgsLayout *layout )
  : QObject( layout )
  , mFlags( Qgis::LayoutRenderFlag::Antialiasing | Qgis::LayoutRenderFlag::UseAdvancedEffects )
  , mLayout( layout )
{
  mSimplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlag::NoSimplification );
}

void QgsLayoutRenderContext::setFlags( const Qgis::LayoutRenderFlags flags )
{
  if ( flags == mFlags )
    return;

  mFlags = flags;
  matchRasterizedRenderingPolicyToFlags();
  emit flagsChanged( mFlags );
}

void QgsLayoutRenderContext::setFlag( const Qgis::LayoutRenderFlag flag, const bool on )
{
  Qgis::LayoutRenderFlags newFlags = mFlags;
  if ( on )
    newFlags |= flag;
  else
    newFlags &= ~static_cast< int >( flag );

  if ( newFlags == mFlags )
    return;

  mFlags = newFlags;
  matchRasterizedRenderingPolicyToFlags();
  emit flagsChanged( mFlags );
}

Qgis::LayoutRenderFlags QgsLayoutRenderContext::flags() const
{
  return mFlags;
}

bool QgsLayoutRenderContext::testFlag( const Qgis::LayoutRenderFlag flag ) const
{
  return mFlags.testFlag( flag );
}

Qgis::RenderContextFlags QgsLayoutRenderContext::renderContextFlags() const
{
  Qgis::RenderContextFlags flags = Qgis::RenderContextFlags();
  if ( mFlags & Qgis::LayoutRenderFlag::Antialiasing )
  {
    flags = flags | Qgis::RenderContextFlag::Antialiasing;
    flags = flags | Qgis::RenderContextFlag::HighQualityImageTransforms;
  }
  if ( mFlags & Qgis::LayoutRenderFlag::UseAdvancedEffects )
    flags = flags | Qgis::RenderContextFlag::UseAdvancedEffects;
  if ( mFlags & Qgis::LayoutRenderFlag::LosslessImageRendering )
    flags = flags | Qgis::RenderContextFlag::LosslessImageRendering;

  // TODO - expose as layout context flag?
  flags |= Qgis::RenderContextFlag::ForceVectorOutput;
  return flags;
}

Qgis::RasterizedRenderingPolicy QgsLayoutRenderContext::rasterizedRenderingPolicy() const
{
  return mRasterizedRenderingPolicy;
}

void QgsLayoutRenderContext::setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy policy )
{
  mRasterizedRenderingPolicy = policy;
  switch ( mRasterizedRenderingPolicy )
  {
    case Qgis::RasterizedRenderingPolicy::Default:
    case Qgis::RasterizedRenderingPolicy::PreferVector:
      mFlags.setFlag( Qgis::LayoutRenderFlag::ForceVectorOutput, false );
      mFlags.setFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects, true );
      break;
    case Qgis::RasterizedRenderingPolicy::ForceVector:
      mFlags.setFlag( Qgis::LayoutRenderFlag::ForceVectorOutput, true );
      mFlags.setFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects, false );
      break;
  }
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

void QgsLayoutRenderContext::setMaskSettings( const QgsMaskRenderSettings &settings )
{
  mMaskRenderSettings = settings;
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

void QgsLayoutRenderContext::matchRasterizedRenderingPolicyToFlags()
{
  if ( !mFlags.testFlag( Qgis::LayoutRenderFlag::ForceVectorOutput )
       && mFlags.testFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects ) )
    mRasterizedRenderingPolicy = Qgis::RasterizedRenderingPolicy::PreferVector;
  else if ( mFlags.testFlag( Qgis::LayoutRenderFlag::ForceVectorOutput )
            || !mFlags.testFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects ) )
    mRasterizedRenderingPolicy = Qgis::RasterizedRenderingPolicy::ForceVector;
}
