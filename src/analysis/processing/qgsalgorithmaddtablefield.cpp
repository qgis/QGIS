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
#include "qgsvariantutils.h"

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
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector );
}

Qgis::ProcessingFeatureSourceFlags QgsAddTableFieldAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsAddTableFieldAlgorithm *QgsAddTableFieldAlgorithm::createInstance() const
{
  return new QgsAddTableFieldAlgorithm();
}

void QgsAddTableFieldAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field name" ) ) );

  QStringList typeStrings;
  QVariantList icons;
  typeStrings.reserve( 11 );
  icons.reserve( 11 );
  for ( const auto &type :
        std::vector<std::pair<QMetaType::Type, QMetaType::Type>> {
          { QMetaType::Type::Int, QMetaType::Type::UnknownType },
          { QMetaType::Type::Double, QMetaType::Type::UnknownType },
          { QMetaType::Type::QString, QMetaType::Type::UnknownType },
          { QMetaType::Type::Bool, QMetaType::Type::UnknownType },
          { QMetaType::Type::QDate, QMetaType::Type::UnknownType },
          { QMetaType::Type::QTime, QMetaType::Type::UnknownType },
          { QMetaType::Type::QDateTime, QMetaType::Type::UnknownType },
          { QMetaType::Type::QByteArray, QMetaType::Type::UnknownType },
          { QMetaType::Type::QStringList, QMetaType::Type::UnknownType },
          { QMetaType::Type::QVariantList, QMetaType::Type::Int },
          { QMetaType::Type::QVariantList, QMetaType::Type::Double }
        } )
  {
    typeStrings << QgsVariantUtils::typeToDisplayString( type.first, type.second );
    icons << QgsFields::iconForFieldType( type.first, type.second );
  }

  std::unique_ptr<QgsProcessingParameterEnum> fieldTypes = std::make_unique<QgsProcessingParameterEnum>( QStringLiteral( "FIELD_TYPE" ), QObject::tr( "Field type" ), typeStrings, false, 0 );
  fieldTypes->setMetadata(
    { QVariantMap( { { QStringLiteral( "widget_wrapper" ), QVariantMap( { { QStringLiteral( "icons" ), icons } } ) } } )
    }
  );
  addParameter( fieldTypes.release() );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "FIELD_LENGTH" ), QObject::tr( "Field length" ), Qgis::ProcessingNumberParameterType::Integer, 10, false, 1, 255 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "FIELD_PRECISION" ), QObject::tr( "Field precision" ), Qgis::ProcessingNumberParameterType::Integer, 0, false, 0, 10 ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_ALIAS" ), QObject::tr( "Field alias" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_COMMENT" ), QObject::tr( "Field comment" ), QVariant(), false, true ) );
}

QgsFields QgsAddTableFieldAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  outFields.append( QgsField( mField ) );
  return outFields;
}

bool QgsAddTableFieldAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QString name = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  const int type = parameterAsInt( parameters, QStringLiteral( "FIELD_TYPE" ), context );
  const int length = parameterAsInt( parameters, QStringLiteral( "FIELD_LENGTH" ), context );
  const int precision = parameterAsInt( parameters, QStringLiteral( "FIELD_PRECISION" ), context );
  const QString alias = parameterAsString( parameters, QStringLiteral( "FIELD_ALIAS" ), context );
  const QString comment = parameterAsString( parameters, QStringLiteral( "FIELD_COMMENT" ), context );

  mField.setName( name );
  mField.setLength( length );
  mField.setPrecision( precision );
  mField.setAlias( alias );
  mField.setComment( comment );

  switch ( type )
  {
    case 0: // Integer
      mField.setType( QMetaType::Type::Int );
      break;
    case 1: // Float
      mField.setType( QMetaType::Type::Double );
      break;
    case 2: // String
      mField.setType( QMetaType::Type::QString );
      break;
    case 3: // Boolean
      mField.setType( QMetaType::Type::Bool );
      break;
    case 4: // Date
      mField.setType( QMetaType::Type::QDate );
      break;
    case 5: // Time
      mField.setType( QMetaType::Type::QTime );
      break;
    case 6: // DateTime
      mField.setType( QMetaType::Type::QDateTime );
      break;
    case 7: // Binary
      mField.setType( QMetaType::Type::QByteArray );
      break;
    case 8: // StringList
      mField.setType( QMetaType::Type::QStringList );
      mField.setSubType( QMetaType::Type::QString );
      break;
    case 9: // IntegerList
      mField.setType( QMetaType::Type::QVariantList );
      mField.setSubType( QMetaType::Type::Int );
      break;
    case 10: // DoubleList
      mField.setType( QMetaType::Type::QVariantList );
      mField.setSubType( QMetaType::Type::Double );
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
