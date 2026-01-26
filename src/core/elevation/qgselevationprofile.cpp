/***************************************************************************
    qgselevationprofile.cpp
    ------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationprofile.h"

#include "qgscurve.h"
#include "qgslayertree.h"
#include "qgslinesymbol.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"

#include "moc_qgselevationprofile.cpp"

QgsElevationProfile::QgsElevationProfile( QgsProject *project )
  : mProject( project )
  , mLayerTree( std::make_unique< QgsLayerTree >() )
{
  setupLayerTreeConnections();
}

QgsElevationProfile::~QgsElevationProfile() = default;

QDomElement QgsElevationProfile::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement profileElem = document.createElement( u"ElevationProfile"_s );
  profileElem.setAttribute( u"name"_s, mName );

  profileElem.setAttribute( u"distanceUnit"_s, qgsEnumValueToKey( mDistanceUnit ) );

  profileElem.setAttribute( u"tolerance"_s, mTolerance );
  if ( mLockAxisScales )
    profileElem.setAttribute( u"lockAxisScales"_s, u"1"_s );

  if ( mCrs.isValid() )
  {
    QDomElement crsElem = document.createElement( u"crs"_s );
    mCrs.writeXml( crsElem, document );
    profileElem.appendChild( crsElem );
  }
  if ( mProfileCurve )
  {
    QDomElement curveElem = document.createElement( u"curve"_s );
    curveElem.appendChild( document.createTextNode( mProfileCurve->asWkt( ) ) );
    profileElem.appendChild( curveElem );
  }

  if ( !mUseProjectLayerTree )
  {
    mLayerTree->writeXml( profileElem, context );
  }
  else
  {
    profileElem.setAttribute( u"useProjectLayerTree"_s, u"1"_s );
  }

  if ( mSubsectionsSymbol )
  {
    QDomElement subsectionsElement = document.createElement( u"subsections"_s );
    const QDomElement symbolElement = QgsSymbolLayerUtils::saveSymbol( u"subsections"_s, mSubsectionsSymbol.get(), document, context );
    subsectionsElement.appendChild( symbolElement );
    profileElem.appendChild( subsectionsElement );
  }

  return profileElem;
}

bool QgsElevationProfile::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  if ( element.nodeName() != "ElevationProfile"_L1 )
  {
    return false;
  }

  setName( element.attribute( u"name"_s ) );

  const QDomNodeList crsNodeList = element.elementsByTagName( u"crs"_s );
  QgsCoordinateReferenceSystem crs;
  if ( !crsNodeList.isEmpty() )
  {
    const QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    crs.readXml( crsElem );
  }
  mCrs = crs;

  setDistanceUnit( qgsEnumKeyToValue( element.attribute( u"distanceUnit"_s ), mCrs.mapUnits() ) );

  const QDomNodeList curveNodeList = element.elementsByTagName( u"curve"_s );
  if ( !curveNodeList.isEmpty() )
  {
    const QDomElement curveElem = curveNodeList.at( 0 ).toElement();
    const QgsGeometry curve = QgsGeometry::fromWkt( curveElem.text() );
    // clang-tidy false positive
    // NOLINTBEGIN(bugprone-branch-clone)
    if ( const QgsCurve *curveGeom = qgsgeometry_cast< const QgsCurve * >( curve.constGet() ) )
    {
      mProfileCurve.reset( curveGeom->clone() );
    }
    else
    {
      mProfileCurve.reset();
    }
    // NOLINTEND(bugprone-branch-clone)
  }

  mTolerance = element.attribute( u"tolerance"_s ).toDouble();
  mLockAxisScales = element.attribute( u"lockAxisScales"_s, u"0"_s ).toInt();

  setUseProjectLayerTree( element.attribute( u"useProjectLayerTree"_s, u"0"_s ).toInt() );
  if ( !mUseProjectLayerTree )
  {
    const QDomElement layerTreeElem = element.firstChildElement( u"layer-tree-group"_s );
    mLayerTree = QgsLayerTree::readXml( layerTreeElem, context );
    setupLayerTreeConnections();
  }

  const QDomElement subsectionsElement = element.firstChildElement( u"subsections"_s );
  const QDomElement symbolsElement = subsectionsElement.firstChildElement( u"symbol"_s );
  if ( !symbolsElement.isNull() )
  {
    std::unique_ptr< QgsLineSymbol > subSectionsSymbol = QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol >( symbolsElement, context );
    if ( subSectionsSymbol )
    {
      setSubsectionsSymbol( subSectionsSymbol.release() );
    }
  }

  return true;
}

void QgsElevationProfile::resolveReferences( const QgsProject *project )
{
  mLayerTree->resolveReferences( project );
}

QIcon QgsElevationProfile::icon() const
{
  return QIcon();
}

QgsLayerTree *QgsElevationProfile::layerTree()
{
  return !mUseProjectLayerTree ? mLayerTree.get() : nullptr;
}

void QgsElevationProfile::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mCrs == crs )
    return;

  mCrs = crs;
  dirtyProject();
}

QgsCoordinateReferenceSystem QgsElevationProfile::crs() const
{
  return mCrs;
}

void QgsElevationProfile::setProfileCurve( QgsCurve *curve )
{
  if ( curve == mProfileCurve.get() )
    return;
  mProfileCurve.reset( curve );
  dirtyProject();
}

QgsCurve *QgsElevationProfile::profileCurve() const
{
  return mProfileCurve.get();
}

void QgsElevationProfile::setTolerance( double tolerance )
{
  if ( qgsDoubleNear( tolerance, mTolerance ) )
    return;

  mTolerance = tolerance;
  dirtyProject();
}

double QgsElevationProfile::tolerance() const
{
  return mTolerance;
}

bool QgsElevationProfile::lockAxisScales() const
{
  return mLockAxisScales;
}

Qgis::DistanceUnit QgsElevationProfile::distanceUnit() const
{
  return mDistanceUnit;
}

void QgsElevationProfile::setLockAxisScales( bool lock )
{
  if ( lock == mLockAxisScales )
    return;

  mLockAxisScales = lock;
  dirtyProject();
}

void QgsElevationProfile::setDistanceUnit( Qgis::DistanceUnit unit )
{
  if ( mDistanceUnit == unit )
    return;

  mDistanceUnit = unit;
  dirtyProject();
}

void QgsElevationProfile::setUseProjectLayerTree( bool useProjectTree )
{
  if ( mUseProjectLayerTree == useProjectTree )
    return;

  mUseProjectLayerTree = useProjectTree;
  emit useProjectLayerTreeChanged( mUseProjectLayerTree );
  dirtyProject();
}

void QgsElevationProfile::dirtyProject()
{
  if ( mProject )
    mProject->setDirty();
}

void QgsElevationProfile::setupLayerTreeConnections()
{
  connect( mLayerTree.get(), &QgsLayerTree::layerOrderChanged, this, &QgsElevationProfile::dirtyProject );
  connect( mLayerTree.get(), &QgsLayerTree::visibilityChanged, this, &QgsElevationProfile::dirtyProject );
  connect( mLayerTree.get(), &QgsLayerTree::nameChanged, this, &QgsElevationProfile::dirtyProject );
}

QgsLineSymbol *QgsElevationProfile::subsectionsSymbol()
{
  return mSubsectionsSymbol.get();
}

void QgsElevationProfile::setSubsectionsSymbol( QgsLineSymbol *symbol )
{
  if ( symbol == mSubsectionsSymbol.get() )
    return;

  mSubsectionsSymbol.reset( symbol );
  dirtyProject();
}

void QgsElevationProfile::setName( const QString &name )
{
  if ( name == mName )
    return;

  mName = name;
  dirtyProject();
  emit nameChanged( mName );
}
