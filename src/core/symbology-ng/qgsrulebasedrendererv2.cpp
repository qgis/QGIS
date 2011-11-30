/***************************************************************************
    qgsrulebasedrendererv2.cpp - Rule-based renderer (symbology-ng)
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder.sk at gmail.com
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

#include <QSet>

#include <QDomDocument>
#include <QDomElement>



QgsRuleBasedRendererV2::Rule::Rule( QgsSymbolV2* symbol, int scaleMinDenom, int scaleMaxDenom, QString filterExp, QString label, QString description )
    : mSymbol( symbol ),
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

QString QgsRuleBasedRendererV2::Rule::dump() const
{
  QString symbolDump = ( mSymbol ? mSymbol->dump() : QString( "[]" ) );
  QString msg = QString( "RULE %1 - scale [%2,%3] - filter %4 - symbol %5\n" )
                .arg( mLabel ).arg( mScaleMinDenom ).arg( mScaleMaxDenom )
                .arg( mFilterExp ).arg( symbolDump );

  QStringList lst;
  foreach( Rule* rule, mChildren )
  {
    lst.append( "- " + rule->dump() );
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
  foreach( Rule* rule, mChildren )
  {
    newrule->mChildren.append( rule->clone() );
  }
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


void QgsRuleBasedRendererV2::Rule::renderFeature( QgsFeature* featPtr, QgsRenderContext& context, QgsRuleBasedRendererV2::RenderQueue& renderQueue )
{
  if ( isFilterOK( *featPtr ) )
  {
    // create job for this feature and this symbol, add to list of jobs
    if ( mSymbol )
    {
      // add job to the queue: each symbol's zLevel must be added
      foreach( int normZLevel, mSymbolNormZLevels )
      {
        //QgsDebugMsg(QString("add job at level %1").arg(normZLevel));
        renderQueue[normZLevel].jobs.append( new RenderJob( featPtr, mSymbol ) );
      }
    }

    // process children
    for ( QList<Rule*>::iterator it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
    {
      Rule* rule = *it;
      rule->renderFeature( featPtr, context, renderQueue );
    }
  }
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
      rule->mChildren.append( childRule );
    else
      QgsDebugMsg( "failed to init a child rule!" );
    childRuleElem = childRuleElem.nextSiblingElement( "rule" );
  }

  return rule;
}


/////////////////////

QgsRuleBasedRendererV2::QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol )
    : QgsFeatureRendererV2( "RuleRenderer" )
{
  mRootRule = new Rule( NULL );

  if ( defaultSymbol )
  {
    // add the default rule
    mRootRule->children().append( new Rule( defaultSymbol ) );
  }
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

void QgsRuleBasedRendererV2::renderFeature( QgsFeature& feature,
    QgsRenderContext& context,
    int layer,
    bool selected,
    bool drawVertexMarker )
{

  // TODO: selected features, vertex markers

  QgsFeature* featPtr = NULL;
  if ( !featPtr )
  {
    featPtr = new QgsFeature( feature );
    mCurrentFeatures.append( featPtr );
  }

  // check each active rule
  mRootRule->renderFeature( featPtr, context, mRenderQueue );
}


void QgsRuleBasedRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  // prepare active children
  mRootRule->startRender( context, vlayer );

  QSet<int> symbolZLevelsSet = mRootRule->collectZLevels();

  // create mapping from unnormalized levels [unlimited range] to normalized levels [0..N-1]
  // and prepare rendering queue
  QMap<int, int> zLevelsToNormLevels;
  int maxNormLevel = -1;
  foreach( int zLevel, symbolZLevelsSet.toList() )
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

  // TODO: selected, markers
  bool selected = false;
  bool drawVertexMarker = false;

  // go through all levels
  foreach( const RenderLevel& level, mRenderQueue )
  {
    //QgsDebugMsg(QString("level %1").arg(level.zIndex));
    // go through all jobs at the level
    foreach( const RenderJob* job, level.jobs )
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
          renderFeatureWithSymbol( *job->f, job->symbol, context, i, selected, drawVertexMarker );
        }
      }
    }
  }

  // clean current features
  foreach( QgsFeature* f, mCurrentFeatures )
  {
    delete f;
  }
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
  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2();
  delete r->mRootRule;
  r->mRootRule = mRootRule->clone();

  r->setUsingSymbolLevels( usingSymbolLevels() );
  setUsingSymbolLevels( usingSymbolLevels() );
  return r;
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

  QDomElement rulesElem = doc.createElement( "rules" );
  rulesElem.appendChild( mRootRule->save( doc, symbols ) );
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

  QDomElement rootRuleElem = rulesElem.firstChildElement( "rule" );
  Rule* root = Rule::create( rootRuleElem, symbolMap );
  if ( root == NULL )
    return NULL;

  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2();
  delete r->mRootRule;
  r->mRootRule = root;

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  return r;
}


int QgsRuleBasedRendererV2::ruleCount()
{
  return mRootRule->children().count();
}

QgsRuleBasedRendererV2::Rule* QgsRuleBasedRendererV2::ruleAt( int index )
{
  return mRootRule->children()[index];
}

void QgsRuleBasedRendererV2::addRule( QgsRuleBasedRendererV2::Rule* rule )
{
  mRootRule->children().append( rule );
}

void QgsRuleBasedRendererV2::insertRule( int index, QgsRuleBasedRendererV2::Rule* rule )
{
  mRootRule->children().insert( index, rule );
}

void QgsRuleBasedRendererV2::updateRuleAt( int index, QgsRuleBasedRendererV2::Rule* rule )
{
  RuleList& rules = mRootRule->children();
  delete rules[index]; // delete previous
  rules[index] = rule;
}

void QgsRuleBasedRendererV2::removeRuleAt( int index )
{
  delete mRootRule->children().takeAt( index );
}

void QgsRuleBasedRendererV2::swapRules( int index1,  int index2 )
{
  mRootRule->children().swap( index1, index2 );
}


#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"

QgsRuleBasedRendererV2::RuleList QgsRuleBasedRendererV2::refineRuleCategories( QgsRuleBasedRendererV2::Rule* initialRule, QgsCategorizedSymbolRendererV2* r )
{
  RuleList rules;
  foreach( const QgsRendererCategoryV2& cat, r->categories() )
  {
    QString newfilter = QString( "%1 = '%2'" ).arg( r->classAttribute() ).arg( cat.value().toString() );
    QString filter = initialRule->filterExpression();
    QString label = initialRule->label();
    QString description = initialRule->description();
    if ( filter.isEmpty() )
      filter = newfilter;
    else
      filter = QString( "(%1) AND (%2)" ).arg( filter ).arg( newfilter );
    rules.append( new Rule( cat.symbol()->clone(), initialRule->scaleMinDenom(), initialRule->scaleMaxDenom(), filter, label, description ) );
  }
  return rules;
}

QgsRuleBasedRendererV2::RuleList QgsRuleBasedRendererV2::refineRuleRanges( QgsRuleBasedRendererV2::Rule* initialRule, QgsGraduatedSymbolRendererV2* r )
{
  RuleList rules;
  foreach( const QgsRendererRangeV2& rng, r->ranges() )
  {
    QString newfilter = QString( "%1 >= '%2' AND %1 <= '%3'" ).arg( r->classAttribute() ).arg( rng.lowerValue() ).arg( rng.upperValue() );
    QString filter = initialRule->filterExpression();
    QString label = initialRule->label();
    QString description = initialRule->description();
    if ( filter.isEmpty() )
      filter = newfilter;
    else
      filter = QString( "(%1) AND (%2)" ).arg( filter ).arg( newfilter );
    rules.append( new Rule( rng.symbol()->clone(), initialRule->scaleMinDenom(), initialRule->scaleMaxDenom(), filter, label, description ) );
  }
  return rules;
}

QgsRuleBasedRendererV2::RuleList QgsRuleBasedRendererV2::refineRuleScales( QgsRuleBasedRendererV2::Rule* initialRule, QList<int> scales )
{
  qSort( scales ); // make sure the scales are in ascending order
  RuleList rules;
  int oldScale = initialRule->scaleMinDenom();
  int maxDenom = initialRule->scaleMaxDenom();
  QString filter = initialRule->filterExpression();
  QString label = initialRule->label();
  QString description = initialRule->description();
  QgsSymbolV2* symbol = initialRule->symbol();
  foreach( int scale, scales )
  {
    if ( initialRule->scaleMinDenom() >= scale )
      continue; // jump over the first scales out of the interval
    if ( maxDenom != 0 && maxDenom  <= scale )
      break; // ignore the latter scales out of the interval
    rules.append( new Rule( symbol->clone(), oldScale, scale, filter, label, description ) );
    oldScale = scale;
  }
  // last rule
  rules.append( new Rule( symbol->clone(), oldScale, maxDenom, filter, label, description ) );
  return rules;
}

QString QgsRuleBasedRendererV2::dump()
{
  QString msg( "Rule-based renderer:\n" );
  msg += mRootRule->dump();
  return msg;
}
