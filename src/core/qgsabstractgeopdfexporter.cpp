/***************************************************************************
                              qgsabtractgeopdfexporter.cpp
                             --------------------------
    begin                : August 2019
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

#include "qgsabstractgeopdfexporter.h"

#include <cpl_string.h>
#include <gdal.h>

#include "qgscoordinatetransformcontext.h"
#include "qgsfileutils.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QDomDocument>
#include <QDomElement>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QTextStream>
#include <QTimeZone>
#include <QUuid>

using namespace Qt::StringLiterals;

bool QgsAbstractGeospatialPdfExporter::geospatialPDFCreationAvailable()
{
  // test if GDAL has read support in PDF driver
  GDALDriverH hDriverMem = GDALGetDriverByName( "PDF" );
  if ( !hDriverMem )
  {
    return false;
  }

  const char *pHavePoppler = GDALGetMetadataItem( hDriverMem, "HAVE_POPPLER", nullptr );
  if ( pHavePoppler && strstr( pHavePoppler, "YES" ) )
    return true;

  const char *pHavePdfium = GDALGetMetadataItem( hDriverMem, "HAVE_PDFIUM", nullptr );
  if ( pHavePdfium && strstr( pHavePdfium, "YES" ) )
    return true;

  return false;
}

QString QgsAbstractGeospatialPdfExporter::geospatialPDFAvailabilityExplanation()
{
  // test if GDAL has read support in PDF driver
  GDALDriverH hDriverMem = GDALGetDriverByName( "PDF" );
  if ( !hDriverMem )
  {
    return QObject::tr( "No GDAL PDF driver available." );
  }

  const char *pHavePoppler = GDALGetMetadataItem( hDriverMem, "HAVE_POPPLER", nullptr );
  if ( pHavePoppler && strstr( pHavePoppler, "YES" ) )
    return QString();

  const char *pHavePdfium = GDALGetMetadataItem( hDriverMem, "HAVE_PDFIUM", nullptr );
  if ( pHavePdfium && strstr( pHavePdfium, "YES" ) )
    return QString();

  return QObject::tr( "GDAL PDF driver was not built with PDF read support. A build with PDF read support is required for geospatial PDF creation." );
}

void CPL_STDCALL collectErrors( CPLErr, int, const char *msg )
{
  QgsDebugError( u"GDAL PDF creation error: %1 "_s.arg( msg ) );
  if ( QStringList *errorList = static_cast< QStringList * >( CPLGetErrorHandlerUserData() ) )
  {
    errorList->append( QString( msg ) );
  }
}

bool QgsAbstractGeospatialPdfExporter::finalize( const QList<ComponentLayerDetail> &components, const QString &destinationFile, const ExportDetails &details )
{
  if ( details.includeFeatures && !saveTemporaryLayers() )
    return false;

  const QString composition = createCompositionXml( components, details );
  QgsDebugMsgLevel( composition, 2 );
  if ( composition.isEmpty() )
    return false;

  // do the creation!
  GDALDriverH driver = GDALGetDriverByName( "PDF" );
  if ( !driver )
  {
    mErrorMessage = QObject::tr( "Cannot load GDAL PDF driver" );
    return false;
  }

  const QString xmlFilePath = generateTemporaryFilepath( u"composition.xml"_s );
  QFile file( xmlFilePath );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QTextStream out( &file );
    out << composition;
  }
  else
  {
    mErrorMessage = QObject::tr( "Could not create geospatial PDF composition file" );
    return false;
  }

  char **papszOptions = CSLSetNameValue( nullptr, "COMPOSITION_FILE", xmlFilePath.toUtf8().constData() );

  QStringList creationErrors;
  CPLPushErrorHandlerEx( collectErrors, &creationErrors );

  // return a non-null (fake) dataset in case of success, nullptr otherwise.
  gdal::dataset_unique_ptr outputDataset( GDALCreate( driver, destinationFile.toUtf8().constData(), 0, 0, 0, GDT_Unknown, papszOptions ) );

  CPLPopErrorHandler();
  // Keep explicit comparison to avoid confusing cppcheck
  const bool res = outputDataset.get() != nullptr;
  if ( !res )
  {
    if ( creationErrors.size() == 1 )
    {
      mErrorMessage = QObject::tr( "Could not create PDF file: %1" ).arg( creationErrors.at( 0 ) );
    }
    else if ( !creationErrors.empty() )
    {
      mErrorMessage = QObject::tr( "Could not create PDF file. Received errors:\n" );
      for ( const QString &error : std::as_const( creationErrors ) )
      {
        mErrorMessage += ( !mErrorMessage.isEmpty() ? u"\n"_s : QString() ) + error;
      }

    }
    else
    {
      mErrorMessage = QObject::tr( "Could not create PDF file, but no error details are available" );
    }
  }
  outputDataset.reset();

  CSLDestroy( papszOptions );

  return res;
}

QString QgsAbstractGeospatialPdfExporter::generateTemporaryFilepath( const QString &filename ) const
{
  return mTemporaryDir.filePath( QgsFileUtils::stringToSafeFilename( filename ) );
}

bool QgsAbstractGeospatialPdfExporter::compositionModeSupported( QPainter::CompositionMode mode )
{
  switch ( mode )
  {
    case QPainter::CompositionMode_SourceOver:
    case QPainter::CompositionMode_Multiply:
    case QPainter::CompositionMode_Screen:
    case QPainter::CompositionMode_Overlay:
    case QPainter::CompositionMode_Darken:
    case QPainter::CompositionMode_Lighten:
    case QPainter::CompositionMode_ColorDodge:
    case QPainter::CompositionMode_ColorBurn:
    case QPainter::CompositionMode_HardLight:
    case QPainter::CompositionMode_SoftLight:
    case QPainter::CompositionMode_Difference:
    case  QPainter::CompositionMode_Exclusion:
      return true;

    default:
      break;
  }

  return false;
}

void QgsAbstractGeospatialPdfExporter::pushRenderedFeature( const QString &layerId, const QgsAbstractGeospatialPdfExporter::RenderedFeature &feature, const QString &group )
{
  // because map layers may be rendered in parallel, we need a mutex here
  QMutexLocker locker( &mMutex );

  // collate all the features which belong to the same layer, replacing their geometries with the rendered feature bounds
  QgsFeature f = feature.feature;
  f.setGeometry( feature.renderedBounds );
  mCollatedFeatures[ group ][ layerId ].append( f );
}

bool QgsAbstractGeospatialPdfExporter::saveTemporaryLayers()
{
  for ( auto groupIt = mCollatedFeatures.constBegin(); groupIt != mCollatedFeatures.constEnd(); ++groupIt )
  {
    for ( auto it = groupIt->constBegin(); it != groupIt->constEnd(); ++it )
    {
      const QString filePath = generateTemporaryFilepath( it.key() + groupIt.key() + u".gpkg"_s );

      VectorComponentDetail detail = componentDetailForLayerId( it.key() );
      detail.sourceVectorPath = filePath;
      detail.group = groupIt.key();

      // write out features to disk
      const QgsFeatureList features = it.value();
      QString layerName;
      QgsVectorFileWriter::SaveVectorOptions saveOptions;
      saveOptions.driverName = u"GPKG"_s;
      saveOptions.symbologyExport = Qgis::FeatureSymbologyExport::NoSymbology;
      std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( filePath, features.first().fields(), features.first().geometry().wkbType(), QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext(), saveOptions, QgsFeatureSink::RegeneratePrimaryKey, nullptr, &layerName ) );
      if ( writer->hasError() )
      {
        mErrorMessage = writer->errorMessage();
        QgsDebugError( mErrorMessage );
        return false;
      }
      for ( const QgsFeature &feature : features )
      {
        QgsFeature f = feature;
        if ( !writer->addFeature( f, QgsFeatureSink::FastInsert ) )
        {
          mErrorMessage = writer->errorMessage();
          QgsDebugError( mErrorMessage );
          return false;
        }
      }
      detail.sourceVectorLayer = layerName;
      mVectorComponents << detail;
    }
  }
  return true;
}

QString QgsAbstractGeospatialPdfExporter::createCompositionXml( const QList<ComponentLayerDetail> &components, const ExportDetails &details ) const
{
  QDomDocument doc;
  QDomElement compositionElem = doc.createElement( u"PDFComposition"_s );

  // metadata
  createMetadataXmlSection( compositionElem, doc, details );

  // pages
  QDomElement page = doc.createElement( u"Page"_s );
  const double pageWidthPdfUnits = std::ceil( details.pageSizeMm.width() / 25.4 * 72 );
  const double pageHeightPdfUnits = std::ceil( details.pageSizeMm.height() / 25.4 * 72 );
  createPageDimensionXmlSection( page, doc, pageWidthPdfUnits, pageHeightPdfUnits );

  // georeferencing
  createGeoreferencingXmlSection( page, doc, details, pageWidthPdfUnits, pageHeightPdfUnits );

  // layer tree and content
  QgsLayerTree *layerTree = qgisLayerTree();
  if ( details.useQgisLayerTreeProperties && layerTree )
  {
    createLayerTreeAndContentXmlSectionsFromLayerTree( layerTree, compositionElem, page, doc, components, details );
  }
  else
  {
    createLayerTreeAndContentXmlSections( compositionElem, page, doc, components, details );
  }

  compositionElem.appendChild( page );
  doc.appendChild( compositionElem );

  QString composition;
  QTextStream stream( &composition );
  doc.save( stream, -1 );

  return composition;
}

void QgsAbstractGeospatialPdfExporter::createMetadataXmlSection( QDomElement &compositionElem, QDomDocument &doc, const ExportDetails &details ) const
{
  // metadata tags
  QDomElement metadata = doc.createElement( u"Metadata"_s );
  if ( !details.author.isEmpty() )
  {
    QDomElement author = doc.createElement( u"Author"_s );
    author.appendChild( doc.createTextNode( details.author ) );
    metadata.appendChild( author );
  }
  if ( !details.producer.isEmpty() )
  {
    QDomElement producer = doc.createElement( u"Producer"_s );
    producer.appendChild( doc.createTextNode( details.producer ) );
    metadata.appendChild( producer );
  }
  if ( !details.creator.isEmpty() )
  {
    QDomElement creator = doc.createElement( u"Creator"_s );
    creator.appendChild( doc.createTextNode( details.creator ) );
    metadata.appendChild( creator );
  }
  if ( details.creationDateTime.isValid() )
  {
    QDomElement creationDate = doc.createElement( u"CreationDate"_s );
    QString creationDateString = u"D:%1"_s.arg( details.creationDateTime.toString( u"yyyyMMddHHmmss"_s ) );
#if QT_FEATURE_timezone > 0
    if ( details.creationDateTime.timeZone().isValid() )
    {
      int offsetFromUtc = details.creationDateTime.timeZone().offsetFromUtc( details.creationDateTime );
      creationDateString += ( offsetFromUtc >= 0 ) ? '+' : '-';
      offsetFromUtc = std::abs( offsetFromUtc );
      int offsetHours = offsetFromUtc / 3600;
      int offsetMins = ( offsetFromUtc % 3600 ) / 60;
      creationDateString += u"%1'%2'"_s.arg( offsetHours ).arg( offsetMins );
    }
#else
    QgsDebugError( u"Qt is built without timezone support, skipping timezone for pdf export"_s );
#endif
    creationDate.appendChild( doc.createTextNode( creationDateString ) );
    metadata.appendChild( creationDate );
  }
  if ( !details.subject.isEmpty() )
  {
    QDomElement subject = doc.createElement( u"Subject"_s );
    subject.appendChild( doc.createTextNode( details.subject ) );
    metadata.appendChild( subject );
  }
  if ( !details.title.isEmpty() )
  {
    QDomElement title = doc.createElement( u"Title"_s );
    title.appendChild( doc.createTextNode( details.title ) );
    metadata.appendChild( title );
  }
  if ( !details.keywords.empty() )
  {
    QStringList allKeywords;
    for ( auto it = details.keywords.constBegin(); it != details.keywords.constEnd(); ++it )
    {
      allKeywords.append( u"%1: %2"_s.arg( it.key(), it.value().join( ',' ) ) );
    }
    QDomElement keywords = doc.createElement( u"Keywords"_s );
    keywords.appendChild( doc.createTextNode( allKeywords.join( ';' ) ) );
    metadata.appendChild( keywords );
  }
  compositionElem.appendChild( metadata );
}

void QgsAbstractGeospatialPdfExporter::createGeoreferencingXmlSection( QDomElement &pageElem, QDomDocument &doc, const ExportDetails &details, const double pageWidthPdfUnits, const double pageHeightPdfUnits ) const
{
  int i = 0;
  for ( const QgsAbstractGeospatialPdfExporter::GeoReferencedSection &section : details.georeferencedSections )
  {
    QDomElement georeferencing = doc.createElement( u"Georeferencing"_s );
    georeferencing.setAttribute( u"id"_s, u"georeferenced_%1"_s.arg( i++ ) );
    georeferencing.setAttribute( u"ISO32000ExtensionFormat"_s, details.useIso32000ExtensionFormatGeoreferencing ? u"true"_s : u"false"_s );

    if ( section.crs.isValid() )
    {
      QDomElement srs = doc.createElement( u"SRS"_s );
      // not currently used by GDAL or the PDF spec, but exposed in the GDAL XML schema. Maybe something we'll need to consider down the track...
      // srs.setAttribute( u"dataAxisToSRSAxisMapping"_s, u"2,1"_s );
      if ( !section.crs.authid().isEmpty() && !section.crs.authid().startsWith( u"user"_s, Qt::CaseInsensitive ) )
      {
        srs.appendChild( doc.createTextNode( section.crs.authid() ) );
      }
      else
      {
        srs.appendChild( doc.createTextNode( section.crs.toWkt( Qgis::CrsWktVariant::PreferredGdal ) ) );
      }
      georeferencing.appendChild( srs );
    }

    if ( !section.pageBoundsPolygon.isEmpty() )
    {
      /*
        Define a polygon / neatline in PDF units into which the
        Measure tool will display coordinates.
        If not specified, BoundingBox will be used instead.
        If none of BoundingBox and BoundingPolygon are specified,
        the whole PDF page will be assumed to be georeferenced.
       */
      QDomElement boundingPolygon = doc.createElement( u"BoundingPolygon"_s );

      // transform to PDF coordinate space
      QTransform t = QTransform::fromTranslate( 0, pageHeightPdfUnits ).scale( pageWidthPdfUnits / details.pageSizeMm.width(),
                     -pageHeightPdfUnits / details.pageSizeMm.height() );

      QgsPolygon p = section.pageBoundsPolygon;
      p.transform( t );
      boundingPolygon.appendChild( doc.createTextNode( p.asWkt() ) );

      georeferencing.appendChild( boundingPolygon );
    }
    else
    {
      /* Define the viewport where georeferenced coordinates are available.
        If not specified, the extent of BoundingPolygon will be used instead.
        If none of BoundingBox and BoundingPolygon are specified,
        the whole PDF page will be assumed to be georeferenced.
        */
      QDomElement boundingBox = doc.createElement( u"BoundingBox"_s );
      boundingBox.setAttribute( u"x1"_s, qgsDoubleToString( section.pageBoundsMm.xMinimum() / 25.4 * 72 ) );
      boundingBox.setAttribute( u"y1"_s, qgsDoubleToString( section.pageBoundsMm.yMinimum() / 25.4 * 72 ) );
      boundingBox.setAttribute( u"x2"_s, qgsDoubleToString( section.pageBoundsMm.xMaximum() / 25.4 * 72 ) );
      boundingBox.setAttribute( u"y2"_s, qgsDoubleToString( section.pageBoundsMm.yMaximum() / 25.4 * 72 ) );
      georeferencing.appendChild( boundingBox );
    }

    for ( const ControlPoint &point : section.controlPoints )
    {
      QDomElement cp1 = doc.createElement( u"ControlPoint"_s );
      cp1.setAttribute( u"x"_s, qgsDoubleToString( point.pagePoint.x() / 25.4 * 72 ) );
      cp1.setAttribute( u"y"_s, qgsDoubleToString( ( details.pageSizeMm.height() - point.pagePoint.y() ) / 25.4 * 72 ) );
      cp1.setAttribute( u"GeoX"_s, qgsDoubleToString( point.geoPoint.x() ) );
      cp1.setAttribute( u"GeoY"_s, qgsDoubleToString( point.geoPoint.y() ) );
      georeferencing.appendChild( cp1 );
    }

    pageElem.appendChild( georeferencing );
  }
}

