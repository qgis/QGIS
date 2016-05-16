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
#include <QDomNode>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgslayertree.h"
#include "qgsmaplayerregistry.h"
#include "qgslayerdefinition.h"

bool QgsLayerDefinition::loadLayerDefinition( const QString &path, QgsLayerTreeGroup *rootGroup, QString &errorMessage )
{
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    errorMessage = QLatin1String( "Can not open file" );
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

  return loadLayerDefinition( doc, rootGroup, errorMessage );
}

bool QgsLayerDefinition::loadLayerDefinition( QDomDocument doc, QgsLayerTreeGroup *rootGroup, QString &errorMessage )
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
    Q_FOREACH ( const QDomNode& node, sortedLayerNodes )
    {
      clonedSorted << node.cloneNode();
    }
    QDomNode layersNode = doc.elementsByTagName( "maplayers" ).at( 0 );
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
  QDomNodeList ids = doc.elementsByTagName( "id" );
  for ( int i = 0; i < ids.size(); ++i )
  {
    QDomNode idnode = ids.at( i );
    QDomElement idElem = idnode.toElement();
    QString oldid = idElem.text();
    // Strip the date part because we will replace it.
    QString layername = oldid.left( oldid.length() - 17 );
    QDateTime dt = QDateTime::currentDateTime();
    QString newid = layername + dt.toString( "yyyyMMddhhmmsszzz" ) + QString::number( qrand() );
    idElem.firstChild().setNodeValue( newid );
    QDomNodeList treeLayerNodes = doc.elementsByTagName( "layer-tree-layer" );

    for ( int i = 0; i < treeLayerNodes.count(); ++i )
    {
      QDomNode layerNode = treeLayerNodes.at( i );
      QDomElement layerElem = layerNode.toElement();
      if ( layerElem.attribute( "id" ) == oldid )
      {
        layerNode.toElement().setAttribute( "id", newid );
      }
    }

    // change layer IDs for vector joins
    QDomNodeList vectorJoinNodes = doc.elementsByTagName( "join" ); // TODO: Find a better way of searching for vectorjoins, there might be other <join> elements within the project.
    for ( int j = 0; j < vectorJoinNodes.size(); ++j )
    {
      QDomNode joinNode = vectorJoinNodes.at( j );
      QDomElement joinElement = joinNode.toElement();
      if ( joinElement.attribute( "joinLayerId" ) == oldid )
      {
        joinNode.toElement().setAttribute( "joinLayerId", newid );
      }
    }
  }

  QDomElement layerTreeElem = doc.documentElement().firstChildElement( "layer-tree-group" );
  bool loadInLegend = true;
  if ( !layerTreeElem.isNull() )
  {
    root->readChildrenFromXML( layerTreeElem );
    loadInLegend = false;
  }

  QList<QgsMapLayer*> layers = QgsMapLayer::fromLayerDefinition( doc, /*addToRegistry*/ true, loadInLegend );

  // Now that all layers are loaded, refresh the vectorjoins to get the joined fields
  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    QgsVectorLayer* vlayer = dynamic_cast< QgsVectorLayer * >( layer );
    if ( vlayer )
    {
      vlayer->createJoinCaches();
      vlayer->updateFields();
    }
  }

  QList<QgsLayerTreeNode*> nodes = root->children();
  Q_FOREACH ( QgsLayerTreeNode *node, nodes )
    root->takeChild( node );
  delete root;

  rootGroup->insertChildNodes( -1, nodes );

  return true;

}

bool QgsLayerDefinition::exportLayerDefinition( QString path, const QList<QgsLayerTreeNode*>& selectedTreeNodes, QString &errorMessage )
{
  if ( !path.endsWith( ".qlr" ) )
    path = path.append( ".qlr" );

  QFile file( path );

  if ( !file.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    errorMessage = file.errorString();
    return false;
  }

  QFileInfo fileinfo( file );

  QDomDocument doc( "qgis-layer-definition" );
  if ( !exportLayerDefinition( doc, selectedTreeNodes, errorMessage, fileinfo.canonicalFilePath() ) )
    return false;

  QTextStream qlayerstream( &file );
  doc.save( qlayerstream, 2 );
  return true;
}

bool QgsLayerDefinition::exportLayerDefinition( QDomDocument doc, const QList<QgsLayerTreeNode*>& selectedTreeNodes, QString &errorMessage, const QString& relativeBasePath )
{
  Q_UNUSED( errorMessage );
  QDomElement qgiselm = doc.createElement( "qlr" );
  doc.appendChild( qgiselm );
  QList<QgsLayerTreeNode*> nodes = selectedTreeNodes;
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup;
  Q_FOREACH ( QgsLayerTreeNode* node, nodes )
  {
    QgsLayerTreeNode* newnode = node->clone();
    root->addChildNode( newnode );
  }
  root->writeXML( qgiselm );

  QDomElement layerselm = doc.createElement( "maplayers" );
  QList<QgsLayerTreeLayer*> layers = root->findLayers();
  Q_FOREACH ( QgsLayerTreeLayer* layer, layers )
  {
    QDomElement layerelm = doc.createElement( "maplayer" );
    layer->layer()->writeLayerXML( layerelm, doc, relativeBasePath );
    layerselm.appendChild( layerelm );
  }
  qgiselm.appendChild( layerselm );
  return true;
}

void QgsLayerDefinition::DependencySorter::init( const QDomDocument& doc )
{
  // Determine a loading order of layers based on a graph of dependencies
  QMap< QString, QVector< QString > > dependencies;
  QStringList sortedLayers;
  QList< QPair<QString, QDomNode> > layersToSort;
  QStringList layerIds;

  QDomNodeList nl = doc.elementsByTagName( "maplayer" );
  for ( int i = 0; i < nl.count(); i++ )
  {
    QVector<QString> deps;
    QDomNode node = nl.item( i );
    QDomElement element = node.toElement();

    QString id = node.namedItem( "id" ).toElement().text();
    layerIds << id;

    // dependencies for this layer
    QDomElement layerDependenciesElem = node.firstChildElement( "layerDependencies" );
    if ( !layerDependenciesElem.isNull() )
    {
      QDomNodeList dependencyList = layerDependenciesElem.elementsByTagName( "layer" );
      for ( int j = 0; j < dependencyList.size(); ++j )
      {
        QDomElement depElem = dependencyList.at( j ).toElement();
        deps << depElem.attribute( "id" );
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
  Q_FOREACH ( const QVector< QString >& ids, dependencies )
  {
    Q_FOREACH ( const QString& depId, ids )
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
      Q_FOREACH ( const QString& dep, dependencies[idToSort] )
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

QgsLayerDefinition::DependencySorter::DependencySorter( const QDomDocument& doc )
    : mHasCycle( false )
    , mHasMissingDependency( false )
{
  init( doc );
}

QgsLayerDefinition::DependencySorter::DependencySorter( const QString& fileName )
    : mHasCycle( false )
    , mHasMissingDependency( false )
{
  QDomDocument doc;
  QFile pFile( fileName );
  ( void )pFile.open( QIODevice::ReadOnly );
  ( void )doc.setContent( &pFile );
  init( doc );
}


