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

#include <QDomDocument>
#include <QStringList>

static const char * const ident_ = "$Id$";


void QgsPropertyValue::dump( size_t tabs ) const
{
    QString tabString;
    tabString.fill( '\t', tabs );

    if (QVariant::StringList == value_.type())
    {
        QStringList sl = value_.toStringList();

        for (QStringList::const_iterator i = sl.begin(); i != sl.end(); ++i)
        {
            qDebug("%s[%s] ", tabString.toLocal8Bit().constData(), (*i).toLocal8Bit().constData());
        } 
    }
    else
    {
        qDebug("%s%s", tabString.toLocal8Bit().constData(), value_.toString().toLocal8Bit().constData());
    }
} // QgsPropertyValue::dump()



bool QgsPropertyValue::readXML(QDomNode & keyNode)
{
    // this *should* be a DOM element node
    QDomElement subkeyElement = keyNode.toElement();

    // get the type so that we can properly parse the key value
    QString typeString = subkeyElement.attribute("type");

    if (QString::null == typeString)
    {
        qDebug("%s:%d null ``type'' attribute for %s", __FILE__, __LINE__,
               keyNode.nodeName().toUtf8().constData());

        return false;
    }

    // the values come in as strings; we need to restore them to their
    // original values *and* types
    value_.clear();

    // get the type associated with the value first
    QVariant::Type type = QVariant::nameToType(typeString.toLocal8Bit().constData());

    // This huge switch is left-over from an earlier incarnation of
    // QgsProject where there was a fine level of granularity for value
    // types.  The current interface, borrowed from QSettings, supports a
    // very small sub-set of these types.  However, I've left all the
    // other types just in case the interface is expanded to include these
    // other types.

    switch (type)
    {
        case QVariant::Invalid:
            qDebug("%s:%d invalid value type %s .. ", __FILE__, __LINE__,
                   typeString.toUtf8().constData());

            return false;

            break;

        case QVariant::Map:
            qDebug("qgsproject.cpp:%d add support for QVariant::Map", __LINE__);

            return false;

            break;

        case QVariant::List:
            qDebug("qgsproject.cpp:%d add support for QVariant::List", __LINE__);

            return false;

            break;

        case QVariant::String:
            value_ = subkeyElement.text();  // no translating necessary
            break;

        case QVariant::StringList:
        {
            int i = 0;
            QDomNodeList values = keyNode.childNodes();

            // all the QStringList values will be inside <value> elements
            QStringList valueStringList;

            while (i < values.count())
            {
                if ("value" == values.item(i).nodeName())
                {                     // <value>s have only one element, which contains actual string value
                    valueStringList.append(values.item(i).firstChild().nodeValue());
                } else
                {
                    qDebug
                        ("qgsproject.cpp:%d non <value> element ``%s'' in string list",
                         __LINE__, values.item(i).nodeName().toUtf8().constData());
                }

                ++i;
            }

            value_ = valueStringList;

            break;
        }
        case QVariant::Font:
            qDebug("qgsproject.cpp:%d add support for QVariant::Font", __LINE__);

            return false;

            break;

        case QVariant::Pixmap:
            qDebug("qgsproject.cpp:%d add support for QVariant::Pixmap", __LINE__);

            return false;

            break;

        case QVariant::Brush:
            qDebug("qgsproject.cpp:%d add support for QVariant::Brush", __LINE__);

            return false;

            break;

        case QVariant::Rect:
            qDebug("qgsproject.cpp:%d add support for QVariant::Rect", __LINE__);

            return false;

            break;

        case QVariant::Size:
            qDebug("qgsproject.cpp:%d add support for QVariant::Size", __LINE__);

            return false;

            break;

        case QVariant::Color:
            qDebug("qgsproject.cpp:%d add support for QVariant::Color", __LINE__);

            return false;

            break;

        case QVariant::Palette:
            qDebug("qgsproject.cpp:%d add support for QVariant::Palette", __LINE__);

            return false;

            break;

        case QVariant::Point:
            qDebug("qgsproject.cpp:%d add support for QVariant::Point", __LINE__);

            return false;

            break;

        case QVariant::Image:
            qDebug("qgsproject.cpp:%d add support for QVariant::Image", __LINE__);

            return false;

            break;

        case QVariant::Int:
            value_ = QVariant(subkeyElement.text()).toInt();

            break;

        case QVariant::UInt:
            value_ = QVariant(subkeyElement.text()).toUInt();

            break;

        case QVariant::Bool:
            value_ = QVariant(subkeyElement.text()).toBool();

            break;

        case QVariant::Double:
            value_ = QVariant(subkeyElement.text()).toDouble();

            break;

        case QVariant::ByteArray:
            value_ = QVariant(subkeyElement.text()).toByteArray();

            break;

        case QVariant::Polygon:
            qDebug("qgsproject.cpp:%d add support for QVariant::Polygon",
                   __LINE__);

            return false;

            break;

        case QVariant::Region:
            qDebug("qgsproject.cpp:%d add support for QVariant::Region", __LINE__);

            return false;

            break;

        case QVariant::Bitmap:
            qDebug("qgsproject.cpp:%d add support for QVariant::Bitmap", __LINE__);

            return false;

            break;


        case QVariant::Cursor:
            qDebug("qgsproject.cpp:%d add support for QVariant::Cursor", __LINE__);
            return false;

            break;

        case QVariant::BitArray :
            qDebug( "qgsproject.cpp:%d add support for QVariant::BitArray", __LINE__ );

            return false;

            break;

        case QVariant::KeySequence :
            qDebug( "qgsproject.cpp:%d add support for QVariant::KeySequence", __LINE__ );

            return false;

            break;

        case QVariant::Pen :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Pen", __LINE__ );

            return false;

            break;
            //
            // QGIS DIES NOT SUPPORT THESE VARIANT TYPES IN VERSION 3.1 DISABLING FOR NOW
            //
            /*
              case QVariant::LongLong :
              value_ = QVariant(subkeyElement.text()).toLongLong();
              break;

              case QVariant::ULongLong :
              value_ = QVariant(subkeyElement.text()).toULongLong();
              break;
            */
        default :
            qDebug( "%s:%d unsupported value type %s .. not propertly translated to QVariant in qgsproject.cpp",
                    __FILE__, __LINE__, typeString.toUtf8().constData() );
    }

    return true;

} // QgsPropertyValue::readXML


