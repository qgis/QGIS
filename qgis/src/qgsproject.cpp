/***************************************************************************
                          qgsproject.cpp -  description
                             -------------------
  begin                : July 23, 2004
  copyright            : (C) 2004 by Mark Coletti
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

#include "qgsproject.h"

#include <memory>
#include <cassert>
#include <iostream>

using namespace std;

// #include "qgisapp.h"
#include "qgsrect.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgssimarenderer.h"
#include "qgsgraduatedmarenderer.h"
#include "qgsmaplayerregistry.h"
#include "qgsexception.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qdict.h>
#include <qmessagebox.h>
#include <qwidgetlist.h>



static const char * const ident_ = "$Id$";




/// canonical project instance
QgsProject * QgsProject::theProject_;


/**
   ABC for property hierarchies

   Each sub-class is either a PropertyKey or PropertyValue.  PropertyKeys can
   contain either PropertyKeys or PropertyValues, thus describing an
   hierarchy.  PropertyValues are always graph leaves.

 */
class Property
{
public:

    Property()
    {}

    virtual ~Property()
    {}

    /// return the QVariant value for the given key
    /**
        keyName will be a QStringList that's been tokenized by the caller from '/' delimiters

        @note will be QVariant::Invalid if no value
    */
    virtual QVariant value( QStringList & keyName ) const = 0;

    /// set value for the given key
    /**
        keyName will be a QStringList that's been tokened by the caller from '/' delimiters
    */
    virtual bool setValue( QStringList & keyName, QVariant const & value ) = 0;

    /** dumps out the keys and values

       @note used for debugging
    */
    virtual void dump( ) const = 0;

    /** returns true if is a PropertyKey */
    virtual bool isKey() const = 0;

    /** returns true if is a PropertyValue */
    virtual bool isValue() const = 0;

    /** returns true if a leaf node 

      A leaf node is a key node that has either no value or a single value.  A
      non-leaf node would be a key node with key sub-nodes.
    */
    virtual bool isLeaf() const = 0;

    /**
       returns list of keys that do not contain other keys a given key

       @param keyName will be a QStringList that's been tokened by the caller
                      from '/' delimiters

       @param entries will be list of key entries built so far from recursive calls
     */
    virtual void entryList( QStringList & keyName, 
                            QStringList & entries ) const = 0;


    /**
       deletes the given key

       @param keyname key list tokenized by '/' by caller
    */
    virtual bool remove( QStringList & keyname ) = 0;

    /**
       restores property hierarchy to given DOM node

       Used for restoring properties from project file
     */
    virtual bool readXML( QDomNode & keyNode ) = 0;

    /**
       adds property hierarchy to given DOM node

       Used for saving properties to project file.

       @param nodeName the tag name associated with this element
       @param node     the parent (or encompassing) property node
       @param documetn the overall project file DOM document
     */
    virtual bool writeXML( QString const & nodeName, QDomNode & node, QDomDocument & document ) = 0;

    /// how many elements are contained within this one?
    /**
       @note does not recursively count all sub-keys, only immediate objects
    */
    virtual size_t count() const = 0;

    /// Does this property not have any subkeys or values?
    virtual bool isEmpty() const = 0;

}; // class Property




/**
   PropertyValue node

   Contains a PropertyKey's value

 */
class PropertyValue : public Property
{
public:

    PropertyValue( )
    {}

    PropertyValue( QVariant const & value )
            : value_(value)
    {}

    virtual ~PropertyValue()
    {}


    /** returns true if is a PropertyKey */
    virtual bool isKey() const
    {
        return false;
    }

    /** returns true if is a PropertyValue */
    virtual bool isValue() const
    {
        return true;
    }

    /** returns true if is a leaf node 

        I suppose, in a way, value nodes can also be qualified as leaf nodes
        even though we're only counting key nodes.

        XXX Maybe this shouldn't be the case?
    */
    virtual bool isLeaf() const
    {
        return true;
    }

    /// just returns the value
    virtual QVariant value( QStringList & keyName ) const
    {
        // if keyName isn't empty, then something went wrong in the recursion
        if ( ! keyName.empty() )
        {
#ifdef QGISDEBUG        
            qDebug( "%s:%d PropertyValue given a non-empty keyName", __FILE__, __LINE__ );
#endif            
        }

        // we ignore keyName since we're a leaf node and don't have one
        return value_;
    }

    /**
       sets the PropertyValue value
    */
    virtual bool setValue( QStringList & keyName, QVariant const & value )
    {
        value_ = value;

        if ( ! keyName.empty() )
        {
            qDebug( "%s:%d PropertyValue given a non-empty keyName", __FILE__, __LINE__ );

            return false;
        }

        if ( ! value_.isValid() )
        {
#ifdef QGISDEBUG                
            qDebug( "%s:%d PropertyValue given an invaild value", __FILE__, __LINE__ );
#endif      

            return false;
        }

        if ( value_.isNull() )
        {
#ifdef QGISDEBUG               
            qDebug( "%s:%d PropertyValue given a null value", __FILE__, __LINE__ );
#endif            

            // XXX return false; I guess this might be ok?
        }

        return true;
    }

    virtual void dump( ) const
    {
        if ( QVariant::StringList == value_.type() )
        {
            QStringList sl = value_.toStringList();

            for ( QStringList::const_iterator i = sl.begin();
                    i != sl.end();
                    ++i )
            {
                qDebug( "[%s] ", (const char*)(*i).utf8() );
            }
        }
        else
        {
            qDebug( "%s", (const char*)value_.toString().utf8() );
        }
    }

    /**
       deletes the given key

       @param keyname key list tokenized by '/' by caller
    */
    /* virtual */ bool remove( QStringList & keyname )
    {
        // NOP; parent PropertyKey will delete this PropertyValue in its remove()

        return false;           // we shouldn't ever be calling this, so return false
    } // remove


    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const
    {   // values are leaf nodes, so are always empty
        return true;
    }


