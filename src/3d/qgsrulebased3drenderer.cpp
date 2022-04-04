/***************************************************************************
  qgsrulebased3drenderer.cpp
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrulebased3drenderer.h"

#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"

#include "qgsrulebasedchunkloader_p.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"

QgsRuleBased3DRendererMetadata::QgsRuleBased3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( QStringLiteral( "rulebased" ) )
{
}

QgsAbstract3DRenderer *QgsRuleBased3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement rulesElem = elem.firstChildElement( QStringLiteral( "rules" ) );

  QgsRuleBased3DRenderer::Rule *root = QgsRuleBased3DRenderer::Rule::create( rulesElem, context );
  if ( !root )
    return nullptr;

  QgsRuleBased3DRenderer *r = new QgsRuleBased3DRenderer( root );
  r->readXml( elem, context );
  return r;
}


// ---------


////////////////////

QgsRuleBased3DRenderer::Rule::Rule( QgsAbstract3DSymbol *symbol, const QString &filterExp, const QString &description, bool elseRule )
  : mSymbol( symbol )
  , mFilterExp( filterExp )
  , mDescription( description )
  , mElseRule( elseRule )

{
  initFilter();
}

QgsRuleBased3DRenderer::Rule::~Rule()
{
  qDeleteAll( mChildren );
  // do NOT delete parent
}

void QgsRuleBased3DRenderer::Rule::setSymbol( QgsAbstract3DSymbol *symbol )
{
  if ( mSymbol.get() == symbol )
    return;

  mSymbol.reset( symbol );
}

QgsRuleBased3DRenderer::RuleList QgsRuleBased3DRenderer::Rule::descendants() const
{
  RuleList l;
  for ( Rule *c : mChildren )
  {
    l += c;
    l += c->descendants();
  }
  return l;
}

void QgsRuleBased3DRenderer::Rule::initFilter()
{
  if ( mElseRule || mFilterExp.compare( QLatin1String( "ELSE" ), Qt::CaseInsensitive ) == 0 )
  {
    mElseRule = true;
    mFilter.reset( nullptr );
  }
  else if ( !mFilterExp.isEmpty() )
  {
    mFilter.reset( new QgsExpression( mFilterExp ) );
  }
  else
  {
    mFilter.reset( nullptr );
  }
}

void QgsRuleBased3DRenderer::Rule::updateElseRules()
{
  mElseRules.clear();
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    if ( rule->isElse() )
      mElseRules << rule;
  }
}


void QgsRuleBased3DRenderer::Rule::appendChild( QgsRuleBased3DRenderer::Rule *rule )
{
  mChildren.append( rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBased3DRenderer::Rule::insertChild( int i, QgsRuleBased3DRenderer::Rule *rule )
{
  mChildren.insert( i, rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBased3DRenderer::Rule::removeChildAt( int i )
{
  delete mChildren.at( i );
  mChildren.removeAt( i );
  updateElseRules();
}

const QgsRuleBased3DRenderer::Rule *QgsRuleBased3DRenderer::Rule::findRuleByKey( const QString &key ) const
{
  // we could use a hash / map for search if this will be slow...

  if ( key == mRuleKey )
    return this;

  for ( Rule *rule : std::as_const( mChildren ) )
  {
    const Rule *r = rule->findRuleByKey( key );
    if ( r )
      return r;
  }
  return nullptr;
}

QgsRuleBased3DRenderer::Rule *QgsRuleBased3DRenderer::Rule::findRuleByKey( const QString &key )
{
  if ( key == mRuleKey )
    return this;

  for ( Rule *rule : std::as_const( mChildren ) )
  {
    Rule *r = rule->findRuleByKey( key );
    if ( r )
      return r;
  }
  return nullptr;
}

QgsRuleBased3DRenderer::Rule *QgsRuleBased3DRenderer::Rule::clone() const
{
  QgsAbstract3DSymbol *symbol = mSymbol.get() ? mSymbol->clone() : nullptr;
  Rule *newrule = new Rule( symbol, mFilterExp, mDescription );
  newrule->setActive( mIsActive );
  // clone children
  for ( Rule *rule : std::as_const( mChildren ) )
    newrule->appendChild( rule->clone() );
  return newrule;
}

QgsRuleBased3DRenderer::Rule *QgsRuleBased3DRenderer::Rule::create( const QDomElement &ruleElem, const QgsReadWriteContext &context )
{
  QgsAbstract3DSymbol *symbol = nullptr;
  QDomElement elemSymbol = ruleElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !elemSymbol.isNull() )
  {
    QString symbolType = elemSymbol.attribute( QStringLiteral( "type" ) );
    symbol = QgsApplication::symbol3DRegistry()->createSymbol( symbolType );
    if ( symbol )
      symbol->readXml( elemSymbol, context );
  }

  QString filterExp = ruleElem.attribute( QStringLiteral( "filter" ) );
  QString description = ruleElem.attribute( QStringLiteral( "description" ) );
  QString ruleKey = ruleElem.attribute( QStringLiteral( "key" ) );
  Rule *rule = new Rule( symbol, filterExp, description );

  if ( !ruleKey.isEmpty() )
    rule->mRuleKey = ruleKey;

  rule->setActive( ruleElem.attribute( QStringLiteral( "active" ), QStringLiteral( "1" ) ).toInt() );

  QDomElement childRuleElem = ruleElem.firstChildElement( QStringLiteral( "rule" ) );
  while ( !childRuleElem.isNull() )
  {
    Rule *childRule = create( childRuleElem, context );
    if ( childRule )
    {
      rule->appendChild( childRule );
    }
    else
    {
      //QgsDebugMsg( QStringLiteral( "failed to init a child rule!" ) );
    }
    childRuleElem = childRuleElem.nextSiblingElement( QStringLiteral( "rule" ) );
  }

  return rule;
}

QDomElement QgsRuleBased3DRenderer::Rule::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement ruleElem = doc.createElement( QStringLiteral( "rule" ) );

  if ( mSymbol )
  {
    QDomElement elemSymbol = doc.createElement( QStringLiteral( "symbol" ) );
    elemSymbol.setAttribute( QStringLiteral( "type" ), mSymbol->type() );
    mSymbol->writeXml( elemSymbol, context );
    ruleElem.appendChild( elemSymbol );
  }

  if ( !mFilterExp.isEmpty() )
    ruleElem.setAttribute( QStringLiteral( "filter" ), mFilterExp );
  if ( !mDescription.isEmpty() )
    ruleElem.setAttribute( QStringLiteral( "description" ), mDescription );
  if ( !mIsActive )
    ruleElem.setAttribute( QStringLiteral( "active" ), 0 );
  ruleElem.setAttribute( QStringLiteral( "key" ), mRuleKey );

  for ( RuleList::const_iterator it = mChildren.constBegin(); it != mChildren.constEnd(); ++it )
  {
    Rule *rule = *it;
    ruleElem.appendChild( rule->save( doc, context ) );
  }
  return ruleElem;
}


void QgsRuleBased3DRenderer::Rule::createHandlers( QgsVectorLayer *layer, QgsRuleBased3DRenderer::RuleToHandlerMap &handlers ) const
{
  if ( mSymbol )
  {
    // add handler!
    Q_ASSERT( !handlers.value( this ) );
    QgsFeature3DHandler *handler = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( layer, mSymbol.get() );
    if ( handler )
      handlers[this] = handler;
  }

  // call recursively
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    rule->createHandlers( layer, handlers );
  }
}


void QgsRuleBased3DRenderer::Rule::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, QgsRuleBased3DRenderer::RuleToHandlerMap &handlers ) const
{
  if ( mSymbol )
  {
    QgsFeature3DHandler *handler = handlers[this];
    if ( !handler->prepare( context, attributeNames ) )
    {
      handlers.remove( this );
      delete handler;
    }
  }

  if ( mFilter )
  {
    attributeNames.unite( mFilter->referencedColumns() );
    mFilter->prepare( &context.expressionContext() );
  }

  // call recursively
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    rule->prepare( context, attributeNames, handlers );
  }
}

QgsRuleBased3DRenderer::Rule::RegisterResult QgsRuleBased3DRenderer::Rule::registerFeature( QgsFeature &feature, Qgs3DRenderContext &context, QgsRuleBased3DRenderer::RuleToHandlerMap &handlers ) const
{
  if ( !isFilterOK( feature, context ) )
    return Filtered;

  bool registered = false;

  // do we have active handler for the rule?
  if ( handlers.contains( this ) && mIsActive )
  {
    handlers[this]->processFeature( feature, context );
    registered = true;
  }

  bool matchedAChild = false;

  // call recursively
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    // Don't process else rules yet
    if ( !rule->isElse() )
    {
      const RegisterResult res = rule->registerFeature( feature, context, handlers );
      // consider inactive items as "matched" so the else rule will ignore them
      matchedAChild |= ( res == Registered || res == Inactive );
      registered |= matchedAChild;
    }
  }

  // If none of the rules passed then we jump into the else rules and process them.
  if ( !matchedAChild )
  {
    for ( Rule *rule : std::as_const( mElseRules ) )
    {
      const RegisterResult res = rule->registerFeature( feature, context, handlers );
      matchedAChild |= ( res == Registered || res == Inactive );
      registered |= res != Filtered;
    }
  }

  if ( !mIsActive || ( matchedAChild && !registered ) )
    return Inactive;
  else if ( registered )
    return Registered;
  else
    return Filtered;
}


bool QgsRuleBased3DRenderer::Rule::isFilterOK( QgsFeature &f, Qgs3DRenderContext &context ) const
{
  if ( ! mFilter || mElseRule )
    return true;

  context.expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context.expressionContext() );
  return res.toInt() != 0;
}


////////////////////


QgsRuleBased3DRenderer::QgsRuleBased3DRenderer( QgsRuleBased3DRenderer::Rule *root )
  : mRootRule( root )
{
}

QgsRuleBased3DRenderer::~QgsRuleBased3DRenderer()
{
  delete mRootRule;
}

QgsRuleBased3DRenderer *QgsRuleBased3DRenderer::clone() const
{
  Rule *rootRule = mRootRule->clone();

  // normally with clone() the individual rules get new keys (UUID), but here we want to keep
  // the tree of rules intact, so that other components that may use the rule keys work nicely (e.g. map themes)
  rootRule->setRuleKey( mRootRule->ruleKey() );
  RuleList origDescendants = mRootRule->descendants();
  RuleList clonedDescendants = rootRule->descendants();
  Q_ASSERT( origDescendants.count() == clonedDescendants.count() );
  for ( int i = 0; i < origDescendants.count(); ++i )
    clonedDescendants[i]->setRuleKey( origDescendants[i]->ruleKey() );

  QgsRuleBased3DRenderer *r = new QgsRuleBased3DRenderer( rootRule );
  copyBaseProperties( r );
  return r;
}

Qt3DCore::QEntity *QgsRuleBased3DRenderer::createEntity( const Qgs3DMapSettings &map ) const
{
  QgsVectorLayer *vl = layer();

  if ( !vl )
    return nullptr;

  double zMin, zMax;
  Qgs3DUtils::estimateVectorLayerZRange( vl, zMin, zMax );

  return new QgsRuleBasedChunkedEntity( vl, zMin + map.terrainElevationOffset(), zMax + map.terrainElevationOffset(), tilingSettings(), mRootRule, map );
}

void QgsRuleBased3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  writeXmlBaseProperties( elem, context );

  QDomElement rulesElem = mRootRule->save( doc, context );
  rulesElem.setTagName( QStringLiteral( "rules" ) ); // instead of just "rule"
  elem.appendChild( rulesElem );
}

void QgsRuleBased3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  readXmlBaseProperties( elem, context );

  // root rule is read before class constructed
}
