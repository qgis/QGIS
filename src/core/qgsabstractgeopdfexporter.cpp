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
#include "qgscoordinatetransformcontext.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgsvectorfilewriter.h"
#include "qgsfileutils.h"

#include <gdal.h>
#include "cpl_string.h"

#include <QMutex>
#include <QMutexLocker>
#include <QDomDocument>
#include <QDomElement>
#include <QTimeZone>
#include <QUuid>
#include <QTextStream>

bool QgsAbstractGeoPdfExporter::geoPDFCreationAvailable()
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

QString QgsAbstractGeoPdfExporter::geoPDFAvailabilityExplanation()
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

  return QObject::tr( "GDAL PDF driver was not built with PDF read support. A build with PDF read support is required for GeoPDF creation." );
}

void CPL_STDCALL collectErrors( CPLErr, int, const char *msg )
{
  QgsDebugError( QStringLiteral( "GDAL PDF creation error: %1 " ).arg( msg ) );
  if ( QStringList *errorList = static_cast< QStringList * >( CPLGetErrorHandlerUserData() ) )
  {
    errorList->append( QString( msg ) );
  }
}

bool QgsAbstractGeoPdfExporter::finalize( const QList<ComponentLayerDetail> &components, const QString &destinationFile, const ExportDetails &details )
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

  const QString xmlFilePath = generateTemporaryFilepath( QStringLiteral( "composition.xml" ) );
  QFile file( xmlFilePath );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QTextStream out( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec( "UTF-8" );
#endif
    out << composition;
  }
  else
  {
    mErrorMessage = QObject::tr( "Could not create GeoPDF composition file" );
    return false;
  }

  char **papszOptions = CSLSetNameValue( nullptr, "COMPOSITION_FILE", xmlFilePath.toUtf8().constData() );

  QStringList creationErrors;
  CPLPushErrorHandlerEx( collectErrors, &creationErrors );

  // return a non-null (fake) dataset in case of success, nullptr otherwise.
  gdal::dataset_unique_ptr outputDataset( GDALCreate( driver, destinationFile.toUtf8().constData(), 0, 0, 0, GDT_Unknown, papszOptions ) );

  CPLPopErrorHandler();
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
        mErrorMessage += ( !mErrorMessage.isEmpty() ? QStringLiteral( "\n" ) : QString() ) + error;
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

QString QgsAbstractGeoPdfExporter::generateTemporaryFilepath( const QString &filename ) const
{
  return mTemporaryDir.filePath( QgsFileUtils::stringToSafeFilename( filename ) );
}

