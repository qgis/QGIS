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


#include <qdom.h>


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
            qDebug("%s[%s] ", tabString.ascii(), (const char *) (*i).ascii());
        } 
    }
    else
    {
        qDebug("%s%s", tabString.ascii(), (const char *) value_.toString().ascii());
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
               (const char *) keyNode.nodeName().utf8());

        return false;
    }

    // the values come in as strings; we need to restore them to their
    // original values *and* types
    value_.clear();

    // get the type associated with the value first
    QVariant::Type type = QVariant::nameToType(typeString);

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
                   (const char *) typeString.utf8());

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
            size_t i = 0;
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
                         __LINE__, (const char *) values.item(i).nodeName().utf8());
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

        case QVariant::ColorGroup:
            qDebug("qgsproject.cpp:%d add support for QVariant::ColorGroup",
                   __LINE__);

            return false;

            break;

        case QVariant::IconSet:
            qDebug("qgsproject.cpp:%d add support for QVariant::IconSet", __LINE__);

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
            value_ = QVariant(subkeyElement.text()).asInt();

            break;

        case QVariant::UInt:
            value_ = QVariant(subkeyElement.text()).asUInt();

            break;

        case QVariant::Bool:
            value_ = QVariant(subkeyElement.text()).asBool();

            break;

        case QVariant::Double:
            value_ = QVariant(subkeyElement.text()).asDouble();

            break;

        case QVariant::CString:
            value_ = QVariant(subkeyElement.text()).asCString();

            break;

        case QVariant::PointArray:
            qDebug("qgsproject.cpp:%d add support for QVariant::PointArray",
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

        case QVariant::ByteArray :
            qDebug( "qgsproject.cpp:%d add support for QVariant::ByteArray", __LINE__ );

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
              value_ = QVariant(subkeyElement.text()).asLongLong();
              break;

              case QVariant::ULongLong :
              value_ = QVariant(subkeyElement.text()).asULongLong();
              break;
            */
        default :
            qDebug( "%s:%d unsupported value type %s .. not propertly translated to QVariant in qgsproject.cpp:%d",
                    __FILE__, __LINE__, (const char*)typeString.utf8() );
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
        QStringList sl = value_.asStringList();

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
    : name_( name )
{
    // since we own our properties, we are responsible for deleting the
    // contents
    properties_.setAutoDelete(true);
}

QVariant QgsPropertyKey::value() const
{
    QgsProperty * foundQgsProperty;

    if ( foundQgsProperty = properties_.find( name()) )
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

    qDebug( "%sname: %s", tabString.ascii(), name().ascii() );
         
    tabs++;
    tabString.fill( '\t', tabs );

    for (QDictIterator < QgsProperty > i(properties_); i.current(); ++i)
    {
        if ( i.current()->isValue() )
        {
            QgsPropertyValue * propertyValue = 
                dynamic_cast<QgsPropertyValue*>( i.current() );

            if ( QVariant::StringList == propertyValue->value().type() )
            {
                qDebug("%skey: <%s>  value:", 
                       tabString.ascii(), 
                       i.currentKey().ascii() );

                propertyValue->dump( tabs + 1 );
            }
            else
            {
                qDebug("%skey: <%s>  value: %s", 
                       tabString.ascii(), 
                       i.currentKey().ascii(), 
                       propertyValue->value().toString().ascii() );
            }
        }
        else
        {
            qDebug("%skey: <%s>  subkey: <%s>", 
                   tabString.ascii(), 
                   i.currentKey().ascii(),
                   dynamic_cast<QgsPropertyKey*>(i.current())->name().ascii() );

            i.current()->dump( tabs + 1 );
        }

//              qDebug("<%s>", (const char *) name().utf8());
//              if ( i.current()->isValue() )
//              {
//                  qDebug("   <%s>", (const char*) i.currentKey().utf8() );
//              }
//              i.current()->dump();
//              if ( i.current()->isValue() )
//              {
//                  qDebug("   </%s>", (const char*) i.currentKey().utf8() );
//              }
//              qDebug("</%s>", (const char *) name().utf8());
    }
} // QgsPropertyKey::dump



bool QgsPropertyKey::readXML(QDomNode & keyNode)
{
    size_t i = 0;
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
            properties_.replace(subkeys.item(i).nodeName(), new QgsPropertyValue);

            QDomNode subkey = subkeys.item(i);

            if (!properties_[subkeys.item(i).nodeName()]->readXML(subkey))
            {
                qDebug("%s:%d unable to parse key value %s", __FILE__, __LINE__,
                       (const char *) subkeys.item(i).nodeName().utf8());
            }
        } else             // otherwise it's a subkey, so just
            // recurse on down the remaining keys
        {
            addKey( subkeys.item(i).nodeName() );

            QDomNode subkey = subkeys.item(i);

            if (!properties_[subkeys.item(i).nodeName()]->readXML(subkey))
            {
                qDebug("%s:%d unable to parse subkey %s", __FILE__, __LINE__,
                       (const char *) subkeys.item(i).nodeName().utf8());
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

    if ( ! properties_.isEmpty() )
    {
        for (QDictIterator < QgsProperty > i(properties_); i.current(); ++i)
        {
            if (!i.current()->writeXML(i.currentKey(), keyElement, document))
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
    for (QDictIterator<QgsProperty> i(properties_); i.current(); ++i)
    {
        const char *currentNodeStr = i.currentKey().ascii(); // debugger probe

        // add any of the nodes that have just a single value
        if (i.current()->isLeaf())
        {
            entries.append(i.currentKey());
        }
    }
} // QgsPropertyKey::entryList



void QgsPropertyKey::subkeyList(QStringList & entries) const
{
    // now add any leaf nodes to the entries list
    for (QDictIterator < QgsProperty > i(properties_); i.current(); ++i)
    {
        const char *currentNodeStr = i.currentKey().ascii(); // debugger probe

        // add any of the nodes that have just a single value
        if (!i.current()->isLeaf())
        {
            entries.append(i.currentKey());
        }
    }
} // QgsPropertyKey::subkeyList


bool QgsPropertyKey::isLeaf() const
{
    int c = count();         // debugger probe

    if (0 == count())
    {
        return true;
    } else if (1 == count())
    {
        QDictIterator < QgsProperty > i(properties_);

        if (i.current() && i.current()->isValue())
        {
            return true;
        }
    }

    return false;
} // QgsPropertyKey::isLeaf



