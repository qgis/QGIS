/***************************************************************************
                         qgsalgorithmbatchnominatimgeocode.cpp
                         ------------------
    begin                : December 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmbatchgeocode.h"
#include "qgsalgorithmbatchnominatimgeocode.h"
#include "qgsgeocoder.h"
#include "qgsgeocoderresult.h"
#include "qgsgeocodercontext.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QgsBatchNominatimGeocodeAlgorithm::QgsBatchNominatimGeocodeAlgorithm()
  : QgsBatchGeocodeAlgorithm( &mNominatimGeocoder )
{
}

QString QgsBatchNominatimGeocodeAlgorithm::name() const
{
  return QStringLiteral( "batchnominatimgeocoder" );
}

QString QgsBatchNominatimGeocodeAlgorithm::displayName() const
{
  return QObject::tr( "Batch Nominatim geocoder" );
}

QStringList QgsBatchNominatimGeocodeAlgorithm::tags() const
{
  return QObject::tr( "geocode,nominatim,batch,bulk,address,match" ).split( ',' );
}

QgsCoordinateReferenceSystem QgsBatchNominatimGeocodeAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mOutputCrs = inputCrs.isValid() ? inputCrs : QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  return mOutputCrs;
}

QgsBatchNominatimGeocodeAlgorithm *QgsBatchNominatimGeocodeAlgorithm::createInstance() const
{
  return new QgsBatchNominatimGeocodeAlgorithm();
}

QString QgsBatchNominatimGeocodeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm performs batch geocoding using the <a href=\"#\">Nominatim</a> service against an input layer string field.\n\n"
                      "The output layer will have a point geometry reflecting the geocoded location as well as a number of attributes associated to the geocoded location." );
}

bool QgsBatchNominatimGeocodeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  feedback->pushInfo( QObject::tr( "The Nominatim geocoder data is made available by OpenStreetMap Foundation and contributors. "
                                   "It is provided under the ODbL license which requires to share alike. Visit https://nominatim.org/ to learn more." ) );
  return QgsBatchGeocodeAlgorithm::prepareAlgorithm( parameters, context, feedback );
}

///@endcond
