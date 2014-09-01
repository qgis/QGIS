/***************************************************************************
    qgsrelationmanager.cpp
     --------------------------------------
    Date                 : 1.3.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationmanager.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

QgsRelationManager::QgsRelationManager( QgsProject* project )
    : QObject( project )
    , mProject( project )
{
  connect( project, SIGNAL( readProject( const QDomDocument& ) ), SLOT( readProject( const QDomDocument& ) ) );
  connect( project, SIGNAL( writeProject( QDomDocument& ) ), SLOT( writeProject( QDomDocument& ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ), this, SLOT( layersRemoved( QStringList ) ) );
}

void QgsRelationManager::setRelations( const QList<QgsRelation>& relations )
{
  mRelations.clear();
  foreach ( const QgsRelation& rel, relations )
  {
    addRelation( rel );
  }
}

const QMap<QString, QgsRelation>& QgsRelationManager::relations() const
{
  return mRelations;
}

void QgsRelationManager::addRelation( const QgsRelation& relation )
{
  if ( !relation.isValid() )
    return;

  mRelations.insert( relation.id(), relation );

  mProject->dirty( true );
}

void QgsRelationManager::removeRelation( const QString& name )
{
  mRelations.remove( name );
}

void QgsRelationManager::removeRelation( const QgsRelation& relation )
{
  mRelations.remove( relation.id() );
}

QgsRelation QgsRelationManager::relation( const QString& id ) const
{
  return mRelations.value( id );
}

void QgsRelationManager::clear()
{
  mRelations.clear();
}

QList<QgsRelation> QgsRelationManager::referencingRelations( QgsVectorLayer* layer, int fieldIdx ) const
{
  if ( !layer )
  {
    return mRelations.values();
  }

  QList<QgsRelation> relations;

  foreach ( const QgsRelation& rel, mRelations )
  {
    if ( rel.referencingLayer() == layer )
    {
      if ( fieldIdx != -2 )
      {
        bool containsField = false;
        foreach ( const QgsRelation::FieldPair& fp, rel.fieldPairs() )
        {
          if ( fieldIdx == layer->fieldNameIndex( fp.referencingField() ) )
          {
            containsField = true;
            break;
          }
        }

        if ( !containsField )
        {
          continue;
        }
      }
      relations.append( rel );
    }
  }

  return relations;
}

QList<QgsRelation> QgsRelationManager::referencedRelations( QgsVectorLayer* layer ) const
{
  if ( !layer )
  {
    return mRelations.values();
  }

  QList<QgsRelation> relations;

  foreach ( const QgsRelation& rel, mRelations )
  {
    if ( rel.referencedLayer() == layer )
    {
      relations.append( rel );
    }
  }

  return relations;
}

void QgsRelationManager::readProject( const QDomDocument & doc )
{
  mRelations.clear();

  QDomNodeList nodes = doc.elementsByTagName( "relations" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    QDomNodeList relationNodes = node.childNodes();
    int relCount = relationNodes.count();
    for ( int i = 0; i < relCount; ++i )
    {
      addRelation( QgsRelation::createFromXML( relationNodes.at( i ) ) );
    }
  }
  else
  {
    QgsDebugMsg( "No relations data present in this document" );
  }

  emit( relationsLoaded() );
}

void QgsRelationManager::writeProject( QDomDocument & doc )
{
  QDomNodeList nl = doc.elementsByTagName( "qgis" );
  if ( !nl.count() )
  {
    QgsDebugMsg( "Unable to find qgis element in project file" );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one

  QDomElement relationsNode = doc.createElement( "relations" );
  qgisNode.appendChild( relationsNode );

  foreach ( const QgsRelation& relation, mRelations )
  {
    relation.writeXML( relationsNode, doc );
  }
}

void QgsRelationManager::layersRemoved( const QStringList& layers )
{
  Q_FOREACH ( const QString& layer, layers )
  {
    QMapIterator<QString, QgsRelation> it( mRelations );

    while ( it.hasNext() )
    {
      it.next();

      if ( it.value().referencedLayerId() == layer
           || it.value().referencingLayerId() == layer )
      {
        mRelations.remove( it.key() );
      }
    }
  }
}