/**
   @param element created by parent QgsPropertyKey
*/
bool QgsPropertyValue::writeXML( QString const & nodeName, 
                                 QDomElement   & keyElement, 
                                 QDomDocument  & document )
{
    QDomElement valueElement = document.createElement( nodeName );

    // remember the type so that we can rebuild it when the project is read in
    valueElement.setAttribute( "type", value_.typeName() );


    // we handle string lists differently from other types in that we
    // create a sequence of repeated elements to cover all the string list
    // members; each value will be in a <value></value> tag.
    // XXX Not the most elegant way to handle string lists?
    if ( QVariant::StringList == value_.type() )
    {
        QStringList sl = value_.toStringList();

        for ( QStringList::iterator i = sl.begin();
              i != sl.end();
              ++i )
        {
            QDomElement stringListElement = document.createElement( "value" );
            QDomText valueText = document.createTextNode( *i );
            stringListElement.appendChild( valueText );

            valueElement.appendChild( stringListElement );
        }
    }
    else                    // we just plop the value in as plain ole text
    {
        QDomText valueText = document.createTextNode( value_.toString() );
        valueElement.appendChild( valueText );
    }

    keyElement.appendChild( valueElement );

    return true;
} // QgsPropertyValue::writeXML




QgsPropertyKey::QgsPropertyKey( QString const name )
    : mName( name )
{}

QgsPropertyKey::~QgsPropertyKey()
{
    clearKeys();
}

QVariant QgsPropertyKey::value() const
{
    QgsProperty * foundQgsProperty;

    if ( 0 == ( foundQgsProperty = mProperties.value(name()) ) )
    {                        // recurse down to next key
        return foundQgsProperty->value();
    } else
    {
        qDebug("%s:%d QgsPropertyKey has null child", __FILE__, __LINE__);

        return QVariant();     // just return an QVariant::Invalid
    }
} // QVariant QgsPropertyKey::value() 


