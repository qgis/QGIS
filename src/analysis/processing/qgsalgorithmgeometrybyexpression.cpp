/***************************************************************************
                         qgsalgorithmgeometrybyexpression.cpp
                         ------------------------
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

#include "qgsalgorithmgeometrybyexpression.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsGeometryByExpressionAlgorithm::name() const
{
  return QStringLiteral( "geometrybyexpression" );
}

QString QgsGeometryByExpressionAlgorithm::displayName() const
{
  return QObject::tr( "Geometry by expression" );
}

QStringList QgsGeometryByExpressionAlgorithm::tags() const
{
  return QObject::tr( "geometry,expression,create,modify,update" ).split( ',' );
}

QString QgsGeometryByExpressionAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsGeometryByExpressionAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsGeometryByExpressionAlgorithm::outputName() const
{
  return QObject::tr( "Modified geometry" );
}

QString QgsGeometryByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm updates existing geometries (or creates new geometries) for input "
                      "features by use of a QGIS expression. This allows complex geometry modifications "
                      "which can utilize all the flexibility of the QGIS expression engine to manipulate "
                      "and create geometries for output features.\n\n"
                      "For help with QGIS expression functions, see the inbuilt help for specific functions "
                      "which is available in the expression builder." );
}

QgsGeometryByExpressionAlgorithm *QgsGeometryByExpressionAlgorithm::createInstance() const
{
  return new QgsGeometryByExpressionAlgorithm();
}

QList<int> QgsGeometryByExpressionAlgorithm::inputLayerTypes() const
{
  return QList< int >() << QgsProcessing::TypeVector;
}

QgsWkbTypes::Type QgsGeometryByExpressionAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return mWkbType;
}

QgsProcessingFeatureSource::Flag QgsGeometryByExpressionAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

void QgsGeometryByExpressionAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "OUTPUT_GEOMETRY" ), QObject::tr( "Output geometry type" ),
                QStringList() << QObject::tr( "Polygon" ) << QObject::tr( "Line" ) << QObject::tr( "Point" ), false, 0 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "WITH_Z" ), QObject::tr( "Output geometry has z dimension" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "WITH_M" ), QObject::tr( "Output geometry has m values" ), false ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Geometry expression" ),
                QStringLiteral( "$geometry" ), QStringLiteral( "INPUT" ) ) );
}

bool QgsGeometryByExpressionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int geometryType = parameterAsInt( parameters, QStringLiteral( "OUTPUT_GEOMETRY" ), context );
  switch ( geometryType )
  {
    case 0:
      mWkbType = QgsWkbTypes::Type::Polygon;
      break;
    case 1:
      mWkbType = QgsWkbTypes::Type::LineString;
      break;
    case 2:
      mWkbType = QgsWkbTypes::Type::Point;
      break;
  }

  if ( parameterAsBoolean( parameters, QStringLiteral( "WITH_Z" ), context ) )
  {
    mWkbType = QgsWkbTypes::addZ( mWkbType );
  }
  if ( parameterAsBoolean( parameters, QStringLiteral( "WITH_M" ), context ) )
  {
    mWkbType = QgsWkbTypes::addM( mWkbType );
  }

  mExpression = QgsExpression( parameterAsString( parameters, QStringLiteral( "EXPRESSION" ), context ) );
  if ( mExpression.hasParserError() )
  {
    feedback->reportError( mExpression.parserErrorString() );
    return false;
  }

  mExpressionContext = createExpressionContext( parameters, context );
  mExpression.prepare( &mExpressionContext );

  return true;
}

QgsFeatureList QgsGeometryByExpressionAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature feature = f;
  mExpressionContext.setFeature( feature );
  const QVariant value = mExpression.evaluate( &mExpressionContext );

  if ( mExpression.hasEvalError() )
  {
    throw QgsProcessingException( QObject::tr( "Evaluation error: %1" ).arg( mExpression.evalErrorString() ) );
  }

  if ( value.isNull() )
  {
    feature.setGeometry( QgsGeometry() );
  }
  else
  {
    if ( value.canConvert< QgsGeometry >() )
    {
      const QgsGeometry geom = value.value<QgsGeometry>();
      feature.setGeometry( geom );
    }
    else
    {
      throw QgsProcessingException( QObject::tr( "%1 is not a geometry" ).arg( value.toString() ) );
    }
  }

  return QgsFeatureList() << feature;
}

///@endcond
