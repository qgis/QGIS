/***************************************************************************
                              qgslayoutgeopdfexporter.cpp
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

#include "qgslayoutgeopdfexporter.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsfeaturerequest.h"
#include "qgslayout.h"
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


class QgsGeoPdfRenderedFeatureHandler: public QgsRenderedFeatureHandlerInterface
{
  public:

    QgsGeoPdfRenderedFeatureHandler( QgsLayoutItemMap *map )
    {
      QTransform mapTransform;
      QPolygonF mapRectPoly = QPolygonF( QRectF( 0, 0, map->rect().width(), map->rect().height() ) );
      //workaround QT Bug #21329
      mapRectPoly.pop_back();

      QPolygonF mapRectInLayout = map->mapToScene( mapRectPoly );

      //create transform from layout coordinates to map coordinates
      QTransform::quadToQuad( mapRectPoly, mapRectInLayout, mMapToLayoutTransform );

      mLayoutToPdfTransform = QTransform::fromTranslate( 0, 595 ).scale( 842.0 / 297, -595.0 / 210 );
    }

    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext &context ) override
    {
      // is it a hack retrieving the layer ID from an expression context like this? possibly... BUT
      // the alternative is adding a layer ID member to QgsRenderContext, and that's just asking for people to abuse it
      // and use it to retrieve QgsMapLayers mid-way through a render operation. Lesser of two evils it is!
      const QString layerId = context.renderContext->expressionContext().variable( QStringLiteral( "layer_id" ) ).toString();

      // transform from pixels to map item coordinates
      QTransform pixelToMapItemTransform = QTransform::fromScale( 1.0 / context.renderContext->scaleFactor(), 1.0 / context.renderContext->scaleFactor() );
      QgsGeometry transformed = renderedBounds;
      transformed.transform( pixelToMapItemTransform );
      // transform from map item coordinates to page coordinates
      transformed.transform( mMapToLayoutTransform );
      // ...and then to PDF coordinate space
      transformed.transform( mLayoutToPdfTransform );

      // always convert to multitype, to make things consistent
      transformed.convertToMultiType();

      // we (currently) don't REALLY need a mutex here, because layout maps are always rendered using a single threaded operation.
      // but we'll play it safe, just in case this changes in future.
      QMutexLocker locker( &mMutex );
      renderedFeatures[ layerId ].append( QgsLayoutGeoPdfExporter::RenderedFeature( feature, transformed ) );
    }

    QSet<QString> usedAttributes( QgsVectorLayer *, const QgsRenderContext & ) const override
    {
      return QSet< QString >() << QgsFeatureRequest::ALL_ATTRIBUTES;
    }

    QMap< QString, QVector< QgsLayoutGeoPdfExporter::RenderedFeature > > renderedFeatures;

  private:
    QTransform mMapToLayoutTransform;
    QTransform mLayoutToPdfTransform;
    QMutex mMutex;
};


QgsLayoutGeoPdfExporter::QgsLayoutGeoPdfExporter( QgsLayout *layout )
  : mLayout( layout )
{
  // on construction, we install a rendered feature handler on layout item maps
  QList< QgsLayoutItemMap * > maps;
  mLayout->layoutItems( maps );
  for ( QgsLayoutItemMap *map : qgis::as_const( maps ) )
  {
    QgsGeoPdfRenderedFeatureHandler *handler = new QgsGeoPdfRenderedFeatureHandler( map );
    mMapHandlers.insert( map, handler );
    map->addRenderedFeatureHandler( handler );
  }
}

QgsLayoutGeoPdfExporter::~QgsLayoutGeoPdfExporter()
{
  // cleanup - remove rendered feature handler from all maps
  for ( auto it = mMapHandlers.constBegin(); it != mMapHandlers.constEnd(); ++it )
  {
    it.key()->removeRenderedFeatureHandler( it.value() );
    delete it.value();
  }
}

QMap<QString, QVector<QgsLayoutGeoPdfExporter::RenderedFeature> > QgsLayoutGeoPdfExporter::renderedFeatures( QgsLayoutItemMap *map ) const
{
  return mMapHandlers.value( map )->renderedFeatures;
}

bool QgsLayoutGeoPdfExporter::finalize( const QList<ComponentLayerDetail> &components )
{
  // collate all the features from different maps which belong to the same layer, replace their geometries with the rendered feature bounds
  for ( auto mapIt = mMapHandlers.constBegin(); mapIt != mMapHandlers.constEnd(); ++mapIt )
  {
    QgsGeoPdfRenderedFeatureHandler *handler = mapIt.value();
    for ( auto layerIt = handler->renderedFeatures.constBegin(); layerIt != handler->renderedFeatures.constEnd(); ++layerIt )
    {
      const QString layerId = layerIt.key();
      const QVector< QgsLayoutGeoPdfExporter::RenderedFeature > features = layerIt.value();
      for ( auto featureIt = features.constBegin(); featureIt != features.constEnd(); ++featureIt )
      {
        // replace feature geometry with transformed rendered bounds
        QgsFeature f = featureIt->feature;
        f.setGeometry( featureIt->renderedBounds );
        mCollatedFeatures[ layerId ].append( f );
      }
    }
  }

  if ( !saveTemporaryLayers() )
    return false;

  const QString composition = createCompositionXml( components );
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

  QString outputFile = "/home/nyall/Temporary/geopdf/test.pdf";

  // return a non-null (fake) dataset in case of success, nullptr otherwise.
  gdal::dataset_unique_ptr outputDataset( GDALCreate( driver, outputFile.toUtf8().constData(), 0, 0, 0, GDT_Unknown, papszOptions ) );
  bool res = outputDataset.get();
  outputDataset.reset();

  CSLDestroy( papszOptions );

  return res;
}

QString QgsLayoutGeoPdfExporter::generateTemporaryFilepath( const QString &filename ) const
{
  return mTemporaryDir.filePath( filename );
}

bool QgsLayoutGeoPdfExporter::saveTemporaryLayers()
{
  QgsProject *project = mLayout->project();
  for ( auto it = mCollatedFeatures.constBegin(); it != mCollatedFeatures.constEnd(); ++it )
  {
    const QString filePath = generateTemporaryFilepath( it.key() + QStringLiteral( ".gpkg" ) );

    VectorComponentDetail detail;

    const QgsMapLayer *layer = project->mapLayer( it.key() );
    detail.name = layer ? layer->name() : it.key();
    detail.mapLayerId = it.key();
    detail.sourceVectorPath = filePath;
    if ( const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer ) )
    {
      detail.displayAttribute = vl->displayField();
    }

    // write out features to disk
    const QgsFeatureList features = it.value();
    QString layerName;
    QgsVectorFileWriter writer( filePath, QString(), features.first().fields(), features.first().geometry().wkbType(), QgsCoordinateReferenceSystem(), QStringLiteral( "GPKG" ), QStringList(), QStringList(), nullptr, QgsVectorFileWriter::NoSymbology, nullptr, &layerName );
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
  return true;
}

QString QgsLayoutGeoPdfExporter::createCompositionXml( const QList<ComponentLayerDetail> &components )
{
  QDomDocument doc;

  QDomElement compositionElem = doc.createElement( QStringLiteral( "PDFComposition" ) );

  // metadata tags
  QDomElement metadata = doc.createElement( QStringLiteral( "Metadata" ) );
  QDomElement author = doc.createElement( QStringLiteral( "Author" ) );
  author.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( author );
  QDomElement producer = doc.createElement( QStringLiteral( "Producer" ) );
  producer.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( producer );
  QDomElement creator = doc.createElement( QStringLiteral( "Creator" ) );
  creator.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( creator );
  QDomElement creationDate = doc.createElement( QStringLiteral( "CreationDate" ) );
  //creationDate.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( creationDate );
  QDomElement subject = doc.createElement( QStringLiteral( "Subject" ) );
  //subject.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( subject );
  QDomElement title = doc.createElement( QStringLiteral( "Title" ) );
  //title.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( title );
  QDomElement keywords = doc.createElement( QStringLiteral( "Keywords" ) );
  //keywords.appendChild( doc.createTextNode( QStringLiteral( "QGIS" ) ) );
  metadata.appendChild( keywords );
  compositionElem.appendChild( metadata );

  // layertree
  QDomElement layerTree = doc.createElement( QStringLiteral( "LayerTree" ) );
  //layerTree.setAttribute( QStringLiteral("displayOnlyOnVisiblePages"), QStringLiteral("true"));
  for ( const VectorComponentDetail &component : qgis::as_const( mVectorComponents ) )
  {
    QDomElement layer = doc.createElement( QStringLiteral( "Layer" ) );
    layer.setAttribute( QStringLiteral( "id" ), component.mapLayerId );
    layer.setAttribute( QStringLiteral( "name" ), component.name );
    layer.setAttribute( QStringLiteral( "initiallyVisible" ), QStringLiteral( "true" ) );
    layerTree.appendChild( layer );
  }
  compositionElem.appendChild( layerTree );

  // pages
  QDomElement page = doc.createElement( QStringLiteral( "Page" ) );
  QDomElement dpi = doc.createElement( QStringLiteral( "DPI" ) );
  dpi.appendChild( doc.createTextNode( QStringLiteral( "300" ) ) );
  page.appendChild( dpi );
  QDomElement width = doc.createElement( QStringLiteral( "Width" ) );
  width.appendChild( doc.createTextNode( QStringLiteral( "842" ) ) );
  page.appendChild( width );
  QDomElement height = doc.createElement( QStringLiteral( "Height" ) );
  height.appendChild( doc.createTextNode( QStringLiteral( "595" ) ) );
  page.appendChild( height );


  // georeferencing - TODO
  QDomElement georeferencing = doc.createElement( QStringLiteral( "Georeferencing" ) );
  georeferencing.setAttribute( QStringLiteral( "id" ), QStringLiteral( "georeferenced" ) );
  georeferencing.setAttribute( QStringLiteral( "OGCBestPracticeFormat" ), QStringLiteral( "true" ) );
  georeferencing.setAttribute( QStringLiteral( "ISO32000ExtensionFormat" ), QStringLiteral( "false" ) );
  QDomElement srs = doc.createElement( QStringLiteral( "SRS" ) );
  srs.setAttribute( QStringLiteral( "dataAxisToSRSAxisMapping" ), QStringLiteral( "2,1" ) );
  srs.appendChild( doc.createTextNode( QStringLiteral( "EPSG:4326" ) ) );
  georeferencing.appendChild( srs );
#if 0
  /* Define the viewport where georeferenced coordinates are available.
    If not specified, the extent of BoundingPolygon will be used instead.
    If none of BoundingBox and BoundingPolygon are specified,
    the whole PDF page will be assumed to be georeferenced.
    */
  QDomElement boundingBox = doc.createElement( QStringLiteral( "BoundingBox" ) );
  boundingBox.setAttribute( QStringLiteral( "x1" ), QStringLiteral( "1" ) );
  boundingBox.setAttribute( QStringLiteral( "y1" ), QStringLiteral( "1" ) );
  boundingBox.setAttribute( QStringLiteral( "x2" ), QStringLiteral( "9" ) );
  boundingBox.setAttribute( QStringLiteral( "y2" ), QStringLiteral( "14" ) );
  georeferencing.appendChild( boundingBox );
  /*
    Define a polygon / neatline in PDF units into which the
    Measure tool will display coordinates.
    If not specified, BoundingBox will be used instead.
    If none of BoundingBox and BoundingPolygon are specified,
    the whole PDF page will be assumed to be georeferenced.
   */
  QDomElement boundingPolygon = doc.createElement( QStringLiteral( "BoundingPolygon" ) );
  boundingPolygon.appendChild( doc.createTextNode( QStringLiteral( "POLYGON((1 1,9 1,9 14,1 14,1 1))" ) ) );
  georeferencing.appendChild( boundingPolygon );
