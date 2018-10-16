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

bool QgsRuleBasedLabelProvider::prepare( const QgsRenderContext &context, QSet<QString> &attributeNames )
{
  Q_FOREACH ( QgsVectorLayerLabelProvider *provider, mSubProviders )
    provider->setEngine( mEngine );

  // populate sub-providers
  mRules->rootRule()->prepare( context, attributeNames, mSubProviders );
  return true;
}

void QgsRuleBasedLabelProvider::registerFeature( QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry )
{
  // will register the feature to relevant sub-providers
  mRules->rootRule()->registerFeature( feature, context, mSubProviders, obstacleGeometry );
}

QList<QgsAbstractLabelProvider *> QgsRuleBasedLabelProvider::subProviders()
{
  QList<QgsAbstractLabelProvider *> lst;
  Q_FOREACH ( QgsVectorLayerLabelProvider *subprovider, mSubProviders )
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

void QgsRuleBasedLabeling::Rule::updateElseRules()
{
  mElseRules.clear();
  Q_FOREACH ( Rule *rule, mChildren )
  {
    if ( rule->isElse() )
      mElseRules << rule;
  }
}

bool QgsRuleBasedLabeling::Rule::requiresAdvancedEffects() const
{
  if ( mSettings && mSettings->format().containsAdvancedEffects() )
    return true;

  Q_FOREACH ( Rule *rule, mChildren )
  {
    if ( rule->requiresAdvancedEffects() )
      return true;
  }

  return false;
}

void QgsRuleBasedLabeling::Rule::subProviderIds( QStringList &list ) const
{
  Q_FOREACH ( const Rule *rule, mChildren )
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

  Q_FOREACH ( Rule *rule, mChildren )
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

  for ( Rule *rule : qgis::as_const( mChildren ) )
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
  Q_FOREACH ( Rule *rule, mChildren )
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
  Q_FOREACH ( Rule *rule, mChildren )
  {
    rule->createSubProviders( layer, subProviders, provider );
  }
}

void QgsRuleBasedLabeling::Rule::prepare( const QgsRenderContext &context, QSet<QString> &attributeNames, QgsRuleBasedLabeling::RuleToProviderMap &subProviders )
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
  Q_FOREACH ( Rule *rule, mChildren )
  {
    rule->prepare( context, attributeNames, subProviders );
  }
}

QgsRuleBasedLabeling::Rule::RegisterResult QgsRuleBasedLabeling::Rule::registerFeature( QgsFeature &feature, QgsRenderContext &context, QgsRuleBasedLabeling::RuleToProviderMap &subProviders, const QgsGeometry &obstacleGeometry )
{
  if ( !isFilterOK( feature, context )
       || !isScaleOK( context.rendererScale() ) )
    return Filtered;

  bool registered = false;

  // do we have active subprovider for the rule?
  if ( subProviders.contains( this ) && mIsActive )
  {
    subProviders[this]->registerFeature( feature, context, obstacleGeometry );
    registered = true;
  }

  bool willRegisterSomething = false;

  // call recursively
  Q_FOREACH ( Rule *rule, mChildren )
  {
    // Don't process else rules yet
    if ( !rule->isElse() )
    {
      RegisterResult res = rule->registerFeature( feature, context, subProviders, obstacleGeometry );
      // consider inactive items as "registered" so the else rule will ignore them
      willRegisterSomething |= ( res == Registered || res == Inactive );
      registered |= willRegisterSomething;
    }
  }

  // If none of the rules passed then we jump into the else rules and process them.
  if ( !willRegisterSomething )
  {
    Q_FOREACH ( Rule *rule, mElseRules )
    {
      registered |= rule->registerFeature( feature, context, subProviders, obstacleGeometry ) != Filtered;
    }
  }

  if ( !mIsActive )
    return Inactive;
  else if ( registered )
    return Registered;
  else
    return Filtered;
}

bool QgsRuleBasedLabeling::Rule::isFilterOK( QgsFeature &f, QgsRenderContext &context ) const
{
  if ( ! mFilter || mElseRule )
    return true;

  context.expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context.expressionContext() );
  return res.toInt() != 0;
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
  delete mRootRule;
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

void QgsRuleBasedLabeling::toSld( QDomNode &parent, const QgsStringMap &props ) const
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
      QgsStringMap localProps = QgsStringMap( props );
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
