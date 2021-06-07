/***************************************************************************
  qgsgeocoder.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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

QgsWkbTypes::Type QgsGeocoderInterface::wkbType() const { return QgsWkbTypes::Unknown; }

QList<QgsGeocoderResult> QgsGeocoderInterface::geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback * ) const
{
  Q_UNUSED( string )
  Q_UNUSED( context )
  return QList< QgsGeocoderResult >();
}
