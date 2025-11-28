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
#include "qgsproject.h"
#include "moc_qgselevationprofile.cpp"
#include "qgscurve.h"
#include "qgslinesymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslayertree.h"

QgsElevationProfile::QgsElevationProfile( QgsProject *project )
  : mProject( project )
  , mLayerTree( std::make_unique< QgsLayerTree >() )
{
  setupLayerTreeConnections();
}

QgsElevationProfile::~QgsElevationProfile() = default;

QDomElement QgsElevationProfile::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement profileElem = document.createElement( QStringLiteral( "ElevationProfile" ) );
  profileElem.setAttribute( QStringLiteral( "name" ), mName );

  profileElem.setAttribute( QStringLiteral( "distanceUnit" ), qgsEnumValueToKey( mDistanceUnit ) );

  profileElem.setAttribute( QStringLiteral( "tolerance" ), mTolerance );
  if ( mLockAxisScales )
    profileElem.setAttribute( QStringLiteral( "lockAxisScales" ), QStringLiteral( "1" ) );

  if ( mCrs.isValid() )
  {
    QDomElement crsElem = document.createElement( QStringLiteral( "crs" ) );
    mCrs.writeXml( crsElem, document );
    profileElem.appendChild( crsElem );
  }
  if ( mProfileCurve )
  {
    QDomElement curveElem = document.createElement( QStringLiteral( "curve" ) );
    curveElem.appendChild( document.createTextNode( mProfileCurve->asWkt( ) ) );
    profileElem.appendChild( curveElem );
  }

  mLayerTree->writeXml( profileElem, context );

  if ( mSubsectionsSymbol )
  {
    QDomElement subsectionsElement = document.createElement( QStringLiteral( "subsections" ) );
    const QDomElement symbolElement = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "subsections" ), mSubsectionsSymbol.get(), document, context );
    subsectionsElement.appendChild( symbolElement );
    profileElem.appendChild( subsectionsElement );
  }

  return profileElem;
}

bool QgsElevationProfile::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  if ( element.nodeName() != QLatin1String( "ElevationProfile" ) )
  {
    return false;
  }

  setName( element.attribute( QStringLiteral( "name" ) ) );

  const QDomNodeList crsNodeList = element.elementsByTagName( QStringLiteral( "crs" ) );
  QgsCoordinateReferenceSystem crs;
  if ( !crsNodeList.isEmpty() )
  {
    const QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    crs.readXml( crsElem );
  }
  mCrs = crs;

  setDistanceUnit( qgsEnumKeyToValue( element.attribute( QStringLiteral( "distanceUnit" ) ), mCrs.mapUnits() ) );

  const QDomNodeList curveNodeList = element.elementsByTagName( QStringLiteral( "curve" ) );
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

  mTolerance = element.attribute( QStringLiteral( "tolerance" ) ).toDouble();
  mLockAxisScales = element.attribute( QStringLiteral( "lockAxisScales" ), QStringLiteral( "0" ) ).toInt();

  {
    const QDomElement layerTreeElem = element.firstChildElement( QStringLiteral( "layer-tree-group" ) );
    mLayerTree = QgsLayerTree::readXml( layerTreeElem, context );
    setupLayerTreeConnections();
  }

  const QDomElement subsectionsElement = element.firstChildElement( QStringLiteral( "subsections" ) );
  const QDomElement symbolsElement = subsectionsElement.firstChildElement( QStringLiteral( "symbol" ) );
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
  return mLayerTree.get();
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
