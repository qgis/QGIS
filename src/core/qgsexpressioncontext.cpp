/***************************************************************************
     qgsexpressioncontext.cpp
     ------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressioncontext.h"
#include "qgsxmlutils.h"
#include "qgsexpression.h"
#include "qgsmaplayerstore.h"
#include "qgsexpressioncontextutils.h"

const QString QgsExpressionContext::EXPR_FIELDS( QStringLiteral( "_fields_" ) );
const QString QgsExpressionContext::EXPR_ORIGINAL_VALUE( QStringLiteral( "value" ) );
const QString QgsExpressionContext::EXPR_SYMBOL_COLOR( QStringLiteral( "symbol_color" ) );
const QString QgsExpressionContext::EXPR_SYMBOL_ANGLE( QStringLiteral( "symbol_angle" ) );
const QString QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT( QStringLiteral( "geometry_part_count" ) );
const QString QgsExpressionContext::EXPR_GEOMETRY_PART_NUM( QStringLiteral( "geometry_part_num" ) );
const QString QgsExpressionContext::EXPR_GEOMETRY_RING_NUM( QStringLiteral( "geometry_ring_num" ) );
const QString QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT( QStringLiteral( "geometry_point_count" ) );
const QString QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM( QStringLiteral( "geometry_point_num" ) );
const QString QgsExpressionContext::EXPR_CLUSTER_SIZE( QStringLiteral( "cluster_size" ) );
const QString QgsExpressionContext::EXPR_CLUSTER_COLOR( QStringLiteral( "cluster_color" ) );

//
// QgsExpressionContextScope
//

QgsExpressionContextScope::QgsExpressionContextScope( const QString &name )
  : mName( name )
{

}

QgsExpressionContextScope::QgsExpressionContextScope( const QgsExpressionContextScope &other )
  : mName( other.mName )
  , mVariables( other.mVariables )
  , mHasFeature( other.mHasFeature )
  , mFeature( other.mFeature )
  , mHasGeometry( other.mHasGeometry )
  , mGeometry( other.mGeometry )
  , mHiddenVariables( other.mHiddenVariables )
  , mLayerStores( other.mLayerStores )
{
  QHash<QString, QgsScopedExpressionFunction * >::const_iterator it = other.mFunctions.constBegin();
  for ( ; it != other.mFunctions.constEnd(); ++it )
  {
    mFunctions.insert( it.key(), it.value()->clone() );
  }
}

QgsExpressionContextScope &QgsExpressionContextScope::operator=( const QgsExpressionContextScope &other )
{
  mName = other.mName;
  mVariables = other.mVariables;
  mHasFeature = other.mHasFeature;
  mFeature = other.mFeature;
  mHasGeometry = other.mHasGeometry;
  mGeometry = other.mGeometry;
  mHiddenVariables = other.mHiddenVariables;
  mLayerStores = other.mLayerStores;

  qDeleteAll( mFunctions );
  mFunctions.clear();
  QHash<QString, QgsScopedExpressionFunction * >::const_iterator it = other.mFunctions.constBegin();
  for ( ; it != other.mFunctions.constEnd(); ++it )
  {
    mFunctions.insert( it.key(), it.value()->clone() );
  }

  return *this;
}

QgsExpressionContextScope::~QgsExpressionContextScope()
{
  qDeleteAll( mFunctions );
}

void QgsExpressionContextScope::setVariable( const QString &name, const QVariant &value, bool isStatic )
{
  auto it = mVariables.find( name );
  if ( it != mVariables.end() )
  {
    it->value = value;
    it->isStatic = isStatic;
  }
  else
  {
    addVariable( QgsExpressionContextScope::StaticVariable( name, value, false, isStatic ) );
  }
}

void QgsExpressionContextScope::addVariable( const QgsExpressionContextScope::StaticVariable &variable )
{
  mVariables.insert( variable.name, variable );
}

bool QgsExpressionContextScope::removeVariable( const QString &name )
{
  return mVariables.remove( name ) > 0;
}

bool QgsExpressionContextScope::hasVariable( const QString &name ) const
{
  return mVariables.contains( name );
}

QVariant QgsExpressionContextScope::variable( const QString &name ) const
{
  return hasVariable( name ) ? mVariables.value( name ).value : QVariant();
}

QStringList QgsExpressionContextScope::variableNames() const
{
  QStringList names = mVariables.keys();

  if ( hasFeature() )
  {
    names.append( QStringLiteral( "feature" ) );
    names.append( QStringLiteral( "id" ) );
    names.append( QStringLiteral( "geometry" ) );
  }

  return names;
}

QStringList QgsExpressionContextScope::hiddenVariables() const
{
  return mHiddenVariables;
}

void QgsExpressionContextScope::setHiddenVariables( const QStringList &hiddenVariables )
{
  mHiddenVariables = hiddenVariables;
}

void QgsExpressionContextScope::addHiddenVariable( const QString &hiddenVariable )
{
  if ( !mHiddenVariables.contains( hiddenVariable ) )
    mHiddenVariables << hiddenVariable;
}

void QgsExpressionContextScope::removeHiddenVariable( const QString &hiddenVariable )
{
  if ( mHiddenVariables.contains( hiddenVariable ) )
    mHiddenVariables.removeAt( mHiddenVariables.indexOf( hiddenVariable ) );
}

void QgsExpressionContextScope::addLayerStore( QgsMapLayerStore *store )
{
  mLayerStores.append( store );
}

QList<QgsMapLayerStore *> QgsExpressionContextScope::layerStores() const
{
  QList<QgsMapLayerStore *> res;
  res.reserve( mLayerStores.size() );
  for ( QgsMapLayerStore *store : std::as_const( mLayerStores ) )
  {
    if ( store )
      res << store;
  }
  return res;
}

/// @cond PRIVATE
class QgsExpressionContextVariableCompare
{
  public:
    explicit QgsExpressionContextVariableCompare( const QgsExpressionContextScope &scope )
      : mScope( scope )
    {  }

    bool operator()( const QString &a, const QString &b ) const
    {
      bool aReadOnly = mScope.isReadOnly( a );
      bool bReadOnly = mScope.isReadOnly( b );
      if ( aReadOnly != bReadOnly )
        return aReadOnly;
      return QString::localeAwareCompare( a, b ) < 0;
    }

  private:
    const QgsExpressionContextScope &mScope;
};
/// @endcond

QStringList QgsExpressionContextScope::filteredVariableNames() const
{
  QStringList allVariables = mVariables.keys();
  QStringList filtered;
  const auto constAllVariables = allVariables;
  for ( const QString &variable : constAllVariables )
  {
    if ( variable.startsWith( '_' ) )
      continue;

    filtered << variable;
  }
  QgsExpressionContextVariableCompare cmp( *this );
  std::sort( filtered.begin(), filtered.end(), cmp );

  return filtered;
}

bool QgsExpressionContextScope::isReadOnly( const QString &name ) const
{
  return hasVariable( name ) ? mVariables.value( name ).readOnly : false;
}

bool QgsExpressionContextScope::isStatic( const QString &name ) const
{
  return hasVariable( name ) ? mVariables.value( name ).isStatic : false;
}

QString QgsExpressionContextScope::description( const QString &name ) const
{
  return hasVariable( name ) ? mVariables.value( name ).description : QString();
}

bool QgsExpressionContextScope::hasFunction( const QString &name ) const
{
  return mFunctions.contains( name );
}

QgsExpressionFunction *QgsExpressionContextScope::function( const QString &name ) const
{
  return mFunctions.contains( name ) ? mFunctions.value( name ) : nullptr;
}

QStringList QgsExpressionContextScope::functionNames() const
{
  return mFunctions.keys();
}

void QgsExpressionContextScope::addFunction( const QString &name, QgsScopedExpressionFunction *function )
{
  mFunctions.insert( name, function );
}


void QgsExpressionContextScope::setFields( const QgsFields &fields )
{
  addVariable( StaticVariable( QgsExpressionContext::EXPR_FIELDS, QVariant::fromValue( fields ), true ) );
}

void QgsExpressionContextScope::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomNodeList variablesNodeList = element.childNodes();
  for ( int i = 0; i < variablesNodeList.size(); ++i )
  {
    const QDomElement variableElement = variablesNodeList.at( i ).toElement();
    const QString key = variableElement.attribute( QStringLiteral( "name" ) );
    if ( variableElement.tagName() == QLatin1String( "Variable" ) )
    {
      const QVariant value = QgsXmlUtils::readVariant( variableElement.firstChildElement( QStringLiteral( "Option" ) ) );
      setVariable( key, value );
    }
    else
      addHiddenVariable( key );

  }
}

bool QgsExpressionContextScope::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  for ( auto it = mVariables.constBegin(); it != mVariables.constEnd(); ++it )
  {
    QDomElement varElem = document.createElement( QStringLiteral( "Variable" ) );
    varElem.setAttribute( QStringLiteral( "name" ), it.key() );
    QDomElement valueElem = QgsXmlUtils::writeVariant( it.value().value, document );
    varElem.appendChild( valueElem );
    element.appendChild( varElem );
  }

  for ( QString hiddenVariable : mHiddenVariables )
  {
    QDomElement varElem = document.createElement( QStringLiteral( "HiddenVariable" ) );
    varElem.setAttribute( QStringLiteral( "name" ), hiddenVariable );
    element.appendChild( varElem );
  }
  return true;
}


//
// QgsExpressionContext
//

QgsExpressionContext::QgsExpressionContext()
{
  mLoadLayerFunction = std::make_unique< LoadLayerFunction >();
}

QgsExpressionContext::QgsExpressionContext( const QList<QgsExpressionContextScope *> &scopes )
  : mStack( scopes )
{
  mLoadLayerFunction = std::make_unique< LoadLayerFunction >();
}

QgsExpressionContext::QgsExpressionContext( const QgsExpressionContext &other ) : mStack{}
{
  for ( const QgsExpressionContextScope *scope : std::as_const( other.mStack ) )
  {
    mStack << new QgsExpressionContextScope( *scope );
  }
  mHighlightedVariables = other.mHighlightedVariables;
  mHighlightedFunctions = other.mHighlightedFunctions;
  mCachedValues = other.mCachedValues;
  mFeedback = other.mFeedback;
  mDestinationStore = other.mDestinationStore;
  mLoadLayerFunction = std::make_unique< LoadLayerFunction >();
}

QgsExpressionContext &QgsExpressionContext::operator=( QgsExpressionContext &&other ) noexcept
{
  if ( this != &other )
  {
    qDeleteAll( mStack );
    // move the stack over
    mStack = other.mStack;
    other.mStack.clear();

    mHighlightedVariables = other.mHighlightedVariables;
    mHighlightedFunctions = other.mHighlightedFunctions;
    mCachedValues = other.mCachedValues;
    mFeedback = other.mFeedback;
    mDestinationStore = other.mDestinationStore;
  }
  return *this;
}

// cppcheck-suppress operatorEqVarError
QgsExpressionContext &QgsExpressionContext::operator=( const QgsExpressionContext &other )
{
  if ( &other == this )
    return *this;

  qDeleteAll( mStack );
  mStack.clear();
  for ( const QgsExpressionContextScope *scope : std::as_const( other.mStack ) )
  {
    mStack << new QgsExpressionContextScope( *scope );
  }
  mHighlightedVariables = other.mHighlightedVariables;
  mHighlightedFunctions = other.mHighlightedFunctions;
  mCachedValues = other.mCachedValues;
  mFeedback = other.mFeedback;
  mDestinationStore = other.mDestinationStore;
  return *this;
}

QgsExpressionContext::~QgsExpressionContext()
{
  qDeleteAll( mStack );
  mStack.clear();
}

bool QgsExpressionContext::hasVariable( const QString &name ) const
{
  const auto constMStack = mStack;
  for ( const QgsExpressionContextScope *scope : constMStack )
  {
    if ( scope->hasVariable( name ) )
      return true;
  }
  return false;
}

QVariant QgsExpressionContext::variable( const QString &name ) const
{
  const QgsExpressionContextScope *scope = activeScopeForVariable( name );
  return scope ? scope->variable( name ) : QVariant();
}

QVariantMap QgsExpressionContext::variablesToMap() const
{
  QStringList names = variableNames();
  QVariantMap m;
  const auto constNames = names;
  for ( const QString &name : constNames )
  {
    m.insert( name, variable( name ) );
  }
  return m;
}

bool QgsExpressionContext::isHighlightedVariable( const QString &name ) const
{
  return mHighlightedVariables.contains( name );
}

QStringList QgsExpressionContext::highlightedVariables() const
{
  return mHighlightedVariables;
}

void QgsExpressionContext::setHighlightedVariables( const QStringList &variableNames )
{
  mHighlightedVariables = variableNames;
}

bool QgsExpressionContext::isHighlightedFunction( const QString &name ) const
{
  return mHighlightedFunctions.contains( name );
}

void QgsExpressionContext::setHighlightedFunctions( const QStringList &names )
{
  mHighlightedFunctions = names;
}

const QgsExpressionContextScope *QgsExpressionContext::activeScopeForVariable( const QString &name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope * >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if ( ( *it )->hasVariable( name ) )
      return ( *it );
  }
  return nullptr;
}

QgsExpressionContextScope *QgsExpressionContext::activeScopeForVariable( const QString &name )
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope * >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if ( ( *it )->hasVariable( name ) )
      return ( *it );
  }
  return nullptr;
}

QgsExpressionContextScope *QgsExpressionContext::scope( int index )
{
  if ( index < 0 || index >= mStack.count() )
    return nullptr;

  return mStack.at( index );
}

QgsExpressionContextScope *QgsExpressionContext::lastScope()
{
  if ( mStack.count() < 1 )
    return nullptr;

  return mStack.last();
}

int QgsExpressionContext::indexOfScope( QgsExpressionContextScope *scope ) const
{
  if ( !scope )
    return -1;

  return mStack.indexOf( scope );
}

int QgsExpressionContext::indexOfScope( const QString &scopeName ) const
{
  int index = 0;
  const auto constMStack = mStack;
  for ( const QgsExpressionContextScope *scope : constMStack )
  {
    if ( scope->name() == scopeName )
      return index;

    index++;
  }
  return -1;
}

QStringList QgsExpressionContext::variableNames() const
{
  QSet< QString> names;
  for ( const QgsExpressionContextScope *scope : mStack )
  {
    const QStringList variableNames = scope->variableNames();
    for ( const QString &name : variableNames )
      names.insert( name );
  }
  return QStringList( names.constBegin(), names.constEnd() );
}

QStringList QgsExpressionContext::filteredVariableNames() const
{
  QStringList allVariables = variableNames();
  QStringList filtered;
  const auto constAllVariables = allVariables;

  QStringList hiddenVariables;

  for ( const QgsExpressionContextScope *scope : mStack )
  {
    const QStringList scopeHiddenVariables = scope->hiddenVariables();
    for ( const QString &name : scopeHiddenVariables )
      hiddenVariables << name ;
  }

  for ( const QString &variable : constAllVariables )
  {
    if ( variable.startsWith( '_' ) ||
         hiddenVariables.contains( variable ) )
      continue;

    filtered << variable;
  }

  filtered.sort();
  return filtered;
}

bool QgsExpressionContext::isReadOnly( const QString &name ) const
{
  const auto constMStack = mStack;
  for ( const QgsExpressionContextScope *scope : constMStack )
  {
    if ( scope->isReadOnly( name ) )
      return true;
  }
  return false;
}

QString QgsExpressionContext::description( const QString &name ) const
{
  const QgsExpressionContextScope *scope = activeScopeForVariable( name );
  return ( scope && !scope->description( name ).isEmpty() ) ? scope->description( name ) : QgsExpression::variableHelpText( name );
}

bool QgsExpressionContext::hasFunction( const QString &name ) const
{
  if ( name.compare( QLatin1String( "load_layer" ) ) == 0 && mDestinationStore )
    return true;

  for ( const QgsExpressionContextScope *scope : mStack )
  {
    if ( scope->hasFunction( name ) )
      return true;
  }
  return false;
}

QStringList QgsExpressionContext::functionNames() const
{
  QSet< QString > result;
  for ( const QgsExpressionContextScope *scope : mStack )
  {
    const QStringList functionNames = scope->functionNames();
    for ( const QString &name : functionNames )
      result.insert( name );
  }

  if ( mDestinationStore )
    result.insert( QStringLiteral( "load_layer" ) );

  QStringList listResult( result.constBegin(), result.constEnd() );
  listResult.sort();
  return listResult;
}

QgsExpressionFunction *QgsExpressionContext::function( const QString &name ) const
{
  if ( name.compare( QLatin1String( "load_layer" ) ) == 0 && mDestinationStore )
  {
    return mLoadLayerFunction.get();
  }

  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope * >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if ( ( *it )->hasFunction( name ) )
      return ( *it )->function( name );
  }
  return nullptr;
}

int QgsExpressionContext::scopeCount() const
{
  return mStack.count();
}

void QgsExpressionContext::appendScope( QgsExpressionContextScope *scope )
{
  mStack.append( scope );
}

void QgsExpressionContext::appendScopes( const QList<QgsExpressionContextScope *> &scopes )
{
  mStack.append( scopes );
}

QgsExpressionContextScope *QgsExpressionContext::popScope()
{
  if ( !mStack.isEmpty() )
    return mStack.takeLast();

  return nullptr;
}

QList<QgsExpressionContextScope *> QgsExpressionContext::takeScopes()
{
  QList<QgsExpressionContextScope *> stack = mStack;
  mStack.clear();
  return stack;
}

QgsExpressionContext &QgsExpressionContext::operator<<( QgsExpressionContextScope *scope )
{
  mStack.append( scope );
  return *this;
}

void QgsExpressionContext::setFeature( const QgsFeature &feature )
{
  if ( mStack.isEmpty() )
    mStack.append( new QgsExpressionContextScope() );

  mStack.last()->setFeature( feature );
}

bool QgsExpressionContext::hasFeature() const
{
  for ( const QgsExpressionContextScope *scope : mStack )
  {
    if ( scope->hasFeature() )
      return true;
  }
  return false;
}

QgsFeature QgsExpressionContext::feature() const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope * >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if ( ( *it )->hasFeature() )
      return ( *it )->feature();
  }
  return QgsFeature();
}

void QgsExpressionContext::setGeometry( const QgsGeometry &geometry )
{
  if ( mStack.isEmpty() )
    mStack.append( new QgsExpressionContextScope() );

  mStack.last()->setGeometry( geometry );
}

bool QgsExpressionContext::hasGeometry() const
{
  for ( const QgsExpressionContextScope *scope : mStack )
  {
    if ( scope->hasGeometry() )
      return true;
  }
  return false;
}

QgsGeometry QgsExpressionContext::geometry() const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope * >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if ( ( *it )->hasGeometry() )
      return ( *it )->geometry();
  }
  return QgsGeometry();
}

void QgsExpressionContext::setFields( const QgsFields &fields )
{
  if ( mStack.isEmpty() )
    mStack.append( new QgsExpressionContextScope() );

  mStack.last()->setFields( fields );
}

QgsFields QgsExpressionContext::fields() const
{
  return qvariant_cast<QgsFields>( variable( QgsExpressionContext::EXPR_FIELDS ) );
}

void QgsExpressionContext::setOriginalValueVariable( const QVariant &value )
{
  if ( mStack.isEmpty() )
    mStack.append( new QgsExpressionContextScope() );

  mStack.last()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_ORIGINAL_VALUE,
                              value, true ) );
}

void QgsExpressionContext::setCachedValue( const QString &key, const QVariant &value ) const
{
  mCachedValues.insert( key, value );
}

bool QgsExpressionContext::hasCachedValue( const QString &key ) const
{
  return mCachedValues.contains( key );
}

QVariant QgsExpressionContext::cachedValue( const QString &key ) const
{
  return mCachedValues.value( key, QVariant() );
}

void QgsExpressionContext::clearCachedValues() const
{
  mCachedValues.clear();
}

QList<QgsMapLayerStore *> QgsExpressionContext::layerStores() const
{
  //iterate through stack backwards, so that higher priority layer stores take precedence
  QList< QgsExpressionContextScope * >::const_iterator it = mStack.constEnd();
  QList<QgsMapLayerStore *> res;
  while ( it != mStack.constBegin() )
  {
    --it;
    res.append( ( *it )->layerStores() );
  }
  // ensure that the destination store is also present in the list
  if ( mDestinationStore && !res.contains( mDestinationStore ) )
    res.append( mDestinationStore );
  return res;
}

void QgsExpressionContext::setLoadedLayerStore( QgsMapLayerStore *store )
{
  mDestinationStore = store;
}

QgsMapLayerStore *QgsExpressionContext::loadedLayerStore() const
{
  return mDestinationStore;
}

void QgsExpressionContext::setFeedback( QgsFeedback *feedback )
{
  mFeedback = feedback;
}

QgsFeedback *QgsExpressionContext::feedback() const
{
  return mFeedback;
}
