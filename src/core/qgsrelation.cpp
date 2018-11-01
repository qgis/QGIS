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
#include "qgsrelation_p.h"

QgsRelation::QgsRelation()
  : d( new QgsRelationPrivate() )
{
}

QgsRelation::~QgsRelation() = default;

QgsRelation::QgsRelation( const QgsRelation &other )
  : d( other.d )
{
}

QgsRelation &QgsRelation::operator=( const QgsRelation &other )
{
  d = other.d;
  return *this;
}

QgsRelation QgsRelation::createFromXml( const QDomNode &node, QgsReadWriteContext &context )
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
  QString name = context.projectTranslator()->translate( QStringLiteral( "project:relations" ), elem.attribute( QStringLiteral( "name" ) ) );
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

  relation.d->mReferencingLayerId = referencingLayerId;
  relation.d->mReferencingLayer = qobject_cast<QgsVectorLayer *>( referencingLayer );
  relation.d->mReferencedLayerId = referencedLayerId;
  relation.d->mReferencedLayer = qobject_cast<QgsVectorLayer *>( referencedLayer );
  relation.d->mRelationId = id;
  relation.d->mRelationName = name;
  if ( strength == QLatin1String( "Composition" ) )
  {
    relation.d->mRelationStrength = RelationStrength::Composition;
  }
  else
  {
    relation.d->mRelationStrength = RelationStrength::Association;
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
  elem.setAttribute( QStringLiteral( "id" ), d->mRelationId );
  elem.setAttribute( QStringLiteral( "name" ), d->mRelationName );
  elem.setAttribute( QStringLiteral( "referencingLayer" ), d->mReferencingLayerId );
  elem.setAttribute( QStringLiteral( "referencedLayer" ), d->mReferencedLayerId );
  if ( d->mRelationStrength == RelationStrength::Composition )
  {
    elem.setAttribute( QStringLiteral( "strength" ), QStringLiteral( "Composition" ) );
  }
  else
  {
    elem.setAttribute( QStringLiteral( "strength" ), QStringLiteral( "Association" ) );
  }

  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    QDomElement referenceElem = doc.createElement( QStringLiteral( "fieldRef" ) );
    referenceElem.setAttribute( QStringLiteral( "referencingField" ), pair.first );
    referenceElem.setAttribute( QStringLiteral( "referencedField" ), pair.second );
    elem.appendChild( referenceElem );
  }

  node.appendChild( elem );
}

void QgsRelation::setId( const QString &id )
{
  d.detach();
  d->mRelationId = id;

  updateRelationStatus();
}

void QgsRelation::setName( const QString &name )
{
  d.detach();
  d->mRelationName = name;
}


void QgsRelation::setStrength( RelationStrength strength )
{
  d.detach();
  d->mRelationStrength = strength;
}

void QgsRelation::setReferencingLayer( const QString &id )
{
  d.detach();
  d->mReferencingLayerId = id;

  updateRelationStatus();
}

void QgsRelation::setReferencedLayer( const QString &id )
{
  d.detach();
  d->mReferencedLayerId = id;

  updateRelationStatus();
}

void QgsRelation::addFieldPair( const QString &referencingField, const QString &referencedField )
{
  d.detach();
  d->mFieldPairs << FieldPair( referencingField, referencedField );
  updateRelationStatus();
}

void QgsRelation::addFieldPair( const FieldPair &fieldPair )
{
  d.detach();
  d->mFieldPairs << fieldPair;
  updateRelationStatus();
}

QgsFeatureIterator QgsRelation::getRelatedFeatures( const QgsFeature &feature ) const
{
  return referencingLayer()->getFeatures( getRelatedFeaturesRequest( feature ) );
}

QgsFeatureRequest QgsRelation::getRelatedFeaturesRequest( const QgsFeature &feature ) const
{
  QString filter = getRelatedFeaturesFilter( feature );
  QgsDebugMsg( QStringLiteral( "Filter conditions: '%1'" ).arg( filter ) );

  QgsFeatureRequest myRequest;
  myRequest.setFilterExpression( filter );
  return myRequest;
}

QString QgsRelation::getRelatedFeaturesFilter( const QgsFeature &feature ) const
{
  QStringList conditions;

  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    QVariant val( feature.attribute( pair.referencedField() ) );
    conditions << QgsExpression::createFieldEqualityExpression( pair.referencingField(), val );
  }

  return conditions.join( QStringLiteral( " AND " ) );
}