#endif
  QDomElement cp1 = doc.createElement( QStringLiteral( "ControlPoint" ) );
  cp1.setAttribute( QStringLiteral( "x" ), QStringLiteral( "1" ) );
  cp1.setAttribute( QStringLiteral( "y" ), QStringLiteral( "1" ) );
  cp1.setAttribute( QStringLiteral( "GeoX" ), QStringLiteral( "2" ) );
  cp1.setAttribute( QStringLiteral( "GeoY" ), QStringLiteral( "48" ) );
  georeferencing.appendChild( cp1 );
  QDomElement cp2 = doc.createElement( QStringLiteral( "ControlPoint" ) );
  cp2.setAttribute( QStringLiteral( "x" ), QStringLiteral( "1" ) );
  cp2.setAttribute( QStringLiteral( "y" ), QStringLiteral( "14" ) );
  cp2.setAttribute( QStringLiteral( "GeoX" ), QStringLiteral( "2" ) );
  cp2.setAttribute( QStringLiteral( "GeoY" ), QStringLiteral( "49" ) );
  georeferencing.appendChild( cp2 );
  QDomElement cp3 = doc.createElement( QStringLiteral( "ControlPoint" ) );
  cp3.setAttribute( QStringLiteral( "x" ), QStringLiteral( "9" ) );
  cp3.setAttribute( QStringLiteral( "y" ), QStringLiteral( "1" ) );
  cp3.setAttribute( QStringLiteral( "GeoX" ), QStringLiteral( "3" ) );
  cp3.setAttribute( QStringLiteral( "GeoY" ), QStringLiteral( "48" ) );
  georeferencing.appendChild( cp3 );
  QDomElement cp4 = doc.createElement( QStringLiteral( "ControlPoint" ) );
  cp4.setAttribute( QStringLiteral( "x" ), QStringLiteral( "9" ) );
  cp4.setAttribute( QStringLiteral( "y" ), QStringLiteral( "14" ) );
  cp4.setAttribute( QStringLiteral( "GeoX" ), QStringLiteral( "3" ) );
  cp4.setAttribute( QStringLiteral( "GeoY" ), QStringLiteral( "49" ) );
  georeferencing.appendChild( cp4 );

  page.appendChild( georeferencing );


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
    else
    {
      QDomElement ifLayerOn = doc.createElement( QStringLiteral( "IfLayerOn" ) );
      ifLayerOn.setAttribute( QStringLiteral( "layerId" ), component.mapLayerId );
      QDomElement pdfDataset = doc.createElement( QStringLiteral( "PDF" ) );
      pdfDataset.setAttribute( QStringLiteral( "dataset" ), component.sourcePdfPath );
      ifLayerOn.appendChild( pdfDataset );
      content.appendChild( ifLayerOn );
    }
  }

  // vector datasets (we draw these on top, just for debugging
  for ( const VectorComponentDetail &component : qgis::as_const( mVectorComponents ) )
  {
    QDomElement ifLayerOn = doc.createElement( QStringLiteral( "IfLayerOn" ) );
    ifLayerOn.setAttribute( QStringLiteral( "layerId" ), component.mapLayerId );
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

  page.appendChild( content );
  compositionElem.appendChild( page );

  doc.appendChild( compositionElem );

  QString composition;
  QTextStream stream( &composition );
  doc.save( stream, -1 );

  return composition;
}