void QgsAbstractGeospatialPdfExporter::createPageDimensionXmlSection( QDomElement &pageElem, QDomDocument &doc, const double pageWidthPdfUnits, const double pageHeightPdfUnits ) const
{
  QDomElement dpi = doc.createElement( u"DPI"_s );
  // hardcode DPI of 72 to get correct page sizes in outputs -- refs discussion in https://github.com/OSGeo/gdal/pull/2961
  dpi.appendChild( doc.createTextNode( qgsDoubleToString( 72 ) ) );
  pageElem.appendChild( dpi );
  // assumes DPI of 72, as noted above.
  QDomElement width = doc.createElement( u"Width"_s );
  width.appendChild( doc.createTextNode( qgsDoubleToString( pageWidthPdfUnits ) ) );
  pageElem.appendChild( width );
  QDomElement height = doc.createElement( u"Height"_s );
  height.appendChild( doc.createTextNode( qgsDoubleToString( pageHeightPdfUnits ) ) );
  pageElem.appendChild( height );
}

void QgsAbstractGeospatialPdfExporter::createLayerTreeAndContentXmlSections( QDomElement &compositionElem, QDomElement &pageElem, QDomDocument &doc,  const QList<ComponentLayerDetail> &components, const ExportDetails &details ) const
{
  QSet< QString > createdLayerIds;
  std::vector< std::unique_ptr< TreeNode > > rootGroups;
  std::vector< std::unique_ptr< TreeNode > > rootLayers;
  QMap< QString, TreeNode * > groupNameMap;

  QStringList layerTreeGroupOrder = details.layerTreeGroupOrder;

  // add any missing groups to end of order
  // Missing groups from the explicitly set custom layer tree groups
  for ( auto it = details.customLayerTreeGroups.constBegin(); it != details.customLayerTreeGroups.constEnd(); ++it )
  {
    if ( layerTreeGroupOrder.contains( it.value() ) )
      continue;
    layerTreeGroupOrder.append( it.value() );
  }

  // Missing groups from vector components
  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : std::as_const( mVectorComponents ) )
    {
      if ( !component.group.isEmpty() && !layerTreeGroupOrder.contains( component.group ) )
      {
        layerTreeGroupOrder.append( component.group );
      }
    }
  }

  // missing groups from other components
  for ( const ComponentLayerDetail &component : components )
  {
    if ( !component.group.isEmpty() && !layerTreeGroupOrder.contains( component.group ) )
    {
      layerTreeGroupOrder.append( component.group );
    }
  }

  // now we are confident that we have a definitive list of all the groups for the export
  QMap< QString, TreeNode * > groupNameToTreeNode;
  QMap< QString, TreeNode * > layerIdToTreeNode;

  auto createGroup = [&details, &groupNameToTreeNode]( const QString & groupName ) -> std::unique_ptr< TreeNode >
  {
    auto group = std::make_unique< TreeNode >();
    const QString id = QUuid::createUuid().toString();
    group->id = id;
    groupNameToTreeNode[ groupName ] = group.get();

    group->name = groupName;
    group->initiallyVisible = true;
    if ( details.mutuallyExclusiveGroups.contains( groupName ) )
      group->mutuallyExclusiveGroupId = u"__mutually_exclusive_groups__"_s;
    return group;
  };

  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : std::as_const( mVectorComponents ) )
    {
      const QString destinationGroup = details.customLayerTreeGroups.value( component.mapLayerId, component.group );

      auto layer = std::make_unique< TreeNode >();
      layer->id = destinationGroup.isEmpty() ? component.mapLayerId : u"%1_%2"_s.arg( destinationGroup, component.mapLayerId );
      layer->name = details.layerIdToPdfLayerTreeNameMap.contains( component.mapLayerId ) ? details.layerIdToPdfLayerTreeNameMap.value( component.mapLayerId ) : component.name;
      layer->initiallyVisible = details.initialLayerVisibility.value( component.mapLayerId, true );
      layer->mapLayerId = component.mapLayerId;

      layerIdToTreeNode.insert( component.mapLayerId, layer.get() );
      if ( !destinationGroup.isEmpty() )
      {
        if ( TreeNode *groupNode = groupNameMap.value( destinationGroup ) )
        {
          groupNode->addChild( std::move( layer ) );
        }
        else
        {
          std::unique_ptr< TreeNode > group = createGroup( destinationGroup );
          group->addChild( std::move( layer ) );
          groupNameMap.insert( destinationGroup, group.get() );
          rootGroups.emplace_back( std::move( group ) );
        }
      }
      else
      {
        rootLayers.emplace_back( std::move( layer ) );
      }

      createdLayerIds.insert( component.mapLayerId );
    }
  }

  // some PDF components may not be linked to vector components - e.g.
  // - layers with labels but no features
  // - raster layers
  // - legends and other map content
  for ( const ComponentLayerDetail &component : components )
  {
    if ( !component.mapLayerId.isEmpty() && createdLayerIds.contains( component.mapLayerId ) )
      continue;

    const QString destinationGroup = details.customLayerTreeGroups.value( component.mapLayerId, component.group );
    if ( destinationGroup.isEmpty() && component.mapLayerId.isEmpty() )
      continue;

    std::unique_ptr< TreeNode > mapLayerNode;
    if ( !component.mapLayerId.isEmpty() )
    {
      mapLayerNode = std::make_unique< TreeNode >();
      mapLayerNode->id = destinationGroup.isEmpty() ? component.mapLayerId : u"%1_%2"_s.arg( destinationGroup, component.mapLayerId );
      mapLayerNode->name = details.layerIdToPdfLayerTreeNameMap.value( component.mapLayerId, component.name );
      mapLayerNode->initiallyVisible = details.initialLayerVisibility.value( component.mapLayerId, true );
      mapLayerNode->mapLayerId = component.mapLayerId;

      layerIdToTreeNode.insert( component.mapLayerId, mapLayerNode.get() );
    }

    if ( !destinationGroup.isEmpty() )
    {
      if ( TreeNode *groupNode = groupNameMap.value( destinationGroup ) )
      {
        if ( mapLayerNode )
          groupNode->addChild( std::move( mapLayerNode ) );
      }
      else
      {
        std::unique_ptr< TreeNode > group = createGroup( destinationGroup );
        if ( mapLayerNode )
          group->addChild( std::move( mapLayerNode ) );
        groupNameMap.insert( destinationGroup, group.get() );
        rootGroups.emplace_back( std::move( group ) );
      }
    }
    else
    {
      if ( mapLayerNode )
        rootLayers.emplace_back( std::move( mapLayerNode ) );
    }

    if ( !component.mapLayerId.isEmpty() )
    {
      createdLayerIds.insert( component.mapLayerId );
    }
  }

  auto createPdfDatasetElement = [&doc]( const ComponentLayerDetail & component ) -> QDomElement
  {
    QDomElement pdfDataset = doc.createElement( u"PDF"_s );
    pdfDataset.setAttribute( u"dataset"_s, component.sourcePdfPath );
    if ( component.opacity != 1.0 || component.compositionMode != QPainter::CompositionMode_SourceOver )
    {
      QDomElement blendingElement = doc.createElement( u"Blending"_s );
      blendingElement.setAttribute( u"opacity"_s, component.opacity );
      blendingElement.setAttribute( u"function"_s, compositionModeToString( component.compositionMode ) );

      pdfDataset.appendChild( blendingElement );
    }
    return pdfDataset;
  };

  // content
  QDomElement content = doc.createElement( u"Content"_s );
  for ( const ComponentLayerDetail &component : components )
  {
    if ( component.mapLayerId.isEmpty() && component.group.isEmpty() )
    {
      content.appendChild( createPdfDatasetElement( component ) );
    }
    else if ( !component.mapLayerId.isEmpty() )
    {
      if ( TreeNode *treeNode = layerIdToTreeNode.value( component.mapLayerId ) )
      {
        QDomElement ifLayerOnElement = treeNode->createNestedIfLayerOnElements( doc, content );
        ifLayerOnElement.appendChild( createPdfDatasetElement( component ) );
      }
    }
    else if ( TreeNode *groupNode = groupNameToTreeNode.value( component.group ) )
    {
      QDomElement ifGroupOn = groupNode->createIfLayerOnElement( doc, content );
      ifGroupOn.appendChild( createPdfDatasetElement( component ) );
    }
  }

  // vector datasets (we "draw" these on top, just for debugging... but they are invisible, so are never really drawn!)
  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : std::as_const( mVectorComponents ) )
    {
      if ( TreeNode *treeNode = layerIdToTreeNode.value( component.mapLayerId ) )
      {
        QDomElement ifLayerOnElement = treeNode->createNestedIfLayerOnElements( doc, content );

        QDomElement vectorDataset = doc.createElement( u"Vector"_s );
        vectorDataset.setAttribute( u"dataset"_s, component.sourceVectorPath );
        vectorDataset.setAttribute( u"layer"_s, component.sourceVectorLayer );
        vectorDataset.setAttribute( u"visible"_s, u"false"_s );
        QDomElement logicalStructure = doc.createElement( u"LogicalStructure"_s );
        logicalStructure.setAttribute( u"displayLayerName"_s, component.name );
        if ( !component.displayAttribute.isEmpty() )
          logicalStructure.setAttribute( u"fieldToDisplay"_s, component.displayAttribute );
        vectorDataset.appendChild( logicalStructure );
        ifLayerOnElement.appendChild( vectorDataset );
      }
    }
  }

  pageElem.appendChild( content );

  // layertree
  QDomElement layerTree = doc.createElement( u"LayerTree"_s );
  //layerTree.setAttribute( u"displayOnlyOnVisiblePages"_s, u"true"_s);

  // groups are added first

  // sort root groups in desired order
  std::sort( rootGroups.begin(), rootGroups.end(), [&layerTreeGroupOrder]( const std::unique_ptr< TreeNode > &a, const std::unique_ptr< TreeNode > &b ) -> bool
  {
    return layerTreeGroupOrder.indexOf( a->name ) < layerTreeGroupOrder.indexOf( b->name );
  } );

  bool haveFoundMutuallyExclusiveGroup = false;
  for ( const auto &node : std::as_const( rootGroups ) )
  {
    if ( !node->mutuallyExclusiveGroupId.isEmpty() )
    {
      // only the first object in a mutually exclusive group is initially visible
      node->initiallyVisible = !haveFoundMutuallyExclusiveGroup;
      haveFoundMutuallyExclusiveGroup = true;
    }
    layerTree.appendChild( node->toElement( doc ) );
  }

  // filter out groups which don't have any content
  layerTreeGroupOrder.erase( std::remove_if( layerTreeGroupOrder.begin(), layerTreeGroupOrder.end(), [&details]( const QString & group )
  {
    return details.customLayerTreeGroups.key( group ).isEmpty();
  } ), layerTreeGroupOrder.end() );


  // then top-level layers
  std::sort( rootLayers.begin(), rootLayers.end(), [&details]( const std::unique_ptr< TreeNode > &a, const std::unique_ptr< TreeNode > &b ) -> bool
  {
    const int indexA = details.layerOrder.indexOf( a->mapLayerId );
    const int indexB = details.layerOrder.indexOf( b->mapLayerId );

    if ( indexA >= 0 && indexB >= 0 )
      return indexA < indexB;
    else if ( indexA >= 0 )
      return true;
    else if ( indexB >= 0 )
      return false;

    return a->name.localeAwareCompare( b->name ) < 0;
  } );

  for ( const auto &node : std::as_const( rootLayers ) )
  {
    layerTree.appendChild( node->toElement( doc ) );
  }

  compositionElem.appendChild( layerTree );
}

