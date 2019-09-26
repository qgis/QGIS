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
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsfeaturerequest.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorfilewriter.h"

#include <gdal.h>
#include "qgsgdalutils.h"
#include "cpl_string.h"

#include <QMutex>
#include <QMutexLocker>
#include <QDomDocument>
#include <QDomElement>


bool QgsAbstractGeoPdfExporter::geoPDFCreationAvailable()
{
#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,0,0)
  return false;
#else

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
#endif
}

QString QgsAbstractGeoPdfExporter::geoPDFAvailabilityExplanation()
{
#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,0,0)
  return QObject::tr( "GeoPDF creation requires GDAL version 3.0 or later." );
#else
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

  return QObject::tr( "GDAL PDF driver was not built with PDF read support. A build with PDF read support is required for GeoPDF creation." );
#endif
}

bool QgsAbstractGeoPdfExporter::finalize( const QList<ComponentLayerDetail> &components, const QString &destinationFile, const ExportDetails &details )
{
  if ( details.includeFeatures && !saveTemporaryLayers() )
    return false;

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,0,0)
  Q_UNUSED( components )
  Q_UNUSED( destinationFile )
  return false;
#else
  const QString composition = createCompositionXml( components, details );
  QgsDebugMsg( composition );
  if ( composition.isEmpty() )
    return false;

  // do the creation!
  GDALDriverH driver = GDALGetDriverByName( "PDF" );
  if ( !driver )
  {
    mErrorMessage = QObject::tr( "Cannot load GDAL PDF driver" );
    return false;
  }

  const QString xmlFilePath = generateTemporaryFilepath( QStringLiteral( "composition.xml" ) );
  QFile file( xmlFilePath );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QTextStream out( &file );
    out << composition;
  }
  else
  {
    mErrorMessage = QObject::tr( "Could not create GeoPDF composition file" );
    return false;
  }

  char **papszOptions = CSLSetNameValue( nullptr, "COMPOSITION_FILE", xmlFilePath.toUtf8().constData() );

  // return a non-null (fake) dataset in case of success, nullptr otherwise.
  gdal::dataset_unique_ptr outputDataset( GDALCreate( driver, destinationFile.toUtf8().constData(), 0, 0, 0, GDT_Unknown, papszOptions ) );
  bool res = outputDataset.get();
  outputDataset.reset();

  CSLDestroy( papszOptions );

  return res;
#endif
}

QString QgsAbstractGeoPdfExporter::generateTemporaryFilepath( const QString &filename ) const
{
  return mTemporaryDir.filePath( filename );
}


void QgsAbstractGeoPdfExporter::pushRenderedFeature( const QString &layerId, const QgsAbstractGeoPdfExporter::RenderedFeature &feature, const QString &group )
{
  // because map layers may be rendered in parallel, we need a mutex here
  QMutexLocker locker( &mMutex );

  // collate all the features which belong to the same layer, replacing their geometries with the rendered feature bounds
  QgsFeature f = feature.feature;
  f.setGeometry( feature.renderedBounds );
  mCollatedFeatures[ group ][ layerId ].append( f );
}

bool QgsAbstractGeoPdfExporter::saveTemporaryLayers()
{
  for ( auto groupIt = mCollatedFeatures.constBegin(); groupIt != mCollatedFeatures.constEnd(); ++groupIt )
  {
    for ( auto it = groupIt->constBegin(); it != groupIt->constEnd(); ++it )
    {
      const QString filePath = generateTemporaryFilepath( it.key() + groupIt.key() + QStringLiteral( ".gpkg" ) );

      VectorComponentDetail detail = componentDetailForLayerId( it.key() );
      detail.sourceVectorPath = filePath;
      detail.group = groupIt.key();

      // write out features to disk
      const QgsFeatureList features = it.value();
      QString layerName;
      QgsVectorFileWriter writer( filePath, QString(), features.first().fields(), features.first().geometry().wkbType(), QgsCoordinateReferenceSystem(), QStringLiteral( "GPKG" ), QStringList(), QStringList(), nullptr, QgsVectorFileWriter::NoSymbology, QgsFeatureSink::RegeneratePrimaryKey, &layerName );
      if ( writer.hasError() )
      {
        mErrorMessage = writer.errorMessage();
        QgsDebugMsg( mErrorMessage );
        return false;
      }
      for ( const QgsFeature &feature : features )
      {
        QgsFeature f = feature;
        if ( !writer.addFeature( f, QgsFeatureSink::FastInsert ) )
        {
          mErrorMessage = writer.errorMessage();
          QgsDebugMsg( mErrorMessage );
          return false;
        }
      }
      detail.sourceVectorLayer = layerName;
      mVectorComponents << detail;
    }
  }
  return true;
}

