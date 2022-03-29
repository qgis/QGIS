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

#include <QDomDocument>
#include <QDomElement>

#include <ctime>
#include <cmath>

#include "qgsgraduatedsymbolrenderer.h"

#include "qgsattributes.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsdatadefinedsizelegend.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgslogger.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsproperty.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgsclassificationmethod.h"
#include "qgsclassificationequalinterval.h"
#include "qgsapplication.h"
#include "qgsclassificationmethodregistry.h"
#include "qgsclassificationcustom.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

QgsGraduatedSymbolRenderer::QgsGraduatedSymbolRenderer( const QString &attrName, const QgsRangeList &ranges )
  : QgsFeatureRenderer( QStringLiteral( "graduatedSymbol" ) )
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

  mClassificationMethod.reset( new QgsClassificationCustom() );
}

QgsGraduatedSymbolRenderer::~QgsGraduatedSymbolRenderer()
{
  mRanges.clear(); // should delete all the symbols
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
    int i = 0;
    for ( const QgsRendererRange &range : mRanges )
    {
      if ( matchingRange == &range )
        return QString::number( i );
      i++;
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
  if ( value.isNull() )
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
    mExpression.reset( new QgsExpression( mAttrName ) );
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
  QString s = QStringLiteral( "GRADUATED: attr %1\n" ).arg( mAttrName );
  for ( int i = 0; i < mRanges.count(); i++ )
    s += mRanges[i].dump();
  return s;
}

QgsGraduatedSymbolRenderer *QgsGraduatedSymbolRenderer::clone() const
{
  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( mAttrName, mRanges );

  r->setClassificationMethod( mClassificationMethod->clone() );

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
  QVariantMap newProps = props;
  newProps[ QStringLiteral( "attribute" )] = mAttrName;
  newProps[ QStringLiteral( "method" )] = graduatedMethodStr( mGraduatedMethod );

  // create a Rule for each range
  bool first = true;
  for ( QgsRangeList::const_iterator it = mRanges.constBegin(); it != mRanges.constEnd(); ++it )
  {
    it->toSld( doc, element, newProps, first );
    first = false;
  }
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
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "%1 - %2" ).arg( range.lowerValue() ).arg( range.upperValue() ), range.label() ) ) )
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
  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( attrName, ranges );
  r->setSourceSymbol( symbol->clone() );
  r->setSourceColorRamp( ramp->clone() );

  QString methodId = methodIdFromMode( mode );
  QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method( methodId );

  if ( method )
  {
    method->setSymmetricMode( useSymmetricMode, symmetryPoint, astride );
    method->setLabelFormat( labelFormat.format() );
    method->setLabelTrimTrailingZeroes( labelFormat.trimTrailingZeroes() );
    method->setLabelPrecision( labelFormat.precision() );
  }
  r->setClassificationMethod( method );

  r->updateClasses( vlayer, classes );
  return r;
}
Q_NOWARN_DEPRECATED_POP

void QgsGraduatedSymbolRenderer::updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses,
    bool useSymmetricMode, double symmetryPoint, bool astride )
{
  if ( mAttrName.isEmpty() )
    return;

  QString methodId = methodIdFromMode( mode );
  QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method( methodId );
  method->setSymmetricMode( useSymmetricMode, symmetryPoint, astride );
  setClassificationMethod( method );

  updateClasses( vlayer, nclasses );
}

