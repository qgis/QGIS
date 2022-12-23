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

#include <QRegularExpression>

#include "qgsexpressioncontext.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"
#include "qgsthreadingutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgspallabeling.h"
#include "qgsrenderer.h"
#include "qgssymbollayer.h"
#include "qgsstyleentityvisitor.h"
#include "qgsstyle.h"
#include "qgsauxiliarystorage.h"
#include "qgssymbollayerreference.h"
#include "qgspainteffect.h"

QgsFeatureIterator QgsVectorLayerUtils::getValuesIterator( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly )
{
  std::unique_ptr<QgsExpression> expression;
  QgsExpressionContext context;

  int attrNum = layer->fields().lookupField( fieldOrExpression );
  if ( attrNum == -1 )
  {
    // try to use expression
    expression.reset( new QgsExpression( fieldOrExpression ) );
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );

    if ( expression->hasParserError() || !expression->prepare( &context ) )
    {
      ok = false;
      return QgsFeatureIterator();
    }
  }

  QSet<QString> lst;
  if ( !expression )
    lst.insert( fieldOrExpression );
  else
    lst = expression->referencedColumns();

  QgsFeatureRequest request = QgsFeatureRequest()
                              .setFlags( ( expression && expression->needsGeometry() ) ?
                                         QgsFeatureRequest::NoFlags :
                                         QgsFeatureRequest::NoGeometry )
                              .setSubsetOfAttributes( lst, layer->fields() );

  ok = true;
  if ( !selectedOnly )
  {
    return layer->getFeatures( request );
  }
  else
  {
    return layer->getSelectedFeatures( request );
  }
}

QList<QVariant> QgsVectorLayerUtils::getValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly, QgsFeedback *feedback )
{
  QList<QVariant> values;
  QgsFeatureIterator fit = getValuesIterator( layer, fieldOrExpression, ok, selectedOnly );
  if ( ok )
  {
    std::unique_ptr<QgsExpression> expression;
    QgsExpressionContext context;

    int attrNum = layer->fields().lookupField( fieldOrExpression );
    if ( attrNum == -1 )
    {
      // use expression, already validated in the getValuesIterator() function
      expression.reset( new QgsExpression( fieldOrExpression ) );
      context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
    }

    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      if ( expression )
      {
        context.setFeature( f );
        QVariant v = expression->evaluate( &context );
        values << v;
      }
      else
      {
        values << f.attribute( attrNum );
      }
      if ( feedback && feedback->isCanceled() )
      {
        ok = false;
        return values;
      }
    }
  }
  return values;
}

QList<double> QgsVectorLayerUtils::getDoubleValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly, int *nullCount, QgsFeedback *feedback )
{
  QList<double> values;

  if ( nullCount )
    *nullCount = 0;

  QList<QVariant> variantValues = getValues( layer, fieldOrExpression, ok, selectedOnly, feedback );
  if ( !ok )
    return values;

  bool convertOk;
  const auto constVariantValues = variantValues;
  for ( const QVariant &value : constVariantValues )
  {
    double val = value.toDouble( &convertOk );
    if ( convertOk )
      values << val;
    else if ( QgsVariantUtils::isNull( value ) )
    {
      if ( nullCount )
        *nullCount += 1;
    }
    if ( feedback && feedback->isCanceled() )
    {
      ok = false;
      return values;
    }
  }
  return values;
}

