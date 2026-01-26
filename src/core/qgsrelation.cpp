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

#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgspolymorphicrelation.h"
#include "qgsproject.h"
#include "qgsrelation_p.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

#include <QApplication>

#include "moc_qgsrelation.cpp"

QgsRelation::QgsRelation()
  : d( new QgsRelationPrivate() )
{
}

QgsRelation::QgsRelation( const QgsRelationContext &context )
  : d( new QgsRelationPrivate() )
  , mContext( context )
{
}

QgsRelation::~QgsRelation() = default;

QgsRelation::QgsRelation( const QgsRelation &other )
  : d( other.d )
  , mContext( other.mContext )
{
}

QgsRelation::QgsRelation( QgsRelation &&other )
  : d( std::move( other.d ) )
  , mContext( std::move( other.mContext ) )
{}

QgsRelation &QgsRelation::operator=( const QgsRelation &other )
{
  if ( &other == this )
    return *this;

  d = other.d;
  mContext = other.mContext;
  return *this;
}

QgsRelation &QgsRelation::operator=( QgsRelation &&other )
{
  if ( &other == this )
    return *this;

  d = std::move( other.d );
  mContext = std::move( other.mContext );
  return *this;
}

QgsRelation QgsRelation::createFromXml( const QDomNode &node, QgsReadWriteContext &context,  const QgsRelationContext &relationContext )
{
  QDomElement elem = node.toElement();

  if ( elem.tagName() != "relation"_L1 )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Cannot create relation. Unexpected tag '%1'" ).arg( elem.tagName() ) );
  }

  QgsRelation relation( relationContext );

  QString referencingLayerId = elem.attribute( u"referencingLayer"_s );
  QString referencedLayerId = elem.attribute( u"referencedLayer"_s );
  QString id = elem.attribute( u"id"_s );
  QString name = context.projectTranslator()->translate( u"project:relations"_s, elem.attribute( u"name"_s ) );
  QString strength = elem.attribute( u"strength"_s );

  QMap<QString, QgsMapLayer *> mapLayers = relationContext.project()->mapLayers();

  QgsMapLayer *referencingLayer = mapLayers[referencingLayerId];
  QgsMapLayer *referencedLayer = mapLayers[referencedLayerId];

  if ( !referencingLayer )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which does not exist." ).arg( referencingLayerId ) );
  }
  else if ( Qgis::LayerType::Vector != referencingLayer->type() )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which is not of type VectorLayer." ).arg( referencingLayerId ) );
  }

  if ( !referencedLayer )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which does not exist." ).arg( referencedLayerId ) );
  }
  else if ( Qgis::LayerType::Vector != referencedLayer->type() )
  {
    QgsLogger::warning( QApplication::translate( "QgsRelation", "Relation defined for layer '%1' which is not of type VectorLayer." ).arg( referencedLayerId ) );
  }

  relation.d->mReferencingLayerId = referencingLayerId;
  relation.d->mReferencingLayer = qobject_cast<QgsVectorLayer *>( referencingLayer );
  relation.d->mReferencedLayerId = referencedLayerId;
  relation.d->mReferencedLayer = qobject_cast<QgsVectorLayer *>( referencedLayer );
  relation.d->mRelationId = id;
  relation.d->mRelationName = name;
  relation.d->mRelationStrength = qgsEnumKeyToValue<Qgis::RelationshipStrength>( strength, Qgis::RelationshipStrength::Association );

  QDomNodeList references = elem.elementsByTagName( u"fieldRef"_s );
  for ( int i = 0; i < references.size(); ++i )
  {
    QDomElement refEl = references.at( i ).toElement();

    QString referencingField = refEl.attribute( u"referencingField"_s );
    QString referencedField = refEl.attribute( u"referencedField"_s );

    relation.addFieldPair( referencingField, referencedField );
  }

  relation.updateRelationStatus();

  return relation;
}

void QgsRelation::writeXml( QDomNode &node, QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"relation"_s );
  elem.setAttribute( u"id"_s, d->mRelationId );
  elem.setAttribute( u"name"_s, d->mRelationName );
  elem.setAttribute( u"referencingLayer"_s, d->mReferencingLayerId );
  elem.setAttribute( u"referencedLayer"_s, d->mReferencedLayerId );
  elem.setAttribute( u"strength"_s, qgsEnumValueToKey<Qgis::RelationshipStrength>( d->mRelationStrength ) );

  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    QDomElement referenceElem = doc.createElement( u"fieldRef"_s );
    referenceElem.setAttribute( u"referencingField"_s, pair.first );
    referenceElem.setAttribute( u"referencedField"_s, pair.second );
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


void QgsRelation::setStrength( Qgis::RelationshipStrength strength )
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
  QgsDebugMsgLevel( u"Filter conditions: '%1'"_s.arg( filter ), 2 );

  QgsFeatureRequest myRequest;
  myRequest.setFilterExpression( filter );
  return myRequest;
}

