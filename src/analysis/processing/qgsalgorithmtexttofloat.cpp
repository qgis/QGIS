/***************************************************************************
                         qgsalgorithmtexttofloat.cpp
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#include "qgsalgorithmtexttofloat.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsTextToFloatAlgorithm::name() const
{
  return u"texttofloat"_s;
}

QString QgsTextToFloatAlgorithm::displayName() const
{
  return QObject::tr( "Text to float" );
}

QString QgsTextToFloatAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm modifies the type of a given attribute in a vector layer, "
    "converting a text attribute containing numeric strings into a numeric attribute.\n\n"
    "Values containing a '%' symbol (e.g., '12.5%') will have the percentage sign removed "
    "and will be converted to their decimal equivalent (e.g., 0.125).\n"
    "Any non-numeric or invalid string values that cannot be parsed into a float will be "
    "replaced with NULL."
  );
}

QString QgsTextToFloatAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts a text attribute containing numeric strings into a numeric attribute." );
}

QStringList QgsTextToFloatAlgorithm::tags() const
{
  return QObject::tr( "convert,string,number,double,cast,type" ).split( ',' );
}

QString QgsTextToFloatAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsTextToFloatAlgorithm::groupId() const
{
  return u"vectortable"_s;
}

QString QgsTextToFloatAlgorithm::outputName() const
{
  return QObject::tr( "Float from text" );
}

QList<int> QgsTextToFloatAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector );
}

QgsTextToFloatAlgorithm *QgsTextToFloatAlgorithm::createInstance() const
{
  return new QgsTextToFloatAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsTextToFloatAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

bool QgsTextToFloatAlgorithm::supportInPlaceEdit( const QgsMapLayer * ) const
{
  return false;
}

void QgsTextToFloatAlgorithm::initParameters( const QVariantMap & )
{
  auto fieldParam = std::make_unique<QgsProcessingParameterField>( u"FIELD"_s, QObject::tr( "Text attribute to convert to float" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::String );
  fieldParam->setHelp( QObject::tr( "Text/string field containing numeric values to convert. Supports standard numeric strings as well as formatted percentage strings." ) );
  addParameter( fieldParam.release() );
}

QgsFields QgsTextToFloatAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QList<QgsField> fieldList = inputFields.toList();
  fieldList[mFieldIndex] = QgsField( mFieldName, QMetaType::Type::Double, u"Double"_s, 24, 15 );

  QgsFields outFields( fieldList );
  return outFields;
}

bool QgsTextToFloatAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mFieldName = parameterAsString( parameters, u"FIELD"_s, context );
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( source )
  {
    mFieldIndex = source->fields().lookupField( mFieldName );
    if ( mFieldIndex < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Field “%1” does not exist in input layer " ).arg( mFieldName ) );
    }
  }

  return true;
}

QgsFeatureList QgsTextToFloatAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QGS_MARK_ALGORITHM_SOURCE

  QgsFeature outFeature = feature;

  const QVariant value = outFeature.attribute( mFieldIndex );
  bool ok = false;

  if ( !QgsVariantUtils::isNull( value ) )
  {
    QString stringValue = value.toString().trimmed();
    if ( stringValue.endsWith( '%' ) )
    {
      stringValue.chop( 1 );
      double d = stringValue.toDouble( &ok );
      if ( ok )
      {
        outFeature.setAttribute( mFieldIndex, d / 100.0 );
      }
    }
    else
    {
      double d = stringValue.toDouble( &ok );
      if ( ok )
      {
        outFeature.setAttribute( mFieldIndex, d );
      }
    }
  }

  if ( !ok )
  {
    outFeature.setAttribute( mFieldIndex, QVariant() );
  }

  return QgsFeatureList() << outFeature;
}

///@endcond
