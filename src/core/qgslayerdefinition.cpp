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
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "qgslayerdefinition.h"
#include "qgslayertree.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgspathresolver.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

bool QgsLayerDefinition::loadLayerDefinition( const QString &path, QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage )
{
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    errorMessage = QStringLiteral( "Can not open file" );
    return false;
  }

  QDomDocument doc;
  QString message;
  if ( !doc.setContent( &file, &message ) )
  {
    errorMessage = message;
    return false;
  }

  QFileInfo fileinfo( file );
  QDir::setCurrent( fileinfo.absoluteDir().path() );

  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( path ) );
  context.setProjectTranslator( project );

  return loadLayerDefinition( doc, project, rootGroup, errorMessage, context );
}

bool QgsLayerDefinition::loadLayerDefinition( QDomDocument doc, QgsProject *project, QgsLayerTreeGroup *rootGroup, QString &errorMessage, QgsReadWriteContext &context )
{
  Q_UNUSED( errorMessage );

  QgsLayerTreeGroup *root = new QgsLayerTreeGroup();

  // reorder maplayer nodes based on dependencies
  // dependencies have to be resolved before IDs get changed
  DependencySorter depSorter( doc );
  if ( !depSorter.hasMissingDependency() )
  {
    QVector<QDomNode> sortedLayerNodes = depSorter.sortedLayerNodes();
    QVector<QDomNode> clonedSorted;
    Q_FOREACH ( const QDomNode &node, sortedLayerNodes )
    {
      clonedSorted << node.cloneNode();
    }
    QDomNode layersNode = doc.elementsByTagName( QStringLiteral( "maplayers" ) ).at( 0 );
    // replace old children with new ones
    QDomNodeList childNodes = layersNode.childNodes();
    for ( int i = 0; i < childNodes.size(); i++ )
    {
      layersNode.replaceChild( clonedSorted.at( i ), childNodes.at( i ) );
    }
  }
  // if a dependency is missing, we still try to load layers, since dependencies may already be loaded

  // IDs of layers should be changed otherwise we may have more then one layer with the same id
  // We have to replace the IDs before we load them because it's too late once they are loaded
  QDomNodeList ids = doc.elementsByTagName( QStringLiteral( "id" ) );
  for ( int i = 0; i < ids.size(); ++i )
  {
    QDomNode idnode = ids.at( i );
    QDomElement idElem = idnode.toElement();
    QString oldid = idElem.text();
    // Strip the date part because we will replace it.
    QString layername = oldid.left( oldid.length() - 17 );
    QDateTime dt = QDateTime::currentDateTime();
    QString newid = layername + dt.toString( QStringLiteral( "yyyyMMddhhmmsszzz" ) ) + QString::number( qrand() );
    idElem.firstChild().setNodeValue( newid );
    QDomNodeList treeLayerNodes = doc.elementsByTagName( QStringLiteral( "layer-tree-layer" ) );

    for ( int i = 0; i < treeLayerNodes.count(); ++i )
    {
      QDomNode layerNode = treeLayerNodes.at( i );
      QDomElement layerElem = layerNode.toElement();
      if ( layerElem.attribute( QStringLiteral( "id" ) ) == oldid )
      {
        layerNode.toElement().setAttribute( QStringLiteral( "id" ), newid );
      }
    }

    // change layer IDs for vector joins
    QDomNodeList vectorJoinNodes = doc.elementsByTagName( QStringLiteral( "join" ) ); // TODO: Find a better way of searching for vectorjoins, there might be other <join> elements within the project.
    for ( int j = 0; j < vectorJoinNodes.size(); ++j )
    {
      QDomNode joinNode = vectorJoinNodes.at( j );
      QDomElement joinElement = joinNode.toElement();
      if ( joinElement.attribute( QStringLiteral( "joinLayerId" ) ) == oldid )
      {
        joinNode.toElement().setAttribute( QStringLiteral( "joinLayerId" ), newid );
      }
    }

    // change IDs of dependencies
    QDomNodeList dataDeps = doc.elementsByTagName( QStringLiteral( "dataDependencies" ) );
    for ( int i = 0; i < dataDeps.size(); i++ )
    {
      QDomNodeList layers = dataDeps.at( i ).childNodes();
      for ( int j = 0; j < layers.size(); j++ )
      {
        QDomElement elt = layers.at( j ).toElement();
        if ( elt.attribute( QStringLiteral( "id" ) ) == oldid )
        {
          elt.setAttribute( QStringLiteral( "id" ), newid );
        }
      }
    }

  }

  QDomElement layerTreeElem = doc.documentElement().firstChildElement( QStringLiteral( "layer-tree-group" ) );
  bool loadInLegend = true;
  if ( !layerTreeElem.isNull() )
  {
    root->readChildrenFromXml( layerTreeElem, context );
    loadInLegend = false;
  }

  QList<QgsMapLayer *> layers = QgsLayerDefinition::loadLayerDefinitionLayers( doc, context );

  project->addMapLayers( layers, loadInLegend );

  // Now that all layers are loaded, refresh the vectorjoins to get the joined fields
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    if ( QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( layer ) )
    {
      vlayer->resolveReferences( project );
    }
  }

  root->resolveReferences( project );

  QList<QgsLayerTreeNode *> nodes = root->children();
  Q_FOREACH ( QgsLayerTreeNode *node, nodes )
    root->takeChild( node );
  delete root;

  rootGroup->insertChildNodes( -1, nodes );

  return true;

}