bool QgsVectorLayerUtils::valueExists( const QgsVectorLayer *layer, int fieldIndex, const QVariant &value, const QgsFeatureIds &ignoreIds )
{
  if ( !layer )
    return false;

  QgsFields fields = layer->fields();

  if ( fieldIndex < 0 || fieldIndex >= fields.count() )
    return false;

  // If it's a joined field search the value in the source layer
  if ( fields.fieldOrigin( fieldIndex ) == QgsFields::FieldOrigin::OriginJoin )
  {
    int srcFieldIndex;
    const QgsVectorLayerJoinInfo *joinInfo { layer->joinBuffer()->joinForFieldIndex( fieldIndex, fields, srcFieldIndex ) };
    if ( ! joinInfo )
    {
      return false;
    }
    fieldIndex = srcFieldIndex;
    layer = joinInfo->joinLayer();
    if ( ! layer )
    {
      return false;
    }
    fields = layer->fields();
  }

  QString fieldName = fields.at( fieldIndex ).name();

  // build up an optimised feature request
  QgsFeatureRequest request;
  request.setNoAttributes();
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

QVariant QgsVectorLayerUtils::createUniqueValueFromCache( const QgsVectorLayer *layer, int fieldIndex, const QSet<QVariant> &existingValues, const QVariant &seed )
{
  if ( !layer )
    return QVariant();

  QgsFields fields = layer->fields();

  if ( fieldIndex < 0 || fieldIndex >= fields.count() )
    return QVariant();

  QgsField field = fields.at( fieldIndex );

  if ( field.isNumeric() )
  {
    QVariant maxVal = existingValues.isEmpty() ? 0 : *std::max_element( existingValues.begin(), existingValues.end(), []( const QVariant & a, const QVariant & b ) { return a.toLongLong() < b.toLongLong(); } );
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
          base = existingValues.isEmpty() ? QString() : existingValues.constBegin()->toString();
        }

        // try variants like base_1, base_2, etc until a new value found
        QStringList vals;
        for ( const auto &v : std::as_const( existingValues ) )
        {
          if ( v.toString().startsWith( base ) )
            vals.push_back( v.toString() );
        }

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
  const QVariant value = feature.attribute( attributeIndex );
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

  bool notNullConstraintViolated { false };

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
      valid = valid && !QgsVariantUtils::isNull( value );

      if ( QgsVariantUtils::isNull( value ) )
      {
        errors << QObject::tr( "value is NULL" );
        notNullConstraintViolated = true;
      }
    }
  }

  // if a NOT NULL constraint is violated we don't need to check for UNIQUE
  if ( ! notNullConstraintViolated )
  {

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
  }

  return valid;
}

QgsFeature QgsVectorLayerUtils::createFeature( const QgsVectorLayer *layer, const QgsGeometry &geometry,
    const QgsAttributeMap &attributes, QgsExpressionContext *context )
{
  QgsFeatureList features { createFeatures( layer, QgsFeaturesDataList() << QgsFeatureData( geometry, attributes ), context ) };
  return features.isEmpty() ? QgsFeature() : features.first();
}