void QgsAbstractGeospatialPdfExporter::createLayerTreeAndContentXmlSectionsFromLayerTree( const QgsLayerTree *layerTree, QDomElement &compositionElem, QDomElement &pageElem, QDomDocument &doc,  const QList<ComponentLayerDetail> &components, const ExportDetails &details ) const
{
  QMap< QString, TreeNode * > groupNameToTreeNode;
  QMap< QString, TreeNode * > layerIdToTreeNode;

  // Add tree structure from QGIS layer tree to the intermediate TreeNode struct
  std::unique_ptr< TreeNode > rootPdfNode = createPdfTreeNodes( groupNameToTreeNode, layerIdToTreeNode, layerTree );
  rootPdfNode->isRootNode = true; // To skip the layer tree root from the PDF layer tree

  // Add missing groups from other components
  for ( const ComponentLayerDetail &component : components )
  {
    if ( !component.group.isEmpty() && !groupNameToTreeNode.contains( component.group ) )
    {
      auto pdfTreeGroup = std::make_unique< TreeNode >();
      const QString id = QUuid::createUuid().toString();
      pdfTreeGroup->id = id;
      pdfTreeGroup->name = component.group;
      pdfTreeGroup->initiallyVisible = true;
      groupNameToTreeNode[ pdfTreeGroup->name ] = pdfTreeGroup.get();
      rootPdfNode->addChild( std::move( pdfTreeGroup ) );
    }
  }

  auto createPdfDatasetElement = [&doc]( const ComponentLayerDetail & component ) -> QDomElement
  {
    QDomElement pdfDataset = doc.createElement( u"PDF"_s );
    pdfDataset.setAttribute( u"dataset"_s, component.sourcePdfPath );
    if ( component.opacity != 1.0 || component.compositionMode != QPainter::CompositionMode_SourceOver )
    {
      QDomElement blendingElement = doc.createElement( u"Blending"_s );
      blendingElement.setAttribute( u"opacity"_s, component.opacity );
      blendingElement.setAttribute( u"function"_s, compositionModeToString( component.compositionMode ) );

      pdfDataset.appendChild( blendingElement );
    }
    return pdfDataset;
  };

  // PDF Content
  QDomElement content = doc.createElement( u"Content"_s );
  for ( const ComponentLayerDetail &component : components )
  {
    if ( component.mapLayerId.isEmpty() && component.group.isEmpty() )
    {
      content.appendChild( createPdfDatasetElement( component ) );
    }
    else if ( !component.mapLayerId.isEmpty() )
    {
      if ( TreeNode *treeNode = layerIdToTreeNode.value( component.mapLayerId ) )
      {
        QDomElement ifLayerOnElement = treeNode->createNestedIfLayerOnElements( doc, content );
        ifLayerOnElement.appendChild( createPdfDatasetElement( component ) );
      }
    }
    else if ( TreeNode *groupNode = groupNameToTreeNode.value( component.group ) )
    {
      QDomElement ifGroupOn = groupNode->createIfLayerOnElement( doc, content );
      ifGroupOn.appendChild( createPdfDatasetElement( component ) );
    }
  }

  // vector datasets (we "draw" these on top, just for debugging... but they are invisible, so are never really drawn!)
  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : std::as_const( mVectorComponents ) )
    {
      if ( TreeNode *treeNode = layerIdToTreeNode.value( component.mapLayerId ) )
      {
        QDomElement ifLayerOnElement = treeNode->createNestedIfLayerOnElements( doc, content );

        QDomElement vectorDataset = doc.createElement( u"Vector"_s );
        vectorDataset.setAttribute( u"dataset"_s, component.sourceVectorPath );
        vectorDataset.setAttribute( u"layer"_s, component.sourceVectorLayer );
        vectorDataset.setAttribute( u"visible"_s, u"false"_s );
        QDomElement logicalStructure = doc.createElement( u"LogicalStructure"_s );
        logicalStructure.setAttribute( u"displayLayerName"_s, component.name );
        if ( !component.displayAttribute.isEmpty() )
          logicalStructure.setAttribute( u"fieldToDisplay"_s, component.displayAttribute );
        vectorDataset.appendChild( logicalStructure );
        ifLayerOnElement.appendChild( vectorDataset );
      }
    }
  }

  pageElem.appendChild( content );

  // PDF Layer Tree
  QDomElement layerTreeElem = doc.createElement( u"LayerTree"_s );
  rootPdfNode->toChildrenElements( doc, layerTreeElem );
  compositionElem.appendChild( layerTreeElem );
}

