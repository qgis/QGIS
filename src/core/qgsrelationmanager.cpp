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

#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

QgsRelationManager::QgsRelationManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{
  if ( project )
  {
    // TODO: QGIS 4 remove: relations are now stored with the layer style
    connect( project, &QgsProject::readProjectWithContext, this, &QgsRelationManager::readProject );
    // TODO: QGIS 4 remove: relations are now stored with the layer style
    connect( project, &QgsProject::writeProject, this, &QgsRelationManager::writeProject );

    connect( project, &QgsProject::layersRemoved, this, &QgsRelationManager::layersRemoved );
  }
}

QgsRelationContext QgsRelationManager::context() const
{
  return QgsRelationContext( mProject );
}

void QgsRelationManager::setRelations( const QList<QgsRelation> &relations )
{
  mRelations.clear();
  for ( const QgsRelation &rel : std::as_const( relations ) )
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
  // Do not add relations to layers that do not exist
  if ( !( relation.referencingLayer() && relation.referencedLayer() ) )
    return;

  mRelations.insert( relation.id(), relation );
  if ( mProject )
  {
    mProject->setDirty( true );
  }
  emit changed();
}


void QgsRelationManager::updateRelationsStatus()
{
  for ( auto relationIt = mRelations.begin(); relationIt != mRelations.end(); ++relationIt )
  {
    relationIt->updateRelationStatus();
  }
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

  for ( const QgsRelation &rel : std::as_const( mRelations ) )
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

  for ( const QgsRelation &rel : std::as_const( mRelations ) )
  {
    if ( rel.referencingLayer() == layer )
    {
      if ( fieldIdx != -2 )
      {
        bool containsField = false;
        const auto constFieldPairs = rel.fieldPairs();
        for ( const QgsRelation::FieldPair &fp : constFieldPairs )
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

QList<QgsRelation> QgsRelationManager::referencedRelations( const QgsVectorLayer *layer ) const
{
  if ( !layer )
  {
    return mRelations.values();
  }

  QList<QgsRelation> relations;

  for ( const QgsRelation &rel : std::as_const( mRelations ) )
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
  mPolymorphicRelations.clear();

  QDomNodeList relationNodes = doc.elementsByTagName( QStringLiteral( "relations" ) );
  if ( relationNodes.count() )
  {
    QgsRelationContext relcontext( mProject );

    QDomNode node = relationNodes.item( 0 );
    QDomNodeList relationNodes = node.childNodes();
    int relCount = relationNodes.count();
    for ( int i = 0; i < relCount; ++i )
    {
      addRelation( QgsRelation::createFromXml( relationNodes.at( i ), context, relcontext ) );
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "No relations data present in this document" ) );
  }

  QDomNodeList polymorphicRelationNodes = doc.elementsByTagName( QStringLiteral( "polymorphicRelations" ) );
  if ( polymorphicRelationNodes.count() )
  {
    QgsRelationContext relcontext( mProject );

    QDomNode node = polymorphicRelationNodes.item( 0 );
    QDomNodeList relationNodes = node.childNodes();
    int relCount = relationNodes.count();
    for ( int i = 0; i < relCount; ++i )
    {
      addPolymorphicRelation( QgsPolymorphicRelation::createFromXml( relationNodes.at( i ), context, relcontext ) );
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "No polymorphic relations data present in this document" ), 3 );
  }

  emit relationsLoaded();
  emit changed();
}

void QgsRelationManager::writeProject( QDomDocument &doc )
{
  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );
  if ( !nl.count() )
  {
    QgsDebugMsg( QStringLiteral( "Unable to find qgis element in project file" ) );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one

  QDomElement relationsNode = doc.createElement( QStringLiteral( "relations" ) );
  qgisNode.appendChild( relationsNode );

  for ( const QgsRelation &relation : std::as_const( mRelations ) )
  {
    // the generated relations for polymorphic relations should be ignored,
    // they are generated every time when a polymorphic relation is added
    switch ( relation.type() )
    {
      case Qgis::RelationshipType::Generated:
        continue;
      case Qgis::RelationshipType::Normal:
        break;
    }

    relation.writeXml( relationsNode, doc );
  }

  QDomElement polymorphicRelationsNode = doc.createElement( QStringLiteral( "polymorphicRelations" ) );
  qgisNode.appendChild( polymorphicRelationsNode );

  for ( const QgsPolymorphicRelation &relation : std::as_const( mPolymorphicRelations ) )
  {
    relation.writeXml( polymorphicRelationsNode, doc );
  }
}

void QgsRelationManager::layersRemoved( const QStringList &layers )
{
  bool relationsChanged = false;
  for ( const QString &layer : std::as_const( layers ) )
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
  for ( const QgsRelation &cur : std::as_const( existingRelations ) )
  {
    if ( cur.hasEqualDefinition( relation ) ) return true;
  }
  return false;
}

QList<QgsRelation> QgsRelationManager::discoverRelations( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers )
{
  QList<QgsRelation> result;
  for ( const QgsVectorLayer *layer : std::as_const( layers ) )
  {
    if ( const QgsVectorDataProvider *provider = layer->dataProvider() )
    {
      const auto constDiscoverRelations = provider->discoverRelations( layer, layers );
      for ( const QgsRelation &relation : constDiscoverRelations )
      {
        if ( !hasRelationWithEqualDefinition( existingRelations, relation ) )
        {
          result.append( relation );
        }
      }
    }
  }
  return result;
}

QMap<QString, QgsPolymorphicRelation> QgsRelationManager::polymorphicRelations() const
{
  return mPolymorphicRelations;
}

QgsPolymorphicRelation QgsRelationManager::polymorphicRelation( const QString &polymorphicRelationId ) const
{
  return mPolymorphicRelations.value( polymorphicRelationId );
}

void QgsRelationManager::addPolymorphicRelation( const QgsPolymorphicRelation &polymorphicRelation )
{
  if ( !polymorphicRelation.referencingLayer() || polymorphicRelation.id().isNull() )
    return;

  mPolymorphicRelations.insert( polymorphicRelation.id(), polymorphicRelation );

  const QList<QgsRelation> generatedRelations = polymorphicRelation.generateRelations();
  for ( const QgsRelation &generatedRelation : generatedRelations )
    addRelation( generatedRelation );
}

void QgsRelationManager::removePolymorphicRelation( const QString &polymorphicRelationId )
{
  QgsPolymorphicRelation relation = mPolymorphicRelations.take( polymorphicRelationId );

  const QList<QgsRelation> generatedRelations = relation.generateRelations();
  for ( const QgsRelation &generatedRelation : generatedRelations )
    removeRelation( generatedRelation.id() );
}

void QgsRelationManager::setPolymorphicRelations( const QList<QgsPolymorphicRelation> &relations )
{
  const QList<QgsPolymorphicRelation> oldRelations = polymorphicRelations().values();
  for ( const QgsPolymorphicRelation &oldRelation : oldRelations )
    removePolymorphicRelation( oldRelation.id() );

  for ( const QgsPolymorphicRelation &newRelation : relations )
    addPolymorphicRelation( newRelation );
}