QgsFeatureRequest QgsRelation::getReferencedFeatureRequest( const QgsAttributes &attributes ) const
{
  QStringList conditions;

  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    int referencingIdx = referencingLayer()->fields().indexFromName( pair.referencingField() );
    conditions << QgsExpression::createFieldEqualityExpression( pair.referencedField(), attributes.at( referencingIdx ) );
  }

  QgsFeatureRequest myRequest;

  QgsDebugMsg( QStringLiteral( "Filter conditions: '%1'" ).arg( conditions.join( " AND " ) ) );

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
  d->mReferencedLayer->getFeatures( request ).nextFeature( f );
  return f;
}

QString QgsRelation::name() const
{
  return d->mRelationName;
}

QgsRelation::RelationStrength QgsRelation::strength() const
{
  return d->mRelationStrength;
}

QString QgsRelation::id() const
{
  return d->mRelationId;
}

void QgsRelation::generateId()
{
  d->mRelationId = QStringLiteral( "%1_%2_%3_%4" )
                   .arg( referencingLayerId(),
                         d->mFieldPairs.at( 0 ).referencingField(),
                         referencedLayerId(),
                         d->mFieldPairs.at( 0 ).referencedField() );
  updateRelationStatus();
}

QString QgsRelation::referencingLayerId() const
{
  return d->mReferencingLayerId;
}

QgsVectorLayer *QgsRelation::referencingLayer() const
{
  return d->mReferencingLayer;
}

QString QgsRelation::referencedLayerId() const
{
  return d->mReferencedLayerId;
}

QgsVectorLayer *QgsRelation::referencedLayer() const
{
  return d->mReferencedLayer;
}

QList<QgsRelation::FieldPair> QgsRelation::fieldPairs() const
{
  return d->mFieldPairs;
}

QgsAttributeList QgsRelation::referencedFields() const
{
  QgsAttributeList attrs;

  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    attrs << d->mReferencedLayer->fields().lookupField( pair.second );
  }
  return attrs;
}

QgsAttributeList QgsRelation::referencingFields() const
{
  QgsAttributeList attrs;

  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    attrs << d->mReferencingLayer->fields().lookupField( pair.first );
  }
  return attrs;

}

bool QgsRelation::isValid() const
{
  return d->mValid && !d->mReferencingLayer.isNull() && !d->mReferencedLayer.isNull() && d->mReferencingLayer.data()->isValid() && d->mReferencedLayer.data()->isValid();
}

bool QgsRelation::hasEqualDefinition( const QgsRelation &other ) const
{
  return d->mReferencedLayerId == other.d->mReferencedLayerId && d->mReferencingLayerId == other.d->mReferencingLayerId && d->mFieldPairs == other.d->mFieldPairs;
}

QString QgsRelation::resolveReferencedField( const QString &referencingField ) const
{
  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    if ( pair.first == referencingField )
      return pair.second;
  }
  return QString();
}

QString QgsRelation::resolveReferencingField( const QString &referencedField ) const
{
  for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
  {
    if ( pair.second == referencedField )
      return pair.first;
  }
  return QString();
}

void QgsRelation::updateRelationStatus()
{
  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();

  d->mReferencingLayer = qobject_cast<QgsVectorLayer *>( mapLayers[d->mReferencingLayerId] );
  d->mReferencedLayer = qobject_cast<QgsVectorLayer *>( mapLayers[d->mReferencedLayerId] );

  d->mValid = true;

  if ( d->mRelationId.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Invalid relation: no ID" ) );
    d->mValid = false;
  }
  else
  {
    if ( !d->mReferencedLayer )
    {
      QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referenced layer does not exist. ID: %1" ).arg( d->mReferencedLayerId ), 4 );
      d->mValid = false;
    }
    else if ( !d->mReferencingLayer )
    {
      QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referencing layer does not exist. ID: %2" ).arg( d->mReferencingLayerId ), 4 );
      d->mValid = false;
    }
    else
    {
      if ( d->mFieldPairs.count() < 1 )
      {
        QgsDebugMsgLevel( QStringLiteral( "Invalid relation: no pair of field is specified." ), 4 );
        d->mValid = false;
      }

      for ( const FieldPair &pair : qgis::as_const( d->mFieldPairs ) )
      {
        if ( -1 == d->mReferencingLayer->fields().lookupField( pair.first ) )
        {
          QgsDebugMsg( QStringLiteral( "Invalid relation: field %1 does not exist in referencing layer %2" ).arg( pair.first, d->mReferencingLayer->name() ) );
          d->mValid = false;
          break;
        }
        else if ( -1 == d->mReferencedLayer->fields().lookupField( pair.second ) )
        {
          QgsDebugMsg( QStringLiteral( "Invalid relation: field %1 does not exist in referencedg layer %2" ).arg( pair.second, d->mReferencedLayer->name() ) );
          d->mValid = false;
          break;
        }
      }
    }

  }
}
