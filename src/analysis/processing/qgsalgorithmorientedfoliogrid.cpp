/***************************************************************************
                         qgsalgorithmorientedfoliogrid.cpp
                         ---------------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmorientedfoliogrid.h"

#include "qgsapplication.h"
#include "qgsgeometryengine.h"
#include "qgslayoutitempage.h"
#include "qgslayoutmeasurementconverter.h"
#include "qgslinestring.h"
#include "qgspagesizeregistry.h"
#include "qgspolygon.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsOrientedFolioGridAlgorithm::name() const
{
  return u"createorientedfoliogrid"_s;
}

QString QgsOrientedFolioGridAlgorithm::displayName() const
{
  return QObject::tr( "Create oriented folio grid" );
}

QStringList QgsOrientedFolioGridAlgorithm::tags() const
{
  return QObject::tr( "grid,folio,atlas,print,layout,oriented,paper,page,sheet,index" ).split( ',' );
}

QString QgsOrientedFolioGridAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsOrientedFolioGridAlgorithm::groupId() const
{
  return u"vectorcreation"_s;
}

QString QgsOrientedFolioGridAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a grid of rectangular folios aligned with the Oriented "
                      "Minimum Bounding Box (OMBB) of the input polygons.\n\n"
                      "The grid is designed for creating print layouts (atlas pages) covering an area "
                      "of interest. Each cell represents a single page at the specified scale." );
}

QString QgsOrientedFolioGridAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates an oriented grid of folios for print layouts covering input polygons." );
}

QgsOrientedFolioGridAlgorithm *QgsOrientedFolioGridAlgorithm::createInstance() const
{
  return new QgsOrientedFolioGridAlgorithm();
}

void QgsOrientedFolioGridAlgorithm::initAlgorithm( const QVariantMap & )
{
  // INPUT: Polygon feature source
  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ),
    QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );

  // PAGE_SIZE: Enum parameter for page size from QgsPageSizeRegistry
  QStringList pageSizeNames;
  const QList<QgsPageSize> sizes = QgsApplication::pageSizeRegistry()->entries();
  for ( const QgsPageSize &size : sizes )
  {
    pageSizeNames << size.name;
  }
  pageSizeNames << QObject::tr( "Custom" );

  qsizetype defaultIndex = pageSizeNames.indexOf( "A4"_L1 );
  if ( defaultIndex < 0 )
    defaultIndex = 0;

  addParameter( new QgsProcessingParameterEnum(
    u"PAGE_SIZE"_s, QObject::tr( "Page size" ),
    pageSizeNames,
    false,
    defaultIndex
  ) );

  // ORIENTATION: Portrait/Landscape
  addParameter( new QgsProcessingParameterEnum(
    u"ORIENTATION"_s, QObject::tr( "Page orientation" ),
    QStringList() << QObject::tr( "Portrait" ) << QObject::tr( "Landscape" ),
    false, 1 // Landscape default (common for plan folios)
  ) );

  // CUSTOM_WIDTH: Optional custom width in mm (only used when PAGE_SIZE=Custom)
  auto widthParam = std::make_unique<QgsProcessingParameterNumber>(
    u"CUSTOM_WIDTH"_s, QObject::tr( "Custom width (mm)" ),
    Qgis::ProcessingNumberParameterType::Double, 297.0, true, 1.0, 10000.0
  );
  addParameter( widthParam.release() );

  // CUSTOM_HEIGHT: Optional custom height in mm
  auto heightParam = std::make_unique<QgsProcessingParameterNumber>(
    u"CUSTOM_HEIGHT"_s, QObject::tr( "Custom height (mm)" ),
    Qgis::ProcessingNumberParameterType::Double, 210.0, true, 1.0, 10000.0
  );
  addParameter( heightParam.release() );

  // SCALE: Print scale denominator
  addParameter( new QgsProcessingParameterNumber(
    u"SCALE"_s, QObject::tr( "Print scale (denominator, e.g., 200 for 1:200)" ),
    Qgis::ProcessingNumberParameterType::Integer, 200, false, 1, 10000000
  ) );

  // HOVERLAY: Horizontal overlap in map units
  addParameter( new QgsProcessingParameterDistance(
    u"HOVERLAY"_s, QObject::tr( "Horizontal overlap" ), 0.0, u"INPUT"_s, false, 0.0
  ) );

  // VOVERLAY: Vertical overlap in map units
  addParameter( new QgsProcessingParameterDistance(
    u"VOVERLAY"_s, QObject::tr( "Vertical overlap" ), 0.0, u"INPUT"_s, false, 0.0
  ) );

  // AVOID_EMPTY: Remove cells not intersecting input
  addParameter( new QgsProcessingParameterBoolean(
    u"AVOID_EMPTY"_s, QObject::tr( "Remove empty cells (not intersecting input)" ), true
  ) );

  // OUTPUT: Feature sink
  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Folio grid" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );
}

bool QgsOrientedFolioGridAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    return false;

  mCrs = source->sourceCrs();

  // Build page size names list
  mPageSizeNames.clear();
  const QList<QgsPageSize> sizes = QgsApplication::pageSizeRegistry()->entries();
  for ( const QgsPageSize &size : sizes )
  {
    mPageSizeNames << size.name;
  }
  mPageSizeNames << QObject::tr( "Custom" );

  // Get parameters
  mPageSizeIndex = parameterAsEnum( parameters, u"PAGE_SIZE"_s, context );
  mOrientation = static_cast<QgsLayoutItemPage::Orientation>( parameterAsEnum( parameters, u"ORIENTATION"_s, context ) );
  mCustomWidth = parameterAsDouble( parameters, u"CUSTOM_WIDTH"_s, context );
  mCustomHeight = parameterAsDouble( parameters, u"CUSTOM_HEIGHT"_s, context );
  mScale = parameterAsInt( parameters, u"SCALE"_s, context );
  mHOverlay = parameterAsDouble( parameters, u"HOVERLAY"_s, context );
  mVOverlay = parameterAsDouble( parameters, u"VOVERLAY"_s, context );
  mAvoidEmpty = parameterAsBool( parameters, u"AVOID_EMPTY"_s, context );

  // Dissolve all input features
  QVector<QgsGeometry> geometries;
  QgsFeatureIterator it = source->getFeatures();
  QgsFeature feature;
  long long featureCount = source->featureCount();
  long long current = 0;

  while ( it.nextFeature( feature ) )
  {
    if ( feedback && feedback->isCanceled() )
      return false;

    if ( feature.hasGeometry() )
      geometries.append( feature.geometry() );

    current++;
    if ( feedback && featureCount > 0 )
      feedback->setProgress( 10.0 * static_cast<double>( current ) / static_cast<double>( featureCount ) );
  }

  if ( geometries.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Input layer contains no valid geometries." ) );
  }

  mDissolvedGeometry = QgsGeometry::unaryUnion( geometries );
  if ( mDissolvedGeometry.isNull() || mDissolvedGeometry.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Failed to dissolve input geometries." ) );
  }

  return true;
}

QgsLayoutSize QgsOrientedFolioGridAlgorithm::getPageSize() const
{
  QgsLayoutSize size;

  // Check if it's a custom size (last index in the list)
  if ( mPageSizeIndex >= mPageSizeNames.size() - 1 )
  {
    size = QgsLayoutSize( mCustomWidth, mCustomHeight, Qgis::LayoutUnit::Millimeters );
  }
  else
  {
    const QString pageSizeName = mPageSizeNames.at( mPageSizeIndex );
    QgsPageSize pageSize;
    if ( QgsApplication::pageSizeRegistry()->decodePageSize( pageSizeName, pageSize ) )
    {
      size = pageSize.size;
    }
    else
    {
      // Fallback to custom size if lookup fails
      size = QgsLayoutSize( mCustomWidth, mCustomHeight, Qgis::LayoutUnit::Millimeters );
    }
  }

  // Apply orientation
  // QgsPageSizeRegistry stores sizes in portrait (height > width for A-series)
  double w = size.width();
  double h = size.height();

  if ( mOrientation == QgsLayoutItemPage::Landscape )
  {
    // Landscape: width > height
    if ( w < h )
      std::swap( w, h );
  }
  else // Portrait
  {
    // Portrait: height > width
    if ( w > h )
      std::swap( w, h );
  }

  return QgsLayoutSize( w, h, size.units() );
}

QVariantMap QgsOrientedFolioGridAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // Compute OMBB
  double ombbArea, ombbAngle, ombbWidth, ombbHeight;
  QgsGeometry ombb = mDissolvedGeometry.orientedMinimumBoundingBox( ombbArea, ombbAngle, ombbWidth, ombbHeight );
  Q_UNUSED( ombbArea )

  if ( ombb.isNull() || ombb.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Failed to compute oriented minimum bounding box." ) );
  }

  // Get OMBB vertices - the first vertex is the origin
  QgsVertexIterator vit = ombb.vertices();
  if ( !vit.hasNext() )
  {
    throw QgsProcessingException( QObject::tr( "OMBB has no vertices." ) );
  }
  QgsPoint ombbOrigin = vit.next();

  // Get the second vertex to determine the direction vector
  if ( !vit.hasNext() )
  {
    throw QgsProcessingException( QObject::tr( "OMBB has insufficient vertices." ) );
  }
  QgsPoint ombbSecond = vit.next();

  // Calculate direction vectors for the OMBB edges
  double dx = ombbSecond.x() - ombbOrigin.x();
  double dy = ombbSecond.y() - ombbOrigin.y();
  double edgeLength = std::sqrt( dx * dx + dy * dy );

  if ( edgeLength < 1e-8 )
  {
    throw QgsProcessingException( QObject::tr( "OMBB has zero-length edge." ) );
  }

  // Unit vectors along OMBB edges
  double ux = dx / edgeLength; // x direction (along first edge)
  double uy = dy / edgeLength; // y direction (along first edge)
  // Perpendicular direction (90 degrees counterclockwise)
  double vx = -uy;
  double vy = ux;

  // Get page size and convert to map units
  QgsLayoutSize pageSize = getPageSize();

  // TODO: have to deal with CRS unit
  QgsLayoutMeasurementConverter converter;
  QgsLayoutSize pageSizeInMm = converter.convert( pageSize, Qgis::LayoutUnit::Millimeters );

  double folioWidth = ( pageSizeInMm.width() / 1000.0 ) * mScale;
  double folioHeight = ( pageSizeInMm.height() / 1000.0 ) * mScale;

  // Calculate effective spacing (folio size minus overlap)
  double effectiveW = folioWidth - mHOverlay;
  double effectiveH = folioHeight - mVOverlay;

  if ( effectiveW <= 0 || effectiveH <= 0 )
  {
    throw QgsProcessingException( QObject::tr( "Overlap must be smaller than folio dimensions. "
                                               "Folio width: %1, height: %2, horizontal overlap: %3, vertical overlap: %4" )
                                    .arg( folioWidth )
                                    .arg( folioHeight )
                                    .arg( mHOverlay )
                                    .arg( mVOverlay ) );
  }

  // Determine which OMBB dimension corresponds to each direction
  // Compare computed edge length with reported width/height to identify dimensions
  double ombbDim1 = edgeLength; // Length along first edge (u direction)
  double ombbDim2 = ( std::abs( edgeLength - ombbWidth ) < 1e-8 ) ? ombbHeight : ombbWidth;

  // Calculate grid dimensions
  long long cols = static_cast<long long>( std::ceil( ombbDim1 / effectiveW ) );
  long long rows = static_cast<long long>( std::ceil( ombbDim2 / effectiveH ) );

  if ( cols <= 0 )
    cols = 1;
  if ( rows <= 0 )
    rows = 1;

  const long long totalCells = cols * rows;

  // Define output fields
  QgsFields fields;
  fields.append( QgsField( u"id"_s, QMetaType::Type::LongLong ) );
  fields.append( QgsField( u"row_index"_s, QMetaType::Type::LongLong ) );
  fields.append( QgsField( u"col_index"_s, QMetaType::Type::LongLong ) );
  fields.append( QgsField( u"angle"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( u"layout"_s, QMetaType::Type::QString, QString(), 50 ) );
  fields.append( QgsField( u"scale"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"left"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( u"top"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( u"right"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  fields.append( QgsField( u"bottom"_s, QMetaType::Type::Double, QString(), 20, 6 ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  // Prepare geometry engine for intersection tests if avoiding empty cells
  std::unique_ptr<QgsGeometryEngine> engine;
  if ( mAvoidEmpty )
  {
    engine.reset( QgsGeometry::createGeometryEngine( mDissolvedGeometry.constGet() ) );
    engine->prepareGeometry();
  }

  // Get page size name for output
  QString pageSizeName;
  if ( mPageSizeIndex >= mPageSizeNames.size() - 1 )
  {
    pageSizeName = QObject::tr( "Custom (%1x%2 mm)" ).arg( pageSizeInMm.width() ).arg( pageSizeInMm.height() );
  }
  else
  {
    pageSizeName = mPageSizeNames.at( mPageSizeIndex );
  }

  // Generate grid cells
  long long id = 1;
  long long processed = 0;
  int lastProgress = 10;

  for ( long long col = 0; col < cols; col++ )
  {
    for ( long long row = 0; row < rows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      // Calculate cell position in OMBB local coordinates
      double localX = static_cast<double>( col ) * effectiveW;
      double localY = static_cast<double>( row ) * effectiveH;

      // Transform to world coordinates using OMBB orientation
      // Corner 0 (origin)
      double x0 = ombbOrigin.x() + localX * ux + localY * vx;
      double y0 = ombbOrigin.y() + localX * uy + localY * vy;

      // Corner 1 (along first edge)
      double x1 = ombbOrigin.x() + ( localX + folioWidth ) * ux + localY * vx;
      double y1 = ombbOrigin.y() + ( localX + folioWidth ) * uy + localY * vy;

      // Corner 2 (opposite corner)
      double x2 = ombbOrigin.x() + ( localX + folioWidth ) * ux + ( localY + folioHeight ) * vx;
      double y2 = ombbOrigin.y() + ( localX + folioWidth ) * uy + ( localY + folioHeight ) * vy;

      // Corner 3 (along perpendicular edge)
      double x3 = ombbOrigin.x() + localX * ux + ( localY + folioHeight ) * vx;
      double y3 = ombbOrigin.y() + localX * uy + ( localY + folioHeight ) * vy;

      // Create polygon
      QVector<double> ringX = { x0, x1, x2, x3, x0 };
      QVector<double> ringY = { y0, y1, y2, y3, y0 };

      auto poly = std::make_unique<QgsPolygon>();
      poly->setExteriorRing( new QgsLineString( ringX, ringY ) );
      QgsGeometry cellGeom( std::move( poly ) );

      // Skip if avoidance enabled and no intersection
      if ( mAvoidEmpty && engine && !engine->intersects( cellGeom.constGet() ) )
      {
        processed++;
        continue;
      }

      // Create feature with attributes
      QgsFeature f;
      f.setGeometry( cellGeom );

      // Calculate bounds in local (pre-rotation) coordinates for reference
      double left = localX;
      double top = localY;
      double right = localX + folioWidth;
      double bottom = localY + folioHeight;

      f.setAttributes( QgsAttributes() << id << row << col << ombbAngle << pageSizeName << mScale << left << top << right << bottom );

      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

      id++;
      processed++;

      if ( feedback && totalCells > 0 )
      {
        int currentProgress = static_cast<int>( 10.0 + 90.0 * static_cast<double>( processed ) / static_cast<double>( totalCells ) );
        if ( currentProgress != lastProgress )
        {
          lastProgress = currentProgress;
          feedback->setProgress( currentProgress );
        }
      }
    }

    if ( feedback && feedback->isCanceled() )
      break;
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
