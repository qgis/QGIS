/***************************************************************************
    qgsrelation.cpp
     --------------------------------------
    Date                 : 29.4.2013
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

#include "qgsrelation.h"

#include "qgsapplication.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

QgsRelation QgsRelation::createFromXml( const QDomNode &node )
{
  QDomElement elem = node.toElement();

  if ( elem.tagName() != QLatin1String( "relation" ) )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Cannot create relation. Unexpected tag '%1'" ).arg( elem.tagName() ) );
  }

  QgsRelation relation;

  QString referencingLayerId = elem.attribute( QStringLiteral( "referencingLayer" ) );
  QString referencedLayerId = elem.attribute( QStringLiteral( "referencedLayer" ) );
  QString id = elem.attribute( QStringLiteral( "id" ) );
  QString name = QgsProject::instance()->translate( QStringLiteral( "project:relations" ), elem.attribute( QStringLiteral( "name" ) ) );
  QString strength = elem.attribute( QStringLiteral( "strength" ) );

  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();

  QgsMapLayer *referencingLayer = mapLayers[referencingLayerId];
  QgsMapLayer *referencedLayer = mapLayers[referencedLayerId];

  if ( !referencingLayer )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which does not exist." ).arg( referencingLayerId ) );
  }
  else if ( QgsMapLayer::VectorLayer  != referencingLayer->type() )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which is not of type VectorLayer." ).arg( referencingLayerId ) );
  }

  if ( !referencedLayer )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which does not exist." ).arg( referencedLayerId ) );
  }
  else if ( QgsMapLayer::VectorLayer  != referencedLayer->type() )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which is not of type VectorLayer." ).arg( referencedLayerId ) );
  }

  relation.mReferencingLayerId = referencingLayerId;
  relation.mReferencingLayer = qobject_cast<QgsVectorLayer *>( referencingLayer );
  relation.mReferencedLayerId = referencedLayerId;
  relation.mReferencedLayer = qobject_cast<QgsVectorLayer *>( referencedLayer );
  relation.mRelationId = id;
  relation.mRelationName = name;
  if ( strength == "Composition" )
  {
    relation.mRelationStrength = RelationStrength::Composition;
  }
  else
  {
    relation.mRelationStrength = RelationStrength::Association;
  }

  QDomNodeList references = elem.elementsByTagName( QStringLiteral( "fieldRef" ) );
  for ( int i = 0; i < references.size(); ++i )
  {
    QDomElement refEl = references.at( i ).toElement();

    QString referencingField = refEl.attribute( QStringLiteral( "referencingField" ) );
    QString referencedField = refEl.attribute( QStringLiteral( "referencedField" ) );

    relation.addFieldPair( referencingField, referencedField );
  }

  relation.updateRelationStatus();

  return relation;
}

void QgsRelation::writeXml( QDomNode &node, QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "relation" ) );
  elem.setAttribute( QStringLiteral( "id" ), mRelationId );
  elem.setAttribute( QStringLiteral( "name" ), mRelationName );
  elem.setAttribute( QStringLiteral( "referencingLayer" ), mReferencingLayerId );
  elem.setAttribute( QStringLiteral( "referencedLayer" ), mReferencedLayerId );
  if ( mRelationStrength == RelationStrength::Composition )
  {
    elem.setAttribute( QStringLiteral( "strength" ), QStringLiteral( "Composition" ) );
  }
  else
  {
    elem.setAttribute( QStringLiteral( "strength" ), QStringLiteral( "Association" ) );
  }

  Q_FOREACH ( const FieldPair &fields, mFieldPairs )
  {
    QDomElement referenceElem = doc.createElement( QStringLiteral( "fieldRef" ) );
    referenceElem.setAttribute( QStringLiteral( "referencingField" ), fields.first );
    referenceElem.setAttribute( QStringLiteral( "referencedField" ), fields.second );
    elem.appendChild( referenceElem );
  }

  node.appendChild( elem );
}

void QgsRelation::setId( const QString &id )
{
  mRelationId = id;

  updateRelationStatus();
}

void QgsRelation::setName( const QString &name )
{
  mRelationName = name;
}


void QgsRelation::setStrength( RelationStrength strength )
{
  mRelationStrength = strength;
}

void QgsRelation::setReferencingLayer( const QString &id )
{
  mReferencingLayerId = id;

  updateRelationStatus();
}

void QgsRelation::setReferencedLayer( const QString &id )
{
  mReferencedLayerId = id;

  updateRelationStatus();
}

void QgsRelation::addFieldPair( const QString &referencingField, const QString &referencedField )
{
  mFieldPairs << FieldPair( referencingField, referencedField );
  updateRelationStatus();
}

void QgsRelation::addFieldPair( const FieldPair &fieldPair )
{
  mFieldPairs << fieldPair;
  updateRelationStatus();
}

QgsFeatureIterator QgsRelation::getRelatedFeatures( const QgsFeature &feature ) const
{
  return referencingLayer()->getFeatures( getRelatedFeaturesRequest( feature ) );
}

QgsFeatureRequest QgsRelation::getRelatedFeaturesRequest( const QgsFeature &feature ) const
{
  QString filter = getRelatedFeaturesFilter( feature );
  QgsDebugMsg( QString( "Filter conditions: '%1'" ).arg( filter ) );

  QgsFeatureRequest myRequest;
  myRequest.setFilterExpression( filter );
  return myRequest;
}

QString QgsRelation::getRelatedFeaturesFilter( const QgsFeature &feature ) const
{
  QStringList conditions;

  Q_FOREACH ( const QgsRelation::FieldPair &fieldPair, mFieldPairs )
  {
    QVariant val( feature.attribute( fieldPair.referencedField() ) );
    conditions << QgsExpression::createFieldEqualityExpression( fieldPair.referencingField(), val );
  }

  return conditions.join( QStringLiteral( " AND " ) );
}

QgsFeatureRequest QgsRelation::getReferencedFeatureRequest( const QgsAttributes &attributes ) const
{
  QStringList conditions;

  Q_FOREACH ( const QgsRelation::FieldPair &fieldPair, mFieldPairs )
  {
    int referencingIdx = referencingLayer()->fields().indexFromName( fieldPair.referencingField() );
    conditions << QgsExpression::createFieldEqualityExpression( fieldPair.referencedField(), attributes.at( referencingIdx ) );
  }

  QgsFeatureRequest myRequest;

  QgsDebugMsg( QString( "Filter conditions: '%1'" ).arg( conditions.join( " AND " ) ) );

  myRequest.setFilterExpression( conditions.join( QStringLiteral( " AND " ) ) );

  return myRequest;
}

QgsFeatureRequest QgsRelation::getReferencedFeatureRequest( const QgsFeature &feature ) const
{
  return getReferencedFeatureRequest( feature.attributes() );
}

QgsFeature QgsRelation::getReferencedFeature( const QgsFeature &feature ) const
{
  QgsFeatureRequest request = getReferencedFeatureRequest( feature );

  QgsFeature f;
  mReferencedLayer->getFeatures( request ).nextFeature( f );
  return f;
}

QString QgsRelation::name() const
{
  return mRelationName;
}

QgsRelation::RelationStrength QgsRelation::strength() const
{
  return mRelationStrength;
}

QString QgsRelation::id() const
{
  return mRelationId;
}

void QgsRelation::generateId()
{
  mRelationId = QStringLiteral( "%1_%2_%3_%4" )
                .arg( referencingLayerId(),
                      mFieldPairs.at( 0 ).referencingField(),
                      referencedLayerId(),
                      mFieldPairs.at( 0 ).referencedField() );
  updateRelationStatus();
}

QString QgsRelation::referencingLayerId() const
{
  return mReferencingLayerId;
}

QgsVectorLayer *QgsRelation::referencingLayer() const
{
  return mReferencingLayer;
}

QString QgsRelation::referencedLayerId() const
{
  return mReferencedLayerId;
}

QgsVectorLayer *QgsRelation::referencedLayer() const
{
  return mReferencedLayer;
}

QList<QgsRelation::FieldPair> QgsRelation::fieldPairs() const
{
  return mFieldPairs;
}

QgsAttributeList QgsRelation::referencedFields() const
{
  QgsAttributeList attrs;

  Q_FOREACH ( const FieldPair &pair, mFieldPairs )
  {
    attrs << mReferencedLayer->fields().lookupField( pair.second );
  }
  return attrs;
}

QgsAttributeList QgsRelation::referencingFields() const
{
  QgsAttributeList attrs;

  Q_FOREACH ( const FieldPair &pair, mFieldPairs )
  {
    attrs << mReferencingLayer->fields().lookupField( pair.first );
  }
  return attrs;

}

bool QgsRelation::isValid() const
{
  return mValid;
}

bool QgsRelation::hasEqualDefinition( const QgsRelation &other ) const
{
  return mReferencedLayerId == other.mReferencedLayerId && mReferencingLayerId == other.mReferencingLayerId && mFieldPairs == other.mFieldPairs;
}

QString QgsRelation::resolveReferencedField( const QString &referencingField ) const
{
  Q_FOREACH ( const FieldPair &pair, mFieldPairs )
  {
    if ( pair.first == referencingField )
      return pair.second;
  }
  return QString();
}

QString QgsRelation::resolveReferencingField( const QString &referencedField ) const
{
  Q_FOREACH ( const FieldPair &pair, mFieldPairs )
  {
    if ( pair.second == referencedField )
      return pair.first;
  }
  return QString();
}

void QgsRelation::updateRelationStatus()
{
  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();

  mReferencingLayer = qobject_cast<QgsVectorLayer *>( mapLayers[mReferencingLayerId] );
  mReferencedLayer = qobject_cast<QgsVectorLayer *>( mapLayers[mReferencedLayerId] );

  mValid = true;

  if ( mRelationId.isEmpty() )
  {
    QgsDebugMsg( "Invalid relation: no ID" );
    mValid = false;
  }
  else
  {
    if ( !mReferencedLayer )
    {
      QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referenced layer does not exist. ID: %1" ).arg( mReferencedLayerId ), 4 );
      mValid = false;
    }
    else if ( !mReferencingLayer )
    {
      QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referencing layer does not exist. ID: %2" ).arg( mReferencingLayerId ), 4 );
      mValid = false;
    }
    else
    {
      if ( mFieldPairs.count() < 1 )
      {
        QgsDebugMsgLevel( "Invalid relation: no pair of field is specified.", 4 );
        mValid = false;
      }

      Q_FOREACH ( const FieldPair &fieldPair, mFieldPairs )
      {
        if ( -1 == mReferencingLayer->fields().lookupField( fieldPair.first ) )
        {
          QgsDebugMsg( QStringLiteral( "Invalid relation: field %1 does not exist in referencing layer %2" ).arg( fieldPair.first, mReferencingLayer->name() ) );
          mValid = false;
          break;
        }
        else if ( -1 == mReferencedLayer->fields().lookupField( fieldPair.second ) )
        {
          QgsDebugMsg( QStringLiteral( "Invalid relation: field %1 does not exist in referencedg layer %2" ).arg( fieldPair.second, mReferencedLayer->name() ) );
          mValid = false;
          break;
        }
      }
    }

  }
}