void QgsPropertyKey::dump( size_t tabs ) const
{
    QString tabString;

    tabString.fill( '\t', tabs );

    qDebug( "%sname: %s", tabString.toLocal8Bit().constData(), name().toLocal8Bit().constData() );
         
    tabs++;
    tabString.fill( '\t', tabs );

    if ( ! mProperties.isEmpty() )
    {
        QHashIterator < QString, QgsProperty* > i(mProperties);
        while (i.hasNext())
        {
            if ( i.next().value()->isValue() )
            {
                QgsPropertyValue * propertyValue = 
                    dynamic_cast<QgsPropertyValue*>( i.value() );

                if ( QVariant::StringList == propertyValue->value().type() )
                {
                    qDebug("%skey: <%s>  value:", 
                           tabString.toLocal8Bit().constData(), 
                           i.key().toLocal8Bit().constData() );

                    propertyValue->dump( tabs + 1 );
                }
                else
                {
                    qDebug("%skey: <%s>  value: %s", 
                           tabString.toLocal8Bit().constData(), 
                           i.key().toLocal8Bit().constData(), 
                           propertyValue->value().toString().toLocal8Bit().constData() );
                }
            }
            else
            {
                qDebug("%skey: <%s>  subkey: <%s>", 
                       tabString.toLocal8Bit().constData(), 
                       i.key().toLocal8Bit().constData(),
                       dynamic_cast<QgsPropertyKey*>(i.value())->name().toLocal8Bit().data() );

                i.value()->dump( tabs + 1 );
            }

//              qDebug("<%s>", name().toUtf8().constData());
//              if ( i.value()->isValue() )
//              {
//                  qDebug("   <%s>", i.key().toUtf8().constData() );
//              }
//              i.value()->dump();
//              if ( i.value()->isValue() )
//              {
//                  qDebug("   </%s>", i.key().toUtf8().constData() );
//              }
//              qDebug("</%s>", name().toUtf8().constData());
        }
    }

} // QgsPropertyKey::dump



bool QgsPropertyKey::readXML(QDomNode & keyNode)
{
    int i = 0;
    QDomNodeList subkeys = keyNode.childNodes();

    while (i < subkeys.count())
    {
        // if the current node is an element that has a "type" attribute,
        // then we know it's a leaf node; i.e., a subkey _value_, and not
        // a subkey
        if (subkeys.item(i).hasAttributes() && // if we have attributes
            subkeys.item(i).isElement() && // and we're an element
            subkeys.item(i).toElement().hasAttribute("type")) // and we have a "type" attribute
        {                   // then we're a key value
            delete mProperties.take(subkeys.item(i).nodeName());
            mProperties.insert(subkeys.item(i).nodeName(), new QgsPropertyValue);

            QDomNode subkey = subkeys.item(i);

            if (!mProperties[subkeys.item(i).nodeName()]->readXML(subkey))
            {
                qDebug("%s:%d unable to parse key value %s", __FILE__, __LINE__,
                       subkeys.item(i).nodeName().toUtf8().constData());
            }
        } else             // otherwise it's a subkey, so just
            // recurse on down the remaining keys
        {
            addKey( subkeys.item(i).nodeName() );

            QDomNode subkey = subkeys.item(i);

            if (!mProperties[subkeys.item(i).nodeName()]->readXML(subkey))
            {
                qDebug("%s:%d unable to parse subkey %s", __FILE__, __LINE__,
                       subkeys.item(i).nodeName().toUtf8().constData());
            }
        }

        ++i;
    }

    return true;
} // QgsPropertyKey::readXML(QDomNode & keyNode)


/**
  Property keys will always create a DOM element for itself and then
  recursively call writeXML for any constituent properties.
*/
bool QgsPropertyKey::writeXML(QString const &nodeName, QDomElement & element, QDomDocument & document)
{
    // If it's an _empty_ node (i.e., one with no properties) we need to emit
    // an empty place holder; else create new DOM elements as necessary.

    QDomElement keyElement = document.createElement(nodeName); // DOM element for this property key

    if ( ! mProperties.isEmpty() )
    {
        QHashIterator < QString, QgsProperty* > i(mProperties);
        while (i.hasNext())
        {
            i.next();
            if (!i.value()->writeXML(i.key(), keyElement, document))
            {
                return false;
            }
        }
    }

    element.appendChild(keyElement);

    return true;
} // QgsPropertyKey::writeXML



/** return keys that do not contain other keys
 */ 
void QgsPropertyKey::entryList( QStringList & entries ) const
{
    // now add any leaf nodes to the entries list
    QHashIterator < QString, QgsProperty* > i(mProperties);
    while (i.hasNext())
    {
        // add any of the nodes that have just a single value
        if (i.next().value()->isLeaf())
        {
            entries.append(i.key());
        }
    }
} // QgsPropertyKey::entryList



void QgsPropertyKey::subkeyList(QStringList & entries) const
{
    // now add any leaf nodes to the entries list
    QHashIterator < QString, QgsProperty* > i(mProperties);
    while (i.hasNext())
    {
        // add any of the nodes that have just a single value
        if (!i.next().value()->isLeaf())
        {
            entries.append(i.key());
        }
    }
} // QgsPropertyKey::subkeyList


bool QgsPropertyKey::isLeaf() const
{
    if (0 == count())
    {
        return true;
    }
    else if (1 == count())
    {
        QHashIterator < QString, QgsProperty* > i(mProperties);

        if (i.hasNext() && i.next().value()->isValue())
        {
            return true;
        }
    }

    return false;
} // QgsPropertyKey::isLeaf