    /* virtual */ bool readXML( QDomNode & keyNode )
    {
        // this *should* be a DOM element node
        QDomElement subkeyElement = keyNode.toElement();

        // get the type so that we can properly parse the key value
        QString typeString = subkeyElement.attribute( "type" );

        if ( QString::null == typeString )
        {
            qDebug( "%s:%d null ``type'' attribute for %s",
                    __FILE__, __LINE__, (const char*)keyNode.nodeName().utf8() );

            return false;
        }

        // the values come in as strings; we need to restore them to their
        // original values *and* types
        value_.clear();

        // get the type associated with the value first
        QVariant::Type type = QVariant::nameToType( typeString );

        // This huge switch is left-over from an earlier incarnation of
        // QgsProject where there was a fine level of granularity for value
        // types.  The current interface, borrowed from QSettings, supports a
        // very small sub-set of these types.  However, I've left all the
        // other types just in case the interface is expanded to include these
        // other types.

        switch ( type )
        {
        case QVariant::Invalid :
            qDebug( "%s:%d invalid value type %s .. ",
                    __FILE__, __LINE__, (const char*)typeString.utf8() );

            return false;

            break;

        case QVariant::Map :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Map", __LINE__ );

            return false;

            break;

        case QVariant::List :
            qDebug( "qgsproject.cpp:%d add support for QVariant::List", __LINE__ );

            return false;

            break;

        case QVariant::String :
            value_ = subkeyElement.text(); // no translating necessary
            break;

        case QVariant::StringList :
            {
                size_t i = 0;
                QDomNodeList values = keyNode.childNodes();

                // all the QStringList values will be inside <value> elements
                QStringList valueStringList;

                while ( i < values.count() )
                {
                    if ( "value" == values.item(i).nodeName() )
                    { // <value>s have only one element, which contains actual string value
                        valueStringList.append( values.item(i).firstChild().nodeValue() );
                    }
                    else
                    {
                        qDebug( "qgsproject.cpp:%d non <value> element ``%s'' in string list",
                                __LINE__, (const char*)values.item(i).nodeName().utf8() );
                    }

                    ++i;
                }

                value_ = valueStringList;

                break;
            }
        case QVariant::Font :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Font", __LINE__ );

            return false;

            break;

        case QVariant::Pixmap :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Pixmap", __LINE__ );

            return false;

            break;

        case QVariant::Brush :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Brush", __LINE__ );

            return false;

            break;

        case QVariant::Rect :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Rect", __LINE__ );

            return false;

            break;

        case QVariant::Size :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Size", __LINE__ );

            return false;

            break;

        case QVariant::Color :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Color", __LINE__ );

            return false;

            break;

        case QVariant::Palette :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Palette", __LINE__ );

            return false;

            break;

        case QVariant::ColorGroup :
            qDebug( "qgsproject.cpp:%d add support for QVariant::ColorGroup", __LINE__ );

            return false;

            break;

        case QVariant::IconSet :
            qDebug( "qgsproject.cpp:%d add support for QVariant::IconSet", __LINE__ );

            return false;

            break;

        case QVariant::Point :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Point", __LINE__ );

            return false;

            break;

        case QVariant::Image :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Image", __LINE__ );

            return false;

            break;

        case QVariant::Int :
            value_ = QVariant(subkeyElement.text()).asInt();

            break;

        case QVariant::UInt :
            value_ = QVariant(subkeyElement.text()).asUInt();

            break;

        case QVariant::Bool :
            value_ = QVariant(subkeyElement.text()).asBool();

            break;

        case QVariant::Double :
            value_ = QVariant(subkeyElement.text()).asDouble();

            break;

        case QVariant::CString :
            value_ = QVariant(subkeyElement.text()).asCString();

            break;

        case QVariant::PointArray :
            qDebug( "qgsproject.cpp:%d add support for QVariant::PointArray", __LINE__ );

            return false;

            break;

        case QVariant::Region :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Region", __LINE__ );

            return false;

            break;

        case QVariant::Bitmap :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Bitmap", __LINE__ );

            return false;

            break;

        case QVariant::Cursor :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Cursor", __LINE__ );

            return false;

            break;

        case QVariant::SizePolicy :
            qDebug( "qgsproject.cpp:%d add support for QVariant::SizePolicy", __LINE__ );

            return false;

            break;

        case QVariant::Date :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Date", __LINE__ );

            return false;

            break;

        case QVariant::Time :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Time", __LINE__ );

            return false;

            break;

        case QVariant::DateTime :
            qDebug( "qgsproject.cpp:%d add support for QVariant::DateTime", __LINE__ );

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

    } // readXML



    /* virtual */ bool writeXML( QString const & nodeName, QDomNode & node, QDomDocument & document )
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

        node.appendChild( valueElement );

        return true;
    }

    /// how many elements are contained within this one?
    size_t count() const
    {
        return 0;
    }


    /** return keys that do not contain other keys

        Since PropertyValue isn't a key, don't do anything.

     */
    void entryList( QStringList & keyName, QStringList & entries ) const
    {
        // NOP
    }

private:

    /** We use QVariant as it's very handy to keep multiple types and provides
        type conversions
    */
    QVariant value_;

}
; // class PropertyValue





/**
   PropertyKey node

   Can container either other PropertyKeys or a PropertyValue

 */
class PropertyKey : public Property
{
public:

    PropertyKey( )
    {
        // since we own our properties, we are responsible for deleting the
        // contents
        properties_.setAutoDelete( true );
    }


    /** construct new property key with given subkey name and value

      Called by setValue().  May possibly recurse to create more sub-keys.

    */
    PropertyKey( QStringList & keyName, QVariant const & value )
    {
        // since we own our properties, we are responsible for deleting the
        // contents
        properties_.setAutoDelete( true );

        // save the current free, which should be the front of the key list
        QString const currentKey = keyName.front();

        // now pop it off
        keyName.pop_front();

        if ( keyName.empty() )  // then we have a leaf node
        {
            properties_.insert( currentKey, new PropertyValue(value) );
        }
        else
        {
            properties_.insert( currentKey, new PropertyKey(keyName, value) );
        }
    }



    virtual ~PropertyKey()
    {}



    /** Give keyName string of the "/key1/key2/key3", return value associated with key3.

        keyName will be a QStringList that's been tokenized by the caller from '/' delimiters

       @note

       This recurses through property hierarchy until it gets to the
       leaf with the given name, which should be a PropertyValue, which then
       has its value returned.

    */
    /* virtual */
    QVariant value( QStringList & keyName ) const
    {
        // save the current key, which should be the front of the key list
        QString currentKey = keyName.front();

        // now pop it off
        keyName.pop_front();

        if ( properties_.find( currentKey ) )
        {   // recurse down to next key
            return properties_[currentKey]->value( keyName );
        }
        else
        {
            qDebug( "%s:%d PropertyKey has null child", __FILE__, __LINE__ );

            return QVariant();  // just return an QVariant::Invalid
        }
    }

