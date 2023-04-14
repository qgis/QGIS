/***************************************************************************
  qgsgeocoder.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocoder.h"

QList<QgsGeocoderResult> QgsGeocoderInterface::geocodeFeature( const QgsFeature &feature, const QgsGeocoderContext &context, QgsFeedback * ) const
{
  Q_UNUSED( feature )
  Q_UNUSED( context )
  return QList< QgsGeocoderResult >();
}

QgsFields QgsGeocoderInterface::appendedFields() const { return QgsFields(); }

Qgis::WkbType QgsGeocoderInterface::wkbType() const { return Qgis::WkbType::Unknown; }

QList<QgsGeocoderResult> QgsGeocoderInterface::geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback * ) const
{
  Q_UNUSED( string )
  Q_UNUSED( context )
  return QList< QgsGeocoderResult >();
}
