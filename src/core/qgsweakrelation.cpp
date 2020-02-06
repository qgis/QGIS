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
  for ( const auto &fp : qgis::as_const( mFieldPairs ) )
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
