/***************************************************************************
                         qgsalgorithmdpointstopaths.cpp
                         ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpointstopaths.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsPointsToPathsAlgorithm::name() const
{
  return QStringLiteral( "pointstopaths" );
}

QString QgsPointsToPathsAlgorithm::displayName() const
{
  return QObject::tr( "Points to paths" );
}

QString QgsPointsToPathsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a point layer and connects its features creating a new line layer.\n\n"
                      "An attribute or expression may be specified to define the order the points should be connected. "
                      "If no order expression is specified, the fid is used.\n\n"
                      "A natural sort can be used when sorting by a string attribute "
                      "or expression (ie. place 'a9' before 'a10').\n\n"
                      "An attribute or expression can be selected to group points having the same value into the same resulting line." );
}

QStringList QgsPointsToPathsAlgorithm::tags() const
{
  return QObject::tr( "create,lines,points,connect,convert,join" ).split( ',' );
}

QString QgsPointsToPathsAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPointsToPathsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

void QgsPointsToPathsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CLOSE_PATHS" ),
                QObject::tr( "Create closed paths" ), false, true ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "ORDER_EXPRESSION" ),
                QObject::tr( "Order expression" ), QVariant(), QStringLiteral( "INPUT" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "NATURAL_SORT" ),
                QObject::tr( "Sort text containing numbers naturally" ), false, true ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "GROUP_EXPRESSION" ),
                QObject::tr( "Path group expression" ), QVariant(), QStringLiteral( "INPUT" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Paths" ), QgsProcessing::TypeVectorLine ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NUM_PATHS" ), QObject::tr( "Number of paths" ) ) );
}

QgsPointsToPathsAlgorithm *QgsPointsToPathsAlgorithm::createInstance() const
{
  return new QgsPointsToPathsAlgorithm();
}

QVariantMap QgsPointsToPathsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const bool closePaths = parameterAsBool( parameters, QStringLiteral( "CLOSE_PATHS" ), context );

  QString orderExpressionString = parameterAsString( parameters, QStringLiteral( "ORDER_EXPRESSION" ), context );
  // If no order expression is given, default to the fid
  if ( orderExpressionString.isEmpty() )
    orderExpressionString = QString( "$id" );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, source.get() );
  QgsExpression orderExpression = QgsExpression( orderExpressionString );
  if ( orderExpression.hasParserError() )
    throw QgsProcessingException( orderExpression.parserErrorString() );

  QStringList requiredFields = QStringList( orderExpression.referencedColumns().values() );
  orderExpression.prepare( &expressionContext );

  QgsFields outputFields = QgsFields();
  const QString groupExpressionString = parameterAsString( parameters, QStringLiteral( "GROUP_EXPRESSION" ), context );
  QgsExpression groupExpression = groupExpressionString.isEmpty() ? QgsExpression( QString( "true" ) ) : QgsExpression( groupExpressionString );
  if ( groupExpression.hasParserError() )
    throw QgsProcessingException( groupExpression.parserErrorString() );
  if ( ! groupExpressionString.isEmpty() )
  {
    requiredFields.append( groupExpression.referencedColumns().values() );
    const QgsField field = groupExpression.isField() ? source->fields().field( requiredFields.last() ) : QString( "group" );
    outputFields.append( field );
  }

  const bool naturalSort = parameterAsBool( parameters, QStringLiteral( "NATURAL_SORT" ), context );
  QCollator collator;
  collator.setNumericMode( true );

  QgsWkbTypes::Type wkbType = QgsWkbTypes::LineString;
  if ( QgsWkbTypes::hasM( source->wkbType() ) )
    wkbType = QgsWkbTypes::addM( wkbType );
  if ( QgsWkbTypes::hasZ( source->wkbType() ) )
    wkbType = QgsWkbTypes::addZ( wkbType );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, wkbType, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // Store the points in a hash with the group identifier as the key
  QHash< QVariant, QVector< QPair< QVariant, const QgsPoint * > > > allPoints;

  QgsFeatureRequest request = QgsFeatureRequest().setSubsetOfAttributes( requiredFields, source->fields() );
  QgsFeatureIterator fit = source->getFeatures( request, QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature f;
  const double totalPoints = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;
  long currentPoint = 0;
  feedback->setProgressText( QObject::tr( "Loading points…" ) );
  while ( fit.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    feedback->setProgress( currentPoint * totalPoints );

    if ( f.hasGeometry() && ! f.geometry().isNull() )
    {
      expressionContext.setFeature( f );
      const QVariant orderValue = orderExpression.evaluate( &expressionContext );
      const QVariant groupValue = groupExpressionString.isEmpty() ? QVariant() : groupExpression.evaluate( &expressionContext );

      if ( ! allPoints.keys().contains( groupValue ) )
        allPoints[ groupValue ] = QVector< QPair< QVariant, const QgsPoint * > >();
      const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( f.geometry().constGet()->clone() );
      allPoints[ groupValue ] << QPair< QVariant, const QgsPoint *>( orderValue, point );
    }
    ++currentPoint;
  }

  int pathCount = 0;
  currentPoint = 0;
  QHashIterator< QVariant, QVector< QPair< QVariant, const QgsPoint * > > > hit( allPoints );
  feedback->setProgressText( QObject::tr( "Creating paths…" ) );
  while ( hit.hasNext() )
  {
    hit.next();
    if ( feedback->isCanceled() )
    {
      break;
    }
    auto pairs = hit.value();

    if ( naturalSort )
    {
      std::sort( pairs.begin(),
                 pairs.end(),
                 [&collator]( const QPair< const QVariant, const QgsPoint * > &pair1,
                              const QPair< const QVariant, const QgsPoint * > &pair2 )
      {
        return collator.compare( pair1.first.toString(), pair2.first.toString() ) < 0;
      } );
    }
    else
    {
      std::sort( pairs.begin(),
                 pairs.end(),
                 []( const QPair< const QVariant, const QgsPoint * > &pair1,
                     const QPair< const QVariant, const QgsPoint * > &pair2 )
      {
        return pair1.first < pair2.first;
      } );
    }


    QVector<QgsPoint> pathPoints;
    for ( QVector< QPair< QVariant, const QgsPoint * > >::ConstIterator pit = pairs.constBegin(); pit != pairs.constEnd(); ++pit )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      feedback->setProgress( currentPoint * totalPoints );
      pathPoints.append( *pit->second );
      ++currentPoint;
    }
    if ( pathPoints.size() < 2 )
    {
      feedback->pushInfo( QObject::tr( "Skipping path with group %1 : insufficient vertices" ).arg( hit.key().toString() ) );
      continue;
    }
    if ( closePaths && pathPoints.size() > 2 && pathPoints.first() != pathPoints.last() )
      pathPoints.append( pathPoints.first() );

    QgsFeature outputFeature;
    QgsAttributes attrs;
    attrs.append( hit.key() );
    outputFeature.setGeometry( QgsGeometry::fromPolyline( pathPoints ) );
    outputFeature.setAttributes( attrs );
    sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
    ++pathCount;
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "NUM_PATHS" ), pathCount );
  return outputs;
}

///@endcond
