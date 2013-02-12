/***************************************************************************
    qgsfeaturerequest.cpp
    ---------------------
    begin                : Mai 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturerequest.h"

#include "qgsfield.h"

#include <QStringList>

QgsFeatureRequest::QgsFeatureRequest()
    : mFilter( FilterNone )
    , mFlags( 0 )
{
}


QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields )
{
  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( attrNames.contains( fields[idx].name() ) )
      mAttrs.append( idx );
  }

  return *this;
}
