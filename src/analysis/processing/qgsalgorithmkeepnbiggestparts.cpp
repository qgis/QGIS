/***************************************************************************
                         qgsalgorithmkeepnbiggestparts.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsalgorithmkeepnbiggestparts.h"

#include "qgsgeometrycollection.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"

#include <queue>

///@cond PRIVATE

QString QgsKeepNBiggestPartsAlgorithm::name() const
{
  return u"keepnbiggestparts"_s;
}

QString QgsKeepNBiggestPartsAlgorithm::displayName() const
{
  return QObject::tr( "Keep N biggest parts" );
}

QStringList QgsKeepNBiggestPartsAlgorithm::tags() const
{
  return QObject::tr( "remove,delete,drop,largest,area,filter" ).split( ',' );
}

QString QgsKeepNBiggestPartsAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsKeepNBiggestPartsAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsKeepNBiggestPartsAlgorithm::outputName() const
{
  return QObject::tr( "Parts" );
}

QList<int> QgsKeepNBiggestPartsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

Qgis::ProcessingSourceType QgsKeepNBiggestPartsAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

QString QgsKeepNBiggestPartsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon layer and creates a new polygon layer in which multipart "
                      "geometries have been removed, leaving only the n largest (in terms of area) parts." );
}

QString QgsKeepNBiggestPartsAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a polygon layer in which multipart geometries have been removed, "
                      "leaving only the n largest (in terms of area) parts." );
}

QgsKeepNBiggestPartsAlgorithm *QgsKeepNBiggestPartsAlgorithm::createInstance() const
{
  return new QgsKeepNBiggestPartsAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsKeepNBiggestPartsAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm can be used to repair geometries
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

void QgsKeepNBiggestPartsAlgorithm::initParameters( const QVariantMap & )
{
  auto partsToKeep = std::make_unique<QgsProcessingParameterNumber>( u"PARTS"_s, QObject::tr( "Parts to keep" ), Qgis::ProcessingNumberParameterType::Integer, 1.0, false, 1.0 );
  partsToKeep->setIsDynamic( true );
  partsToKeep->setDynamicPropertyDefinition( QgsPropertyDefinition( u"PARTS"_s, QObject::tr( "Parts to keep" ), QgsPropertyDefinition::IntegerPositive ) );
  partsToKeep->setDynamicLayerParameterName( u"POLYGONS"_s );
  addParameter( partsToKeep.release() );
}

QString QgsKeepNBiggestPartsAlgorithm::inputParameterName() const
{
  return u"POLYGONS"_s;
}

bool QgsKeepNBiggestPartsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mPartsToKeep = parameterAsInt( parameters, u"PARTS"_s, context );
  mDynamicPartsToKeep = QgsProcessingParameters::isDynamic( parameters, u"PARTS"_s );
  if ( mDynamicPartsToKeep )
    mPartsToKeepProperty = parameters.value( u"PARTS"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsKeepNBiggestPartsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();

    QgsGeometry outputGeometry;

    if ( const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geometry.constGet() ) )
    {
      int nPartsToKeep = mPartsToKeep;
      if ( mDynamicPartsToKeep )
        nPartsToKeep = mPartsToKeepProperty.valueAsInt( context.expressionContext(), nPartsToKeep );

      const int numParts = collection->numGeometries();
      if ( nPartsToKeep >= numParts )
      {
        // nothing to do
        outputGeometry = geometry;
      }
      else
      {
        struct GreaterThanByArea
        {
            bool operator()( const QgsAbstractGeometry *lhs, const QgsAbstractGeometry *rhs ) const
            {
              return lhs->area() < rhs->area();
            }
        };

        std::unique_ptr<QgsMultiSurface> res = QgsWkbTypes::isCurvedType( collection->wkbType() ) ? std::make_unique<QgsMultiSurface>() : std::make_unique<QgsMultiPolygon>();
        std::priority_queue<const QgsAbstractGeometry *, std::vector<const QgsAbstractGeometry *>, GreaterThanByArea> areaQueue;
        for ( int i = 0; i < numParts; ++i )
        {
          areaQueue.push( collection->geometryN( i ) );
        }

        for ( int i = 0; i < nPartsToKeep; ++i )
        {
          const QgsAbstractGeometry *part = areaQueue.top();
          areaQueue.pop();
          res->addGeometry( part->clone() );
        }

        outputGeometry = QgsGeometry( std::move( res ) );
      }
    }
    else
    {
      outputGeometry = geometry;
      outputGeometry.convertToMultiType();
    }

    f.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << f;
}


///@endcond
