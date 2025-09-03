/***************************************************************************
                         qgsalgorithmuniquevalues.cpp
                         ---------------------
    begin                : May 2025
    copyright            : (C) 2025 by Alexander Bruy
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

#include "qgsalgorithmuniquevalues.h"

///@cond PRIVATE

QString QgsUniqueValuesAlgorithm::name() const
{
  return QStringLiteral( "listuniquevalues" );
}

QString QgsUniqueValuesAlgorithm::displayName() const
{
  return QObject::tr( "List unique values" );
}

QStringList QgsUniqueValuesAlgorithm::tags() const
{
  return QObject::tr( "count,unique,values" ).split( ',' );
}

QString QgsUniqueValuesAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsUniqueValuesAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

QString QgsUniqueValuesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a report with information about the unique values found in a given attribute (or attributes) of a vector layer." );
}

QString QgsUniqueValuesAlgorithm::shortDescription() const
{
  return QObject::tr( "Returns list of unique values in given field(s) of a vector layer." );
}

QgsUniqueValuesAlgorithm *QgsUniqueValuesAlgorithm::createInstance() const
{
  return new QgsUniqueValuesAlgorithm();
}

void QgsUniqueValuesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELDS" ), QObject::tr( "Target field(s)" ), QVariant(), QStringLiteral( "INPUT" ), Qgis::ProcessingFieldParameterDataType::Any, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Unique values" ), Qgis::ProcessingSourceType::Vector, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ), QObject::tr( "HTML report" ), QObject::tr( "HTML files (*.html *.htm)" ), QVariant(), true ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_VALUES" ), QObject::tr( "Total unique values" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "UNIQUE_VALUES" ), QObject::tr( "Unique values" ) ) );
}

QVariantMap QgsUniqueValuesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QStringList fieldNames = parameterAsStrings( parameters, QStringLiteral( "FIELDS" ), context );
  const QString outputHtml = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );

  QgsFields fields;
  QList<int> fieldIndices;

  for ( const QString &fieldName : fieldNames )
  {
    int fieldIndex = source->fields().lookupField( fieldName );
    if ( fieldIndex < 0 )
    {
      feedback->reportError( QObject::tr( "Invalid field name %1" ).arg( fieldName ) );
      continue;
    }
    fields.append( source->fields().at( fieldIndex ) );
    fieldIndices << fieldIndex;
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QSet<QgsAttributes> values;
  if ( fieldIndices.size() == 1 )
  {
    const QSet<QVariant> unique = source->uniqueValues( fieldIndices.at( 0 ) );
    for ( const QVariant &v : unique )
    {
      values.insert( QgsAttributes() << v );
    }
  }
  else
  {
    // we have to scan whole table
    // TODO: add this support to QgsVectorDataProvider, so we can run it on the backend
    QgsFeatureRequest request;
    request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    request.setSubsetOfAttributes( fieldIndices );

    const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;
    QgsFeatureIterator features = source->getFeatures( request );
    QgsFeature f;
    long long i = 0;
    while ( features.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      QgsAttributes attrs;
      for ( auto &i : std::as_const( fieldIndices ) )
      {
        attrs << f.attribute( i );
      }
      values.insert( attrs );

      i++;
      feedback->setProgress( i * step );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "TOTAL_VALUES" ), values.size() );

  QStringList valueList;
  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    QStringList s;
    for ( const QVariant &v : std::as_const( *it ) )
    {
      s.append( v.toString() );
    }
    valueList.append( s.join( ',' ) );
  }
  outputs.insert( QStringLiteral( "UNIQUE_VALUES" ), valueList.join( ';' ) );

  if ( sink )
  {
    for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
    {
      if ( feedback->isCanceled() )
        break;

      QgsFeature f;
      f.setAttributes( *it );
      if ( !sink->addFeature( f, QgsFeatureSink::Flag::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
    sink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  }

  if ( !outputHtml.isEmpty() )
  {
    QFile file( outputHtml );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream out( &file );
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
      out.setCodec( "UTF-8" );
#endif
      out << QStringLiteral( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      out << QObject::tr( "<p>Total unique values: %1</p>" ).arg( values.size() );
      out << QObject::tr( "<p>Unique values:</p>" );
      out << QStringLiteral( "<ul>" );
      for ( auto &v : std::as_const( valueList ) )
      {
        out << QStringLiteral( "<li>%1</li>" ).arg( v );
      }
      out << QStringLiteral( "</ul></body></html>" );

      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputHtml );
    }
  }

  return outputs;
}

///@endcond
