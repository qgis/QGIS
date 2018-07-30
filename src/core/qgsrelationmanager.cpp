/***************************************************************************
    qgsrelationmanager.cpp
     --------------------------------------
    Date                 : 1.3.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

QgsRelationManager::QgsRelationManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{
  if ( mProject )
  {
    connect( project, &QgsProject::readProject, this, &QgsRelationManager::readProject );
    connect( project, &QgsProject::writeProject, this, &QgsRelationManager::writeProject );
    connect( project, &QgsProject::layersRemoved, this, &QgsRelationManager::layersRemoved );
  }
}

void QgsRelationManager::setRelations( const QList<QgsRelation> &relations )
{
  mRelations.clear();
  Q_FOREACH ( const QgsRelation &rel, relations )
  {
    addRelation( rel );
  }
  emit changed();
}

QMap<QString, QgsRelation> QgsRelationManager::relations() const
{
  return mRelations;
}

void QgsRelationManager::addRelation( const QgsRelation &relation )
{
  if ( !relation.isValid() )
    return;

  mRelations.insert( relation.id(), relation );

  if ( mProject )
    mProject->setDirty( true );
  emit changed();
}

void QgsRelationManager::removeRelation( const QString &id )
{
  mRelations.remove( id );
  emit changed();
}

void QgsRelationManager::removeRelation( const QgsRelation &relation )
{
  mRelations.remove( relation.id() );
  emit changed();
}

QgsRelation QgsRelationManager::relation( const QString &id ) const
{
  return mRelations.value( id );
}

QList<QgsRelation> QgsRelationManager::relationsByName( const QString &name ) const
{
  QList<QgsRelation> relations;

  Q_FOREACH ( const QgsRelation &rel, mRelations )
  {
    if ( QString::compare( rel.name(), name, Qt::CaseInsensitive ) == 0 )
      relations << rel;
  }

  return relations;
}

void QgsRelationManager::clear()
{
  mRelations.clear();
  emit changed();
}

QList<QgsRelation> QgsRelationManager::referencingRelations( const QgsVectorLayer *layer, int fieldIdx ) const
{
  if ( !layer )
  {
    return mRelations.values();
  }

  QList<QgsRelation> relations;

  Q_FOREACH ( const QgsRelation &rel, mRelations )
  {
    if ( rel.referencingLayer() == layer )
    {
      if ( fieldIdx != -2 )
      {
        bool containsField = false;
        Q_FOREACH ( const QgsRelation::FieldPair &fp, rel.fieldPairs() )
        {
          if ( fieldIdx == layer->fields().lookupField( fp.referencingField() ) )
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

QList<QgsRelation> QgsRelationManager::referencedRelations( QgsVectorLayer *layer ) const
{
  if ( !layer )
  {
    return mRelations.values();
  }

  QList<QgsRelation> relations;

  Q_FOREACH ( const QgsRelation &rel, mRelations )
  {
    if ( rel.referencedLayer() == layer )
    {
      relations.append( rel );
    }
  }

  return relations;
}

void QgsRelationManager::readProject( const QDomDocument &doc, QgsReadWriteContext &context )
{
  mRelations.clear();

  QDomNodeList nodes = doc.elementsByTagName( QStringLiteral( "relations" ) );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    QDomNodeList relationNodes = node.childNodes();
    int relCount = relationNodes.count();
    for ( int i = 0; i < relCount; ++i )
    {
      addRelation( QgsRelation::createFromXml( relationNodes.at( i ), context ) );
    }
  }
  else
  {
    QgsDebugMsg( "No relations data present in this document" );
  }

  emit relationsLoaded();
  emit changed();
}

void QgsRelationManager::writeProject( QDomDocument &doc )
{
  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );
  if ( !nl.count() )
  {
    QgsDebugMsg( "Unable to find qgis element in project file" );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one

  QDomElement relationsNode = doc.createElement( QStringLiteral( "relations" ) );
  qgisNode.appendChild( relationsNode );

  Q_FOREACH ( const QgsRelation &relation, mRelations )
  {
    relation.writeXml( relationsNode, doc );
  }
}

void QgsRelationManager::layersRemoved( const QStringList &layers )
{
  bool relationsChanged = false;
  Q_FOREACH ( const QString &layer, layers )
  {
    QMapIterator<QString, QgsRelation> it( mRelations );

    while ( it.hasNext() )
    {
      it.next();

      if ( it.value().referencedLayerId() == layer
           || it.value().referencingLayerId() == layer )
      {
        mRelations.remove( it.key() );
        relationsChanged = true;
      }
    }
  }
  if ( relationsChanged )
  {
    emit changed();
  }
}

static bool hasRelationWithEqualDefinition( const QList<QgsRelation> &existingRelations, const QgsRelation &relation )
{
  Q_FOREACH ( const QgsRelation &cur, existingRelations )
  {
    if ( cur.hasEqualDefinition( relation ) ) return true;
  }
  return false;
}

QList<QgsRelation> QgsRelationManager::discoverRelations( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers )
{
  QList<QgsRelation> result;
  Q_FOREACH ( const QgsVectorLayer *layer, layers )
  {
    Q_FOREACH ( const QgsRelation &relation, layer->dataProvider()->discoverRelations( layer, layers ) )
    {
      if ( !hasRelationWithEqualDefinition( existingRelations, relation ) )
      {
        result.append( relation );
      }
    }
  }
  return result;
}
