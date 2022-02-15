/***************************************************************************
    qgsrulebasedrenderer.cpp - Rule-based renderer (symbology)
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

#include "qgsrulebasedrenderer.h"
#include "qgssymbollayer.h"
#include "qgsexpression.h"
#include "qgssymbollayerutils.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsogcutils.h"
#include "qgssinglesymbolrenderer.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsproperty.h"
#include "qgsstyleentityvisitor.h"
#include "qgsembeddedsymbolrenderer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"

#include <QSet>

#include <QDomDocument>
#include <QDomElement>
#include <QUuid>


QgsRuleBasedRenderer::Rule::Rule( QgsSymbol *symbol, int scaleMinDenom, int scaleMaxDenom, const QString &filterExp, const QString &label, const QString &description, bool elseRule )
  : mParent( nullptr )
  , mSymbol( symbol )
  , mMaximumScale( scaleMinDenom )
  , mMinimumScale( scaleMaxDenom )
  , mFilterExp( filterExp )
  , mLabel( label )
  , mDescription( description )
  , mElseRule( elseRule )
{
  if ( mElseRule )
    mFilterExp = QStringLiteral( "ELSE" );

  mRuleKey = QUuid::createUuid().toString();
  initFilter();
}

QgsRuleBasedRenderer::Rule::~Rule()
{
  qDeleteAll( mChildren );
  // do NOT delete parent
}

void QgsRuleBasedRenderer::Rule::initFilter()
{
  if ( mFilterExp.trimmed().compare( QLatin1String( "ELSE" ), Qt::CaseInsensitive ) == 0 )
  {
    mElseRule = true;
    mFilter.reset();
  }
  else if ( mFilterExp.trimmed().isEmpty() )
  {
    mElseRule = false;
    mFilter.reset();
  }
  else
  {
    mElseRule = false;
    mFilter = std::make_unique< QgsExpression >( mFilterExp );
  }
}

void QgsRuleBasedRenderer::Rule::appendChild( Rule *rule )
{
  mChildren.append( rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBasedRenderer::Rule::insertChild( int i, Rule *rule )
{
  mChildren.insert( i, rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBasedRenderer::Rule::removeChild( Rule *rule )
{
  mChildren.removeAll( rule );
  delete rule;
  updateElseRules();
}

void QgsRuleBasedRenderer::Rule::removeChildAt( int i )
{
  delete mChildren.takeAt( i );
  updateElseRules();
}

QgsRuleBasedRenderer::Rule  *QgsRuleBasedRenderer::Rule::takeChild( Rule *rule )
{
  mChildren.removeAll( rule );
  rule->mParent = nullptr;
  updateElseRules();
  return rule;
}

QgsRuleBasedRenderer::Rule *QgsRuleBasedRenderer::Rule::takeChildAt( int i )
{
  Rule *rule = mChildren.takeAt( i );
  rule->mParent = nullptr;
  updateElseRules();
  return rule;
}

QgsRuleBasedRenderer::Rule *QgsRuleBasedRenderer::Rule::findRuleByKey( const QString &key )
{
  // we could use a hash / map for search if this will be slow...

  if ( key == mRuleKey )
    return this;

  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    Rule *r = rule->findRuleByKey( key );
    if ( r )
      return r;
  }
  return nullptr;
}

void QgsRuleBasedRenderer::Rule::updateElseRules()
{
  mElseRules.clear();
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    if ( rule->isElse() )
      mElseRules << rule;
  }
}

void QgsRuleBasedRenderer::Rule::setIsElse( bool iselse )
{
  mFilterExp = QStringLiteral( "ELSE" );
  mElseRule = iselse;
  mFilter.reset();
}

bool QgsRuleBasedRenderer::Rule::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  // NOTE: if visitEnter returns false it means "don't visit the rule", not "abort all further visitations"
  if ( mParent && !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::SymbolRule, mRuleKey, mLabel ) ) )
    return true;

  if ( mSymbol )
  {
    QgsStyleSymbolEntity entity( mSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) ) )
      return false;
  }

  if ( !mChildren.empty() )
  {
    for ( const Rule *rule : mChildren )
    {

      if ( !rule->accept( visitor ) )
        return false;
    }
  }

  if ( mParent && !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::SymbolRule, mRuleKey, mLabel ) ) )
    return false;

  return true;
}

QString QgsRuleBasedRenderer::Rule::dump( int indent ) const
{
  QString off;
  off.fill( QChar( ' ' ), indent );
  QString symbolDump = ( mSymbol ? mSymbol->dump() : QStringLiteral( "[]" ) );
  QString msg = off + QStringLiteral( "RULE %1 - scale [%2,%3] - filter %4 - symbol %5\n" )
                .arg( mLabel ).arg( mMaximumScale ).arg( mMinimumScale )
                .arg( mFilterExp, symbolDump );

  QStringList lst;
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    lst.append( rule->dump( indent + 2 ) );
  }
  msg += lst.join( QLatin1Char( '\n' ) );
  return msg;
}

QSet<QString> QgsRuleBasedRenderer::Rule::usedAttributes( const QgsRenderContext &context ) const
{
  // attributes needed by this rule
  QSet<QString> attrs;
  if ( mFilter )
    attrs.unite( mFilter->referencedColumns() );
  if ( mSymbol )
    attrs.unite( mSymbol->usedAttributes( context ) );

  // attributes needed by child rules
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    attrs.unite( rule->usedAttributes( context ) );
  }
  return attrs;
}

bool QgsRuleBasedRenderer::Rule::needsGeometry() const
{
  if ( mFilter && mFilter->needsGeometry() )
    return true;

  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    if ( rule->needsGeometry() )
      return true;
  }

  return false;
}

QgsSymbolList QgsRuleBasedRenderer::Rule::symbols( const QgsRenderContext &context ) const
{
  QgsSymbolList lst;
  if ( mSymbol )
    lst.append( mSymbol.get() );

  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    lst += rule->symbols( context );
  }
  return lst;
}

void QgsRuleBasedRenderer::Rule::setSymbol( QgsSymbol *sym )
{
  mSymbol.reset( sym );
}

void QgsRuleBasedRenderer::Rule::setFilterExpression( const QString &filterExp )
{
  mFilterExp = filterExp;
  initFilter();
}

QgsLegendSymbolList QgsRuleBasedRenderer::Rule::legendSymbolItems( int currentLevel ) const
{
  QgsLegendSymbolList lst;
  if ( currentLevel != -1 ) // root rule should not be shown
  {
    lst << QgsLegendSymbolItem( mSymbol.get(), mLabel, mRuleKey, true, mMaximumScale, mMinimumScale, currentLevel, mParent ? mParent->mRuleKey : QString() );
  }

  for ( RuleList::const_iterator it = mChildren.constBegin(); it != mChildren.constEnd(); ++it )
  {
    Rule *rule = *it;
    lst << rule->legendSymbolItems( currentLevel + 1 );
  }
  return lst;
}


bool QgsRuleBasedRenderer::Rule::isFilterOK( const QgsFeature &f, QgsRenderContext *context ) const
{
  if ( ! mFilter || mElseRule || ! context )
    return true;

  context->expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context->expressionContext() );
  return res.toBool();
}

bool QgsRuleBasedRenderer::Rule::isScaleOK( double scale ) const
{
  if ( qgsDoubleNear( scale, 0.0 ) ) // so that we can count features in classes without scale context
    return true;
  if ( qgsDoubleNear( mMaximumScale, 0.0 ) && qgsDoubleNear( mMinimumScale, 0.0 ) )
    return true;
  if ( !qgsDoubleNear( mMaximumScale, 0.0 ) && mMaximumScale > scale )
    return false;
  if ( !qgsDoubleNear( mMinimumScale, 0.0 ) && mMinimumScale < scale )
    return false;
  return true;
}

QgsRuleBasedRenderer::Rule *QgsRuleBasedRenderer::Rule::clone() const
{
  QgsSymbol *sym = mSymbol ? mSymbol->clone() : nullptr;
  Rule *newrule = new Rule( sym, mMaximumScale, mMinimumScale, mFilterExp, mLabel, mDescription );
  newrule->setActive( mIsActive );
  // clone children
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
    newrule->appendChild( rule->clone() );
  return newrule;
}

QDomElement QgsRuleBasedRenderer::Rule::save( QDomDocument &doc, QgsSymbolMap &symbolMap ) const
{
  QDomElement ruleElem = doc.createElement( QStringLiteral( "rule" ) );

  if ( mSymbol )
  {
    int symbolIndex = symbolMap.size();
    symbolMap[QString::number( symbolIndex )] = mSymbol.get();
    ruleElem.setAttribute( QStringLiteral( "symbol" ), symbolIndex );
  }
  if ( !mFilterExp.isEmpty() )
    ruleElem.setAttribute( QStringLiteral( "filter" ), mFilterExp );
  if ( mMaximumScale != 0 )
    ruleElem.setAttribute( QStringLiteral( "scalemindenom" ), mMaximumScale );
  if ( mMinimumScale != 0 )
    ruleElem.setAttribute( QStringLiteral( "scalemaxdenom" ), mMinimumScale );
  if ( !mLabel.isEmpty() )
    ruleElem.setAttribute( QStringLiteral( "label" ), mLabel );
  if ( !mDescription.isEmpty() )
    ruleElem.setAttribute( QStringLiteral( "description" ), mDescription );
  if ( !mIsActive )
    ruleElem.setAttribute( QStringLiteral( "checkstate" ), 0 );
  ruleElem.setAttribute( QStringLiteral( "key" ), mRuleKey );

  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    ruleElem.appendChild( rule->save( doc, symbolMap ) );
  }
  return ruleElem;
}

void QgsRuleBasedRenderer::Rule::toSld( QDomDocument &doc, QDomElement &element, QVariantMap props ) const
{
  // do not convert this rule if there are no symbols
  QgsRenderContext context;
  if ( symbols( context ).isEmpty() )
    return;

  if ( !mFilterExp.isEmpty() )
  {
    QString filter = props.value( QStringLiteral( "filter" ), QString() ).toString();
    if ( !filter.isEmpty() )
      filter += QLatin1String( " AND " );
    filter += mFilterExp;
    props[ QStringLiteral( "filter" )] = filter;
  }

  QgsSymbolLayerUtils::mergeScaleDependencies( mMaximumScale, mMinimumScale, props );

  if ( mSymbol )
  {
    QDomElement ruleElem = doc.createElement( QStringLiteral( "se:Rule" ) );
    element.appendChild( ruleElem );

    //XXX: <se:Name> is the rule identifier, but our the Rule objects
    // have no properties could be used as identifier. Use the label.
    QDomElement nameElem = doc.createElement( QStringLiteral( "se:Name" ) );
    nameElem.appendChild( doc.createTextNode( mLabel ) );
    ruleElem.appendChild( nameElem );

    if ( !mLabel.isEmpty() || !mDescription.isEmpty() )
    {
      QDomElement descrElem = doc.createElement( QStringLiteral( "se:Description" ) );
      if ( !mLabel.isEmpty() )
      {
        QDomElement titleElem = doc.createElement( QStringLiteral( "se:Title" ) );
        titleElem.appendChild( doc.createTextNode( mLabel ) );
        descrElem.appendChild( titleElem );
      }
      if ( !mDescription.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "se:Abstract" ) );
        abstractElem.appendChild( doc.createTextNode( mDescription ) );
        descrElem.appendChild( abstractElem );
      }
      ruleElem.appendChild( descrElem );
    }

    if ( !props.value( QStringLiteral( "filter" ), QString() ).toString().isEmpty() )
    {
      QgsSymbolLayerUtils::createFunctionElement( doc, ruleElem, props.value( QStringLiteral( "filter" ), QString() ).toString() );
    }

    QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElem, props );

    mSymbol->toSld( doc, ruleElem, props );
  }

  // loop into children rule list
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    rule->toSld( doc, element, props );
  }
}

bool QgsRuleBasedRenderer::Rule::startRender( QgsRenderContext &context, const QgsFields &fields, QString &filter )
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
    mSymbol->startRender( context, fields );

  // init children
  // build temporary list of active rules (usable with this scale)
  QStringList subfilters;
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    QString subfilter;
    if ( rule->startRender( context, fields, subfilter ) )
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
    if ( subfilters.contains( QStringLiteral( "TRUE" ) ) )
    {
      sf = QStringLiteral( "TRUE" );
    }
    else
    {
      // test for a common case -- all subfilters can be combined into a single "field in (...)" expression
      if ( QgsExpression::attemptReduceToInClause( subfilters, sf ) )
      {
        // success! we can use a simple "field IN (...)" list!
      }
      // If we have more than 50 rules (to stay on the safe side) make a binary tree or SQLITE will fail,
      // see: https://github.com/qgis/QGIS/issues/27269
      else if ( subfilters.count() > 50 )
      {
        std::function<QString( const QStringList & )>bt = [ &bt ]( const QStringList & subf )
        {
          if ( subf.count( ) == 1 )
          {
            return subf.at( 0 );
          }
          else if ( subf.count( ) == 2 )
          {
            return subf.join( QLatin1String( ") OR (" ) ).prepend( '(' ).append( ')' );
          }
          else
          {
            int midpos = static_cast<int>( subf.length() / 2 );
            return QStringLiteral( "(%1) OR (%2)" ).arg( bt( subf.mid( 0, midpos ) ), bt( subf.mid( midpos ) ) );
          }
        };
        sf = bt( subfilters );
      }
      else
      {
        sf = subfilters.join( QLatin1String( ") OR (" ) ).prepend( '(' ).append( ')' );
      }
    }
  }

  // Now join the subfilters with their parent (this) based on if
  // * The parent is an else rule
  // * The existence of parent filter and subfilters

  // No filter expression: ELSE rule or catchall rule
  if ( !mFilter )
  {
    if ( mSymbol || sf.isEmpty() )
      filter = QStringLiteral( "TRUE" );
    else
      filter = sf;
  }
  else if ( mSymbol )
    filter = mFilterExp;
  else if ( !mFilterExp.trimmed().isEmpty() && !sf.isEmpty() )
    filter = QStringLiteral( "(%1) AND (%2)" ).arg( mFilterExp, sf );
  else if ( !mFilterExp.trimmed().isEmpty() )
    filter = mFilterExp;
  else if ( sf.isEmpty() )
    filter = QStringLiteral( "TRUE" );
  else
    filter = sf;

  filter = filter.trimmed();

  return true;
}

QSet<int> QgsRuleBasedRenderer::Rule::collectZLevels()
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
  QList<Rule *>::iterator it;
  for ( it = mActiveChildren.begin(); it != mActiveChildren.end(); ++it )
  {
    Rule *rule = *it;
    symbolZLevelsSet.unite( rule->collectZLevels() );
  }
  return symbolZLevelsSet;
}

void QgsRuleBasedRenderer::Rule::setNormZLevels( const QMap<int, int> &zLevelsToNormLevels )
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
  const auto constMActiveChildren = mActiveChildren;
  for ( Rule *rule : constMActiveChildren )
  {
    rule->setNormZLevels( zLevelsToNormLevels );
  }
}


QgsRuleBasedRenderer::Rule::RenderResult QgsRuleBasedRenderer::Rule::renderFeature( QgsRuleBasedRenderer::FeatureToRender &featToRender, QgsRenderContext &context, QgsRuleBasedRenderer::RenderQueue &renderQueue )
{
  if ( !isFilterOK( featToRender.feat, &context ) )
    return Filtered;

  bool rendered = false;

  // create job for this feature and this symbol, add to list of jobs
  if ( mSymbol && mIsActive )
  {
    // add job to the queue: each symbol's zLevel must be added
    const auto constMSymbolNormZLevels = mSymbolNormZLevels;
    for ( int normZLevel : constMSymbolNormZLevels )
    {
      //QgsDebugMsg(QString("add job at level %1").arg(normZLevel));
      renderQueue[normZLevel].jobs.append( new RenderJob( featToRender, mSymbol.get() ) );
      rendered = true;
    }
  }

  bool matchedAChild = false;

  // process children
  const auto constMChildren = mChildren;
  for ( Rule *rule : constMChildren )
  {
    // Don't process else rules yet
    if ( !rule->isElse() )
    {
      const RenderResult res = rule->renderFeature( featToRender, context, renderQueue );
      // consider inactive items as "matched" so the else rule will ignore them
      matchedAChild |= ( res == Rendered || res == Inactive );
      rendered |= ( res == Rendered );
    }
  }

  // If none of the rules passed then we jump into the else rules and process them.
  if ( !matchedAChild )
  {
    const auto constMElseRules = mElseRules;
    for ( Rule *rule : constMElseRules )
    {
      const RenderResult res = rule->renderFeature( featToRender, context, renderQueue );
      matchedAChild |= ( res == Rendered || res == Inactive );
      rendered |= res == Rendered;
    }
  }
  if ( !mIsActive || ( mSymbol && !rendered ) || ( matchedAChild && !rendered ) )
    return Inactive;
  else if ( rendered )
    return Rendered;
  else
    return Filtered;
}

bool QgsRuleBasedRenderer::Rule::willRenderFeature( const QgsFeature &feature, QgsRenderContext *context )
{
  if ( !isFilterOK( feature, context ) )
    return false;

  if ( mSymbol )
    return true;

  const auto constMActiveChildren = mActiveChildren;
  for ( Rule *rule : constMActiveChildren )
  {
    if ( rule->isElse() )
    {
      if ( rule->children().isEmpty() )
      {
        RuleList lst = rulesForFeature( feature, context, false );
        lst.removeOne( rule );

        if ( lst.empty() )
        {
          return true;
        }
      }
      else
      {
        return rule->willRenderFeature( feature, context );
      }
    }
    else if ( rule->willRenderFeature( feature, context ) )
    {
      return true;
    }
  }
  return false;
}

QgsSymbolList QgsRuleBasedRenderer::Rule::symbolsForFeature( const QgsFeature &feature, QgsRenderContext *context )
{
  QgsSymbolList lst;
  if ( !isFilterOK( feature, context ) )
    return lst;
  if ( mSymbol )
    lst.append( mSymbol.get() );

  const auto constMActiveChildren = mActiveChildren;
  for ( Rule *rule : constMActiveChildren )
  {
    lst += rule->symbolsForFeature( feature, context );
  }
  return lst;
}

QSet<QString> QgsRuleBasedRenderer::Rule::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext *context )
{
  QSet< QString> lst;
  if ( !isFilterOK( feature, context ) )
    return lst;
  lst.insert( mRuleKey );

  const auto constMActiveChildren = mActiveChildren;
  for ( Rule *rule : constMActiveChildren )
  {
    bool validKey = false;
    if ( rule->isElse() )
    {
      RuleList lst = rulesForFeature( feature, context, false );
      lst.removeOne( rule );

      if ( lst.empty() )
      {
        validKey = true;
      }
    }
    else if ( !rule->isElse( ) && rule->willRenderFeature( feature, context ) )
    {
      validKey = true;
    }

    if ( validKey )
    {
      lst.unite( rule->legendKeysForFeature( feature, context ) );
    }
  }
  return lst;
}

QgsRuleBasedRenderer::RuleList QgsRuleBasedRenderer::Rule::rulesForFeature( const QgsFeature &feature, QgsRenderContext *context, bool onlyActive )
{
  RuleList lst;
  if ( ! isFilterOK( feature, context ) || ( context && ! isScaleOK( context->rendererScale() ) ) )
    return lst;

  if ( mSymbol )
    lst.append( this );

  RuleList listChildren = children();
  if ( onlyActive )
    listChildren = mActiveChildren;

  const auto constListChildren = listChildren;
  for ( Rule *rule : constListChildren )
  {
    lst += rule->rulesForFeature( feature, context, onlyActive );
  }
  return lst;
}

void QgsRuleBasedRenderer::Rule::stopRender( QgsRenderContext &context )
{
  if ( mSymbol )
    mSymbol->stopRender( context );

  const auto constMActiveChildren = mActiveChildren;
  for ( Rule *rule : constMActiveChildren )
  {
    rule->stopRender( context );
  }

  mActiveChildren.clear();
  mSymbolNormZLevels.clear();
}

QgsRuleBasedRenderer::Rule *QgsRuleBasedRenderer::Rule::create( QDomElement &ruleElem, QgsSymbolMap &symbolMap )
{
  QString symbolIdx = ruleElem.attribute( QStringLiteral( "symbol" ) );
  QgsSymbol *symbol = nullptr;
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

  QString filterExp = ruleElem.attribute( QStringLiteral( "filter" ) );
  QString label = ruleElem.attribute( QStringLiteral( "label" ) );
  QString description = ruleElem.attribute( QStringLiteral( "description" ) );
  int scaleMinDenom = ruleElem.attribute( QStringLiteral( "scalemindenom" ), QStringLiteral( "0" ) ).toInt();
  int scaleMaxDenom = ruleElem.attribute( QStringLiteral( "scalemaxdenom" ), QStringLiteral( "0" ) ).toInt();
  QString ruleKey = ruleElem.attribute( QStringLiteral( "key" ) );
  Rule *rule = new Rule( symbol, scaleMinDenom, scaleMaxDenom, filterExp, label, description );

  if ( !ruleKey.isEmpty() )
    rule->mRuleKey = ruleKey;

  rule->setActive( ruleElem.attribute( QStringLiteral( "checkstate" ), QStringLiteral( "1" ) ).toInt() );

  QDomElement childRuleElem = ruleElem.firstChildElement( QStringLiteral( "rule" ) );
  while ( !childRuleElem.isNull() )
  {
    Rule *childRule = create( childRuleElem, symbolMap );
    if ( childRule )
    {
      rule->appendChild( childRule );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "failed to init a child rule!" ) );
    }
    childRuleElem = childRuleElem.nextSiblingElement( QStringLiteral( "rule" ) );
  }

  return rule;
}

QgsRuleBasedRenderer::RuleList QgsRuleBasedRenderer::Rule::descendants() const
{
  RuleList l;
  for ( QgsRuleBasedRenderer::Rule *c : mChildren )
  {
    l += c;
    l += c->descendants();
  }
  return l;
}

QgsRuleBasedRenderer::Rule *QgsRuleBasedRenderer::Rule::createFromSld( QDomElement &ruleElem, QgsWkbTypes::GeometryType geomType )
{
  if ( ruleElem.localName() != QLatin1String( "Rule" ) )
  {
    QgsDebugMsg( QStringLiteral( "invalid element: Rule element expected, %1 found!" ).arg( ruleElem.tagName() ) );
    return nullptr;
  }

  QString label, description, filterExp;
  int scaleMinDenom = 0, scaleMaxDenom = 0;
  QgsSymbolLayerList layers;

  // retrieve the Rule element child nodes
  QDomElement childElem = ruleElem.firstChildElement();
  while ( !childElem.isNull() )
  {
    if ( childElem.localName() == QLatin1String( "Name" ) )
    {
      // <se:Name> tag contains the rule identifier,
      // so prefer title tag for the label property value
      if ( label.isEmpty() )
        label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == QLatin1String( "Description" ) )
    {
      // <se:Description> can contains a title and an abstract
      QDomElement titleElem = childElem.firstChildElement( QStringLiteral( "Title" ) );
      if ( !titleElem.isNull() )
      {
        label = titleElem.firstChild().nodeValue();
      }

      QDomElement abstractElem = childElem.firstChildElement( QStringLiteral( "Abstract" ) );
      if ( !abstractElem.isNull() )
      {
        description = abstractElem.firstChild().nodeValue();
      }
    }
    else if ( childElem.localName() == QLatin1String( "Abstract" ) )
    {
      // <sld:Abstract> (v1.0)
      description = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == QLatin1String( "Title" ) )
    {
      // <sld:Title> (v1.0)
      label = childElem.firstChild().nodeValue();
    }
    else if ( childElem.localName() == QLatin1String( "Filter" ) )
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
    else if ( childElem.localName() == QLatin1String( "MinScaleDenominator" ) )
    {
      bool ok;
      int v = childElem.firstChild().nodeValue().toInt( &ok );
      if ( ok )
        scaleMinDenom = v;
    }
    else if ( childElem.localName() == QLatin1String( "MaxScaleDenominator" ) )
    {
      bool ok;
      int v = childElem.firstChild().nodeValue().toInt( &ok );
      if ( ok )
        scaleMaxDenom = v;
    }
    else if ( childElem.localName().endsWith( QLatin1String( "Symbolizer" ) ) )
    {
      // create symbol layers for this symbolizer
      QgsSymbolLayerUtils::createSymbolLayerListFromSld( childElem, geomType, layers );
    }

    childElem = childElem.nextSiblingElement();
  }

  // now create the symbol
  QgsSymbol *symbol = nullptr;
  if ( !layers.isEmpty() )
  {
    switch ( geomType )
    {
      case QgsWkbTypes::LineGeometry:
        symbol = new QgsLineSymbol( layers );
        break;

      case QgsWkbTypes::PolygonGeometry:
        symbol = new QgsFillSymbol( layers );
        break;

      case QgsWkbTypes::PointGeometry:
        symbol = new QgsMarkerSymbol( layers );
        break;

      default:
        QgsDebugMsg( QStringLiteral( "invalid geometry type: found %1" ).arg( geomType ) );
        return nullptr;
    }
  }

  // and then create and return the new rule
  return new Rule( symbol, scaleMinDenom, scaleMaxDenom, filterExp, label, description );
}


/////////////////////

QgsRuleBasedRenderer::QgsRuleBasedRenderer( QgsRuleBasedRenderer::Rule *root )
  : QgsFeatureRenderer( QStringLiteral( "RuleRenderer" ) )
  , mRootRule( root )
{
}

QgsRuleBasedRenderer::QgsRuleBasedRenderer( QgsSymbol *defaultSymbol )
  : QgsFeatureRenderer( QStringLiteral( "RuleRenderer" ) )
{
  mRootRule = new Rule( nullptr ); // root has no symbol, no filter etc - just a container
  mRootRule->appendChild( new Rule( defaultSymbol ) );
}

QgsRuleBasedRenderer::~QgsRuleBasedRenderer()
{
  delete mRootRule;
}


QgsSymbol *QgsRuleBasedRenderer::symbolForFeature( const QgsFeature &, QgsRenderContext & ) const
{
  // not used at all
  return nullptr;
}

bool QgsRuleBasedRenderer::renderFeature( const QgsFeature &feature,
    QgsRenderContext &context,
    int layer,
    bool selected,
    bool drawVertexMarker )
{
  Q_UNUSED( layer )

  int flags = ( selected ? FeatIsSelected : 0 ) | ( drawVertexMarker ? FeatDrawMarkers : 0 );
  mCurrentFeatures.append( FeatureToRender( feature, flags ) );

  // check each active rule
  return mRootRule->renderFeature( mCurrentFeatures.last(), context, mRenderQueue ) == Rule::Rendered;
}


void QgsRuleBasedRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  // prepare active children
  mRootRule->startRender( context, fields, mFilter );

  QSet<int> symbolZLevelsSet = mRootRule->collectZLevels();
  QList<int> symbolZLevels = qgis::setToList( symbolZLevelsSet );
  std::sort( symbolZLevels.begin(), symbolZLevels.end() );

  // create mapping from unnormalized levels [unlimited range] to normalized levels [0..N-1]
  // and prepare rendering queue
  QMap<int, int> zLevelsToNormLevels;
  int maxNormLevel = -1;
  const auto constSymbolZLevels = symbolZLevels;
  for ( int zLevel : constSymbolZLevels )
  {
    zLevelsToNormLevels[zLevel] = ++maxNormLevel;
    mRenderQueue.append( RenderLevel( zLevel ) );
    QgsDebugMsgLevel( QStringLiteral( "zLevel %1 -> %2" ).arg( zLevel ).arg( maxNormLevel ), 4 );
  }

  mRootRule->setNormZLevels( zLevelsToNormLevels );
}

void QgsRuleBasedRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  //
  // do the actual rendering
  //

  // go through all levels
  if ( !context.renderingStopped() )
  {
    const auto constMRenderQueue = mRenderQueue;
    for ( const RenderLevel &level : constMRenderQueue )
    {
      //QgsDebugMsg(QString("level %1").arg(level.zIndex));
      // go through all jobs at the level
      for ( const RenderJob *job : std::as_const( level.jobs ) )
      {
        context.expressionContext().setFeature( job->ftr.feat );
        //QgsDebugMsg(QString("job fid %1").arg(job->f->id()));
        // render feature - but only with symbol layers with specified zIndex
        QgsSymbol *s = job->symbol;
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
  }

  // clean current features
  mCurrentFeatures.clear();

  // clean render queue
  mRenderQueue.clear();

  // clean up rules from temporary stuff
  mRootRule->stopRender( context );
}

QString QgsRuleBasedRenderer::filter( const QgsFields & )
{
  return mFilter;
}

QSet<QString> QgsRuleBasedRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  return mRootRule->usedAttributes( context );
}

bool QgsRuleBasedRenderer::filterNeedsGeometry() const
{
  return mRootRule->needsGeometry();
}

QgsRuleBasedRenderer *QgsRuleBasedRenderer::clone() const
{
  QgsRuleBasedRenderer::Rule *clonedRoot = mRootRule->clone();

  // normally with clone() the individual rules get new keys (UUID), but here we want to keep
  // the tree of rules intact, so that other components that may use the rule keys work nicely (e.g. map themes)
  clonedRoot->setRuleKey( mRootRule->ruleKey() );
  RuleList origDescendants = mRootRule->descendants();
  RuleList clonedDescendants = clonedRoot->descendants();
  Q_ASSERT( origDescendants.count() == clonedDescendants.count() );
  for ( int i = 0; i < origDescendants.count(); ++i )
    clonedDescendants[i]->setRuleKey( origDescendants[i]->ruleKey() );

  QgsRuleBasedRenderer *r = new QgsRuleBasedRenderer( clonedRoot );

  copyRendererData( r );
  return r;
}

void QgsRuleBasedRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  mRootRule->toSld( doc, element, props );
}

// TODO: ideally this function should be removed in favor of legendSymbol(ogy)Items
QgsSymbolList QgsRuleBasedRenderer::symbols( QgsRenderContext &context ) const
{
  return mRootRule->symbols( context );
}

QDomElement QgsRuleBasedRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "RuleRenderer" ) );

  QgsSymbolMap symbols;

  QDomElement rulesElem = mRootRule->save( doc, symbols );
  rulesElem.setTagName( QStringLiteral( "rules" ) ); // instead of just "rule"
  rendererElem.appendChild( rulesElem );

  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  rendererElem.appendChild( symbolsElem );

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

bool QgsRuleBasedRenderer::legendSymbolItemsCheckable() const
{
  return true;
}

bool QgsRuleBasedRenderer::legendSymbolItemChecked( const QString &key )
{
  Rule *rule = mRootRule->findRuleByKey( key );
  return rule ? rule->active() : true;
}

void QgsRuleBasedRenderer::checkLegendSymbolItem( const QString &key, bool state )
{
  Rule *rule = mRootRule->findRuleByKey( key );
  if ( rule )
    rule->setActive( state );
}

void QgsRuleBasedRenderer::setLegendSymbolItem( const QString &key, QgsSymbol *symbol )
{
  Rule *rule = mRootRule->findRuleByKey( key );
  if ( rule )
    rule->setSymbol( symbol );
  else
    delete symbol;
}

QgsLegendSymbolList QgsRuleBasedRenderer::legendSymbolItems() const
{
  return mRootRule->legendSymbolItems();
}


QgsFeatureRenderer *QgsRuleBasedRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  // load symbols
  QDomElement symbolsElem = element.firstChildElement( QStringLiteral( "symbols" ) );
  if ( symbolsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );

  QDomElement rulesElem = element.firstChildElement( QStringLiteral( "rules" ) );

  Rule *root = Rule::create( rulesElem, symbolMap );
  if ( !root )
    return nullptr;

  QgsRuleBasedRenderer *r = new QgsRuleBasedRenderer( root );

  // delete symbols if there are any more
  QgsSymbolLayerUtils::clearSymbolMap( symbolMap );

  return r;
}

QgsFeatureRenderer *QgsRuleBasedRenderer::createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType )
{
  // retrieve child rules
  Rule *root = nullptr;

  QDomElement ruleElem = element.firstChildElement( QStringLiteral( "Rule" ) );
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

    ruleElem = ruleElem.nextSiblingElement( QStringLiteral( "Rule" ) );
  }

  if ( !root )
  {
    // no valid rules was found
    return nullptr;
  }

  // create and return the new renderer
  return new QgsRuleBasedRenderer( root );
}

#include "qgscategorizedsymbolrenderer.h"
#include "qgsgraduatedsymbolrenderer.h"

void QgsRuleBasedRenderer::refineRuleCategories( QgsRuleBasedRenderer::Rule *initialRule, QgsCategorizedSymbolRenderer *r )
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

  const auto constCategories = r->categories();
  for ( const QgsRendererCategory &cat : constCategories )
  {
    QString value;
    // not quoting numbers saves a type cast
    if ( cat.value().isNull() )
      value = "NULL";
    else if ( cat.value().type() == QVariant::Int )
      value = cat.value().toString();
    else if ( cat.value().type() == QVariant::Double )
      // we loose precision here - so we may miss some categories :-(
      // TODO: have a possibility to construct expressions directly as a parse tree to avoid loss of precision
      value = QString::number( cat.value().toDouble(), 'f', 4 );
    else
      value = QgsExpression::quotedString( cat.value().toString() );
    const QString filter = QStringLiteral( "%1 %2 %3" ).arg( attr, cat.value().isNull() ? QStringLiteral( "IS" ) : QStringLiteral( "=" ), value );
    const QString label = !cat.label().isEmpty() ? cat.label() :
                          cat.value().isValid() ? value : QString();
    initialRule->appendChild( new Rule( cat.symbol()->clone(), 0, 0, filter, label ) );
  }
}

void QgsRuleBasedRenderer::refineRuleRanges( QgsRuleBasedRenderer::Rule *initialRule, QgsGraduatedSymbolRenderer *r )
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
    attr = QStringLiteral( "(%1)" ).arg( attr );
  }

  bool firstRange = true;
  const auto constRanges = r->ranges();
  for ( const QgsRendererRange &rng : constRanges )
  {
    // due to the loss of precision in double->string conversion we may miss out values at the limit of the range
    // TODO: have a possibility to construct expressions directly as a parse tree to avoid loss of precision
    QString filter = QStringLiteral( "%1 %2 %3 AND %1 <= %4" ).arg( attr, firstRange ? QStringLiteral( ">=" ) : QStringLiteral( ">" ),
                     QString::number( rng.lowerValue(), 'f', 4 ),
                     QString::number( rng.upperValue(), 'f', 4 ) );
    firstRange = false;
    QString label = rng.label().isEmpty() ? filter : rng.label();
    initialRule->appendChild( new Rule( rng.symbol()->clone(), 0, 0, filter, label ) );
  }
}

void QgsRuleBasedRenderer::refineRuleScales( QgsRuleBasedRenderer::Rule *initialRule, QList<int> scales )
{
  std::sort( scales.begin(), scales.end() ); // make sure the scales are in ascending order
  double oldScale = initialRule->maximumScale();
  double maxDenom = initialRule->minimumScale();
  QgsSymbol *symbol = initialRule->symbol();
  const auto constScales = scales;
  for ( int scale : constScales )
  {
    if ( initialRule->maximumScale() >= scale )
      continue; // jump over the first scales out of the interval
    if ( maxDenom != 0 && maxDenom  <= scale )
      break; // ignore the latter scales out of the interval
    initialRule->appendChild( new Rule( symbol->clone(), oldScale, scale, QString(), QStringLiteral( "%1 - %2" ).arg( oldScale ).arg( scale ) ) );
    oldScale = scale;
  }
  // last rule
  initialRule->appendChild( new Rule( symbol->clone(), oldScale, maxDenom, QString(), QStringLiteral( "%1 - %2" ).arg( oldScale ).arg( maxDenom ) ) );
}

QString QgsRuleBasedRenderer::dump() const
{
  QString msg( QStringLiteral( "Rule-based renderer:\n" ) );
  msg += mRootRule->dump();
  return msg;
}

bool QgsRuleBasedRenderer::willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return mRootRule->willRenderFeature( feature, &context );
}

QgsSymbolList QgsRuleBasedRenderer::symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return mRootRule->symbolsForFeature( feature, &context );
}

QgsSymbolList QgsRuleBasedRenderer::originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return mRootRule->symbolsForFeature( feature, &context );
}

QSet< QString > QgsRuleBasedRenderer::legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  return mRootRule->legendKeysForFeature( feature, &context );
}

bool QgsRuleBasedRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  return mRootRule->accept( visitor );
}

QgsRuleBasedRenderer *QgsRuleBasedRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer, QgsVectorLayer *layer )
{
  std::unique_ptr< QgsRuleBasedRenderer > r;
  if ( renderer->type() == QLatin1String( "RuleRenderer" ) )
  {
    r.reset( dynamic_cast<QgsRuleBasedRenderer *>( renderer->clone() ) );
  }
  else if ( renderer->type() == QLatin1String( "singleSymbol" ) )
  {
    const QgsSingleSymbolRenderer *singleSymbolRenderer = dynamic_cast<const QgsSingleSymbolRenderer *>( renderer );
    if ( !singleSymbolRenderer )
      return nullptr;

    std::unique_ptr< QgsSymbol > origSymbol( singleSymbolRenderer->symbol()->clone() );
    r = std::make_unique< QgsRuleBasedRenderer >( origSymbol.release() );
  }
  else if ( renderer->type() == QLatin1String( "categorizedSymbol" ) )
  {
    const QgsCategorizedSymbolRenderer *categorizedRenderer = dynamic_cast<const QgsCategorizedSymbolRenderer *>( renderer );
    if ( !categorizedRenderer )
      return nullptr;

    QString attr = categorizedRenderer->classAttribute();
    // categorizedAttr could be either an attribute name or an expression.
    bool isField = false;
    if ( layer )
    {
      isField = QgsExpression::expressionToLayerFieldIndex( attr, layer ) != -1;
    }
    else
    {
      QgsExpression testExpr( attr );
      isField = testExpr.hasParserError() || testExpr.isField();
    }
    if ( isField && !attr.contains( '\"' ) )
    {
      //not an expression, so need to quote column name
      attr = QgsExpression::quotedColumnRef( attr );
    }

    std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );

    QString expression;
    QString value;
    QgsRendererCategory category;
    for ( const QgsRendererCategory &category : categorizedRenderer->categories() )
    {
      std::unique_ptr< QgsRuleBasedRenderer::Rule > rule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );

      rule->setLabel( category.label() );

      //We first define the rule corresponding to the category
      if ( category.value().type() == QVariant::List )
      {
        QStringList values;
        const QVariantList list = category.value().toList();
        for ( const QVariant &v : list )
        {
          //If the value is a number, we can use it directly, otherwise we need to quote it in the rule
          if ( QVariant( v ).convert( QVariant::Double ) )
          {
            values << v.toString();
          }
          else
          {
            values << QgsExpression::quotedString( v.toString() );
          }
        }

        if ( values.empty() )
        {
          expression = QStringLiteral( "ELSE" );
        }
        else
        {
          expression = QStringLiteral( "%1 IN (%2)" ).arg( attr, values.join( ',' ) );
        }
      }
      else
      {
        //If the value is a number, we can use it directly, otherwise we need to quote it in the rule
        if ( category.value().convert( QVariant::Double ) )
        {
          value = category.value().toString();
        }
        else
        {
          value = QgsExpression::quotedString( category.value().toString() );
        }

        //An empty category is equivalent to the ELSE keyword
        if ( value == QLatin1String( "''" ) )
        {
          expression = QStringLiteral( "ELSE" );
        }
        else
        {
          expression = QStringLiteral( "%1 = %2" ).arg( attr, value );
        }
      }
      rule->setFilterExpression( expression );

      //Then we construct an equivalent symbol.
      //Ideally we could simply copy the symbol, but the categorized renderer allows a separate interface to specify
      //data dependent area and rotation, so we need to convert these to obtain the same rendering

      std::unique_ptr< QgsSymbol > origSymbol( category.symbol()->clone() );
      rule->setSymbol( origSymbol.release() );

      rootrule->appendChild( rule.release() );
    }

    r = std::make_unique< QgsRuleBasedRenderer >( rootrule.release() );
  }
  else if ( renderer->type() == QLatin1String( "graduatedSymbol" ) )
  {
    const QgsGraduatedSymbolRenderer *graduatedRenderer = dynamic_cast<const QgsGraduatedSymbolRenderer *>( renderer );
    if ( !graduatedRenderer )
      return nullptr;

    QString attr = graduatedRenderer->classAttribute();
    // categorizedAttr could be either an attribute name or an expression.
    // the only way to differentiate is to test it as an expression...
    bool isField = false;
    if ( layer )
    {
      isField = QgsExpression::expressionToLayerFieldIndex( attr, layer ) != -1;
    }
    else
    {
      QgsExpression testExpr( attr );
      isField = testExpr.hasParserError() || testExpr.isField();
    }
    if ( isField  && !attr.contains( '\"' ) )
    {
      //not an expression, so need to quote column name
      attr = QgsExpression::quotedColumnRef( attr );
    }
    else if ( !isField )
    {
      //otherwise wrap expression in brackets
      attr = QStringLiteral( "(%1)" ).arg( attr );
    }

    std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );

    QString expression;
    QgsRendererRange range;
    for ( int i = 0; i < graduatedRenderer->ranges().size(); ++i )
    {
      range = graduatedRenderer->ranges().value( i );
      std::unique_ptr< QgsRuleBasedRenderer::Rule > rule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
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

      std::unique_ptr< QgsSymbol > symbol( range.symbol()->clone() );
      rule->setSymbol( symbol.release() );

      rootrule->appendChild( rule.release() );
    }

    r = std::make_unique< QgsRuleBasedRenderer >( rootrule.release() );
  }
  else if ( renderer->type() == QLatin1String( "pointDisplacement" ) || renderer->type() == QLatin1String( "pointCluster" ) )
  {
    if ( const QgsPointDistanceRenderer *pointDistanceRenderer = dynamic_cast<const QgsPointDistanceRenderer *>( renderer ) )
      return convertFromRenderer( pointDistanceRenderer->embeddedRenderer() );
  }
  else if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
  {
    if ( const QgsInvertedPolygonRenderer *invertedPolygonRenderer = dynamic_cast<const QgsInvertedPolygonRenderer *>( renderer ) )
      r.reset( convertFromRenderer( invertedPolygonRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == QLatin1String( "mergedFeatureRenderer" ) )
  {
    if ( const QgsMergedFeatureRenderer *mergedRenderer = dynamic_cast<const QgsMergedFeatureRenderer *>( renderer ) )
      r.reset( convertFromRenderer( mergedRenderer->embeddedRenderer() ) );
  }
  else if ( renderer->type() == QLatin1String( "embeddedSymbol" ) && layer )
  {
    const QgsEmbeddedSymbolRenderer *embeddedRenderer = dynamic_cast<const QgsEmbeddedSymbolRenderer *>( renderer );

    std::unique_ptr< QgsRuleBasedRenderer::Rule > rootrule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );

    QgsFeatureRequest req;
    req.setFlags( QgsFeatureRequest::EmbeddedSymbols | QgsFeatureRequest::NoGeometry );
    req.setNoAttributes();
    QgsFeatureIterator it = layer->getFeatures( req );
    QgsFeature feature;
    while ( it.nextFeature( feature ) && rootrule->children().size() < 500 )
    {
      if ( feature.embeddedSymbol() )
      {
        std::unique_ptr< QgsRuleBasedRenderer::Rule > rule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
        rule->setFilterExpression( QStringLiteral( "$id=%1" ).arg( feature.id() ) );
        rule->setLabel( QString::number( feature.id() ) );
        rule->setSymbol( feature.embeddedSymbol()->clone() );
        rootrule->appendChild( rule.release() );
      }
    }

    std::unique_ptr< QgsRuleBasedRenderer::Rule > rule = std::make_unique< QgsRuleBasedRenderer::Rule >( nullptr );
    rule->setFilterExpression( QStringLiteral( "ELSE" ) );
    rule->setLabel( QObject::tr( "All other features" ) );
    rule->setSymbol( embeddedRenderer->defaultSymbol()->clone() );
    rootrule->appendChild( rule.release() );

    r = std::make_unique< QgsRuleBasedRenderer >( rootrule.release() );
  }

  if ( r )
  {
    renderer->copyRendererData( r.get() );
  }

  return r.release();
}

void QgsRuleBasedRenderer::convertToDataDefinedSymbology( QgsSymbol *symbol, const QString &sizeScaleField, const QString &rotationField )
{
  QString sizeExpression;
  switch ( symbol->type() )
  {
    case Qgis::SymbolType::Marker:
      for ( int j = 0; j < symbol->symbolLayerCount(); ++j )
      {
        QgsMarkerSymbolLayer *msl = static_cast<QgsMarkerSymbolLayer *>( symbol->symbolLayer( j ) );
        if ( ! sizeScaleField.isEmpty() )
        {
          sizeExpression = QStringLiteral( "%1*(%2)" ).arg( msl->size() ).arg( sizeScaleField );
          msl->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( sizeExpression ) );
        }
        if ( ! rotationField.isEmpty() )
        {
          msl->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromField( rotationField ) );
        }
      }
      break;
    case Qgis::SymbolType::Line:
      if ( ! sizeScaleField.isEmpty() )
      {
        for ( int j = 0; j < symbol->symbolLayerCount(); ++j )
        {
          if ( symbol->symbolLayer( j )->layerType() == QLatin1String( "SimpleLine" ) )
          {
            QgsLineSymbolLayer *lsl = static_cast<QgsLineSymbolLayer *>( symbol->symbolLayer( j ) );
            sizeExpression = QStringLiteral( "%1*(%2)" ).arg( lsl->width() ).arg( sizeScaleField );
            lsl->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsProperty::fromExpression( sizeExpression ) );
          }
          if ( symbol->symbolLayer( j )->layerType() == QLatin1String( "MarkerLine" ) )
          {
            QgsSymbol *marker = symbol->symbolLayer( j )->subSymbol();
            for ( int k = 0; k < marker->symbolLayerCount(); ++k )
            {
              QgsMarkerSymbolLayer *msl = static_cast<QgsMarkerSymbolLayer *>( marker->symbolLayer( k ) );
              sizeExpression = QStringLiteral( "%1*(%2)" ).arg( msl->size() ).arg( sizeScaleField );
              msl->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( sizeExpression ) );
            }
          }
        }
      }
      break;
    default:
      break;
  }
}