    /* virtual */ bool setValue( QStringList & keyName, QVariant const & value )
    {
        // save the current free, which should be the front of the key list
        QString const currentKey = keyName.front();

        // now pop it off
        keyName.pop_front();

        if ( keyName.empty() )  // then we have a leaf node
        {
            properties_.replace( currentKey, new PropertyValue );

            return properties_[currentKey]->setValue( keyName, value );
        }
        else if ( properties_.find( currentKey ) )
        {   // recurse down to next key
            return properties_[currentKey]->setValue( keyName, value );
        }
        else
        {
            properties_.replace( currentKey, new PropertyKey );

            return properties_[currentKey]->setValue( keyName, value );
        }

        return true;
    }


    /* virtual */ void dump( ) const
    {
        for ( QDictIterator<Property> i(properties_); i.current(); ++i )
        {
            qDebug( "<%s>", (const char*)i.currentKey().utf8() );
            i.current()->dump( );
            qDebug( "</%s>", (const char*)i.currentKey().utf8() );
        }
    }


    /**
       deletes the given key

       @param keyname key list tokenized by '/' by caller
    */
    /* virtual */ bool remove
        ( QStringList & keyName )
    {
        // save the current key, which should be the front of the key list
        QString const currentKey = keyName.front();

        // now pop it off
        keyName.pop_front();

        if ( keyName.empty() )  // then we have a leaf node
        {
            properties_.remove( currentKey );
        }
        else if ( properties_.find( currentKey ) )
        {   // recurse down to next key
            properties_[currentKey]->remove
            ( keyName );

            // if that subkey is now empty, then delete it, too
            if ( properties_[currentKey]->isEmpty() )
            {
                properties_.remove( currentKey );
            }
        }
        else
        {
            qDebug( "%s:%d cannot find key %s to remove",
                    __FILE__, __LINE__,  (const char*)currentKey.utf8() );

            return false;
        }

        return true;

    } // remove


    /* virtual */ bool readXML( QDomNode & keyNode )
    {
        size_t i = 0;
        QDomNodeList subkeys = keyNode.childNodes();

        while ( i < subkeys.count() )
        {
            // if the current node is an element that has a "type" attribute,
            // then we know it's a leaf node; i.e., a subkey _value_, and not
            // a subkey
            if ( subkeys.item(i).hasAttributes() && // if we have attributes
                    subkeys.item(i).isElement() && // and we're an element
                    subkeys.item(i).toElement().hasAttribute("type") ) // and we have a "type" attribute
            {                                                       // then we're a key value
                properties_.replace( subkeys.item(i).nodeName(), new PropertyValue );

                QDomNode subkey = subkeys.item(i);

                if ( ! properties_[subkeys.item(i).nodeName()]->readXML( subkey ) )
                {
                    qDebug( "%s:%d unable to parse key value %s",
                            __FILE__, __LINE__, (const char*)subkeys.item(i).nodeName().utf8() );
                }
            }
            else // otherwise it's a subkey, so just recurse on down the remaining keys
            {
                properties_.replace( subkeys.item(i).nodeName(), new PropertyKey );

                QDomNode subkey = subkeys.item(i);

                if ( ! properties_[subkeys.item(i).nodeName()]->readXML( subkey) )
                {
                    qDebug( "%s:%d unable to parse subkey %s",
                            __FILE__, __LINE__, (const char*)subkeys.item(i).nodeName().utf8() );
                }
            }

            ++i;
        }

        return true;
    }


    /* virtual */ bool writeXML( QString const & nodeName, QDomNode & node, QDomDocument & document )
    {
        QDomElement keyElement = document.createElement( nodeName );

        for ( QDictIterator<Property> i(properties_); i.current(); ++i )
        {
            if ( ! i.current()->writeXML( i.currentKey(), keyElement, document ) )
            {
                return false;
            }
        }

        node.appendChild( keyElement );

        return true;
    }


    /// how many elements are contained within this one?
    size_t count() const
    {
        return properties_.count();
    }


    /// Does this property not have any subkeys or values?
    /* virtual */ bool isEmpty() const
    {
        return properties_.isEmpty();
    }

    /** returns true if is a PropertyKey */
    virtual bool isKey() const
    {
        return true;
    }

    /** returns true if is a PropertyValue */
    virtual bool isValue() const
    {
        return false;
    }


    /** return keys that do not contain other keys

        Since PropertyValue isn't a key, return keys without modification.

     */
    void entryList( QStringList & keyName, QStringList & entries ) const
    {
        // save the current key, which should be the front of the key list
        QString const currentKey = keyName.front();

        // now pop it off
        keyName.pop_front();

        // XXX actually we need to recurse until we pop off the last key node

        for ( QDictIterator<Property> i(properties_); i.current(); ++i )
        {
            // add any of the nodes that have just a single value
            if ( i.current()->isLeaf() )
            {
                entries.append( i.currentKey() );
            }
        }
    }


    /** returns true if a leaf node 

      A leaf node is a key node that has either no value or a single value.  A
      non-leaf node would be a key node with key sub-nodes.
    */
    bool isLeaf() const
    {
        if ( 0 == count() )
        {
            return true;
        }
        else if ( 1 == count() && properties_[0]->isValue() )
        {
            return true;
        }

        return false;
    }


private:

    /// sub-keys
    QDict<Property> properties_;

}
; // class PropertyKey




struct QgsProject::Imp
{
    /// current physical project file
    QFile file;

    /** set of plug-in (and possibly qgis) related properties

        String key is scope; i.e., QMap<Scope,PropertyKey>
     */
    QMap< QString, PropertyKey > properties_;
    // DEPRECATED QMap< QString, QgsProject::Properties > properties_;

    /// project title
    QString title;

    /// true if project has been modified since it has been read or saved
    bool dirty;

    /// map units for current project
    QgsScaleCalculator::units mapUnits;

    Imp()
      : title(""), dirty(false), mapUnits(QgsScaleCalculator::METERS)
      {}

    /* clear project properties when a new project is started */
    void clear()
    {
#ifdef QGISDEBUG
    std::cout << "Clearing project properties Impl->clear();" << std::endl;
#endif
        properties_.clear();
        mapUnits=  QgsScaleCalculator::METERS;
        title="";
    }

}
; // struct QgsProject::Imp




QgsProject::QgsProject()
        : imp_( new QgsProject::Imp )
{
  // Set some default project properties
  writeEntry("PositionPrecision","/Automatic", true);
  writeEntry("PositionPrecision","/DecimalPlaces", 2);
} // QgsProject ctor



QgsProject::~QgsProject()
{
    // std::auto_ptr automatically deletes imp_ when it's destroyed
} // QgsProject dtor