QString QgsRelation::getRelatedFeaturesFilter( const QgsFeature &feature ) const
{
  QStringList conditions;

  if ( ! d->mPolymorphicRelationId.isEmpty() )
  {
    QgsPolymorphicRelation polyRel = polymorphicRelation();
    if ( polyRel.isValid() )
    {
      conditions << QgsExpression::createFieldEqualityExpression( polyRel.referencedLayerField(), polyRel.layerRepresentation( referencedLayer() ) );
    }
    else
    {
      QgsDebugError( "The polymorphic relation is invalid" );
      conditions << u" FALSE "_s;
    }
  }

  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    QVariant val( feature.attribute( pair.referencedField() ) );
    int referencingIdx = referencingLayer()->fields().lookupField( pair.referencingField() );
    if ( referencingIdx >= 0 )
    {
      QMetaType::Type fieldType = referencingLayer()->fields().at( referencingIdx ).type();
      conditions << QgsExpression::createFieldEqualityExpression( pair.referencingField(), val, fieldType );
    }
    else
    {
      conditions << QgsExpression::createFieldEqualityExpression( pair.referencingField(), val );
    }
  }

  return conditions.join( " AND "_L1 );
}

QgsFeatureRequest QgsRelation::getReferencedFeatureRequest( const QgsAttributes &attributes ) const
{
  QStringList conditions;

  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    int referencedIdx = referencedLayer()->fields().lookupField( pair.referencedField() );
    int referencingIdx = referencingLayer()->fields().lookupField( pair.referencingField() );
    if ( referencedIdx >= 0 )
    {
      QMetaType::Type fieldType = referencedLayer()->fields().at( referencedIdx ).type();
      conditions << QgsExpression::createFieldEqualityExpression( pair.referencedField(), attributes.at( referencingIdx ), fieldType );
    }
    else
    {
      conditions << QgsExpression::createFieldEqualityExpression( pair.referencedField(), attributes.at( referencingIdx ) );
    }
  }

  QgsFeatureRequest myRequest;

  QgsDebugMsgLevel( u"Filter conditions: '%1'"_s.arg( conditions.join( " AND " ) ), 2 );

  myRequest.setFilterExpression( conditions.join( " AND "_L1 ) );

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
  ( void )d->mReferencedLayer->getFeatures( request ).nextFeature( f );
  return f;
}

QString QgsRelation::name() const
{
  return d->mRelationName;
}

Qgis::RelationshipStrength QgsRelation::strength() const
{
  return d->mRelationStrength;
}

QString QgsRelation::id() const
{
  return d->mRelationId;
}

void QgsRelation::generateId()
{
  if ( !d->mFieldPairs.isEmpty() )
  {
    const QgsRelation::FieldPair fieldPair = d->mFieldPairs.at( 0 );
    d->mRelationId = u"%1_%2_%3_%4"_s
                     .arg( referencingLayerId(),
                           fieldPair.referencingField(),
                           referencedLayerId(),
                           fieldPair.referencedField() );
  }
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
  attrs.reserve( d->mFieldPairs.size() );
  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    attrs << d->mReferencedLayer->fields().lookupField( pair.second );
  }
  return attrs;
}

QgsAttributeList QgsRelation::referencingFields() const
{
  QgsAttributeList attrs;

  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    attrs << d->mReferencingLayer->fields().lookupField( pair.first );
  }
  return attrs;

}

bool QgsRelation::referencingFieldsAllowNull() const
{
  if ( ! referencingLayer() )
  {
    return false;
  }

  const auto fields = referencingFields();

  return std::find_if( fields.constBegin(), fields.constEnd(), [&]( const auto & fieldIdx )
  {
    if ( !referencingLayer()->fields().exists( fieldIdx ) )
    {
      return false;
    }
    const QgsField field = referencingLayer()->fields().field( fieldIdx );
    return field.constraints().constraints().testFlag( QgsFieldConstraints::Constraint::ConstraintNotNull );
  } ) == fields.constEnd();
}

bool QgsRelation::isValid() const
{
  return d->mValid && !d->mReferencingLayer.isNull() && !d->mReferencedLayer.isNull() && d->mReferencingLayer.data()->isValid() && d->mReferencedLayer.data()->isValid();
}

QString QgsRelation::validationError() const
{
  if ( isValid() )
    return QString();

  if ( d->mReferencingLayer.isNull() )
  {
    if ( d->mReferencingLayerId.isEmpty() )
      return QObject::tr( "Referencing layer not set" );
    else
      return QObject::tr( "Referencing layer %1 does not exist" ).arg( d->mReferencingLayerId );
  }
  else if ( !d->mReferencingLayer.data()->isValid() )
    return QObject::tr( "Referencing layer %1 is not valid" ).arg( d->mReferencingLayerId );
  else if ( d->mReferencedLayer.isNull() )
  {
    if ( d->mReferencedLayerId.isEmpty() )
      return QObject::tr( "Referenced layer not set" );
    else
      return QObject::tr( "Referenced layer %1 does not exist" ).arg( d->mReferencedLayerId );
  }
  else if ( !d->mReferencedLayer.data()->isValid() )
    return QObject::tr( "Referenced layer %1 is not valid" ).arg( d->mReferencedLayerId );
  else
    return d->mValidationError;
}