QgsFeatureList QgsVectorLayerUtils::createFeatures( const QgsVectorLayer *layer, const QgsFeaturesDataList &featuresData, QgsExpressionContext *context )
{
  if ( !layer )
    return QgsFeatureList();

  QgsFeatureList result;
  result.reserve( featuresData.length() );

  QgsExpressionContext *evalContext = context;
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !evalContext )
  {
    // no context passed, so we create a default one
    tempContext.reset( new QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) ) );
    evalContext = tempContext.get();
  }

  QgsFields fields = layer->fields();

  // Cache unique values
  QMap<int, QSet<QVariant>> uniqueValueCache;

  auto checkUniqueValue = [ & ]( const int fieldIdx, const QVariant & value )
  {
    if ( ! uniqueValueCache.contains( fieldIdx ) )
    {
      // If the layer is filtered, get unique values from an unfiltered clone
      if ( ! layer->subsetString().isEmpty() )
      {
        std::unique_ptr<QgsVectorLayer> unfilteredClone { layer->clone( ) };
        unfilteredClone->setSubsetString( QString( ) );
        uniqueValueCache[ fieldIdx ] = unfilteredClone->uniqueValues( fieldIdx );
      }
      else
      {
        uniqueValueCache[ fieldIdx ] = layer->uniqueValues( fieldIdx );
      }
    }
    return uniqueValueCache[ fieldIdx ].contains( value );
  };

  for ( const auto &fd : std::as_const( featuresData ) )
  {

    QgsFeature newFeature( fields );
    newFeature.setValid( true );
    newFeature.setGeometry( fd.geometry() );

    // initialize attributes
    newFeature.initAttributes( fields.count() );
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      QVariant v;
      bool checkUnique = true;
      const bool hasUniqueConstraint { static_cast<bool>( fields.at( idx ).constraints().constraints() & QgsFieldConstraints::ConstraintUnique ) };

      // in order of priority:
      // 1. passed attribute value and if field does not have a unique constraint like primary key
      if ( fd.attributes().contains( idx ) )
      {
        v = fd.attributes().value( idx );
      }

      // 2. client side default expression
      // note - deliberately not using else if!
      QgsDefaultValue defaultValueDefinition = layer->defaultValueDefinition( idx );
      if ( ( QgsVariantUtils::isNull( v ) || ( hasUniqueConstraint
             && checkUniqueValue( idx, v ) )
             || defaultValueDefinition.applyOnUpdate() )
           && defaultValueDefinition.isValid() )
      {
        // client side default expression set - takes precedence over all. Why? Well, this is the only default
        // which QGIS users have control over, so we assume that they're deliberately overriding any
        // provider defaults for some good reason and we should respect that
        v = layer->defaultValue( idx, newFeature, evalContext );
      }

      // 3. provider side default value clause
      // note - not an else if deliberately. Users may return null from a default value expression to fallback to provider defaults
      if ( ( QgsVariantUtils::isNull( v ) || ( hasUniqueConstraint
             && checkUniqueValue( idx, v ) ) )
           && fields.fieldOrigin( idx ) == QgsFields::OriginProvider )
      {
        int providerIndex = fields.fieldOriginIndex( idx );
        QString providerDefault = layer->dataProvider()->defaultValueClause( providerIndex );
        if ( !providerDefault.isEmpty() )
        {
          v = providerDefault;
          checkUnique = false;
        }
      }

      // 4. provider side default literal
      // note - deliberately not using else if!
      if ( ( QgsVariantUtils::isNull( v ) || ( checkUnique
             && hasUniqueConstraint
             && checkUniqueValue( idx, v ) ) )
           && fields.fieldOrigin( idx ) == QgsFields::OriginProvider )
      {
        int providerIndex = fields.fieldOriginIndex( idx );
        v = layer->dataProvider()->defaultValue( providerIndex );
        if ( v.isValid() )
        {
          //trust that the provider default has been sensibly set not to violate any constraints
          checkUnique = false;
        }
      }

      // 5. passed attribute value
      // note - deliberately not using else if!
      if ( QgsVariantUtils::isNull( v ) && fd.attributes().contains( idx ) )
      {
        v = fd.attributes().value( idx );
      }

      // last of all... check that unique constraints are respected if the value is valid
      if ( v.isValid() )
      {
        // we can't handle not null or expression constraints here, since there's no way to pick a sensible
        // value if the constraint is violated
        if ( checkUnique && hasUniqueConstraint )
        {
          if ( checkUniqueValue( idx,  v ) )
          {
            // unique constraint violated
            QVariant uniqueValue = QgsVectorLayerUtils::createUniqueValueFromCache( layer, idx, uniqueValueCache[ idx ], v );
            if ( uniqueValue.isValid() )
              v = uniqueValue;
          }
        }
        if ( hasUniqueConstraint )
        {
          uniqueValueCache[ idx ].insert( v );
        }
      }
      newFeature.setAttribute( idx, v );
    }
    result.append( newFeature );
  }
  return result;
}

