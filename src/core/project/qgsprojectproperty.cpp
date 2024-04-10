/***************************************************************************
                          qgsproject.cpp -  description
                             -------------------
  begin                : February 24, 2005
  copyright            : (C) 2005 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectproperty.h"
#include "qgslogger.h"
#include "qgis.h"
#include "qgsmessagelog.h"

#include <QDomDocument>
#include <QStringList>

QgsProjectProperty::QgsProjectProperty() //NOLINT
{
}

void QgsProjectPropertyValue::dump( int tabs ) const
{
  Q_UNUSED( tabs )
#ifdef QGISDEBUG

  QString tabString;
  tabString.fill( '\t', tabs );

  if ( QMetaType::Type::QStringList == mValue.userType() )
  {
    const QStringList sl = mValue.toStringList();

    for ( const auto &string : sl )
    {
      QgsDebugMsgLevel( QStringLiteral( "%1[%2] " ).arg( tabString, string ), 4 );
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "%1%2" ).arg( tabString, mValue.toString() ), 4 );
  }
#endif
}

bool QgsProjectPropertyValue::readXml( const QDomNode &keyNode )
{
  // this *should* be a Dom element node
  QDomElement subkeyElement = keyNode.toElement();

  // get the type so that we can properly parse the key value
  QString typeString = subkeyElement.attribute( QStringLiteral( "type" ) );

  if ( typeString.isNull() )
  {
    QgsDebugError( QStringLiteral( "null ``type'' attribute for %1" ).arg( keyNode.nodeName() ) );

    return false;
  }

  // the values come in as strings; we need to restore them to their
  // original values *and* types
  mValue.clear();

  // get the type associated with the value first
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QMetaType::Type type = static_cast<QMetaType::Type>( QMetaType::type( typeString.toLocal8Bit().constData() ) );
#else
  QMetaType::Type type = static_cast<QMetaType::Type>( QMetaType::fromName( typeString.toLocal8Bit().constData() ).id() );
#endif

  // This huge switch is left-over from an earlier incarnation of
  // QgsProject where there was a fine level of granularity for value
  // types.  The current interface, borrowed from QgsSettings, supports a
  // very small sub-set of these types.  However, I've left all the
  // other types just in case the interface is expanded to include these
  // other types.

  switch ( type )
  {
    case QMetaType::Type::UnknownType:
      QgsDebugError( QStringLiteral( "invalid value type %1 .. " ).arg( typeString ) );
      return false;

    case QMetaType::Type::QVariantMap:
      QgsDebugError( QStringLiteral( "no support for QVariant::Map" ) );
      return false;

    case QMetaType::Type::QVariantList:
      QgsDebugError( QStringLiteral( "no support for QVariant::List" ) );
      return false;

    case QMetaType::Type::QString:
      mValue = subkeyElement.text();  // no translating necessary
      break;

    case QMetaType::Type::QStringList:
    {
      int i = 0;
      QDomNodeList values = keyNode.childNodes();

      // all the QStringList values will be inside <value> elements
      QStringList valueStringList;

      while ( i < values.count() )
      {
        if ( "value" == values.item( i ).nodeName() )
        {
          // <value>s have only one element, which contains actual string value
          valueStringList.append( values.item( i ).firstChild().nodeValue() );
        }
        else
        {
          QgsDebugError( QStringLiteral( "non <value> element ``%1'' in string list" ).arg( values.item( i ).nodeName() ) );
        }

        ++i;
      }

      mValue = valueStringList;
      break;
    }

    case QMetaType::Type::QFont:
      QgsDebugError( QStringLiteral( "no support for QVariant::Font" ) );
      return false;

    case QMetaType::Type::QPixmap:
      QgsDebugError( QStringLiteral( "no support for QVariant::Pixmap" ) );
      return false;

    case QMetaType::Type::QBrush:
      QgsDebugError( QStringLiteral( "no support for QVariant::Brush" ) );
      return false;

    case QMetaType::Type::QRect:
      QgsDebugError( QStringLiteral( "no support for QVariant::Rect" ) );
      return false;

    case QMetaType::Type::QSize:
      QgsDebugError( QStringLiteral( "no support for QVariant::Size" ) );
      return false;

    case QMetaType::Type::QColor:
      QgsDebugError( QStringLiteral( "no support for QVariant::Color" ) );
      return false;

    case QMetaType::Type::QPalette:
      QgsDebugError( QStringLiteral( "no support for QVariant::Palette" ) );
      return false;

    case QMetaType::Type::QPoint:
      QgsDebugError( QStringLiteral( "no support for QVariant::Point" ) );
      return false;

    case QMetaType::Type::QImage:
      QgsDebugError( QStringLiteral( "no support for QVariant::Image" ) );
      return false;

    case QMetaType::Type::Int:
      mValue = QVariant( subkeyElement.text() ).toInt();
      break;

    case QMetaType::Type::UInt:
      mValue = QVariant( subkeyElement.text() ).toUInt();
      break;

    case QMetaType::Type::Bool:
      mValue = QVariant( subkeyElement.text() ).toBool();
      break;

    case QMetaType::Type::Double:
      mValue = QVariant( subkeyElement.text() ).toDouble();
      break;

    case QMetaType::Type::QByteArray:
      mValue = QVariant( subkeyElement.text() ).toByteArray();
      break;

    case QMetaType::Type::QPolygon:
      QgsDebugError( QStringLiteral( "no support for QVariant::Polygon" ) );
      return false;

    case QMetaType::Type::QRegion:
      QgsDebugError( QStringLiteral( "no support for QVariant::Region" ) );
      return false;

    case QMetaType::Type::QBitmap:
      QgsDebugError( QStringLiteral( "no support for QVariant::Bitmap" ) );
      return false;

    case QMetaType::Type::QCursor:
      QgsDebugError( QStringLiteral( "no support for QVariant::Cursor" ) );
      return false;

    case QMetaType::Type::QBitArray :
      QgsDebugError( QStringLiteral( "no support for QVariant::BitArray" ) );
      return false;

    case QMetaType::Type::QKeySequence :
      QgsDebugError( QStringLiteral( "no support for QVariant::KeySequence" ) );
      return false;

    case QMetaType::Type::QPen :
      QgsDebugError( QStringLiteral( "no support for QVariant::Pen" ) );
      return false;

#if 0 // Currently unsupported variant types
    case QVariant::LongLong :
      value_ = QVariant( subkeyElement.text() ).toLongLong();
      break;

    case QVariant::ULongLong :
      value_ = QVariant( subkeyElement.text() ).toULongLong();
      break;
#endif
    default :
      QgsDebugError( QStringLiteral( "unsupported value type %1 .. not properly translated to QVariant" ).arg( typeString ) );
  }

  return true;

}


// keyElement is created by parent QgsProjectPropertyKey
bool QgsProjectPropertyValue::writeXml( QString const &nodeName,
                                        QDomElement    &keyElement,
                                        QDomDocument   &document )
{
  QDomElement valueElement = document.createElement( nodeName );

  // remember the type so that we can rebuild it when the project is read in
  valueElement.setAttribute( QStringLiteral( "type" ), mValue.typeName() );


  // we handle string lists differently from other types in that we
  // create a sequence of repeated elements to cover all the string list
  // members; each value will be in a <value></value> tag.
  // XXX Not the most elegant way to handle string lists?
  if ( QMetaType::Type::QStringList == mValue.userType() )
  {
    QStringList sl = mValue.toStringList();

    for ( QStringList::iterator i = sl.begin();
          i != sl.end();
          ++i )
    {
      QDomElement stringListElement = document.createElement( QStringLiteral( "value" ) );
      QDomText valueText = document.createTextNode( *i );
      stringListElement.appendChild( valueText );

      valueElement.appendChild( stringListElement );
    }
  }
  else                    // we just plop the value in as plain ole text
  {
    QDomText valueText = document.createTextNode( mValue.toString() );
    valueElement.appendChild( valueText );
  }

  keyElement.appendChild( valueElement );

  return true;
}


QgsProjectPropertyKey::QgsProjectPropertyKey( const QString &name )
  : mName( name )
{}

QgsProjectPropertyKey::~QgsProjectPropertyKey()
{
  clearKeys();
}

QVariant QgsProjectPropertyKey::value() const
{
  QgsProjectProperty *foundQgsProperty = mProperties.value( name() );

  if ( !foundQgsProperty )
  {
    QgsDebugError( QStringLiteral( "key has null child" ) );
    return QVariant();     // just return an QVariant::Invalid
  }

  return foundQgsProperty->value();
}


void QgsProjectPropertyKey::dump( int tabs ) const
{
  QString tabString;

  tabString.fill( '\t', tabs );

  QgsDebugMsgLevel( QStringLiteral( "%1name: %2" ).arg( tabString, name() ), 4 );

  tabs++;
  tabString.fill( '\t', tabs );

  if ( ! mProperties.isEmpty() )
  {
    QHashIterator < QString, QgsProjectProperty * > i( mProperties );
    while ( i.hasNext() )
    {
      if ( i.next().value()->isValue() )
      {
        QgsProjectPropertyValue *propertyValue = static_cast<QgsProjectPropertyValue *>( i.value() );

        if ( QMetaType::Type::QStringList == propertyValue->value().userType() )
        {
          QgsDebugMsgLevel( QStringLiteral( "%1key: <%2>  value:" ).arg( tabString, i.key() ), 4 );
          propertyValue->dump( tabs + 1 );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "%1key: <%2>  value: %3" ).arg( tabString, i.key(), propertyValue->value().toString() ), 4 );
        }
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "%1key: <%2>  subkey: <%3>" )
                          .arg( tabString,
                                i.key(),
                                static_cast<QgsProjectPropertyKey *>( i.value() )->name() ), 4 );
        i.value()->dump( tabs + 1 );
      }

#if 0
      qDebug( "<%s>", name().toUtf8().constData() );
      if ( i.value()->isValue() )
      {
        qDebug( "   <%s>", i.key().toUtf8().constData() );
      }
      i.value()->dump();
      if ( i.value()->isValue() )
      {
        qDebug( "   </%s>", i.key().toUtf8().constData() );
      }
      qDebug( "</%s>", name().toUtf8().constData() );
#endif
    }
  }

}



bool QgsProjectPropertyKey::readXml( const QDomNode &keyNode )
{
  int i = 0;
  QDomNodeList subkeys = keyNode.childNodes();

  while ( i < subkeys.count() )
  {
    // if the current node is an element that has a "type" attribute,
    // then we know it's a leaf node; i.e., a subkey _value_, and not
    // a subkey
    if ( subkeys.item( i ).hasAttributes() && // if we have attributes
         subkeys.item( i ).isElement() && // and we're an element
         subkeys.item( i ).toElement().hasAttribute( QStringLiteral( "type" ) ) ) // and we have a "type" attribute
    {
      // then we're a key value
      delete mProperties.take( subkeys.item( i ).nodeName() );
      mProperties.insert( subkeys.item( i ).nodeName(), new QgsProjectPropertyValue );

      QDomNode subkey = subkeys.item( i );

      if ( !mProperties[subkeys.item( i ).nodeName()]->readXml( subkey ) )
      {
        QgsDebugError( QStringLiteral( "unable to parse key value %1" ).arg( subkeys.item( i ).nodeName() ) );
      }
    }
    else // otherwise it's a subkey, so just recurse on down the remaining keys
    {
      addKey( subkeys.item( i ).nodeName() );

      QDomNode subkey = subkeys.item( i );

      if ( !mProperties[subkeys.item( i ).nodeName()]->readXml( subkey ) )
      {
        QgsDebugError( QStringLiteral( "unable to parse subkey %1" ).arg( subkeys.item( i ).nodeName() ) );
      }
    }

    ++i;
  }

  return true;
}


/*
  Property keys will always create a Dom element for itself and then
  recursively call writeXml for any constituent properties.
*/
bool QgsProjectPropertyKey::writeXml( QString const &nodeName, QDomElement &element, QDomDocument &document )
{
  // If it's an _empty_ node (i.e., one with no properties) we need to emit
  // an empty place holder; else create new Dom elements as necessary.

  QDomElement keyElement = document.createElement( nodeName ); // Dom element for this property key

  if ( ! mProperties.isEmpty() )
  {
    auto keys = mProperties.keys();
    std::sort( keys.begin(), keys.end() );

    for ( const auto &key : std::as_const( keys ) )
    {
      if ( !mProperties.value( key )->writeXml( key, keyElement, document ) )
        QgsMessageLog::logMessage( tr( "Failed to save project property %1" ).arg( key ) );
    }
  }

  element.appendChild( keyElement );

  return true;
}

void QgsProjectPropertyKey::entryList( QStringList &entries ) const
{
  // now add any leaf nodes to the entries list
  QHashIterator < QString, QgsProjectProperty * > i( mProperties );
  while ( i.hasNext() )
  {
    // add any of the nodes that have just a single value
    if ( i.next().value()->isLeaf() )
    {
      entries.append( i.key() );
    }
  }
}

void QgsProjectPropertyKey::subkeyList( QStringList &entries ) const
{
  // now add any leaf nodes to the entries list
  QHashIterator < QString, QgsProjectProperty * > i( mProperties );
  while ( i.hasNext() )
  {
    // add any of the nodes that have just a single value
    if ( !i.next().value()->isLeaf() )
    {
      entries.append( i.key() );
    }
  }
}


bool QgsProjectPropertyKey::isLeaf() const
{
  if ( 0 == count() )
  {
    return true;
  }
  else if ( 1 == count() )
  {
    QHashIterator < QString, QgsProjectProperty * > i( mProperties );

    if ( i.hasNext() && i.next().value()->isValue() )
    {
      return true;
    }
  }

  return false;
}

void QgsProjectPropertyKey::setName( const QString &name )
{
  mName = name;
}
