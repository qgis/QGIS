/***************************************************************************
     qgspropertycollection.cpp
     -------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspropertycollection.h"
#include "qgsproperty.h"

//
// QgsAbstractPropertyCollection
//

QgsAbstractPropertyCollection::QgsAbstractPropertyCollection( const QString& name )
    : mName( name )
{

}

QColor QgsAbstractPropertyCollection::valueAsColor( int key, const QgsExpressionContext &context, const QColor &defaultColor ) const
{
  const QgsAbstractProperty* prop = property( key );
  if ( !prop || !prop->isActive() )
    return defaultColor;

  return prop->valueAsColor( context, defaultColor );
}

double QgsAbstractPropertyCollection::valueAsDouble( int key, const QgsExpressionContext &context, double defaultValue ) const
{
  const QgsAbstractProperty* prop = property( key );
  if ( !prop || !prop->isActive() )
    return defaultValue;

  return prop->valueAsDouble( context, defaultValue );
}

int QgsAbstractPropertyCollection::valueAsInt( int key, const QgsExpressionContext &context, int defaultValue ) const
{
  const QgsAbstractProperty* prop = property( key );
  if ( !prop || !prop->isActive() )
    return defaultValue;

  return prop->valueAsInt( context, defaultValue );
}

bool QgsAbstractPropertyCollection::valueAsBool( int key, const QgsExpressionContext& context, bool defaultValue ) const
{
  const QgsAbstractProperty* prop = property( key );
  if ( !prop || !prop->isActive() )
    return defaultValue;

  return prop->valueAsBool( context, defaultValue );
}



//
// QgsPropertyCollection
//

QgsPropertyCollection::QgsPropertyCollection( const QString& name )
    : QgsAbstractPropertyCollection( name )
    , mDirty( false )
    , mHasActiveProperties( false )
    , mHasActiveDynamicProperties( false )
{

}

QgsPropertyCollection::~QgsPropertyCollection()
{
  clear();
}

QgsPropertyCollection::QgsPropertyCollection( const QgsPropertyCollection &other )
    : QgsAbstractPropertyCollection( other.name() )
    , mDirty( false )
    , mHasActiveProperties( false )
    , mHasActiveDynamicProperties( false )
{
  QHash< int, QgsAbstractProperty* >::const_iterator it = other.mProperties.constBegin();
  for ( ; it != other.mProperties.constEnd(); ++it )
  {
    mProperties.insert( it.key(), it.value()->clone() );
    if ( it.value()->isActive() )
    {
      mHasActiveProperties = true;
      if ( it.value()->propertyType() != QgsAbstractProperty::StaticProperty )
        mHasActiveDynamicProperties = true;
    }
  }
}

QgsPropertyCollection &QgsPropertyCollection::operator=( const QgsPropertyCollection & other )
{
  setName( other.name() );
  clear();
  QHash< int, QgsAbstractProperty* >::const_iterator it = other.mProperties.constBegin();
  for ( ; it != other.mProperties.constEnd(); ++it )
  {
    mProperties.insert( it.key(), it.value()->clone() );
    if ( it.value()->isActive() )
    {
      mHasActiveProperties = true;
      if ( it.value()->propertyType() != QgsAbstractProperty::StaticProperty )
        mHasActiveDynamicProperties = true;
    }
  }
  return *this;
}

int QgsPropertyCollection::count() const
{
  return mProperties.size();
}

QSet<int> QgsPropertyCollection::propertyKeys() const
{
  return mProperties.keys().toSet();
}

void QgsPropertyCollection::clear()
{
  qDeleteAll( mProperties );
  mProperties.clear();
  mDirty = false;
  mHasActiveProperties = false;
  mHasActiveDynamicProperties = false;
}

void QgsPropertyCollection::setProperty( int key, QgsAbstractProperty* property )
{
  if ( hasProperty( key ) )
    delete mProperties.take( key );

  if ( property )
    mProperties.insert( key, property );

  mDirty = true;
}

void QgsPropertyCollection::setProperty( int key, const QVariant& value )
{
  QgsStaticProperty* property = new QgsStaticProperty( value );
  setProperty( key, property );
}

bool QgsPropertyCollection::hasProperty( int key ) const
{
  if ( mProperties.isEmpty() )
    return false;

  return mProperties.contains( key );
}

QgsAbstractProperty* QgsPropertyCollection::property( int key )
{
  if ( mProperties.isEmpty() )
    return nullptr;

  mDirty = true;
  return mProperties.value( key, nullptr );
}

const QgsAbstractProperty *QgsPropertyCollection::property( int key ) const
{
  if ( mProperties.isEmpty() )
    return nullptr;

  return mProperties.value( key, nullptr );
}

QVariant QgsPropertyCollection::value( int key, const QgsExpressionContext& context, const QVariant& defaultValue ) const
{
  if ( mProperties.isEmpty() )
    return defaultValue;

  QgsAbstractProperty* prop = mProperties.value( key, nullptr );
  if ( !prop || !prop->isActive() )
    return defaultValue;

  return prop->value( context, defaultValue );
}

bool QgsPropertyCollection::prepare( const QgsExpressionContext& context ) const
{
  bool result = true;
  QHash<int, QgsAbstractProperty*>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( !it.value()->isActive() )
      continue;

    result = result && it.value()->prepare( context );
  }
  return result;
}

QSet< QString > QgsPropertyCollection::referencedFields( const QgsExpressionContext &context ) const
{
  QSet< QString > cols;
  QHash<int, QgsAbstractProperty*>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( !it.value()->isActive() )
      continue;

    cols.unite( it.value()->referencedFields( context ) );
  }
  return cols;
}

bool QgsPropertyCollection::isActive( int key ) const
{
  if ( mProperties.isEmpty() )
    return false;

  QgsAbstractProperty* prop = mProperties.value( key, nullptr );
  return prop && prop->isActive();
}

void QgsPropertyCollection::rescan() const
{
  mHasActiveProperties = false;
  mHasActiveDynamicProperties = false;
  QHash<int, QgsAbstractProperty*>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( it.value()->isActive() )
    {
      mHasActiveProperties = true;
      if ( it.value()->propertyType() != QgsAbstractProperty::StaticProperty )
      {
        mHasActiveDynamicProperties = true;
        break;
      }
    }
  }
  mDirty = false;
}

bool QgsPropertyCollection::hasActiveProperties() const
{
  if ( mDirty )
    rescan();

  return mHasActiveProperties;
}

bool QgsPropertyCollection::hasActiveDynamicProperties() const
{
  if ( mDirty )
    rescan();

  return mHasActiveDynamicProperties;
}

bool QgsPropertyCollection::writeXml( QDomElement &collectionElem, QDomDocument &doc, const QMap<int, QString> &propertyNameMap ) const
{
  collectionElem.setAttribute( "name", name() );
  collectionElem.setAttribute( "type", "collection" );
  QHash<int, QgsAbstractProperty*>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    QDomElement propertyElement = doc.createElement( "p" );
    int key = it.key();
    QString propName = propertyNameMap.value( key );
    propertyElement.setAttribute( "n", propName );
    propertyElement.setAttribute( "t", static_cast< int >( it.value()->propertyType() ) );
    it.value()->writeXml( propertyElement, doc );
    collectionElem.appendChild( propertyElement );
  }
  return true;
}

bool QgsPropertyCollection::readXml( const QDomElement &collectionElem, const QDomDocument &doc, const QMap<int, QString> &propertyNameMap )
{
  clear();

  setName( collectionElem.attribute( "name" ) );

  QDomNodeList propertyNodeList = collectionElem.elementsByTagName( "p" );
  for ( int i = 0; i < propertyNodeList.size(); ++i )
  {
    QDomElement propertyElem = propertyNodeList.at( i ).toElement();
    QString propName = propertyElem.attribute( "n" );
    if ( propName.isEmpty() )
      continue;

    // match name to int key
    int key = propertyNameMap.key( propName, -1 );
    if ( key < 0 )
      continue;

    QgsAbstractProperty::Type type = static_cast< QgsAbstractProperty::Type >( propertyElem.attribute( "t", "0" ).toInt() );
    QgsAbstractProperty* prop = QgsAbstractProperty::create( type );
    if ( !prop )
      continue;
    prop->readXml( propertyElem, doc );
    mProperties.insert( key, prop );
  }
  return true;
}

//
// QgsPropertyCollectionStack
//

QgsPropertyCollectionStack::QgsPropertyCollectionStack()
    : mDirty( false )
    , mHasActiveProperties( false )
    , mHasActiveDynamicProperties( false )
{

}

QgsPropertyCollectionStack::~QgsPropertyCollectionStack()
{
  clear();
}

QgsPropertyCollectionStack::QgsPropertyCollectionStack( const QgsPropertyCollectionStack &other )
    : QgsAbstractPropertyCollection( other )
    , mDirty( false )
    , mHasActiveProperties( false )
    , mHasActiveDynamicProperties( false )
{
  clear();

  Q_FOREACH ( QgsPropertyCollection* collection, other.mStack )
  {
    mStack << new QgsPropertyCollection( *collection );
    mHasActiveProperties |= collection->hasActiveProperties();
    mHasActiveDynamicProperties |= collection->hasActiveDynamicProperties();
  }
}

QgsPropertyCollectionStack &QgsPropertyCollectionStack::operator=( const QgsPropertyCollectionStack & other )
{
  setName( other.name() );
  clear();

  Q_FOREACH ( QgsPropertyCollection* collection, other.mStack )
  {
    mStack << new QgsPropertyCollection( *collection );
    mHasActiveProperties |= collection->hasActiveProperties();
    mHasActiveDynamicProperties |= collection->hasActiveDynamicProperties();
  }

  return *this;
}

int QgsPropertyCollectionStack::count() const
{
  return mStack.size();
}

void QgsPropertyCollectionStack::clear()
{
  qDeleteAll( mStack );
  mStack.clear();
  mHasActiveProperties = false;
  mHasActiveDynamicProperties = false;
  mDirty = false;
}

void QgsPropertyCollectionStack::appendCollection( QgsPropertyCollection* collection )
{
  mStack.append( collection );
  mDirty = true;
}

QgsPropertyCollection* QgsPropertyCollectionStack::at( int index )
{
  mDirty = true;
  return mStack.value( index );
}

const QgsPropertyCollection* QgsPropertyCollectionStack::at( int index ) const
{
  return mStack.value( index );
}

QgsPropertyCollection* QgsPropertyCollectionStack::collection( const QString &name )
{
  mDirty = true;
  Q_FOREACH ( QgsPropertyCollection* collection, mStack )
  {
    if ( collection->name() == name )
      return collection;
  }
  return nullptr;
}

bool QgsPropertyCollectionStack::hasActiveProperties() const
{
  if ( mDirty )
    rescan();

  return mHasActiveProperties;
}

bool QgsPropertyCollectionStack::hasActiveDynamicProperties() const
{
  if ( mDirty )
    rescan();

  return mHasActiveDynamicProperties;
}

bool QgsPropertyCollectionStack::isActive( int key ) const
{
  const QgsAbstractProperty* p = property( key );
  return static_cast< bool >( p );
}

const QgsAbstractProperty* QgsPropertyCollectionStack::property( int key ) const
{
  //loop through stack looking for last active matching property
  for ( int i = mStack.size() - 1; i >= 0; --i )
  {
    const QgsPropertyCollection* collection = mStack.at( i );
    const QgsAbstractProperty* property = collection->property( key );
    if ( property && property->isActive() )
    {
      return property;
    }
  }
  //not found
  return nullptr;
}

QgsAbstractProperty*QgsPropertyCollectionStack::property( int key )
{
  //loop through stack looking for last active matching property
  for ( int i = mStack.size() - 1; i >= 0; --i )
  {
    QgsPropertyCollection* collection = mStack.at( i );
    QgsAbstractProperty* property = collection->property( key );
    if ( property && property->isActive() )
    {
      mDirty = true;
      return property;
    }
  }
  //not found
  return nullptr;
}

QVariant QgsPropertyCollectionStack::value( int key, const QgsExpressionContext& context, const QVariant& defaultValue ) const
{
  const QgsAbstractProperty* p = property( key );
  if ( !p )
  {
    return defaultValue;
  }
  return p->value( context, defaultValue );
}

QSet< QString > QgsPropertyCollectionStack::referencedFields( const QgsExpressionContext &context ) const
{
  QSet< QString > cols;
  Q_FOREACH ( QgsPropertyCollection* collection, mStack )
  {
    cols.unite( collection->referencedFields( context ) );
  }
  return cols;
}

bool QgsPropertyCollectionStack::prepare( const QgsExpressionContext& context ) const
{
  bool result = true;
  Q_FOREACH ( QgsPropertyCollection* collection, mStack )
  {
    result = result && collection->prepare( context );
  }
  return result;
}

QSet<int> QgsPropertyCollectionStack::propertyKeys() const
{
  QSet<int> keys;
  Q_FOREACH ( QgsPropertyCollection* collection, mStack )
  {
    keys.unite( collection->propertyKeys() );
  }
  return keys;
}

bool QgsPropertyCollectionStack::hasProperty( int key ) const
{
  Q_FOREACH ( QgsPropertyCollection* collection, mStack )
  {
    if ( collection->hasProperty( key ) )
      return true;
  }
  return false;
}

bool QgsPropertyCollectionStack::writeXml( QDomElement& collectionElem, QDomDocument& doc, const QMap<int, QString>& propertyNameMap ) const
{
  collectionElem.setAttribute( "type", "stack" );
  collectionElem.setAttribute( "name", name() );

  Q_FOREACH ( QgsPropertyCollection* child, mStack )
  {
    QDomElement childElement = doc.createElement( "props" );
    if ( !child->writeXml( childElement, doc, propertyNameMap ) )
      return false;
    collectionElem.appendChild( childElement );
  }
  return true;
}

bool QgsPropertyCollectionStack::readXml( const QDomElement& collectionElem, const QDomDocument& doc, const QMap<int, QString>& propertyNameMap )
{
  clear();

  setName( collectionElem.attribute( "name" ) );

  QDomNodeList childNodeList = collectionElem.elementsByTagName( "props" );
  for ( int i = 0; i < childNodeList.size(); ++i )
  {
    QDomElement childElem = childNodeList.at( i ).toElement();
    QgsPropertyCollection* child = new QgsPropertyCollection();
    child->readXml( childElem, doc, propertyNameMap );
    mStack.append( child );
  }
  mDirty = true;
  return true;
}

void QgsPropertyCollectionStack::rescan() const
{
  mHasActiveProperties = false;
  mHasActiveDynamicProperties = false;
  Q_FOREACH ( const QgsPropertyCollection* collection, mStack )
  {
    mHasActiveProperties |= collection->hasActiveProperties();
    mHasActiveDynamicProperties |= collection->hasActiveDynamicProperties();
    if ( mHasActiveProperties && mHasActiveDynamicProperties )
      break;
  }
  mDirty = false;
}