QgsFeature QgsVectorLayerUtils::duplicateFeature( QgsVectorLayer *layer, const QgsFeature &feature, QgsProject *project, QgsDuplicateFeatureContext &duplicateFeatureContext, const int maxDepth, int depth, QList<QgsVectorLayer *> referencedLayersBranch )
{
  if ( !layer )
    return QgsFeature();

  if ( !layer->isEditable() )
    return QgsFeature();

  //get context from layer
  QgsExpressionContext context = layer->createExpressionContext();
  context.setFeature( feature );

  QgsFeature newFeature = createFeature( layer, feature.geometry(), feature.attributes().toMap(), &context );
  layer->addFeature( newFeature );

  const QList<QgsRelation> relations = project->relationManager()->referencedRelations( layer );

  const int effectiveMaxDepth = maxDepth > 0 ? maxDepth : 100;

  for ( const QgsRelation &relation : relations )
  {
    //check if composition (and not association)
    if ( relation.strength() == Qgis::RelationshipStrength::Composition && !referencedLayersBranch.contains( relation.referencedLayer() ) && depth < effectiveMaxDepth )
    {
      depth++;
      referencedLayersBranch << layer;

      //get features connected over this relation
      QgsFeatureIterator relatedFeaturesIt = relation.getRelatedFeatures( feature );
      QgsFeatureIds childFeatureIds;
      QgsFeature childFeature;
      while ( relatedFeaturesIt.nextFeature( childFeature ) )
      {
        //set childlayer editable
        relation.referencingLayer()->startEditing();
        //change the fk of the child to the id of the new parent
        const auto pairs = relation.fieldPairs();
        for ( const QgsRelation::FieldPair &fieldPair : pairs )
        {
          childFeature.setAttribute( fieldPair.first, newFeature.attribute( fieldPair.second ) );
        }
        //call the function for the child
        childFeatureIds.insert( duplicateFeature( relation.referencingLayer(), childFeature, project, duplicateFeatureContext, maxDepth, depth, referencedLayersBranch ).id() );
      }

      //store for feedback
      duplicateFeatureContext.setDuplicatedFeatures( relation.referencingLayer(), childFeatureIds );
    }
  }


  return newFeature;
}

std::unique_ptr<QgsVectorLayerFeatureSource> QgsVectorLayerUtils::getFeatureSource( QPointer<QgsVectorLayer> layer, QgsFeedback *feedback )
{
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;

  auto getFeatureSource = [ layer, &featureSource, feedback ]
  {
    Q_ASSERT( QThread::currentThread() == qApp->thread() || feedback );
    QgsVectorLayer *lyr = layer.data();

    if ( lyr )
    {
      featureSource.reset( new QgsVectorLayerFeatureSource( lyr ) );
    }
  };

  QgsThreadingUtils::runOnMainThread( getFeatureSource, feedback );

  return featureSource;
}

void QgsVectorLayerUtils::matchAttributesToFields( QgsFeature &feature, const QgsFields &fields )
{
  if ( !feature.fields().isEmpty() )
  {
    QgsAttributes attributes;
    attributes.reserve( fields.size() );
    // feature has a field mapping, so we can match attributes to field names
    for ( const QgsField &field : fields )
    {
      int index = feature.fields().lookupField( field.name() );
      attributes.append( index >= 0 ? feature.attribute( index ) : QVariant( field.type() ) );
    }
    feature.setAttributes( attributes );
  }
  else
  {
    // no field name mapping in feature, just use order
    const int lengthDiff = feature.attributes().count() - fields.count();
    if ( lengthDiff > 0 )
    {
      // truncate extra attributes
      QgsAttributes attributes = feature.attributes().mid( 0, fields.count() );
      feature.setAttributes( attributes );
    }
    else if ( lengthDiff < 0 )
    {
      // add missing null attributes
      QgsAttributes attributes = feature.attributes();
      attributes.reserve( fields.count() );
      for ( int i = feature.attributes().count(); i < fields.count(); ++i )
      {
        attributes.append( QVariant( fields.at( i ).type() ) );
      }
      feature.setAttributes( attributes );
    }
  }
  feature.setFields( fields );
}