std::unique_ptr< TreeNode > QgsAbstractGeospatialPdfExporter::createPdfTreeNodes( QMap< QString, TreeNode * > &groupNameToTreeNode, QMap< QString, TreeNode * > &layerIdToTreeNode, const QgsLayerTreeGroup *layerTreeGroup ) const
{
  auto pdfTreeNodes = std::make_unique< TreeNode >();
  const QString id = QUuid::createUuid().toString();
  pdfTreeNodes->id = id;
  pdfTreeNodes->name = layerTreeGroup->name();
  pdfTreeNodes->initiallyVisible = layerTreeGroup->itemVisibilityChecked();

  const QList<QgsLayerTreeNode *> groupChildren = layerTreeGroup->children();

  for ( QgsLayerTreeNode *qgisNode : groupChildren )
  {
    switch ( qgisNode->nodeType() )
    {
      case QgsLayerTreeNode::NodeLayer:
      {
        QgsLayerTreeLayer *layerTreeLayer = qobject_cast<QgsLayerTreeLayer *>( qgisNode );

        // Skip invalid layers, tables and unknown geometry types, since they won't appear in the PDF
        if ( !layerTreeLayer->layer()->isValid() )
          break;

        if ( layerTreeLayer->layer()->type() == Qgis::LayerType::Vector )
        {
          QgsVectorLayer *vectorLayer = qobject_cast< QgsVectorLayer * >( layerTreeLayer->layer() );
          if ( vectorLayer->geometryType() == Qgis::GeometryType::Unknown || vectorLayer->geometryType() == Qgis::GeometryType::Null )
            break;
        }

        auto pdfLayerNode = std::make_unique< TreeNode >();
        pdfLayerNode->id = layerTreeLayer->layerId();
        pdfLayerNode->name = layerTreeLayer->name();
        pdfLayerNode->initiallyVisible = layerTreeLayer->itemVisibilityChecked();
        pdfLayerNode->mapLayerId = layerTreeLayer->layerId();
        layerIdToTreeNode.insert( pdfLayerNode->id, pdfLayerNode.get() );
        pdfTreeNodes->addChild( std::move( pdfLayerNode ) );
        break;
      }

      case QgsLayerTreeNode::NodeGroup:
      {
        QgsLayerTreeGroup *childLayerTreeGroup = qobject_cast<QgsLayerTreeGroup *>( qgisNode );

        // GroupLayers support
        if ( QgsGroupLayer *groupLayer = childLayerTreeGroup->groupLayer() )
        {
          // We deal with it as another map layer
          auto pdfLayerNode = std::make_unique< TreeNode >();
          pdfLayerNode->id = groupLayer->id();
          pdfLayerNode->name = childLayerTreeGroup->name();
          pdfLayerNode->initiallyVisible = childLayerTreeGroup->itemVisibilityChecked();
          pdfLayerNode->mapLayerId = groupLayer->id();
          layerIdToTreeNode.insert( pdfLayerNode->id, pdfLayerNode.get() );
          pdfTreeNodes->addChild( std::move( pdfLayerNode ) );
          break;
        }

        // Skip empty groups
        if ( !childLayerTreeGroup->children().empty() )
        {
          std::unique_ptr< TreeNode > pdfGroupNode = createPdfTreeNodes( groupNameToTreeNode, layerIdToTreeNode, childLayerTreeGroup );

          // A group that is not empty in the QGIS layer tree, may be emptied
          // if it only contais invalid and/or geometryless layers. Skip it!
          if ( !pdfGroupNode->children.empty() )
          {
            pdfTreeNodes->addChild( std::move( pdfGroupNode ) );
          }
        }
        break;
      }

      case QgsLayerTreeNode::NodeCustom:
        break;
    }
  }

  // Now we know if our group is not empty. Add it to the groupNameToTreeNode then.
  if ( !pdfTreeNodes->children.empty() )
  {
    groupNameToTreeNode[ pdfTreeNodes->name ] = pdfTreeNodes.get();
  }

  return pdfTreeNodes;
}

