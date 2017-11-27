/***************************************************************************
  qgsvectorlayerutils.cpp
  -----------------------
  Date                 : October 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerutils.h"
#include "qgsvectordataprovider.h"
#include <QRegularExpression>
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgslogger.h"

bool QgsVectorLayerUtils::valueExists( const QgsVectorLayer *layer, int fieldIndex, const QVariant &value, const QgsFeatureIds &ignoreIds )
{
  if ( !layer )
    return false;

  QgsFields fields = layer->fields();

  if ( fieldIndex < 0 || fieldIndex >= fields.count() )
    return false;

  QString fieldName = fields.at( fieldIndex ).name();

  // build up an optimised feature request
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  request.setFlags( QgsFeatureRequest::NoGeometry );

  // at most we need to check ignoreIds.size() + 1 - the feature not in ignoreIds is the one we're interested in
  int limit = ignoreIds.size() + 1;
  request.setLimit( limit );

  request.setFilterExpression( QStringLiteral( "%1=%2" ).arg( QgsExpression::quotedColumnRef( fieldName ),
                               QgsExpression::quotedValue( value ) ) );

  QgsFeature feat;
  QgsFeatureIterator it = layer->getFeatures( request );
  while ( it.nextFeature( feat ) )
  {
    if ( ignoreIds.contains( feat.id() ) )
      continue;

    return true;
  }

  return false;
}

QVariant QgsVectorLayerUtils::createUniqueValue( const QgsVectorLayer *layer, int fieldIndex, const QVariant &seed )
{
  if ( !layer )
    return QVariant();

  QgsFields fields = layer->fields();

  if ( fieldIndex < 0 || fieldIndex >= fields.count() )
    return QVariant();

  QgsField field = fields.at( fieldIndex );

  if ( field.isNumeric() )
  {
    QVariant maxVal = layer->maximumValue( fieldIndex );
    QVariant newVar( maxVal.toLongLong() + 1 );
    if ( field.convertCompatible( newVar ) )
      return newVar;
    else
      return QVariant();
  }
  else
  {
    switch ( field.type() )
    {
      case QVariant::String:
      {
        QString base;
        if ( seed.isValid() )
          base = seed.toString();

        if ( !base.isEmpty() )
        {
          // strip any existing _1, _2 from the seed
          QRegularExpression rx( QStringLiteral( "(.*)_\\d+" ) );
          QRegularExpressionMatch match = rx.match( base );
          if ( match.hasMatch() )
          {
            base = match.captured( 1 );
          }
        }
        else
        {
          // no base seed - fetch first value from layer
          QgsFeatureRequest req;
          req.setLimit( 1 );
          req.setSubsetOfAttributes( QgsAttributeList() << fieldIndex );
          req.setFlags( QgsFeatureRequest::NoGeometry );
          QgsFeature f;
          layer->getFeatures( req ).nextFeature( f );
          base = f.attribute( fieldIndex ).toString();
        }

        // try variants like base_1, base_2, etc until a new value found
        QStringList vals = layer->uniqueStringsMatching( fieldIndex, base );

        // might already be unique
        if ( !base.isEmpty() && !vals.contains( base ) )
          return base;

        for ( int i = 1; i < 10000; ++i )
        {
          QString testVal = base + '_' + QString::number( i );
          if ( !vals.contains( testVal ) )
            return testVal;
        }

        // failed
        return QVariant();
      }

      default:
        // todo other types - dates? times?
        break;
    }
  }

  return QVariant();
}

bool QgsVectorLayerUtils::validateAttribute( const QgsVectorLayer *layer, const QgsFeature &feature, int attributeIndex, QStringList &errors,
    QgsFieldConstraints::ConstraintStrength strength, QgsFieldConstraints::ConstraintOrigin origin )
{
  if ( !layer )
    return false;

  if ( attributeIndex < 0 || attributeIndex >= layer->fields().count() )
    return false;

  QgsFields fields = layer->fields();
  QgsField field = fields.at( attributeIndex );
  QVariant value = feature.attribute( attributeIndex );
  bool valid = true;
  errors.clear();

  QgsFieldConstraints constraints = field.constraints();

  if ( constraints.constraints() & QgsFieldConstraints::ConstraintExpression && !constraints.constraintExpression().isEmpty()
       && ( strength == QgsFieldConstraints::ConstraintStrengthNotSet || strength == constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) )
       && ( origin == QgsFieldConstraints::ConstraintOriginNotSet || origin == constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) ) )
  {
    QgsExpressionContext context = layer->createExpressionContext();
    context.setFeature( feature );

    QgsExpression expr( constraints.constraintExpression() );

    valid = expr.evaluate( &context ).toBool();

    if ( expr.hasParserError() )
    {
      errors << QObject::tr( "parser error: %1" ).arg( expr.parserErrorString() );
    }
    else if ( expr.hasEvalError() )
    {
      errors << QObject::tr( "evaluation error: %1" ).arg( expr.evalErrorString() );
    }
    else if ( !valid )
    {
      errors << QObject::tr( "%1 check failed" ).arg( constraints.constraintDescription() );
    }
  }

  if ( constraints.constraints() & QgsFieldConstraints::ConstraintNotNull
       && ( strength == QgsFieldConstraints::ConstraintStrengthNotSet || strength == constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) )
       && ( origin == QgsFieldConstraints::ConstraintOriginNotSet || origin == constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) ) )
  {
    bool exempt = false;
    if ( fields.fieldOrigin( attributeIndex ) == QgsFields::OriginProvider
         && constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    {
      int providerIdx = fields.fieldOriginIndex( attributeIndex );
      exempt = layer->dataProvider()->skipConstraintCheck( providerIdx, QgsFieldConstraints::ConstraintNotNull, value );
    }

    if ( !exempt )
    {
      valid = valid && !value.isNull();

      if ( value.isNull() )
      {
        errors << QObject::tr( "value is NULL" );
      }
    }
  }

  if ( constraints.constraints() & QgsFieldConstraints::ConstraintUnique
       && ( strength == QgsFieldConstraints::ConstraintStrengthNotSet || strength == constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) )
       && ( origin == QgsFieldConstraints::ConstraintOriginNotSet || origin == constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) ) )
  {
    bool exempt = false;
    if ( fields.fieldOrigin( attributeIndex ) == QgsFields::OriginProvider
         && constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    {
      int providerIdx = fields.fieldOriginIndex( attributeIndex );
      exempt = layer->dataProvider()->skipConstraintCheck( providerIdx, QgsFieldConstraints::ConstraintUnique, value );
    }

    if ( !exempt )
    {
      bool alreadyExists = QgsVectorLayerUtils::valueExists( layer, attributeIndex, value, QgsFeatureIds() << feature.id() );
      valid = valid && !alreadyExists;

      if ( alreadyExists )
      {
        errors << QObject::tr( "value is not unique" );
      }
    }
  }

  return valid;
}

QgsFeature QgsVectorLayerUtils::createFeature( QgsVectorLayer *layer, const QgsGeometry &geometry,
    const QgsAttributeMap &attributes, QgsExpressionContext *context )
{
  if ( !layer )
  {
    return QgsFeature();
  }

  QgsExpressionContext *evalContext = context;
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !evalContext )
  {
    // no context passed, so we create a default one
    tempContext.reset( new QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) ) );
    evalContext = tempContext.get();
  }

  QgsFields fields = layer->fields();

  QgsFeature newFeature( fields );
  newFeature.setValid( true );
  newFeature.setGeometry( geometry );

  // initialize attributes
  newFeature.initAttributes( fields.count() );
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QVariant v;
    bool checkUnique = true;

    // in order of priority:

    // 1. client side default expression
    if ( layer->defaultValueDefinition( idx ).isValid() )
    {
      // client side default expression set - takes precedence over all. Why? Well, this is the only default
      // which QGIS users have control over, so we assume that they're deliberately overriding any
      // provider defaults for some good reason and we should respect that
      v = layer->defaultValue( idx, newFeature, evalContext );
    }

    // 2. provider side default value clause
    // note - not an else if deliberately. Users may return null from a default value expression to fallback to provider defaults
    if ( !v.isValid() && fields.fieldOrigin( idx ) == QgsFields::OriginProvider )
    {
      int providerIndex = fields.fieldOriginIndex( idx );
      QString providerDefault = layer->dataProvider()->defaultValueClause( providerIndex );
      if ( !providerDefault.isEmpty() )
      {
        v = providerDefault;
        checkUnique = false;
      }
    }

    // 3. provider side default literal
    // note - deliberately not using else if!
    if ( !v.isValid() && fields.fieldOrigin( idx ) == QgsFields::OriginProvider )
    {
      int providerIndex = fields.fieldOriginIndex( idx );
      v = layer->dataProvider()->defaultValue( providerIndex );
      if ( v.isValid() )
      {
        //trust that the provider default has been sensibly set not to violate any constraints
        checkUnique = false;
      }
    }

    // 4. passed attribute value
    // note - deliberately not using else if!
    if ( !v.isValid() && attributes.contains( idx ) )
    {
      v = attributes.value( idx );
    }

    // last of all... check that unique constraints are respected
    // we can't handle not null or expression constraints here, since there's no way to pick a sensible
    // value if the constraint is violated
    if ( checkUnique && fields.at( idx ).constraints().constraints() & QgsFieldConstraints::ConstraintUnique )
    {
      if ( QgsVectorLayerUtils::valueExists( layer, idx, v ) )
      {
        // unique constraint violated
        QVariant uniqueValue = QgsVectorLayerUtils::createUniqueValue( layer, idx, v );
        if ( uniqueValue.isValid() )
          v = uniqueValue;
      }
    }

    newFeature.setAttribute( idx, v );
  }

  return newFeature;
}

QgsFeature QgsVectorLayerUtils::duplicateFeature( QgsVectorLayer *layer, const QgsFeature &feature, QgsProject *project, int depth, QgsDuplicateFeatureContext &duplicateFeatureContext )
{
  if ( !layer )
    return QgsFeature();

  if ( !layer->isEditable() )
    return QgsFeature();

  //get context from layer
  QgsExpressionContext context = layer->createExpressionContext();
  context.setFeature( feature );

  //create the attribute map
  QgsAttributes srcAttr = feature.attributes();
  QgsAttributeMap dstAttr;
  for ( int src = 0; src < srcAttr.count(); ++src )
  {
    dstAttr[ src ] = srcAttr.at( src );
  }

  QgsFeature newFeature = createFeature( layer, feature.geometry(), dstAttr, &context );

  const QList<QgsRelation> relations = project->relationManager()->referencedRelations( layer );

  for ( const QgsRelation &relation : relations )
  {
    //check if composition (and not association)
    if ( relation.strength() == QgsRelation::Composition && depth < 1 )
    {
      depth++;
      //get features connected over this relation
      QgsFeatureIterator relatedFeaturesIt = relation.getRelatedFeatures( feature );
      QgsFeatureIds childFeatureIds;
      QgsFeature childFeature;
      while ( relatedFeaturesIt.nextFeature( childFeature ) )
      {
        //set childlayer editable
        relation.referencingLayer()->startEditing();
        //change the fk of the child to the id of the new parent
        for ( const QgsRelation::FieldPair &fieldPair : relation.fieldPairs() )
        {
          childFeature.setAttribute( fieldPair.first, newFeature.attribute( fieldPair.second ) );
        }
        //call the function for the child
        childFeatureIds.insert( duplicateFeature( relation.referencingLayer(), childFeature, project, depth, duplicateFeatureContext ).id() );
      }

      //store for feedback
      duplicateFeatureContext.setDuplicatedFeatures( relation.referencingLayer(), childFeatureIds );
    }
  }

  layer->addFeature( newFeature );

  return newFeature;
}

QList<QgsVectorLayer *> QgsVectorLayerUtils::QgsDuplicateFeatureContext::layers() const
{
  QList<QgsVectorLayer *> layers;
  QMap<QgsVectorLayer *, QgsFeatureIds>::const_iterator i;
  for ( i = mDuplicatedFeatures.begin(); i != mDuplicatedFeatures.end(); ++i )
    layers.append( i.key() );
  return layers;
}

QgsFeatureIds QgsVectorLayerUtils::QgsDuplicateFeatureContext::duplicatedFeatures( QgsVectorLayer *layer ) const
{
  return mDuplicatedFeatures[layer];
}

void QgsVectorLayerUtils::QgsDuplicateFeatureContext::setDuplicatedFeatures( QgsVectorLayer *layer, QgsFeatureIds ids )
{
  mDuplicatedFeatures.insert( layer, ids );
}
/*
QMap<QgsVectorLayer *, QgsFeatureIds>  QgsVectorLayerUtils::QgsDuplicateFeatureContext::duplicateFeatureContext() const
{
  return mDuplicatedFeatures;
}
*/