QgsProject *
QgsProject::instance()
{
    if ( ! QgsProject::theProject_ )
    {
        QgsProject::theProject_ = new QgsProject;
    }

    return QgsProject::theProject_;
} // QgsProject * instance()




void QgsProject::title( QString const & title )
{
    imp_->title = title;

    dirty(true);
} // void QgsProject::title


QString const & QgsProject::title() const
{
    return imp_->title;
} // QgsProject::title() const



QgsScaleCalculator::units QgsProject::mapUnits() const
{
    return imp_->mapUnits;
} // QgsScaleCalculator::units QgsProject::mapUnits() const



void QgsProject::mapUnits(QgsScaleCalculator::units u)
{
    imp_->mapUnits = u;

    dirty(true);
} // void QgsProject::mapUnits(QgsScaleCalculator::units u)

bool QgsProject::dirty() const
{
    return imp_->dirty;
} // bool QgsProject::dirty() const


void QgsProject::dirty( bool b )
{
    imp_->dirty = b;
} // bool QgsProject::dirty()





void QgsProject::filename( QString const & name )
{
    imp_->file.setName( name );

    dirty(true);
} // void QgsProject::filename( QString const & name )


QString QgsProject::filename() const
{
    return imp_->file.name();
} // QString QgsProject::filename() const



/// basically a debugging tool to dump property list values
static
void
dump_( QMap< QString, PropertyKey > const & property_list )
{
    qDebug( "current properties:" );

    for ( QMap< QString, PropertyKey >::const_iterator curr_scope =
                property_list.begin();
            curr_scope != property_list.end();
            curr_scope++ )
    {
        //        qDebug( "<%s>", curr_scope.key() );
        curr_scope.data().dump( );
        //       qDebug( "</%s>", curr_scope.key() );
    }
} // dump_



/**
   Fetches extents, or area of interest, saved in project.

   @note extent XML of form:
<extent>
        <xmin>[number]</xmin>
        <ymin>[number]</ymin>
        <xmax>[number]</xmax>
        <ymax>[number]</ymax>
</extent>

 */
static
bool
_getExtents( QDomDocument const & doc, QgsRect & aoi )
{
    QDomNodeList extents = doc.elementsByTagName("extent");

    if ( extents.count() > 1 )
    {
        qDebug( "there appears to be more than one extent in the project\nusing first one" );
    }
    else if ( extents.count() < 1 ) // no extents found, so bail
    {
        return false;
    }

    QDomNode extentNode = extents.item(0);

    QDomNode xminNode = extentNode.namedItem("xmin");
    QDomNode yminNode = extentNode.namedItem("ymin");
    QDomNode xmaxNode = extentNode.namedItem("xmax");
    QDomNode ymaxNode = extentNode.namedItem("ymax");

    QDomElement exElement = xminNode.toElement();
    double xmin = exElement.text().toDouble();
    aoi.setXmin( xmin );

    exElement = yminNode.toElement();
    double ymin = exElement.text().toDouble();
    aoi.setYmin( ymin );

    exElement = xmaxNode.toElement();
    double xmax = exElement.text().toDouble();
    aoi.setXmax( xmax );

    exElement = ymaxNode.toElement();
    double ymax = exElement.text().toDouble();
    aoi.setYmax( ymax );

    return true;

} // _getExtents



/**

  Used by _getProperties() to fetch individual properties for the given scope

*/
#ifdef DEPRECATED
static
void
_getScopeProperties( QDomNode const & scopeNode,
                     QMap< QString, QgsProject::Properties > & project_properties )
{
    QString scopeName = scopeNode.nodeName(); // syntactic short-cut

    size_t i = 0;
    QDomNodeList properties = scopeNode.childNodes();

    while ( i < properties.count() )
    {
        QDomNode currentProperty = properties.item(i);
        QDomNode currentValue    = currentProperty.firstChild(); // should only have one child

        qDebug( "Got property %s:%s (%s)",
                currentProperty.nodeName(),
                currentValue.nodeValue(),
                currentProperty.toElement().attributeNode("type").value() );

        // the values come in as strings; we need to restore them to their
        // original values *and* types
        QVariant restoredValue;

        // get the type associated with the value first
        QVariant::Type type =
            QVariant::nameToType( currentProperty.toElement().attributeNode("type").value() );

        switch ( type )
        {
        case QVariant::Invalid :
            qDebug( "qgsproject.cpp:%d invalid value type %s .. ",
                    __LINE__,
                    currentProperty.toElement().attributeNode("type").value() );

            restoredValue.clear();

            ++i;

            continue;

            break;

        case QVariant::Map :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Map", __LINE__ );
            break;

        case QVariant::List :
            qDebug( "qgsproject.cpp:%d add support for QVariant::List", __LINE__ );
            break;

        case QVariant::String :
            restoredValue = currentValue.nodeValue(); // no translating necessary
            break;

        case QVariant::StringList :
            qDebug( "qgsproject.cpp:%d add support for QVariant::StringList", __LINE__ );
            break;

        case QVariant::Font :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Font", __LINE__ );
            break;

        case QVariant::Pixmap :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Pixmap", __LINE__ );
            break;

        case QVariant::Brush :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Brush", __LINE__ );
            break;

        case QVariant::Rect :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Rect", __LINE__ );
            break;

        case QVariant::Size :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Size", __LINE__ );
            break;

        case QVariant::Color :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Color", __LINE__ );
            break;

        case QVariant::Palette :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Palette", __LINE__ );
            break;

        case QVariant::ColorGroup :
            qDebug( "qgsproject.cpp:%d add support for QVariant::ColorGroup", __LINE__ );
            break;

        case QVariant::IconSet :
            qDebug( "qgsproject.cpp:%d add support for QVariant::IconSet", __LINE__ );
            break;

        case QVariant::Point :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Point", __LINE__ );
            break;

        case QVariant::Image :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Image", __LINE__ );
            break;

        case QVariant::Int :
            restoredValue = QVariant(currentValue.nodeValue()).asInt();
            break;

        case QVariant::UInt :
            restoredValue = QVariant(currentValue.nodeValue()).asUInt();
            break;

        case QVariant::Bool :
            restoredValue = QVariant(currentValue.nodeValue()).asBool();
            break;

        case QVariant::Double :
            restoredValue = QVariant(currentValue.nodeValue()).asDouble();
            break;

        case QVariant::CString :
            qDebug( "qgsproject.cpp:%d add support for QVariant::CString", __LINE__ );
            break;

        case QVariant::PointArray :
            qDebug( "qgsproject.cpp:%d add support for QVariant::PointArray", __LINE__ );
            break;

        case QVariant::Region :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Region", __LINE__ );
            break;

        case QVariant::Bitmap :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Bitmap", __LINE__ );
            break;

        case QVariant::Cursor :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Cursor", __LINE__ );
            break;

        case QVariant::SizePolicy :
            qDebug( "qgsproject.cpp:%d add support for QVariant::SizePolicy", __LINE__ );
            break;

        case QVariant::Date :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Date", __LINE__ );
            break;

        case QVariant::Time :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Time", __LINE__ );
            break;

        case QVariant::DateTime :
            qDebug( "qgsproject.cpp:%d add support for QVariant::DateTime", __LINE__ );
            break;

        case QVariant::ByteArray :
            qDebug( "qgsproject.cpp:%d add support for QVariant::ByteArray", __LINE__ );
            break;

        case QVariant::BitArray :
            qDebug( "qgsproject.cpp:%d add support for QVariant::BitArray", __LINE__ );
            break;

        case QVariant::KeySequence :
            qDebug( "qgsproject.cpp:%d add support for QVariant::KeySequence", __LINE__ );
            break;

        case QVariant::Pen :
            qDebug( "qgsproject.cpp:%d add support for QVariant::Pen", __LINE__ );
            break;

        case QVariant::LongLong :
            restoredValue = QVariant(currentValue.nodeValue()).asLongLong();
            break;

        case QVariant::ULongLong :
            restoredValue = QVariant(currentValue.nodeValue()).asULongLong();
            break;

        default :
            qDebug( "unsupported value type %s .. not propertly translated to QVariant in qgsproject.cpp:%d",
                    currentProperty.toElement().attributeNode("type").value(),
                    __LINE__ );

            restoredValue.clear();

            ++i;

            continue;
        }


        project_properties[ scopeName ].append( QgsProject::PropertyValue( currentProperty.nodeName(),
                                                restoredValue ) );

        ++i;
    }

    _dump( project_properties );

} // _getScopeProperties()

