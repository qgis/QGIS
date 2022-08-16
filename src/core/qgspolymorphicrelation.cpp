/***************************************************************************
    qgspolymorphicrelation.h
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 Ivan Ivanov
    Email                : ivan at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspolymorphicrelation.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgspolymorphicrelation_p.h"

#include <QApplication>

QgsPolymorphicRelation::QgsPolymorphicRelation()
  : d( new QgsPolymorphicRelationPrivate() )
{
}

QgsPolymorphicRelation::QgsPolymorphicRelation( const QgsRelationContext &context )
  : d( new QgsPolymorphicRelationPrivate() )
  , mContext( context )
{
}

QgsPolymorphicRelation::~QgsPolymorphicRelation() = default;

QgsPolymorphicRelation::QgsPolymorphicRelation( const QgsPolymorphicRelation &other )
  : d( other.d )
  , mContext( other.mContext )
{
}

QgsPolymorphicRelation &QgsPolymorphicRelation::operator=( const QgsPolymorphicRelation &other )
{
  d = other.d;
  mContext = other.mContext;
  return *this;
}

QgsPolymorphicRelation QgsPolymorphicRelation::createFromXml( const QDomNode &node, QgsReadWriteContext &context,  const QgsRelationContext &relationContext )
{
  Q_UNUSED( context );
  QDomElement elem = node.toElement();

  if ( elem.tagName() != QLatin1String( "relation" ) )
  {
    QgsLogger::warning( QApplication::translate( "QgsPolymorphicRelation", "Cannot create relation. Unexpected tag '%1'" ).arg( elem.tagName() ) );
  }

  QgsPolymorphicRelation relation( relationContext );

  QString referencingLayerId = elem.attribute( QStringLiteral( "referencingLayer" ) );
  QString referencedLayerField = elem.attribute( QStringLiteral( "referencedLayerField" ) );
  QString referencedLayerExpression = elem.attribute( QStringLiteral( "referencedLayerExpression" ) );
  QString id = elem.attribute( QStringLiteral( "id" ) );
  QString name = elem.attribute( QStringLiteral( "name" ) );
  QString relationStrength = elem.attribute( QStringLiteral( "relationStrength" ) );
  QStringList referencedLayerIds = elem.attribute( QStringLiteral( "referencedLayerIds" ) ).split( "," );

  QMap<QString, QgsMapLayer *> mapLayers = relationContext.project()->mapLayers();

  relation.d->mReferencingLayerId = referencingLayerId;
  relation.d->mReferencingLayer = qobject_cast<QgsVectorLayer *>( mapLayers[referencingLayerId] );
  relation.d->mReferencedLayerField = referencedLayerField;
  relation.d->mReferencedLayerExpression = referencedLayerExpression;
  relation.d->mReferencedLayerIds = referencedLayerIds;
  relation.d->mRelationId = id;
  relation.d->mRelationName = name;
  relation.d->mRelationStrength = qgsEnumKeyToValue<Qgis::RelationshipStrength>( relationStrength, Qgis::RelationshipStrength::Association );

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

void QgsPolymorphicRelation::writeXml( QDomNode &node, QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "relation" ) );
  elem.setAttribute( QStringLiteral( "id" ), d->mRelationId );
  elem.setAttribute( QStringLiteral( "name" ), d->mRelationName );
  elem.setAttribute( QStringLiteral( "referencingLayer" ), d->mReferencingLayerId );
  elem.setAttribute( QStringLiteral( "referencedLayerField" ), d->mReferencedLayerField );
  elem.setAttribute( QStringLiteral( "referencedLayerExpression" ), d->mReferencedLayerExpression );
  elem.setAttribute( QStringLiteral( "referencedLayerIds" ), d->mReferencedLayerIds.join( "," ) );
  elem.setAttribute( QStringLiteral( "relationStrength" ), qgsEnumValueToKey<Qgis::RelationshipStrength>( d->mRelationStrength ) );

  // note that a layer id can store a comma in theory. Luckyly, this is not easy to achieve, e.g. you need to modify the .qgs file manually
  for ( const QString &layerId : std::as_const( d->mReferencedLayerIds ) )
    Q_ASSERT( ! layerId.contains( "," ) );

  for ( const QgsRelation::FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    QDomElement referenceElem = doc.createElement( QStringLiteral( "fieldRef" ) );
    referenceElem.setAttribute( QStringLiteral( "referencingField" ), pair.first );
    referenceElem.setAttribute( QStringLiteral( "referencedField" ), pair.second );
    elem.appendChild( referenceElem );
  }

  node.appendChild( elem );
}

void QgsPolymorphicRelation::setId( const QString &id )
{
  if ( d->mRelationId == id )
    return;

  d.detach();
  d->mRelationId = id;

  updateRelationStatus();
}

void QgsPolymorphicRelation::setReferencingLayer( const QString &id )
{
  d.detach();
  d->mReferencingLayerId = id;

  updateRelationStatus();
}

void QgsPolymorphicRelation::addFieldPair( const QString &referencingField, const QString &referencedField )
{
  d.detach();
  d->mFieldPairs << QgsRelation::FieldPair( referencingField, referencedField );
  updateRelationStatus();
}

void QgsPolymorphicRelation::addFieldPair( const QgsRelation::FieldPair &fieldPair )
{
  d.detach();
  d->mFieldPairs << fieldPair;
  updateRelationStatus();
}

QString QgsPolymorphicRelation::id() const
{
  return d->mRelationId;
}

void QgsPolymorphicRelation::generateId()
{
  d->mRelationId = QStringLiteral( "%1_%2_%3_%4" )
                   .arg( referencingLayerId(),
                         d->mFieldPairs.at( 0 ).referencingField(),
                         referencedLayerField(),
                         d->mFieldPairs.at( 0 ).referencedField() );
  updateRelationStatus();
}

QString QgsPolymorphicRelation::referencingLayerId() const
{
  return d->mReferencingLayerId;
}

QgsVectorLayer *QgsPolymorphicRelation::referencingLayer() const
{
  return d->mReferencingLayer;
}

QList<QgsRelation::FieldPair> QgsPolymorphicRelation::fieldPairs() const
{
  return d->mFieldPairs;
}

QgsAttributeList QgsPolymorphicRelation::referencedFields( const QString &layerId ) const
{
  QgsAttributeList attrs;

  if ( d->mReferencedLayerIds.contains( layerId ) )
  {
    QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( layerId ) );

    if ( vl && vl->isValid() )
    {
      for ( const QgsRelation::FieldPair &pair : std::as_const( d->mFieldPairs ) )
      {
        attrs << vl->fields().lookupField( pair.second );
      }
    }
  }

  return attrs;
}

QgsAttributeList QgsPolymorphicRelation::referencingFields() const
{
  QgsAttributeList attrs;

  for ( const QgsRelation::FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    attrs << d->mReferencingLayer->fields().lookupField( pair.first );
  }
  return attrs;

}

bool QgsPolymorphicRelation::isValid() const
{
  return d->mValid && !d->mReferencingLayer.isNull() && !d->mReferencedLayerField.isNull() && d->mReferencingLayer.data()->isValid() && !d->mReferencedLayerExpression.isNull();
}

bool QgsPolymorphicRelation::hasEqualDefinition( const QgsPolymorphicRelation &other ) const
{
  return d->mReferencedLayerField == other.d->mReferencedLayerField
         && d->mReferencedLayerExpression == other.d->mReferencedLayerExpression
         && d->mReferencingLayerId == other.d->mReferencingLayerId
         && d->mFieldPairs == other.d->mFieldPairs;
}

void QgsPolymorphicRelation::updateRelationStatus()
{
  const QMap<QString, QgsMapLayer *> &mapLayers = mContext.project()->mapLayers();

  d->mValid = true;
  d->mReferencingLayer = mapLayers.contains( d->mReferencingLayerId )
                         ? qobject_cast<QgsVectorLayer *>( mapLayers[d->mReferencingLayerId] )
                         : nullptr;
  d->mReferencedLayersMap.clear();

  if ( d->mRelationId.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Invalid relation: no ID" ) );
    d->mValid = false;
    return;
  }

  if ( !d->mReferencingLayer )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referencing layer does not exist. ID: %1" ).arg( d->mReferencingLayerId ), 4 );
    d->mValid = false;
    return;
  }

  if ( d->mReferencingLayer->fields().lookupField( d->mReferencedLayerField ) == -1 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referenced layer field \"%1\" does not exist in layer with ID: %2" ).arg( d->mReferencedLayerField, d->mReferencingLayerId ), 4 );
    d->mValid = false;
    return;
  }

  if ( d->mReferencedLayerExpression.trimmed().isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referenced layer expression \"%1\" is missing" ).arg( d->mReferencedLayerExpression ), 4 );
    d->mValid = false;
    return;
  }

  const QStringList referencedLayerIds = d->mReferencedLayerIds;
  for ( const QString &referencedLayerId : referencedLayerIds )
  {
    d->mReferencedLayersMap.insert( referencedLayerId, qobject_cast<QgsVectorLayer *>( mapLayers[referencedLayerId] ) );

    if ( !d->mReferencedLayersMap[referencedLayerId] || !d->mReferencedLayersMap[referencedLayerId]->isValid() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Invalid relation: referenced layer does not exist. ID: %1" ).arg( d->mReferencedLayersMap[referencedLayerId]->id() ), 4 );
      d->mValid = false;
      return;
    }
  }

  if ( d->mFieldPairs.count() == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid relation: no pair of field is specified." ), 4 );
    d->mValid = false;
    return;
  }

  for ( const QgsRelation::FieldPair &pair : std::as_const( d->mFieldPairs ) )
  {
    if ( d->mReferencingLayer->fields().lookupField( pair.first ) == -1 )
    {
      QgsDebugMsg( QStringLiteral( "Invalid relation: field %1 does not exist in referencing layer %2" ).arg( pair.first, d->mReferencingLayer->name() ) );
      d->mValid = false;
      return;
    }

    for ( const QString &referencedLayerId : referencedLayerIds )
    {
      if ( d->mReferencedLayersMap[referencedLayerId]->fields().lookupField( pair.second ) == -1 )
      {
        QgsDebugMsg( QStringLiteral( "Invalid relation: field %1 does not exist in referenced layer %2" ).arg( pair.second, d->mReferencedLayersMap[referencedLayerId]->name() ) );
        d->mValid = false;
        return;
      }
    }
  }
}

void QgsPolymorphicRelation::setName( const QString &name )
{
  if ( d->mRelationName == name && !name.isEmpty() )
    return;

  d.detach();
  d->mRelationName = name;
  updateRelationStatus();
}

QString QgsPolymorphicRelation::name() const
{
  if ( d->mRelationName.isEmpty() )
    return QObject::tr( "Polymorphic relations for \"%1\"" ).arg( d->mReferencingLayer ? d->mReferencingLayer->name() : QStringLiteral( "<NO LAYER>" ) );

  return d->mRelationName;
}

void QgsPolymorphicRelation::setReferencedLayerField( const QString &referencedLayerField )
{
  d.detach();
  d->mReferencedLayerField = referencedLayerField;
  updateRelationStatus();
}

QString QgsPolymorphicRelation::referencedLayerField() const
{
  return d->mReferencedLayerField;
}

void QgsPolymorphicRelation::setReferencedLayerExpression( const QString &referencedLayerExpression )
{
  d.detach();
  d->mReferencedLayerExpression = referencedLayerExpression;
  updateRelationStatus();
}

QString QgsPolymorphicRelation::referencedLayerExpression() const
{
  return d->mReferencedLayerExpression;
}

void QgsPolymorphicRelation::setReferencedLayerIds( const QStringList &referencedLayerIds )
{
  d.detach();
  d->mReferencedLayerIds = referencedLayerIds;
  updateRelationStatus();
}

QStringList QgsPolymorphicRelation::referencedLayerIds() const
{
  return d->mReferencedLayerIds;
}

Qgis::RelationshipStrength QgsPolymorphicRelation::strength() const
{
  return d->mRelationStrength;
}

void QgsPolymorphicRelation::setRelationStrength( Qgis::RelationshipStrength relationStrength )
{
  d.detach();
  d->mRelationStrength = relationStrength;
  updateRelationStatus();
}

QList<QgsRelation> QgsPolymorphicRelation::generateRelations() const
{
  QList<QgsRelation> relations;

  if ( !isValid() )
    return relations;

  const QStringList referencedLayerIds = d->mReferencedLayerIds;

  for ( const QString &referencedLayerId : referencedLayerIds )
  {
    QgsRelation relation;
    QString referencedLayerName = d->mReferencedLayersMap[referencedLayerId]->name();

    relation.setId( QStringLiteral( "%1_%2" ).arg( d->mRelationId, referencedLayerName ) );
    relation.setReferencedLayer( referencedLayerId );
    relation.setReferencingLayer( d->mReferencingLayerId );
    relation.setName( QStringLiteral( "Generated for \"%1\"" ).arg( referencedLayerName ) );
    relation.setPolymorphicRelationId( d->mRelationId );
    relation.setStrength( d->mRelationStrength );

    const QList<QgsRelation::FieldPair> constFieldPairs = fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
      relation.addFieldPair( fieldPair );

    if ( !relation.isValid() )
      continue;

    relations << relation;
  }

  return relations;
}

QString QgsPolymorphicRelation::layerRepresentation( const QgsVectorLayer *layer ) const
{
  if ( !layer || !layer->isValid() )
    return QString();

  QgsExpressionContext context = layer->createExpressionContext();
  QgsExpression expr( d->mReferencedLayerExpression );

  return expr.evaluate( &context ).toString();
}