QgsFeatureList QgsVectorLayerUtils::makeFeatureCompatible( const QgsFeature &feature, const QgsVectorLayer *layer, QgsFeatureSink::SinkFlags sinkFlags )
{
  QgsWkbTypes::Type inputWkbType( layer->wkbType( ) );
  QgsFeatureList resultFeatures;
  QgsFeature newF( feature );
  // Fix attributes
  QgsVectorLayerUtils::matchAttributesToFields( newF, layer->fields( ) );

  if ( sinkFlags & QgsFeatureSink::RegeneratePrimaryKey )
  {
    // drop incoming primary key values, let them be regenerated
    const QgsAttributeList pkIndexes = layer->dataProvider()->pkAttributeIndexes();
    for ( int index : pkIndexes )
    {
      if ( index >= 0 )
        newF.setAttribute( index, QVariant() );
    }
  }

  // Does geometry need transformations?
  QgsWkbTypes::GeometryType newFGeomType( QgsWkbTypes::geometryType( newF.geometry().wkbType() ) );
  bool newFHasGeom = newFGeomType !=
                     QgsWkbTypes::GeometryType::UnknownGeometry &&
                     newFGeomType != QgsWkbTypes::GeometryType::NullGeometry;
  bool layerHasGeom = inputWkbType !=
                      QgsWkbTypes::Type::NoGeometry &&
                      inputWkbType != QgsWkbTypes::Type::Unknown;
  // Drop geometry if layer is geometry-less
  if ( ( newFHasGeom && !layerHasGeom ) || !newFHasGeom )
  {
    QgsFeature _f = QgsFeature( layer->fields() );
    _f.setAttributes( newF.attributes() );
    resultFeatures.append( _f );
  }
  else
  {
    // Geometry need fixing?
    const QVector< QgsGeometry > geometries = newF.geometry().coerceToType( inputWkbType );

    if ( geometries.count() != 1 )
    {
      QgsAttributeMap attrMap;
      for ( int j = 0; j < newF.fields().count(); j++ )
      {
        attrMap[j] = newF.attribute( j );
      }
      resultFeatures.reserve( geometries.size() );
      for ( const QgsGeometry &geometry : geometries )
      {
        QgsFeature _f( createFeature( layer, geometry, attrMap ) );
        resultFeatures.append( _f );
      }
    }
    else
    {
      newF.setGeometry( geometries.at( 0 ) );
      resultFeatures.append( newF );
    }
  }
  return resultFeatures;
}

QgsFeatureList QgsVectorLayerUtils::makeFeaturesCompatible( const QgsFeatureList &features, const QgsVectorLayer *layer, QgsFeatureSink::SinkFlags sinkFlags )
{
  QgsFeatureList resultFeatures;
  for ( const QgsFeature &f : features )
  {
    const QgsFeatureList features( makeFeatureCompatible( f, layer, sinkFlags ) );
    for ( const auto &_f : features )
    {
      resultFeatures.append( _f );
    }
  }
  return resultFeatures;
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

void QgsVectorLayerUtils::QgsDuplicateFeatureContext::setDuplicatedFeatures( QgsVectorLayer *layer, const QgsFeatureIds &ids )
{
  if ( mDuplicatedFeatures.contains( layer ) )
    mDuplicatedFeatures[layer] += ids;
  else
    mDuplicatedFeatures.insert( layer, ids );
}
/*
QMap<QgsVectorLayer *, QgsFeatureIds>  QgsVectorLayerUtils::QgsDuplicateFeatureContext::duplicateFeatureContext() const
{
  return mDuplicatedFeatures;
}
*/

QgsVectorLayerUtils::QgsFeatureData::QgsFeatureData( const QgsGeometry &geometry, const QgsAttributeMap &attributes ):
  mGeometry( geometry ),
  mAttributes( attributes )
{}

QgsGeometry QgsVectorLayerUtils::QgsFeatureData::geometry() const
{
  return mGeometry;
}

QgsAttributeMap QgsVectorLayerUtils::QgsFeatureData::attributes() const
{
  return mAttributes;
}

bool _fieldIsEditable( const QgsVectorLayer *layer, int fieldIndex, const QgsFeature &feature )
{
  return layer->isEditable() &&
         !layer->editFormConfig().readOnly( fieldIndex ) &&
         // Provider permissions
         layer->dataProvider() &&
         ( ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) ||
           ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures  && ( FID_IS_NULL( feature.id() ) || FID_IS_NEW( feature.id() ) ) ) )  &&
         // Field must not be read only
         !layer->fields().at( fieldIndex ).isReadOnly();
}