#endif



/**

  Restore any optional properties found in "doc" to "properties".

  <properties> tags for all optional properties.  Within that there will be
  scope tags.  In the following example there exist one property in the
  "fsplugin" scope.  "layers" is a list containing three string values.

    <properties>
        <fsplugin>
            <foo type="int" >42</foo>
            <baz type="int" >1</baz>
            <layers type="QStringList" >
                <value>railroad</value>
                <value>airport</value>
            </layers>
            <xyqzzy type="int" >1</xyqzzy>
            <bar type="double" >123.456</bar>
            <feature_types type="QStringList" >
                <value>type</value>
            </feature_types>
        </fsplugin>
    </properties>

 */
static
void
_getProperties( QDomDocument const & doc, QMap< QString, PropertyKey > & project_properties )
{
    QDomNodeList properties = doc.elementsByTagName("properties");

    if ( properties.count() > 1 )
    {
        qDebug( "there appears to be more than one ``properties'' XML tag ... bailing" );
        return;
    }
    else if ( properties.count() < 1 ) // no properties found, so we're done
    {
        return;
    }

    size_t i = 0;
    QDomNodeList scopes = properties.item(0).childNodes(); // item(0) because there should only be ONE
    // "properties" node
    if ( scopes.count() < 1 )
    {
        qDebug( "empty ``properties'' XML tag ... bailing" );
        return;
    }

    while ( i < scopes.count() )
    {
        QDomNode curr_scope_node = scopes.item( i );

        qDebug( "found %d property node(s) for scope %s",
                curr_scope_node.childNodes().count(),
                (const char*)curr_scope_node.nodeName().utf8() );

        // DEPRECATED _getScopeProperties( curr_scope_node, project_properties );

        if ( ! project_properties[curr_scope_node.nodeName()].readXML( curr_scope_node ) )
        {
            qDebug ("%s:%d unable to read XML for property %s",
                    __FILE__, __LINE__, (const char*)curr_scope_node.nodeName().utf8() );
        }

        ++i;
    }

} // _getProperties




/**
   Get the project map units

   XML in file has this form:
     <units>feet</units>
 */
static
bool
_getMapUnits( QDomDocument const & doc )
{
    QDomNodeList nl = doc.elementsByTagName("units");

    // since "units" is a new project file type, legacy project files will not
    // have this element.  If we do have such a legacy project file missing
    // "units", then just return;
    if ( ! nl.count() )
    {
        return false;
    }

    QDomNode    node    = nl.item(0); // there should only be one, so zeroth element ok
    QDomElement element = node.toElement();

    if ( "meters" == element.text() )
    {
        QgsProject::instance()->mapUnits( QgsScaleCalculator::METERS );
    }
    else if ( "feet" == element.text() )
    {
        QgsProject::instance()->mapUnits( QgsScaleCalculator::FEET );
    }
    else if ( "degrees" == element.text() )
    {
        QgsProject::instance()->mapUnits( QgsScaleCalculator::DEGREES );
    }
    else
    {
        std::cerr << __FILE__ << ":" << __LINE__
        << " unknown map unit type " << element.text() << "\n";
        false;
    }

    return true;

} // _getMapUnits

/**
   Get the project title

   XML in file has this form:
     <qgis projectname="default project">
        <title>a project title</title>

  @todo XXX we should go with the attribute xor <title>, not both.
 */
static
void
_getTitle( QDomDocument const & doc, QString & title )
{
    QDomNodeList nl = doc.elementsByTagName("title");

    if ( ! nl.count() )
    {
        qDebug( "%s : %d %s", __FILE__ , __LINE__, " unable to find title element\n" );
        return;
    }

    QDomNode titleNode = nl.item(0); // there should only be one, so zeroth element ok

    if ( ! titleNode.hasChildNodes() ) // if not, then there's no actual text
    {
        qDebug( "%s : %d %s", __FILE__ , __LINE__, " unable to find title element\n" );
        return;
    }

    QDomNode titleTextNode = titleNode.firstChild(); // should only have one child

    if ( ! titleTextNode.isText() )
    {
        qDebug( "%s : %d %s", __FILE__ , __LINE__, " unable to find title element\n" );
        return;
    }

    QDomText titleText = titleTextNode.toText();

    title = titleText.data();

} // _getTitle




/**
   locate the qgis app object
*/
// static
// QgisApp *
// _findQgisApp()
// {
//     QgisApp * qgisApp;

//     QWidgetList  * list = QApplication::allWidgets();
//     QWidgetListIt it( *list );  // iterate over the widgets
//     QWidget * w;

//     while ( (w=it.current()) != 0 )
//     {   // for each top level widget...

