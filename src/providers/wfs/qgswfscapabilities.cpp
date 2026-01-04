/***************************************************************************
    qgswfscapabilities.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
                           (C) 2016 by Even Rouault
    email                : wonder dot sk at gmail dot com
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfscapabilities.h"

#include "qgsogcutils.h"
#include "qgswfsutils.h"

QgsWfsCapabilities::QgsWfsCapabilities()
{
  clear();
}

void QgsWfsCapabilities::clear()
{
  maxFeatures = 0;
  supportsHits = false;
  supportsPaging = false;
  supportsJoins = false;
  version.clear();
  featureTypes.clear();
  spatialPredicatesList.clear();
  functionList.clear();
  setAllTypenames.clear();
  mapUnprefixedTypenameToPrefixedTypename.clear();
  setAmbiguousUnprefixedTypename.clear();
  useEPSGColumnFormat = false;
}

QString QgsWfsCapabilities::addPrefixIfNeeded( const QString &name ) const
{
  if ( name.contains( ':' ) )
    return name;
  if ( setAmbiguousUnprefixedTypename.contains( name ) )
    return QString();
  return mapUnprefixedTypenameToPrefixedTypename[name];
}

QString QgsWfsCapabilities::getNamespaceForTypename( const QString &name ) const
{
  for ( const QgsWfsCapabilities::FeatureType &f : featureTypes )
  {
    if ( f.name == name )
    {
      return f.nameSpace;
    }
  }
  return "";
}

QString QgsWfsCapabilities::getNamespaceParameterValue( const QString &WFSVersion, const QString &typeName ) const
{
  QString namespaces = getNamespaceForTypename( typeName );
  bool tryNameSpacing = ( !namespaces.isEmpty() && typeName.contains( ':' ) );
  if ( tryNameSpacing )
  {
    QString prefixOfTypename = QgsWFSUtils::nameSpacePrefix( typeName );
    return "xmlns(" + prefixOfTypename + ( WFSVersion.startsWith( "2.0"_L1 ) ? "," : "=" ) + namespaces + ")";
  }
  return QString();
}

bool QgsWfsCapabilities::supportsGeometryTypeFilters() const
{
  // Detect servers, such as Deegree, that expose additional filter functions
  // to test if a geometry is a (multi)point, (multi)curve or (multi)surface
  // This can be used to figure out which geometry types are present in layers
  // that describe a generic geometry type.
  bool hasIsPoint = false;
  bool hasIsCurve = false;
  bool hasIsSurface = false;
  for ( const auto &function : functionList )
  {
    if ( function.minArgs == 1 && function.maxArgs == 1 )
    {
      if ( function.name == "IsPoint"_L1 )
      {
        hasIsPoint = true;
      }
      else if ( function.name == "IsCurve"_L1 )
      {
        hasIsCurve = true;
      }
      else if ( function.name == "IsSurface"_L1 )
      {
        hasIsSurface = true;
      }
    }
  }
  return hasIsPoint && hasIsCurve && hasIsSurface;
}
