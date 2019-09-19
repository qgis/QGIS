/***************************************************************************
                         qgsbookmarkalgorithms.cpp
                         ---------------------
    begin                : September 2019
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

#include "qgsbookmarkalgorithms.h"
#include "qgsapplication.h"

///@cond PRIVATE

//
// QgsBookmarksToLayerAlgorithm
//

void QgsBookmarksToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterEnum > sourceParam = qgis::make_unique<QgsProcessingParameterEnum >( QStringLiteral( "SOURCE" ), QObject::tr( "Bookmark source" ), QStringList() <<
      QObject::tr( "Project bookmarks" ) << QObject::tr( "User bookmarks" ), true, QVariantList() << 0 << 1 );
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
  QVariantMap metadata;
  metadata.insert( QStringLiteral( "widget_wrapper" ), wrapperMetadata );
  sourceParam->setMetadata( metadata );
  addParameter( sourceParam.release() );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Output CRS" ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ), QgsProcessing::TypeVectorPolygon ) );
}

QString QgsBookmarksToLayerAlgorithm::name() const
{
  return QStringLiteral( "bookmarkstolayer" );
}

QString QgsBookmarksToLayerAlgorithm::displayName() const
{
  return QObject::tr( "Convert spatial bookmarks to layer" );
}

QStringList QgsBookmarksToLayerAlgorithm::tags() const
{
  return QObject::tr( "save,extract" ).split( ',' );
}

QString QgsBookmarksToLayerAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsBookmarksToLayerAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsBookmarksToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new layer containing polygon features for stored spatial bookmarks.\n\n"
                      "The export can be filtered to only bookmarks belonging to the current project, to all user bookmarks, or a combination of both." );
}

QString QgsBookmarksToLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts stored spatial bookmarks to a polygon layer." );
}

QIcon QgsBookmarksToLayerAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mActionShowBookmarks.svg" ) );
}

QString QgsBookmarksToLayerAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "mActionShowBookmarks.svg" ) );
}

QgsBookmarksToLayerAlgorithm *QgsBookmarksToLayerAlgorithm::createInstance() const
{
  return new QgsBookmarksToLayerAlgorithm();
}

bool QgsBookmarksToLayerAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QList< int > sources = parameterAsEnums( parameters, QStringLiteral( "SOURCE" ), context );
  if ( sources.contains( 0 ) )
    mBookmarks.append( context.project()->bookmarkManager()->bookmarks() );
  if ( sources.contains( 1 ) )
    mBookmarks.append( QgsApplication::bookmarkManager()->bookmarks() );

  return true;
}

QVariantMap QgsBookmarksToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "group" ), QVariant::String ) );
  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, crs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  int count = mBookmarks.count();
  int current = 0;
  double step = count > 0 ? 100.0 / count : 1;

  for ( const QgsBookmark &b : qgis::as_const( mBookmarks ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature feat;
    feat.setAttributes( QgsAttributes() << b.name() << b.group() );

    QgsGeometry geom = QgsGeometry::fromRect( b.extent() );
    if ( b.extent().crs() != crs )
    {
      QgsCoordinateTransform xform( b.extent().crs(), crs, context.transformContext() );
      geom = geom.densifyByCount( 20 );
      try
      {
        geom.transform( xform );
      }
      catch ( QgsCsException & )
      {
        feedback->reportError( QObject::tr( "Could not reproject bookmark %1 to destination CRS" ).arg( b.name() ) );
        feedback->setProgress( current++ * step );
        continue;
      }
    }

    feat.setGeometry( geom );

    sink->addFeature( feat, QgsFeatureSink::FastInsert );

    feedback->setProgress( current++ * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


//
// QgsLayerToBookmarksAlgorithm
//

void QgsLayerToBookmarksAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon ) );

  std::unique_ptr< QgsProcessingParameterEnum > sourceParam = qgis::make_unique<QgsProcessingParameterEnum >( QStringLiteral( "DESTINATION" ), QObject::tr( "Bookmark destination" ), QStringList() <<
      QObject::tr( "Project bookmarks" ) << QObject::tr( "User bookmarks" ), false, 0 );
  addParameter( sourceParam.release() );

  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "NAME_EXPRESSION" ), QObject::tr( "Name field" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "GROUP_EXPRESSION" ), QObject::tr( "Group field" ), QVariant(), QStringLiteral( "INPUT" ), true ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "COUNT" ), QObject::tr( "Count of bookmarks added" ) ) );
}

QString QgsLayerToBookmarksAlgorithm::name() const
{
  return QStringLiteral( "layertobookmarks" );
}

QString QgsLayerToBookmarksAlgorithm::displayName() const
{
  return QObject::tr( "Convert layer to spatial bookmarks" );
}

QStringList QgsLayerToBookmarksAlgorithm::tags() const
{
  return QObject::tr( "save,extract,store" ).split( ',' );
}

QString QgsLayerToBookmarksAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsLayerToBookmarksAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsLayerToBookmarksAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates spatial bookmarks corresponding to the extent of features contained in a layer." );
}

QString QgsLayerToBookmarksAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts feature extents to stored spatial bookmarks." );
}

QIcon QgsLayerToBookmarksAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mActionShowBookmarks.svg" ) );
}

QString QgsLayerToBookmarksAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "mActionShowBookmarks.svg" ) );
}

QgsLayerToBookmarksAlgorithm *QgsLayerToBookmarksAlgorithm::createInstance() const
{
  return new QgsLayerToBookmarksAlgorithm();
}

QVariantMap QgsLayerToBookmarksAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mDest = parameterAsEnum( parameters, QStringLiteral( "DESTINATION" ), context );
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );


  QString nameExpressionString = parameterAsExpression( parameters, QStringLiteral( "NAME_EXPRESSION" ), context );
  QString groupExpressionString = parameterAsExpression( parameters, QStringLiteral( "GROUP_EXPRESSION" ), context );

  QgsExpressionContext expressionContext = context.expressionContext();
  expressionContext.appendScope( source->createExpressionContextScope() );

  QgsExpression nameExpression = QgsExpression( nameExpressionString );
  if ( !nameExpression.prepare( &expressionContext ) )
    throw QgsProcessingException( QObject::tr( "Invalid name expression: %1" ).arg( nameExpression.parserErrorString() ) );

  QSet< QString > requiredColumns = nameExpression.referencedColumns();

  std::unique_ptr< QgsExpression > groupExpression;
  if ( !groupExpressionString.isEmpty() )
  {
    groupExpression = qgis::make_unique< QgsExpression >( groupExpressionString );
    if ( !groupExpression->prepare( &expressionContext ) )
      throw QgsProcessingException( QObject::tr( "Invalid group expression: %1" ).arg( groupExpression->parserErrorString() ) );
    requiredColumns.unite( groupExpression->referencedColumns() );
  }

  QgsFeatureRequest req;
  req.setSubsetOfAttributes( requiredColumns, source->fields() );

  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  QgsFeatureIterator fi = source->getFeatures( req, QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature f;
  int current = 0;
  while ( fi.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( f.hasGeometry() )
    {
      const QgsReferencedRectangle extent( f.geometry().boundingBox(), source->sourceCrs() );
      expressionContext.setFeature( f );
      const QString name = nameExpression.evaluate( &expressionContext ).toString();
      if ( !nameExpression.evalErrorString().isEmpty() )
      {
        feedback->reportError( QObject::tr( "Error evaluating name expression: %1" ).arg( nameExpression.evalErrorString() ) );
        feedback->setProgress( current * step );
        current++;
        continue;
      }
      QString group;
      if ( groupExpression )
      {
        group = groupExpression->evaluate( &expressionContext ).toString();
        if ( !groupExpression->evalErrorString().isEmpty() )
        {
          feedback->reportError( QObject::tr( "Error evaluating group expression: %1" ).arg( groupExpression->evalErrorString() ) );
          feedback->setProgress( current * step );
          current++;
          continue;
        }
      }

      QgsBookmark b;
      b.setName( name );
      b.setGroup( group );
      b.setExtent( extent );
      mBookmarks << b;
    }
    feedback->setProgress( current * step );
    current++;
  }

  return QVariantMap();
}

QVariantMap QgsLayerToBookmarksAlgorithm::postProcessAlgorithm( QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsBookmarkManager *dest = nullptr;
  switch ( mDest )
  {
    case 0:
      dest = context.project()->bookmarkManager();
      break;

    case 1:
      dest = QgsApplication::bookmarkManager();
      break;
  }

  for ( const QgsBookmark &b : qgis::as_const( mBookmarks ) )
    dest->addBookmark( b );

  QVariantMap res;
  res.insert( QStringLiteral( "COUNT" ), mBookmarks.size() );
  return res;
}

///@endcond