bool QgsAbstractGeoPdfExporter::compositionModeSupported( QPainter::CompositionMode mode )
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
      QgsVectorFileWriter::SaveVectorOptions saveOptions;
      saveOptions.driverName = QStringLiteral( "GPKG" );
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

  QMap< QString, QSet< QString > > createdLayerIds;
  QMap< QString, QDomElement > groupLayerMap;

  QMultiMap< QString, QDomElement > pendingLayerTreeElements;

  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : std::as_const( mVectorComponents ) )
    {
      if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
        continue;

      QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
      layer.setAttribute( QStringLiteral( "id" ), component.group.isEmpty() ? component.mapLayerId : QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
      layer.setAttribute( QStringLiteral( "name" ), details.layerIdToPdfLayerTreeNameMap.contains( component.mapLayerId ) ? details.layerIdToPdfLayerTreeNameMap.value( component.mapLayerId ) : component.name );
      layer.setAttribute( QStringLiteral( "initiallyVisible" ), details.initialLayerVisibility.value( component.mapLayerId, true ) ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );

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
          pendingLayerTreeElements.insert( component.mapLayerId, group );
          group.appendChild( layer );
          groupLayerMap[ component.group ] = group;
        }
      }
      else
      {
        pendingLayerTreeElements.insert( component.mapLayerId, layer );
      }

      createdLayerIds[ component.group ].insert( component.mapLayerId );
    }
  }
  // some PDF components may not be linked to vector components - e.g. layers with labels but no features (or raster layers)
  for ( const ComponentLayerDetail &component : components )
  {
    if ( component.mapLayerId.isEmpty() || createdLayerIds.value( component.group ).contains( component.mapLayerId ) )
      continue;

    if ( details.customLayerTreeGroups.contains( component.mapLayerId ) )
      continue;

    QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
    layer.setAttribute( QStringLiteral( "id" ), component.group.isEmpty() ? component.mapLayerId : QStringLiteral( "%1_%2" ).arg( component.group, component.mapLayerId ) );
    layer.setAttribute( QStringLiteral( "name" ), details.layerIdToPdfLayerTreeNameMap.contains( component.mapLayerId ) ? details.layerIdToPdfLayerTreeNameMap.value( component.mapLayerId ) : component.name );
    layer.setAttribute( QStringLiteral( "initiallyVisible" ), details.initialLayerVisibility.value( component.mapLayerId, true ) ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );

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
        pendingLayerTreeElements.insert( component.mapLayerId, group );
        group.appendChild( layer );
        groupLayerMap[ component.group ] = group;
      }
    }
    else
    {
      pendingLayerTreeElements.insert( component.mapLayerId, layer );
    }

    createdLayerIds[ component.group ].insert( component.mapLayerId );
  }

  // layertree
  QDomElement layerTree = doc.createElement( QStringLiteral( "LayerTree" ) );
  //layerTree.setAttribute( QStringLiteral("displayOnlyOnVisiblePages"), QStringLiteral("true"));

  // create custom layer tree entries
  QStringList layerTreeGroupOrder = details.layerTreeGroupOrder;

  // add any missing groups to end of order
  for ( auto it = details.customLayerTreeGroups.constBegin(); it != details.customLayerTreeGroups.constEnd(); ++it )
  {
    if ( layerTreeGroupOrder.contains( it.value() ) )
      continue;
    layerTreeGroupOrder.append( it.value() );
  }
  // filter out groups which don't have any content
  layerTreeGroupOrder.erase( std::remove_if( layerTreeGroupOrder.begin(), layerTreeGroupOrder.end(), [&details]( const QString & group )
  {
    return details.customLayerTreeGroups.key( group ).isEmpty();
  } ), layerTreeGroupOrder.end() );

  QMap< QString, QString > customGroupNamesToIds;
  for ( const QString &group : std::as_const( layerTreeGroupOrder ) )
  {
    if ( customGroupNamesToIds.contains( group ) )
      continue;

    QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
    const QString id = QUuid::createUuid().toString();
    customGroupNamesToIds[ group ] = id;
    layer.setAttribute( QStringLiteral( "id" ), id );
    layer.setAttribute( QStringLiteral( "name" ), group );
    layer.setAttribute( QStringLiteral( "initiallyVisible" ), QStringLiteral( "true" ) );
    layerTree.appendChild( layer );
  }

  // start by adding layer tree elements with known layer orders
  for ( const QString &layerId : details.layerOrder )
  {
    const QList< QDomElement> elements = pendingLayerTreeElements.values( layerId );
    for ( const QDomElement &element : elements )
      layerTree.appendChild( element );
  }
  // then add all the rest (those we don't have an explicit order for)
  for ( auto it = pendingLayerTreeElements.constBegin(); it != pendingLayerTreeElements.constEnd(); ++it )
  {
    if ( details.layerOrder.contains( it.key() ) )
    {
      // already added this one, just above...
      continue;
    }

    layerTree.appendChild( it.value() );
  }

  compositionElem.appendChild( layerTree );

  // pages
  QDomElement page = doc.createElement( QStringLiteral( "Page" ) );
  QDomElement dpi = doc.createElement( QStringLiteral( "DPI" ) );
  // hardcode DPI of 72 to get correct page sizes in outputs -- refs discussion in https://github.com/OSGeo/gdal/pull/2961
  dpi.appendChild( doc.createTextNode( qgsDoubleToString( 72 ) ) );
  page.appendChild( dpi );
  // assumes DPI of 72, as noted above.
  QDomElement width = doc.createElement( QStringLiteral( "Width" ) );
  const double pageWidthPdfUnits = std::ceil( details.pageSizeMm.width() / 25.4 * 72 );
  width.appendChild( doc.createTextNode( qgsDoubleToString( pageWidthPdfUnits ) ) );
  page.appendChild( width );
  QDomElement height = doc.createElement( QStringLiteral( "Height" ) );
  const double pageHeightPdfUnits = std::ceil( details.pageSizeMm.height() / 25.4 * 72 );
  height.appendChild( doc.createTextNode( qgsDoubleToString( pageHeightPdfUnits ) ) );
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
      if ( !section.crs.authid().isEmpty() && !section.crs.authid().startsWith( QStringLiteral( "user" ), Qt::CaseInsensitive ) )
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
      boundingBox.setAttribute( QStringLiteral( "x1" ), qgsDoubleToString( section.pageBoundsMm.xMinimum() / 25.4 * 72 ) );
      boundingBox.setAttribute( QStringLiteral( "y1" ), qgsDoubleToString( section.pageBoundsMm.yMinimum() / 25.4 * 72 ) );
      boundingBox.setAttribute( QStringLiteral( "x2" ), qgsDoubleToString( section.pageBoundsMm.xMaximum() / 25.4 * 72 ) );
      boundingBox.setAttribute( QStringLiteral( "y2" ), qgsDoubleToString( section.pageBoundsMm.yMaximum() / 25.4 * 72 ) );
      georeferencing.appendChild( boundingBox );
    }

    for ( const ControlPoint &point : section.controlPoints )
    {
      QDomElement cp1 = doc.createElement( QStringLiteral( "ControlPoint" ) );
      cp1.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( point.pagePoint.x() / 25.4 * 72 ) );
      cp1.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( ( details.pageSizeMm.height() - point.pagePoint.y() ) / 25.4 * 72 ) );
      cp1.setAttribute( QStringLiteral( "GeoX" ), qgsDoubleToString( point.geoPoint.x() ) );
      cp1.setAttribute( QStringLiteral( "GeoY" ), qgsDoubleToString( point.geoPoint.y() ) );
      georeferencing.appendChild( cp1 );
    }

    page.appendChild( georeferencing );
  }

  auto createPdfDatasetElement = [&doc]( const ComponentLayerDetail & component ) -> QDomElement
  {
    QDomElement pdfDataset = doc.createElement( QStringLiteral( "PDF" ) );
    pdfDataset.setAttribute( QStringLiteral( "dataset" ), component.sourcePdfPath );
    if ( component.opacity != 1.0 || component.compositionMode != QPainter::CompositionMode_SourceOver )
    {
      QDomElement blendingElement = doc.createElement( QStringLiteral( "Blending" ) );
      blendingElement.setAttribute( QStringLiteral( "opacity" ), component.opacity );
      blendingElement.setAttribute( QStringLiteral( "function" ), compositionModeToString( component.compositionMode ) );

      pdfDataset.appendChild( blendingElement );
    }
    return pdfDataset;
  };

  // content
  QDomElement content = doc.createElement( QStringLiteral( "Content" ) );
  for ( const ComponentLayerDetail &component : components )
  {
    if ( component.mapLayerId.isEmpty() )
    {
      content.appendChild( createPdfDatasetElement( component ) );
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

      ifLayerOn.appendChild( createPdfDatasetElement( component ) );
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
      ifLayerOn.appendChild( createPdfDatasetElement( component ) );
      content.appendChild( ifLayerOn );
    }
  }

  // vector datasets (we "draw" these on top, just for debugging... but they are invisible, so are never really drawn!)
  if ( details.includeFeatures )
  {
    for ( const VectorComponentDetail &component : std::as_const( mVectorComponents ) )
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

QString QgsAbstractGeoPdfExporter::compositionModeToString( QPainter::CompositionMode mode )
{
  switch ( mode )
  {
    case QPainter::CompositionMode_SourceOver:
      return QStringLiteral( "Normal" );

    case QPainter::CompositionMode_Multiply:
      return QStringLiteral( "Multiply" );

    case QPainter::CompositionMode_Screen:
      return QStringLiteral( "Screen" );

    case QPainter::CompositionMode_Overlay:
      return QStringLiteral( "Overlay" );

    case QPainter::CompositionMode_Darken:
      return QStringLiteral( "Darken" );

    case QPainter::CompositionMode_Lighten:
      return QStringLiteral( "Lighten" );

    case QPainter::CompositionMode_ColorDodge:
      return QStringLiteral( "ColorDodge" );

    case QPainter::CompositionMode_ColorBurn:
      return QStringLiteral( "ColorBurn" );

    case QPainter::CompositionMode_HardLight:
      return QStringLiteral( "HardLight" );

    case QPainter::CompositionMode_SoftLight:
      return QStringLiteral( "SoftLight" );

    case QPainter::CompositionMode_Difference:
      return QStringLiteral( "Difference" );

    case  QPainter::CompositionMode_Exclusion:
      return QStringLiteral( "Exclusion" );

    default:
      break;
  }

  QgsDebugError( QStringLiteral( "Unsupported PDF blend mode %1" ).arg( mode ) );
  return QStringLiteral( "Normal" );
}

