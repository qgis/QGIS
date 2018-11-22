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
#include "qgsxmlutils.h"

//
// QgsAbstractPropertyCollection
//

QgsAbstractPropertyCollection::QgsAbstractPropertyCollection( const QString &name )
  : mName( name )
{

}

QString QgsAbstractPropertyCollection::valueAsString( int key, const QgsExpressionContext &context, const QString &defaultString, bool *ok ) const
{
  if ( ok )
    *ok = false;

  QgsProperty prop = property( key );
  if ( !prop || !prop.isActive() )
    return defaultString;

  return prop.valueAsString( context, defaultString, ok );
}

QColor QgsAbstractPropertyCollection::valueAsColor( int key, const QgsExpressionContext &context, const QColor &defaultColor, bool *ok ) const
{
  if ( ok )
    *ok = false;

  QgsProperty prop = property( key );
  if ( !prop || !prop.isActive() )
    return defaultColor;

  return prop.valueAsColor( context, defaultColor, ok );
}

double QgsAbstractPropertyCollection::valueAsDouble( int key, const QgsExpressionContext &context, double defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;
  QgsProperty prop = property( key );
  if ( !prop || !prop.isActive() )
    return defaultValue;

  return prop.valueAsDouble( context, defaultValue, ok );
}

int QgsAbstractPropertyCollection::valueAsInt( int key, const QgsExpressionContext &context, int defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;
  QgsProperty prop = property( key );
  if ( !prop || !prop.isActive() )
    return defaultValue;

  return prop.valueAsInt( context, defaultValue, ok );
}

bool QgsAbstractPropertyCollection::valueAsBool( int key, const QgsExpressionContext &context, bool defaultValue, bool *ok ) const
{
  if ( ok )
    *ok = false;
  QgsProperty prop = property( key );
  if ( !prop || !prop.isActive() )
    return defaultValue;

  return prop.valueAsBool( context, defaultValue, ok );
}

bool QgsAbstractPropertyCollection::writeXml( QDomElement &collectionElem, const QgsPropertiesDefinition &definitions ) const
{
  QVariant collection = toVariant( definitions );
  QDomDocument doc = collectionElem.ownerDocument();
  QDomElement element = QgsXmlUtils::writeVariant( collection, doc );
  collectionElem.appendChild( element );
  return true;
}

bool QgsAbstractPropertyCollection::readXml( const QDomElement &collectionElem, const QgsPropertiesDefinition &definitions )
{
  QVariant collection = QgsXmlUtils::readVariant( collectionElem.firstChild().toElement() );
  return loadVariant( collection.toMap(), definitions );
}



//
// QgsPropertyCollection
//

QgsPropertyCollection::QgsPropertyCollection( const QString &name )
  : QgsAbstractPropertyCollection( name )
{}

QgsPropertyCollection::QgsPropertyCollection( const QgsPropertyCollection &other )
  : QgsAbstractPropertyCollection( other )
  , mProperties( other.mProperties )
  , mDirty( other.mDirty )
  , mHasActiveProperties( other.mHasActiveProperties )
  , mHasDynamicProperties( other.mHasDynamicProperties )
  , mCount( other.mCount )
{
  mProperties.detach();
}

QgsPropertyCollection &QgsPropertyCollection::operator=( const QgsPropertyCollection &other )
{
  QgsAbstractPropertyCollection::operator=( other );
  mProperties = other.mProperties;
  mProperties.detach();
  mDirty = other.mDirty;
  mHasActiveProperties = other.mHasActiveProperties;
  mHasDynamicProperties = other.mHasDynamicProperties;
  mCount = other.mCount;
  return *this;
}

int QgsPropertyCollection::count() const
{
  if ( !mDirty )
    return mCount;

  rescan();
  return mCount;
}

QSet<int> QgsPropertyCollection::propertyKeys() const
{
  QSet<int> keys;
  QHash<int, QgsProperty>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( it.value() )
      keys.insert( it.key() );
  }
  return keys;
}

void QgsPropertyCollection::clear()
{
  mProperties.clear();
  mDirty = false;
  mHasActiveProperties = false;
  mHasDynamicProperties = false;
  mCount = 0;
}

void QgsPropertyCollection::setProperty( int key, const QgsProperty &property )
{
  if ( property )
    mProperties.insert( key, property );
  else
    mProperties.remove( key );

  mDirty = true;
}

void QgsPropertyCollection::setProperty( int key, const QVariant &value )
{
  mProperties.insert( key, QgsProperty::fromValue( value ) );
  mDirty = true;
}