bool QgsVectorLayerUtils::fieldIsReadOnly( const QgsVectorLayer *layer, int fieldIndex )
{
  if ( layer->fields().fieldOrigin( fieldIndex ) == QgsFields::OriginJoin )
  {
    int srcFieldIndex;
    const QgsVectorLayerJoinInfo *info = layer->joinBuffer()->joinForFieldIndex( fieldIndex, layer->fields(), srcFieldIndex );

    if ( !info || !info->isEditable() || !info->joinLayer() )
      return true;

    return fieldIsReadOnly( info->joinLayer(), srcFieldIndex );
  }
  else
  {
    // any of these properties makes the field read only
    if ( !layer->isEditable() ||
         layer->editFormConfig().readOnly( fieldIndex ) ||
         !layer->dataProvider() ||
         ( !( layer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues )
           && !( layer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures ) ) ||
         layer->fields().at( fieldIndex ).isReadOnly() )
      return true;

    return false;
  }
}

bool QgsVectorLayerUtils::fieldEditabilityDependsOnFeature( const QgsVectorLayer *layer, int fieldIndex )
{
  // editability will vary feature-by-feature only for joined fields
  if ( layer->fields().fieldOrigin( fieldIndex ) == QgsFields::OriginJoin )
  {
    int srcFieldIndex;
    const QgsVectorLayerJoinInfo *info = layer->joinBuffer()->joinForFieldIndex( fieldIndex, layer->fields(), srcFieldIndex );

    if ( !info || !info->isEditable() || info->hasUpsertOnEdit() )
      return false;

    // join does not have upsert capabilities, so the ability to edit the joined field will
    // vary feature-by-feature, depending on whether the join target feature already exists
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsVectorLayerUtils::fieldIsEditable( const QgsVectorLayer *layer, int fieldIndex, const QgsFeature &feature )
{
  if ( layer->fields().fieldOrigin( fieldIndex ) == QgsFields::OriginJoin )
  {
    int srcFieldIndex;
    const QgsVectorLayerJoinInfo *info = layer->joinBuffer()->joinForFieldIndex( fieldIndex, layer->fields(), srcFieldIndex );

    if ( !info || !info->isEditable() )
      return false;

    // check that joined feature exist, else it is not editable
    if ( !info->hasUpsertOnEdit() )
    {
      const QgsFeature joinedFeature = layer->joinBuffer()->joinedFeatureOf( info, feature );
      if ( !joinedFeature.isValid() )
        return false;
    }

    return _fieldIsEditable( info->joinLayer(), srcFieldIndex, feature );
  }
  else
    return _fieldIsEditable( layer, fieldIndex, feature );
}


QHash<QString, QgsMaskedLayers> QgsVectorLayerUtils::labelMasks( const QgsVectorLayer *layer )
{
  class LabelMasksVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type == QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
        {
          currentRule = node.identifier;
          return true;
        }
        return false;
      }
      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( leaf.entity && leaf.entity->type() == QgsStyle::LabelSettingsEntity )
        {
          auto labelSettingsEntity = static_cast<const QgsStyleLabelSettingsEntity *>( leaf.entity );
          const QgsTextMaskSettings &maskSettings = labelSettingsEntity->settings().format().mask();
          if ( maskSettings.enabled() )
          {
            // transparency is considered has effects because it implies rasterization when masking
            // is involved
            const bool hasEffects = maskSettings.opacity() < 1 ||
                                    ( maskSettings.paintEffect() && maskSettings.paintEffect()->enabled() );
            for ( const auto &r : maskSettings.maskedSymbolLayers() )
            {
              QgsMaskedLayer &maskedLayer = maskedLayers[currentRule][r.layerId()];
              maskedLayer.symbolLayerIds.insert( r.symbolLayerId() );
              maskedLayer.hasEffects = hasEffects;
            }
          }
        }
        return true;
      }

      QHash<QString, QgsMaskedLayers> maskedLayers;
      // Current label rule, empty string for a simple labeling
      QString currentRule;
  };

  if ( ! layer->labeling() )
    return {};

  LabelMasksVisitor visitor;
  layer->labeling()->accept( &visitor );
  return std::move( visitor.maskedLayers );
}

