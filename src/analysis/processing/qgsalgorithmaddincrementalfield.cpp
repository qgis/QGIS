/***************************************************************************
                         qgsalgorithmaddincrementalfield.cpp
                         -----------------------------------
    begin                : April 2017
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

#include "qgsalgorithmaddincrementalfield.h"

///@cond PRIVATE

QString QgsAddIncrementalFieldAlgorithm::name() const
{
  return QStringLiteral( "addautoincrementalfield" );
}

QString QgsAddIncrementalFieldAlgorithm::displayName() const
{
  return QObject::tr( "Add autoincremental field" );
}

QString QgsAddIncrementalFieldAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm adds a new integer field to a vector layer, with a sequential value for each feature.\n\n"
                      "This field can be used as a unique ID for features in the layer. The new attribute "
                      "is not added to the input layer but a new layer is generated instead.\n\n"
                      "The initial starting value for the incremental series can be specified." );
}

QStringList QgsAddIncrementalFieldAlgorithm::tags() const
{
  return QObject::tr( "add,create,serial,primary,key,unique,field" ).split( ',' );
}

QString QgsAddIncrementalFieldAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsAddIncrementalFieldAlgorithm::outputName() const
{
  return QObject::tr( "Incremented" );
}

QList<int> QgsAddIncrementalFieldAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVector;
}

QgsAddIncrementalFieldAlgorithm *QgsAddIncrementalFieldAlgorithm::createInstance() const
{
  return new QgsAddIncrementalFieldAlgorithm();
}

void QgsAddIncrementalFieldAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field name" ), QStringLiteral( "AUTO" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "START" ), QObject::tr( "Start values at" ),
                QgsProcessingParameterNumber::Integer, 0, true ) );
}

QgsFields QgsAddIncrementalFieldAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  outFields.append( QgsField( mFieldName, QVariant::LongLong ) );
  return outFields;
}

bool QgsAddIncrementalFieldAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mValue = parameterAsInt( parameters, QStringLiteral( "START" ), context );
  mFieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  return true;
}

QgsFeature QgsAddIncrementalFieldAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  QgsAttributes attributes = f.attributes();
  attributes.append( mValue );
  mValue++;
  f.setAttributes( attributes );
  return f;
}

///@endcond
