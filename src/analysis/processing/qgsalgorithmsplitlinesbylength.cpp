/***************************************************************************
                         qgsalgorithmsplitlinesbylength.cpp
                         ---------------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmsplitlinesbylength.h"

#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgscurve.h"
#include "qgsgeometrycollection.h"
#include "qgslinestring.h"

///@cond PRIVATE

QString QgsSplitLinesByLengthAlgorithm::name() const
{
  return u"splitlinesbylength"_s;
}

QString QgsSplitLinesByLengthAlgorithm::displayName() const
{
  return QObject::tr( "Split lines by maximum length" );
}

QStringList QgsSplitLinesByLengthAlgorithm::tags() const
{
  return QObject::tr( "segments,parts,distance,cut,chop" ).split( ',' );
}

QString QgsSplitLinesByLengthAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSplitLinesByLengthAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsSplitLinesByLengthAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a line (or curve) layer and splits each feature into multiple parts, "
                      "where each part is of a specified maximum length.\n\n"
                      "Z and M values at the start and end of the new line substrings are linearly interpolated from existing values." );
}

QString QgsSplitLinesByLengthAlgorithm::shortDescription() const
{
  return QObject::tr( "Splits lines into parts which are not longer than a specified length." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsSplitLinesByLengthAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QList<int> QgsSplitLinesByLengthAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

Qgis::ProcessingSourceType QgsSplitLinesByLengthAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorLine;
}

QgsSplitLinesByLengthAlgorithm *QgsSplitLinesByLengthAlgorithm::createInstance() const
{
  return new QgsSplitLinesByLengthAlgorithm();
}

void QgsSplitLinesByLengthAlgorithm::initParameters( const QVariantMap & )
{
  auto length = std::make_unique<QgsProcessingParameterDistance>( u"LENGTH"_s, QObject::tr( "Maximum line length" ), 10, u"INPUT"_s, false, 0 );
  length->setIsDynamic( true );
  length->setDynamicPropertyDefinition( QgsPropertyDefinition( u"LENGTH"_s, QObject::tr( "Maximum length" ), QgsPropertyDefinition::DoublePositive ) );
  length->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( length.release() );
}

bool QgsSplitLinesByLengthAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mLength = parameterAsDouble( parameters, u"LENGTH"_s, context );
  mDynamicLength = QgsProcessingParameters::isDynamic( parameters, u"LENGTH"_s );
  if ( mDynamicLength )
    mLengthProperty = parameters.value( u"LENGTH"_s ).value<QgsProperty>();

  return true;
}

QString QgsSplitLinesByLengthAlgorithm::outputName() const
{
  return QObject::tr( "Split" );
}

Qgis::WkbType QgsSplitLinesByLengthAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  return QgsWkbTypes::singleType( inputWkbType );
}

QgsFields QgsSplitLinesByLengthAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( "order", QMetaType::Type::Int ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

QgsFeatureList QgsSplitLinesByLengthAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !f.hasGeometry() )
  {
    return QgsFeatureList() << f;
  }
  else
  {
    double distance = mLength;
    if ( mDynamicLength )
      distance = mLengthProperty.valueAsDouble( context.expressionContext(), distance );

    QgsFeature outputFeature;
    QgsFeatureList features;
    const QgsGeometry inputGeom = f.geometry();
    int order = 0;
    for ( auto it = inputGeom.const_parts_begin(); it != inputGeom.const_parts_end(); ++it )
    {
      const QgsCurve *part = qgsgeometry_cast<const QgsCurve *>( *it );
      if ( !part )
        continue;

      double start = 0.0;
      double end = distance;
      const double length = part->length();
      while ( start < length )
      {
        outputFeature.setGeometry( QgsGeometry( part->curveSubstring( start, end ) ) );
        outputFeature.setAttributes( f.attributes() << order );
        order++;
        start += distance;
        end += distance;
        features << outputFeature;
      }
    }
    return features;
  }
}

Qgis::ProcessingFeatureSourceFlags QgsSplitLinesByLengthAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsFeatureSink::SinkFlags QgsSplitLinesByLengthAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}


///@endcond
