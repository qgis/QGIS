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
#include "qgsmultipoint.h"
#include "qgsdistancearea.h"

#include <QCollator>
#include <QTextStream>

///@cond PRIVATE

QString QgsPointsToPathsAlgorithm::name() const
{
  return QStringLiteral( "pointstopath" );
}

QString QgsPointsToPathsAlgorithm::displayName() const
{
  return QObject::tr( "Points to path" );
}

QString QgsPointsToPathsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a point layer and connects its features creating a new line layer.\n\n"
                      "An attribute or expression may be specified to define the order the points should be connected. "
                      "If no order expression is specified, the feature ID is used.\n\n"
                      "A natural sort can be used when sorting by a string attribute "
                      "or expression (ie. place 'a9' before 'a10').\n\n"
                      "An attribute or expression can be selected to group points having the same value into the same resulting line." );
}

QStringList QgsPointsToPathsAlgorithm::tags() const
{
  return QObject::tr( "create,lines,points,connect,convert,join,path" ).split( ',' );
}

QString QgsPointsToPathsAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsPointsToPathsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsPointsToPathsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CLOSE_PATH" ),
                QObject::tr( "Create closed paths" ), false, true ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "ORDER_EXPRESSION" ),
                QObject::tr( "Order expression" ), QVariant(), QStringLiteral( "INPUT" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "NATURAL_SORT" ),
                QObject::tr( "Sort text containing numbers naturally" ), false, true ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "GROUP_EXPRESSION" ),
                QObject::tr( "Path group expression" ), QVariant(), QStringLiteral( "INPUT" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Paths" ), QgsProcessing::TypeVectorLine ) );
  // TODO QGIS 4: remove parameter. move logic to separate algorithm if needed.
  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT_TEXT_DIR" ),
                QObject::tr( "Directory for text output" ), QVariant(), true, false ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NUM_PATHS" ), QObject::tr( "Number of paths" ) ) );

  // backwards compatibility parameters
  // TODO QGIS 4: remove compatibility parameters and their logic
  QgsProcessingParameterField *orderField = new QgsProcessingParameterField( QStringLiteral( "ORDER_FIELD" ),
      QObject::tr( "Order field" ), QVariant(), QString(), QgsProcessingParameterField::Any, false, true );
  orderField->setFlags( orderField->flags() | QgsProcessingParameterDefinition::FlagHidden );
  addParameter( orderField );
  QgsProcessingParameterField *groupField = new QgsProcessingParameterField( QStringLiteral( "GROUP_FIELD" ),
      QObject::tr( "Group field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true );
  groupField->setFlags( orderField->flags() | QgsProcessingParameterDefinition::FlagHidden );
  addParameter( groupField );
  QgsProcessingParameterString *dateFormat = new QgsProcessingParameterString( QStringLiteral( "DATE_FORMAT" ),
      QObject::tr( "Date format (if order field is DateTime)" ), QVariant(), false, true );
  dateFormat->setFlags( orderField->flags() | QgsProcessingParameterDefinition::FlagHidden );
  addParameter( dateFormat );
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

  const bool closePaths = parameterAsBool( parameters, QStringLiteral( "CLOSE_PATH" ), context );

  QString orderExpressionString = parameterAsString( parameters, QStringLiteral( "ORDER_EXPRESSION" ), context );
  const QString orderFieldString = parameterAsString( parameters, QStringLiteral( "ORDER_FIELD" ), context );
  if ( ! orderFieldString.isEmpty() )
  {
    // this is a backwards compatibility parameter
    orderExpressionString = QgsExpression::quotedColumnRef( orderFieldString );

    QString dateFormat = parameterAsString( parameters, QStringLiteral( "DATE_FORMAT" ), context );
    if ( ! dateFormat.isEmpty() )
    {
      QVector< QPair< QString, QString > > codeMap;
      codeMap << QPair< QString, QString >( "%%", "%" )
              << QPair< QString, QString >( "%a", "ddd" )
              << QPair< QString, QString >( "%A", "dddd" )
              << QPair< QString, QString >( "%w", "" ) //day of the week 0-6
              << QPair< QString, QString >( "%d", "dd" )
              << QPair< QString, QString >( "%b", "MMM" )
              << QPair< QString, QString >( "%B", "MMMM" )
              << QPair< QString, QString >( "%m", "MM" )
              << QPair< QString, QString >( "%y", "yy" )
              << QPair< QString, QString >( "%Y", "yyyy" )
              << QPair< QString, QString >( "%H", "hh" )
              << QPair< QString, QString >( "%I", "hh" ) // 12 hour
              << QPair< QString, QString >( "%p", "AP" )
              << QPair< QString, QString >( "%M", "mm" )
              << QPair< QString, QString >( "%S", "ss" )
              << QPair< QString, QString >( "%f", "zzz" ) // milliseconds instead of microseconds
              << QPair< QString, QString >( "%z", "" ) // utc offset
              << QPair< QString, QString >( "%Z", "" ) // timezone name
              << QPair< QString, QString >( "%j", "" ) // day of the year
              << QPair< QString, QString >( "%U", "" ) // week number of the year sunday based
              << QPair< QString, QString >( "%W", "" ) // week number of the year monday based
              << QPair< QString, QString >( "%c", "" ) // full datetime
              << QPair< QString, QString >( "%x", "" ) // full date
              << QPair< QString, QString >( "%X", "" ) // full time
              << QPair< QString, QString >( "%G", "yyyy" )
              << QPair< QString, QString >( "%u", "" ) // day of the week 1-7
              << QPair< QString, QString >( "%V", "" ); // week number
      for ( const auto &pair : codeMap )
      {
        dateFormat.replace( pair.first, pair.second );
      }
      orderExpressionString = QString( "to_datetime(%1, '%2')" ).arg( orderExpressionString ).arg( dateFormat );
    }
  }
  else if ( orderExpressionString.isEmpty() )
  {
    // If no order expression is given, default to the fid
    orderExpressionString = QString( "$id" );
  }
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, source.get() );
  QgsExpression orderExpression = QgsExpression( orderExpressionString );
  if ( orderExpression.hasParserError() )
    throw QgsProcessingException( orderExpression.parserErrorString() );

  QStringList requiredFields = QStringList( orderExpression.referencedColumns().values() );
  orderExpression.prepare( &expressionContext );

  QVariant::Type orderFieldType = QVariant::String;
  if ( orderExpression.isField() )
  {
    const int orderFieldIndex = source->fields().indexFromName( orderExpression.referencedColumns().values().first() );
    orderFieldType = source->fields().field( orderFieldIndex ).type();
  }


  QString groupExpressionString = parameterAsString( parameters, QStringLiteral( "GROUP_EXPRESSION" ), context );
  // handle backwards compatibility parameter GROUP_FIELD
  const QString groupFieldString = parameterAsString( parameters, QStringLiteral( "GROUP_FIELD" ), context );
  if ( ! groupFieldString.isEmpty() )
    groupExpressionString = QgsExpression::quotedColumnRef( groupFieldString );

  QgsExpression groupExpression = groupExpressionString.isEmpty() ? QgsExpression( QString( "true" ) ) : QgsExpression( groupExpressionString );
  if ( groupExpression.hasParserError() )
    throw QgsProcessingException( groupExpression.parserErrorString() );

  QgsFields outputFields = QgsFields();
  if ( ! groupExpressionString.isEmpty() )
  {
    requiredFields.append( groupExpression.referencedColumns().values() );
    const QgsField field = groupExpression.isField() ? source->fields().field( requiredFields.last() ) : QStringLiteral( "group" );
    outputFields.append( field );
  }
  outputFields.append( QgsField( "begin", orderFieldType ) );
  outputFields.append( QgsField( "end", orderFieldType ) );

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

  const QString textDir = parameterAsString( parameters, QStringLiteral( "OUTPUT_TEXT_DIR" ), context );
  if ( ! textDir.isEmpty() &&
       ! QDir( textDir ).exists() )
    throw QgsProcessingException( QObject::tr( "The text output directory does not exist" ) );

  QgsDistanceArea da = QgsDistanceArea();
  da.setSourceCrs( source->sourceCrs(), context.transformContext() );
  da.setEllipsoid( context.ellipsoid() );

  // Store the points in a hash with the group identifier as the key
  QHash< QVariant, QVector< QPair< QVariant, QgsPoint > > > allPoints;

  const QgsFeatureRequest request = QgsFeatureRequest().setSubsetOfAttributes( requiredFields, source->fields() );
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
    feedback->setProgress( 0.5 * currentPoint * totalPoints );

    if ( f.hasGeometry() )
    {
      expressionContext.setFeature( f );
      const QVariant orderValue = orderExpression.evaluate( &expressionContext );
      const QVariant groupValue = groupExpressionString.isEmpty() ? QVariant() : groupExpression.evaluate( &expressionContext );

      if ( ! allPoints.contains( groupValue ) )
        allPoints[ groupValue ] = QVector< QPair< QVariant, QgsPoint > >();
      const QgsAbstractGeometry *geom = f.geometry().constGet();
      if ( QgsWkbTypes::isMultiType( geom->wkbType() ) )
      {
        const QgsMultiPoint mp( *qgsgeometry_cast< const QgsMultiPoint * >( geom ) );
        for ( auto pit = mp.const_parts_begin(); pit != mp.const_parts_end(); ++pit )
        {
          const QgsPoint point( *qgsgeometry_cast< const QgsPoint * >( *pit ) );
          allPoints[ groupValue ] << qMakePair( orderValue, point );
        }
      }
      else
      {
        const QgsPoint point( *qgsgeometry_cast< const QgsPoint * >( geom ) );
        allPoints[ groupValue ] << qMakePair( orderValue, point );
      }
    }
    ++currentPoint;
  }

  int pathCount = 0;
  currentPoint = 0;
  QHashIterator< QVariant, QVector< QPair< QVariant, QgsPoint > > > hit( allPoints );
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
      std::stable_sort( pairs.begin(),
                        pairs.end(),
                        [&collator]( const QPair< const QVariant, QgsPoint > &pair1,
                                     const QPair< const QVariant, QgsPoint > &pair2 )
      {
        return collator.compare( pair1.first.toString(), pair2.first.toString() ) < 0;
      } );
    }
    else
    {
      std::stable_sort( pairs.begin(),
                        pairs.end(),
                        []( const QPair< const QVariant, QgsPoint > &pair1,
                            const QPair< const QVariant, QgsPoint > &pair2 )
      {
        return qgsVariantLessThan( pair1.first, pair2.first );
      } );
    }


    QVector<QgsPoint> pathPoints;
    for ( auto pit = pairs.constBegin(); pit != pairs.constEnd(); ++pit )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      feedback->setProgress( 50 + 0.5 * currentPoint * totalPoints );
      pathPoints.append( pit->second );
      ++currentPoint;
    }
    if ( pathPoints.size() < 2 )
    {
      feedback->pushInfo( QObject::tr( "Skipping path with group %1 : insufficient vertices" ).arg( hit.key().toString() ) );
      continue;
    }
    if ( closePaths && pathPoints.size() > 2 && pathPoints.constFirst() != pathPoints.constLast() )
      pathPoints.append( pathPoints.constFirst() );

    QgsFeature outputFeature;
    QgsAttributes attrs;
    if ( ! groupExpressionString.isEmpty() )
      attrs.append( hit.key() );
    attrs.append( hit.value().first().first );
    attrs.append( hit.value().last().first );
    outputFeature.setGeometry( QgsGeometry::fromPolyline( pathPoints ) );
    outputFeature.setAttributes( attrs );
    if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    if ( ! textDir.isEmpty() )
    {
      const QString filename = QDir( textDir ).filePath( hit.key().toString() + QString( ".txt" ) );
      QFile textFile( filename );
      if ( !textFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        throw QgsProcessingException( QObject::tr( "Cannot open file for writing " ) + filename );

      QTextStream out( &textFile );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      out.setCodec( "UTF-8" );
#endif
      out << QString( "angle=Azimuth\n"
                      "heading=Coordinate_System\n"
                      "dist_units=Default\n"
                      "startAt=%1;%2;90\n"
                      "survey=Polygonal\n"
                      "[data]\n" ).arg( pathPoints.at( 0 ).x() ).arg( pathPoints.at( 0 ).y() );

      for ( int i = 1; i < pathPoints.size(); ++i )
      {
        const double angle = pathPoints.at( i - 1 ).azimuth( pathPoints.at( i ) );
        const double distance = da.measureLine( pathPoints.at( i - 1 ), pathPoints.at( i ) );
        out << QString( "%1;%2;90\n" ).arg( angle ).arg( distance );
      }
    }

    ++pathCount;
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "NUM_PATHS" ), pathCount );
  return outputs;
}

///@endcond
