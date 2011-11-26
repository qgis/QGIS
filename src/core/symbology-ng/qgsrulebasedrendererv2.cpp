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

QgsRuleBasedRendererV2::Rule::Rule( const QgsRuleBasedRendererV2::Rule& other )
    : mSymbol( NULL ), mFilter( NULL )
{
  *this = other;
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
  return QString( "RULE %1 - scale [%2,%3] - filter %4 - symbol %5\n" )
         .arg( mLabel ).arg( mScaleMinDenom ).arg( mScaleMaxDenom )
         .arg( mFilterExp ).arg( mSymbol->dump() );

}

QStringList QgsRuleBasedRendererV2::Rule::needsFields() const
{
  if ( ! mFilter )
    return QStringList();

  return mFilter->referencedColumns();
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

QgsRuleBasedRendererV2::Rule& QgsRuleBasedRendererV2::Rule::operator=( const QgsRuleBasedRendererV2::Rule & other )
{
  if ( this != &other )
  {
    delete mSymbol;
    mSymbol = other.mSymbol->clone();

    mScaleMinDenom = other.mScaleMinDenom;
    mScaleMaxDenom = other.mScaleMaxDenom;
    mFilterExp = other.mFilterExp;
    mLabel = other.mLabel;
    mDescription = other.mDescription;
    initFilter();
  }
  return *this;
}

/////////////////////

QgsRuleBasedRendererV2::QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol )
    : QgsFeatureRendererV2( "RuleRenderer" ), mDefaultSymbol( defaultSymbol ), mCurrentSymbol( 0 )
{
  // add the default rule
  mRules << Rule( defaultSymbol->clone() );
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

  // check each active rule
  QgsFeature* featPtr = NULL;
  for ( QList<Rule*>::iterator it = mCurrentRules.begin(); it != mCurrentRules.end(); ++it )
  {
    Rule* rule = *it;
    // if matching: add rendering job to queue
    if ( rule->isFilterOK( feature ) )
    {
      //QgsDebugMsg(QString("matching fid %1").arg(feature.id()));
      // make a copy of the feature if not yet exists
      if ( !featPtr )
      {
        featPtr = new QgsFeature( feature );
        mCurrentFeatures.append( featPtr );
      }

      // create job for this feature and this symbol, add to list of jobs
      //RenderJob* job = new RenderJob( featPtr, rule->symbol() );
      //mRenderJobs.append( job );
      // add job to the queue: each symbol's zLevel must be added
      foreach( int normZLevel, rule->mSymbolNormZLevels )
      {
        //QgsDebugMsg(QString("add job at level %1").arg(normZLevel));
        mRenderQueue.levels[normZLevel].jobs.append( new RenderJob( featPtr, rule->symbol() ) );
      }
    }
  }

}


void QgsRuleBasedRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer )
{
  double currentScale = context.rendererScale();
  // filter out rules which are not compatible with this scale

  // build temporary list of active rules (usable with this scale)
  mCurrentRules.clear();
  for ( QList<Rule>::iterator it = mRules.begin(); it != mRules.end(); ++it )
  {
    Rule& rule = *it;
    if ( rule.isScaleOK( currentScale ) )
      mCurrentRules.append( &rule );
  }

  QgsFieldMap pendingFields = vlayer->pendingFields();

  QSet<int> symbolZLevelsSet;

  QList<Rule*>::iterator it;
  for ( it = mCurrentRules.begin(); it != mCurrentRules.end(); ++it )
  {
    Rule* rule = *it;
    QgsExpression* exp = rule->filter();
    if ( exp )
      exp->prepare( pendingFields );

    // prepare symbol
    QgsSymbolV2* s = rule->symbol();
    s->startRender( context, vlayer );

    QgsDebugMsg( "rule " + rule->dump() );

    // find out which Z-levels are used
    for ( int i = 0; i < s->symbolLayerCount(); i++ )
    {
      symbolZLevelsSet.insert( s->symbolLayer( i )->renderingPass() );
      rule->mSymbolNormZLevels.clear();
    }
  }

  // create mapping from unnormalized levels [unlimited range] to normalized levels [0..N-1]
  // and prepare rendering queue
  QMap<int, int> zLevelsToNormLevels;
  int maxNormLevel = -1;
  mRenderQueue.levels.clear();
  foreach( int zLevel, symbolZLevelsSet.toList() )
  {
    zLevelsToNormLevels[zLevel] = ++maxNormLevel;
    mRenderQueue.levels.append( RenderLevel( zLevel ) );
    QgsDebugMsg( QString( "zLevel %1 -> %2" ).arg( zLevel ).arg( maxNormLevel ) );
  }

  // prepare list of normalized levels for each rule
  for ( it = mCurrentRules.begin(); it != mCurrentRules.end(); ++it )
  {
    Rule* rule = *it;
    QgsSymbolV2* s = rule->symbol();
    for ( int i = 0; i < s->symbolLayerCount(); i++ )
    {
      int normLevel = zLevelsToNormLevels.value( s->symbolLayer( i )->renderingPass() );
      rule->mSymbolNormZLevels.append( normLevel );
    }
  }
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
  foreach( const RenderLevel& level, mRenderQueue.levels )
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

  // TODO:
  // clear render queue, render jobs

  foreach( QgsFeature* f, mCurrentFeatures )
  {
    delete f;
  }
  mCurrentFeatures.clear();

  for ( QList<Rule*>::iterator it = mCurrentRules.begin(); it != mCurrentRules.end(); ++it )
  {
    Rule* rule = *it;
    rule->symbol()->stopRender( context );
  }

  mCurrentRules.clear();
}

