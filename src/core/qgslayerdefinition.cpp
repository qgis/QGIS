/***************************************************************************
    qgslayerdefinition.cpp
    ---------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayerdefinition.h"

#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgsgrouplayer.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsmeshlayer.h"
#include "qgspathresolver.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgspointcloudlayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgstiledscenelayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

bool QgsLayerDefinition::loadLayerDefinition( const QString &path, QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage, Qgis::LayerTreeInsertionMethod insertMethod, const QgsLayerTreeRegistryBridge::InsertionPoint *insertPoint )
{
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    errorMessage = u"Can not open file"_s;
    return false;
  }

  QDomDocument doc;
  QString message;
  if ( !doc.setContent( &file, &message ) )
  {
    errorMessage = message;
    return false;
  }

  const QFileInfo fileinfo( file );
  QDir::setCurrent( fileinfo.absoluteDir().path() );

  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( path ) );
  context.setProjectTranslator( project );

  return loadLayerDefinition( doc, project, rootGroup, errorMessage, context, insertMethod, insertPoint );
}

bool QgsLayerDefinition::loadLayerDefinition( QDomDocument doc, QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage, QgsReadWriteContext &context, Qgis::LayerTreeInsertionMethod insertMethod, const QgsLayerTreeRegistryBridge::InsertionPoint *insertPoint )
{
  errorMessage.clear();

  QgsLayerTreeGroup root;

  // reorder maplayer nodes based on dependencies
  // dependencies have to be resolved before IDs get changed
  const DependencySorter depSorter( doc );
  if ( !depSorter.hasMissingDependency() )
  {
    const QVector<QDomNode> sortedLayerNodes = depSorter.sortedLayerNodes();
    QVector<QDomNode> clonedSorted;
    const auto constSortedLayerNodes = sortedLayerNodes;
    for ( const QDomNode &node : constSortedLayerNodes )
    {
      clonedSorted << node.cloneNode();
    }
    QDomNode layersNode = doc.elementsByTagName( u"maplayers"_s ).at( 0 );
    // replace old children with new ones
    QDomNode childNode = layersNode.firstChild();
    for ( int i = 0; ! childNode.isNull(); i++ )
    {
      layersNode.replaceChild( clonedSorted.at( i ), childNode );
      childNode = childNode.nextSibling();
    }
  }
  // if a dependency is missing, we still try to load layers, since dependencies may already be loaded

  // IDs of layers should be changed otherwise we may have more then one layer with the same id
  // We have to replace the IDs before we load them because it's too late once they are loaded
  const QDomNodeList treeLayerNodes = doc.elementsByTagName( u"layer-tree-layer"_s );
  for ( int i = 0; i < treeLayerNodes.length(); ++i )
  {
    const QDomNode treeLayerNode = treeLayerNodes.item( i );
    QDomElement treeLayerElem = treeLayerNode.toElement();
    const QString oldid = treeLayerElem.attribute( u"id"_s );
    const QString layername = treeLayerElem.attribute( u"name"_s );
    const QString newid = QgsMapLayer::generateId( layername );
    treeLayerElem.setAttribute( u"id"_s, newid );

    // Replace IDs for map layers
    const QDomNodeList ids = doc.elementsByTagName( u"id"_s );
    QDomNode idnode = ids.at( 0 );
    for ( int j = 0; ! idnode.isNull() ; ++j )
    {
      idnode = ids.at( j );
      const QDomElement idElem = idnode.toElement();
      if ( idElem.text() == oldid )
      {
        idElem.firstChild().setNodeValue( newid );
      }
    }

    // change layer IDs for vector joins
    const QDomNodeList vectorJoinNodes = doc.elementsByTagName( u"join"_s ); // TODO: Find a better way of searching for vectorjoins, there might be other <join> elements within the project.
    for ( int j = 0; j < vectorJoinNodes.size(); ++j )
    {
      const QDomNode joinNode = vectorJoinNodes.at( j );
      const QDomElement joinElement = joinNode.toElement();
      if ( joinElement.attribute( u"joinLayerId"_s ) == oldid )
      {
        joinNode.toElement().setAttribute( u"joinLayerId"_s, newid );
      }
    }

    // change IDs of dependencies
    const QDomNodeList dataDeps = doc.elementsByTagName( u"dataDependencies"_s );
    for ( int i = 0; i < dataDeps.size(); i++ )
    {
      const QDomNodeList layers = dataDeps.at( i ).childNodes();
      for ( int j = 0; j < layers.size(); j++ )
      {
        QDomElement elt = layers.at( j ).toElement();
        if ( elt.attribute( u"id"_s ) == oldid )
        {
          elt.setAttribute( u"id"_s, newid );
        }
      }
    }

    // Change IDs of widget config values
    const QDomNodeList widgetConfig = doc.elementsByTagName( u"editWidget"_s );
    for ( int i = 0; i < widgetConfig.size(); i++ )
    {
      const QDomNodeList config = widgetConfig.at( i ).childNodes();
      for ( int j = 0; j < config.size(); j++ )
      {
        const QDomNodeList optMap = config.at( j ).childNodes();
        for ( int z = 0; z < optMap.size(); z++ )
        {
          const QDomNodeList opts = optMap.at( z ).childNodes();
          for ( int k = 0; k < opts.size(); k++ )
          {
            QDomElement opt = opts.at( k ).toElement();
            if ( opt.attribute( u"value"_s ) == oldid )
            {
              opt.setAttribute( u"value"_s, newid );
            }
          }
        }
      }
    }
  }

  QDomElement layerTreeElem = doc.documentElement().firstChildElement( u"layer-tree-group"_s );
  bool loadInLegend = true;
  if ( !layerTreeElem.isNull() )
  {
    root.readChildrenFromXml( layerTreeElem, context );
    loadInLegend = false;
  }

  const QList<QgsMapLayer *> layers = QgsLayerDefinition::loadLayerDefinitionLayersInternal( doc, context, errorMessage );

  project->addMapLayers( layers, loadInLegend );

  // Now that all layers are loaded, refresh the vectorjoins to get the joined fields
  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    layer->resolveReferences( project );
  }

  root.resolveReferences( project );

  const QList<QgsLayerTreeNode *> nodes = root.children();
  root.abandonChildren();

  switch ( insertMethod )
  {
    case Qgis::LayerTreeInsertionMethod::AboveInsertionPoint:
      if ( insertPoint )
      {
        insertPoint->group->insertChildNodes( insertPoint->position, nodes );
      }
      else
      {
        rootGroup->insertChildNodes( -1, nodes );
      }
      break;
    case Qgis::LayerTreeInsertionMethod::TopOfTree:
      rootGroup->insertChildNodes( 0, nodes );
      break;
    default: //Keep current behavior for Qgis::LayerTreeInsertionMethod::OptimalInInsertionGroup
      rootGroup->insertChildNodes( -1, nodes );
  }

  return true;
}

bool QgsLayerDefinition::exportLayerDefinition( const QString &path, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage )
{
  return exportLayerDefinition( path, selectedTreeNodes, QgsProject::instance()->filePathStorage(), errorMessage ); // skip-keyword-check
}

bool QgsLayerDefinition::exportLayerDefinition( const QString &p, const QList<QgsLayerTreeNode *> &selectedTreeNodes, Qgis::FilePathType pathType, QString &errorMessage )
{
  const QString path = QgsFileUtils::ensureFileNameHasExtension( p, { u"qlr"_s} );

  QFile file( path );
  if ( !file.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    errorMessage = file.errorString();
    return false;
  }

  QgsReadWriteContext context;
  switch ( pathType )
  {
    case Qgis::FilePathType::Absolute:
      context.setPathResolver( QgsPathResolver( QString() ) );
      break;
    case Qgis::FilePathType::Relative:
      context.setPathResolver( QgsPathResolver( path ) );
      break;
  }

  const QDomDocument doc( u"qgis-layer-definition"_s );
  if ( !exportLayerDefinition( doc, selectedTreeNodes, errorMessage, context ) )
    return false;

  QTextStream qlayerstream( &file );
  doc.save( qlayerstream, 2 );
  return true;
}

bool QgsLayerDefinition::exportLayerDefinition( QDomDocument doc, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage, const QgsReadWriteContext &context )
{
  Q_UNUSED( errorMessage )
  QDomElement qgiselm = doc.createElement( u"qlr"_s );
  doc.appendChild( qgiselm );
  QgsLayerTreeGroup root;
  for ( QgsLayerTreeNode *node : selectedTreeNodes )
  {
    QgsLayerTreeNode *newnode = node->clone();
    root.addChildNode( newnode );
  }
  root.writeXml( qgiselm, context );

  QDomElement layerselm = doc.createElement( u"maplayers"_s );
  const QList<QgsLayerTreeLayer *> layers = root.findLayers();
  for ( QgsLayerTreeLayer *layer : layers )
  {
    if ( ! layer->layer() )
    {
      QgsDebugMsgLevel( u"Not a valid map layer: skipping %1"_s.arg( layer->name( ) ), 4 );
      continue;
    }
    QDomElement layerelm = doc.createElement( u"maplayer"_s );
    layer->layer()->writeLayerXml( layerelm, doc, context );
    layerselm.appendChild( layerelm );
  }
  qgiselm.appendChild( layerselm );
  return true;
}

QDomDocument QgsLayerDefinition::exportLayerDefinitionLayers( const QList<QgsMapLayer *> &layers, const QgsReadWriteContext &context )
{
  QDomDocument doc( u"qgis-layer-definition"_s );
  QDomElement qgiselm = doc.createElement( u"qlr"_s );
  doc.appendChild( qgiselm );
  QDomElement layerselm = doc.createElement( u"maplayers"_s );
  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    QDomElement layerelm = doc.createElement( u"maplayer"_s );
    layer->writeLayerXml( layerelm, doc, context );
    layerselm.appendChild( layerelm );
  }
  qgiselm.appendChild( layerselm );
  return doc;
}

QList<QgsMapLayer *> QgsLayerDefinition::loadLayerDefinitionLayers( QDomDocument &document, QgsReadWriteContext &context )
{
  QString errorMessage;
  return loadLayerDefinitionLayersInternal( document, context, errorMessage );
}

QList<QgsMapLayer *> QgsLayerDefinition::loadLayerDefinitionLayersInternal( QDomDocument &document, QgsReadWriteContext &context, QString &errorMessage )
{
  QList<QgsMapLayer *> layers;
  QDomElement layerElem = document.documentElement().firstChildElement( u"projectlayers"_s ).firstChildElement( u"maplayer"_s );
  // For QLR:
  if ( layerElem.isNull() )
  {
    layerElem = document.documentElement().firstChildElement( u"maplayers"_s ).firstChildElement( u"maplayer"_s );
  }

  while ( ! layerElem.isNull() )
  {
    const QString type = layerElem.attribute( u"type"_s );
    QgsMapLayer *layer = nullptr;

    bool ok = false;
    const Qgis::LayerType layerType = QgsMapLayerFactory::typeFromString( type, ok );
    if ( ok )
    {
      switch ( layerType )
      {
        case Qgis::LayerType::Vector:
          layer = new QgsVectorLayer();
          break;

        case Qgis::LayerType::Raster:
          layer = new QgsRasterLayer();
          break;

        case Qgis::LayerType::Plugin:
        {
          const QString typeName = layerElem.attribute( u"name"_s );
          layer = QgsApplication::pluginLayerRegistry()->createLayer( typeName );
          break;
        }

        case Qgis::LayerType::Mesh:
          layer = new QgsMeshLayer();
          break;

        case Qgis::LayerType::VectorTile:
          layer = new QgsVectorTileLayer;
          break;

        case Qgis::LayerType::PointCloud:
          layer = new QgsPointCloudLayer();
          break;

        case Qgis::LayerType::TiledScene:
          layer = new QgsTiledSceneLayer;
          break;

        case Qgis::LayerType::Group:
          layer = new QgsGroupLayer( QString(), QgsGroupLayer::LayerOptions( QgsCoordinateTransformContext() ) );
          break;

        case Qgis::LayerType::Annotation:
          break;
      }
    }

    if ( layer )
    {
      // always add the layer, even if the source is invalid -- this allows users to fix the source
      // at a later stage and still retain all the layer properties intact
      layer->readLayerXml( layerElem, context );
      layers << layer;
    }
    else
    {
      errorMessage = QObject::tr( "Unsupported layer type: %1" ).arg( type );
    }
    layerElem = layerElem.nextSiblingElement( u"maplayer"_s );
  }
  return layers;
}

QList<QgsMapLayer *> QgsLayerDefinition::loadLayerDefinitionLayers( const QString &qlrfile )
{
  QFile file( qlrfile );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugError( u"Can't open file"_s );
    return QList<QgsMapLayer *>();
  }

  QDomDocument doc;
  if ( !doc.setContent( &file ) )
  {
    QgsDebugError( u"Can't set content"_s );
    return QList<QgsMapLayer *>();
  }

  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( qlrfile ) );
  //no project translator defined here
  return QgsLayerDefinition::loadLayerDefinitionLayers( doc, context );
}

void QgsLayerDefinition::DependencySorter::init( const QDomDocument &doc )
{
  // Determine a loading order of layers based on a graph of dependencies
  QMap< QString, QVector< QString > > dependencies;
  QStringList sortedLayers;
  QList< QPair<QString, QDomNode> > layersToSort;
  QStringList layerIds;

  QDomElement layerElem = doc.documentElement().firstChildElement( u"projectlayers"_s ).firstChildElement( u"maplayer"_s );
  // For QLR:
  if ( layerElem.isNull() )
  {
    layerElem = doc.documentElement().firstChildElement( u"maplayers"_s ).firstChildElement( u"maplayer"_s );
  }
  // For tests (I don't know if there is a real use case for such a document except for test_qgslayerdefinition.py)
  if ( layerElem.isNull() )
  {
    layerElem = doc.documentElement().firstChildElement( u"maplayer"_s );
  }

  const QDomElement &firstElement { layerElem };

  QVector<QString> deps; //avoid expensive allocation for list for every iteration
  while ( !layerElem.isNull() )
  {
    deps.resize( 0 ); // preserve capacity - don't use clear

    const QString id = layerElem.namedItem( u"id"_s ).toElement().text();
    layerIds << id;

    // dependencies for this layer
    const QDomElement layerDependenciesElem = layerElem.firstChildElement( u"layerDependencies"_s );
    if ( !layerDependenciesElem.isNull() )
    {
      const QDomNodeList dependencyList = layerDependenciesElem.elementsByTagName( u"layer"_s );
      for ( int j = 0; j < dependencyList.size(); ++j )
      {
        const QDomElement depElem = dependencyList.at( j ).toElement();
        deps << depElem.attribute( u"id"_s );
      }
    }
    dependencies[id] = deps;

    if ( deps.empty() )
    {
      sortedLayers << id;
      mSortedLayerNodes << layerElem;
      mSortedLayerIds << id;
    }
    else
    {
      layersToSort << qMakePair( id, layerElem );
      mDependentLayerIds.insert( id );
    }
    layerElem = layerElem.nextSiblingElement( );
  }

  // check that all dependencies are present
  const auto constDependencies = dependencies;
  for ( const QVector< QString > &ids : constDependencies )
  {
    const auto constIds = ids;
    for ( const QString &depId : constIds )
    {
      if ( !dependencies.contains( depId ) )
      {
        // some dependencies are not satisfied
        mHasMissingDependency = true;
        layerElem = firstElement;
        while ( ! layerElem.isNull() )
        {
          mSortedLayerNodes << layerElem;
          layerElem = layerElem.nextSiblingElement( );
        }
        mSortedLayerIds = layerIds;
        return;
      }
    }
  }

  // cycles should be very rare, since layers with cyclic dependencies may only be created by
  // manually modifying the project file
  mHasCycle = false;

  while ( !layersToSort.empty() && !mHasCycle )
  {
    QList< QPair<QString, QDomNode> >::iterator it = layersToSort.begin();
    while ( it != layersToSort.end() )
    {
      const QString idToSort = it->first;
      const QDomNode node = it->second;
      mHasCycle = true;
      bool resolved = true;
      const auto deps { dependencies.value( idToSort ) };
      for ( const QString &dep : deps )
      {
        if ( !sortedLayers.contains( dep ) )
        {
          resolved = false;
          break;
        }
      }
      if ( resolved ) // dependencies for this layer are resolved
      {
        sortedLayers << idToSort;
        mSortedLayerNodes << node;
        mSortedLayerIds << idToSort;
        it = layersToSort.erase( it ); // erase and go to the next
        mHasCycle = false;
      }
      else
      {
        ++it;
      }
    }
  }
}

QgsLayerDefinition::DependencySorter::DependencySorter( const QDomDocument &doc )
  : mHasCycle( false )
  , mHasMissingDependency( false )
{
  init( doc );
}

QgsLayerDefinition::DependencySorter::DependencySorter( const QString &fileName )
  : mHasCycle( false )
  , mHasMissingDependency( false )
{
  QString qgsProjectFile = fileName;
  QgsProjectArchive archive;
  if ( fileName.endsWith( ".qgz"_L1, Qt::CaseInsensitive ) )
  {
    archive.unzip( fileName );
    qgsProjectFile = archive.projectFile();
  }

  QDomDocument doc;
  QFile pFile( qgsProjectFile );
  ( void )pFile.open( QIODevice::ReadOnly );
  ( void )doc.setContent( &pFile );
  init( doc );
}

bool QgsLayerDefinition::DependencySorter::isLayerDependent( const QString &layerId ) const
{
  return mDependentLayerIds.contains( layerId );
}


