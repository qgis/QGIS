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

#include <QSet>

#include <QDomDocument>
#include <QDomElement>



QgsRuleBasedRendererV2::Rule::Rule( QgsSymbolV2* symbol, int scaleMinDenom, int scaleMaxDenom, QString filterExp, QString label, QString description )
    : mParent( NULL ), mSymbol( symbol ),
    mScaleMinDenom( scaleMinDenom ), mScaleMaxDenom( scaleMaxDenom ),
    mFilterExp( filterExp ), mLabel( label ), mDescription( description ),
    mFilter( NULL )
{
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
  if ( !mFilterExp.isEmpty() )
  {
    delete mFilter;
    mFilter = new QgsExpression( mFilterExp );
  }
  else
  {
    mFilter = NULL;
  }
}

QString QgsRuleBasedRendererV2::Rule::dump( int offset ) const
{
  QString off;
  off.fill( QChar( ' ' ), offset );
  QString symbolDump = ( mSymbol ? mSymbol->dump() : QString( "[]" ) );
  QString msg = off + QString( "RULE %1 - scale [%2,%3] - filter %4 - symbol %5\n" )
                .arg( mLabel ).arg( mScaleMinDenom ).arg( mScaleMaxDenom )
                .arg( mFilterExp ).arg( symbolDump );

  QStringList lst;
  foreach ( Rule* rule, mChildren )
  {
    lst.append( rule->dump( offset + 2 ) );
  }
  msg += lst.join( "\n" );
  return msg;
}

QSet<QString> QgsRuleBasedRendererV2::Rule::usedAttributes()
{
  // attributes needed by this rule
  QSet<QString> attrs;
  if ( mFilter )
    attrs.unite( mFilter->referencedColumns().toSet() );
  if ( mSymbol )
    attrs.unite( mSymbol->usedAttributes() );

  // attributes needed by child rules
  for ( RuleList::iterator it = mChildren.begin(); it != mChildren.end(); ++it )
  {
    Rule* rule = *it;
    attrs.unite( rule->usedAttributes() );
  }
  return attrs;
}

QgsSymbolV2List QgsRuleBasedRendererV2::Rule::symbols()
{
  QgsSymbolV2List lst;
  if ( mSymbol )
    lst.append( mSymbol );

  for ( RuleList::iterator it = mChildren.begin(); it != mChildren.end(); ++it )
  {
    Rule* rule = *it;
    lst += rule->symbols();
  }
  return lst;
}

void QgsRuleBasedRendererV2::Rule::setSymbol( QgsSymbolV2* sym )
{
  delete mSymbol;
  mSymbol = sym;
}

QgsLegendSymbolList QgsRuleBasedRendererV2::Rule::legendSymbolItems()
{
  QgsLegendSymbolList lst;
  if ( mSymbol )
    lst << qMakePair( mLabel, mSymbol );

  for ( RuleList::iterator it = mChildren.begin(); it != mChildren.end(); ++it )
  {
    Rule* rule = *it;
    lst << rule->legendSymbolItems();
  }
  return lst;
}


bool QgsRuleBasedRendererV2::Rule::isFilterOK( QgsFeature& f ) const
{
  if ( ! mFilter )
    return true;

  QVariant res = mFilter->evaluate( &f );
  return res.toInt() != 0;
}

bool QgsRuleBasedRendererV2::Rule::isScaleOK( double scale ) const
{
  if ( scale == 0 ) // so that we can count features in classes without scale context
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
  QgsSymbolV2* sym = mSymbol ? mSymbol->clone() : NULL;
  Rule* newrule = new Rule( sym, mScaleMinDenom, mScaleMaxDenom, mFilterExp, mLabel, mDescription );
  // clone children
  foreach ( Rule* rule, mChildren )
    newrule->appendChild( rule->clone() );
  return newrule;
}

QDomElement QgsRuleBasedRendererV2::Rule::save( QDomDocument& doc, QgsSymbolV2Map& symbolMap )
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

  for ( RuleList::iterator it = mChildren.begin(); it != mChildren.end(); ++it )
  {
    Rule* rule = *it;
    ruleElem.appendChild( rule->save( doc, symbolMap ) );
  }
  return ruleElem;
}

void QgsRuleBasedRendererV2::Rule::toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props )
{
  // do not convert this rule if there are no symbols
  if ( symbols().isEmpty() )
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
      QDomElement filterElem = doc.createElement( "ogc:Filter" );
      QgsSymbolLayerV2Utils::createFunctionElement( doc, filterElem, props.value( "filter", "" ) );
      ruleElem.appendChild( filterElem );
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
  for ( RuleList::iterator it = mChildren.begin(); it != mChildren.end(); ++it )
  {
    ( *it )->toSld( doc, element, props );
  }
}

