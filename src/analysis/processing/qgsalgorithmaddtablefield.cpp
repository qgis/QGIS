/***************************************************************************
                         qgsalgorithmaddtablefield.cpp
                         -----------------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmaddtablefield.h"

///@cond PRIVATE

QString QgsAddTableFieldAlgorithm::name() const
{
  return QStringLiteral( "addfieldtoattributestable" );
}

QString QgsAddTableFieldAlgorithm::displayName() const
{
  return QObject::tr( "Add field to attributes table" );
}

QString QgsAddTableFieldAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm adds a new attribute to a vector layer.\n\n"
                      "The name and characteristics of the attribute are defined as parameters. The new attribute "
                      "is not added to the input layer but a new layer is generated instead.\n\n" );
}

QStringList QgsAddTableFieldAlgorithm::tags() const
{
  return QObject::tr( "add,create,new,attribute,fields" ).split( ',' );
}

QString QgsAddTableFieldAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsAddTableFieldAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QString QgsAddTableFieldAlgorithm::outputName() const
{
  return QObject::tr( "Added" );
}

QList<int> QgsAddTableFieldAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVector;
}

QgsProcessingFeatureSource::Flag QgsAddTableFieldAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsAddTableFieldAlgorithm *QgsAddTableFieldAlgorithm::createInstance() const
{
  return new QgsAddTableFieldAlgorithm();
}

void QgsAddTableFieldAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field name" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "FIELD_TYPE" ), QObject::tr( "Field type" ),
                QStringList() << QObject::tr( "Integer" ) << QObject::tr( "Float" ) << QObject::tr( "String" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "FIELD_LENGTH" ), QObject::tr( "Field length" ),
                QgsProcessingParameterNumber::Integer, 10, false, 1, 255 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "FIELD_PRECISION" ), QObject::tr( "Field precision" ),
                QgsProcessingParameterNumber::Integer, 0, false, 0, 10 ) );
}

QgsFields QgsAddTableFieldAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  outFields.append( QgsField( mField ) );
  return outFields;
}

bool QgsAddTableFieldAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QString name = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  int type = parameterAsInt( parameters, QStringLiteral( "FIELD_TYPE" ), context );
  int length = parameterAsInt( parameters, QStringLiteral( "FIELD_LENGTH" ), context );
  int precision = parameterAsInt( parameters, QStringLiteral( "FIELD_PRECISION" ), context );

  mField.setName( name );
  mField.setLength( length );
  mField.setPrecision( precision );

  switch ( type )
  {
    case 0:
      mField.setType( QVariant::Int );
      break;
    case 1:
      mField.setType( QVariant::Double );
      break;
    case 2:
      mField.setType( QVariant::String );
      break;
  }

  return true;
}

QgsFeatureList QgsAddTableFieldAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  QgsAttributes attributes = f.attributes();
  attributes.append( QVariant() );
  f.setAttributes( attributes );
  return QgsFeatureList() << f;
}

bool QgsAddTableFieldAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
