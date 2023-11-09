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

#include <QApplication>
#include "qgsweakrelation.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

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
      relationLeft.setId( mRelationId + QStringLiteral( "_forward" ) );
      relationLeft.setName( mRelationName + QStringLiteral( "_forward" ) );
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
      relationRight.setId( mRelationId + QStringLiteral( "_backward" ) );
      relationRight.setName( mRelationName + QStringLiteral( "_backward" ) );
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

QString QgsWeakRelation::referencingLayerName() const
{
  if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( mReferencingLayer.provider ) )
  {
    return metadata->decodeUri( mReferencingLayer.source ).value( QStringLiteral( "layerName" ) ).toString();
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
    return metadata->decodeUri( mReferencedLayer.source ).value( QStringLiteral( "layerName" ) ).toString();
  }
  return QString();
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
    return metadata->decodeUri( mMappingTable.source ).value( QStringLiteral( "layerName" ) ).toString();
  }
  return QString();
}

Qgis::RelationshipStrength QgsWeakRelation::strength() const
{
  return mStrength;
}

QgsWeakRelation QgsWeakRelation::readXml( const QgsVectorLayer *layer, WeakRelationType type, const QDomNode &node,  const QgsPathResolver resolver )
{
  QDomElement relationElement = node.toElement();

  if ( relationElement.tagName() != QLatin1String( "relation" ) )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Cannot create relation. Unexpected tag '%1'" ).arg( relationElement.tagName() ) );
  }

  QStringList referencingFields;
  QStringList referencedFields;
  const QDomNodeList fieldPairNodes { relationElement.elementsByTagName( QStringLiteral( "fieldRef" ) ) };
  for ( int j = 0; j < fieldPairNodes.length(); ++j )
  {
    const QDomElement fieldPairElement = fieldPairNodes.at( j ).toElement();
    referencingFields.push_back( fieldPairElement.attribute( QStringLiteral( "referencingField" ) ) );
    referencedFields.push_back( fieldPairElement.attribute( QStringLiteral( "referencedField" ) ) );
  }

  switch ( type )
  {
    case Referencing:
    {
      QgsWeakRelation rel{ relationElement.attribute( QStringLiteral( "id" ) ),
                           relationElement.attribute( QStringLiteral( "name" ) ),
                           static_cast<Qgis::RelationshipStrength>( relationElement.attribute( QStringLiteral( "strength" ) ).toInt() ),
                           // Referencing
                           layer->id(),
                           layer->name(),
                           resolver.writePath( layer->publicSource() ),
                           layer->providerType(),
                           // Referenced
                           relationElement.attribute( QStringLiteral( "layerId" ) ),
                           relationElement.attribute( QStringLiteral( "layerName" ) ),
                           relationElement.attribute( QStringLiteral( "dataSource" ) ),
                           relationElement.attribute( QStringLiteral( "providerKey" ) )
                         };
      rel.setReferencedLayerFields( referencedFields );
      rel.setReferencingLayerFields( referencingFields );
      return rel;
    }

    case Referenced:
    {
      QgsWeakRelation rel{ relationElement.attribute( QStringLiteral( "id" ) ),
                           relationElement.attribute( QStringLiteral( "name" ) ),
                           static_cast<Qgis::RelationshipStrength>( relationElement.attribute( QStringLiteral( "strength" ) ).toInt() ),
                           // Referencing
                           relationElement.attribute( QStringLiteral( "layerId" ) ),
                           relationElement.attribute( QStringLiteral( "layerName" ) ),
                           relationElement.attribute( QStringLiteral( "dataSource" ) ),
                           relationElement.attribute( QStringLiteral( "providerKey" ) ),
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

  const QgsPathResolver resolver { QgsProject::instance()->pathResolver() };

  relation.writeXml( node, doc );
  QDomNodeList relationsNodeList = node.toElement().elementsByTagName( QStringLiteral( "relation" ) );
  QDomElement relationElement;

  for ( int i = 0; i < relationsNodeList.size(); ++i )
  {
    relationElement = relationsNodeList.at( i ).toElement();
    if ( relationElement.hasAttribute( QStringLiteral( "id" ) ) && relationElement.attribute( QStringLiteral( "id" ) ) == relation.id() )
    {
      switch ( type )
      {
        case Referencing:
          // if the layer is the referencing one, we save the referenced layer info
          relationElement.setAttribute( QStringLiteral( "layerId" ), relation.referencedLayer()->id() );
          relationElement.setAttribute( QStringLiteral( "layerName" ), relation.referencedLayer()->name() );
          relationElement.setAttribute( QStringLiteral( "dataSource" ), resolver.writePath( relation.referencedLayer()->publicSource() ) );
          relationElement.setAttribute( QStringLiteral( "providerKey" ), relation.referencedLayer()->providerType() );
          break;

        case Referenced:
          // if the layer is the referenced one, we save the referencing layer info
          relationElement.setAttribute( QStringLiteral( "layerId" ), relation.referencingLayer()->id() );
          relationElement.setAttribute( QStringLiteral( "layerName" ), relation.referencingLayer()->name() );
          relationElement.setAttribute( QStringLiteral( "dataSource" ), resolver.writePath( relation.referencingLayer()->publicSource() ) );
          relationElement.setAttribute( QStringLiteral( "providerKey" ), relation.referencingLayer()->providerType() );
          break;
      }
    }
  }
}