QString QgsAbstractGeospatialPdfExporter::compositionModeToString( QPainter::CompositionMode mode )
{
  switch ( mode )
  {
    case QPainter::CompositionMode_SourceOver:
      return u"Normal"_s;

    case QPainter::CompositionMode_Multiply:
      return u"Multiply"_s;

    case QPainter::CompositionMode_Screen:
      return u"Screen"_s;

    case QPainter::CompositionMode_Overlay:
      return u"Overlay"_s;

    case QPainter::CompositionMode_Darken:
      return u"Darken"_s;

    case QPainter::CompositionMode_Lighten:
      return u"Lighten"_s;

    case QPainter::CompositionMode_ColorDodge:
      return u"ColorDodge"_s;

    case QPainter::CompositionMode_ColorBurn:
      return u"ColorBurn"_s;

    case QPainter::CompositionMode_HardLight:
      return u"HardLight"_s;

    case QPainter::CompositionMode_SoftLight:
      return u"SoftLight"_s;

    case QPainter::CompositionMode_Difference:
      return u"Difference"_s;

    case  QPainter::CompositionMode_Exclusion:
      return u"Exclusion"_s;

    default:
      break;
  }

  QgsDebugError( u"Unsupported PDF blend mode %1"_s.arg( mode ) );
  return u"Normal"_s;
}