bool QgsRuleBasedRendererV2::Rule::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  mActiveChildren.clear();

  // filter out rules which are not compatible with this scale
  if ( !isScaleOK( context.rendererScale() ) )
    return false;

  // init this rule
  if ( mFilter )
    mFilter->prepare( vlayer->pendingFields() );
  if ( mSymbol )
    mSymbol->startRender( context, vlayer );

  // init children
  // build temporary list of active rules (usable with this scale)
  for ( RuleList::iterator it = mChildren.begin(); it != mChildren.end(); ++it )
  {
    Rule* rule = *it;
    if ( rule->startRender( context, vlayer ) )
    {
      // only add those which are active with current scale
      mActiveChildren.append( rule );
    }
  }
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
      mSymbolNormZLevels.append( normLevel );
    }
  }

  // prepare list of normalized levels for each rule
  for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    rule->setNormZLevels( zLevelsToNormLevels );
  }
}


bool QgsRuleBasedRendererV2::Rule::renderFeature( QgsRuleBasedRendererV2::FeatureToRender& featToRender, QgsRenderContext& context, QgsRuleBasedRendererV2::RenderQueue& renderQueue )
{
  if ( !isFilterOK( featToRender.feat ) )
    return false;

  bool rendered = false;

  // create job for this feature and this symbol, add to list of jobs
  if ( mSymbol )
  {
    // add job to the queue: each symbol's zLevel must be added
    foreach ( int normZLevel, mSymbolNormZLevels )
    {
      //QgsDebugMsg(QString("add job at level %1").arg(normZLevel));
      renderQueue[normZLevel].jobs.append( new RenderJob( featToRender, mSymbol ) );
    }
    rendered = true;
  }

  // process children
  for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    rendered |= rule->renderFeature( featToRender, context, renderQueue );
  }
  return rendered;
}

bool QgsRuleBasedRendererV2::Rule::willRenderFeature( QgsFeature& feat )
{
  if ( !isFilterOK( feat ) )
    return false;
  if ( mSymbol )
    return true;

  for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    if ( rule->willRenderFeature( feat ) )
      return true;
  }
  return false;
}

QgsSymbolV2List QgsRuleBasedRendererV2::Rule::symbolsForFeature( QgsFeature& feat )
{
  QgsSymbolV2List lst;
  if ( !isFilterOK( feat ) )
    return lst;
  if ( mSymbol )
    lst.append( mSymbol );

  for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    lst += rule->symbolsForFeature( feat );
  }
  return lst;
}

QgsRuleBasedRendererV2::RuleList QgsRuleBasedRendererV2::Rule::rulesForFeature( QgsFeature& feat )
{
  RuleList lst;
  if ( !isFilterOK( feat ) )
    return lst;

  if ( mSymbol )
    lst.append( this );

  for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    lst += rule->rulesForFeature( feat );
  }
  return lst;
}

void QgsRuleBasedRendererV2::Rule::stopRender( QgsRenderContext& context )
{
  if ( mSymbol )
    mSymbol->stopRender( context );

  for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule* rule = *it;
    rule->stopRender( context );
  }

  mActiveChildren.clear();
  mSymbolNormZLevels.clear();
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::Rule::create( QDomElement& ruleElem, QgsSymbolV2Map& symbolMap )
{
  QString symbolIdx = ruleElem.attribute( "symbol" );
  QgsSymbolV2* symbol = NULL;
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
  Rule* rule = new Rule( symbol, scaleMinDenom, scaleMaxDenom, filterExp, label, description );

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
    return NULL;
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
          filterExp = filter->dump();
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
  QgsSymbolV2 *symbol = 0;
  if ( layers.size() > 0 )
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
        return NULL;
    }
  }

  // and then create and return the new rule
  return new Rule( symbol, scaleMinDenom, scaleMaxDenom, filterExp, label, description );
}


/////////////////////

QgsRuleBasedRendererV2::QgsRuleBasedRendererV2( QgsRuleBasedRendererV2::Rule* root )
    : QgsFeatureRendererV2( "RuleRenderer" ), mRootRule( root )
{
}

QgsRuleBasedRendererV2::QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol )
    : QgsFeatureRendererV2( "RuleRenderer" )
{
  mRootRule = new Rule( NULL ); // root has no symbol, no filter etc - just a container
  mRootRule->appendChild( new Rule( defaultSymbol ) );
}

QgsRuleBasedRendererV2::~QgsRuleBasedRendererV2()
{
  delete mRootRule;
}


QgsSymbolV2* QgsRuleBasedRendererV2::symbolForFeature( QgsFeature& )
{
  // not used at all
  return 0;
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
  return mRootRule->renderFeature( mCurrentFeatures.last(), context, mRenderQueue );
}


void QgsRuleBasedRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  // prepare active children
  mRootRule->startRender( context, vlayer );

  QSet<int> symbolZLevelsSet = mRootRule->collectZLevels();
  QList<int> symbolZLevels = symbolZLevelsSet.toList();
  qSort( symbolZLevels );

  // create mapping from unnormalized levels [unlimited range] to normalized levels [0..N-1]
  // and prepare rendering queue
  QMap<int, int> zLevelsToNormLevels;
  int maxNormLevel = -1;
  foreach ( int zLevel, symbolZLevels )
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
  foreach ( const RenderLevel& level, mRenderQueue )
  {
    //QgsDebugMsg(QString("level %1").arg(level.zIndex));
    // go through all jobs at the level
    foreach ( const RenderJob* job, level.jobs )
    {
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

QList<QString> QgsRuleBasedRendererV2::usedAttributes()
{
  QSet<QString> attrs = mRootRule->usedAttributes();
  return attrs.values();
}

QgsFeatureRendererV2* QgsRuleBasedRendererV2::clone()
{
  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2( mRootRule->clone() );

  r->setUsingSymbolLevels( usingSymbolLevels() );
  setUsingSymbolLevels( usingSymbolLevels() );
  return r;
}

void QgsRuleBasedRendererV2::toSld( QDomDocument& doc, QDomElement &element ) const
{
  mRootRule->toSld( doc, element, QgsStringMap() );
}

// TODO: ideally this function should be removed in favor of legendSymbol(ogy)Items
QgsSymbolV2List QgsRuleBasedRendererV2::symbols()
{
  return mRootRule->symbols();
}

QDomElement QgsRuleBasedRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "RuleRenderer" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );

  QgsSymbolV2Map symbols;

  QDomElement rulesElem = mRootRule->save( doc, symbols );
  rulesElem.setTagName( "rules" ); // instead of just "rule"
  rendererElem.appendChild( rulesElem );

  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  return rendererElem;
}

QgsLegendSymbologyList QgsRuleBasedRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  QgsLegendSymbolList items = legendSymbolItems();
  for ( QgsLegendSymbolList::iterator it = items.begin(); it != items.end(); it++ )
  {
    QPair<QString, QgsSymbolV2*> pair = *it;
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( pair.second, iconSize );
    lst << qMakePair( pair.first, pix );
  }
  return lst;
}

QgsLegendSymbolList QgsRuleBasedRendererV2::legendSymbolItems()
{
  return mRootRule->legendSymbolItems();
}


QgsFeatureRendererV2* QgsRuleBasedRendererV2::create( QDomElement& element )
{
  // load symbols
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );

  QDomElement rulesElem = element.firstChildElement( "rules" );

  Rule* root = Rule::create( rulesElem, symbolMap );
  if ( root == NULL )
    return NULL;

  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2( root );

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  return r;
}

QgsFeatureRendererV2* QgsRuleBasedRendererV2::createFromSld( QDomElement& element, QGis::GeometryType geomType )
{
  // retrieve child rules
  Rule* root = 0;

  QDomElement ruleElem = element.firstChildElement( "Rule" );
  while ( !ruleElem.isNull() )
  {
    Rule *child = Rule::createFromSld( ruleElem, geomType );
    if ( child )
    {
      // create the root rule if not done before
      if ( !root )
        root = new Rule( 0 );

      root->appendChild( child );
    }

    ruleElem = ruleElem.nextSiblingElement( "Rule" );
  }

  if ( !root )
  {
    // no valid rules was found
    return NULL;
  }

  // create and return the new renderer
  return new QgsRuleBasedRendererV2( root );
}

#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"

void QgsRuleBasedRendererV2::refineRuleCategories( QgsRuleBasedRendererV2::Rule* initialRule, QgsCategorizedSymbolRendererV2* r )
{
  foreach ( const QgsRendererCategoryV2& cat, r->categories() )
  {
    QString attr = QgsExpression::quotedColumnRef( r->classAttribute() );
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
    QString filter = QString( "%1 = %2" ).arg( attr ).arg( value );
    QString label = filter;
    initialRule->appendChild( new Rule( cat.symbol()->clone(), 0, 0, filter, label ) );
  }
}

void QgsRuleBasedRendererV2::refineRuleRanges( QgsRuleBasedRendererV2::Rule* initialRule, QgsGraduatedSymbolRendererV2* r )
{
  foreach ( const QgsRendererRangeV2& rng, r->ranges() )
  {
    // due to the loss of precision in double->string conversion we may miss out values at the limit of the range
    // TODO: have a possibility to construct expressions directly as a parse tree to avoid loss of precision
    QString attr = QgsExpression::quotedColumnRef( r->classAttribute() );
    QString filter = QString( "%1 >= %2 AND %1 <= %3" ).arg( attr )
                     .arg( QString::number( rng.lowerValue(), 'f', 4 ) )
                     .arg( QString::number( rng.upperValue(), 'f', 4 ) );
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
  foreach ( int scale, scales )
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

bool QgsRuleBasedRendererV2::willRenderFeature( QgsFeature& feat )
{
  return mRootRule->willRenderFeature( feat );
}

QgsSymbolV2List QgsRuleBasedRendererV2::symbolsForFeature( QgsFeature& feat )
{
  return mRootRule->symbolsForFeature( feat );
}
