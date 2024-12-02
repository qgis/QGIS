/* Copyright 2014, Uwe L. Korn <uwelk@xhochy.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Json.h"

#include <QJsonDocument>
#include <QMetaProperty>
#include <QVariantHash>

namespace QJsonWrapper
{

  QVariantMap
  qobject2qvariant( const QObject *object )
  {
    QVariantMap map;
    if ( !object )
    {
      return map;
    }

    const QMetaObject *metaObject = object->metaObject();
    for ( int i = 0; i < metaObject->propertyCount(); ++i )
    {
      QMetaProperty metaproperty = metaObject->property( i );
      if ( metaproperty.isReadable() )
      {
        QVariant val = object->property( metaproperty.name() );
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
        if ( ( val.metaType().flags() & QMetaType::IsEnumeration ) )
        {
          val.convert( QMetaType::Int );
        }
#endif
        map[ QLatin1String( metaproperty.name() ) ] = val;
      }
    }
    return map;
  }


  void
  qvariant2qobject( const QVariantMap &variant, QObject *object )
  {
    for ( QVariantMap::const_iterator iter = variant.begin(); iter != variant.end(); ++iter )
    {
      QVariant property = object->property( iter.key().toLatin1() );
      Q_ASSERT( property.isValid() );
      if ( property.isValid() )
      {
        QVariant value = iter.value();
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
        const QVariant::Type propertyType = property.type();
#else
        const QMetaType propertyType = property.metaType();
#endif
        if ( value.canConvert( propertyType ) )
        {
          value.convert( propertyType );
          object->setProperty( iter.key().toLatin1(), value );
        }
        else if ( QString( QLatin1String( "QVariant" ) ).compare( QLatin1String( property.typeName() ) ) == 0 )
        {
          object->setProperty( iter.key().toLatin1(), value );
        }
      }
    }
  }


  QVariant
  parseJson( const QByteArray &jsonData, bool *ok, QByteArray *errorString )
  {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson( jsonData, &error );
    if ( ok )
    {
      *ok = ( error.error == QJsonParseError::NoError );
    }
    if ( errorString && !ok )
    {
      *errorString = error.errorString().toUtf8();
    }
    return doc.toVariant();
  }


  QByteArray
  toJson( const QVariant &variant, bool *ok, QByteArray *errorString, bool indented )
  {
    QVariant _variant = variant;
    if ( variant.type() == QVariant::Hash )
    {
      // QJsonDocument cannot deal with QVariantHash, so convert.
      const QVariantHash hash = variant.toHash();
      QVariantMap map;
      QHashIterator<QString, QVariant> it( hash );
      while ( it.hasNext() )
      {
        it.next();
        map.insert( it.key(), it.value() );
      }
      _variant = map;
    }

    QJsonDocument doc = QJsonDocument::fromVariant( _variant );
    if ( ok )
    {
      *ok = !doc.isNull();
    }
    if ( errorString && !ok )
    {
      *errorString = QByteArray( "Failed to convert from variant" );
    }
    return doc.toJson( indented ? QJsonDocument::Indented : QJsonDocument::Compact );
  }

}