bool QgsPropertyCollection::hasProperty( int key ) const
{
  if ( mProperties.isEmpty() )
    return false;

  return mProperties.contains( key ) && mProperties.value( key );
}

QgsProperty QgsPropertyCollection::property( int key ) const
{
  if ( mProperties.isEmpty() )
    return QgsProperty();

  return mProperties.value( key );
}

QgsProperty &QgsPropertyCollection::property( int key )
{
  mDirty = true;
  return mProperties[ key ];
}

QVariant QgsPropertyCollection::value( int key, const QgsExpressionContext &context, const QVariant &defaultValue ) const
{
  if ( mProperties.isEmpty() )
    return defaultValue;

  QgsProperty prop = mProperties.value( key );
  if ( !prop || !prop.isActive() )
    return defaultValue;

  return prop.value( context, defaultValue );
}

bool QgsPropertyCollection::prepare( const QgsExpressionContext &context ) const
{
  bool result = true;
  QHash<int, QgsProperty>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( !it.value().isActive() )
      continue;

    result = result && it.value().prepare( context );
  }
  return result;
}

QSet< QString > QgsPropertyCollection::referencedFields( const QgsExpressionContext &context ) const
{
  QSet< QString > cols;
  QHash<int, QgsProperty>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( !it.value().isActive() )
      continue;

    cols.unite( it.value().referencedFields( context ) );
  }
  return cols;
}

bool QgsPropertyCollection::isActive( int key ) const
{
  if ( mProperties.isEmpty() )
    return false;

  return mProperties.value( key ).isActive();
}

