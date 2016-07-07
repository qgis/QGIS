/***************************************************************************
    qgsrulebasedrendererv2.cpp - Rule-based renderer (symbology-ng)
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrulebasedrendererv2.h"
#include "qgssymbollayerv2.h"
#include "qgsexpression.h"
#include "qgssymbollayerv2utils.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsogcutils.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsdatadefined.h"

#include <QSet>

#include <QDomDocument>
#include <QDomElement>
#include <QUuid>


QgsRuleBasedRendererV2::Rule::Rule( QgsSymbolV2* symbol, int scaleMinDenom, int scaleMaxDenom, const QString& filterExp, const QString& label, const QString& description, bool elseRule )
    : mParent( nullptr )
    , mSymbol( symbol )
    , mScaleMinDenom( scaleMinDenom )
    , mScaleMaxDenom( scaleMaxDenom )
    , mFilterExp( filterExp )
    , mLabel( label )
    , mDescription( description )
    , mElseRule( elseRule )
    , mIsActive( true )
    , mFilter( nullptr )
{
  if ( mElseRule )
    mFilterExp = "ELSE";

  mRuleKey = QUuid::createUuid().toString();
  initFilter();
}

QgsRuleBasedRendererV2::Rule::~Rule()
{
  delete mSymbol;
  delete mFilter;
  qDeleteAll( mChildren );
  // do NOT delete parent
}

void QgsRuleBasedRendererV2::Rule::initFilter()
{
  if ( mFilterExp.trimmed().compare( "ELSE", Qt::CaseInsensitive ) == 0 )
  {
    mElseRule = true;
    delete mFilter;
    mFilter = nullptr;
  }
  else if ( mFilterExp.trimmed().isEmpty() )
  {
    mElseRule = false;
    delete mFilter;
    mFilter = nullptr;
  }
  else
  {
    mElseRule = false;
    delete mFilter;
    mFilter = new QgsExpression( mFilterExp );
  }
}

void QgsRuleBasedRendererV2::Rule::appendChild( Rule* rule )
{
  mChildren.append( rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBasedRendererV2::Rule::insertChild( int i, Rule* rule )
{
  mChildren.insert( i, rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBasedRendererV2::Rule::removeChild( Rule* rule )
{
  mChildren.removeAll( rule );
  delete rule;
  updateElseRules();
}

void QgsRuleBasedRendererV2::Rule::removeChildAt( int i )
{
  delete mChildren.takeAt( i );
  updateElseRules();
}

QgsRuleBasedRendererV2::Rule*  QgsRuleBasedRendererV2::Rule::takeChild( Rule* rule )
{
  mChildren.removeAll( rule );
  rule->mParent = nullptr;
  updateElseRules();
  return rule;
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::Rule::takeChildAt( int i )
{
  Rule* rule = mChildren.takeAt( i );
  rule->mParent = nullptr;
  updateElseRules();
  return rule;
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::Rule::findRuleByKey( const QString& key )
{
  // we could use a hash / map for search if this will be slow...

  if ( key == mRuleKey )
    return this;

  Q_FOREACH ( Rule* rule, mChildren )
  {
    Rule* r = rule->findRuleByKey( key );
    if ( r )
      return r;
  }
  return nullptr;
}

void QgsRuleBasedRendererV2::Rule::updateElseRules()
{
  mElseRules.clear();
  Q_FOREACH ( Rule* rule, mChildren )
  {
    if ( rule->isElse() )
      mElseRules << rule;
  }
}

void QgsRuleBasedRendererV2::Rule::setIsElse( bool iselse )
{
  mFilterExp = "ELSE";
  mElseRule = iselse;
  delete mFilter;
  mFilter = nullptr;
}


QString QgsRuleBasedRendererV2::Rule::dump( int indent ) const
{
  QString off;
  off.fill( QChar( ' ' ), indent );
  QString symbolDump = ( mSymbol ? mSymbol->dump() : QString( "[]" ) );
  QString msg = off + QString( "RULE %1 - scale [%2,%3] - filter %4 - symbol %5\n" )
                .arg( mLabel ).arg( mScaleMinDenom ).arg( mScaleMaxDenom )
                .arg( mFilterExp, symbolDump );

  QStringList lst;
  Q_FOREACH ( Rule* rule, mChildren )
  {
    lst.append( rule->dump( indent + 2 ) );
  }
  msg += lst.join( "\n" );
  return msg;
}

QSet<QString> QgsRuleBasedRendererV2::Rule::usedAttributes() const
{
  // attributes needed by this rule
  QSet<QString> attrs;
  if ( mFilter )
    attrs.unite( mFilter->referencedColumns().toSet() );
  if ( mSymbol )
    attrs.unite( mSymbol->usedAttributes() );

  // attributes needed by child rules
  Q_FOREACH ( Rule* rule, mChildren )
  {
    attrs.unite( rule->usedAttributes() );
  }
  return attrs;
}

bool QgsRuleBasedRendererV2::Rule::needsGeometry() const
{
  if ( mFilter && mFilter->needsGeometry() )
    return true;

  Q_FOREACH ( Rule* rule, mChildren )
  {
    if ( rule->needsGeometry() )
      return true;
  }

  return false;
}

QgsSymbolV2List QgsRuleBasedRendererV2::Rule::symbols( const QgsRenderContext& context ) const
{
  QgsSymbolV2List lst;
  if ( mSymbol )
    lst.append( mSymbol );

  Q_FOREACH ( Rule* rule, mChildren )
  {
    lst += rule->symbols( context );
  }
  return lst;
}

void QgsRuleBasedRendererV2::Rule::setSymbol( QgsSymbolV2* sym )
{
  delete mSymbol;
  mSymbol = sym;
}

void QgsRuleBasedRendererV2::Rule::setFilterExpression( const QString& filterExp )
{
  mFilterExp = filterExp;
  initFilter();
}

QgsLegendSymbolList QgsRuleBasedRendererV2::Rule::legendSymbolItems( double scaleDenominator, const QString& ruleFilter ) const
{
  QgsLegendSymbolList lst;
  if ( mSymbol && ( ruleFilter.isEmpty() || mLabel == ruleFilter ) )
    lst << qMakePair( mLabel, mSymbol );

  Q_FOREACH ( Rule* rule, mChildren )
  {
    if ( qgsDoubleNear( scaleDenominator, -1 ) || rule->isScaleOK( scaleDenominator ) )
    {
      lst << rule->legendSymbolItems( scaleDenominator, ruleFilter );
    }
  }
  return lst;
}

QgsLegendSymbolListV2 QgsRuleBasedRendererV2::Rule::legendSymbolItemsV2( int currentLevel ) const
{
  QgsLegendSymbolListV2 lst;
  if ( currentLevel != -1 ) // root rule should not be shown
  {
    lst << QgsLegendSymbolItemV2( mSymbol, mLabel, mRuleKey, true, mScaleMinDenom, mScaleMaxDenom, currentLevel, mParent ? mParent->mRuleKey : QString() );
  }

  for ( RuleList::const_iterator it = mChildren.constBegin(); it != mChildren.constEnd(); ++it )
  {
    Rule* rule = *it;
    lst << rule->legendSymbolItemsV2( currentLevel + 1 );
  }
  return lst;
}


bool QgsRuleBasedRendererV2::Rule::isFilterOK( QgsFeature& f, QgsRenderContext* context ) const
{
  if ( ! mFilter || mElseRule )
    return true;

  context->expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context->expressionContext() );
  return res.toInt() != 0;
}

bool QgsRuleBasedRendererV2::Rule::isScaleOK( double scale ) const
{
  if ( qgsDoubleNear( scale, 0.0 ) ) // so that we can count features in classes without scale context
    return true;
  if ( mScaleMinDenom == 0 && mScaleMaxDenom == 0 )
    return true;
  if ( mScaleMinDenom != 0 && mScaleMinDenom > scale )
    return false;
  if ( mScaleMaxDenom != 0 && mScaleMaxDenom < scale )
    return false;
  return true;
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::Rule::clone() const
{
  QgsSymbolV2* sym = mSymbol ? mSymbol->clone() : nullptr;
  Rule* newrule = new Rule( sym, mScaleMinDenom, mScaleMaxDenom, mFilterExp, mLabel, mDescription );
  newrule->setActive( mIsActive );
  // clone children
  Q_FOREACH ( Rule* rule, mChildren )
    newrule->appendChild( rule->clone() );
  return newrule;
}

QDomElement QgsRuleBasedRendererV2::Rule::save( QDomDocument& doc, QgsSymbolV2Map& symbolMap ) const
{
  QDomElement ruleElem = doc.createElement( "rule" );

  if ( mSymbol )
  {
    int symbolIndex = symbolMap.size();
    symbolMap[QString::number( symbolIndex )] = mSymbol;
    ruleElem.setAttribute( "symbol", symbolIndex );
  }
  if ( !mFilterExp.isEmpty() )
    ruleElem.setAttribute( "filter", mFilterExp );
  if ( mScaleMinDenom != 0 )
    ruleElem.setAttribute( "scalemindenom", mScaleMinDenom );
  if ( mScaleMaxDenom != 0 )
    ruleElem.setAttribute( "scalemaxdenom", mScaleMaxDenom );
  if ( !mLabel.isEmpty() )
    ruleElem.setAttribute( "label", mLabel );
  if ( !mDescription.isEmpty() )
    ruleElem.setAttribute( "description", mDescription );
  if ( !mIsActive )
    ruleElem.setAttribute( "checkstate", 0 );
  ruleElem.setAttribute( "key", mRuleKey );

  Q_FOREACH ( Rule* rule, mChildren )
  {
    ruleElem.appendChild( rule->save( doc, symbolMap ) );
  }
  return ruleElem;
}

void QgsRuleBasedRendererV2::Rule::toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const
{
  // do not convert this rule if there are no symbols
  QgsRenderContext context;
  if ( symbols( context ).isEmpty() )
    return;

  if ( !mFilterExp.isEmpty() )
  {
    if ( !props.value( "filter", "" ).isEmpty() )
      props[ "filter" ] += " AND ";
    props[ "filter" ] += mFilterExp;
  }

  if ( mScaleMinDenom != 0 )
  {
    bool ok;
    int parentScaleMinDenom = props.value( "scaleMinDenom", "0" ).toInt( &ok );
    if ( !ok || parentScaleMinDenom <= 0 )
      props[ "scaleMinDenom" ] = QString::number( mScaleMinDenom );
    else
      props[ "scaleMinDenom" ] = QString::number( qMax( parentScaleMinDenom, mScaleMinDenom ) );
  }

  if ( mScaleMaxDenom != 0 )
  {
    bool ok;
    int parentScaleMaxDenom = props.value( "scaleMaxDenom", "0" ).toInt( &ok );
    if ( !ok || parentScaleMaxDenom <= 0 )
      props[ "scaleMaxDenom" ] = QString::number( mScaleMaxDenom );
    else
      props[ "scaleMaxDenom" ] = QString::number( qMin( parentScaleMaxDenom, mScaleMaxDenom ) );
  }

  if ( mSymbol )
  {
    QDomElement ruleElem = doc.createElement( "se:Rule" );
    element.appendChild( ruleElem );

    //XXX: <se:Name> is the rule identifier, but our the Rule objects
    // have no properties could be used as identifier. Use the label.
    QDomElement nameElem = doc.createElement( "se:Name" );
    nameElem.appendChild( doc.createTextNode( mLabel ) );
    ruleElem.appendChild( nameElem );

    if ( !mLabel.isEmpty() || !mDescription.isEmpty() )
    {
      QDomElement descrElem = doc.createElement( "se:Description" );
      if ( !mLabel.isEmpty() )
      {
        QDomElement titleElem = doc.createElement( "se:Title" );
        titleElem.appendChild( doc.createTextNode( mLabel ) );
        descrElem.appendChild( titleElem );
      }
      if ( !mDescription.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( "se:Abstract" );
        abstractElem.appendChild( doc.createTextNode( mDescription ) );
        descrElem.appendChild( abstractElem );
      }
      ruleElem.appendChild( descrElem );
    }

    if ( !props.value( "filter", "" ).isEmpty() )
    {
      QgsSymbolLayerV2Utils::createFunctionElement( doc, ruleElem, props.value( "filter", "" ) );
    }

    if ( !props.value( "scaleMinDenom", "" ).isEmpty() )
    {
      QDomElement scaleMinDenomElem = doc.createElement( "se:MinScaleDenominator" );
      scaleMinDenomElem.appendChild( doc.createTextNode( props.value( "scaleMinDenom", "" ) ) );
      ruleElem.appendChild( scaleMinDenomElem );
    }

    if ( !props.value( "scaleMaxDenom", "" ).isEmpty() )
    {
      QDomElement scaleMaxDenomElem = doc.createElement( "se:MaxScaleDenominator" );
      scaleMaxDenomElem.appendChild( doc.createTextNode( props.value( "scaleMaxDenom", "" ) ) );
      ruleElem.appendChild( scaleMaxDenomElem );
    }

    mSymbol->toSld( doc, ruleElem, props );
  }

  // loop into childern rule list
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->toSld( doc, element, props );
  }
}

bool QgsRuleBasedRendererV2::Rule::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  QString filter;
  return startRender( context, fields, filter );
}

bool QgsRuleBasedRendererV2::Rule::startRender( QgsRenderContext& context, const QgsFields& fields, QString& filter )
{
  mActiveChildren.clear();

  if ( ! mIsActive )
    return false;

  // filter out rules which are not compatible with this scale
  if ( !isScaleOK( context.rendererScale() ) )
    return false;

  // init this rule
  if ( mFilter )
    mFilter->prepare( &context.expressionContext() );
  if ( mSymbol )
    mSymbol->startRender( context, &fields );

  // init children
  // build temporary list of active rules (usable with this scale)
  QStringList subfilters;
  Q_FOREACH ( Rule* rule, mChildren )
  {
    QString subfilter;
    if ( rule->startRender( context, fields , subfilter ) )
    {
      // only add those which are active with current scale
      mActiveChildren.append( rule );
      subfilters.append( subfilter );
    }
  }

  // subfilters (on the same level) are joined with OR
  // Finally they are joined with their parent (this) with AND
  QString sf;
  // If there are subfilters present (and it's not a single empty one), group them and join them with OR
  if ( subfilters.length() > 1 || !subfilters.value( 0 ).isEmpty() )
  {
    if ( subfilters.contains( "TRUE" ) )
      sf = "TRUE";
    else
      sf = subfilters.join( ") OR (" ).prepend( '(' ).append( ')' );
  }

  // Now join the subfilters with their parent (this) based on if
  // * The parent is an else rule
  // * The existence of parent filter and subfilters

  // No filter expression: ELSE rule or catchall rule
  if ( !mFilter )
  {
    if ( mSymbol || sf.isEmpty() )
      filter = "TRUE";
    else
      filter = sf;
  }
  else if ( mSymbol )
    filter = mFilterExp;
  else if ( !mFilterExp.trimmed().isEmpty() && !sf.isEmpty() )
    filter = QString( "(%1) AND (%2)" ).arg( mFilterExp, sf );
  else if ( !mFilterExp.trimmed().isEmpty() )
    filter = mFilterExp;
  else if ( sf.isEmpty() )
    filter = "TRUE";
  else
    filter = sf;

  filter = filter.trimmed();

  return true;
}

QSet<int> QgsRuleBasedRendererV2::Rule::collectZLevels()
{
  QSet<int> symbolZLevelsSet;

  // process this rule
  if ( mSymbol )
  {
    // find out which Z-levels are used
    for ( int i = 0; i < mSymbol->symbolLayerCount(); i++ )
    {
      symbolZLevelsSet.insert( mSymbol->symbolLayer( i )->renderingPass() );
    }
  }

  // process children
  QList<Rule*>::iterator it;
  for ( it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    symbolZLevelsSet.unite( rule->collectZLevels() );
  }
  return symbolZLevelsSet;
}

void QgsRuleBasedRendererV2::Rule::setNormZLevels( const QMap<int, int>& zLevelsToNormLevels )
{
  if ( mSymbol )
  {
    for ( int i = 0; i < mSymbol->symbolLayerCount(); i++ )
    {
      int normLevel = zLevelsToNormLevels.value( mSymbol->symbolLayer( i )->renderingPass() );
      mSymbolNormZLevels.insert( normLevel );
    }
  }

  // prepare list of normalized levels for each rule
  Q_FOREACH ( Rule* rule, mActiveChildren )
  {
    rule->setNormZLevels( zLevelsToNormLevels );
  }
}


QgsRuleBasedRendererV2::Rule::RenderResult QgsRuleBasedRendererV2::Rule::renderFeature( QgsRuleBasedRendererV2::FeatureToRender& featToRender, QgsRenderContext& context, QgsRuleBasedRendererV2::RenderQueue& renderQueue )
{
  if ( !isFilterOK( featToRender.feat, &context ) )
    return Filtered;

  bool rendered = false;

  // create job for this feature and this symbol, add to list of jobs
  if ( mSymbol && mIsActive )
  {
    // add job to the queue: each symbol's zLevel must be added
    Q_FOREACH ( int normZLevel, mSymbolNormZLevels )
    {
      //QgsDebugMsg(QString("add job at level %1").arg(normZLevel));
      renderQueue[normZLevel].jobs.append( new RenderJob( featToRender, mSymbol ) );
      rendered = true;
    }
  }

  bool willrendersomething = false;

  // process children
  Q_FOREACH ( Rule* rule, mChildren )
  {
    // Don't process else rules yet
    if ( !rule->isElse() )
    {
      RenderResult res = rule->renderFeature( featToRender, context, renderQueue );
      // consider inactive items as "rendered" so the else rule will ignore them
      willrendersomething |= ( res == Rendered || res == Inactive );
      rendered |= ( res == Rendered );
    }
  }

  // If none of the rules passed then we jump into the else rules and process them.
  if ( !willrendersomething )
  {
    Q_FOREACH ( Rule* rule, mElseRules )
    {
      rendered |= rule->renderFeature( featToRender, context, renderQueue ) == Rendered;
    }
  }
  if ( !mIsActive || ( mSymbol && !rendered ) )
    return Inactive;
  else if ( rendered )
    return Rendered;
  else
    return Filtered;
}

bool QgsRuleBasedRendererV2::Rule::willRenderFeature( QgsFeature& feat, QgsRenderContext *context )
{
  if ( !isFilterOK( feat, context ) )
    return false;
  if ( mSymbol )
    return true;

  Q_FOREACH ( Rule* rule, mActiveChildren )
  {
    if ( rule->willRenderFeature( feat, context ) )
      return true;
  }
  return false;
}

QgsSymbolV2List QgsRuleBasedRendererV2::Rule::symbolsForFeature( QgsFeature& feat, QgsRenderContext* context )
{
  QgsSymbolV2List lst;
  if ( !isFilterOK( feat, context ) )
    return lst;
  if ( mSymbol )
    lst.append( mSymbol );

  Q_FOREACH ( Rule* rule, mActiveChildren )
  {
    lst += rule->symbolsForFeature( feat, context );
  }
  return lst;
}

QSet<QString> QgsRuleBasedRendererV2::Rule::legendKeysForFeature( QgsFeature& feat, QgsRenderContext* context )
{
  QSet< QString> lst;
  if ( !isFilterOK( feat, context ) )
    return lst;
  lst.insert( mRuleKey );

  Q_FOREACH ( Rule* rule, mActiveChildren )
  {
    lst.unite( rule->legendKeysForFeature( feat, context ) );
  }
  return lst;
}

QgsRuleBasedRendererV2::RuleList QgsRuleBasedRendererV2::Rule::rulesForFeature( QgsFeature& feat, QgsRenderContext* context )
{
  RuleList lst;
  if ( !isFilterOK( feat, context ) )
    return lst;

  if ( mSymbol )
    lst.append( this );

  Q_FOREACH ( Rule* rule, mActiveChildren )
  {
    lst += rule->rulesForFeature( feat, context );
  }
  return lst;
}

void QgsRuleBasedRendererV2::Rule::stopRender( QgsRenderContext& context )
{
  if ( mSymbol )
    mSymbol->stopRender( context );

  Q_FOREACH ( Rule* rule, mActiveChildren )
  {
    rule->stopRender( context );
  }

  mActiveChildren.clear();
  mSymbolNormZLevels.clear();
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::Rule::create( QDomElement& ruleElem, QgsSymbolV2Map& symbolMap )
{
  QString symbolIdx = ruleElem.attribute( "symbol" );
  QgsSymbolV2* symbol = nullptr;
  if ( !symbolIdx.isEmpty() )
  {
    if ( symbolMap.contains( symbolIdx ) )
    {
      symbol = symbolMap.take( symbolIdx );
    }
    else
    {
      QgsDebugMsg( "symbol for rule " + symbolIdx + " not found!" );
    }
  }

  QString filterExp = ruleElem.attribute( "filter" );
  QString label = ruleElem.attribute( "label" );
  QString description = ruleElem.attribute( "description" );
  int scaleMinDenom = ruleElem.attribute( "scalemindenom", "0" ).toInt();
  int scaleMaxDenom = ruleElem.attribute( "scalemaxdenom", "0" ).toInt();
  QString ruleKey = ruleElem.attribute( "key" );
  Rule* rule = new Rule( symbol, scaleMinDenom, scaleMaxDenom, filterExp, label, description );

  if ( !ruleKey.isEmpty() )
    rule->mRuleKey = ruleKey;

  rule->setActive( ruleElem.attribute( "checkstate", "1" ).toInt() );

  QDomElement childRuleElem = ruleElem.firstChildElement( "rule" );
  while ( !childRuleElem.isNull() )
  {
    Rule* childRule = create( childRuleElem, symbolMap );
    if ( childRule )
    {
      rule->appendChild( childRule );
    }
    else
    {
      QgsDebugMsg( "failed to init a child rule!" );
    }
    childRuleElem = childRuleElem.nextSiblingElement( "rule" );
  }

  return rule;
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::Rule::createFromSld( QDomElement& ruleElem, QGis::GeometryType geomType )
{
  if ( ruleElem.localName() != "Rule" )
  {
    QgsDebugMsg( QString( "invalid element: Rule element expected, %1 found!" ).arg( ruleElem.tagName() ) );
    return nullptr;
  }

  QString label, description, filterExp;
  int scaleMinDenom = 0, scaleMaxDenom = 0;
  QgsSymbolLayerV2List layers;

  // retrieve the Rule element child nodes
  QDomElement childElem = ruleElem.firstChildElement();
  while ( !childElem.isNull() )
  {
    if ( childElem.localName() == "Name" )
    {
      // <se:Name> tag contains the rule identifier,
      // so prefer title tag for the label property value
      if ( label.isEmpty() )
        label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Description" )
    {
      // <se:Description> can contains a title and an abstract
      QDomElement titleElem = childElem.firstChildElement( "Title" );
      if ( !titleElem.isNull() )
      {
        label = titleElem.firstChild().nodeValue();
      }

      QDomElement abstractElem = childElem.firstChildElement( "Abstract" );
      if ( !abstractElem.isNull() )
      {
        description = abstractElem.firstChild().nodeValue();
      }
    }
    else if ( childElem.localName() == "Abstract" )
    {
      // <sld:Abstract> (v1.0)
      description = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Title" )
    {
      // <sld:Title> (v1.0)
      label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == "Filter" )
    {
      QgsExpression *filter = QgsOgcUtils::expressionFromOgcFilter( childElem );
      if ( filter )
      {
        if ( filter->hasParserError() )
        {
          QgsDebugMsg( "parser error: " + filter->parserErrorString() );
        }
        else
        {
          filterExp = filter->expression();
        }
        delete filter;
      }
    }
    else if ( childElem.localName() == "MinScaleDenominator" )
    {
      bool ok;
      int v = childElem.firstChild().nodeValue().toInt( &ok );
      if ( ok )
        scaleMinDenom = v;
    }
    else if ( childElem.localName() == "MaxScaleDenominator" )
    {
      bool ok;
      int v = childElem.firstChild().nodeValue().toInt( &ok );
      if ( ok )
        scaleMaxDenom = v;
    }
    else if ( childElem.localName().endsWith( "Symbolizer" ) )
    {
      // create symbol layers for this symbolizer
      QgsSymbolLayerV2Utils::createSymbolLayerV2ListFromSld( childElem, geomType, layers );
    }

    childElem = childElem.nextSiblingElement();
  }

  // now create the symbol
  QgsSymbolV2 *symbol = nullptr;
  if ( !layers.isEmpty() )
  {
    switch ( geomType )
    {
      case QGis::Line:
        symbol = new QgsLineSymbolV2( layers );
        break;

      case QGis::Polygon:
        symbol = new QgsFillSymbolV2( layers );
        break;

      case QGis::Point:
        symbol = new QgsMarkerSymbolV2( layers );
        break;

      default:
        QgsDebugMsg( QString( "invalid geometry type: found %1" ).arg( geomType ) );
        return nullptr;
    }
  }

  // and then create and return the new rule
  return new Rule( symbol, scaleMinDenom, scaleMaxDenom, filterExp, label, description );
}


/////////////////////

QgsRuleBasedRendererV2::QgsRuleBasedRendererV2( QgsRuleBasedRendererV2::Rule* root )
    : QgsFeatureRendererV2( "RuleRenderer" )
    , mRootRule( root )
{
}

QgsRuleBasedRendererV2::QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol )
    : QgsFeatureRendererV2( "RuleRenderer" )
{
  mRootRule = new Rule( nullptr ); // root has no symbol, no filter etc - just a container
  mRootRule->appendChild( new Rule( defaultSymbol ) );
}

QgsRuleBasedRendererV2::~QgsRuleBasedRendererV2()
{
  delete mRootRule;
}


QgsSymbolV2* QgsRuleBasedRendererV2::symbolForFeature( QgsFeature& , QgsRenderContext& )
{
  // not used at all
  return nullptr;
}

bool QgsRuleBasedRendererV2::renderFeature( QgsFeature& feature,
    QgsRenderContext& context,
    int layer,
    bool selected,
    bool drawVertexMarker )
{
  Q_UNUSED( layer );

  int flags = ( selected ? FeatIsSelected : 0 ) | ( drawVertexMarker ? FeatDrawMarkers : 0 );
  mCurrentFeatures.append( FeatureToRender( feature, flags ) );

  // check each active rule
  return mRootRule->renderFeature( mCurrentFeatures.last(), context, mRenderQueue ) == Rule::Rendered;
}


void QgsRuleBasedRendererV2::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  // prepare active children
  mRootRule->startRender( context, fields, mFilter );

  QSet<int> symbolZLevelsSet = mRootRule->collectZLevels();
  QList<int> symbolZLevels = symbolZLevelsSet.toList();
  qSort( symbolZLevels );

  // create mapping from unnormalized levels [unlimited range] to normalized levels [0..N-1]
  // and prepare rendering queue
  QMap<int, int> zLevelsToNormLevels;
  int maxNormLevel = -1;
  Q_FOREACH ( int zLevel, symbolZLevels )
  {
    zLevelsToNormLevels[zLevel] = ++maxNormLevel;
    mRenderQueue.append( RenderLevel( zLevel ) );
    QgsDebugMsg( QString( "zLevel %1 -> %2" ).arg( zLevel ).arg( maxNormLevel ) );
  }

  mRootRule->setNormZLevels( zLevelsToNormLevels );
}

void QgsRuleBasedRendererV2::stopRender( QgsRenderContext& context )
{
  //
  // do the actual rendering
  //

  // go through all levels
  Q_FOREACH ( const RenderLevel& level, mRenderQueue )
  {
    //QgsDebugMsg(QString("level %1").arg(level.zIndex));
    // go through all jobs at the level
    Q_FOREACH ( const RenderJob* job, level.jobs )
    {
      context.expressionContext().setFeature( job->ftr.feat );
      //QgsDebugMsg(QString("job fid %1").arg(job->f->id()));
      // render feature - but only with symbol layers with specified zIndex
      QgsSymbolV2* s = job->symbol;
      int count = s->symbolLayerCount();
      for ( int i = 0; i < count; i++ )
      {
        // TODO: better solution for this
        // renderFeatureWithSymbol asks which symbol layer to draw
        // but there are multiple transforms going on!
        if ( s->symbolLayer( i )->renderingPass() == level.zIndex )
        {
          int flags = job->ftr.flags;
          renderFeatureWithSymbol( job->ftr.feat, job->symbol, context, i, flags & FeatIsSelected, flags & FeatDrawMarkers );
        }
      }
    }
  }

  // clean current features
  mCurrentFeatures.clear();

  // clean render queue
  mRenderQueue.clear();

  // clean up rules from temporary stuff
  mRootRule->stopRender( context );
}

QString QgsRuleBasedRendererV2::filter( const QgsFields& )
{
  return mFilter;
}

QList<QString> QgsRuleBasedRendererV2::usedAttributes()
{
  QSet<QString> attrs = mRootRule->usedAttributes();
  return attrs.toList();
}

bool QgsRuleBasedRendererV2::filterNeedsGeometry() const
{
  return mRootRule->needsGeometry();
}

QgsRuleBasedRendererV2* QgsRuleBasedRendererV2::clone() const
{
  QgsRuleBasedRendererV2::Rule* clonedRoot = mRootRule->clone();

  // normally with clone() the individual rules get new keys (UUID), but here we want to keep
  // the tree of rules intact, so that other components that may use the rule keys work nicely (e.g. visibility presets)
  clonedRoot->setRuleKey( mRootRule->ruleKey() );
  RuleList origDescendants = mRootRule->descendants();
  RuleList clonedDescendants = clonedRoot->descendants();
  Q_ASSERT( origDescendants.count() == clonedDescendants.count() );
  for ( int i = 0; i < origDescendants.count(); ++i )
    clonedDescendants[i]->setRuleKey( origDescendants[i]->ruleKey() );

  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2( clonedRoot );

  r->setUsingSymbolLevels( usingSymbolLevels() );
  copyRendererData( r );
  return r;
}

void QgsRuleBasedRendererV2::toSld( QDomDocument& doc, QDomElement &element ) const
{
  mRootRule->toSld( doc, element, QgsStringMap() );
}

// TODO: ideally this function should be removed in favor of legendSymbol(ogy)Items
QgsSymbolV2List QgsRuleBasedRendererV2::symbols( QgsRenderContext& context )
{
  return mRootRule->symbols( context );
}

QDomElement QgsRuleBasedRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "RuleRenderer" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );
  rendererElem.setAttribute( "forceraster", ( mForceRaster ? "1" : "0" ) );

  QgsSymbolV2Map symbols;

  QDomElement rulesElem = mRootRule->save( doc, symbols );
  rulesElem.setTagName( "rules" ); // instead of just "rule"
  rendererElem.appendChild( rulesElem );

  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( "orderby" );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( "enableorderby", ( mOrderByEnabled ? "1" : "0" ) );

  return rendererElem;
}

QgsLegendSymbologyList QgsRuleBasedRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  QgsLegendSymbolList items = legendSymbolItems();
  for ( QgsLegendSymbolList::iterator it = items.begin(); it != items.end(); ++it )
  {
    QPair<QString, QgsSymbolV2*> pair = *it;
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( pair.second, iconSize );
    lst << qMakePair( pair.first, pix );
  }
  return lst;
}

bool QgsRuleBasedRendererV2::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsRuleBasedRendererV2::legendSymbolItemChecked( const QString& key )
{
  Rule* rule = mRootRule->findRuleByKey( key );
  return rule ? rule->active() : true;
}

void QgsRuleBasedRendererV2::checkLegendSymbolItem( const QString& key, bool state )
{
  Rule* rule = mRootRule->findRuleByKey( key );
  if ( rule )
    rule->setActive( state );
}

void QgsRuleBasedRendererV2::setLegendSymbolItem( const QString& key, QgsSymbolV2* symbol )
{
  Rule* rule = mRootRule->findRuleByKey( key );
  if ( rule )
    rule->setSymbol( symbol );
  else
    delete symbol;
}

QgsLegendSymbolList QgsRuleBasedRendererV2::legendSymbolItems( double scaleDenominator, const QString& rule )
{
  return mRootRule->legendSymbolItems( scaleDenominator, rule );
}

QgsLegendSymbolListV2 QgsRuleBasedRendererV2::legendSymbolItemsV2() const
{
  return mRootRule->legendSymbolItemsV2();
}


QgsFeatureRendererV2* QgsRuleBasedRendererV2::create( QDomElement& element )
{
  // load symbols
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return nullptr;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );

  QDomElement rulesElem = element.firstChildElement( "rules" );

  Rule* root = Rule::create( rulesElem, symbolMap );
  if ( !root )
    return nullptr;

  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2( root );

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  return r;
}

QgsFeatureRendererV2* QgsRuleBasedRendererV2::createFromSld( QDomElement& element, QGis::GeometryType geomType )
{
  // retrieve child rules
  Rule* root = nullptr;

  QDomElement ruleElem = element.firstChildElement( "Rule" );
  while ( !ruleElem.isNull() )
  {
    Rule *child = Rule::createFromSld( ruleElem, geomType );
    if ( child )
    {
      // create the root rule if not done before
      if ( !root )
        root = new Rule( nullptr );

      root->appendChild( child );
    }

    ruleElem = ruleElem.nextSiblingElement( "Rule" );
  }

  if ( !root )
  {
    // no valid rules was found
    return nullptr;
  }

  // create and return the new renderer
  return new QgsRuleBasedRendererV2( root );
}

#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"

void QgsRuleBasedRendererV2::refineRuleCategories( QgsRuleBasedRendererV2::Rule* initialRule, QgsCategorizedSymbolRendererV2* r )
{
  QString attr = r->classAttribute();
  // categorizedAttr could be either an attribute name or an expression.
  // the only way to differentiate is to test it as an expression...
  QgsExpression testExpr( attr );
  if ( testExpr.hasParserError() || ( testExpr.isField() && !attr.startsWith( '\"' ) ) )
  {
    //not an expression, so need to quote column name
    attr = QgsExpression::quotedColumnRef( attr );
  }

  Q_FOREACH ( const QgsRendererCategoryV2& cat, r->categories() )
  {
    QString value;
    // not quoting numbers saves a type cast
    if ( cat.value().type() == QVariant::Int )
      value = cat.value().toString();
    else if ( cat.value().type() == QVariant::Double )
      // we loose precision here - so we may miss some categories :-(
      // TODO: have a possibility to construct expressions directly as a parse tree to avoid loss of precision
      value = QString::number( cat.value().toDouble(), 'f', 4 );
    else
      value = QgsExpression::quotedString( cat.value().toString() );
    QString filter = QString( "%1 = %2" ).arg( attr, value );
    QString label = filter;
    initialRule->appendChild( new Rule( cat.symbol()->clone(), 0, 0, filter, label ) );
  }
}

void QgsRuleBasedRendererV2::refineRuleRanges( QgsRuleBasedRendererV2::Rule* initialRule, QgsGraduatedSymbolRendererV2* r )
{
  QString attr = r->classAttribute();
  // categorizedAttr could be either an attribute name or an expression.
  // the only way to differentiate is to test it as an expression...
  QgsExpression testExpr( attr );
  if ( testExpr.hasParserError() || ( testExpr.isField() && !attr.startsWith( '\"' ) ) )
  {
    //not an expression, so need to quote column name
    attr = QgsExpression::quotedColumnRef( attr );
  }
  else if ( !testExpr.isField() )
  {
    //otherwise wrap expression in brackets
    attr = QString( "(%1)" ).arg( attr );
  }

  bool firstRange = true;
  Q_FOREACH ( const QgsRendererRangeV2& rng, r->ranges() )
  {
    // due to the loss of precision in double->string conversion we may miss out values at the limit of the range
    // TODO: have a possibility to construct expressions directly as a parse tree to avoid loss of precision
    QString filter = QString( "%1 %2 %3 AND %1 <= %4" ).arg( attr, firstRange ? ">=" : ">",
                     QString::number( rng.lowerValue(), 'f', 4 ),
                     QString::number( rng.upperValue(), 'f', 4 ) );
    firstRange = false;
    QString label = filter;
    initialRule->appendChild( new Rule( rng.symbol()->clone(), 0, 0, filter, label ) );
  }
}

void QgsRuleBasedRendererV2::refineRuleScales( QgsRuleBasedRendererV2::Rule* initialRule, QList<int> scales )
{
  qSort( scales ); // make sure the scales are in ascending order
  int oldScale = initialRule->scaleMinDenom();
  int maxDenom = initialRule->scaleMaxDenom();
  QgsSymbolV2* symbol = initialRule->symbol();
  Q_FOREACH ( int scale, scales )
  {
    if ( initialRule->scaleMinDenom() >= scale )
      continue; // jump over the first scales out of the interval
    if ( maxDenom != 0 && maxDenom  <= scale )
      break; // ignore the latter scales out of the interval
    initialRule->appendChild( new Rule( symbol->clone(), oldScale, scale, QString(), QString( "%1 - %2" ).arg( oldScale ).arg( scale ) ) );
    oldScale = scale;
  }
  // last rule
  initialRule->appendChild( new Rule( symbol->clone(), oldScale, maxDenom, QString(), QString( "%1 - %2" ).arg( oldScale ).arg( maxDenom ) ) );
}

QString QgsRuleBasedRendererV2::dump() const
{
  QString msg( "Rule-based renderer:\n" );
  msg += mRootRule->dump();
  return msg;
}

bool QgsRuleBasedRendererV2::willRenderFeature( QgsFeature& feat, QgsRenderContext& context )
{
  return mRootRule->willRenderFeature( feat, &context );
}

QgsSymbolV2List QgsRuleBasedRendererV2::symbolsForFeature( QgsFeature& feat, QgsRenderContext& context )
{
  return mRootRule->symbolsForFeature( feat, &context );
}

QgsSymbolV2List QgsRuleBasedRendererV2::originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext& context )
{
  return mRootRule->symbolsForFeature( feat, &context );
}

QSet< QString > QgsRuleBasedRendererV2::legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  return mRootRule->legendKeysForFeature( feature, &context );
}

QgsRuleBasedRendererV2* QgsRuleBasedRendererV2::convertFromRenderer( const QgsFeatureRendererV2* renderer )
{
  QgsRuleBasedRendererV2* r = nullptr;
  if ( renderer->type() == "RuleRenderer" )
  {
    r = dynamic_cast<QgsRuleBasedRendererV2*>( renderer->clone() );
  }
  else if ( renderer->type() == "singleSymbol" )
  {
    const QgsSingleSymbolRendererV2* singleSymbolRenderer = dynamic_cast<const QgsSingleSymbolRendererV2*>( renderer );
    if ( !singleSymbolRenderer )
      return nullptr;

    QgsSymbolV2* origSymbol = singleSymbolRenderer->symbol()->clone();
    convertToDataDefinedSymbology( origSymbol, singleSymbolRenderer->sizeScaleField() );
    r = new QgsRuleBasedRendererV2( origSymbol );
  }
  else if ( renderer->type() == "categorizedSymbol" )
  {
    const QgsCategorizedSymbolRendererV2* categorizedRenderer = dynamic_cast<const QgsCategorizedSymbolRendererV2*>( renderer );
    if ( !categorizedRenderer )
      return nullptr;

    QString attr = categorizedRenderer->classAttribute();
    // categorizedAttr could be either an attribute name or an expression.
    // the only way to differentiate is to test it as an expression...
    QgsExpression testExpr( attr );
    if ( testExpr.hasParserError() || ( testExpr.isField() && !attr.startsWith( '\"' ) ) )
    {
      //not an expression, so need to quote column name
      attr = QgsExpression::quotedColumnRef( attr );
    }

    QgsRuleBasedRendererV2::Rule* rootrule = new QgsRuleBasedRendererV2::Rule( nullptr );

    QString expression;
    QString value;
    QgsRendererCategoryV2 category;
    for ( int i = 0; i < categorizedRenderer->categories().size(); ++i )
    {
      category = categorizedRenderer->categories().value( i );
      QgsRuleBasedRendererV2::Rule* rule = new QgsRuleBasedRendererV2::Rule( nullptr );

      rule->setLabel( category.label() );

      //We first define the rule corresponding to the category
      //If the value is a number, we can use it directly, otherwise we need to quote it in the rule
      if ( QVariant( category.value() ).convert( QVariant::Double ) )
      {
        value = category.value().toString();
      }
      else
      {
        value = QgsExpression::quotedString( category.value().toString() );
      }

      //An empty category is equivalent to the ELSE keyword
      if ( value == "''" )
      {
        expression = "ELSE";
      }
      else
      {
        expression = QString( "%1 = %2" ).arg( attr, value );
      }
      rule->setFilterExpression( expression );

      //Then we construct an equivalent symbol.
      //Ideally we could simply copy the symbol, but the categorized renderer allows a separate interface to specify
      //data dependent area and rotation, so we need to convert these to obtain the same rendering

      QgsSymbolV2* origSymbol = category.symbol()->clone();
      convertToDataDefinedSymbology( origSymbol, categorizedRenderer->sizeScaleField() );
      rule->setSymbol( origSymbol );

      rootrule->appendChild( rule );
    }

    r = new QgsRuleBasedRendererV2( rootrule );
  }
  else if ( renderer->type() == "graduatedSymbol" )
  {
    const QgsGraduatedSymbolRendererV2* graduatedRenderer = dynamic_cast<const QgsGraduatedSymbolRendererV2*>( renderer );
    if ( !graduatedRenderer )
      return nullptr;

    QString attr = graduatedRenderer->classAttribute();
    // categorizedAttr could be either an attribute name or an expression.
    // the only way to differentiate is to test it as an expression...
    QgsExpression testExpr( attr );
    if ( testExpr.hasParserError() || ( testExpr.isField() && !attr.startsWith( '\"' ) ) )
    {
      //not an expression, so need to quote column name
      attr = QgsExpression::quotedColumnRef( attr );
    }
    else if ( !testExpr.isField() )
    {
      //otherwise wrap expression in brackets
      attr = QString( "(%1)" ).arg( attr );
    }

    QgsRuleBasedRendererV2::Rule* rootrule = new QgsRuleBasedRendererV2::Rule( nullptr );

    QString expression;
    QgsRendererRangeV2 range;
    for ( int i = 0; i < graduatedRenderer->ranges().size();++i )
    {
      range = graduatedRenderer->ranges().value( i );
      QgsRuleBasedRendererV2::Rule* rule = new QgsRuleBasedRendererV2::Rule( nullptr );
      rule->setLabel( range.label() );
      if ( i == 0 )//The lower boundary of the first range is included, while it is excluded for the others
      {
        expression = attr + " >= " + QString::number( range.lowerValue(), 'f' ) + " AND " + \
                     attr + " <= " + QString::number( range.upperValue(), 'f' );
      }
      else
      {
        expression = attr + " > " + QString::number( range.lowerValue(), 'f' ) + " AND " + \
                     attr + " <= " + QString::number( range.upperValue(), 'f' );
      }
      rule->setFilterExpression( expression );

      //Then we construct an equivalent symbol.
      //Ideally we could simply copy the symbol, but the graduated renderer allows a separate interface to specify
      //data dependent area and rotation, so we need to convert these to obtain the same rendering

      QgsSymbolV2* symbol = range.symbol()->clone();
      convertToDataDefinedSymbology( symbol, graduatedRenderer->sizeScaleField() );

      rule->setSymbol( symbol );

      rootrule->appendChild( rule );
    }

    r = new QgsRuleBasedRendererV2( rootrule );
  }
  else if ( renderer->type() == "pointDisplacement" )
  {
    const QgsPointDisplacementRenderer* pointDisplacementRenderer = dynamic_cast<const QgsPointDisplacementRenderer*>( renderer );
    if ( pointDisplacementRenderer )
      r = convertFromRenderer( pointDisplacementRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == "invertedPolygonRenderer" )
  {
    const QgsInvertedPolygonRenderer* invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer*>( renderer );
    if ( invertedPolygonRenderer )
      r = convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() );
  }

  if ( r )
  {
    r->setOrderBy( renderer->orderBy() );
    r->setOrderByEnabled( renderer->orderByEnabled() );
  }

  return r;
}

void QgsRuleBasedRendererV2::convertToDataDefinedSymbology( QgsSymbolV2* symbol, const QString& sizeScaleField, const QString& rotationField )
{
  QString sizeExpression;
  switch ( symbol->type() )
  {
    case QgsSymbolV2::Marker:
      for ( int j = 0; j < symbol->symbolLayerCount();++j )
      {
        QgsMarkerSymbolLayerV2* msl = static_cast<QgsMarkerSymbolLayerV2*>( symbol->symbolLayer( j ) );
        if ( ! sizeScaleField.isEmpty() )
        {
          sizeExpression = QString( "%1*(%2)" ).arg( msl->size() ).arg( sizeScaleField );
          msl->setDataDefinedProperty( "size", new QgsDataDefined( sizeExpression ) );
        }
        if ( ! rotationField.isEmpty() )
        {
          msl->setDataDefinedProperty( "angle", new QgsDataDefined( true, false, QString(), rotationField ) );
        }
      }
      break;
    case QgsSymbolV2::Line:
      if ( ! sizeScaleField.isEmpty() )
      {
        for ( int j = 0; j < symbol->symbolLayerCount();++j )
        {
          if ( symbol->symbolLayer( j )->layerType() == "SimpleLine" )
          {
            QgsLineSymbolLayerV2* lsl = static_cast<QgsLineSymbolLayerV2*>( symbol->symbolLayer( j ) );
            sizeExpression = QString( "%1*(%2)" ).arg( lsl->width() ).arg( sizeScaleField );
            lsl->setDataDefinedProperty( "width", new QgsDataDefined( sizeExpression ) );
          }
          if ( symbol->symbolLayer( j )->layerType() == "MarkerLine" )
          {
            QgsSymbolV2* marker = symbol->symbolLayer( j )->subSymbol();
            for ( int k = 0; k < marker->symbolLayerCount();++k )
            {
              QgsMarkerSymbolLayerV2* msl = static_cast<QgsMarkerSymbolLayerV2*>( marker->symbolLayer( k ) );
              sizeExpression = QString( "%1*(%2)" ).arg( msl->size() ).arg( sizeScaleField );
              msl->setDataDefinedProperty( "size", new QgsDataDefined( sizeExpression ) );
            }
          }
        }
      }
      break;
    default:
      break;
  }
}