void QgsGraduatedSymbolRenderer::updateClasses( const QgsVectorLayer *vl, int nclasses )
{
  if ( mClassificationMethod->id() == QgsClassificationCustom::METHOD_ID )
    return;

  QList<QgsClassificationRange> classes = mClassificationMethod->classes( vl, mAttrName, nclasses );

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
  QDomElement symbolsElem = element.firstChildElement( QStringLiteral( "symbols" ) );
  if ( symbolsElem.isNull() )
    return nullptr;

  QDomElement rangesElem = element.firstChildElement( QStringLiteral( "ranges" ) );
  if ( rangesElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
  QgsRangeList ranges;

  QDomElement rangeElem = rangesElem.firstChildElement();
  while ( !rangeElem.isNull() )
  {
    if ( rangeElem.tagName() == QLatin1String( "range" ) )
    {
      double lowerValue = rangeElem.attribute( QStringLiteral( "lower" ) ).toDouble();
      double upperValue = rangeElem.attribute( QStringLiteral( "upper" ) ).toDouble();
      QString symbolName = rangeElem.attribute( QStringLiteral( "symbol" ) );
      QString label = rangeElem.attribute( QStringLiteral( "label" ) );
      bool render = rangeElem.attribute( QStringLiteral( "render" ), QStringLiteral( "true" ) ) != QLatin1String( "false" );
      if ( symbolMap.contains( symbolName ) )
      {
        QgsSymbol *symbol = symbolMap.take( symbolName );
        ranges.append( QgsRendererRange( lowerValue, upperValue, symbol, label, render ) );
      }
    }
    rangeElem = rangeElem.nextSiblingElement();
  }

  QString attrName = element.attribute( QStringLiteral( "attr" ) );

  QgsGraduatedSymbolRenderer *r = new QgsGraduatedSymbolRenderer( attrName, ranges );

  QString attrMethod = element.attribute( QStringLiteral( "graduatedMethod" ) );
  if ( !attrMethod.isEmpty() )
  {
    if ( attrMethod == graduatedMethodStr( GraduatedColor ) )
      r->setGraduatedMethod( GraduatedColor );
    else if ( attrMethod == graduatedMethodStr( GraduatedSize ) )
      r->setGraduatedMethod( GraduatedSize );
  }


  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  // try to load source symbol (optional)
  QDomElement sourceSymbolElem = element.firstChildElement( QStringLiteral( "source-symbol" ) );
  if ( !sourceSymbolElem.isNull() )
  {
    QgsSymbolMap sourceSymbolMap = QgsSymbolLayerUtils::loadSymbols( sourceSymbolElem, context );
    if ( sourceSymbolMap.contains( QStringLiteral( "0" ) ) )
    {
      r->setSourceSymbol( sourceSymbolMap.take( QStringLiteral( "0" ) ) );
    }
    QgsSymbolLayerUtils::clearSymbolMap( sourceSymbolMap );
  }

  // try to load color ramp (optional)
  QDomElement sourceColorRampElem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    r->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }

  // try to load mode

  QDomElement modeElem = element.firstChildElement( QStringLiteral( "mode" ) ); // old format,  backward compatibility
  QDomElement methodElem = element.firstChildElement( QStringLiteral( "classificationMethod" ) );
  QgsClassificationMethod *method = nullptr;

  // TODO QGIS 4 Remove
  // backward compatibility for QGIS project < 3.10
  if ( !modeElem.isNull() )
  {
    QString modeString = modeElem.attribute( QStringLiteral( "name" ) );
    QString methodId;
    // the strings saved in the project does not match with the old Mode enum
    if ( modeString == QLatin1String( "equal" ) )
      methodId = QStringLiteral( "EqualInterval" );
    else if ( modeString == QLatin1String( "quantile" ) )
      methodId = QStringLiteral( "Quantile" );
    else if ( modeString == QLatin1String( "jenks" ) )
      methodId = QStringLiteral( "Jenks" );
    else if ( modeString == QLatin1String( "stddev" ) )
      methodId = QStringLiteral( "StdDev" );
    else if ( modeString == QLatin1String( "pretty" ) )
      methodId = QStringLiteral( "Pretty" );

    method = QgsApplication::classificationMethodRegistry()->method( methodId );

    // symmetric mode
    QDomElement symmetricModeElem = element.firstChildElement( QStringLiteral( "symmetricMode" ) );
    if ( !symmetricModeElem.isNull() )
    {
      // symmetry
      QString symmetricEnabled = symmetricModeElem.attribute( QStringLiteral( "enabled" ) );
      QString symmetricPointString = symmetricModeElem.attribute( QStringLiteral( "symmetryPoint" ) );
      QString astrideEnabled = symmetricModeElem.attribute( QStringLiteral( "astride" ) );
      method->setSymmetricMode( symmetricEnabled == QLatin1String( "true" ), symmetricPointString.toDouble(), astrideEnabled == QLatin1String( "true" ) );
    }
    QDomElement labelFormatElem = element.firstChildElement( QStringLiteral( "labelformat" ) );
    if ( !labelFormatElem.isNull() )
    {
      // label format
      QString format = labelFormatElem.attribute( QStringLiteral( "format" ), "%1" + QStringLiteral( " - " ) + "%2" );
      int precision = labelFormatElem.attribute( QStringLiteral( "decimalplaces" ), QStringLiteral( "4" ) ).toInt();
      bool trimTrailingZeroes = labelFormatElem.attribute( QStringLiteral( "trimtrailingzeroes" ), QStringLiteral( "false" ) ) == QLatin1String( "true" );
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
  r->setClassificationMethod( method );

  QDomElement rotationElem = element.firstChildElement( QStringLiteral( "rotation" ) );
  if ( !rotationElem.isNull() && !rotationElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    for ( const QgsRendererRange &range : std::as_const( r->mRanges ) )
    {
      convertSymbolRotation( range.symbol(), rotationElem.attribute( QStringLiteral( "field" ) ) );
    }
    if ( r->mSourceSymbol )
    {
      convertSymbolRotation( r->mSourceSymbol.get(), rotationElem.attribute( QStringLiteral( "field" ) ) );
    }
  }
  QDomElement sizeScaleElem = element.firstChildElement( QStringLiteral( "sizescale" ) );
  if ( !sizeScaleElem.isNull() && !sizeScaleElem.attribute( QStringLiteral( "field" ) ).isEmpty() )
  {
    for ( const QgsRendererRange &range : std::as_const( r->mRanges ) )
    {
      convertSymbolSizeScale( range.symbol(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                              sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
    }
    if ( r->mSourceSymbol && r->mSourceSymbol->type() == Qgis::SymbolType::Marker )
    {
      convertSymbolSizeScale( r->mSourceSymbol.get(),
                              QgsSymbolLayerUtils::decodeScaleMethod( sizeScaleElem.attribute( QStringLiteral( "scalemethod" ) ) ),
                              sizeScaleElem.attribute( QStringLiteral( "field" ) ) );
    }
  }

  QDomElement ddsLegendSizeElem = element.firstChildElement( QStringLiteral( "data-defined-size-legend" ) );
  if ( !ddsLegendSizeElem.isNull() )
  {
    r->mDataDefinedSizeLegend.reset( QgsDataDefinedSizeLegend::readXml( ddsLegendSizeElem, context ) );
  }
// TODO: symbol levels
  return r;
}

QDomElement QgsGraduatedSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "graduatedSymbol" ) );
  rendererElem.setAttribute( QStringLiteral( "attr" ), mAttrName );
  rendererElem.setAttribute( QStringLiteral( "graduatedMethod" ), graduatedMethodStr( mGraduatedMethod ) );

  // ranges
  int i = 0;
  QgsSymbolMap symbols;
  QDomElement rangesElem = doc.createElement( QStringLiteral( "ranges" ) );
  QgsRangeList::const_iterator it = mRanges.constBegin();
  for ( ; it != mRanges.constEnd(); ++it )
  {
    const QgsRendererRange &range = *it;
    QString symbolName = QString::number( i );
    symbols.insert( symbolName, range.symbol() );

    QDomElement rangeElem = doc.createElement( QStringLiteral( "range" ) );
    rangeElem.setAttribute( QStringLiteral( "lower" ), QString::number( range.lowerValue(), 'f', 15 ) );
    rangeElem.setAttribute( QStringLiteral( "upper" ), QString::number( range.upperValue(), 'f', 15 ) );
    rangeElem.setAttribute( QStringLiteral( "symbol" ), symbolName );
    rangeElem.setAttribute( QStringLiteral( "label" ), range.label() );
    rangeElem.setAttribute( QStringLiteral( "render" ), range.renderState() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    rangesElem.appendChild( rangeElem );
    i++;
  }

  rendererElem.appendChild( rangesElem );

  // save symbols
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  rendererElem.appendChild( symbolsElem );

  // save source symbol
  if ( mSourceSymbol )
  {
    QgsSymbolMap sourceSymbols;
    sourceSymbols.insert( QStringLiteral( "0" ), mSourceSymbol.get() );
    QDomElement sourceSymbolElem = QgsSymbolLayerUtils::saveSymbols( sourceSymbols, QStringLiteral( "source-symbol" ), doc, context );
    rendererElem.appendChild( sourceSymbolElem );
  }

  // save source color ramp
  if ( mSourceColorRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mSourceColorRamp.get(), doc );
    rendererElem.appendChild( colorRampElem );
  }

  // save classification method
  QDomElement classificationMethodElem = mClassificationMethod->save( doc, context );
  rendererElem.appendChild( classificationMethodElem );

  QDomElement rotationElem = doc.createElement( QStringLiteral( "rotation" ) );
  rendererElem.appendChild( rotationElem );

  QDomElement sizeScaleElem = doc.createElement( QStringLiteral( "sizescale" ) );
  rendererElem.appendChild( sizeScaleElem );

  if ( mDataDefinedSizeLegend )
  {
    QDomElement ddsLegendElem = doc.createElement( QStringLiteral( "data-defined-size-legend" ) );
    mDataDefinedSizeLegend->writeXml( ddsLegendElem, context );
    rendererElem.appendChild( ddsLegendElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsLegendSymbolList QgsGraduatedSymbolRenderer::baseLegendSymbolItems() const
{
  QgsLegendSymbolList lst;
  int i = 0;
  lst.reserve( mRanges.size() );
  for ( const QgsRendererRange &range : mRanges )
  {
    lst << QgsLegendSymbolItem( range.symbol(), range.label(), QString::number( i++ ), true );
  }
  return lst;
}

Q_NOWARN_DEPRECATED_PUSH
QString QgsGraduatedSymbolRenderer::methodIdFromMode( QgsGraduatedSymbolRenderer::Mode mode )
{
  switch ( mode )
  {
    case EqualInterval:
      return QStringLiteral( "EqualInterval" );
    case Quantile:
      return QStringLiteral( "Quantile" );
    case Jenks:
      return QStringLiteral( "Jenks" );
    case StdDev:
      return QStringLiteral( "StdDev" );
    case Pretty:
      return QStringLiteral( "Pretty" );
    case Custom:
      return QString();
  }
  return QString();
}

QgsGraduatedSymbolRenderer::Mode QgsGraduatedSymbolRenderer::modeFromMethodId( const QString &methodId )
{
  if ( methodId == QLatin1String( "EqualInterval" ) )
    return EqualInterval;
  if ( methodId == QLatin1String( "Quantile" ) )
    return Quantile;
  if ( methodId == QLatin1String( "Jenks" ) )
    return Jenks;
  if ( methodId == QLatin1String( "StdDev" ) )
    return StdDev;
  if ( methodId == QLatin1String( "Pretty" ) )
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
  if ( value.isNull() )
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
  int ruleIndex = key.toInt( &ok );
  if ( !ok || ruleIndex < 0 || ruleIndex >= mRanges.size() )
  {
    ok = false;
    return QString();
  }

  const QString attributeComponent = QgsExpression::quoteFieldExpression( mAttrName, layer );

  ok = true;
  const QgsRendererRange &range = mRanges[ ruleIndex ];

  return QStringLiteral( "(%1 >= %2) AND (%1 <= %3)" ).arg( attributeComponent, QgsExpression::quotedValue( range.lowerValue(), QVariant::Double ),
         QgsExpression::quotedValue( range.upperValue(), QVariant::Double ) );
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
    if ( symbol->type() == Qgis::SymbolType::Marker )
      static_cast< QgsMarkerSymbol * >( symbol.get() )->setSize( size );
    if ( symbol->type() == Qgis::SymbolType::Line )
      static_cast< QgsLineSymbol * >( symbol.get() )->setWidth( size );
    updateRangeSymbol( i, symbol.release() );
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
    if ( mGraduatedMethod == GraduatedColor )
    {
      symbol->setColor( range.symbol()->color() );
    }
    else if ( mGraduatedMethod == GraduatedSize )
    {
      if ( symbol->type() == Qgis::SymbolType::Marker )
        static_cast<QgsMarkerSymbol *>( symbol.get() )->setSize(
          static_cast<QgsMarkerSymbol *>( range.symbol() )->size() );
      else if ( symbol->type() == Qgis::SymbolType::Line )
        static_cast<QgsLineSymbol *>( symbol.get() )->setWidth(
          static_cast<QgsLineSymbol *>( range.symbol() )->width() );
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
  bool ok;
  int index = key.toInt( &ok );
  if ( ok && index >= 0 && index < mRanges.size() )
    return mRanges.at( index ).renderState();
  else
    return true;
}

void QgsGraduatedSymbolRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateRangeRenderState( index, state );
}

void QgsGraduatedSymbolRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  bool ok;
  int index = key.toInt( &ok );
  if ( ok )
    updateRangeSymbol( index, symbol );
  else
    delete symbol;
}

void QgsGraduatedSymbolRenderer::addClass( QgsSymbol *symbol )
{
  QgsSymbol *newSymbol = symbol->clone();
  QString label = QStringLiteral( "0.0 - 0.0" );
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
      case GraduatedColor:
        updateColorRamp( mSourceColorRamp.get() );
        break;
      case GraduatedSize:
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
  QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method( methodId );
  setClassificationMethod( method );
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
  if ( renderer->type() == QLatin1String( "graduatedSymbol" ) )
  {
    r.reset( static_cast<QgsGraduatedSymbolRenderer *>( renderer->clone() ) );
  }
  else if ( renderer->type() == QLatin1String( "categorizedSymbol" ) )
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
  else if ( renderer->type() == QLatin1String( "pointDisplacement" ) || renderer->type() == QLatin1String( "pointCluster" ) )
  {
    const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer );
    if ( pointDistanceRenderer )
      r.reset( convertFromRenderer( pointDistanceRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
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

QString QgsGraduatedSymbolRenderer::graduatedMethodStr( GraduatedMethod method )
{
  switch ( method )
  {
    case GraduatedColor:
      return QStringLiteral( "GraduatedColor" );
    case GraduatedSize:
      return QStringLiteral( "GraduatedSize" );
  }
  return QString();
}


