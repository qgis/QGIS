#include "qgsrulebasedlabeling.h"



QgsRuleBasedLabelProvider::QgsRuleBasedLabelProvider( const QgsRuleBasedLabeling& rules, QgsVectorLayer* layer, bool withFeatureLoop )
    : QgsVectorLayerLabelProvider( layer, withFeatureLoop )
    , mRules( rules )
{
  mRules.rootRule()->createSubProviders( layer, mSubProviders );
}

QgsRuleBasedLabelProvider::~QgsRuleBasedLabelProvider()
{
  // sub-providers owned by labeling engine
}


bool QgsRuleBasedLabelProvider::prepare( const QgsRenderContext& context, QStringList& attributeNames )
{
  Q_FOREACH ( QgsVectorLayerLabelProvider* provider, mSubProviders.values() )
    provider->setEngine( mEngine );

  // populate sub-providers
  mRules.rootRule()->prepare( context, attributeNames, mSubProviders );
  return true;
}

void QgsRuleBasedLabelProvider::registerFeature( QgsFeature& feature, const QgsRenderContext& context )
{
  // will register the feature to relevant sub-providers
  mRules.rootRule()->registerFeature( feature, context, mSubProviders );
}

QList<QgsAbstractLabelProvider*> QgsRuleBasedLabelProvider::subProviders()
{
  QList<QgsAbstractLabelProvider*> lst;
  Q_FOREACH ( QgsVectorLayerLabelProvider* subprovider, mSubProviders.values() )
    lst << subprovider;
  return lst;
}


////////////////////

QgsRuleBasedLabeling::Rule::Rule( QgsPalLayerSettings* settings, int scaleMinDenom, int scaleMaxDenom, const QString& filterExp, const QString& label, const QString& description, bool elseRule )
    : mParent( 0 ), mSettings( settings )
    , mScaleMinDenom( scaleMinDenom ), mScaleMaxDenom( scaleMaxDenom )
    , mFilterExp( filterExp ), mLabel( label ), mDescription( description )
    , mElseRule( elseRule )
    //, mIsActive( true )
    , mFilter( 0 )
{
  initFilter();
}

QgsRuleBasedLabeling::Rule::~Rule()
{
  delete mSettings;
  delete mFilter;
  qDeleteAll( mChildren );
  // do NOT delete parent
}

void QgsRuleBasedLabeling::Rule::setSettings( QgsPalLayerSettings* settings )
{
  if ( mSettings == settings )
    return;

  delete mSettings;
  mSettings = settings;
}

void QgsRuleBasedLabeling::Rule::initFilter()
{
  if ( mElseRule || mFilterExp.compare( "ELSE", Qt::CaseInsensitive ) == 0 )
  {
    mElseRule = true;
    mFilter = 0;
  }
  else if ( !mFilterExp.isEmpty() )
  {
    delete mFilter;
    mFilter = new QgsExpression( mFilterExp );
  }
  else
  {
    mFilter = 0;
  }
}


void QgsRuleBasedLabeling::Rule::appendChild( QgsRuleBasedLabeling::Rule* rule )
{
  mChildren.append( rule );
  rule->mParent = this;
  // TODO updateElseRules();
}

void QgsRuleBasedLabeling::Rule::insertChild( int i, QgsRuleBasedLabeling::Rule* rule )
{
  mChildren.insert( i, rule );
  rule->mParent = this;
  // TODO updateElseRules();
}

void QgsRuleBasedLabeling::Rule::removeChildAt( int i )
{
  Rule* rule = mChildren[i];
  mChildren.removeAt( i );
  delete rule;
  // TODO updateElseRules();
}

QgsRuleBasedLabeling::Rule*QgsRuleBasedLabeling::Rule::clone() const
{
  QgsPalLayerSettings* s = mSettings ? new QgsPalLayerSettings( *mSettings ) : 0;
  Rule* newrule = new Rule( s, mScaleMinDenom, mScaleMaxDenom, mFilterExp, mLabel, mDescription );
  // TODO newrule->setCheckState( mIsActive );
  // clone children
  Q_FOREACH ( Rule* rule, mChildren )
    newrule->appendChild( rule->clone() );
  return newrule;
}

