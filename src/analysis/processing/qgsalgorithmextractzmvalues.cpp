/***************************************************************************
                         qgsalgorithmextractzmvalues.cpp
                         ---------------------------------
    begin                : January 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmextractzmvalues.h"
#include "qgsfeaturerequest.h"
#include <vector>

///@cond PRIVATE

const std::vector< QgsStatisticalSummary::Statistic > STATS
{
  QgsStatisticalSummary::First,
  QgsStatisticalSummary::Last,
  QgsStatisticalSummary::Count,
  QgsStatisticalSummary::Sum,
  QgsStatisticalSummary::Mean,
  QgsStatisticalSummary::Median,
  QgsStatisticalSummary::StDev,
  QgsStatisticalSummary::Min,
  QgsStatisticalSummary::Max,
  QgsStatisticalSummary::Range,
  QgsStatisticalSummary::Minority,
  QgsStatisticalSummary::Majority,
  QgsStatisticalSummary::Variety,
  QgsStatisticalSummary::FirstQuartile,
  QgsStatisticalSummary::ThirdQuartile,
  QgsStatisticalSummary::InterQuartileRange,
};

QString QgsExtractZMValuesAlgorithmBase::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExtractZMValuesAlgorithmBase::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExtractZMValuesAlgorithmBase::outputName() const
{
  return QObject::tr( "Extracted" );
}

QList<int> QgsExtractZMValuesAlgorithmBase::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorAnyGeometry;
}

QgsProcessingFeatureSource::Flag QgsExtractZMValuesAlgorithmBase::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

void QgsExtractZMValuesAlgorithmBase::initParameters( const QVariantMap & )
{
  QStringList statChoices;
  statChoices.reserve( STATS.size() );
  for ( QgsStatisticalSummary::Statistic stat : STATS )
  {
    statChoices << QgsStatisticalSummary::displayName( stat );
  }

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "SUMMARIES" ),
                QObject::tr( "Summaries to calculate" ),
                statChoices, true, QVariantList() << 0 ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "COLUMN_PREFIX" ), QObject::tr( "Output column prefix" ), mDefaultFieldPrefix, false, true ) );
}

QgsFields QgsExtractZMValuesAlgorithmBase::outputFields( const QgsFields &inputFields ) const
{
  return QgsProcessingUtils::combineFields( inputFields, mNewFields );
}

bool QgsExtractZMValuesAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mPrefix = parameterAsString( parameters, QStringLiteral( "COLUMN_PREFIX" ), context );

  const QList< int > stats = parameterAsEnums( parameters, QStringLiteral( "SUMMARIES" ), context );
  mStats = QgsStatisticalSummary::Statistics();
  for ( int s : stats )
  {
    mStats |= STATS.at( s );
    mSelectedStats << STATS.at( s );
    mNewFields.append( QgsField( mPrefix + QgsStatisticalSummary::shortName( STATS.at( s ) ), STATS.at( s ) == QgsStatisticalSummary::Count || STATS.at( s ) == QgsStatisticalSummary::Variety ? QVariant::Int : QVariant::Double ) );
  }

  return true;
}

QgsFeatureList QgsExtractZMValuesAlgorithmBase::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  QgsAttributes attrs = f.attributes();
  attrs.reserve( attrs.count() + mSelectedStats.count() );
  if ( !f.hasGeometry() || f.geometry().isEmpty() || !mTestGeomFunc( f.geometry() ) )
  {
    attrs.resize( attrs.count() + mNewFields.size() );
  }
  else
  {
    const QgsGeometry g = f.geometry();
    QgsStatisticalSummary stat( mStats );
    for ( auto it = g.vertices_begin(); it != g.vertices_end(); ++it )
    {
      stat.addValue( mExtractValFunc( *it ) );
      if ( mStats == QgsStatisticalSummary::First )
      {
        // only retrieving first vertex info (default behavior), so short cut and
        // don't iterate remaining vertices
        break;
      }
    }
    stat.finalize();

    for ( QgsStatisticalSummary::Statistic s : qgis::as_const( mSelectedStats ) )
      attrs.append( stat.statistic( s ) );
  }
  f.setAttributes( attrs );

  return QgsFeatureList() << f;
}

bool QgsExtractZMValuesAlgorithmBase::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

//
// QgsExtractZValuesAlgorithm
//

QgsExtractZValuesAlgorithm::QgsExtractZValuesAlgorithm()
{
  mExtractValFunc = []( const QgsPoint & p ) -> double
  {
    return p.z();
  };
  mTestGeomFunc = []( const QgsGeometry & g ) -> bool
  {
    return QgsWkbTypes::hasZ( g.wkbType() );
  };
  mDefaultFieldPrefix = QStringLiteral( "z_" );
}

QString QgsExtractZValuesAlgorithm::name() const
{
  return QStringLiteral( "extractzvalues" );
}

QString QgsExtractZValuesAlgorithm::displayName() const
{
  return QObject::tr( "Extract Z values" );
}

QgsProcessingAlgorithm *QgsExtractZValuesAlgorithm::createInstance() const
{
  return new QgsExtractZValuesAlgorithm();
}

QStringList QgsExtractZValuesAlgorithm::tags() const
{
  return QObject::tr( "add,z,value,elevation,height,attribute,statistics,stats" ).split( ',' );
}

QString QgsExtractZValuesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Extracts z values from geometries into feature attributes.\n\n"
                      "By default only the z value from the first vertex of each feature is extracted, however the algorithm "
                      "can optionally calculate statistics on all of the geometry's z values, including sums, means, and minimums and maximums" );
}

QString QgsExtractZValuesAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts z values (or z value statistics) from geometries into feature attributes." );
}


//
// QgsExtractMValuesAlgorithm
//

QgsExtractMValuesAlgorithm::QgsExtractMValuesAlgorithm()
{
  mExtractValFunc = []( const QgsPoint & p ) -> double
  {
    return p.m();
  };
  mTestGeomFunc = []( const QgsGeometry & g ) -> bool
  {
    return QgsWkbTypes::hasM( g.wkbType() );
  };
  mDefaultFieldPrefix = QStringLiteral( "m_" );
}

QString QgsExtractMValuesAlgorithm::name() const
{
  return QStringLiteral( "extractmvalues" );
}

QString QgsExtractMValuesAlgorithm::displayName() const
{
  return QObject::tr( "Extract M values" );
}

QgsProcessingAlgorithm *QgsExtractMValuesAlgorithm::createInstance() const
{
  return new QgsExtractMValuesAlgorithm();
}

QStringList QgsExtractMValuesAlgorithm::tags() const
{
  return QObject::tr( "add,m,value,measure,attribute,statistics,stats" ).split( ',' );
}

QString QgsExtractMValuesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Extracts m values from geometries into feature attributes.\n\n"
                      "By default only the m value from the first vertex of each feature is extracted, however the algorithm "
                      "can optionally calculate statistics on all of the geometry's m values, including sums, means, and minimums and maximums" );
}

QString QgsExtractMValuesAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts m values (or m value statistics) from geometries into feature attributes." );
}


///@endcond