bool QgsLayerDefinition::exportLayerDefinition( QString path, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage )
{
  if ( !path.endsWith( QLatin1String( ".qlr" ) ) )
    path = path.append( ".qlr" );

  QFile file( path );

  if ( !file.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    errorMessage = file.errorString();
    return false;
  }

  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( path ) );

  QDomDocument doc( QStringLiteral( "qgis-layer-definition" ) );
  if ( !exportLayerDefinition( doc, selectedTreeNodes, errorMessage, context ) )
    return false;

  QTextStream qlayerstream( &file );
  doc.save( qlayerstream, 2 );
  return true;
}

bool QgsLayerDefinition::exportLayerDefinition( QDomDocument doc, const QList<QgsLayerTreeNode *> &selectedTreeNodes, QString &errorMessage, const QgsReadWriteContext &context )
{
  Q_UNUSED( errorMessage );
  QDomElement qgiselm = doc.createElement( QStringLiteral( "qlr" ) );
  doc.appendChild( qgiselm );
  QList<QgsLayerTreeNode *> nodes = selectedTreeNodes;
  QgsLayerTreeGroup *root = new QgsLayerTreeGroup;
  Q_FOREACH ( QgsLayerTreeNode *node, nodes )
  {
    QgsLayerTreeNode *newnode = node->clone();
    root->addChildNode( newnode );
  }
  root->writeXml( qgiselm, context );

  QDomElement layerselm = doc.createElement( QStringLiteral( "maplayers" ) );
  QList<QgsLayerTreeLayer *> layers = root->findLayers();
  Q_FOREACH ( QgsLayerTreeLayer *layer, layers )
  {
    if ( ! layer->layer() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Not a valid map layer: skipping %1" ).arg( layer->name( ) ), 4 );
      continue;
    }
    QDomElement layerelm = doc.createElement( QStringLiteral( "maplayer" ) );
    layer->layer()->writeLayerXml( layerelm, doc, context );
    layerselm.appendChild( layerelm );
  }
  qgiselm.appendChild( layerselm );
  return true;
}

QDomDocument QgsLayerDefinition::exportLayerDefinitionLayers( const QList<QgsMapLayer *> &layers, const QgsReadWriteContext &context )
{
  QDomDocument doc( QStringLiteral( "qgis-layer-definition" ) );
  QDomElement qgiselm = doc.createElement( QStringLiteral( "qlr" ) );
  doc.appendChild( qgiselm );
  QDomElement layerselm = doc.createElement( QStringLiteral( "maplayers" ) );
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    QDomElement layerelm = doc.createElement( QStringLiteral( "maplayer" ) );
    layer->writeLayerXml( layerelm, doc, context );
    layerselm.appendChild( layerelm );
  }
  qgiselm.appendChild( layerselm );
  return doc;
}

