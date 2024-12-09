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
#include "qgsmultisurface.h"
#include "qgsmultipolygon.h"
#include <queue>

///@cond PRIVATE

QString QgsKeepNBiggestPartsAlgorithm::name() const
{
  return QStringLiteral( "keepnbiggestparts" );
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
  return QStringLiteral( "vectorgeometry" );
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
  std::unique_ptr<QgsProcessingParameterNumber> partsToKeep = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "PARTS" ), QObject::tr( "Parts to keep" ), Qgis::ProcessingNumberParameterType::Integer, 1.0, false, 1.0 );
  partsToKeep->setIsDynamic( true );
  partsToKeep->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "PARTS" ), QObject::tr( "Parts to keep" ), QgsPropertyDefinition::IntegerPositive ) );
  partsToKeep->setDynamicLayerParameterName( QStringLiteral( "POLYGONS" ) );
  addParameter( partsToKeep.release() );
}

QString QgsKeepNBiggestPartsAlgorithm::inputParameterName() const
{
  return QStringLiteral( "POLYGONS" );
}

bool QgsKeepNBiggestPartsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mPartsToKeep = parameterAsInt( parameters, QStringLiteral( "PARTS" ), context );
  mDynamicPartsToKeep = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "PARTS" ) );
  if ( mDynamicPartsToKeep )
    mPartsToKeepProperty = parameters.value( QStringLiteral( "PARTS" ) ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsKeepNBiggestPartsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();

    QgsGeometry outputGeometry;
    if ( !geometry.isMultipart() )
    {
      outputGeometry = geometry;
      outputGeometry.convertToMultiType();
    }
    else
    {
      int nPartsToKeep = mPartsToKeep;
      if ( mDynamicPartsToKeep )
        nPartsToKeep = mPartsToKeepProperty.valueAsInt( context.expressionContext(), nPartsToKeep );

      const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geometry.constGet() );
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

    f.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << f;
}


///@endcond
