/***************************************************************************
  qgsmapsettings.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapsettings.h"

#include "qgscoordinatetransform.h"
#include "qgsellipsoidutils.h"
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgsgrouplayer.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgspainting.h"
#include "qgsscalecalculator.h"
#include "qgsunittypes.h"
#include "qgsxmlutils.h"

QgsMapSettings::QgsMapSettings()
  : mDpi( QgsPainting::qtDefaultDpiX() ) // DPI that will be used by default for QImage instances
  , mSize( QSize( 0, 0 ) )
  , mBackgroundColor( Qt::white )
  , mSelectionColor( Qt::yellow )
  , mFlags( Qgis::MapSettingsFlag::Antialiasing | Qgis::MapSettingsFlag::UseAdvancedEffects | Qgis::MapSettingsFlag::DrawLabeling | Qgis::MapSettingsFlag::DrawSelection )
  , mSegmentationTolerance( M_PI_2 / 90 )
{
  mScaleCalculator.setMapUnits( Qgis::DistanceUnit::Unknown );
  mSimplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlag::NoSimplification );

  updateDerived();
}

void QgsMapSettings::setMagnificationFactor( double factor, const QgsPointXY *center )
{
  const double ratio = mMagnificationFactor / factor;

  mMagnificationFactor = factor;

  const double rot = rotation();
  setRotation( 0.0 );

  QgsRectangle ext = visibleExtent();
  ext.scale( ratio, center );

  mRotation = rot;
  mExtent = ext;
  mDpi = mDpi / ratio;

  QgsDebugMsgLevel( u"Magnification factor: %1  dpi: %2  ratio: %3"_s.arg( factor ).arg( mDpi ).arg( ratio ), 3 );

  updateDerived();
}

double QgsMapSettings::magnificationFactor() const
{
  return mMagnificationFactor;
}

QgsRectangle QgsMapSettings::extent() const
{
  return mExtent;
}

void QgsMapSettings::setExtent( const QgsRectangle &extent, bool magnified )
{
  QgsRectangle magnifiedExtent = extent;

  if ( !magnified )
    magnifiedExtent.scale( 1 / mMagnificationFactor );

  mExtent = magnifiedExtent;

  updateDerived();
}

double QgsMapSettings::extentBuffer() const
{
  return mExtentBuffer;
}

void QgsMapSettings::setExtentBuffer( const double buffer )
{
  mExtentBuffer = buffer;
}

double QgsMapSettings::rotation() const
{
  return mRotation;
}

void QgsMapSettings::setRotation( double degrees )
{
  if ( qgsDoubleNear( mRotation, degrees ) )
    return;

  mRotation = degrees;

  // TODO: update extent while keeping scale ?
  updateDerived();
}


void QgsMapSettings::updateDerived()
{
  const QgsRectangle extent = mExtent;

  if ( extent.isEmpty() || !extent.isFinite() )
  {
    mValid = false;
    return;
  }

  // Don't allow zooms where the current extent is so small that it
  // can't be accurately represented using a double (which is what
  // currentExtent uses). Excluding 0 avoids a divide by zero and an
  // infinite loop when rendering to a new canvas. Excluding extents
  // greater than 1 avoids doing unnecessary calculations.

  // The scheme is to compare the width against the mean x coordinate
  // (and height against mean y coordinate) and only allow zooms where
  // the ratio indicates that there is more than about 12 significant
  // figures (there are about 16 significant figures in a double).

  if ( extent.width()  > 0 &&
       extent.height() > 0 &&
       extent.width()  < 1 &&
       extent.height() < 1 )
  {
    // Use abs() on the extent to avoid the case where the extent is
    // symmetrical about 0.
    const double xMean = ( std::fabs( extent.xMinimum() ) + std::fabs( extent.xMaximum() ) ) * 0.5;
    const double yMean = ( std::fabs( extent.yMinimum() ) + std::fabs( extent.yMaximum() ) ) * 0.5;

    const double xRange = extent.width() / xMean;
    const double yRange = extent.height() / yMean;

    static const double MIN_PROPORTION = 1e-12;
    if ( xRange < MIN_PROPORTION || yRange < MIN_PROPORTION )
    {
      mValid = false;
      return;
    }
  }

  const double myHeight = mSize.height();
  const double myWidth = mSize.width();

  if ( !myWidth || !myHeight )
  {
    mValid = false;
    return;
  }

  // calculate the translation and scaling parameters
  const double mapUnitsPerPixelY = mExtent.height() / myHeight;
  const double mapUnitsPerPixelX = mExtent.width() / myWidth;
  mMapUnitsPerPixel = mapUnitsPerPixelY > mapUnitsPerPixelX ? mapUnitsPerPixelY : mapUnitsPerPixelX;

  // calculate the actual extent of the mapCanvas
  double dxmin = mExtent.xMinimum(), dxmax = mExtent.xMaximum(),
         dymin = mExtent.yMinimum(), dymax = mExtent.yMaximum(), whitespace;

  if ( mapUnitsPerPixelY > mapUnitsPerPixelX )
  {
    whitespace = ( ( myWidth * mMapUnitsPerPixel ) - mExtent.width() ) * 0.5;
    dxmin -= whitespace;
    dxmax += whitespace;
  }
  else
  {
    whitespace = ( ( myHeight * mMapUnitsPerPixel ) - mExtent.height() ) * 0.5;
    dymin -= whitespace;
    dymax += whitespace;
  }

  mVisibleExtent.set( dxmin, dymin, dxmax, dymax );

  // update the scale
  mScaleCalculator.setDpi( mDpi );
  mScale = mScaleCalculator.calculate( mVisibleExtent, mSize.width() );

  bool ok = true;
  mMapToPixel.setParameters(
    mapUnitsPerPixel(),
    visibleExtent().center().x(),
    visibleExtent().center().y(),
    outputSize().width(),
    outputSize().height(),
    mRotation, &ok );

  mValid = ok;

#if 1 // set visible extent taking rotation in consideration
  if ( mRotation )
  {
    const QgsPointXY p1 = mMapToPixel.toMapCoordinates( QPoint( 0, 0 ) );
    const QgsPointXY p2 = mMapToPixel.toMapCoordinates( QPoint( 0, myHeight ) );
    const QgsPointXY p3 = mMapToPixel.toMapCoordinates( QPoint( myWidth, 0 ) );
    const QgsPointXY p4 = mMapToPixel.toMapCoordinates( QPoint( myWidth, myHeight ) );
    dxmin = std::min( p1.x(), std::min( p2.x(), std::min( p3.x(), p4.x() ) ) );
    dymin = std::min( p1.y(), std::min( p2.y(), std::min( p3.y(), p4.y() ) ) );
    dxmax = std::max( p1.x(), std::max( p2.x(), std::max( p3.x(), p4.x() ) ) );
    dymax = std::max( p1.y(), std::max( p2.y(), std::max( p3.y(), p4.y() ) ) );
    mVisibleExtent.set( dxmin, dymin, dxmax, dymax );
  }
#endif

  QgsDebugMsgLevel( u"Map units per pixel (x,y) : %1, %2"_s.arg( qgsDoubleToString( mapUnitsPerPixelX ), qgsDoubleToString( mapUnitsPerPixelY ) ), 5 );
  QgsDebugMsgLevel( u"Pixmap dimensions (x,y) : %1, %2"_s.arg( qgsDoubleToString( mSize.width() ), qgsDoubleToString( mSize.height() ) ), 5 );
  QgsDebugMsgLevel( u"Extent dimensions (x,y) : %1, %2"_s.arg( qgsDoubleToString( mExtent.width() ), qgsDoubleToString( mExtent.height() ) ), 5 );
  QgsDebugMsgLevel( mExtent.toString(), 5 );
  QgsDebugMsgLevel( u"Adjusted map units per pixel (x,y) : %1, %2"_s.arg( qgsDoubleToString( mVisibleExtent.width() / myWidth ), qgsDoubleToString( mVisibleExtent.height() / myHeight ) ), 5 );
  QgsDebugMsgLevel( u"Recalced pixmap dimensions (x,y) : %1, %2"_s.arg( qgsDoubleToString( mVisibleExtent.width() / mMapUnitsPerPixel ), qgsDoubleToString( mVisibleExtent.height() / mMapUnitsPerPixel ) ), 5 );
  QgsDebugMsgLevel( u"Scale (assuming meters as map units) = 1:%1"_s.arg( qgsDoubleToString( mScale ) ), 5 );
  QgsDebugMsgLevel( u"Rotation: %1 degrees"_s.arg( mRotation ), 5 );
  QgsDebugMsgLevel( u"Extent: %1"_s.arg( mExtent.asWktCoordinates() ), 5 );
  QgsDebugMsgLevel( u"Visible Extent: %1"_s.arg( mVisibleExtent.asWktCoordinates() ), 5 );
  QgsDebugMsgLevel( u"Magnification factor: %1"_s.arg( mMagnificationFactor ), 5 );

}

void QgsMapSettings::matchRasterizedRenderingPolicyToFlags()
{
  if ( !mFlags.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput )
       && mFlags.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) )
    mRasterizedRenderingPolicy = Qgis::RasterizedRenderingPolicy::Default;
  else if ( mFlags.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput )
            && mFlags.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) )
    mRasterizedRenderingPolicy = Qgis::RasterizedRenderingPolicy::PreferVector;
  else if ( mFlags.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput )
            && !mFlags.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) )
    mRasterizedRenderingPolicy = Qgis::RasterizedRenderingPolicy::ForceVector;
}

QSize QgsMapSettings::outputSize() const
{
  return mSize;
}

void QgsMapSettings::setOutputSize( QSize size )
{
  mSize = size;

  updateDerived();
}

float QgsMapSettings::devicePixelRatio() const
{
  return mDevicePixelRatio;
}

void QgsMapSettings::setDevicePixelRatio( float dpr )
{
  mDevicePixelRatio = dpr;
  updateDerived();
}

QSize QgsMapSettings::deviceOutputSize() const
{
  return outputSize() * mDevicePixelRatio;
}

double QgsMapSettings::outputDpi() const
{
  return mDpi;
}

void QgsMapSettings::setOutputDpi( double dpi )
{
  mDpi = dpi;

  updateDerived();
}

double QgsMapSettings::dpiTarget() const
{
  return mDpiTarget;
}

void QgsMapSettings::setDpiTarget( double dpi )
{
  mDpiTarget = dpi;
}

QStringList QgsMapSettings::layerIds( bool expandGroupLayers ) const
{
  if ( !expandGroupLayers || !mHasGroupLayers )
  {
    return mLayerIds;
  }
  else
  {
    const QList<QgsMapLayer * > mapLayers = layers( expandGroupLayers );
    QStringList res;
    res.reserve( mapLayers.size() );
    for ( const QgsMapLayer *layer : mapLayers )
      res << layer->id();
    return res;
  }
}

QList<QgsMapLayer *> QgsMapSettings::layers( bool expandGroupLayers ) const
{
  const QList<QgsMapLayer *> actualLayers = _qgis_listQPointerToRaw( mLayers );
  if ( !expandGroupLayers )
    return actualLayers;

  QList< QgsMapLayer * > result;

  std::function< void( const QList< QgsMapLayer * >& layers ) > expandLayers;
  expandLayers = [&result, &expandLayers]( const QList< QgsMapLayer * > &layers )
  {
    for ( QgsMapLayer *layer : layers )
    {
      if ( QgsGroupLayer *groupLayer = qobject_cast< QgsGroupLayer * >( layer ) )
      {
        expandLayers( groupLayer->childLayers() );
      }
      else
      {
        result << layer;
      }
    }
  };

  expandLayers( actualLayers );
  return result;
}

template<typename T>
QVector<T> QgsMapSettings::layers() const
{
  const QList<QgsMapLayer *> actualLayers = _qgis_listQPointerToRaw( mLayers );

  QVector<T> layers;
  for ( QgsMapLayer *layer : actualLayers )
  {
    T tLayer = qobject_cast<T>( layer );
    if ( tLayer )
    {
      layers << tLayer;
    }
  }
  return layers;
}

void QgsMapSettings::setLayers( const QList<QgsMapLayer *> &layers )
{
  // filter list, removing null layers and non-spatial layers
  auto filteredList = layers;
  filteredList.erase( std::remove_if( filteredList.begin(), filteredList.end(),
                                      []( QgsMapLayer * layer )
  {
    return !layer || !layer->isSpatial();
  } ), filteredList.end() );

  mLayers = _qgis_listRawToQPointer( filteredList );

  // pre-generate and store layer IDs, so that we can safely access them from other threads
  // without needing to touch the actual map layer object
  mLayerIds.clear();
  mHasGroupLayers = false;
  mLayerIds.reserve( mLayers.size() );
  for ( const QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    mLayerIds << layer->id();
    if ( layer->type() == Qgis::LayerType::Group )
      mHasGroupLayers = true;
  }
}

QMap<QString, QString> QgsMapSettings::layerStyleOverrides() const
{
  return mLayerStyleOverrides;
}

void QgsMapSettings::setLayerStyleOverrides( const QMap<QString, QString> &overrides )
{
  mLayerStyleOverrides = overrides;
}

void QgsMapSettings::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  mDestCRS = crs;
  mScaleCalculator.setMapUnits( crs.mapUnits() );
  // Since the map units have changed, force a recalculation of the scale.
  updateDerived();
}

QgsCoordinateReferenceSystem QgsMapSettings::destinationCrs() const
{
  return mDestCRS;
}

bool QgsMapSettings::setEllipsoid( const QString &ellipsoid )
{
  const QgsEllipsoidUtils::EllipsoidParameters params = QgsEllipsoidUtils::ellipsoidParameters( ellipsoid );
  if ( !params.valid )
  {
    return false;
  }
  else
  {
    mEllipsoid = ellipsoid;
    return true;
  }
}

void QgsMapSettings::setFlags( Qgis::MapSettingsFlags flags )
{
  mFlags = flags;
  matchRasterizedRenderingPolicyToFlags();
}

void QgsMapSettings::setFlag( Qgis::MapSettingsFlag flag, bool on )
{
  if ( on )
    mFlags |= flag;
  else
    mFlags &= ~( static_cast< int >( flag ) );
  matchRasterizedRenderingPolicyToFlags();
}

Qgis::MapSettingsFlags QgsMapSettings::flags() const
{
  return mFlags;
}

bool QgsMapSettings::testFlag( Qgis::MapSettingsFlag flag ) const
{
  return mFlags.testFlag( flag );
}

Qgis::DistanceUnit QgsMapSettings::mapUnits() const
{
  return mScaleCalculator.mapUnits();
}

Qgis::ScaleCalculationMethod QgsMapSettings::scaleMethod() const
{
  return mScaleCalculator.method();
}

void QgsMapSettings::setScaleMethod( Qgis::ScaleCalculationMethod method )
{
  mScaleCalculator.setMethod( method );
  updateDerived();
}

bool QgsMapSettings::hasValidSettings() const
{
  return mValid;
}

QgsRectangle QgsMapSettings::visibleExtent() const
{
  return mVisibleExtent;
}

QPolygonF QgsMapSettings::visiblePolygon() const
{
  QPolygonF poly;

  const QSize &sz = outputSize();
  const QgsMapToPixel &m2p = mapToPixel();

  poly << m2p.toMapCoordinates( 0.0, 0.0 ).toQPointF();
  poly << m2p.toMapCoordinates( static_cast<double>( sz.width() ), 0.0 ).toQPointF();
  poly << m2p.toMapCoordinates( static_cast<double>( sz.width() ), static_cast<double>( sz.height() ) ).toQPointF();
  poly << m2p.toMapCoordinates( 0.0, static_cast<double>( sz.height() ) ).toQPointF();

  return poly;
}

QPolygonF QgsMapSettings::visiblePolygonWithBuffer() const
{
  QPolygonF poly;

  const QSize &sz = outputSize();
  const QgsMapToPixel &m2p = mapToPixel();

  // Transform tilebuffer in pixel.
  // Original tilebuffer is in pixel and transformed only according
  // extent width (see QgsWmsRenderContext::mapTileBuffer)

  const double mapUnitsPerPixel = mExtent.width() / sz.width();
  const double buffer = mExtentBuffer / mapUnitsPerPixel;

  poly << m2p.toMapCoordinates( -buffer, -buffer ).toQPointF();
  poly << m2p.toMapCoordinates( static_cast<double>( sz.width() + buffer ), -buffer ).toQPointF();
  poly << m2p.toMapCoordinates( static_cast<double>( sz.width() + buffer ), static_cast<double>( sz.height() + buffer ) ).toQPointF();
  poly << m2p.toMapCoordinates( -buffer, static_cast<double>( sz.height() + buffer ) ).toQPointF();

  return poly;
}

double QgsMapSettings::mapUnitsPerPixel() const
{
  return mMapUnitsPerPixel;
}

double QgsMapSettings::scale() const
{
  return mScale;
}

QgsCoordinateTransformContext QgsMapSettings::transformContext() const
{
#ifdef QGISDEBUG
  if ( !mHasTransformContext )
    QgsDebugMsgLevel( u"No QgsCoordinateTransformContext context set for transform"_s, 4 );
#endif

  return mTransformContext;
}

void QgsMapSettings::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
#ifdef QGISDEBUG
  mHasTransformContext = true;
#endif
}

QgsCoordinateTransform QgsMapSettings::layerTransform( const QgsMapLayer *layer ) const
{
  if ( !layer )
    return QgsCoordinateTransform();

  return QgsCoordinateTransform( layer->crs(), mDestCRS, mTransformContext );
}

QgsRectangle QgsMapSettings::computeExtentForScale( const QgsPointXY &center, double scale ) const
{
  // Output width in inches
  const double outputWidthInInches = outputSize().width() / outputDpi();

  // Desired visible width (honouring scale)
  const double scaledWidthInInches = outputWidthInInches * scale;

  if ( mapUnits() == Qgis::DistanceUnit::Degrees )
  {
    // Start with some fraction of the current extent around the center
    const double delta = mExtent.width() / 100.;
    QgsRectangle ext( center.x() - delta, center.y() - delta, center.x() + delta, center.y() + delta );
    // Get scale at extent, and then scale extent to the desired scale
    const double testScale = mScaleCalculator.calculate( ext, outputSize().width() );
    ext.scale( scale / testScale );
    return ext;
  }
  else
  {
    // Conversion from inches to mapUnits  - this is safe to use, because we know here that the map units AREN'T in degrees
    const double conversionFactor = QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Feet, mapUnits() ) / 12;

    const double delta = 0.5 * scaledWidthInInches * conversionFactor;
    return QgsRectangle( center.x() - delta, center.y() - delta, center.x() + delta, center.y() + delta );
  }
}

double QgsMapSettings::computeScaleForExtent( const QgsRectangle &extent ) const
{
  return mScaleCalculator.calculate( extent, outputSize().width() );
}

double QgsMapSettings::layerToMapUnits( const QgsMapLayer *layer, const QgsRectangle &referenceExtent ) const
{
  return layerTransform( layer ).scaleFactor( referenceExtent );
}


QgsRectangle QgsMapSettings::layerExtentToOutputExtent( const QgsMapLayer *layer, QgsRectangle extent ) const
{
  try
  {
    QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
    {
      QgsDebugMsgLevel( u"sourceCrs = %1"_s.arg( ct.sourceCrs().authid() ), 3 );
      QgsDebugMsgLevel( u"destCRS = %1"_s.arg( ct.destinationCrs().authid() ), 3 );
      QgsDebugMsgLevel( u"extent %1"_s.arg( extent.toString() ), 3 );
      ct.setBallparkTransformsAreAppropriate( true );
      extent = ct.transformBoundingBox( extent );
    }
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  QgsDebugMsgLevel( u"proj extent = %1 "_s.arg( extent.toString() ), 3 );

  return extent;
}


QgsRectangle QgsMapSettings::outputExtentToLayerExtent( const QgsMapLayer *layer, QgsRectangle extent ) const
{
  try
  {
    QgsCoordinateTransform ct = layerTransform( layer );
    ct.setBallparkTransformsAreAppropriate( true );
    if ( ct.isValid() )
    {
      QgsDebugMsgLevel( u"sourceCrs = %1"_s.arg( ct.sourceCrs().authid() ), 3 );
      QgsDebugMsgLevel( u"destCRS = %1"_s.arg( ct.destinationCrs().authid() ), 3 );
      QgsDebugMsgLevel( u"extent = %1"_s.arg( extent.toString() ), 3 );
      extent = ct.transformBoundingBox( extent, Qgis::TransformDirection::Reverse );
    }
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  QgsDebugMsgLevel( u"proj extent =  %1"_s.arg( extent.toString() ), 3 );

  return extent;
}


QgsPointXY QgsMapSettings::layerToMapCoordinates( const QgsMapLayer *layer, QgsPointXY point ) const
{
  try
  {
    const QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
      point = ct.transform( point, Qgis::TransformDirection::Forward );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  return point;
}

QgsPoint QgsMapSettings::layerToMapCoordinates( const QgsMapLayer *layer, const QgsPoint &point ) const
{
  double x = point.x();
  double y = point.y();
  double z = point.z();
  const double m = point.m();

  try
  {
    const QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
      ct.transformInPlace( x, y, z, Qgis::TransformDirection::Forward );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  return QgsPoint( x, y, z, m );
}


QgsRectangle QgsMapSettings::layerToMapCoordinates( const QgsMapLayer *layer, QgsRectangle rect ) const
{
  try
  {
    const QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
      rect = ct.transform( rect, Qgis::TransformDirection::Forward );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  return rect;
}


QgsPointXY QgsMapSettings::mapToLayerCoordinates( const QgsMapLayer *layer, QgsPointXY point ) const
{
  try
  {
    const QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
      point = ct.transform( point, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  return point;
}

QgsPoint QgsMapSettings::mapToLayerCoordinates( const QgsMapLayer *layer, const QgsPoint &point ) const
{
  double x = point.x();
  double y = point.y();
  double z = point.z();
  const double m = point.m();

  try
  {
    const QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
      ct.transformInPlace( x, y, z, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  return QgsPoint( x, y, z, m );
}


QgsRectangle QgsMapSettings::mapToLayerCoordinates( const QgsMapLayer *layer, QgsRectangle rect ) const
{
  try
  {
    const QgsCoordinateTransform ct = layerTransform( layer );
    if ( ct.isValid() )
      rect = ct.transform( rect, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  return rect;
}



QgsRectangle QgsMapSettings::fullExtent() const
{
  // reset the map canvas extent since the extent may now be smaller
  // We can't use a constructor since QgsRectangle normalizes the rectangle upon construction
  QgsRectangle fullExtent;
  fullExtent.setNull();

  // iterate through the map layers and test each layers extent
  // against the current min and max values
  QgsDebugMsgLevel( u"Layer count: %1"_s.arg( mLayers.count() ), 5 );
  const auto constMLayers = mLayers;
  for ( const QgsWeakMapLayerPointer &layerPtr : constMLayers )
  {
    if ( QgsMapLayer *lyr = layerPtr.data() )
    {
      QgsDebugMsgLevel( "Updating extent using " + lyr->name(), 5 );
      QgsDebugMsgLevel( "Input extent: " + lyr->extent().toString(), 5 );

      if ( lyr->extent().isNull() )
        continue;

      // Layer extents are stored in the coordinate system (CS) of the
      // layer. The extent must be projected to the canvas CS
      const QgsRectangle extent = layerExtentToOutputExtent( lyr, lyr->extent() );

      QgsDebugMsgLevel( "Output extent: " + extent.toString(), 5 );
      fullExtent.combineExtentWith( extent );
    }
  }

  if ( fullExtent.width() == 0.0 || fullExtent.height() == 0.0 )
  {
    // If all of the features are at the one point, buffer the
    // rectangle a bit. If they are all at zero, do something a bit
    // more crude.

    if ( fullExtent.xMinimum() == 0.0 && fullExtent.xMaximum() == 0.0 &&
         fullExtent.yMinimum() == 0.0 && fullExtent.yMaximum() == 0.0 )
    {
      fullExtent.set( -1.0, -1.0, 1.0, 1.0 );
    }
    else
    {
      const double padFactor = 1e-8;
      const double widthPad = fullExtent.xMinimum() * padFactor;
      const double heightPad = fullExtent.yMinimum() * padFactor;
      const double xmin = fullExtent.xMinimum() - widthPad;
      const double xmax = fullExtent.xMaximum() + widthPad;
      const double ymin = fullExtent.yMinimum() - heightPad;
      const double ymax = fullExtent.yMaximum() + heightPad;
      fullExtent.set( xmin, ymin, xmax, ymax );
    }
  }

  QgsDebugMsgLevel( "Full extent: " + fullExtent.toString(), 5 );
  return fullExtent;
}


void QgsMapSettings::readXml( QDomNode &node )
{
  // set destination CRS
  QgsCoordinateReferenceSystem srs;
  const QDomNode srsNode = node.namedItem( u"destinationsrs"_s );
  if ( !srsNode.isNull() )
  {
    srs.readXml( srsNode );
  }
  setDestinationCrs( srs );

  // set extent
  const QDomNode extentNode = node.namedItem( u"extent"_s );
  const QgsRectangle aoi = QgsXmlUtils::readRectangle( extentNode.toElement() );
  setExtent( aoi );

  // set rotation
  const QDomNode rotationNode = node.namedItem( u"rotation"_s );
  const QString rotationVal = rotationNode.toElement().text();
  if ( ! rotationVal.isEmpty() )
  {
    const double rot = rotationVal.toDouble();
    setRotation( rot );
  }

  //render map tile
  const QDomElement renderMapTileElem = node.firstChildElement( u"rendermaptile"_s );
  if ( !renderMapTileElem.isNull() )
  {
    setFlag( Qgis::MapSettingsFlag::RenderMapTile, renderMapTileElem.text() == "1"_L1 );
  }
}



void QgsMapSettings::writeXml( QDomNode &node, QDomDocument &doc )
{
  // units
  node.appendChild( QgsXmlUtils::writeMapUnits( mapUnits(), doc ) );

  // Write current view extents
  node.appendChild( QgsXmlUtils::writeRectangle( extent(), doc ) );

  // Write current view rotation
  QDomElement rotNode = doc.createElement( u"rotation"_s );
  rotNode.appendChild(
    doc.createTextNode( qgsDoubleToString( rotation() ) )
  );
  node.appendChild( rotNode );

  // destination CRS
  if ( mDestCRS.isValid() )
  {
    QDomElement srsNode = doc.createElement( u"destinationsrs"_s );
    node.appendChild( srsNode );
    mDestCRS.writeXml( srsNode, doc );
  }

  //render map tile
  QDomElement renderMapTileElem = doc.createElement( u"rendermaptile"_s );
  const QDomText renderMapTileText = doc.createTextNode( testFlag( Qgis::MapSettingsFlag::RenderMapTile ) ? "1" : "0" );
  renderMapTileElem.appendChild( renderMapTileText );
  node.appendChild( renderMapTileElem );
}

QgsGeometry QgsMapSettings::labelBoundaryGeometry() const
{
  return mLabelBoundaryGeometry;
}

void QgsMapSettings::setLabelBoundaryGeometry( const QgsGeometry &boundary )
{
  mLabelBoundaryGeometry = boundary;
}

void QgsMapSettings::addClippingRegion( const QgsMapClippingRegion &region )
{
  mClippingRegions.append( region );
}

void QgsMapSettings::setClippingRegions( const QList<QgsMapClippingRegion> &regions )
{
  mClippingRegions = regions;
}

QList<QgsMapClippingRegion> QgsMapSettings::clippingRegions() const
{
  return mClippingRegions;
}

void QgsMapSettings::setMaskSettings( const QgsMaskRenderSettings &settings )
{
  mMaskRenderSettings = settings;
}

void QgsMapSettings::addRenderedFeatureHandler( QgsRenderedFeatureHandlerInterface *handler )
{
  mRenderedFeatureHandlers.append( handler );
}

QList<QgsRenderedFeatureHandlerInterface *> QgsMapSettings::renderedFeatureHandlers() const
{
  return mRenderedFeatureHandlers;
}

QgsDoubleRange QgsMapSettings::zRange() const
{
  return mZRange;
}

void QgsMapSettings::setZRange( const QgsDoubleRange &zRange )
{
  mZRange = zRange;
}

Qgis::RendererUsage QgsMapSettings::rendererUsage() const
{
  return mRendererUsage;
}

void QgsMapSettings::setRendererUsage( Qgis::RendererUsage rendererUsage )
{
  mRendererUsage = rendererUsage;
}

double QgsMapSettings::frameRate() const
{
  return mFrameRate;
}

void QgsMapSettings::setFrameRate( double rate )
{
  mFrameRate = rate;
}

long long QgsMapSettings::currentFrame() const
{
  return mCurrentFrame;
}

void QgsMapSettings::setCurrentFrame( long long frame )
{
  mCurrentFrame = frame;
}

const QgsElevationShadingRenderer &QgsMapSettings::elevationShadingRenderer() const
{
  return mShadingRenderer;
}

void QgsMapSettings::setElevationShadingRenderer( const QgsElevationShadingRenderer &elevationShadingRenderer )
{
  mShadingRenderer = elevationShadingRenderer;
}

Qgis::RasterizedRenderingPolicy QgsMapSettings::rasterizedRenderingPolicy() const
{
  return mRasterizedRenderingPolicy;
}

void QgsMapSettings::setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy policy )
{
  mRasterizedRenderingPolicy = policy;
  switch ( mRasterizedRenderingPolicy )
  {
    case Qgis::RasterizedRenderingPolicy::Default:
      mFlags.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );
      mFlags.setFlag( Qgis::MapSettingsFlag::UseAdvancedEffects, true );
      break;
    case Qgis::RasterizedRenderingPolicy::PreferVector:
      mFlags.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
      mFlags.setFlag( Qgis::MapSettingsFlag::UseAdvancedEffects, true );
      break;
    case Qgis::RasterizedRenderingPolicy::ForceVector:
      mFlags.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
      mFlags.setFlag( Qgis::MapSettingsFlag::UseAdvancedEffects, false );
      break;
  }
}