bool QgsRelation::hasEqualDefinition( const QgsRelation &other ) const
{
  return d->mReferencedLayerId == other.d->mReferencedLayerId && d->mReferencingLayerId == other.d->mReferencingLayerId && d->mFieldPairs == other.d->mFieldPairs;
}

QString QgsRelation::resolveReferencedField( const QString &referencingField ) const
{
  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    if ( pair.first == referencingField )
      return pair.second;
  }
  return QString();
}

QString QgsRelation::resolveReferencingField( const QString &referencedField ) const
{
  for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    if ( pair.second == referencedField )
      return pair.first;
  }
  return QString();
}

void QgsRelation::updateRelationStatus()
{
  const QMap<QString, QgsMapLayer *> &mapLayers = mContext.project()->mapLayers();

  d->mReferencingLayer = qobject_cast<QgsVectorLayer *>( mapLayers[d->mReferencingLayerId] );
  d->mReferencedLayer = qobject_cast<QgsVectorLayer *>( mapLayers[d->mReferencedLayerId] );

  d->mValid = true;

  if ( d->mRelationId.isEmpty() )
  {
    QgsDebugError( u"Invalid relation: no ID"_s );
    d->mValidationError = QObject::tr( "Relationship has no ID" );
    d->mValid = false;
  }
  else
  {
    if ( !d->mReferencedLayer )
    {
      QgsDebugMsgLevel( u"Invalid relation: referenced layer does not exist. ID: %1"_s.arg( d->mReferencedLayerId ), 4 );
      d->mValidationError = QObject::tr( "Referenced layer %1 does not exist" ).arg( d->mReferencedLayerId );
      d->mValid = false;
    }
    else if ( !d->mReferencingLayer )
    {
      QgsDebugMsgLevel( u"Invalid relation: referencing layer does not exist. ID: %2"_s.arg( d->mReferencingLayerId ), 4 );
      d->mValidationError = QObject::tr( "Referencing layer %1 does not exist" ).arg( d->mReferencingLayerId );
      d->mValid = false;
    }
    else
    {
      if ( d->mFieldPairs.count() < 1 )
      {
        QgsDebugMsgLevel( u"Invalid relation: no pair of field is specified."_s, 4 );
        d->mValidationError = QObject::tr( "No fields specified for relationship" );
        d->mValid = false;
      }

      for ( const FieldPair &pair : std::as_const( d->mFieldPairs ) )
      {
        if ( -1 == d->mReferencingLayer->fields().lookupField( pair.first ) )
        {
          QgsDebugError( u"Invalid relation: field %1 does not exist in referencing layer %2"_s.arg( pair.first, d->mReferencingLayer->name() ) );
          d->mValidationError = QObject::tr( "Field %1 does not exist in referencing layer %2" ).arg( pair.first, d->mReferencingLayer->name() );
          d->mValid = false;
          break;
        }
        else if ( -1 == d->mReferencedLayer->fields().lookupField( pair.second ) )
        {
          QgsDebugError( u"Invalid relation: field %1 does not exist in referenced layer %2"_s.arg( pair.second, d->mReferencedLayer->name() ) );
          d->mValidationError = QObject::tr( "Field %1 does not exist in referenced layer %2" ).arg( pair.second, d->mReferencedLayer->name() );
          d->mValid = false;
          break;
        }
      }
    }

  }
}

void QgsRelation::setPolymorphicRelationId( const QString &polymorphicRelationId )
{
  d.detach();
  d->mPolymorphicRelationId = polymorphicRelationId;
}

QString QgsRelation::polymorphicRelationId() const
{
  return d->mPolymorphicRelationId;
}

QgsPolymorphicRelation QgsRelation::polymorphicRelation() const
{
  if ( ! mContext.project() || ! mContext.project()->relationManager() )
    return QgsPolymorphicRelation();

  return mContext.project()->relationManager()->polymorphicRelation( d->mPolymorphicRelationId );
}

Qgis::RelationshipType QgsRelation::type() const
{
  if ( d->mPolymorphicRelationId.isNull() )
    return Qgis::RelationshipType::Normal;
  else
    return Qgis::RelationshipType::Generated;
}

QString QgsRelation::cardinalityToDisplayString( Qgis::RelationshipCardinality cardinality )
{
  switch ( cardinality )
  {
    case Qgis::RelationshipCardinality::OneToOne:
      return QObject::tr( "One-to-one" );
    case Qgis::RelationshipCardinality::OneToMany:
      return QObject::tr( "One-to-many" );
    case Qgis::RelationshipCardinality::ManyToOne:
      return QObject::tr( "Many-to-one" );
    case Qgis::RelationshipCardinality::ManyToMany:
      return QObject::tr( "Many-to-many" );
  }
  BUILTIN_UNREACHABLE
}

QString QgsRelation::strengthToDisplayString( Qgis::RelationshipStrength strength )
{
  switch ( strength )
  {
    case Qgis::RelationshipStrength::Association:
      return QObject::tr( "Association" );
    case Qgis::RelationshipStrength::Composition:
      return QObject::tr( "Composition" );
  }
  BUILTIN_UNREACHABLE
}