QList<QString> QgsRuleBasedRendererV2::usedAttributes()
{
  QSet<QString> attrs;
  for ( QList<Rule>::iterator it = mRules.begin(); it != mRules.end(); ++it )
  {
    Rule& rule = *it;
    attrs.unite( rule.needsFields().toSet() );
    if ( rule.symbol() )
    {
      attrs.unite( rule.symbol()->usedAttributes() );
    }
  }
  return attrs.values();
}

QgsFeatureRendererV2* QgsRuleBasedRendererV2::clone()
{
  QgsSymbolV2* s = mDefaultSymbol->clone();
  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2( s );
  r->mRules = mRules;
  r->setUsingSymbolLevels( usingSymbolLevels() );
  setUsingSymbolLevels( usingSymbolLevels() );
  return r;
}

QgsSymbolV2List QgsRuleBasedRendererV2::symbols()
{
  QgsSymbolV2List lst;
  for ( QList<Rule>::iterator it = mRules.begin(); it != mRules.end(); ++it )
  {
    Rule& rule = *it;
    lst.append( rule.symbol() );
  }

  return lst;
}

QDomElement QgsRuleBasedRendererV2::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "RuleRenderer" );
  rendererElem.setAttribute( "symbollevels", ( mUsingSymbolLevels ? "1" : "0" ) );

  QDomElement rulesElem = doc.createElement( "rules" );

  QgsSymbolV2Map symbols;
  symbols["default"] = mDefaultSymbol;

  int i = 0;
  for ( QList<Rule>::iterator it = mRules.begin(); it != mRules.end(); ++i, ++it )
  {
    Rule& rule = *it;
    symbols[QString::number( i )] = rule.symbol();
    QDomElement ruleElem = doc.createElement( "rule" );
    ruleElem.setAttribute( "symbol", i );
    ruleElem.setAttribute( "filter", rule.filterExpression() );
    ruleElem.setAttribute( "scalemindenom", rule.scaleMinDenom() );
    ruleElem.setAttribute( "scalemaxdenom", rule.scaleMaxDenom() );
    ruleElem.setAttribute( "label", rule.label() );
    ruleElem.setAttribute( "description", rule.description() );
    rulesElem.appendChild( ruleElem );
  }
  rendererElem.appendChild( rulesElem );

  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( symbols, "symbols", doc );
  rendererElem.appendChild( symbolsElem );

  return rendererElem;
}


QgsLegendSymbologyList QgsRuleBasedRendererV2::legendSymbologyItems( QSize iconSize )
{
  QgsLegendSymbologyList lst;
  for ( QList<Rule>::iterator it = mRules.begin(); it != mRules.end(); ++it )
  {
    QPixmap pix = QgsSymbolLayerV2Utils::symbolPreviewPixmap( it->symbol(), iconSize );
    lst << qMakePair( it->label(), pix );
  }
  return lst;
}

QgsLegendSymbolList QgsRuleBasedRendererV2::legendSymbolItems()
{
  QgsLegendSymbolList lst;
  for ( QList<Rule>::iterator it = mRules.begin(); it != mRules.end(); ++it )
  {
    lst << qMakePair( it->label(), it->symbol() );
  }
  return lst;
}


QgsFeatureRendererV2* QgsRuleBasedRendererV2::create( QDomElement& element )
{
  // load symbols
  QDomElement symbolsElem = element.firstChildElement( "symbols" );
  if ( symbolsElem.isNull() )
    return NULL;

  QgsSymbolV2Map symbolMap = QgsSymbolLayerV2Utils::loadSymbols( symbolsElem );

  if ( !symbolMap.contains( "default" ) )
  {
    QgsDebugMsg( "default symbol not found!" );
    return NULL;
  }

  QgsRuleBasedRendererV2* r = new QgsRuleBasedRendererV2( symbolMap.take( "default" ) );
  r->mRules.clear();

  QDomElement rulesElem = element.firstChildElement( "rules" );
  QDomElement ruleElem = rulesElem.firstChildElement( "rule" );
  while ( !ruleElem.isNull() )
  {
    QString symbolIdx = ruleElem.attribute( "symbol" );
    if ( symbolMap.contains( symbolIdx ) )
    {
      QString filterExp = ruleElem.attribute( "filter" );
      QString label = ruleElem.attribute( "label" );
      QString description = ruleElem.attribute( "description" );
      int scaleMinDenom = ruleElem.attribute( "scalemindenom", "0" ).toInt();
      int scaleMaxDenom = ruleElem.attribute( "scalemaxdenom", "0" ).toInt();
      r->mRules.append( Rule( symbolMap.take( symbolIdx ), scaleMinDenom, scaleMaxDenom, filterExp, label, description ) );
    }
    else
    {
      QgsDebugMsg( "symbol for rule " + symbolIdx + " not found! (skipping)" );
    }
    ruleElem = ruleElem.nextSiblingElement( "rule" );
  }

  // delete symbols if there are any more
  QgsSymbolLayerV2Utils::clearSymbolMap( symbolMap );

  return r;
}