QgsRuleBasedLabeling::Rule*QgsRuleBasedLabeling::Rule::create( const QDomElement& ruleElem )
{
  QgsPalLayerSettings* settings = 0;
  QDomElement settingsElem = ruleElem.firstChildElement( "settings" );
  if ( !settingsElem.isNull() )
  {
    settings = new QgsPalLayerSettings;
    settings->readXml( settingsElem );
  }

  QString filterExp = ruleElem.attribute( "filter" );
  QString label = ruleElem.attribute( "label" );
  QString description = ruleElem.attribute( "description" );
  int scaleMinDenom = ruleElem.attribute( "scalemindenom", "0" ).toInt();
  int scaleMaxDenom = ruleElem.attribute( "scalemaxdenom", "0" ).toInt();
  //QString ruleKey = ruleElem.attribute( "key" );
  Rule* rule = new Rule( settings, scaleMinDenom, scaleMaxDenom, filterExp, label, description );

  //if ( !ruleKey.isEmpty() )
  //  rule->mRuleKey = ruleKey;

  //rule->setCheckState( ruleElem.attribute( "checkstate", "1" ).toInt() );

  QDomElement childRuleElem = ruleElem.firstChildElement( "rule" );
  while ( !childRuleElem.isNull() )
  {
    Rule* childRule = create( childRuleElem );
    if ( childRule )
    {
      rule->appendChild( childRule );
    }
    else
    {
      //QgsDebugMsg( "failed to init a child rule!" );
    }
    childRuleElem = childRuleElem.nextSiblingElement( "rule" );
  }

  return rule;
}

QDomElement QgsRuleBasedLabeling::Rule::save( QDomDocument& doc ) const
{
  QDomElement ruleElem = doc.createElement( "rule" );

  if ( mSettings )
  {
    ruleElem.appendChild( mSettings->writeXml( doc ) );
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
  //if ( !mIsActive )
  //  ruleElem.setAttribute( "checkstate", 0 );
  //ruleElem.setAttribute( "key", mRuleKey );

  for ( RuleList::const_iterator it = mChildren.constBegin(); it != mChildren.constEnd(); ++it )
  {
    Rule* rule = *it;
    ruleElem.appendChild( rule->save( doc ) );
  }
  return ruleElem;
}

void QgsRuleBasedLabeling::Rule::createSubProviders( QgsVectorLayer* layer, QgsRuleBasedLabeling::RuleToProviderMap& subProviders )
{
  if ( mSettings )
  {
    // add provider!
    QgsVectorLayerLabelProvider* p = new QgsVectorLayerLabelProvider( layer, false, mSettings );
    subProviders[this] = p;
  }

  // call recursively
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->createSubProviders( layer, subProviders );
  }
}

void QgsRuleBasedLabeling::Rule::prepare( const QgsRenderContext& context, QStringList& attributeNames, QgsRuleBasedLabeling::RuleToProviderMap& subProviders )
{
  if ( mSettings )
  {
    QgsVectorLayerLabelProvider* p = subProviders[this];
    if ( !p->prepare( context, attributeNames ) )
    {
      subProviders.remove( this );
      delete p;
    }
  }

  if ( mFilter )
    mFilter->prepare( &context.expressionContext() );

  // call recursively
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->prepare( context, attributeNames, subProviders );
  }
}

void QgsRuleBasedLabeling::Rule::registerFeature( QgsFeature& feature, const QgsRenderContext& context, QgsRuleBasedLabeling::RuleToProviderMap& subProviders )
{
  bool isOK = isFilterOK( feature, const_cast<QgsRenderContext&>( context ) );  // TODO: remove const_cast

  if ( !isOK )
    return;

  // do we have active subprovider for the rule?
  if ( subProviders.contains( this ) )
    subProviders[this]->registerFeature( feature, context );

  // call recursively
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->registerFeature( feature, context, subProviders );
  }
}

bool QgsRuleBasedLabeling::Rule::isFilterOK( QgsFeature& f, QgsRenderContext& context ) const
{
  if ( ! mFilter || mElseRule )
    return true;

  context.expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context.expressionContext() );
  return res.toInt() != 0;
}

////////////////////

QgsRuleBasedLabeling::QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule* root )
    : mRootRule( root )
{

}

QgsRuleBasedLabeling::QgsRuleBasedLabeling( const QgsRuleBasedLabeling& other )
{
  mRootRule = other.mRootRule->clone();
}

QgsRuleBasedLabeling::~QgsRuleBasedLabeling()
{
  delete mRootRule;
}

QgsRuleBasedLabeling*QgsRuleBasedLabeling::create( const QDomElement& element )
{
  QDomElement rulesElem = element.firstChildElement( "rules" );

  Rule* root = Rule::create( rulesElem );
  if ( !root )
    return 0;

  QgsRuleBasedLabeling* rl = new QgsRuleBasedLabeling( root );
  return rl;
}

QString QgsRuleBasedLabeling::type() const
{
  return "rule-based";
}

QDomElement QgsRuleBasedLabeling::save( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "labeling" );
  elem.setAttribute( "type", "rule-based" );

  QDomElement rulesElem = mRootRule->save( doc );
  rulesElem.setTagName( "rules" ); // instead of just "rule"
  elem.appendChild( rulesElem );

  return elem;
}

QgsVectorLayerLabelProvider* QgsRuleBasedLabeling::provider( QgsVectorLayer* layer ) const
{
  return new QgsRuleBasedLabelProvider( *this, layer, false );
}
