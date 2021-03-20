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


QgsWeakRelation::QgsWeakRelation( const QString &relationId, const QString &relationName, const QgsRelation::RelationStrength strength,
                                  const QString &referencingLayerId, const QString &referencingLayerName, const QString &referencingLayerSource, const QString &referencingLayerProviderKey,
                                  const QString &referencedLayerId, const QString &referencedLayerName, const QString &referencedLayerSource, const QString &referencedLayerProviderKey,
                                  const QList<QgsRelation::FieldPair> &fieldPairs )
  : mReferencingLayer( referencingLayerId, referencingLayerName, referencingLayerSource, referencingLayerProviderKey )
  , mReferencedLayer( referencedLayerId, referencedLayerName, referencedLayerSource, referencedLayerProviderKey )
  , mRelationId( relationId )
  , mRelationName( relationName )
  , mStrength( strength )
  , mFieldPairs( fieldPairs )
{
}

QgsRelation QgsWeakRelation::resolvedRelation( const QgsProject *project, QgsVectorLayerRef::MatchType matchType ) const
{
  QgsRelation relation;
  relation.setId( mRelationId );
  relation.setName( mRelationName );
  relation.setStrength( mStrength );
  QgsVectorLayerRef referencedLayerRef { mReferencedLayer };
  QgsMapLayer *referencedLayer { referencedLayerRef.resolveWeakly( project, matchType ) };
  if ( referencedLayer )
  {
    relation.setReferencedLayer( referencedLayer->id() );
  }
  QgsVectorLayerRef referencingLayerRef { mReferencingLayer };
  QgsMapLayer *referencingLayer { referencingLayerRef.resolveWeakly( project, matchType ) };
  if ( referencingLayer )
  {
    relation.setReferencingLayer( referencingLayer->id() );
  }
  for ( const auto &fp : std::as_const( mFieldPairs ) )
  {
    relation.addFieldPair( fp );
  }
  return relation;
}

QgsVectorLayerRef QgsWeakRelation::referencingLayer() const
{
  return mReferencingLayer;
}

QgsVectorLayerRef QgsWeakRelation::referencedLayer() const
{
  return mReferencedLayer;
}

QgsRelation::RelationStrength QgsWeakRelation::strength() const
{
  return mStrength;
}

QList<QgsRelation::FieldPair> QgsWeakRelation::fieldPairs() const
{
  return mFieldPairs;
}

QgsWeakRelation QgsWeakRelation::readXml( const QgsVectorLayer *layer, WeakRelationType type, const QDomNode &node,  const QgsPathResolver resolver )
{
  QDomElement relationElement = node.toElement();

  if ( relationElement.tagName() != QLatin1String( "relation" ) )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Cannot create relation. Unexpected tag '%1'" ).arg( relationElement.tagName() ) );
  }

  QList<QgsRelation::FieldPair> fieldPairs;
  const QDomNodeList fieldPairNodes { relationElement.elementsByTagName( QStringLiteral( "fieldRef" ) ) };
  for ( int j = 0; j < fieldPairNodes.length(); ++j )
  {
    const QDomElement fieldPairElement = fieldPairNodes.at( j ).toElement();
    fieldPairs.push_back( { fieldPairElement.attribute( QStringLiteral( "referencingField" ) ),
                            fieldPairElement.attribute( QStringLiteral( "referencedField" ) )
                          } );
  }

  switch ( type )
  {
    case Referencing:
      return QgsWeakRelation { relationElement.attribute( QStringLiteral( "id" ) ),
                               relationElement.attribute( QStringLiteral( "name" ) ),
                               static_cast<QgsRelation::RelationStrength>( relationElement.attribute( QStringLiteral( "strength" ) ).toInt() ),
                               // Referencing
                               layer->id(),
                               layer->name(),
                               resolver.writePath( layer->publicSource() ),
                               layer->providerType(),
                               // Referenced
                               relationElement.attribute( QStringLiteral( "layerId" ) ),
                               relationElement.attribute( QStringLiteral( "layerName" ) ),
                               relationElement.attribute( QStringLiteral( "dataSource" ) ),
                               relationElement.attribute( QStringLiteral( "providerKey" ) ),
                               fieldPairs
                           };
    case Referenced:
      return QgsWeakRelation { relationElement.attribute( QStringLiteral( "id" ) ),
                               relationElement.attribute( QStringLiteral( "name" ) ),
                               static_cast<QgsRelation::RelationStrength>( relationElement.attribute( QStringLiteral( "strength" ) ).toInt() ),
                               // Referencing
                               relationElement.attribute( QStringLiteral( "layerId" ) ),
                               relationElement.attribute( QStringLiteral( "layerName" ) ),
                               relationElement.attribute( QStringLiteral( "dataSource" ) ),
                               relationElement.attribute( QStringLiteral( "providerKey" ) ),
                               // Referenced
                               layer->id(),
                               layer->name(),
                               resolver.writePath( layer->publicSource() ),
                               layer->providerType(),
                               fieldPairs
                           };
  }
  // avoid build warnings
  return QgsWeakRelation( QString(), QString(), QgsRelation::RelationStrength::Association, QString(), QString(), QString(),
                          QString(), QString(), QString(), QString(), QString(), QList< QgsRelation::FieldPair >() );
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