QList<QgsMapLayer *> QgsLayerDefinition::loadLayerDefinitionLayers( QDomDocument &document, QgsReadWriteContext &context )
{
  QList<QgsMapLayer *> layers;
  QDomNodeList layernodes = document.elementsByTagName( QStringLiteral( "maplayer" ) );
  for ( int i = 0; i < layernodes.size(); ++i )
  {
    QDomNode layernode = layernodes.at( i );
    QDomElement layerElem = layernode.toElement();

    QString type = layerElem.attribute( QStringLiteral( "type" ) );
    QgsDebugMsg( type );
    QgsMapLayer *layer = nullptr;

    if ( type == QLatin1String( "vector" ) )
    {
      layer = new QgsVectorLayer;
    }
    else if ( type == QLatin1String( "raster" ) )
    {
      layer = new QgsRasterLayer;
    }
    else if ( type == QLatin1String( "plugin" ) )
    {
      QString typeName = layerElem.attribute( QStringLiteral( "name" ) );
      layer = QgsApplication::pluginLayerRegistry()->createLayer( typeName );
    }

    if ( !layer )
      continue;

    if ( layer->readLayerXml( layerElem, context ) )
    {
      layers << layer;
    }
  }
  return layers;
}

QList<QgsMapLayer *> QgsLayerDefinition::loadLayerDefinitionLayers( const QString &qlrfile )
{
  QFile file( qlrfile );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QStringLiteral( "Can't open file" ) );
    return QList<QgsMapLayer *>();
  }

  QDomDocument doc;
  if ( !doc.setContent( &file ) )
  {
    QgsDebugMsg( QStringLiteral( "Can't set content" ) );
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

  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "maplayer" ) );
  layerIds.reserve( nl.count() );
  QVector<QString> deps; //avoid expensive allocation for list for every iteration
  for ( int i = 0; i < nl.count(); i++ )
  {
    deps.resize( 0 ); // preserve capacity - don't use clear
    QDomNode node = nl.item( i );

    QString id = node.namedItem( QStringLiteral( "id" ) ).toElement().text();
    layerIds << id;

    // dependencies for this layer
    QDomElement layerDependenciesElem = node.firstChildElement( QStringLiteral( "layerDependencies" ) );
    if ( !layerDependenciesElem.isNull() )
    {
      QDomNodeList dependencyList = layerDependenciesElem.elementsByTagName( QStringLiteral( "layer" ) );
      for ( int j = 0; j < dependencyList.size(); ++j )
      {
        QDomElement depElem = dependencyList.at( j ).toElement();
        deps << depElem.attribute( QStringLiteral( "id" ) );
      }
    }
    dependencies[id] = deps;

    if ( deps.empty() )
    {
      sortedLayers << id;
      mSortedLayerNodes << node;
      mSortedLayerIds << id;
    }
    else
      layersToSort << qMakePair( id, node );
  }

  // check that all dependencies are present
  Q_FOREACH ( const QVector< QString > &ids, dependencies )
  {
    Q_FOREACH ( const QString &depId, ids )
    {
      if ( !dependencies.contains( depId ) )
      {
        // some dependencies are not satisfied
        mHasMissingDependency = true;
        for ( int i = 0; i < nl.size(); i++ )
          mSortedLayerNodes << nl.at( i );
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
      QString idToSort = it->first;
      QDomNode node = it->second;
      mHasCycle = true;
      bool resolved = true;
      Q_FOREACH ( const QString &dep, dependencies[idToSort] )
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
  QDomDocument doc;
  QFile pFile( fileName );
  ( void )pFile.open( QIODevice::ReadOnly );
  ( void )doc.setContent( &pFile );
  init( doc );
}