//         if ( "QgisApp" == w->name() )
//         {
//             qgisApp = dynamic_cast<QgisApp*>(w);
//             break;
//         }
//         // "QgisApp" canonical name assigned in main.cpp
//         qgisApp = dynamic_cast<QgisApp*>(w->child( "QgisApp", 0, true ));

//         if ( qgisApp )
//         {
//             break;
//         }

//         ++it;
//     }
//     delete list;                // delete the list, not the widgets


//     //     if ( ! qgisApp )            // another tactic for finding qgisapp
//     //     {
//     //         if ( "QgisApp" == QApplication::mainWidget().name() )
//     //         { qgisApp = QApplication::mainWidget(); }
//     //     }

//     if( ! qgisApp )
//     {
//         qDebug( "Unable to find QgisApp" );

//         return 0x0;             // XXX some sort of error value?  Exception?
//     }

//     return qgisApp;
// } // _findQgisApp



/**
  locate a qgsMapCanvas object
*/
static QgsMapCanvas *_findMapCanvas(QString const &canonicalMapCanvasName)
{
    QgsMapCanvas *theMapCanvas;

    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);    // iterate over the widgets
    QWidget *w;

    while ((w = it.current()) != 0)
    {   // for each top level widget...
        ++it;
        theMapCanvas = dynamic_cast < QgsMapCanvas * >(w->child(canonicalMapCanvasName, 0, true));

        if (theMapCanvas)
        {
            break;
        }
    }
    delete list;    // delete the list, not the widgets

    if (theMapCanvas)
    {
        return theMapCanvas;
    }
    else
    {
        qDebug("Unable to find canvas widget " + canonicalMapCanvasName);

        return 0x0;               // XXX some sort of error value?  Exception?
    }

} // _findMapCanvas



/**
   Read map layers from project file

@note XML of form:

<maplayer type="vector" visible="1" showInOverviewFlag="0">
           <layername>Hydrop</layername>
           <datasource>/data/usgs/city_shp/hydrop.shp</datasource>
           <zorder>0</zorder>
           <provider>ogr</provider>
           <singlesymbol>
                   <renderitem>
                          <value>blabla</value>
                          <symbol>
                              <outlinecolor red="85" green="0" blue="255" />
                              <outlinestyle>SolidLine</outlinestyle>
                              <outlinewidth>1</outlinewidth>
                              <fillcolor red="0" green="170" blue="255" />
                              <fillpattern>SolidPattern</fillpattern>
                           </symbol>
                           <label>blabla</label>
                   </renderitem>
           </singlesymbol>
           <label>0</label>
           <labelattributes>
                   <label text="Label" field="" />
                   <family name="Sans Serif" field="" />
                   <size value="12" units="pt" field="" />
                   <bold on="0" field="" />
                   <italic on="0" field="" />
                   <underline on="0" field="" />
                   <color red="0" green="0" blue="0" field="" />
                   <x field="" />
                   <y field="" />
                   <offset  units="pt" x="0" xfield="" y="0" yfield="" />
                   <angle value="0" field="" />
                   <alignment value="center" field="" />
           </labelattributes>
</maplayer>

*/
static
bool
_getMapLayers( QDomDocument const & doc )
{
    // Layer order is implicit in the order they are stored in the project file

    QDomNodeList nl = doc.elementsByTagName("maplayer");

    // XXX what is this used for? QString layerCount( QString::number(nl.count()) );

    QString wk;

    // process the map layer nodes

    if ( 0 == nl.count() )      // if we have no layers to process, bail
    {
        return false;
    }

    for (size_t i = 0; i < nl.count(); i++)
    {
        QDomNode    node    = nl.item(i);
        QDomElement element = node.toElement();

        QString type = element.attribute("type");


        QgsMapLayer * mapLayer;
#ifdef QGISDEBUG

        std::cerr << "type is " << type << std::endl;
#endif

        if (type == "vector")
        {
            mapLayer = new QgsVectorLayer;
        }
        else if (type == "raster")
        {
            mapLayer = new QgsRasterLayer;
        }

        Q_CHECK_PTR( mapLayer );

        if ( ! mapLayer )
        {
#ifdef QGISDEBUG
            std::cerr << __FILE__ << " : " << __LINE__
            << " unable to create layer\n";
#endif

            return false;
        }

        // have the layer restore state that is stored in DOM node
        mapLayer->readXML( node );

        mapLayer = QgsMapLayerRegistry::instance()->addMapLayer( mapLayer );

        // XXX kludge for ensuring that overview canvas updates happen correctly;
        // XXX eventually this should be replaced by mechanism whereby overview
        // XXX canvas implicitly knows about all new layers
        //         if ( mapLayer )         // if successfully added to registry
        //         {
        //             // find canonical Qgis application object
        //             QgisApp * qgisApp = _findQgisApp();

        //             // make connection
        //             if ( qgisApp )
        //             {
        //                 QObject::connect(mapLayer,
        //                                  SIGNAL(showInOverview(QString,bool)),
        //                                  qgisApp,
        //                                  SLOT(setLayerOverviewStatus(QString,bool)));
        //             }
        //         }

    }

    return true;

} // _getMapLayers




/**
   Sets the given canvas' extents

   @param canonicalName will be "theMapCanvas" or "theOverviewCanvas"; these
   are set when those are created in qgisapp ctor
 */
static
void
_setCanvasExtent( QString const & canonicalMapCanvasName, QgsRect const & newExtent )
{
    // first find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas(canonicalMapCanvasName);

    if( ! theMapCanvas )
    {
        qDebug( "Unable to find canvas widget " + canonicalMapCanvasName );

        return;                 // XXX some sort of error value?  Exception?
    }

    theMapCanvas->setExtent( newExtent );

    // XXX sometimes the canvases are frozen here, sometimes not; this is a
    // XXX worrisome inconsistency; regardless, unfreeze the canvases to ensure
    // XXX a redraw
    theMapCanvas->freeze( false );

} // _setCanvasExtent()



/**
   Get the full extent for the given canvas.

   This is used to get the full extent of the main map canvas so that we can
   set the overview canvas to that instead of stupidly setting the overview
   canvas to the *same* extent that's in the main map canvas.

   @param canonicalMapCanvasName will be "theMapCanvas" or "theOverviewCanvas"; these
   are set when those are created in qgisapp ctor

 */
