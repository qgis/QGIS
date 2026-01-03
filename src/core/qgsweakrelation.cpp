/***************************************************************************
  qgsweakrelation.cpp - QgsWeakRelation

 ---------------------
 begin                : 5.12.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsweakrelation.h"

#include "qgslogger.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include <QApplication>

QgsWeakRelation::QgsWeakRelation() = default;

QgsWeakRelation::QgsWeakRelation( const QString &relationId, const QString &relationName, const Qgis::RelationshipStrength strength,
                                  const QString &referencingLayerId, const QString &referencingLayerName, const QString &referencingLayerSource, const QString &referencingLayerProviderKey,
                                  const QString &referencedLayerId, const QString &referencedLayerName, const QString &referencedLayerSource, const QString &referencedLayerProviderKey )
  : mReferencingLayer( referencingLayerId, referencingLayerName, referencingLayerSource, referencingLayerProviderKey )
  , mReferencedLayer( referencedLayerId, referencedLayerName, referencedLayerSource, referencedLayerProviderKey )
  , mRelationId( relationId )
  , mRelationName( relationName )
  , mStrength( strength )
{
}

QList< QgsRelation > QgsWeakRelation::resolvedRelations( const QgsProject *project, QgsVectorLayerRef::MatchType matchType ) const
{
  QList< QgsRelation > res;

  switch ( mCardinality )
  {
    case Qgis::RelationshipCardinality::OneToOne:
    case Qgis::RelationshipCardinality::OneToMany:
    case Qgis::RelationshipCardinality::ManyToOne:
    {

      QgsRelation relation;
      relation.setId( mRelationId );
      relation.setName( mRelationName );
      relation.setStrength( mStrength );
      QgsVectorLayerRef referencedLayerRef = mReferencedLayer;
      QgsMapLayer *referencedLayer = referencedLayerRef.resolveWeakly( project, matchType );
      if ( referencedLayer )
      {
        relation.setReferencedLayer( referencedLayer->id() );
      }
      QgsVectorLayerRef referencingLayerRef = mReferencingLayer;
      QgsMapLayer *referencingLayer = referencingLayerRef.resolveWeakly( project, matchType );
      if ( referencingLayer )
      {
        relation.setReferencingLayer( referencingLayer->id() );
      }

      for ( int i = 0 ; i < std::min( mReferencingLayerFields.size(), mReferencedLayerFields.size() ); ++i )
      {
        relation.addFieldPair( mReferencingLayerFields.at( i ), mReferencedLayerFields.at( i ) );
      }

      res.push_back( relation );
      break;
    }

    case Qgis::RelationshipCardinality::ManyToMany:
    {
      // a many-to-many relationship is represented by two QgsRelations
      QgsRelation relationLeft;
      relationLeft.setId( mRelationId + u"_forward"_s );
      relationLeft.setName( mRelationName + u"_forward"_s );
      relationLeft.setStrength( mStrength );
      QgsVectorLayerRef referencedLayerRef = mReferencedLayer;
      QgsMapLayer *referencedLayer = referencedLayerRef.resolveWeakly( project, matchType );
      if ( referencedLayer )
      {
        relationLeft.setReferencedLayer( referencedLayer->id() );
      }

      QgsVectorLayerRef mappingTableRef = mMappingTable;
      QgsMapLayer *mappingLayer = mappingTableRef.resolveWeakly( project, matchType );
      if ( mappingLayer )
      {
        relationLeft.setReferencingLayer( mappingLayer->id() );
      }

      for ( int i = 0 ; i < std::min( mMappingReferencedLayerFields.size(), mReferencedLayerFields.size() ); ++i )
      {
        relationLeft.addFieldPair( mMappingReferencedLayerFields.at( i ), mReferencedLayerFields.at( i ) );
      }

      res.push_back( relationLeft );

      QgsRelation relationRight;
      relationRight.setId( mRelationId + u"_backward"_s );
      relationRight.setName( mRelationName + u"_backward"_s );
      relationRight.setStrength( mStrength );

      QgsVectorLayerRef referencingLayerRef = mReferencingLayer;
      QgsMapLayer *referencingLayer = referencingLayerRef.resolveWeakly( project, matchType );
      if ( referencingLayer )
      {
        relationRight.setReferencedLayer( referencingLayer->id() );
      }
      if ( mappingLayer )
      {
        relationRight.setReferencingLayer( mappingLayer->id() );
      }

      for ( int i = 0 ; i < std::min( mMappingReferencingLayerFields.size(), mReferencingLayerFields.size() ); ++i )
      {
        relationRight.addFieldPair( mMappingReferencingLayerFields.at( i ), mReferencingLayerFields.at( i ) );
      }

      res.push_back( relationRight );
      break;
    }
  }

  return res;
}

QgsVectorLayerRef QgsWeakRelation::referencingLayer() const
{
  return mReferencingLayer;
}

QString QgsWeakRelation::referencingLayerSource() const
{
  return mReferencingLayer.source;
}

QString QgsWeakRelation::referencingLayerProvider() const
{
  return mReferencingLayer.provider;
}

void QgsWeakRelation::setReferencingLayer( const QString &sourceUri, const QString &provider )
{
  mReferencingLayer.source = sourceUri;
  mReferencingLayer.provider = provider;
}

QString QgsWeakRelation::referencingLayerName() const
{
  if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( mReferencingLayer.provider ) )
  {
    return metadata->decodeUri( mReferencingLayer.source ).value( u"layerName"_s ).toString();
  }
  return QString();
}

QgsVectorLayerRef QgsWeakRelation::referencedLayer() const
{
  return mReferencedLayer;
}

QString QgsWeakRelation::referencedLayerSource() const
{
  return mReferencedLayer.source;
}

QString QgsWeakRelation::referencedLayerProvider() const
{
  return mReferencedLayer.provider;
}

QString QgsWeakRelation::referencedLayerName() const
{
  if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( mReferencedLayer.provider ) )
  {
    return metadata->decodeUri( mReferencedLayer.source ).value( u"layerName"_s ).toString();
  }
  return QString();
}

void QgsWeakRelation::setReferencedLayer( const QString &sourceUri, const QString &provider )
{
  mReferencedLayer.source = sourceUri;
  mReferencedLayer.provider = provider;
}

QgsVectorLayerRef QgsWeakRelation::mappingTable() const
{
  return mMappingTable;
}

void QgsWeakRelation::setMappingTable( const QgsVectorLayerRef &table )
{
  mMappingTable = table;
}

QString QgsWeakRelation::mappingTableSource() const
{
  return mMappingTable.source;
}

QString QgsWeakRelation::mappingTableProvider() const
{
  return mMappingTable.provider;
}

QString QgsWeakRelation::mappingTableName() const
{
  if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( mMappingTable.provider ) )
  {
    return metadata->decodeUri( mMappingTable.source ).value( u"layerName"_s ).toString();
  }
  return QString();
}

void QgsWeakRelation::setMappingTable( const QString &sourceUri, const QString &provider )
{
  mMappingTable.source = sourceUri;
  mMappingTable.provider = provider;
}

Qgis::RelationshipStrength QgsWeakRelation::strength() const
{
  return mStrength;
}

QgsWeakRelation QgsWeakRelation::readXml( const QgsVectorLayer *layer, WeakRelationType type, const QDomNode &node,  const QgsPathResolver resolver )
{
  QDomElement relationElement = node.toElement();

  if ( relationElement.tagName() != "relation"_L1 )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Cannot create relation. Unexpected tag '%1'" ).arg( relationElement.tagName() ) );
  }

  QStringList referencingFields;
  QStringList referencedFields;
  const QDomNodeList fieldPairNodes { relationElement.elementsByTagName( u"fieldRef"_s ) };
  for ( int j = 0; j < fieldPairNodes.length(); ++j )
  {
    const QDomElement fieldPairElement = fieldPairNodes.at( j ).toElement();
    referencingFields.push_back( fieldPairElement.attribute( u"referencingField"_s ) );
    referencedFields.push_back( fieldPairElement.attribute( u"referencedField"_s ) );
  }

  switch ( type )
  {
    case Referencing:
    {
      QgsWeakRelation rel{ relationElement.attribute( u"id"_s ),
                           relationElement.attribute( u"name"_s ),
                           static_cast<Qgis::RelationshipStrength>( relationElement.attribute( u"strength"_s ).toInt() ),
                           // Referencing
                           layer->id(),
                           layer->name(),
                           resolver.writePath( layer->publicSource() ),
                           layer->providerType(),
                           // Referenced
                           relationElement.attribute( u"layerId"_s ),
                           relationElement.attribute( u"layerName"_s ),
                           relationElement.attribute( u"dataSource"_s ),
                           relationElement.attribute( u"providerKey"_s )
                         };
      rel.setReferencedLayerFields( referencedFields );
      rel.setReferencingLayerFields( referencingFields );
      return rel;
    }

    case Referenced:
    {
      QgsWeakRelation rel{ relationElement.attribute( u"id"_s ),
                           relationElement.attribute( u"name"_s ),
                           static_cast<Qgis::RelationshipStrength>( relationElement.attribute( u"strength"_s ).toInt() ),
                           // Referencing
                           relationElement.attribute( u"layerId"_s ),
                           relationElement.attribute( u"layerName"_s ),
                           relationElement.attribute( u"dataSource"_s ),
                           relationElement.attribute( u"providerKey"_s ),
                           // Referenced
                           layer->id(),
                           layer->name(),
                           resolver.writePath( layer->publicSource() ),
                           layer->providerType()
                         };
      rel.setReferencedLayerFields( referencedFields );
      rel.setReferencingLayerFields( referencingFields );
      return rel;
    }
  }

  // avoid build warnings
  return QgsWeakRelation();
}

void QgsWeakRelation::writeXml( const QgsVectorLayer *layer, WeakRelationType type, const QgsRelation &relation, QDomNode &node, QDomDocument &doc )
{
  if ( !layer )
    return;

  if ( layer != relation.referencingLayer() && layer != relation.referencedLayer() )
    return;

  QgsPathResolver resolver;
  if ( QgsProject *project = layer->project() )
    resolver = project->pathResolver();

  relation.writeXml( node, doc );
  QDomNodeList relationsNodeList = node.toElement().elementsByTagName( u"relation"_s );
  QDomElement relationElement;

  for ( int i = 0; i < relationsNodeList.size(); ++i )
  {
    relationElement = relationsNodeList.at( i ).toElement();
    if ( relationElement.hasAttribute( u"id"_s ) && relationElement.attribute( u"id"_s ) == relation.id() )
    {
      switch ( type )
      {
        case Referencing:
          // if the layer is the referencing one, we save the referenced layer info
          relationElement.setAttribute( u"layerId"_s, relation.referencedLayer()->id() );
          relationElement.setAttribute( u"layerName"_s, relation.referencedLayer()->name() );
          relationElement.setAttribute( u"dataSource"_s, resolver.writePath( relation.referencedLayer()->publicSource() ) );
          relationElement.setAttribute( u"providerKey"_s, relation.referencedLayer()->providerType() );
          break;

        case Referenced:
          // if the layer is the referenced one, we save the referencing layer info
          relationElement.setAttribute( u"layerId"_s, relation.referencingLayer()->id() );
          relationElement.setAttribute( u"layerName"_s, relation.referencingLayer()->name() );
          relationElement.setAttribute( u"dataSource"_s, resolver.writePath( relation.referencingLayer()->publicSource() ) );
          relationElement.setAttribute( u"providerKey"_s, relation.referencingLayer()->providerType() );
          break;
      }
    }
  }
}