QString QgsAbstractGeoPdfExporter::createCompositionXml( const QList<ComponentLayerDetail> &components, const ExportDetails &details )
{
  QDomDocument doc;

  QDomElement compositionElem = doc.createElement( QStringLiteral( "PDFComposition" ) );

  // metadata tags
  QDomElement metadata = doc.createElement( QStringLiteral( "Metadata" ) );
  if ( !details.author.isEmpty() )
  {
    QDomElement author = doc.createElement( QStringLiteral( "Author" ) );
    author.appendChild( doc.createTextNode( details.author ) );
    metadata.appendChild( author );
  }
  if ( !details.producer.isEmpty() )
  {
    QDomElement producer = doc.createElement( QStringLiteral( "Producer" ) );
    producer.appendChild( doc.createTextNode( details.producer ) );
    metadata.appendChild( producer );
  }
  if ( !details.creator.isEmpty() )
  {
    QDomElement creator = doc.createElement( QStringLiteral( "Creator" ) );
    creator.appendChild( doc.createTextNode( details.creator ) );
    metadata.appendChild( creator );
  }
  if ( details.creationDateTime.isValid() )
  {
    QDomElement creationDate = doc.createElement( QStringLiteral( "CreationDate" ) );
    QString creationDateString = QStringLiteral( "D:%1" ).arg( details.creationDateTime.toString( QStringLiteral( "yyyyMMddHHmmss" ) ) );
    if ( details.creationDateTime.timeZone().isValid() )
    {
      int offsetFromUtc = details.creationDateTime.timeZone().offsetFromUtc( details.creationDateTime );
      creationDateString += ( offsetFromUtc >= 0 ) ? '+' : '-';
      offsetFromUtc = std::abs( offsetFromUtc );
      int offsetHours = offsetFromUtc / 3600;
      int offsetMins = ( offsetFromUtc % 3600 ) / 60;
      creationDateString += QStringLiteral( "%1'%2'" ).arg( offsetHours ).arg( offsetMins );
    }
    creationDate.appendChild( doc.createTextNode( creationDateString ) );
    metadata.appendChild( creationDate );
  }
  if ( !details.subject.isEmpty() )
  {
    QDomElement subject = doc.createElement( QStringLiteral( "Subject" ) );
    subject.appendChild( doc.createTextNode( details.subject ) );
    metadata.appendChild( subject );
  }
  if ( !details.title.isEmpty() )
  {
    QDomElement title = doc.createElement( QStringLiteral( "Title" ) );
    title.appendChild( doc.createTextNode( details.title ) );
    metadata.appendChild( title );
  }
  if ( !details.keywords.empty() )
  {
    QStringList allKeywords;
    for ( auto it = details.keywords.constBegin(); it != details.keywords.constEnd(); ++it )
    {
      allKeywords.append( QStringLiteral( "%1: %2" ).arg( it.key(), it.value().join( ',' ) ) );
    }
    QDomElement keywords = doc.createElement( QStringLiteral( "Keywords" ) );
    keywords.appendChild( doc.createTextNode( allKeywords.join( ';' ) ) );
    metadata.appendChild( keywords );
  }
  compositionElem.appendChild( metadata );

  // layertree
  QDomElement layerTree = doc.createElement( QStringLiteral( "LayerTree" ) );
  //layerTree.setAttribute( QStringLiteral("displayOnlyOnVisiblePages"), QStringLiteral("true"));
  QMap< QString, QSet< QString > > createdLayerIds;
  QMap< QString, QDomElement > groupLayerMap;
  QMap< QString, QString > customGroupNamesToIds;
  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : qgis::as_const( mVectorComponents ) )
    {
      if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
        continue;

      QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
      layer.setAttribute( QStringLiteral( "id" ), component.group.isEmpty() ? component.mapLayerId : QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
      layer.setAttribute( QStringLiteral( "name" ), component.name );
      layer.setAttribute( QStringLiteral( "initiallyVisible" ), QStringLiteral( "true" ) );

      if ( !component.group.isEmpty() )
      {
        if ( groupLayerMap.contains( component.group ) )
        {
          groupLayerMap[ component.group ].appendChild( layer );
        }
        else
        {
          QDomElement group = doc.createElement( QStringLiteral( "Layer" ) );
          group.setAttribute( QStringLiteral( "id" ), QStringLiteral( "group_%1" ).arg( component.group ) );
          group.setAttribute( QStringLiteral( "name" ), component.group );
          group.setAttribute( QStringLiteral( "initiallyVisible" ), groupLayerMap.empty() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
          group.setAttribute( QStringLiteral( "mutuallyExclusiveGroupId" ), QStringLiteral( "__mutually_exclusive_groups__" ) );
          layerTree.appendChild( group );
          group.appendChild( layer );
          groupLayerMap[ component.group ] = group;
        }
      }
      else
      {
        layerTree.appendChild( layer );
      }

      createdLayerIds[ component.group ].insert( component.mapLayerId );
    }
  }
  // some PDF components may not be linked to vector components - e.g. layers with labels but no features
  for ( const ComponentLayerDetail &component : components )
  {
    if ( component.mapLayerId.isEmpty() || createdLayerIds.value( component.group ).contains( component.mapLayerId ) )
      continue;

    if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
      continue;

    QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
    layer.setAttribute( QStringLiteral( "id" ), component.group.isEmpty() ? component.mapLayerId : QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
    layer.setAttribute( QStringLiteral( "name" ), component.name );
    layer.setAttribute( QStringLiteral( "initiallyVisible" ), QStringLiteral( "true" ) );

    if ( !component.group.isEmpty() )
    {
      if ( groupLayerMap.contains( component.group ) )
      {
        groupLayerMap[ component.group ].appendChild( layer );
      }
      else
      {
        QDomElement group = doc.createElement( QStringLiteral( "Layer" ) );
        group.setAttribute( QStringLiteral( "id" ), QStringLiteral( "group_%1" ).arg( component.group ) );
        group.setAttribute( QStringLiteral( "name" ), component.group );
        group.setAttribute( QStringLiteral( "initiallyVisible" ), groupLayerMap.empty() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
        group.setAttribute( QStringLiteral( "mutuallyExclusiveGroupId" ), QStringLiteral( "__mutually_exclusive_groups__" ) );
        layerTree.appendChild( group );
        group.appendChild( layer );
        groupLayerMap[ component.group ] = group;
      }
    }
    else
    {
      layerTree.appendChild( layer );
    }

    createdLayerIds[ component.group ].insert( component.mapLayerId );
  }

  // create custom layer tree entries
  for ( auto it = details.customLayerTreeGroups.constBegin(); it != details.customLayerTreeGroups.constEnd(); ++it )
  {
    if ( customGroupNamesToIds.contains( it.value() ) )
      continue;

    QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
    const QString id = QUuid::createUuid().toString();
    customGroupNamesToIds[ it.value() ] = id;
    layer.setAttribute( QStringLiteral( "id" ), id );
    layer.setAttribute( QStringLiteral( "name" ), it.value() );
    layer.setAttribute( QStringLiteral( "initiallyVisible" ), QStringLiteral( "true" ) );
    layerTree.appendChild( layer );
  }

  compositionElem.appendChild( layerTree );

  // pages
  QDomElement page = doc.createElement( QStringLiteral( "Page" ) );
  QDomElement dpi = doc.createElement( QStringLiteral( "DPI" ) );
  dpi.appendChild( doc.createTextNode( QString::number( details.dpi ) ) );
  page.appendChild( dpi );
  // assumes DPI of 72, which is an assumption on GDALs/PDF side. It's only related to the PDF coordinate space and doesn't affect the actual output DPI!
  QDomElement width = doc.createElement( QStringLiteral( "Width" ) );
  const double pageWidthPdfUnits = std::ceil( details.pageSizeMm.width() / 25.4 * 72 );
  width.appendChild( doc.createTextNode( QString::number( pageWidthPdfUnits ) ) );
  page.appendChild( width );
  QDomElement height = doc.createElement( QStringLiteral( "Height" ) );
  const double pageHeightPdfUnits = std::ceil( details.pageSizeMm.height() / 25.4 * 72 );
  height.appendChild( doc.createTextNode( QString::number( pageHeightPdfUnits ) ) );
  page.appendChild( height );


  // georeferencing
  int i = 0;
  for ( const QgsAbstractGeoPdfExporter::GeoReferencedSection &section : details.georeferencedSections )
  {
    QDomElement georeferencing = doc.createElement( QStringLiteral( "Georeferencing" ) );
    georeferencing.setAttribute( QStringLiteral( "id" ), QStringLiteral( "georeferenced_%1" ).arg( i++ ) );
    georeferencing.setAttribute( QStringLiteral( "OGCBestPracticeFormat" ), details.useOgcBestPracticeFormatGeoreferencing ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    georeferencing.setAttribute( QStringLiteral( "ISO32000ExtensionFormat" ), details.useIso32000ExtensionFormatGeoreferencing ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );

    if ( section.crs.isValid() )
    {
      QDomElement srs = doc.createElement( QStringLiteral( "SRS" ) );
      // not currently used by GDAL or the PDF spec, but exposed in the GDAL XML schema. Maybe something we'll need to consider down the track...
      // srs.setAttribute( QStringLiteral( "dataAxisToSRSAxisMapping" ), QStringLiteral( "2,1" ) );
      if ( !section.crs.authid().startsWith( QStringLiteral( "user" ), Qt::CaseInsensitive ) )
      {
        srs.appendChild( doc.createTextNode( section.crs.authid() ) );
      }
      else
      {
        srs.appendChild( doc.createTextNode( section.crs.toWkt( QgsCoordinateReferenceSystem::WKT2_2018 ) ) );
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
      QDomElement boundingPolygon = doc.createElement( QStringLiteral( "BoundingPolygon" ) );

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
      QDomElement boundingBox = doc.createElement( QStringLiteral( "BoundingBox" ) );
      boundingBox.setAttribute( QStringLiteral( "x1" ), QString::number( section.pageBoundsMm.xMinimum() / 25.4 * 72 ) );
      boundingBox.setAttribute( QStringLiteral( "y1" ), QString::number( section.pageBoundsMm.yMinimum() / 25.4 * 72 ) );
      boundingBox.setAttribute( QStringLiteral( "x2" ), QString::number( section.pageBoundsMm.xMaximum() / 25.4 * 72 ) );
      boundingBox.setAttribute( QStringLiteral( "y2" ), QString::number( section.pageBoundsMm.yMaximum() / 25.4 * 72 ) );
      georeferencing.appendChild( boundingBox );
    }

    for ( const ControlPoint &point : section.controlPoints )
    {
      QDomElement cp1 = doc.createElement( QStringLiteral( "ControlPoint" ) );
      cp1.setAttribute( QStringLiteral( "x" ), QString::number( point.pagePoint.x() / 25.4 * 72 ) );
      cp1.setAttribute( QStringLiteral( "y" ), QString::number( ( details.pageSizeMm.height() - point.pagePoint.y() ) / 25.4 * 72 ) );
      cp1.setAttribute( QStringLiteral( "GeoX" ), QString::number( point.geoPoint.x() ) );
      cp1.setAttribute( QStringLiteral( "GeoY" ), QString::number( point.geoPoint.y() ) );
      georeferencing.appendChild( cp1 );
    }

    page.appendChild( georeferencing );
  }

  // content
  QDomElement content = doc.createElement( QStringLiteral( "Content" ) );
  for ( const ComponentLayerDetail &component : components )
  {
    if ( component.mapLayerId.isEmpty() )
    {
      QDomElement pdfDataset = doc.createElement( QStringLiteral( "PDF" ) );
      pdfDataset.setAttribute( QStringLiteral( "dataset" ), component.sourcePdfPath );
      content.appendChild( pdfDataset );
    }
    else if ( !component.group.isEmpty() )
    {
      // if content belongs to a group, we need nested "IfLayerOn" elements, one for the group and one for the layer
      QDomElement ifGroupOn = doc.createElement( QStringLiteral( "IfLayerOn" ) );
      ifGroupOn.setAttribute( QStringLiteral( "layerId" ), QStringLiteral( "group_%1" ).arg( component.group ) );
      QDomElement ifLayerOn = doc.createElement( QStringLiteral( "IfLayerOn" ) );
      if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), customGroupNamesToIds.value( details.customLayerTreeGroups.value( component.mapLayerId ) ) );
      else if ( component.group.isEmpty() )
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), component.mapLayerId );
      else
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
      QDomElement pdfDataset = doc.createElement( QStringLiteral( "PDF" ) );
      pdfDataset.setAttribute( QStringLiteral( "dataset" ), component.sourcePdfPath );
      ifLayerOn.appendChild( pdfDataset );
      ifGroupOn.appendChild( ifLayerOn );
      content.appendChild( ifGroupOn );
    }
    else
    {
      QDomElement ifLayerOn = doc.createElement( QStringLiteral( "IfLayerOn" ) );
      if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), customGroupNamesToIds.value( details.customLayerTreeGroups.value( component.mapLayerId ) ) );
      else if ( component.group.isEmpty() )
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), component.mapLayerId );
      else
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
      QDomElement pdfDataset = doc.createElement( QStringLiteral( "PDF" ) );
      pdfDataset.setAttribute( QStringLiteral( "dataset" ), component.sourcePdfPath );
      ifLayerOn.appendChild( pdfDataset );
      content.appendChild( ifLayerOn );
    }
  }

  // vector datasets (we "draw" these on top, just for debugging... but they are invisible, so are never really drawn!)
  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : qgis::as_const( mVectorComponents ) )
    {
      QDomElement ifLayerOn = doc.createElement( QStringLiteral( "IfLayerOn" ) );
      if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), customGroupNamesToIds.value( details.customLayerTreeGroups.value( component.mapLayerId ) ) );
      else if ( component.group.isEmpty() )
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), component.mapLayerId );
      else
        ifLayerOn.setAttribute( QStringLiteral( "layerId" ), QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
      QDomElement vectorDataset = doc.createElement( QStringLiteral( "Vector" ) );
      vectorDataset.setAttribute( QStringLiteral( "dataset" ), component.sourceVectorPath );
      vectorDataset.setAttribute( QStringLiteral( "layer" ), component.sourceVectorLayer );
      vectorDataset.setAttribute( QStringLiteral( "visible" ), QStringLiteral( "false" ) );
      QDomElement logicalStructure = doc.createElement( QStringLiteral( "LogicalStructure" ) );
      logicalStructure.setAttribute( QStringLiteral( "displayLayerName" ), component.name );
      if ( !component.displayAttribute.isEmpty() )
        logicalStructure.setAttribute( QStringLiteral( "fieldToDisplay" ), component.displayAttribute );
      vectorDataset.appendChild( logicalStructure );
      ifLayerOn.appendChild( vectorDataset );
      content.appendChild( ifLayerOn );
    }
  }

  page.appendChild( content );
  compositionElem.appendChild( page );

  doc.appendChild( compositionElem );

  QString composition;
  QTextStream stream( &composition );
  doc.save( stream, -1 );

  return composition;
}