static
QgsRect _getFullExtent( QString const & canonicalMapCanvasName )
{
    // XXX since this is a cut-n-paste from above, maybe generalize to a
    // XXX separate function?
    // first find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas(canonicalMapCanvasName);

    if( ! theMapCanvas )
    {
        qDebug( "Unable to find canvas widget " + canonicalMapCanvasName );

        return QgsRect();       // XXX some sort of error value?  Exception?
    }


    return theMapCanvas->fullExtent();

} // _getFullExtent( QString const & canonicalMapCanvasName )






/**
   Get the  extent for the given canvas.

   This is used to get the  extent of the main map canvas so that we can
   set the overview canvas to that instead of stupidly setting the overview
   canvas to the *same* extent that's in the main map canvas.

   @param canonicalMapCanvasName will be "theMapCanvas" or "theOverviewCanvas"; these
   are set when those are created in qgisapp ctor

 */
static
QgsRect _getExtent( QString const & canonicalMapCanvasName )
{
    // XXX since this is a cut-n-paste from above, maybe generalize to a
    // XXX separate function?
    // first find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas(canonicalMapCanvasName);

    if( ! theMapCanvas )
    {
        qDebug( "Unable to find canvas widget " + canonicalMapCanvasName );

        return QgsRect();       // XXX some sort of error value?  Exception?
    }


    return theMapCanvas->extent();

} // _getExtent( QString const & canonicalMapCanvasName )




/**
   Please note that most of the contents were copied from qgsproject
*/
bool
QgsProject::read( QFileInfo const & file )
{
    imp_->file.setName( file.filePath() );

    return read();
} // QgsProject::read( QFile & file )



/**
   @note it's presumed that the caller has already reset the map canvas, map registry, and legend
*/
bool
QgsProject::read( )
{
    std::auto_ptr<QDomDocument> doc =
        std::auto_ptr<QDomDocument>(new QDomDocument("qgis"));

    if ( ! imp_->file.open(IO_ReadOnly) )
    {
        imp_->file.close();     // even though we got an error, let's make
        // sure it's closed anyway

        throw QgsIOException( "Unable to open " + imp_->file.name() );

        return false;           // XXX raise exception? Ok now superfluous
        // XXX because of exception.
    }

    // location of problem associated with errorMsg
    int line, column;
    QString errorMsg;

    if ( ! doc->setContent(&imp_->file, &errorMsg, &line, &column) )
    {
        // want to make this class as GUI independent as possible; so commented out
        //         QMessageBox::critical( 0x0, "Project File Read Error",
        //                                errorMsg + " at line " + QString::number( line ) +
        //                                " column " + QString::number( column ) );

        QString errorString = "Project file read error" +
                              errorMsg + " at line " + QString::number( line ) +
                              " column " + QString::number( column );

        qDebug( (const char*)errorString.utf8() );

        imp_->file.close();

        throw QgsException( errorString + " for file " + imp_->file.name() );

        return false;           // XXX superfluous because of exception
    }

    imp_->file.close();


#ifdef QGISDEBUG

    qWarning("opened document " + imp_->file.name());
#endif

    // before we start loading everything, let's clear out the current set of
    // properties first so that we don't have the properties from the previous
    // project still hanging around

    imp_->clear();


    // first get the map layers
    if ( ! _getMapLayers( *doc ) )
    {
#ifdef QGISDEBUG
        qDebug( "Unable to get map layers from project file." );
#endif

        throw QgsException( "Cannot get map layers from " + imp_->file.name() );

        return false;
    }

    // restore the canvas' area of interest

    // restor the area of interest, or extent
    QgsRect savedExtent;

    if ( ! _getExtents( *doc, savedExtent ) )
    {
#ifdef QGISDEBUG
        qDebug( "Unable to get extents from project file." );
#endif

        throw QgsException( "Cannot get extents from " + imp_->file.name() );

        return false;
    }

    // now restore the extent for the main canvas

    _setCanvasExtent( "theMapCanvas", savedExtent );

    // ensure that overview map canvas is set to *entire* extent
    QgsRect mapCanvasFullExtent =  _getFullExtent( "theMapCanvas" );
    _setCanvasExtent( "theOverviewCanvas", mapCanvasFullExtent );


    // now get project title
    _getTitle( *doc, imp_->title );
#ifdef QGISDEBUG

    qDebug( "Project title: " + imp_->title );
#endif

    // now set the map units; note, alters QgsProject::instance().
    _getMapUnits( *doc );

    // now get any properties
    _getProperties( *doc, imp_->properties_ );

    qDebug( "%s:%d %d properties read", __FILE__, __LINE__, imp_->properties_.count() );

    dump_( imp_->properties_ );


    // can't be dirty since we're allegedly in pristine state
    dirty( false );

    return true;

} // QgsProject::read



bool
QgsProject::write( QFileInfo const & file )
{
    imp_->file.setName( file.filePath() );

    return write();
} // QgsProject::write( QFileInfo const & file )