QgsMaskedLayers QgsVectorLayerUtils::symbolLayerMasks( const QgsVectorLayer *layer )
{
  if ( ! layer->renderer() )
    return {};

  class SymbolLayerVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        return ( node.type == QgsStyleEntityVisitorInterface::NodeType::SymbolRule );
      }

      // Returns true if the visited symbol has effects
      bool visitSymbol( const QgsSymbol *symbol )
      {
        // transparency is considered has effects because it implies rasterization when masking
        // is involved
        bool symbolHasEffect = symbol->opacity() < 1;
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          const QgsSymbolLayer *sl = symbol->symbolLayer( idx );
          bool slHasEffects = sl->paintEffect() && sl->paintEffect()->enabled();
          symbolHasEffect |= slHasEffects;

          // recurse over sub symbols
          const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol();
          if ( subSymbol )
            slHasEffects |= visitSymbol( subSymbol );

          for ( const auto &mask : sl->masks() )
          {
            QgsMaskedLayer &maskedLayer = maskedLayers[mask.layerId()];
            maskedLayer.hasEffects |= slHasEffects;
            maskedLayer.symbolLayerIds.insert( mask.symbolLayerId() );
          }
        }

        return symbolHasEffect;
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( leaf.entity && leaf.entity->type() == QgsStyle::SymbolEntity )
        {
          auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
          if ( symbolEntity->symbol() )
            visitSymbol( symbolEntity->symbol() );
        }
        return true;
      }
      QgsMaskedLayers maskedLayers;
  };

  SymbolLayerVisitor visitor;
  layer->renderer()->accept( &visitor );
  return visitor.maskedLayers;
}

QString QgsVectorLayerUtils::getFeatureDisplayString( const QgsVectorLayer *layer, const QgsFeature &feature )
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );

  QgsExpression exp( layer->displayExpression() );
  context.setFeature( feature );
  exp.prepare( &context );
  QString displayString = exp.evaluate( &context ).toString();

  return displayString;
}

bool QgsVectorLayerUtils::impactsCascadeFeatures( const QgsVectorLayer *layer, const QgsFeatureIds &fids, const QgsProject *project, QgsDuplicateFeatureContext &context, CascadedFeatureFlags flags )
{
  if ( !layer )
    return false;

  const QList<QgsRelation> relations = project->relationManager()->referencedRelations( layer );
  for ( const QgsRelation &relation : relations )
  {
    switch ( relation.strength() )
    {
      case Qgis::RelationshipStrength::Composition:
      {
        QgsFeatureIds childFeatureIds;

        const auto constFids = fids;
        for ( const QgsFeatureId fid : constFids )
        {
          //get features connected over this relation
          QgsFeatureIterator relatedFeaturesIt = relation.getRelatedFeatures( layer->getFeature( fid ) );
          QgsFeature childFeature;
          while ( relatedFeaturesIt.nextFeature( childFeature ) )
          {
            childFeatureIds.insert( childFeature.id() );
          }
        }

        if ( childFeatureIds.count() > 0 )
        {
          if ( context.layers().contains( relation.referencingLayer() ) )
          {
            QgsFeatureIds handledFeatureIds = context.duplicatedFeatures( relation.referencingLayer() );
            // add feature ids
            handledFeatureIds.unite( childFeatureIds );
            context.setDuplicatedFeatures( relation.referencingLayer(), handledFeatureIds );
          }
          else
          {
            // add layer and feature id
            context.setDuplicatedFeatures( relation.referencingLayer(), childFeatureIds );
          }
        }
        break;
      }

      case Qgis::RelationshipStrength::Association:
        break;
    }
  }

  if ( layer->joinBuffer()->containsJoins() )
  {
    const QgsVectorJoinList joins = layer->joinBuffer()->vectorJoins();
    for ( const QgsVectorLayerJoinInfo &info : joins )
    {
      if ( qobject_cast< QgsAuxiliaryLayer * >( info.joinLayer() ) && flags & IgnoreAuxiliaryLayers )
        continue;

      if ( info.isEditable() && info.hasCascadedDelete() )
      {
        QgsFeatureIds joinFeatureIds;
        const auto constFids = fids;
        for ( const QgsFeatureId &fid : constFids )
        {
          const QgsFeature joinFeature = layer->joinBuffer()->joinedFeatureOf( &info, layer->getFeature( fid ) );
          if ( joinFeature.isValid() )
            joinFeatureIds.insert( joinFeature.id() );
        }

        if ( joinFeatureIds.count() > 0 )
        {
          if ( context.layers().contains( info.joinLayer() ) )
          {
            QgsFeatureIds handledFeatureIds = context.duplicatedFeatures( info.joinLayer() );
            // add feature ids
            handledFeatureIds.unite( joinFeatureIds );
            context.setDuplicatedFeatures( info.joinLayer(), handledFeatureIds );
          }
          else
          {
            // add layer and feature id
            context.setDuplicatedFeatures( info.joinLayer(), joinFeatureIds );
          }
        }
      }
    }
  }

  return !context.layers().isEmpty();
}

