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
#include "qgsexpressioncontextutils.h"
#include "qgsmultisurface.h"
#include "qgsgeometryfactory.h"
#include "qgscurvepolygon.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgspallabeling.h"
#include "qgsrenderer.h"
#include "qgssymbollayer.h"
#include "qgsstyleentityvisitor.h"
#include "qgsstyle.h"

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
    QVariant maxVal = existingValues.isEmpty() ? 0 : *std::max_element( existingValues.begin(), existingValues.end() );
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
          base = existingValues.isEmpty() ? QString() : existingValues.values().first().toString();
        }

        // try variants like base_1, base_2, etc until a new value found
        QStringList vals;
        for ( const auto &v : qgis::as_const( existingValues ) )
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
  QMap<int, QSet<QVariant>> uniqueValueCaches;

  for ( const auto &fd : qgis::as_const( featuresData ) )
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

      // Cache unique values
      if ( hasUniqueConstraint && ! uniqueValueCaches.contains( idx ) )
      {
        // If the layer is filtered, get unique values from an unfiltered clone
        if ( ! layer->subsetString().isEmpty() )
        {
          std::unique_ptr<QgsVectorLayer> unfilteredClone { layer->clone( ) };
          unfilteredClone->setSubsetString( QString( ) );
          uniqueValueCaches[ idx ] = unfilteredClone->uniqueValues( idx );
        }
        else
        {
          uniqueValueCaches[ idx ] = layer->uniqueValues( idx );
        }
      }

      // 2. client side default expression
      // note - deliberately not using else if!
      QgsDefaultValue defaultValueDefinition = layer->defaultValueDefinition( idx );
      if ( ( v.isNull() || ( hasUniqueConstraint
                             && uniqueValueCaches[ idx ].contains( v ) )
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
      if ( ( v.isNull() || ( hasUniqueConstraint
                             && uniqueValueCaches[ idx ].contains( v ) ) )
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
      if ( ( v.isNull() || ( checkUnique && hasUniqueConstraint
                             && uniqueValueCaches[ idx ].contains( v ) ) )
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
      if ( v.isNull() && fd.attributes().contains( idx ) )
      {
        v = fd.attributes().value( idx );
      }

      // last of all... check that unique constraints are respected
      // we can't handle not null or expression constraints here, since there's no way to pick a sensible
      // value if the constraint is violated
      if ( checkUnique && hasUniqueConstraint )
      {
        if ( uniqueValueCaches[ idx ].contains( v ) )
        {
          // unique constraint violated
          QVariant uniqueValue = QgsVectorLayerUtils::createUniqueValueFromCache( layer, idx, uniqueValueCaches[ idx ], v );
          if ( uniqueValue.isValid() )
            v = uniqueValue;
        }
      }
      if ( hasUniqueConstraint )
        uniqueValueCaches[ idx ].insert( v );
      newFeature.setAttribute( idx, v );
    }
    result.append( newFeature );
  }
  return result;
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

  QgsFeature newFeature = createFeature( layer, feature.geometry(), feature.attributes().toMap(), &context );

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

std::unique_ptr<QgsVectorLayerFeatureSource> QgsVectorLayerUtils::getFeatureSource( QPointer<QgsVectorLayer> layer, QgsFeedback *feedback )
{
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;

  auto getFeatureSource = [ layer, &featureSource, feedback ]
  {
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
    Q_ASSERT( QThread::currentThread() == qApp->thread() || feedback );
#else
    Q_UNUSED( feedback )
#endif
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
      // Curved -> straight
      if ( !QgsWkbTypes::isCurvedType( inputWkbType ) && QgsWkbTypes::isCurvedType( newF.geometry().wkbType() ) )
      {
        QgsGeometry newGeom( newF.geometry().constGet()->segmentize() );
        newF.setGeometry( newGeom );
      }

      // polygon -> line
      if ( QgsWkbTypes::geometryType( inputWkbType ) == QgsWkbTypes::LineGeometry &&
           newF.geometry().type() == QgsWkbTypes::PolygonGeometry )
      {
        // boundary gives us a (multi)line string of exterior + interior rings
        QgsGeometry newGeom( newF.geometry().constGet()->boundary() );
        newF.setGeometry( newGeom );
      }
      // line -> polygon
      if ( QgsWkbTypes::geometryType( inputWkbType ) == QgsWkbTypes::PolygonGeometry &&
           newF.geometry().type() == QgsWkbTypes::LineGeometry )
      {
        std::unique_ptr< QgsGeometryCollection > gc( QgsGeometryFactory::createCollectionOfType( inputWkbType ) );
        const QgsGeometry source = newF.geometry();
        for ( auto part = source.const_parts_begin(); part != source.const_parts_end(); ++part )
        {
          std::unique_ptr< QgsAbstractGeometry > exterior( ( *part )->clone() );
          if ( QgsCurve *curve = qgsgeometry_cast< QgsCurve * >( exterior.get() ) )
          {
            if ( QgsWkbTypes::isCurvedType( inputWkbType ) )
            {
              std::unique_ptr< QgsCurvePolygon > cp = qgis::make_unique< QgsCurvePolygon >();
              cp->setExteriorRing( curve );
              exterior.release();
              gc->addGeometry( cp.release() );
            }
            else
            {
              std::unique_ptr< QgsPolygon > p = qgis::make_unique< QgsPolygon  >();
              p->setExteriorRing( qgsgeometry_cast< QgsLineString * >( curve ) );
              exterior.release();
              gc->addGeometry( p.release() );
            }
          }
        }
        QgsGeometry newGeom( std::move( gc ) );
        newF.setGeometry( newGeom );
      }

      // line/polygon -> points
      if ( QgsWkbTypes::geometryType( inputWkbType ) == QgsWkbTypes::PointGeometry &&
           ( newF.geometry().type() == QgsWkbTypes::LineGeometry ||
             newF.geometry().type() == QgsWkbTypes::PolygonGeometry ) )
      {
        // lines/polygons to a point layer, extract all vertices
        std::unique_ptr< QgsMultiPoint > mp = qgis::make_unique< QgsMultiPoint >();
        const QgsGeometry source = newF.geometry();
        QSet< QgsPoint > added;
        for ( auto vertex = source.vertices_begin(); vertex != source.vertices_end(); ++vertex )
        {
          if ( added.contains( *vertex ) )
            continue; // avoid duplicate points, e.g. start/end of rings
          mp->addGeometry( ( *vertex ).clone() );
          added.insert( *vertex );
        }
        QgsGeometry newGeom( std::move( mp ) );
        newF.setGeometry( newGeom );
      }

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
        resultFeatures.reserve( parts->partCount() );
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
         ( ( layer->dataProvider() && layer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) || FID_IS_NEW( feature.id() ) );
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

QHash<QString, QHash<QString, QSet<QgsSymbolLayerId>>> QgsVectorLayerUtils::labelMasks( const QgsVectorLayer *layer )
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
          if ( labelSettingsEntity->settings().format().mask().enabled() )
          {
            for ( const auto &r : labelSettingsEntity->settings().format().mask().maskedSymbolLayers() )
            {
              masks[currentRule][r.layerId()].insert( r.symbolLayerId() );
            }
          }
        }
        return true;
      }

      QHash<QString, QHash<QString, QSet<QgsSymbolLayerId>>> masks;
      // Current label rule, empty string for a simple labeling
      QString currentRule;
  };

  if ( ! layer->labeling() )
    return {};

  LabelMasksVisitor visitor;
  layer->labeling()->accept( &visitor );
  return std::move( visitor.masks );
}

QHash<QString, QSet<QgsSymbolLayerId>> QgsVectorLayerUtils::symbolLayerMasks( const QgsVectorLayer *layer )
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

      void visitSymbol( const QgsSymbol *symbol )
      {
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          const QgsSymbolLayer *sl = symbol->symbolLayer( idx );
          for ( const auto &mask : sl->masks() )
          {
            masks[mask.layerId()].insert( mask.symbolLayerId() );
          }
          // recurse over sub symbols
          const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol();
          if ( subSymbol )
            visitSymbol( subSymbol );
        }
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
      QHash<QString, QSet<QgsSymbolLayerId>> masks;
  };

  SymbolLayerVisitor visitor;
  layer->renderer()->accept( &visitor );
  return visitor.masks;
}
