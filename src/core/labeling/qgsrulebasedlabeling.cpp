/***************************************************************************
    qgsrulebasedlabeling.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrulebasedlabeling.h"
#include "qgssymbollayerutils.h"
#include "qgsstyleentityvisitor.h"

QgsRuleBasedLabelProvider::QgsRuleBasedLabelProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer *layer, bool withFeatureLoop )
  : QgsVectorLayerLabelProvider( layer, QString(), withFeatureLoop, nullptr )
{
  mRules.reset( rules.clone() );
  mRules->rootRule()->createSubProviders( layer, mSubProviders, this );
}

QgsVectorLayerLabelProvider *QgsRuleBasedLabelProvider::createProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings )
{
  return new QgsVectorLayerLabelProvider( layer, providerId, withFeatureLoop, settings );
}

bool QgsRuleBasedLabelProvider::prepare( QgsRenderContext &context, QSet<QString> &attributeNames )
{
  for ( QgsVectorLayerLabelProvider *provider : std::as_const( mSubProviders ) )
    provider->setEngine( mEngine );

  // populate sub-providers
  mRules->rootRule()->prepare( context, attributeNames, mSubProviders );
  return true;
}

QList<QgsLabelFeature *> QgsRuleBasedLabelProvider::registerFeature( const QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry, const QgsSymbol *symbol )
{
  // will register the feature to relevant sub-providers
  return std::get< 1 >( mRules->rootRule()->registerFeature( feature, context, mSubProviders, obstacleGeometry, symbol ) );
}

QList<QgsAbstractLabelProvider *> QgsRuleBasedLabelProvider::subProviders()
{
  QList<QgsAbstractLabelProvider *> lst;
  for ( QgsVectorLayerLabelProvider *subprovider : std::as_const( mSubProviders ) )
    lst << subprovider;
  return lst;
}


////////////////////

QgsRuleBasedLabeling::Rule::Rule( QgsPalLayerSettings *settings, double scaleMinDenom, double scaleMaxDenom, const QString &filterExp, const QString &description, bool elseRule )
  : mSettings( settings )
  , mMaximumScale( scaleMinDenom )
  , mMinimumScale( scaleMaxDenom )
  , mFilterExp( filterExp )
  , mDescription( description )
  , mElseRule( elseRule )
{
  if ( mElseRule )
    mFilterExp = QStringLiteral( "ELSE" );

  initFilter();
}

QgsRuleBasedLabeling::Rule::~Rule()
{
  qDeleteAll( mChildren );
  // do NOT delete parent
}

void QgsRuleBasedLabeling::Rule::setSettings( QgsPalLayerSettings *settings )
{
  if ( mSettings.get() == settings )
    return;

  mSettings.reset( settings );
}

QgsRuleBasedLabeling::RuleList QgsRuleBasedLabeling::Rule::descendants() const
{
  RuleList l;
  for ( Rule *c : mChildren )
  {
    l += c;
    l += c->descendants();
  }
  return l;
}

void QgsRuleBasedLabeling::Rule::initFilter()
{
  if ( mFilterExp.trimmed().compare( QLatin1String( "ELSE" ), Qt::CaseInsensitive ) == 0 )
  {
    mElseRule = true;
    mFilter.reset( );
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

void QgsRuleBasedLabeling::Rule::updateElseRules()
{
  mElseRules.clear();
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    if ( rule->isElse() )
      mElseRules << rule;
  }
}

bool QgsRuleBasedLabeling::Rule::requiresAdvancedEffects() const
{
  if ( mSettings && mSettings->format().containsAdvancedEffects() )
    return true;

  for ( Rule *rule : std::as_const( mChildren ) )
  {
    if ( rule->requiresAdvancedEffects() )
      return true;
  }

  return false;
}

bool QgsRuleBasedLabeling::Rule::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  // NOTE: if visitEnter returns false it means "don't visit the rule", not "abort all further visitations"
  if ( mParent && !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::SymbolRule, mRuleKey, mDescription ) ) )
    return true;

  if ( mSettings )
  {
    QgsStyleLabelSettingsEntity entity( *mSettings );
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

  if ( mParent && !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::SymbolRule, mRuleKey, mDescription ) ) )
    return false;

  return true;
}

void QgsRuleBasedLabeling::Rule::subProviderIds( QStringList &list ) const
{
  for ( const Rule *rule : std::as_const( mChildren ) )
  {
    if ( rule->settings() )
      list << rule->ruleKey();

    rule->subProviderIds( list );
  }
}


void QgsRuleBasedLabeling::Rule::appendChild( QgsRuleBasedLabeling::Rule *rule )
{
  mChildren.append( rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBasedLabeling::Rule::insertChild( int i, QgsRuleBasedLabeling::Rule *rule )
{
  mChildren.insert( i, rule );
  rule->mParent = this;
  updateElseRules();
}

void QgsRuleBasedLabeling::Rule::removeChildAt( int i )
{
  delete mChildren.at( i );
  mChildren.removeAt( i );
  updateElseRules();
}

const QgsRuleBasedLabeling::Rule *QgsRuleBasedLabeling::Rule::findRuleByKey( const QString &key ) const
{
  // we could use a hash / map for search if this will be slow...

  if ( key == mRuleKey )
    return this;

  for ( Rule *rule : mChildren )
  {
    const Rule *r = rule->findRuleByKey( key );
    if ( r )
      return r;
  }
  return nullptr;
}

QgsRuleBasedLabeling::Rule *QgsRuleBasedLabeling::Rule::findRuleByKey( const QString &key )
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

QgsRuleBasedLabeling::Rule *QgsRuleBasedLabeling::Rule::clone() const
{
  QgsPalLayerSettings *s = mSettings.get() ? new QgsPalLayerSettings( *mSettings ) : nullptr;
  Rule *newrule = new Rule( s, mMaximumScale, mMinimumScale, mFilterExp, mDescription );
  newrule->setActive( mIsActive );
  // clone children
  for ( Rule *rule : mChildren )
    newrule->appendChild( rule->clone() );
  return newrule;
}

QgsRuleBasedLabeling::Rule *QgsRuleBasedLabeling::Rule::create( const QDomElement &ruleElem, const QgsReadWriteContext &context )
{
  QgsPalLayerSettings *settings = nullptr;
  QDomElement settingsElem = ruleElem.firstChildElement( QStringLiteral( "settings" ) );
  if ( !settingsElem.isNull() )
  {
    settings = new QgsPalLayerSettings;
    settings->readXml( settingsElem, context );
  }

  QString filterExp = ruleElem.attribute( QStringLiteral( "filter" ) );
  QString description = ruleElem.attribute( QStringLiteral( "description" ) );
  int scaleMinDenom = ruleElem.attribute( QStringLiteral( "scalemindenom" ), QStringLiteral( "0" ) ).toInt();
  int scaleMaxDenom = ruleElem.attribute( QStringLiteral( "scalemaxdenom" ), QStringLiteral( "0" ) ).toInt();
  QString ruleKey = ruleElem.attribute( QStringLiteral( "key" ) );
  Rule *rule = new Rule( settings, scaleMinDenom, scaleMaxDenom, filterExp, description );

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

QDomElement QgsRuleBasedLabeling::Rule::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement ruleElem = doc.createElement( QStringLiteral( "rule" ) );

  if ( mSettings )
  {
    ruleElem.appendChild( mSettings->writeXml( doc, context ) );
  }
  if ( !mFilterExp.isEmpty() )
    ruleElem.setAttribute( QStringLiteral( "filter" ), mFilterExp );
  if ( !qgsDoubleNear( mMaximumScale, 0 ) )
    ruleElem.setAttribute( QStringLiteral( "scalemindenom" ), mMaximumScale );
  if ( !qgsDoubleNear( mMinimumScale, 0 ) )
    ruleElem.setAttribute( QStringLiteral( "scalemaxdenom" ), mMinimumScale );
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

void QgsRuleBasedLabeling::Rule::createSubProviders( QgsVectorLayer *layer, QgsRuleBasedLabeling::RuleToProviderMap &subProviders, QgsRuleBasedLabelProvider *provider )
{
  if ( mSettings )
  {
    // add provider!
    QgsVectorLayerLabelProvider *p = provider->createProvider( layer, mRuleKey, false, mSettings.get() );
    delete subProviders.value( this, nullptr );
    subProviders[this] = p;
  }

  // call recursively
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    rule->createSubProviders( layer, subProviders, provider );
  }
}

void QgsRuleBasedLabeling::Rule::prepare( QgsRenderContext &context, QSet<QString> &attributeNames, QgsRuleBasedLabeling::RuleToProviderMap &subProviders )
{
  if ( mSettings )
  {
    QgsVectorLayerLabelProvider *p = subProviders[this];
    if ( !p->prepare( context, attributeNames ) )
    {
      subProviders.remove( this );
      delete p;
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
    rule->prepare( context, attributeNames, subProviders );
  }
}

std::tuple< QgsRuleBasedLabeling::Rule::RegisterResult, QList< QgsLabelFeature * > > QgsRuleBasedLabeling::Rule::registerFeature( const QgsFeature &feature, QgsRenderContext &context, QgsRuleBasedLabeling::RuleToProviderMap &subProviders, const QgsGeometry &obstacleGeometry, const QgsSymbol *symbol )
{
  QList< QgsLabelFeature * > labels;
  if ( !isFilterOK( feature, context )
       || !isScaleOK( context.rendererScale() ) )
  {
    return { Filtered, labels };
  }

  bool registered = false;

  // do we have active subprovider for the rule?
  if ( subProviders.contains( this ) && mIsActive )
  {
    labels.append( subProviders[this]->registerFeature( feature, context, obstacleGeometry, symbol ) );
    registered = true;
  }

  bool matchedAChild = false;

  // call recursively
  for ( Rule *rule : std::as_const( mChildren ) )
  {
    // Don't process else rules yet
    if ( !rule->isElse() )
    {
      RegisterResult res;
      QList< QgsLabelFeature * > added;
      std::tie( res, added ) = rule->registerFeature( feature, context, subProviders, obstacleGeometry );
      labels.append( added );
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
      RegisterResult res;
      QList< QgsLabelFeature * > added;
      std::tie( res, added ) = rule->registerFeature( feature, context, subProviders, obstacleGeometry, symbol ) ;
      matchedAChild |= ( res == Registered || res == Inactive );
      registered |= res != Filtered;
      labels.append( added );
    }
  }

  if ( !mIsActive || ( matchedAChild && !registered ) )
    return { Inactive, labels };
  else if ( registered )
    return { Registered, labels };
  else
    return { Filtered, labels };
}

bool QgsRuleBasedLabeling::Rule::isFilterOK( const QgsFeature &f, QgsRenderContext &context ) const
{
  if ( ! mFilter || mElseRule )
    return true;

  context.expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context.expressionContext() );
  return res.toBool();
}

bool QgsRuleBasedLabeling::Rule::isScaleOK( double scale ) const
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

////////////////////

QgsRuleBasedLabeling::QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule *root )
  : mRootRule( root )
{
}

QgsRuleBasedLabeling *QgsRuleBasedLabeling::clone() const
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

  return new QgsRuleBasedLabeling( rootRule );
}

QgsRuleBasedLabeling::~QgsRuleBasedLabeling()
{
}

QgsRuleBasedLabeling::Rule *QgsRuleBasedLabeling::rootRule()
{
  return mRootRule.get();
}

const QgsRuleBasedLabeling::Rule *QgsRuleBasedLabeling::rootRule() const
{
  return mRootRule.get();
}


QgsRuleBasedLabeling *QgsRuleBasedLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement rulesElem = element.firstChildElement( QStringLiteral( "rules" ) );

  Rule *root = Rule::create( rulesElem, context );
  if ( !root )
    return nullptr;

  QgsRuleBasedLabeling *rl = new QgsRuleBasedLabeling( root );
  return rl;
}

QString QgsRuleBasedLabeling::type() const
{
  return QStringLiteral( "rule-based" );
}

QDomElement QgsRuleBasedLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "labeling" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "rule-based" ) );

  QDomElement rulesElem = mRootRule->save( doc, context );
  rulesElem.setTagName( QStringLiteral( "rules" ) ); // instead of just "rule"
  elem.appendChild( rulesElem );

  return elem;
}

QgsVectorLayerLabelProvider *QgsRuleBasedLabeling::provider( QgsVectorLayer *layer ) const
{
  return new QgsRuleBasedLabelProvider( *this, layer, false );
}

QStringList QgsRuleBasedLabeling::subProviders() const
{
  QStringList lst;
  mRootRule->subProviderIds( lst );
  return lst;
}

QgsPalLayerSettings QgsRuleBasedLabeling::settings( const QString &providerId ) const
{
  const Rule *rule = mRootRule->findRuleByKey( providerId );
  if ( rule && rule->settings() )
    return *rule->settings();

  return QgsPalLayerSettings();
}

bool QgsRuleBasedLabeling::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  return mRootRule->accept( visitor );
}

bool QgsRuleBasedLabeling::requiresAdvancedEffects() const
{
  return mRootRule->requiresAdvancedEffects();
}

void QgsRuleBasedLabeling::setSettings( QgsPalLayerSettings *settings, const QString &providerId )
{
  if ( settings )
  {
    Rule *rule = mRootRule->findRuleByKey( providerId );
    if ( rule && rule->settings() )
      rule->setSettings( settings );
  }
}

void QgsRuleBasedLabeling::toSld( QDomNode &parent, const QVariantMap &props ) const
{
  if ( !mRootRule )
  {
    return;
  }

  const QgsRuleBasedLabeling::RuleList rules = mRootRule->children();
  for ( Rule *rule : rules )
  {
    QgsPalLayerSettings *settings = rule->settings();

    if ( settings && settings->drawLabels )
    {
      QDomDocument doc = parent.ownerDocument();

      QDomElement ruleElement = doc.createElement( QStringLiteral( "se:Rule" ) );
      parent.appendChild( ruleElement );

      if ( !rule->filterExpression().isEmpty() )
      {
        QgsSymbolLayerUtils::createFunctionElement( doc, ruleElement, rule->filterExpression() );
      }

      // scale dependencies, the actual behavior is that the PAL settings min/max and
      // the rule min/max get intersected
      QVariantMap localProps = QVariantMap( props );
      QgsSymbolLayerUtils::mergeScaleDependencies( rule->maximumScale(), rule->minimumScale(), localProps );
      if ( settings->scaleVisibility )
      {
        QgsSymbolLayerUtils::mergeScaleDependencies( settings->maximumScale, settings->minimumScale, localProps );
      }
      QgsSymbolLayerUtils::applyScaleDependency( doc, ruleElement, localProps );

      QgsAbstractVectorLayerLabeling::writeTextSymbolizer( ruleElement, *settings, props );
    }

  }

}
