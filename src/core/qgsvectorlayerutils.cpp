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
#include "qgsgeometrycollection.h"

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
  Q_FOREACH ( const QVariant &value, variantValues )
  {
    double val = value.toDouble( &convertOk );
    if ( convertOk )
      values << val;
    else if ( value.isNull() )
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

QgsFeature QgsVectorLayerUtils::createFeature( const QgsVectorLayer *layer, const QgsGeometry &geometry,
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
    if ( v.isValid() && attributes.contains( idx ) && !layer->primaryKeyAttributes().contains(idx) )
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
        const auto pairs = relation.fieldPairs();
        for ( const QgsRelation::FieldPair &fieldPair : pairs )
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

std::unique_ptr<QgsVectorLayerFeatureSource> QgsVectorLayerUtils::getFeatureSource( QPointer<QgsVectorLayer> layer )
{
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;

  auto getFeatureSource = [ layer, &featureSource ]
  {
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
    Q_ASSERT( QThread::currentThread() == qApp->thread() );
#endif
    QgsVectorLayer *lyr = layer.data();

    if ( lyr )
    {
      featureSource.reset( new QgsVectorLayerFeatureSource( lyr ) );
    }
  };

  QgsThreadingUtils::runOnMainThread( getFeatureSource );

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
}

QgsFeatureList QgsVectorLayerUtils::makeFeatureCompatible( const QgsFeature &feature, const QgsVectorLayer *layer )
{
  QgsWkbTypes::Type inputWkbType( layer->wkbType( ) );
  QgsFeatureList resultFeatures;
  QgsFeature newF( feature );
  // Fix attributes
  QgsVectorLayerUtils::matchAttributesToFields( newF, layer->fields( ) );
  // Does geometry need transformations?
  QgsWkbTypes::GeometryType newFGeomType( QgsWkbTypes::geometryType( newF.geometry().wkbType() ) );
  bool newFHasGeom = newFGeomType !=
                     QgsWkbTypes::GeometryType::UnknownGeometry &&
                     newFGeomType != QgsWkbTypes::GeometryType::NullGeometry;
  bool layerHasGeom = inputWkbType !=
                      QgsWkbTypes::Type::NoGeometry &&
                      inputWkbType != QgsWkbTypes::Type::Unknown;
  // Drop geometry if layer is geometry-less
  if ( newFHasGeom && ! layerHasGeom )
  {
    QgsFeature _f = QgsFeature( layer->fields() );
    _f.setAttributes( newF.attributes() );
    resultFeatures.append( _f );
  }
  else
  {
    // Geometry need fixing
    if ( newFHasGeom && layerHasGeom && newF.geometry().wkbType() != inputWkbType )
    {
      // Single -> multi
      if ( QgsWkbTypes::isMultiType( inputWkbType ) && ! newF.geometry().isMultipart( ) )
      {
        QgsGeometry newGeom( newF.geometry( ) );
        newGeom.convertToMultiType();
        newF.setGeometry( newGeom );
      }
      // Drop Z/M
      if ( newF.geometry().constGet()->is3D() && ! QgsWkbTypes::hasZ( inputWkbType ) )
      {
        QgsGeometry newGeom( newF.geometry( ) );
        newGeom.get()->dropZValue();
        newF.setGeometry( newGeom );
      }
      if ( newF.geometry().constGet()->isMeasure() && ! QgsWkbTypes::hasM( inputWkbType ) )
      {
        QgsGeometry newGeom( newF.geometry( ) );
        newGeom.get()->dropMValue();
        newF.setGeometry( newGeom );
      }
      // Add Z/M back, set to 0
      if ( ! newF.geometry().constGet()->is3D() && QgsWkbTypes::hasZ( inputWkbType ) )
      {
        QgsGeometry newGeom( newF.geometry( ) );
        newGeom.get()->addZValue( 0.0 );
        newF.setGeometry( newGeom );
      }
      if ( ! newF.geometry().constGet()->isMeasure() && QgsWkbTypes::hasM( inputWkbType ) )
      {
        QgsGeometry newGeom( newF.geometry( ) );
        newGeom.get()->addMValue( 0.0 );
        newF.setGeometry( newGeom );
      }
      // Multi -> single
      if ( ! QgsWkbTypes::isMultiType( inputWkbType ) && newF.geometry().isMultipart( ) )
      {
        QgsGeometry newGeom( newF.geometry( ) );
        const QgsGeometryCollection *parts( static_cast< const QgsGeometryCollection * >( newGeom.constGet() ) );
        QgsAttributeMap attrMap;
        for ( int j = 0; j < newF.fields().count(); j++ )
        {
          attrMap[j] = newF.attribute( j );
        }
        for ( int i = 0; i < parts->partCount( ); i++ )
        {
          QgsGeometry g( parts->geometryN( i )->clone() );
          QgsFeature _f( createFeature( layer, g, attrMap ) );
          resultFeatures.append( _f );
        }
      }
      else
      {
        resultFeatures.append( newF );
      }
    }
    else
    {
      resultFeatures.append( newF );
    }
  }
  return resultFeatures;
}

QgsFeatureList QgsVectorLayerUtils::makeFeaturesCompatible( const QgsFeatureList &features, const QgsVectorLayer *layer )
{
  QgsFeatureList resultFeatures;
  for ( const QgsFeature &f : features )
  {
    const QgsFeatureList features( makeFeatureCompatible( f, layer ) );
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
  mDuplicatedFeatures.insert( layer, ids );
}
/*
QMap<QgsVectorLayer *, QgsFeatureIds>  QgsVectorLayerUtils::QgsDuplicateFeatureContext::duplicateFeatureContext() const
{
  return mDuplicatedFeatures;
}
*/