QString QgsVectorLayerUtils::guessFriendlyIdentifierField( const QgsFields &fields, bool *foundFriendly )
{
  if ( foundFriendly )
    *foundFriendly = false;

  if ( fields.isEmpty() )
    return QString();

  // Check the fields and keep the first one that matches.
  // We assume that the user has organized the data with the
  // more "interesting" field names first. As such, name should
  // be selected before oldname, othername, etc.
  // This candidates list is a prioritized list of candidates ranked by "interestingness"!
  // See discussion at https://github.com/qgis/QGIS/pull/30245 - this list must NOT be translated,
  // but adding hardcoded localized variants of the strings is encouraged.
  static QStringList sCandidates{ QStringLiteral( "name" ),
                                  QStringLiteral( "title" ),
                                  QStringLiteral( "heibt" ),
                                  QStringLiteral( "desc" ),
                                  QStringLiteral( "nom" ),
                                  QStringLiteral( "street" ),
                                  QStringLiteral( "road" ),
                                  QStringLiteral( "label" ) };

  // anti-names
  // this list of strings indicates parts of field names which make the name "less interesting".
  // For instance, we'd normally like to default to a field called "name" or "id", but if instead we
  // find one called "typename" or "typeid", then that's most likely a classification of the feature and not the
  // best choice to default to
  static QStringList sAntiCandidates{ QStringLiteral( "type" ),
                                      QStringLiteral( "class" ),
                                      QStringLiteral( "cat" )
                                    };

  QString bestCandidateName;
  QString bestCandidateNameWithAntiCandidate;

  for ( const QString &candidate : sCandidates )
  {
    for ( const QgsField &field : fields )
    {
      const QString fldName = field.name();
      if ( fldName.contains( candidate, Qt::CaseInsensitive ) )
      {
        bool isAntiCandidate = false;
        for ( const QString &antiCandidate : sAntiCandidates )
        {
          if ( fldName.contains( antiCandidate, Qt::CaseInsensitive ) )
          {
            isAntiCandidate = true;
            break;
          }
        }

        if ( isAntiCandidate )
        {
          if ( bestCandidateNameWithAntiCandidate.isEmpty() )
          {
            bestCandidateNameWithAntiCandidate = fldName;
          }
        }
        else
        {
          bestCandidateName = fldName;
          break;
        }
      }
    }

    if ( !bestCandidateName.isEmpty() )
      break;
  }

  const QString candidateName = bestCandidateName.isEmpty() ? bestCandidateNameWithAntiCandidate : bestCandidateName;
  if ( !candidateName.isEmpty() )
  {
    if ( foundFriendly )
      *foundFriendly = true;
    return candidateName;
  }
  else
  {
    // no good matches found by name, so scan through and look for the first string field
    for ( const QgsField &field : fields )
    {
      if ( field.type() == QVariant::String )
        return field.name();
    }

    // no string fields found - just return first field
    return fields.at( 0 ).name();
  }
}
