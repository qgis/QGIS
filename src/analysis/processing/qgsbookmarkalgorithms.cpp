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
  auto sourceParam = std::make_unique<QgsProcessingParameterEnum>( u"SOURCE"_s, QObject::tr( "Bookmark source" ), QStringList() << QObject::tr( "Project bookmarks" ) << QObject::tr( "User bookmarks" ), true, QVariantList() << 0 << 1 );
  QVariantMap wrapperMetadata;
  wrapperMetadata.insert( u"useCheckBoxes"_s, true );
  QVariantMap metadata;
  metadata.insert( u"widget_wrapper"_s, wrapperMetadata );
  sourceParam->setMetadata( metadata );
  addParameter( sourceParam.release() );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Output CRS" ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Output" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QString QgsBookmarksToLayerAlgorithm::name() const
{
  return u"bookmarkstolayer"_s;
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
  return u"vectorgeneral"_s;
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
  return QgsApplication::getThemeIcon( u"mActionShowBookmarks.svg"_s );
}

QString QgsBookmarksToLayerAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"mActionShowBookmarks.svg"_s );
}

QgsBookmarksToLayerAlgorithm *QgsBookmarksToLayerAlgorithm::createInstance() const
{
  return new QgsBookmarksToLayerAlgorithm();
}

bool QgsBookmarksToLayerAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QList<int> sources = parameterAsEnums( parameters, u"SOURCE"_s, context );
  if ( sources.contains( 0 ) )
  {
    if ( !context.project() )
      throw QgsProcessingException( QObject::tr( "No project is available for bookmark extraction" ) );
    mBookmarks.append( context.project()->bookmarkManager()->bookmarks() );
  }
  if ( sources.contains( 1 ) )
    mBookmarks.append( QgsApplication::bookmarkManager()->bookmarks() );

  return true;
}

QVariantMap QgsBookmarksToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );
  QgsFields fields;
  fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"group"_s, QMetaType::Type::QString ) );
  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, crs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  int count = mBookmarks.count();
  int current = 0;
  double step = count > 0 ? 100.0 / count : 1;

  for ( const QgsBookmark &b : std::as_const( mBookmarks ) )
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

    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

    feedback->setProgress( current++ * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}


//
// QgsLayerToBookmarksAlgorithm
//

void QgsLayerToBookmarksAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );

  auto sourceParam = std::make_unique<QgsProcessingParameterEnum>( u"DESTINATION"_s, QObject::tr( "Bookmark destination" ), QStringList() << QObject::tr( "Project bookmarks" ) << QObject::tr( "User bookmarks" ), false, 0 );
  addParameter( sourceParam.release() );

  addParameter( new QgsProcessingParameterExpression( u"NAME_EXPRESSION"_s, QObject::tr( "Name field" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterExpression( u"GROUP_EXPRESSION"_s, QObject::tr( "Group field" ), QVariant(), u"INPUT"_s, true ) );

  addOutput( new QgsProcessingOutputNumber( u"COUNT"_s, QObject::tr( "Count of bookmarks added" ) ) );
}

QString QgsLayerToBookmarksAlgorithm::name() const
{
  return u"layertobookmarks"_s;
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
  return u"vectorgeneral"_s;
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
  return QgsApplication::getThemeIcon( u"mActionShowBookmarks.svg"_s );
}

QString QgsLayerToBookmarksAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"mActionShowBookmarks.svg"_s );
}

QgsLayerToBookmarksAlgorithm *QgsLayerToBookmarksAlgorithm::createInstance() const
{
  return new QgsLayerToBookmarksAlgorithm();
}

QVariantMap QgsLayerToBookmarksAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mDest = parameterAsEnum( parameters, u"DESTINATION"_s, context );
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );


  QString nameExpressionString = parameterAsExpression( parameters, u"NAME_EXPRESSION"_s, context );
  QString groupExpressionString = parameterAsExpression( parameters, u"GROUP_EXPRESSION"_s, context );

  QgsExpressionContext expressionContext = context.expressionContext();
  expressionContext.appendScope( source->createExpressionContextScope() );

  QgsExpression nameExpression = QgsExpression( nameExpressionString );
  if ( !nameExpression.prepare( &expressionContext ) )
    throw QgsProcessingException( QObject::tr( "Invalid name expression: %1" ).arg( nameExpression.parserErrorString() ) );

  QSet<QString> requiredColumns = nameExpression.referencedColumns();

  std::unique_ptr<QgsExpression> groupExpression;
  if ( !groupExpressionString.isEmpty() )
  {
    groupExpression = std::make_unique<QgsExpression>( groupExpressionString );
    if ( !groupExpression->prepare( &expressionContext ) )
      throw QgsProcessingException( QObject::tr( "Invalid group expression: %1" ).arg( groupExpression->parserErrorString() ) );
    requiredColumns.unite( groupExpression->referencedColumns() );
  }

  QgsFeatureRequest req;
  req.setSubsetOfAttributes( requiredColumns, source->fields() );

  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  QgsFeatureIterator fi = source->getFeatures( req, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
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

    default:
      throw QgsProcessingException( QObject::tr( "Invalid bookmark destination" ) );
  }

  for ( const QgsBookmark &b : std::as_const( mBookmarks ) )
    dest->addBookmark( b );

  QVariantMap res;
  res.insert( u"COUNT"_s, mBookmarks.size() );
  return res;
}

///@endcond