int QgsRuleBasedRendererV2::ruleCount()
{
  return mRules.count();
}

QgsRuleBasedRendererV2::Rule& QgsRuleBasedRendererV2::ruleAt( int index )
{
  return mRules[index];
}

void QgsRuleBasedRendererV2::addRule( const QgsRuleBasedRendererV2::Rule& rule )
{
  mRules.append( rule );
}

void QgsRuleBasedRendererV2::insertRule( int index, const QgsRuleBasedRendererV2::Rule& rule )
{
  mRules.insert( index, rule );
}

void QgsRuleBasedRendererV2::updateRuleAt( int index, const QgsRuleBasedRendererV2::Rule& rule )
{
  mRules[index] = rule;
}

void QgsRuleBasedRendererV2::removeRuleAt( int index )
{
  mRules.removeAt( index );
}

void QgsRuleBasedRendererV2::swapRules( int index1,  int index2 )
{
  mRules.swap( index1, index2 );
}


#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"

QList<QgsRuleBasedRendererV2::Rule> QgsRuleBasedRendererV2::refineRuleCategories( QgsRuleBasedRendererV2::Rule& initialRule, QgsCategorizedSymbolRendererV2* r )
{
  QList<Rule> rules;
  foreach( const QgsRendererCategoryV2& cat, r->categories() )
  {
    QString newfilter = QString( "%1 = '%2'" ).arg( r->classAttribute() ).arg( cat.value().toString() );
    QString filter = initialRule.filterExpression();
    QString label = initialRule.label();
    QString description = initialRule.description();
    if ( filter.isEmpty() )
      filter = newfilter;
    else
      filter = QString( "(%1) AND (%2)" ).arg( filter ).arg( newfilter );
    rules.append( Rule( cat.symbol()->clone(), initialRule.scaleMinDenom(), initialRule.scaleMaxDenom(), filter, initialRule.label(), initialRule.description() ) );
  }
  return rules;
}

QList<QgsRuleBasedRendererV2::Rule> QgsRuleBasedRendererV2::refineRuleRanges( QgsRuleBasedRendererV2::Rule& initialRule, QgsGraduatedSymbolRendererV2* r )
{
  QList<Rule> rules;
  foreach( const QgsRendererRangeV2& rng, r->ranges() )
  {
    QString newfilter = QString( "%1 >= '%2' AND %1 <= '%3'" ).arg( r->classAttribute() ).arg( rng.lowerValue() ).arg( rng.upperValue() );
    QString filter = initialRule.filterExpression();
    QString label = initialRule.label();
    QString description = initialRule.description();
    if ( filter.isEmpty() )
      filter = newfilter;
    else
      filter = QString( "(%1) AND (%2)" ).arg( filter ).arg( newfilter );
    rules.append( Rule( rng.symbol()->clone(), initialRule.scaleMinDenom(), initialRule.scaleMaxDenom(), filter, initialRule.label(), initialRule.description() ) );
  }
  return rules;
}

QList<QgsRuleBasedRendererV2::Rule> QgsRuleBasedRendererV2::refineRuleScales( QgsRuleBasedRendererV2::Rule& initialRule, QList<int> scales )
{
  qSort( scales ); // make sure the scales are in ascending order
  QList<Rule> rules;
  int oldScale = initialRule.scaleMinDenom();
  int maxDenom = initialRule.scaleMaxDenom();
  foreach( int scale, scales )
  {
    if ( initialRule.scaleMinDenom() >= scale )
      continue; // jump over the first scales out of the interval
    if ( maxDenom != 0 && maxDenom  <= scale )
      break; // ignore the latter scales out of the interval
    rules.append( Rule( initialRule.symbol()->clone(), oldScale, scale, initialRule.filterExpression(), initialRule.label(), initialRule.description() ) );
    oldScale = scale;
  }
  // last rule
  rules.append( Rule( initialRule.symbol()->clone(), oldScale, maxDenom, initialRule.filterExpression(), initialRule.label(), initialRule.description() ) );
  return rules;
}

QString QgsRuleBasedRendererV2::dump()
{
  QString msg( "Rule-based renderer:\n" );
  foreach( const Rule& rule, mRules )
  {
    msg += rule.dump();
  }
  return msg;
}
