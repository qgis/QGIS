/***************************************************************************
    qgsgraduatedsymbolrenderer.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgraduatedsymbolrenderer.h"

#include <cmath>
#include <ctime>
#include <memory>

#include "qgsapplication.h"
#include "qgsattributes.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsclassificationcustom.h"
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationmethod.h"
#include "qgsclassificationmethodregistry.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgspainteffect.h"
#include "qgspointdistancerenderer.h"
#include "qgsproperty.h"
#include "qgssldexportcontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"

#include <QDomDocument>
#include <QDomElement>

QgsGraduatedSymbolRenderer::QgsGraduatedSymbolRenderer( const QString &attrName, const QgsRangeList &ranges )
  : QgsFeatureRenderer( u"graduatedSymbol"_s )
  , mAttrName( attrName )
{
  // TODO: check ranges for sanity (NULL symbols, invalid ranges)

  //important - we need a deep copy of the ranges list, not a shared copy. This is required because
  //QgsRendererRange::symbol() is marked const, and so retrieving the symbol via this method does not
  //trigger a detachment and copy of mRanges BUT that same method CAN be used to modify a symbol in place
  const auto constRanges = ranges;
  for ( const QgsRendererRange &range : constRanges )
  {
    mRanges << range;
  }

  mClassificationMethod = std::make_shared<QgsClassificationCustom>( );
}

QgsGraduatedSymbolRenderer::~QgsGraduatedSymbolRenderer()
{
  mRanges.clear(); // should delete all the symbols
}

Qgis::FeatureRendererFlags QgsGraduatedSymbolRenderer::flags() const
{
  Qgis::FeatureRendererFlags res;
  auto catIt = mRanges.constBegin();
  for ( ; catIt != mRanges.constEnd(); ++catIt )
  {
    if ( QgsSymbol *catSymbol = catIt->symbol() )
    {
      if ( catSymbol->flags().testFlag( Qgis::SymbolFlag::AffectsLabeling ) )
        res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
    }
  }

  return res;
}

const QgsRendererRange *QgsGraduatedSymbolRenderer::rangeForValue( double value ) const
{
  for ( const QgsRendererRange &range : mRanges )
  {
    if ( range.lowerValue() <= value && range.upperValue() >= value )
    {
      if ( range.renderState() || mCounting )
        return &range;
      else
        return nullptr;
    }
  }

  // second chance -- use a bit of double tolerance to avoid floating point equality fuzziness
  // if a value falls just outside of a range, but within acceptable double precision tolerance
  // then we accept it anyway
  for ( const QgsRendererRange &range : mRanges )
  {
    if ( qgsDoubleNear( range.lowerValue(), value ) || qgsDoubleNear( range.upperValue(), value ) )
    {
      if ( range.renderState() || mCounting )
        return &range;
      else
        return nullptr;
    }
  }
  // the value is out of the range: return NULL instead of symbol
  return nullptr;
}

QgsSymbol *QgsGraduatedSymbolRenderer::symbolForValue( double value ) const
{
  if ( const QgsRendererRange *range = rangeForValue( value ) )
    return range->symbol();
  return nullptr;
}

QString QgsGraduatedSymbolRenderer::legendKeyForValue( double value ) const
{
  if ( const QgsRendererRange *matchingRange = rangeForValue( value ) )
  {
    for ( const QgsRendererRange &range : mRanges )
    {
      if ( matchingRange == &range )
        return range.uuid();
    }
  }
  return QString();
}

QgsSymbol *QgsGraduatedSymbolRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return originalSymbolForFeature( feature, context );
}

QVariant QgsGraduatedSymbolRenderer::valueForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QgsAttributes attrs = feature.attributes();
  QVariant value;
  if ( mExpression )
  {
    value = mExpression->evaluate( &context.expressionContext() );
  }
  else
  {
    value = attrs.value( mAttrNum );
  }

  return value;
}

QgsSymbol *QgsGraduatedSymbolRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QVariant value = valueForFeature( feature, context );

  // Null values should not be categorized
  if ( QgsVariantUtils::isNull( value ) )
    return nullptr;

  // find the right category
  return symbolForValue( value.toDouble() );
}

void QgsGraduatedSymbolRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  mCounting = context.rendererScale() == 0.0;

  // find out classification attribute index from name
  mAttrNum = fields.lookupField( mAttrName );

  if ( mAttrNum == -1 )
  {
    mExpression = std::make_unique<QgsExpression>( mAttrName );
    mExpression->prepare( &context.expressionContext() );
  }

  for ( const QgsRendererRange &range : std::as_const( mRanges ) )
  {
    if ( !range.symbol() )
      continue;

    range.symbol()->startRender( context, fields );
  }
}

void QgsGraduatedSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  for ( const QgsRendererRange &range : std::as_const( mRanges ) )
  {
    if ( !range.symbol() )
      continue;

    range.symbol()->stopRender( context );
  }
}

QSet<QString> QgsGraduatedSymbolRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes;

  // mAttrName can contain either attribute name or an expression.
  // Sometimes it is not possible to distinguish between those two,
  // e.g. "a - b" can be both a valid attribute name or expression.
  // Since we do not have access to fields here, try both options.
  attributes << mAttrName;

  QgsExpression testExpr( mAttrName );
  if ( !testExpr.hasParserError() )
    attributes.unite( testExpr.referencedColumns() );

  QgsRangeList::const_iterator range_it = mRanges.constBegin();
  for ( ; range_it != mRanges.constEnd(); ++range_it )
  {
    QgsSymbol *symbol = range_it->symbol();
    if ( symbol )
    {
      attributes.unite( symbol->usedAttributes( context ) );
    }
  }
  return attributes;
}

bool QgsGraduatedSymbolRenderer::filterNeedsGeometry() const
{
  QgsExpression testExpr( mAttrName );
  if ( !testExpr.hasParserError() )
  {
    QgsExpressionContext context;
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) ); // unfortunately no layer access available!
    testExpr.prepare( &context );
    return testExpr.needsGeometry();
  }
  return false;
}

bool QgsGraduatedSymbolRenderer::updateRangeSymbol( int rangeIndex, QgsSymbol *symbol )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setSymbol( symbol );
  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeLabel( int rangeIndex, const QString &label )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setLabel( label );
  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeUpperValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  QgsRendererRange &range = mRanges[rangeIndex];
  QgsClassificationMethod::ClassPosition pos = QgsClassificationMethod::Inner;
  if ( rangeIndex == 0 )
    pos = QgsClassificationMethod::LowerBound;
  else if ( rangeIndex == mRanges.count() )
    pos = QgsClassificationMethod::UpperBound;

  bool isDefaultLabel = mClassificationMethod->labelForRange( range, pos ) == range.label();
  range.setUpperValue( value );
  if ( isDefaultLabel )
    range.setLabel( mClassificationMethod->labelForRange( range, pos ) );

  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeLowerValue( int rangeIndex, double value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;

  QgsRendererRange &range = mRanges[rangeIndex];
  QgsClassificationMethod::ClassPosition pos = QgsClassificationMethod::Inner;
  if ( rangeIndex == 0 )
    pos = QgsClassificationMethod::LowerBound;
  else if ( rangeIndex == mRanges.count() )
    pos = QgsClassificationMethod::UpperBound;

  bool isDefaultLabel = mClassificationMethod->labelForRange( range, pos ) == range.label();
  range.setLowerValue( value );
  if ( isDefaultLabel )
    range.setLabel( mClassificationMethod->labelForRange( range, pos ) );

  return true;
}

bool QgsGraduatedSymbolRenderer::updateRangeRenderState( int rangeIndex, bool value )
{
  if ( rangeIndex < 0 || rangeIndex >= mRanges.size() )
    return false;
  mRanges[rangeIndex].setRenderState( value );
  return true;
}

QString QgsGraduatedSymbolRenderer::dump() const
{
  QString s = u"GRADUATED: attr %1\n"_s.arg( mAttrName );
  for ( int i = 0; i < mRanges.count(); i++ )
    s += mRanges[i].dump();
  return s;
}

QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::clone() const
{
  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( mAttrName, mRanges );

  r->setClassificationMethod( mClassificationMethod->clone().release() );

  if ( mSourceSymbol )
    r->setSourceSymbol( mSourceSymbol->clone() );
  if ( mSourceColorRamp )
  {
    r->setSourceColorRamp( mSourceColorRamp->clone() );
  }
  r->setDataDefinedSizeLegend( mDataDefinedSizeLegend ? new QgsDataDefinedSizeLegend( *mDataDefinedSizeLegend ) : nullptr );
  r->setGraduatedMethod( graduatedMethod() );
  copyRendererData( r );
  return r;
}

void QgsGraduatedSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsGraduatedSymbolRenderer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  const QVariantMap oldProps = context.extraProperties();
  QVariantMap newProps = oldProps;
  newProps[ u"attribute"_s] = mAttrName;
  newProps[ u"method"_s] = graduatedMethodStr( mGraduatedMethod );
  context.setExtraProperties( newProps );

  // create a Rule for each range
  bool first = true;
  bool result = true;
  for ( QgsRangeList::const_iterator it = mRanges.constBegin(); it != mRanges.constEnd(); ++it )
  {
    if ( !it->toSld( doc, element, mAttrName, context, first ) )
      result = false;
    first = false;
  }
  context.setExtraProperties( oldProps );
  return result;
}

QgsSymbolList QgsGraduatedSymbolRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  QgsSymbolList lst;
  lst.reserve( mRanges.count() );
  for ( const QgsRendererRange &range : std::as_const( mRanges ) )
  {
    lst.append( range.symbol() );
  }
  return lst;
}

bool QgsGraduatedSymbolRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  for ( const QgsRendererRange &range : std::as_const( mRanges ) )
  {
    QgsStyleSymbolEntity entity( range.symbol() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"%1 - %2"_s.arg( range.lowerValue() ).arg( range.upperValue() ), range.label() ) ) )
      return false;
  }

  if ( mSourceColorRamp )
  {
    QgsStyleColorRampEntity entity( mSourceColorRamp.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) ) )
      return false;
  }

  return true;
}

void QgsGraduatedSymbolRenderer::makeBreaksSymmetric( QList<double> &breaks, double symmetryPoint, bool astride )
{
  return QgsClassificationMethod::makeBreaksSymmetric( breaks, symmetryPoint, astride );
}

QList<double> QgsGraduatedSymbolRenderer::calcEqualIntervalBreaks( double minimum, double maximum, int classes, bool useSymmetricMode, double symmetryPoint, bool astride )
{
  QgsClassificationEqualInterval method;
  method.setSymmetricMode( useSymmetricMode, symmetryPoint, astride );
  QList<QgsClassificationRange> _classes = method.classes( minimum, maximum, classes );
  return QgsClassificationMethod::rangesToBreaks( _classes );
}

Q_NOWARN_DEPRECATED_PUSH
QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::createRenderer(
  QgsVectorLayer *vlayer,
  const QString &attrName,
  int classes,
  Mode mode,
  QgsSymbol *symbol,
  QgsColorRamp *ramp,
  const QgsRendererRangeLabelFormat &labelFormat,
  bool useSymmetricMode,
  double symmetryPoint,
  const QStringList &listForCboPrettyBreaks,
  bool astride
)
{
  Q_UNUSED( listForCboPrettyBreaks )

  QgsRangeList ranges;
  auto r = std::make_unique< QgsGraduatedSymbolRenderer >( attrName, ranges );
  r->setSourceSymbol( symbol->clone() );
  r->setSourceColorRamp( ramp->clone() );

  QString methodId = methodIdFromMode( mode );
  std::unique_ptr< QgsClassificationMethod > method = QgsApplication::classificationMethodRegistry()->method( methodId );

  if ( method )
  {
    method->setSymmetricMode( useSymmetricMode, symmetryPoint, astride );
    method->setLabelFormat( labelFormat.format() );
    method->setLabelTrimTrailingZeroes( labelFormat.trimTrailingZeroes() );
    method->setLabelPrecision( labelFormat.precision() );
  }
  r->setClassificationMethod( method.release() );

  QString error;
  r->updateClasses( vlayer, classes, error );
  ( void )error;

  return r.release();
}
Q_NOWARN_DEPRECATED_POP

void QgsGraduatedSymbolRenderer::updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses,
    bool useSymmetricMode, double symmetryPoint, bool astride )
{
  if ( mAttrName.isEmpty() )
    return;

  QString methodId = methodIdFromMode( mode );
  std::unique_ptr< QgsClassificationMethod > method = QgsApplication::classificationMethodRegistry()->method( methodId );
  method->setSymmetricMode( useSymmetricMode, symmetryPoint, astride );
  setClassificationMethod( method.release() );

  QString error;
  updateClasses( vlayer, nclasses, error );
  ( void )error;
}

void QgsGraduatedSymbolRenderer::updateClasses( const QgsVectorLayer *vl, int nclasses, QString &error )
{
  Q_UNUSED( error )
  if ( mClassificationMethod->id() == QgsClassificationCustom::METHOD_ID )
    return;

  QList<QgsClassificationRange> classes = mClassificationMethod->classesV2( vl, mAttrName, nclasses, error );

  deleteAllClasses();

  for ( QList<QgsClassificationRange>::iterator it = classes.begin(); it != classes.end(); ++it )
  {
    QgsSymbol *newSymbol = mSourceSymbol ? mSourceSymbol->clone() : QgsSymbol::defaultSymbol( vl->geometryType() );
    addClass( QgsRendererRange( *it, newSymbol ) );
  }
  updateColorRamp( nullptr );
}

Q_NOWARN_DEPRECATED_PUSH
QgsRendererRangeLabelFormat QgsGraduatedSymbolRenderer::labelFormat() const
{
  return QgsRendererRangeLabelFormat( mClassificationMethod->labelFormat(), mClassificationMethod->labelPrecision(), mClassificationMethod->labelTrimTrailingZeroes() );
}
Q_NOWARN_DEPRECATED_POP

QgsFeatureRenderer *QgsGraduatedSymbolRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement symbolsElem = element.firstChildElement( u"symbols"_s );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement rangesElem = element.firstChildElement( u"ranges"_s );
  if ( rangesElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
  QgsRangeList ranges;

  QDomElement rangeElem = rangesElem.firstChildElement();
  int i = 0;
  QSet<QString> usedUuids;
  while ( !rangeElem.isNull() )
  {
    if ( rangeElem.tagName() == "range"_L1 )
    {
      double lowerValue = rangeElem.attribute( u"lower"_s ).toDouble();
      double upperValue = rangeElem.attribute( u"upper"_s ).toDouble();
      QString symbolName = rangeElem.attribute( u"symbol"_s );
      QString label = rangeElem.attribute( u"label"_s );
      bool render = rangeElem.attribute( u"render"_s, u"true"_s ) != "false"_L1;
      QString uuid = rangeElem.attribute( u"uuid"_s, QString::number( i++ ) );
      while ( usedUuids.contains( uuid ) )
      {
        uuid = QUuid::createUuid().toString();
      }
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbol *symbol = symbolMap.take( symbolName );
        ranges.append( QgsRendererRange( lowerValue, upperValue, symbol, label, render, uuid ) );
        usedUuids << uuid;
      }
    }
    rangeElem = rangeElem.nextSiblingElement();
  }

  QString attrName = element.attribute( u"attr"_s );

  auto r = std::make_unique< QgsGraduatedSymbolRenderer >( attrName, ranges );

  QString attrMethod = element.attribute( u"graduatedMethod"_s );
  if ( !attrMethod.isEmpty() )
  {
    if ( attrMethod == graduatedMethodStr( Qgis::GraduatedMethod::Color ) )
      r->setGraduatedMethod( Qgis::GraduatedMethod::Color );
    else if ( attrMethod == graduatedMethodStr( Qgis::GraduatedMethod::Size ) )
      r->setGraduatedMethod( Qgis::GraduatedMethod::Size );
  }


  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  // try to load source symbol (optional)
  QDomElement sourceSymbolElem = element.firstChildElement( u"source-symbol"_s );
  if ( !sourceSymbolElem.isNull() )
  {
    QgsSymbolMap sourceSymbolMap = QgsSymbolLayerUtils::loadSymbols( sourceSymbolElem, context );
    if ( sourceSymbolMap.contains( u"0"_s ) )
    {
      r->setSourceSymbol( sourceSymbolMap.take( u"0"_s ) );
    }
    QgsSymbolLayerUtils::clearSymbolMap( sourceSymbolMap );
  }

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = element.firstChildElement( u"colorramp"_s );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( u"name"_s ) == "[source]"_L1 )
  {
    r->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ).release() );
  }

  // try to load mode

  QDomElement modeElem = element.firstChildElement( u"mode"_s ); // old format,  backward compatibility
  QDomElement methodElem = element.firstChildElement( u"classificationMethod"_s );
  std::unique_ptr< QgsClassificationMethod > method;

  // TODO QGIS 5 Remove
  // backward compatibility for QGIS project < 3.10
  if ( !modeElem.isNull() )
  {
    QString modeString = modeElem.attribute( u"name"_s );
    QString methodId;
    // the strings saved in the project does not match with the old Mode enum
    if ( modeString == "equal"_L1 )
      methodId = u"EqualInterval"_s;
    else if ( modeString == "quantile"_L1 )
      methodId = u"Quantile"_s;
    else if ( modeString == "jenks"_L1 )
      methodId = u"Jenks"_s;
    else if ( modeString == "stddev"_L1 )
      methodId = u"StdDev"_s;
    else if ( modeString == "pretty"_L1 )
      methodId = u"Pretty"_s;

    method = QgsApplication::classificationMethodRegistry()->method( methodId );

    // symmetric mode
    QDomElement symmetricModeElem = element.firstChildElement( u"symmetricMode"_s );
    if ( !symmetricModeElem.isNull() )
    {
      // symmetry
      QString symmetricEnabled = symmetricModeElem.attribute( u"enabled"_s );
      QString symmetricPointString = symmetricModeElem.attribute( u"symmetryPoint"_s );
      QString astrideEnabled = symmetricModeElem.attribute( u"astride"_s );
      method->setSymmetricMode( symmetricEnabled == "true"_L1, symmetricPointString.toDouble(), astrideEnabled == "true"_L1 );
    }
    QDomElement labelFormatElem = element.firstChildElement( u"labelformat"_s );
    if ( !labelFormatElem.isNull() )
    {
      // label format
      QString format = labelFormatElem.attribute( u"format"_s, "%1" + u" - "_s + "%2" );
      int precision = labelFormatElem.attribute( u"decimalplaces"_s, u"4"_s ).toInt();
      bool trimTrailingZeroes = labelFormatElem.attribute( u"trimtrailingzeroes"_s, u"false"_s ) == "true"_L1;
      method->setLabelFormat( format );
      method->setLabelPrecision( precision );
      method->setLabelTrimTrailingZeroes( trimTrailingZeroes );
    }
    // End of backward compatibility
  }
  else
  {
    // QGIS project 3.10+
    method = QgsClassificationMethod::create( methodElem, context );
  }

  // apply the method
  r->setClassificationMethod( method.release() );

  QDomElement rotationElem = element.firstChildElement( u"rotation"_s );
  if ( !rotationElem.isNull() && !rotationElem.attribute( u"field"_s ).isEmpty() )
  {
    for ( const QgsRendererRange &range : std::as_const( r->mRanges ) )
    {
      convertSymbolRotation( range.symbol(), rotationElem.attribute( u"field"_s ) );
    }
    if ( r->mSourceSymbol )
    {
      convertSymbolRotation( r->mSourceSymbol.get(), rotationElem.attribute( u"field"_s ) );
    }
  }
  QDomElement sizeScaleElem = element.firstChildElement( u"sizescale"_s );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( u"field"_s ).isEmpty() )
  {
    for ( const QgsRendererRange &range : std::as_const( r->mRanges ) )
    {
      convertSymbolSizeScale( range.symbol(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( u"scalemethod"_s ) ),
                              sizeScaleElem.attribute( u"field"_s ) );
    }
    if ( r->mSourceSymbol && r->mSourceSymbol->type() == Qgis::SymbolType::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.get(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( u"scalemethod"_s ) ),
                              sizeScaleElem.attribute( u"field"_s ) );
    }
  }

  QDomElement ddsLegendSizeElem = element.firstChildElement( u"data-defined-size-legend"_s );
  if ( !ddsLegendSizeElem.isNull() )
  {
    r->mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }
// TODO: symbol levels
  return r.release();
}

QDomElement QgsGraduatedSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( u"type"_s, u"graduatedSymbol"_s );
  rendererElem.setAttribute( u"attr"_s, mAttrName );
  rendererElem.setAttribute( u"graduatedMethod"_s, graduatedMethodStr( mGraduatedMethod ) );

  // ranges
  int i = 0;
  QgsSymbolMap symbols;
  QDomElement rangesElem = doc.createElement( u"ranges"_s );
  QgsRangeList::const_iterator it = mRanges.constBegin();
  for ( ; it != mRanges.constEnd(); ++it )
  {
    const QgsRendererRange &range = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, range.symbol() );

    QDomElement rangeElem = doc.createElement( u"range"_s );
    rangeElem.setAttribute( u"lower"_s, QString::number( range.lowerValue(), 'f', 15 ) );
    rangeElem.setAttribute( u"upper"_s, QString::number( range.upperValue(), 'f', 15 ) );
    rangeElem.setAttribute( u"symbol"_s, symbolName );
    rangeElem.setAttribute( u"label"_s, range.label() );
    rangeElem.setAttribute( u"render"_s, range.renderState() ? u"true"_s : u"false"_s );
    rangeElem.setAttribute( u"uuid"_s, range.uuid() );
    rangesElem.appendChild( rangeElem );
    i++;
  }

  rendererElem.appendChild( rangesElem );

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, u"symbols"_s, doc, context );
  rendererElem.appendChild( symbolsElem );

  // save source symbol
  if ( mSourceSymbol )
  {
    QgsSymbolMap sourceSymbols;
    sourceSymbols.insert( u"0"_s, mSourceSymbol.get() );
    QDomElement sourceSymbolElem = QgsSymbolLayerUtils::saveSymbols( sourceSymbols, u"source-symbol"_s, doc, context );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( u"[source]"_s, mSourceColorRamp.get(), doc );
    rendererElem.appendChild( colorRampElem );
  }

  // save classification method
  QDomElement classificationMethodElem = mClassificationMethod->save( doc, context );
  rendererElem.appendChild( classificationMethodElem );

  QDomElement rotationElem = doc.createElement( u"rotation"_s );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( u"sizescale"_s );
  rendererElem.appendChild( sizeScaleElem );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( u"data-defined-size-legend"_s );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsLegendSymbolList QgsGraduatedSymbolRenderer::baseLegendSymbolItems() const
{
  QgsLegendSymbolList lst;
  lst.reserve( mRanges.size() );
  for ( const QgsRendererRange &range : mRanges )
  {
    lst << QgsLegendSymbolItem( range.symbol(), range.label(), range.uuid(), true );
  }
  return lst;
}

Q_NOWARN_DEPRECATED_PUSH
QString QgsGraduatedSymbolRenderer::methodIdFromMode( QgsGraduatedSymbolRenderer::Mode mode )
{
  switch ( mode )
  {
    case EqualInterval:
      return u"EqualInterval"_s;
    case Quantile:
      return u"Quantile"_s;
    case Jenks:
      return u"Jenks"_s;
    case StdDev:
      return u"StdDev"_s;
    case Pretty:
      return u"Pretty"_s;
    case Custom:
      return QString();
  }
  return QString();
}

QgsGraduatedSymbolRenderer::Mode QgsGraduatedSymbolRenderer::modeFromMethodId( const QString &methodId )
{
  if ( methodId == "EqualInterval"_L1 )
    return EqualInterval;
  if ( methodId == "Quantile"_L1 )
    return Quantile;
  if ( methodId == "Jenks"_L1 )
    return Jenks;
  if ( methodId == "StdDev"_L1 )
    return StdDev;
  if ( methodId == "Pretty"_L1 )
    return Pretty;
  else
    return Custom;
}
Q_NOWARN_DEPRECATED_POP

QgsLegendSymbolList QgsGraduatedSymbolRenderer::legendSymbolItems() const
{
  if ( mDataDefinedSizeLegend && mSourceSymbol && mSourceSymbol->type() == Qgis::SymbolType::Marker )
  {
    // check that all symbols that have the same size expression
    QgsProperty ddSize;
    for ( const QgsRendererRange &range : mRanges )
    {
      const QgsMarkerSymbol *symbol = static_cast<const QgsMarkerSymbol *>( range.symbol() );
      if ( ddSize )
      {
        QgsProperty sSize( symbol->dataDefinedSize() );
        if ( sSize && sSize != ddSize )
        {
          // no common size expression
          return baseLegendSymbolItems();
        }
      }
      else
      {
        ddSize = symbol->dataDefinedSize();
      }
    }

    if ( ddSize && ddSize.isActive() )
    {
      QgsLegendSymbolList lst;

      QgsDataDefinedSizeLegend ddSizeLegend( *mDataDefinedSizeLegend );
      ddSizeLegend.updateFromSymbolAndProperty( static_cast<const QgsMarkerSymbol *>( mSourceSymbol.get() ), ddSize );
      lst += ddSizeLegend.legendSymbolList();

      lst += baseLegendSymbolItems();
      return lst;
    }
  }

  return baseLegendSymbolItems();
}

QSet< QString > QgsGraduatedSymbolRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  QVariant value = valueForFeature( feature, context );

  // Null values should not be categorized
  if ( QgsVariantUtils::isNull( value ) )
    return QSet< QString >();

  // find the right category
  QString key = legendKeyForValue( value.toDouble() );
  if ( !key.isNull() )
    return QSet< QString >() << key;
  else
    return QSet< QString >();
}

QString QgsGraduatedSymbolRenderer::legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const
{
  ok = false;
  int i = 0;
  for ( i = 0; i < mRanges.size(); i++ )
  {
    if ( mRanges[i].uuid() == key )
    {
      ok = true;
      break;
    }
  }

  if ( !ok )
  {
    ok = false;
    return QString();
  }

  const QString attributeComponent = QgsExpression::quoteFieldExpression( mAttrName, layer );
  const QgsRendererRange &range = mRanges[i];

  return u"(%1 >= %2) AND (%1 <= %3)"_s.arg( attributeComponent, QgsExpression::quotedValue( range.lowerValue(), QMetaType::Type::Double ),
         QgsExpression::quotedValue( range.upperValue(), QMetaType::Type::Double ) );
}

QgsSymbol *QgsGraduatedSymbolRenderer::sourceSymbol()
{
  return mSourceSymbol.get();
}

const QgsSymbol *QgsGraduatedSymbolRenderer::sourceSymbol() const
{
  return mSourceSymbol.get();
}

void QgsGraduatedSymbolRenderer::setSourceSymbol( QgsSymbol *sym )
{
  mSourceSymbol.reset( sym );
}

QgsColorRamp *QgsGraduatedSymbolRenderer::sourceColorRamp()
{
  return mSourceColorRamp.get();
}

const QgsColorRamp *QgsGraduatedSymbolRenderer::sourceColorRamp() const
{
  return mSourceColorRamp.get();
}

void QgsGraduatedSymbolRenderer::setSourceColorRamp( QgsColorRamp *ramp )
{
  if ( ramp == mSourceColorRamp.get() )
    return;

  mSourceColorRamp.reset( ramp );
}

double QgsGraduatedSymbolRenderer::minSymbolSize() const
{
  double min = std::numeric_limits<double>::max();
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    double sz = 0;
    if ( mRanges[i].symbol()->type() == Qgis::SymbolType::Marker )
      sz = static_cast< QgsMarkerSymbol * >( mRanges[i].symbol() )->size();
    else if ( mRanges[i].symbol()->type() == Qgis::SymbolType::Line )
      sz = static_cast< QgsLineSymbol * >( mRanges[i].symbol() )->width();
    min = std::min( sz, min );
  }
  return min;
}

double QgsGraduatedSymbolRenderer::maxSymbolSize() const
{
  double max = std::numeric_limits<double>::min();
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    double sz = 0;
    if ( mRanges[i].symbol()->type() == Qgis::SymbolType::Marker )
      sz = static_cast< QgsMarkerSymbol * >( mRanges[i].symbol() )->size();
    else if ( mRanges[i].symbol()->type() == Qgis::SymbolType::Line )
      sz = static_cast< QgsLineSymbol * >( mRanges[i].symbol() )->width();
    max = std::max( sz, max );
  }
  return max;
}

void QgsGraduatedSymbolRenderer::setSymbolSizes( double minSize, double maxSize )
{
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    std::unique_ptr<QgsSymbol> symbol( mRanges.at( i ).symbol() ? mRanges.at( i ).symbol()->clone() : nullptr );
    const double size = mRanges.count() > 1
                        ? minSize + i * ( maxSize - minSize ) / ( mRanges.count() - 1 )
                        : .5 * ( maxSize + minSize );
    if ( symbol )
    {
      switch ( symbol->type() )
      {
        case Qgis::SymbolType::Marker:
          static_cast< QgsMarkerSymbol * >( symbol.get() )->setSize( size );
          break;
        case Qgis::SymbolType::Line:
          static_cast< QgsLineSymbol * >( symbol.get() )->setWidth( size );
          break;
        case Qgis::SymbolType::Fill:
        case Qgis::SymbolType::Hybrid:
          break;
      }
      updateRangeSymbol( i, symbol.release() );
    }
  }
}

void QgsGraduatedSymbolRenderer::updateColorRamp( QgsColorRamp *ramp )
{
  int i = 0;
  if ( ramp )
  {
    setSourceColorRamp( ramp );
  }

  if ( mSourceColorRamp )
  {
    for ( const QgsRendererRange &range : std::as_const( mRanges ) )
    {
      QgsSymbol *symbol = range.symbol() ? range.symbol()->clone() : nullptr;
      if ( symbol )
      {
        double colorValue;
        colorValue = ( mRanges.count() > 1 ? static_cast< double >( i ) / ( mRanges.count() - 1 ) : 0 );
        symbol->setColor( mSourceColorRamp->color( colorValue ) );
      }
      updateRangeSymbol( i, symbol );
      ++i;
    }
  }

}

void QgsGraduatedSymbolRenderer::updateSymbols( QgsSymbol *sym )
{
  if ( !sym )
    return;

  int i = 0;
  for ( const QgsRendererRange &range : std::as_const( mRanges ) )
  {
    std::unique_ptr<QgsSymbol> symbol( sym->clone() );
    switch ( mGraduatedMethod )
    {
      case Qgis::GraduatedMethod::Color:
      {
        symbol->setColor( range.symbol()->color() );
        break;
      }
      case Qgis::GraduatedMethod::Size:
      {
        if ( symbol->type() == Qgis::SymbolType::Marker )
          static_cast<QgsMarkerSymbol *>( symbol.get() )->setSize(
            static_cast<QgsMarkerSymbol *>( range.symbol() )->size() );
        else if ( symbol->type() == Qgis::SymbolType::Line )
          static_cast<QgsLineSymbol *>( symbol.get() )->setWidth(
            static_cast<QgsLineSymbol *>( range.symbol() )->width() );
        break;
      }
    }
    updateRangeSymbol( i, symbol.release() );
    ++i;
  }
  setSourceSymbol( sym->clone() );
}

bool QgsGraduatedSymbolRenderer::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsGraduatedSymbolRenderer::legendSymbolItemChecked( const QString &key )
{
  for ( const QgsRendererRange &range : std::as_const( mRanges ) )
  {
    if ( range.uuid() == key )
    {
      return range.renderState();
    }
  }
  return true;
}

void QgsGraduatedSymbolRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  for ( int i = 0; i < mRanges.size(); i++ )
  {
    if ( mRanges[i].uuid() == key )
    {
      updateRangeRenderState( i, state );
      break;
    }
  }
}

void QgsGraduatedSymbolRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  bool ok = false;
  int i = 0;
  for ( i = 0; i < mRanges.size(); i++ )
  {
    if ( mRanges[i].uuid() == key )
    {
      ok = true;
      break;
    }
  }

  if ( ok )
    updateRangeSymbol( i, symbol );
  else
    delete symbol;
}

void QgsGraduatedSymbolRenderer::addClass( QgsSymbol *symbol )
{
  QgsSymbol *newSymbol = symbol->clone();
  QString label = u"0.0 - 0.0"_s;
  mRanges.insert( 0, QgsRendererRange( 0.0, 0.0, newSymbol, label ) );
}

void QgsGraduatedSymbolRenderer::addClass( double lower, double upper )
{
  QgsSymbol *newSymbol = mSourceSymbol->clone();
  QString label = mClassificationMethod->labelForRange( lower, upper );
  mRanges.append( QgsRendererRange( lower, upper, newSymbol, label ) );
}

void QgsGraduatedSymbolRenderer::addBreak( double breakValue, bool updateSymbols )
{
  QMutableListIterator< QgsRendererRange > it( mRanges );
  while ( it.hasNext() )
  {
    QgsRendererRange range = it.next();
    if ( range.lowerValue() < breakValue && range.upperValue() > breakValue )
    {
      QgsRendererRange newRange = QgsRendererRange();
      newRange.setLowerValue( breakValue );
      newRange.setUpperValue( range.upperValue() );
      newRange.setLabel( mClassificationMethod->labelForRange( newRange ) );
      newRange.setSymbol( mSourceSymbol->clone() );

      //update old range
      bool isDefaultLabel = range.label() == mClassificationMethod->labelForRange( range );
      range.setUpperValue( breakValue );
      if ( isDefaultLabel )
        range.setLabel( mClassificationMethod->labelForRange( range.lowerValue(), breakValue ) );
      it.setValue( range );

      it.insert( newRange );
      break;
    }
  }

  if ( updateSymbols )
  {
    switch ( mGraduatedMethod )
    {
      case Qgis::GraduatedMethod::Color:
        updateColorRamp( mSourceColorRamp.get() );
        break;
      case Qgis::GraduatedMethod::Size:
        setSymbolSizes( minSymbolSize(), maxSymbolSize() );
        break;
    }
  }
}

void QgsGraduatedSymbolRenderer::addClass( const QgsRendererRange &range )
{
  mRanges.append( range );
}

void QgsGraduatedSymbolRenderer::deleteClass( int idx )
{
  mRanges.removeAt( idx );
}

void QgsGraduatedSymbolRenderer::deleteAllClasses()
{
  mRanges.clear();
}

Q_NOWARN_DEPRECATED_PUSH
void QgsGraduatedSymbolRenderer::setLabelFormat( const QgsRendererRangeLabelFormat &labelFormat, bool updateRanges )
{
  mClassificationMethod->setLabelFormat( labelFormat.format() );
  mClassificationMethod->setLabelPrecision( labelFormat.precision() );
  mClassificationMethod->setLabelTrimTrailingZeroes( labelFormat.trimTrailingZeroes() );

  if ( updateRanges )
  {
    updateRangeLabels();
  }
}
Q_NOWARN_DEPRECATED_POP

void QgsGraduatedSymbolRenderer::updateRangeLabels()
{
  for ( int i = 0; i < mRanges.count(); i++ )
  {
    QgsClassificationMethod::ClassPosition pos = QgsClassificationMethod::Inner;
    if ( i == 0 )
      pos = QgsClassificationMethod::LowerBound;
    else if ( i == mRanges.count() - 1 )
      pos = QgsClassificationMethod::UpperBound;
    mRanges[i].setLabel( mClassificationMethod->labelForRange( mRanges[i], pos ) );
  }
}

void QgsGraduatedSymbolRenderer::calculateLabelPrecision( bool updateRanges )
{
  // Find the minimum size of a class
  double minClassRange = 0.0;
  for ( const QgsRendererRange &rendererRange : std::as_const( mRanges ) )
  {
    double range = rendererRange.upperValue() - rendererRange.lowerValue();
    if ( range <= 0.0 )
      continue;
    if ( minClassRange == 0.0 || range < minClassRange )
      minClassRange = range;
  }
  if ( minClassRange <= 0.0 )
    return;

  // Now set the number of decimal places to ensure no more than 20% error in
  // representing this range (up to 10% at upper and lower end)

  int ndp = 10;
  double nextDpMinRange = 0.0000000099;
  while ( ndp > 0 && nextDpMinRange < minClassRange )
  {
    ndp--;
    nextDpMinRange *= 10.0;
  }
  mClassificationMethod->setLabelPrecision( ndp );
  if ( updateRanges )
    updateRangeLabels();
}

void QgsGraduatedSymbolRenderer::moveClass( int from, int to )
{
  if ( from < 0 || from >= mRanges.size() || to < 0 || to >= mRanges.size() )
    return;
  mRanges.move( from, to );
}

bool valueLessThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return r1 < r2;
}

bool valueGreaterThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return !valueLessThan( r1, r2 );
}

void QgsGraduatedSymbolRenderer::sortByValue( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    std::sort( mRanges.begin(), mRanges.end(), valueLessThan );
  }
  else
  {
    std::sort( mRanges.begin(), mRanges.end(), valueGreaterThan );
  }
}

bool QgsGraduatedSymbolRenderer::rangesOverlap() const
{
  QgsRangeList sortedRanges = mRanges;
  std::sort( sortedRanges.begin(), sortedRanges.end(), valueLessThan );

  QgsRangeList::const_iterator it = sortedRanges.constBegin();
  if ( it == sortedRanges.constEnd() )
    return false;

  if ( ( *it ).upperValue() < ( *it ).lowerValue() )
    return true;

  double prevMax = ( *it ).upperValue();
  ++it;

  for ( ; it != sortedRanges.constEnd(); ++it )
  {
    if ( ( *it ).upperValue() < ( *it ).lowerValue() )
      return true;

    if ( ( *it ).lowerValue() < prevMax )
      return true;

    prevMax = ( *it ).upperValue();
  }
  return false;
}

bool QgsGraduatedSymbolRenderer::rangesHaveGaps() const
{
  QgsRangeList sortedRanges = mRanges;
  std::sort( sortedRanges.begin(), sortedRanges.end(), valueLessThan );

  QgsRangeList::const_iterator it = sortedRanges.constBegin();
  if ( it == sortedRanges.constEnd() )
    return false;

  double prevMax = ( *it ).upperValue();
  ++it;

  for ( ; it != sortedRanges.constEnd(); ++it )
  {
    if ( !qgsDoubleNear( ( *it ).lowerValue(), prevMax ) )
      return true;

    prevMax = ( *it ).upperValue();
  }
  return false;
}

bool labelLessThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return QString::localeAwareCompare( r1.label(), r2.label() ) < 0;
}

bool labelGreaterThan( const QgsRendererRange &r1, const QgsRendererRange &r2 )
{
  return !labelLessThan( r1, r2 );
}

void QgsGraduatedSymbolRenderer::sortByLabel( Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    std::sort( mRanges.begin(), mRanges.end(), labelLessThan );
  }
  else
  {
    std::sort( mRanges.begin(), mRanges.end(), labelGreaterThan );
  }
}

QgsClassificationMethod *QgsGraduatedSymbolRenderer::classificationMethod() const
{
  return mClassificationMethod.get();
}

void QgsGraduatedSymbolRenderer::setClassificationMethod( QgsClassificationMethod *method )
{
  mClassificationMethod.reset( method );
}

void QgsGraduatedSymbolRenderer::setMode( QgsGraduatedSymbolRenderer::Mode mode )
{
  QString methodId = methodIdFromMode( mode );
  std::unique_ptr< QgsClassificationMethod > method = QgsApplication::classificationMethodRegistry()->method( methodId );
  setClassificationMethod( method.release() );
}

void QgsGraduatedSymbolRenderer::setUseSymmetricMode( bool useSymmetricMode ) SIP_DEPRECATED
{
  mClassificationMethod->setSymmetricMode( useSymmetricMode, mClassificationMethod->symmetryPoint(), mClassificationMethod->symmetryAstride() );
}

void QgsGraduatedSymbolRenderer::setSymmetryPoint( double symmetryPoint ) SIP_DEPRECATED
{
  mClassificationMethod->setSymmetricMode( mClassificationMethod->symmetricModeEnabled(), symmetryPoint, mClassificationMethod->symmetryAstride() );
}

void QgsGraduatedSymbolRenderer::setAstride( bool astride ) SIP_DEPRECATED
{
  mClassificationMethod->setSymmetricMode( mClassificationMethod->symmetricModeEnabled(), mClassificationMethod->symmetryPoint(), astride );
}

QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  std::unique_ptr< QgsGraduatedSymbolRenderer > r;
  if ( renderer->type() == "graduatedSymbol"_L1 )
  {
    r.reset( static_cast<QgsGraduatedSymbolRenderer *>( renderer->clone() ) );
  }
  else if ( renderer->type() == "categorizedSymbol"_L1 )
  {
    const QgsCategorizedSymbolRenderer *categorizedSymbolRenderer = dynamic_cast<const QgsCategorizedSymbolRenderer *>( renderer );
    if ( categorizedSymbolRenderer )
    {
      r = std::make_unique< QgsGraduatedSymbolRenderer >( QString(), QgsRangeList() );
      if ( categorizedSymbolRenderer->sourceSymbol() )
        r->setSourceSymbol( categorizedSymbolRenderer->sourceSymbol()->clone() );
      if ( categorizedSymbolRenderer->sourceColorRamp() )
      {
        bool isRandom = dynamic_cast<const QgsRandomColorRamp *>( categorizedSymbolRenderer->sourceColorRamp() ) ||
                        dynamic_cast<const QgsLimitedRandomColorRamp *>( categorizedSymbolRenderer->sourceColorRamp() );
        if ( !isRandom )
          r->setSourceColorRamp( categorizedSymbolRenderer->sourceColorRamp()->clone() );
      }
      r->setClassAttribute( categorizedSymbolRenderer->classAttribute() );
    }
  }
  else if ( renderer->type() == "pointDisplacement"_L1 || renderer->type() == "pointCluster"_L1 )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r.reset( convertFromRenderer( pointDistanceRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == "invertedPolygonRenderer"_L1 )
  {
    const QgsInvertedPolygonRenderer *invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer *>( renderer );
    if ( invertedPolygonRenderer )
      r.reset( convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() ) );
  }

  // If not one of the specifically handled renderers, then just grab the symbol from the renderer
  // Could have applied this to specific renderer types (singleSymbol, graduatedSymbol)

  if ( !r )
  {
    r = std::make_unique< QgsGraduatedSymbolRenderer >( QString(), QgsRangeList() );
    QgsRenderContext context;
    QgsSymbolList symbols = const_cast<QgsFeatureRenderer *>( renderer )->symbols( context );
    if ( !symbols.isEmpty() )
    {
      r->setSourceSymbol( symbols.at( 0 )->clone() );
    }
  }

  renderer->copyRendererData( r.get() );

  return r.release();
}

void QgsGraduatedSymbolRenderer::setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings )
{
  mDataDefinedSizeLegend.reset( settings );
}

QgsDataDefinedSizeLegend *QgsGraduatedSymbolRenderer::dataDefinedSizeLegend() const
{
  return mDataDefinedSizeLegend.get();
}

QString QgsGraduatedSymbolRenderer::graduatedMethodStr( Qgis::GraduatedMethod method )
{
  switch ( method )
  {
    case Qgis::GraduatedMethod::Color:
      return u"GraduatedColor"_s;
    case Qgis::GraduatedMethod::Size:
      return u"GraduatedSize"_s;
  }
  return QString();
}