void QgsPropertyCollection::rescan() const
{
  mHasActiveProperties = false;
  mHasDynamicProperties = false;
  mCount = 0;
  if ( !mProperties.isEmpty() )
  {
    QHash<int, QgsProperty>::const_iterator it = mProperties.constBegin();
    for ( ; it != mProperties.constEnd(); ++it )
    {
      if ( it.value() )
        mCount++;
      if ( it.value().isActive() )
      {
        mHasActiveProperties = true;
        if ( it.value().propertyType() != QgsProperty::StaticProperty )
        {
          mHasDynamicProperties = true;
        }
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

bool QgsPropertyCollection::hasDynamicProperties() const
{
  if ( mDirty )
    rescan();

  return mHasDynamicProperties;
}

QVariant QgsPropertyCollection::toVariant( const QgsPropertiesDefinition &definitions ) const
{
  QVariantMap collection;

  collection.insert( QStringLiteral( "name" ), name() );
  collection.insert( QStringLiteral( "type" ), QStringLiteral( "collection" ) );

  QVariantMap properties;

  QHash<int, QgsProperty>::const_iterator it = mProperties.constBegin();
  for ( ; it != mProperties.constEnd(); ++it )
  {
    if ( it.value() )
    {
      properties.insert( definitions.value( it.key() ).name(), it.value().toVariant() );
    }
  }
  collection.insert( QStringLiteral( "properties" ), properties );
  return collection;
}

bool QgsPropertyCollection::loadVariant( const QVariant &collection, const QgsPropertiesDefinition &definitions )
{
  clear();

  QVariantMap collectionMap = collection.toMap();

  setName( collectionMap.value( QStringLiteral( "name" ) ).toString() );

  mCount = 0;
  QVariantMap properties = collectionMap.value( QStringLiteral( "properties" ) ).toMap();
  for ( auto propertyIterator = properties.constBegin(); propertyIterator != properties.constEnd(); ++propertyIterator )
  {
    // match name to int key
    int key = -1;
    QgsPropertiesDefinition::const_iterator it = definitions.constBegin();
    for ( ; it != definitions.constEnd(); ++it )
    {
      if ( it->name() == propertyIterator.key() )
      {
        key = it.key();
        break;
      }
    }

    if ( key < 0 )
      continue;

    QgsProperty prop;
    prop.loadVariant( propertyIterator.value() );
    mProperties.insert( key, prop );

    mCount++;

    mHasActiveProperties = mHasActiveProperties || prop.isActive();
    mHasDynamicProperties = mHasDynamicProperties ||
                            ( prop.isActive() &&
                              ( prop.propertyType() == QgsProperty::FieldBasedProperty ||
                                prop.propertyType() == QgsProperty::ExpressionBasedProperty ) );
  }
  return true;
}

//
// QgsPropertyCollectionStack
//

QgsPropertyCollectionStack::~QgsPropertyCollectionStack()
{
  clear();
}

QgsPropertyCollectionStack::QgsPropertyCollectionStack( const QgsPropertyCollectionStack &other )
  : QgsAbstractPropertyCollection( other )
{
  clear();

  Q_FOREACH ( QgsPropertyCollection *collection, other.mStack )
  {
    mStack << new QgsPropertyCollection( *collection );
  }
}

QgsPropertyCollectionStack &QgsPropertyCollectionStack::operator=( const QgsPropertyCollectionStack &other )
{
  setName( other.name() );
  clear();

  Q_FOREACH ( QgsPropertyCollection *collection, other.mStack )
  {
    mStack << new QgsPropertyCollection( *collection );
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
}

void QgsPropertyCollectionStack::appendCollection( QgsPropertyCollection *collection )
{
  mStack.append( collection );
}

QgsPropertyCollection *QgsPropertyCollectionStack::at( int index )
{
  return mStack.value( index );
}

const QgsPropertyCollection *QgsPropertyCollectionStack::at( int index ) const
{
  return mStack.value( index );
}

QgsPropertyCollection *QgsPropertyCollectionStack::collection( const QString &name )
{
  Q_FOREACH ( QgsPropertyCollection *collection, mStack )
  {
    if ( collection->name() == name )
      return collection;
  }
  return nullptr;
}

bool QgsPropertyCollectionStack::hasActiveProperties() const
{
  Q_FOREACH ( const QgsPropertyCollection *collection, mStack )
  {
    if ( collection->hasActiveProperties() )
      return true;
  }
  return false;
}

bool QgsPropertyCollectionStack::hasDynamicProperties() const
{
  Q_FOREACH ( const QgsPropertyCollection *collection, mStack )
  {
    if ( collection->hasDynamicProperties() )
      return true;
  }
  return false;
}

bool QgsPropertyCollectionStack::isActive( int key ) const
{
  return static_cast< bool >( property( key ) );
}

QgsProperty QgsPropertyCollectionStack::property( int key ) const
{
  //loop through stack looking for last active matching property
  for ( int i = mStack.size() - 1; i >= 0; --i )
  {
    const QgsPropertyCollection *collection = mStack.at( i );
    QgsProperty property = collection->property( key );
    if ( property && property.isActive() )
    {
      return property;
    }
  }
  //not found
  return QgsProperty();
}


QVariant QgsPropertyCollectionStack::value( int key, const QgsExpressionContext &context, const QVariant &defaultValue ) const
{
  QgsProperty p = property( key );
  if ( !p )
  {
    return defaultValue;
  }
  return p.value( context, defaultValue );
}

QSet< QString > QgsPropertyCollectionStack::referencedFields( const QgsExpressionContext &context ) const
{
  QSet< QString > cols;
  Q_FOREACH ( QgsPropertyCollection *collection, mStack )
  {
    cols.unite( collection->referencedFields( context ) );
  }
  return cols;
}

bool QgsPropertyCollectionStack::prepare( const QgsExpressionContext &context ) const
{
  bool result = true;
  Q_FOREACH ( QgsPropertyCollection *collection, mStack )
  {
    result = result && collection->prepare( context );
  }
  return result;
}

QSet<int> QgsPropertyCollectionStack::propertyKeys() const
{
  QSet<int> keys;
  Q_FOREACH ( QgsPropertyCollection *collection, mStack )
  {
    keys.unite( collection->propertyKeys() );
  }
  return keys;
}

bool QgsPropertyCollectionStack::hasProperty( int key ) const
{
  Q_FOREACH ( QgsPropertyCollection *collection, mStack )
  {
    if ( collection->hasProperty( key ) )
      return true;
  }
  return false;
}

QVariant QgsPropertyCollectionStack::toVariant( const QgsPropertiesDefinition &definitions ) const
{
  QVariantMap collection;
  collection.insert( QStringLiteral( "type" ), QStringLiteral( "stack" ) );
  collection.insert( QStringLiteral( "name" ), name() );

  QVariantList properties;

  Q_FOREACH ( QgsPropertyCollection *child, mStack )
  {
    properties.append( child->toVariant( definitions ) );
  }

  collection.insert( QStringLiteral( "properties" ), properties );

  return collection;
}

bool QgsPropertyCollectionStack::loadVariant( const QVariant &collection, const QgsPropertiesDefinition &definitions )
{
  clear();

  QVariantMap collectionMap = collection.toMap();

  setName( collectionMap.value( QStringLiteral( "name" ) ).toString() );

  QVariantList properties = collectionMap.value( QStringLiteral( "properties" ) ).toList();

  Q_FOREACH ( const QVariant &property, properties )
  {
    QgsPropertyCollection *propertyCollection = new QgsPropertyCollection();
    propertyCollection->loadVariant( property.toMap(), definitions );
    mStack.append( propertyCollection );
  }

  return true;
}