bool
QgsProject::write( )
{
    // if we have problems creating or otherwise writing to the project file,
    // let's find out up front before we go through all the hand-waving
    // necessary to create all the DOM objects
    if ( ! imp_->file.open( IO_WriteOnly | IO_Translate | IO_Truncate ) )
    {
        imp_->file.close();     // even though we got an error, let's make
        // sure it's closed anyway

        throw QgsIOException( "Unable to open " + imp_->file.name() );

        return false;           // XXX raise exception? Ok now superfluous
        // XXX because of exception.
    }

    QDomImplementation DOMImplementation;

    QDomDocumentType documentType = DOMImplementation.createDocumentType("qgis","http://mrcc.com/qgis.dtd","SYSTEM");
    std::auto_ptr<QDomDocument> doc =
        std::auto_ptr<QDomDocument>( new QDomDocument( documentType ) );


    QDomElement qgisNode = doc->createElement( "qgis" );
    qgisNode.setAttribute( "projectname", title() );

    doc->appendChild( qgisNode );

    // title
    QDomElement titleNode = doc->createElement( "title" );
    qgisNode.appendChild( titleNode );

    QDomText titleText = doc->createTextNode( title() ); // XXX why have title TWICE?
    titleNode.appendChild( titleText );

    // units

    QDomElement unitsNode = doc->createElement( "units" );
    qgisNode.appendChild( unitsNode );

    QString unitsString;

    switch (instance()->imp_->mapUnits)
    {
    case QgsScaleCalculator::METERS :
        unitsString = "meters";
        break;
    case QgsScaleCalculator::FEET :
        unitsString = "feet";
        break;
    case QgsScaleCalculator::DEGREES :
        unitsString = "degrees";
        break;
    default :
        unitsString = "unknown";
        break;
    }

    QDomText unitsText = doc->createTextNode( unitsString );
    unitsNode.appendChild( unitsText );

    // extents and layers info are written by the map canvas
    // find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas( "theMapCanvas" );
    theMapCanvas->writeXML(qgisNode, *doc);

    if( ! theMapCanvas )
    {
        qDebug( "Unable to find canvas widget theMapCanvas" );

        return false;                 // XXX some sort of error value?  Exception?
    }

    // now add the optional extra properties

    dump_( imp_->properties_ );

    qDebug( "there are %d property scopes", imp_->properties_.count() );

    if ( ! imp_->properties_.empty() ) // only worry about properties if we
        // actually have any
    {
        // <properties>
        QDomElement propertiesElement = doc->createElement( "properties" );
        qgisNode.appendChild( propertiesElement );

        for ( QMap< QString, PropertyKey >::iterator curr_scope =
                    imp_->
                    properties_.begin();
                curr_scope != imp_->properties_.end();
                curr_scope++ )
        {
            qDebug( "scope ``%s'' has %d entries", (const char*)curr_scope.key().utf8(), curr_scope.data().count() );

            // <$scope>
            if ( ! curr_scope.data().writeXML( curr_scope.key(), propertiesElement, *doc ) )
            {
                qDebug ( "%s:%d error create property %s's DOM objects",
                         __FILE__, __LINE__, (const char*)curr_scope.key().utf8() );
            }
            // </$scope>

        } // </properties>

    } // if any properties

    // now wrap it up and ship it to the project file


    doc->normalize();           // XXX I'm not entirely sure what this does

    QString xml = doc->toString( 4 ); // write to string with indentation of four characters
    // (yes, four is arbitrary)

    // const char * xmlString = xml; // debugger probe point
    // qDebug( "project file output:\n\n" + xml );

    QTextStream projectFileStream( &imp_->file );

    projectFileStream << xml << endl;

    imp_->file.close();


    dirty( false );             // reset to pristine state

    return true;
} // QgsProject::write



void
QgsProject::clearProperties()
{
#ifdef QGISDEBUG
    std::cout << "Clearing project properties QgsProject::clearProperties()" << std::endl;
#endif
    imp_->clear();

    dirty( true );
} // QgsProject::clearProperties()




bool
QgsProject::writeEntry ( QString const & scope, const QString & key, bool value )
{
    QStringList keyTokens = QStringList::split( '/', key );

    dirty( true );

    return imp_->properties_[scope].setValue( keyTokens , value );
} // QgsProject::writeEntry ( ..., bool value )


bool
QgsProject::writeEntry ( QString const & scope, const QString & key, double value )
{
    QStringList keyTokens = QStringList::split( '/', key );

    dirty( true );

    return imp_->properties_[scope].setValue( keyTokens, value );
} // QgsProject::writeEntry ( ..., double value )


bool
QgsProject::writeEntry ( QString const & scope, const QString & key, int value )
{
    QStringList keyTokens = QStringList::split( '/', key );

    dirty( true );

    return imp_->properties_[scope].setValue( keyTokens , value );
} // QgsProject::writeEntry ( ..., int value )


bool
QgsProject::writeEntry ( QString const & scope, const QString & key, const QString & value )
{
    QStringList keyTokens = QStringList::split( '/', key );

    dirty( true );

    return imp_->properties_[scope].setValue( keyTokens , value );
} // QgsProject::writeEntry ( ..., const QString & value )


bool
QgsProject::writeEntry ( QString const & scope, const QString & key, const QStringList & value )
{
    QStringList keyTokens = QStringList::split( '/', key );

    dirty( true );

    return imp_->properties_[scope].setValue( keyTokens , value );
} // QgsProject::writeEntry ( ..., const QStringList & value )




QStringList
QgsProject::readListEntry ( QString const & scope, const QString & key, bool * ok ) const
{
    QStringList keyTokens = QStringList::split( '/', key );

    QVariant value = imp_->properties_[scope].value( keyTokens );

    bool valid = QVariant::StringList == value.type();

    if ( ok )
    {
        *ok = valid;
    }

    if ( valid )
    {
        return value.asStringList();
    }

    return QStringList();
} // QgsProject::readListEntry


QString
QgsProject::readEntry ( QString const & scope,
                        const QString & key,
                        const QString & def,
                        bool * ok ) const
{
    QStringList keyTokens = QStringList::split( '/', key );

    QVariant value = imp_->properties_[scope].value( keyTokens );

    bool valid = value.canCast( QVariant::String );

    if ( ok )
    {
        *ok = valid;
    }

    if ( valid )
    {
        return value.toString();
    }

    return QString( def );
} // QgsProject::readEntry


int
QgsProject::readNumEntry ( QString const & scope,
                           const QString & key,
                           int def,
                           bool * ok ) const
{
    QStringList keyTokens = QStringList::split( '/', key );

    QVariant value = imp_->properties_[scope].value( keyTokens );

    bool valid = value.canCast( QVariant::String );

    if ( ok )
    {
        *ok = valid;
    }

    if ( valid )
    {
        return value.toInt();
    }

    return def;
} // QgsProject::readNumEntry


double
QgsProject::readDoubleEntry ( QString const & scope,
                              const QString & key,
                              double def,
                              bool * ok ) const
{
    QStringList keyTokens = QStringList::split( '/', key );

    QVariant value = imp_->properties_[scope].value( keyTokens );

    bool valid = value.canCast( QVariant::Double );

    if ( ok )
    {
        *ok = valid;
    }

    if ( valid )
    {
        return value.toDouble();
    }

    return def;
} // QgsProject::readDoubleEntry


bool
QgsProject::readBoolEntry ( QString const & scope,
                            const QString & key,
                            bool def,
                            bool * ok ) const
{
    QStringList keyTokens = QStringList::split( '/', key );

    QVariant value = imp_->properties_[scope].value( keyTokens );

    bool valid = value.canCast( QVariant::Bool );

    if ( ok )
    {
        *ok = valid;
    }

    if ( valid )
    {
        return value.toBool();
    }

    return def;
} // QgsProject::readBoolEntry


bool
QgsProject::removeEntry ( QString const & scope, const QString & key )
{
    QStringList keyTokens = QStringList::split( '/', key );

    dirty( true );

    return imp_->properties_[scope].remove( keyTokens );
} // QgsProject::removeEntry



QStringList
QgsProject::entryList( QString const & scope, QString const & key ) const
{
    QStringList keyTokens = QStringList::split( '/', key );

    QStringList entries;
    
    imp_->properties_[scope].entryList( keyTokens, entries );

    return entries;
} // QgsProject::entryList
