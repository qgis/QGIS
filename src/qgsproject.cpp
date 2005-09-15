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

#include "qgsrect.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgsmaplayerregistry.h"
#include "qgsexception.h"
#include "qgsprojectproperty.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qdict.h>
#include <qmessagebox.h>
#include <qwidgetlist.h>
#include <qglobal.h>



static const char *const ident_ = "$Id$";





// / canonical project instance
QgsProject * QgsProject::theProject_;







/**
    Take the given scope and key and convert them to a string list of key
    tokens that will be used to navigate through a Property hierarchy

    E.g., scope "someplugin" and key "/foo/bar/baz" will become a string list
    of { "properties", "someplugin", "foo", "bar", "baz" }.  "properties" is
    always first because that's the permanent ``root'' Property node.
 */
 static 
 QStringList makeKeyTokens_(QString const &scope, QString const &key)
 {
     const char * scope_str = scope.ascii(); // debugger probes
     const char * key_str   = key.ascii();

     QStringList keyTokens = QStringList(scope);
     keyTokens += QStringList::split('/', key);

     // be sure to include the canonical root node
     keyTokens.push_front( "properties" );

     return keyTokens;
 } // makeKeyTokens_




 /**
    return the property that matches the given key sequence, if any

    @param sequence is a tokenized key sequence
    @param p is likely to be the top level QgsPropertyKey in QgsProject:e:Imp.

    @return null if not found, otherwise located Property
 */
 static
 QgsProperty * findKey_( QString const & scope, 
                         QString const & key, 
                         QgsPropertyKey &   rootProperty )
 {
     QgsPropertyKey * currentProperty = &rootProperty;
     QgsProperty * nextProperty;           // link to next property down hiearchy

     QStringList keySequence = makeKeyTokens_( scope, key );

     while ( ! keySequence.isEmpty() )
     {
         // debugger probes
         const char * currentPropertyName = currentProperty->name().ascii();
         const char * keySequenceFirst    = keySequence.first().ascii();

         // if the current head of the sequence list matches the property name,
         // then traverse down the property hierarchy
         if ( keySequence.first() == currentProperty->name() )
         {
             // remove front key since we're traversing down a level
             keySequence.pop_front();

             // if we have only one key name left, then return the key found
             if ( 1 == keySequence.count() )
             {
                 return currentProperty->find( keySequence.front() );
                 
             }
             // if we're out of keys then the current property is the one we
             // want; i.e., we're in the rate case of being at the top-most
             // property node
             else if ( keySequence.isEmpty() )
             {
                 return currentProperty;
             }
             else if ( nextProperty = currentProperty->find( keySequence.first() ) )
             {
                 keySequenceFirst    = keySequence.first().ascii();

                 if ( nextProperty->isKey() )
                 {
                     currentProperty = dynamic_cast<QgsPropertyKey*>(nextProperty);
                 }
                 // it may be that this may be one of several property value
                 // nodes keyed by QDict string; if this is the last remaining
                 // key token and the next property is a value node, then
                 // that's the situation, so return the currentProperty
                 else if ( nextProperty->isValue() && (1 == keySequence.count()) )
                 {
                     return currentProperty;
                 }
                 else            // QgsPropertyValue not Key, so return null
                 {
                     return 0x0;
                 }
             }
             else                // if the next key down isn't found
             {                   // then the overall key sequence doesn't exist
                 return 0x0;
             }
         }
         else
         {
             return 0x0;
         }
     }

     return 0x0;
 } // findKey_



 /** add the given key and value

 @param sequence is list of keys
 @param rootProperty is the property from which to start adding
 @param value the value associated with the key
 */
 static
 QgsProperty * addKey_( QString const & scope,
                        QString const & key, 
                        QgsPropertyKey * rootProperty,
                        QVariant value )
 {
     QStringList keySequence = makeKeyTokens_( scope, key );

     // cursor through property key/value hierarchy
     QgsPropertyKey * currentProperty = rootProperty;

     QgsProperty * newProperty; // link to next property down hiearchy

     while ( ! keySequence.isEmpty() )
     {
         // debugger probes
         const char * currentPropertyName = currentProperty->name().ascii();
         const char * keySeqeunceFirst    = keySequence.first().ascii();

         // if the current head of the sequence list matches the property name,
         // then traverse down the property hierarchy
         if ( keySequence.first() == currentProperty->name() )
         {
             // remove front key since we're traversing down a level
             keySequence.pop_front();

             // if key sequence has one last element, then we use that as the
             // name to store the value
             if ( 1 == keySequence.count() )
             {
                 currentProperty->setValue( keySequence.front(), value );
                 return currentProperty;
             }
             // we're at the top element if popping the keySequence element
             // will leave it empty; in that case, just add the key
             else if ( keySequence.isEmpty() )
             {
                 currentProperty->setValue( value );

                 return currentProperty;
             }
             else if ( newProperty = currentProperty->find( keySequence.first() ) )
             {
                 currentProperty = dynamic_cast<QgsPropertyKey*>(newProperty);

                 if ( currentProperty )
                 {
                     continue;
                 }
                 else            // QgsPropertyValue not Key, so return null
                 {
                     return 0x0;
                 }
             }
             else                // the next subkey doesn't exist, so add it
             {
                 newProperty = currentProperty->addKey( keySequence.first() );

                 if ( newProperty )
                 {
                     currentProperty = dynamic_cast<QgsPropertyKey*>(newProperty);
                 }
                 continue;
             }
         }
         else
         {
             return 0x0;
         }
     }

     return 0x0;

 } // addKey_



 static
 void removeKey_(QString const & scope,
                 QString const & key, 
                 QgsPropertyKey & rootProperty)
 {
     QgsPropertyKey * currentProperty = &rootProperty;

     QgsProperty * nextProperty;   // link to next property down hiearchy
     QgsPropertyKey * previousQgsPropertyKey; // link to previous property up hiearchy

     QStringList keySequence = makeKeyTokens_( scope, key );

     while ( ! keySequence.isEmpty() )
     {
         // debugger probes
         const char * currentPropertyName = currentProperty->name().ascii();
         const char * keySequenceFirst    = keySequence.first().ascii();

         // if the current head of the sequence list matches the property name,
         // then traverse down the property hierarchy
         if ( keySequence.first() == currentProperty->name() )
         {
             // remove front key since we're traversing down a level
             keySequence.pop_front();

             // if we have only one key name left, then try to remove the key
             // with that name
             if ( 1 == keySequence.count() )
             {
                 currentProperty->removeKey( keySequence.front() );
             }
             // if we're out of keys then the current property is the one we
             // want to remove, but we can't delete it directly; we need to
             // delete it from the parent property key container
             else if ( keySequence.isEmpty() )
             {
                 previousQgsPropertyKey->removeKey( currentProperty->name() );
             }
             else if ( nextProperty = currentProperty->find( keySequence.first() ) )
             {
                 keySequenceFirst    = keySequence.first().ascii();

                 previousQgsPropertyKey = currentProperty;
                 currentProperty = dynamic_cast<QgsPropertyKey*>(nextProperty);

                 if ( currentProperty )
                 {
                     continue;
                 }
                 else            // QgsPropertyValue not Key, so return null
                 {
                     return;
                 }
             }
             else                // if the next key down isn't found
             {                   // then the overall key sequence doesn't exist
                 return;
             }
         }
         else
         {
             return;
         }
     }

 } // void removeKey_



 struct QgsProject::Imp
 {
     /// current physical project file
     QFile file;

     /// property hierarchy
     QgsPropertyKey properties_;

     /// project title
     QString title;

     /// true if project has been modified since it has been read or saved
     bool dirty;

     /// map units for current project
     QGis::units mapUnits;

     Imp()
         : title(""), 
           dirty(false), 
           mapUnits(QGis::METERS)
     {                             // top property node is the root
                                   // "properties" that contains all plug-in
                                   // and extra property keys and values
         properties_.name() = "properties"; // root property node always this value
     }

     /** clear project properties when a new project is started 
      */ 
     void clear()
     {
 #ifdef QGISDEBUG
         std::cout << "Clearing project properties Impl->clear();" << std::endl;
 #endif
         properties_.clearKeys();
         mapUnits = QGis::METERS;
         title = "";

         // reset some default project properties
         // XXX THESE SHOULD BE MOVED TO STATUSBAR RELATED SOURCE
         QgsProject::instance()->writeEntry("PositionPrecision", "/Automatic", true);
         QgsProject::instance()->writeEntry("PositionPrecision", "/DecimalPlaces", 2);
     }

 };  // struct QgsProject::Imp



 QgsProject::QgsProject()
     : imp_(new QgsProject::Imp)
 {
     // Set some default project properties
     // XXX THESE SHOULD BE MOVED TO STATUSBAR RELATED SOURCE
     writeEntry("PositionPrecision", "/Automatic", true);
     writeEntry("PositionPrecision", "/DecimalPlaces", 2);
     // XXX writeEntry() makes the project dirty, but it doesn't make sense
     // for a new project to be dirty, so let's clean it up
     dirty(false);
 } // QgsProject ctor



 QgsProject::~QgsProject()
 {
     // note that std::auto_ptr automatically deletes imp_ when it's destroyed
 } // QgsProject dtor



 QgsProject * QgsProject::instance()
 {
     if (!QgsProject::theProject_)
     {
         QgsProject::theProject_ = new QgsProject;
     }

     return QgsProject::theProject_;
 } // QgsProject * instance()




 void QgsProject::title(QString const &title)
 {
     imp_->title = title;

     dirty(true);
 } // void QgsProject::title


 QString const & QgsProject::title() const
 {
     return imp_->title;
 } // QgsProject::title() const


 QGis::units QgsProject::mapUnits() const
 {
     return imp_->mapUnits;
 } // QGis::units QgsProject::mapUnits() const



 void QgsProject::mapUnits(QGis::units u)
 {
     imp_->mapUnits = u;

     dirty(true);
 } // void QgsProject::mapUnits(QGis::units u)


 bool QgsProject::dirty() const
 {
     return imp_->dirty;
 } // bool QgsProject::dirty() 


 void QgsProject::dirty(bool b)
 {
     imp_->dirty = b;
 } // bool QgsProject::dirty()



 void QgsProject::filename(QString const &name)
 {
     imp_->file.setName(name);

     dirty(true);
 } // void QgsProject::filename( QString const & name )



 QString QgsProject::filename() const
 {
     return imp_->file.name();
 } // QString QgsProject::filename() const



 /// basically a debugging tool to dump property list values 
 static void dump_( QgsPropertyKey const & topQgsPropertyKey )
 {
     qDebug("current properties:");

     topQgsPropertyKey.dump();
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
 static bool _getExtents(QDomDocument const &doc, QgsRect & aoi)
 {
     QDomNodeList extents = doc.elementsByTagName("extent");

     if (extents.count() > 1)
     {
         qDebug
             ("there appears to be more than one extent in the project\nusing first one");
     } else if (extents.count() < 1) // no extents found, so bail
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
     aoi.setXmin(xmin);

     exElement = yminNode.toElement();
     double ymin = exElement.text().toDouble();
     aoi.setYmin(ymin);

     exElement = xmaxNode.toElement();
     double xmax = exElement.text().toDouble();
     aoi.setXmax(xmax);

     exElement = ymaxNode.toElement();
     double ymax = exElement.text().toDouble();
     aoi.setYmax(ymax);

     return true;

 }                               // _getExtents



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

 @param project_properties should be the top QgsPropertyKey node.

 */
static 
void
_getProperties(QDomDocument const &doc, QgsPropertyKey & project_properties)
{
    QDomNodeList properties = doc.elementsByTagName("properties");

    if (properties.count() > 1)
    {
        qDebug("there appears to be more than one ``properties'' XML tag ... bailing");
        return;
    } else if (properties.count() < 1)  // no properties found, so we're done
    {
        return;
    }

    // item(0) because there should only be ONE
    // "properties" node
    QDomNodeList scopes = properties.item(0).childNodes();

    if (scopes.count() < 1)
    {
        qDebug("empty ``properties'' XML tag ... bailing");
        return;
    }

    QDomNode propertyNode = properties.item(0);

    if ( ! project_properties.readXML( propertyNode ) )
    {
        qDebug("%s:%d project_properties.readXML() failed");
    }

// DEPRECATED as functionality has been shoved down to QgsProperyKey::readXML()

//     size_t i = 0;
//     while (i < scopes.count())
//     {
//         QDomNode curr_scope_node = scopes.item(i);

//         qDebug("found %d property node(s) for scope %s",
//                curr_scope_node.childNodes().count(),
//                (const char *) curr_scope_node.nodeName().utf8());

//         QString key(curr_scope_node.nodeName());

//         QgsPropertyKey * currentKey = 
//             dynamic_cast<QgsPropertyKey*>(project_properties.find( key ));

//         if ( ! currentKey )
//         {
//             // if the property key doesn't yet exist, create an empty instance
//             // of that key

//             currentKey = project_properties.addKey( key );

//             if ( ! currentKey )
//             {
//                 qDebug( "%s:%d unable to add key", __FILE__, __LINE__ );
//             }
//         }

//         if (! currentKey->readXML(curr_scope_node))
//         {
//             qDebug("%s:%d unable to read XML for property %s", __FILE__, __LINE__,
//                    (const char *) curr_scope_node.nodeName().utf8());
//         }

//         ++i;
//     }
} // _getProperties



/**
   Get the project map units

   XML in file has this form:
   <units>feet</units>
*/
static bool _getMapUnits(QDomDocument const &doc)
{
    QDomNodeList nl = doc.elementsByTagName("units");

    // since "units" is a new project file type, legacy project files will not
    // have this element.  If we do have such a legacy project file missing
    // "units", then just return;
    if (!nl.count())
    {
        return false;
    }

    QDomNode node = nl.item(0);   // there should only be one, so zeroth element ok
    QDomElement element = node.toElement();

    if ("meters" == element.text())
    {
        QgsProject::instance()->mapUnits(QGis::METERS);
    } else if ("feet" == element.text())
    {
        QgsProject::instance()->mapUnits(QGis::FEET);
    } else if ("degrees" == element.text())
    {
        QgsProject::instance()->mapUnits(QGis::DEGREES);
    } else if ("unknown" == element.text())
    {
        QgsProject::instance()->mapUnits(QGis::UNKNOWN);
    } else
    {
        std::
            cerr << __FILE__ << ":" << __LINE__ << " unknown map unit type " <<
            element.text().local8Bit() << "\n";
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
static void _getTitle(QDomDocument const &doc, QString & title)
{
    QDomNodeList nl = doc.elementsByTagName("title");

    if (!nl.count())
    {
        qDebug("%s : %d %s", __FILE__, __LINE__, " unable to find title element\n");
        return;
    }

    QDomNode titleNode = nl.item(0);  // there should only be one, so zeroth element ok

    if (!titleNode.hasChildNodes()) // if not, then there's no actual text
    {
        qDebug("%s : %d %s", __FILE__, __LINE__, " unable to find title element\n");
        return;
    }

    QDomNode titleTextNode = titleNode.firstChild();  // should only have one child

    if (!titleTextNode.isText())
    {
        qDebug("%s : %d %s", __FILE__, __LINE__, " unable to find title element\n");
        return;
    }

    QDomText titleText = titleTextNode.toText();

    title = titleText.data();

} // _getTitle



/**
   locate a qgsMapCanvas object
*/
static QgsMapCanvas * _findMapCanvas(QString const &canonicalMapCanvasName)
{
    QgsMapCanvas * theMapCanvas = 0x0;

// TODO: Need to refactor for Qt4 - uses QWidgetList only (no pointer-to)
#if QT_VERSION < 0x040000
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);      // iterate over the widgets
    QWidget *w;

    while ((w = it.current()) != 0)
    {                             // for each top level widget...
        ++it;
        theMapCanvas =
            dynamic_cast <QgsMapCanvas *>(w->child(canonicalMapCanvasName, 0, true));

        if (theMapCanvas)
        {
            break;
        }
    }

    delete list;                  // delete the list, not the widgets
#endif

    if (theMapCanvas)
    {
        return theMapCanvas;
    } else
    {
        qDebug("Unable to find canvas widget " + canonicalMapCanvasName);

        return 0x0;                 // XXX some sort of error value? Exception?
    }

} // _findMapCanvas



/**
   Read map layers from project file

   
   @returns pair of < bool, list<QDomNode> >; bool is true if function worked;
            else is false.  list contains nodes corresponding to layers that couldn't
            be loaded

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
static pair< bool, list<QDomNode> > _getMapLayers(QDomDocument const &doc)
{
    // Layer order is implicit in the order they are stored in the project file

    QDomNodeList nl = doc.elementsByTagName("maplayer");

    // XXX what is this used for? QString layerCount( QString::number(nl.count()) );

    QString wk;

    list<QDomNode> brokenNodes; // a list of DOM nodes corresponding to layers
                                // that we were unable to load; this could be
                                // because the layers were removed or
                                // re-located after the project was last saved

    // process the map layer nodes

    if (0 == nl.count())        // if we have no layers to process, bail
    {
        return make_pair(true, brokenNodes); // Decided to return "true" since it's
                                // possible for there to be a project with no
                                // layers; but also, more imporantly, this
                                // would cause the tests/qgsproject to fail
                                // since the test suite doesn't currently
                                // support test layers
    }

    bool returnStatus = true;

    for (size_t i = 0; i < nl.count(); i++)
    {
        QDomNode node = nl.item(i);
        QDomElement element = node.toElement();

        QString type = element.attribute("type");


        QgsMapLayer *mapLayer;
#ifdef QGISDEBUG

        std::cerr << "type is " << type.local8Bit() << std::endl;
#endif

        if (type == "vector")
        {
            mapLayer = new QgsVectorLayer;
        } else if (type == "raster")
        {
            mapLayer = new QgsRasterLayer;
        }

        Q_CHECK_PTR(mapLayer);

        if (!mapLayer)
        {
#ifdef QGISDEBUG
            std::cerr << __FILE__ << " : " << __LINE__ << " unable to create layer\n";
#endif
            return make_pair(false, brokenNodes);
        }

        // have the layer restore state that is stored in DOM node
        if ( mapLayer->readXML(node) )
        {
            mapLayer = QgsMapLayerRegistry::instance()->addMapLayer(mapLayer);
        }
        else
        {
            delete mapLayer;

            qDebug( "%s:%d unable to load %s layer", __FILE__, __LINE__, type.ascii() );

            returnStatus = false; // flag that we had problems loading layers

            brokenNodes.push_back( node );
        }
    }

    return make_pair(returnStatus, brokenNodes);

} // _getMapLayers




/**
   Sets the given canvas' extents

   @param canonicalName will be "theMapCanvas" or "theOverviewCanvas"; these
   are set when those are created in qgisapp ctor
*/
static void _setCanvasExtent(QString const &canonicalMapCanvasName,
                             QgsRect const &newExtent)
{
    // first find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas(canonicalMapCanvasName);

    if (!theMapCanvas)
    {
        qDebug("Unable to find canvas widget " + canonicalMapCanvasName);

        return;                     // XXX some sort of error value? Exception?
    }

    theMapCanvas->setExtent(newExtent);

    // XXX sometimes the canvases are frozen here, sometimes not; this is a
    // XXX worrisome inconsistency; regardless, unfreeze the canvases to ensure
    // XXX a redraw
    theMapCanvas->freeze(false);

}                               // _setCanvasExtent()



/**
   Get the full extent for the given canvas.

   This is used to get the full extent of the main map canvas so that we can
   set the overview canvas to that instead of stupidly setting the overview
   canvas to the *same* extent that's in the main map canvas.

   @param canonicalMapCanvasName will be "theMapCanvas" or "theOverviewCanvas"; these
   are set when those are created in qgisapp ctor

*/
static QgsRect _getFullExtent(QString const &canonicalMapCanvasName)
{
    // XXX since this is a cut-n-paste from above, maybe generalize to a
    // XXX separate function?
    // first find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas(canonicalMapCanvasName);

    if (!theMapCanvas)
    {
        qDebug("Unable to find canvas widget " + canonicalMapCanvasName);

        return QgsRect();           // XXX some sort of error value? Exception?
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
static QgsRect _getExtent(QString const &canonicalMapCanvasName)
{
    // XXX since this is a cut-n-paste from above, maybe generalize to a
    // XXX separate function?
    // first find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas(canonicalMapCanvasName);

    if (!theMapCanvas)
    {
        qDebug("Unable to find canvas widget " + canonicalMapCanvasName);

        return QgsRect();           // XXX some sort of error value? Exception?
    }


    return theMapCanvas->extent();

} // _getExtent( QString const & canonicalMapCanvasName )




/**
   Please note that most of the contents were copied from qgsproject
*/
bool QgsProject::read(QFileInfo const &file)
{
    imp_->file.setName(file.filePath());

    return read();
} // QgsProject::read( QFile & file )



/**
   @note it's presumed that the caller has already reset the map canvas, map
   registry, and legend
*/
bool QgsProject::read()
{
    std::auto_ptr< QDomDocument > doc =
        std::auto_ptr < QDomDocument > (new QDomDocument("qgis"));

    if (!imp_->file.open(IO_ReadOnly))
    {
        imp_->file.close();     // even though we got an error, let's make
                                // sure it's closed anyway

        throw QgsIOException("Unable to open " + imp_->file.name());

        return false;           // XXX raise exception? Ok now superfluous
                                // XXX because of exception.
    }

    // location of problem associated with errorMsg
    int line, column;
    QString errorMsg;

    if (!doc->setContent(&imp_->file, &errorMsg, &line, &column))
    {
        // want to make this class as GUI independent as possible; so commented out
        // QMessageBox::critical( 0x0, "Project File Read Error",
        // errorMsg + " at line " + QString::number( line ) +
        // " column " + QString::number( column ) );

        QString errorString = QObject::tr("Project file read error") +
            errorMsg + QObject::tr(" at line ") + QString::number(line) + QObject::tr(" column ") +
            QString::number(column);

        qDebug((const char *) errorString.utf8());

        imp_->file.close();

        throw QgsException(errorString + QObject::tr(" for file ") + imp_->file.name());

        return false;               // XXX superfluous because of exception
    }

    imp_->file.close();


#ifdef QGISDEBUG
    qWarning("opened document " + imp_->file.name());
#endif

    // before we start loading everything, let's clear out the current set of
    // properties first so that we don't have the properties from the previous
    // project still hanging around

    imp_->clear();

    // now get any properties
    _getProperties(*doc, imp_->properties_);

    qDebug("%s:%d %d properties read", __FILE__, __LINE__,
           imp_->properties_.count());

    dump_(imp_->properties_);


    // restore the canvas' area of interest

    // now get project title
    _getTitle(*doc, imp_->title);

#ifdef QGISDEBUG
    qDebug("Project title: " + imp_->title);
#endif

    // now set the map units; note, alters QgsProject::instance().
    _getMapUnits(*doc);

    // get the map layers
    pair< bool, list<QDomNode> > getMapLayersResults =  _getMapLayers(*doc);

    QgsRect savedExtent;

    if (!_getExtents(*doc, savedExtent))
    {
#ifdef QGISDEBUG
        qDebug("Unable to get extents from project file.");
#endif

        // Since we could be executing this from the test harness which
        // doesn't *have* layers -- nor a GUI for that matter -- we'll just
        // leave in the whining and boldly stomp on.

         throw QgsException("Cannot get extents from " + imp_->file.name());

         // return false;
    }

    // now restore the extent for the main canvas
    _setCanvasExtent("theMapCanvas", savedExtent);

    // ensure that overview map canvas is set to *entire* extent
    QgsRect mapCanvasFullExtent = _getFullExtent("theMapCanvas");
    _setCanvasExtent("theOverviewCanvas", mapCanvasFullExtent);

    if ( ! getMapLayersResults.first )
    {
#ifdef QGISDEBUG
        qDebug("Unable to get map layers from project file.");
#endif

        if ( ! getMapLayersResults.second.empty() )
        {
            qDebug( "%s:%d there are %d broken layers", __FILE__, __LINE__, getMapLayersResults.second.size() );
        }

        // Since we could be executing this from the test harness which
        // doesn't *have* layers -- nor a GUI for that matter -- we'll just
        // leave in the whining and boldly stomp on.

        throw QgsProjectBadLayerException( getMapLayersResults.second );

//         return false;
    }

    // can't be dirty since we're allegedly in pristine state
    dirty(false);

    return true;

} // QgsProject::read





bool QgsProject::read( QDomNode & layerNode )
{
    QString type = layerNode.toElement().attribute("type");

    QgsMapLayer *mapLayer;

    if (type == "vector")
    {
        mapLayer = new QgsVectorLayer;
    } 
    else if (type == "raster")
    {
        mapLayer = new QgsRasterLayer;
    }
    else
    {
#ifdef QGISDEBUG
        std::cerr << __FILE__ << " : " << __LINE__ << " bad layer type\n";
#endif
        return false;
    }

    if (!mapLayer)
    {
#ifdef QGISDEBUG
        std::cerr << __FILE__ << " : " << __LINE__ << " unable to create layer\n";
#endif
        return false;
    }

    // have the layer restore state that is stored in DOM node
    if ( mapLayer->readXML(layerNode) )
    {
        mapLayer = QgsMapLayerRegistry::instance()->addMapLayer(mapLayer);
    }
    else
    {
        delete mapLayer;

        qDebug( "%s:%d unable to load %s layer", __FILE__, __LINE__, type.ascii() );

        return false;
    }

    return  true;
} // QgsProject::read( QDomNode & layerNode )



bool QgsProject::write(QFileInfo const &file)
{
    imp_->file.setName(file.filePath());

    return write();
} // QgsProject::write( QFileInfo const & file )


bool QgsProject::write()
{
    // if we have problems creating or otherwise writing to the project file,
    // let's find out up front before we go through all the hand-waving
    // necessary to create all the DOM objects
    if (!imp_->file.open(IO_WriteOnly | IO_Translate | IO_Truncate))
    {
        imp_->file.close();         // even though we got an error, let's make
        // sure it's closed anyway

        throw QgsIOException("Unable to open " + imp_->file.name());

        return false;               // XXX raise exception? Ok now superfluous
        // XXX because of exception.
    }

    QDomImplementation DOMImplementation;

    QDomDocumentType documentType =
        DOMImplementation.createDocumentType("qgis", "http://mrcc.com/qgis.dtd",
                                             "SYSTEM");
    std::auto_ptr < QDomDocument > doc =
        std::auto_ptr < QDomDocument > (new QDomDocument(documentType));


    QDomElement qgisNode = doc->createElement("qgis");
    qgisNode.setAttribute("projectname", title());

    doc->appendChild(qgisNode);

    // title
    QDomElement titleNode = doc->createElement("title");
    qgisNode.appendChild(titleNode);

    QDomText titleText = doc->createTextNode(title());  // XXX why have title TWICE?
    titleNode.appendChild(titleText);

    // units

    QDomElement unitsNode = doc->createElement("units");
    qgisNode.appendChild(unitsNode);

    QString unitsString;

    switch (instance()->imp_->mapUnits)
    {
        case QGis::METERS:
            unitsString = "meters";
            break;
        case QGis::FEET:
            unitsString = "feet";
            break;
        case QGis::DEGREES:
            unitsString = "degrees";
            break;
        case QGis::UNKNOWN:
        default:
            unitsString = "unknown";
            break;
    }

    QDomText unitsText = doc->createTextNode(unitsString);
    unitsNode.appendChild(unitsText);

    // extents and layers info are written by the map canvas
    // find the canonical map canvas
    QgsMapCanvas *theMapCanvas = _findMapCanvas("theMapCanvas");

    if (!theMapCanvas)
    {
        qDebug("Unable to find canvas widget theMapCanvas");

        // return false;               // XXX some sort of error value? Exception?

        // Actually this might be run from the test harness, and therefore
        // there won't be a GUI, so no map canvas.   Just blithely continue on.
    }
    else
    {
        theMapCanvas->writeXML(qgisNode, *doc);
    }
    // now add the optional extra properties

    dump_(imp_->properties_);

    qDebug("there are %d property scopes", imp_->properties_.count());

    if (!imp_->properties_.isEmpty()) // only worry about properties if we
                                      // actually have any properties
    {
        imp_->properties_.writeXML( "properties", qgisNode, *doc );
    } 

    // now wrap it up and ship it to the project file
    doc->normalize();             // XXX I'm not entirely sure what this does

    QString xml = doc->toString(4); // write to string with indentation of four characters
                                // (yes, four is arbitrary)

    // const char * xmlString = xml; // debugger probe point
    // qDebug( "project file output:\n\n" + xml );

    QTextStream projectFileStream(&imp_->file);

    projectFileStream << xml << endl;

    imp_->file.close();


    dirty(false);                 // reset to pristine state

    return true;
} // QgsProject::write



void QgsProject::clearProperties()
{
#ifdef QGISDEBUG
    std::cout << "Clearing project properties QgsProject::clearProperties()" << std::endl;
#endif
    imp_->clear();

    dirty(true);
} // QgsProject::clearProperties()



bool
QgsProject::writeEntry(QString const &scope, const QString & key, bool value)
{
    dirty(true);

    return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., bool value )


bool
QgsProject::writeEntry(QString const &scope, const QString & key,
                       double value)
{
    dirty(true);

    return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., double value )


bool
QgsProject::writeEntry(QString const &scope, const QString & key, int value)
{
    dirty(true);

    return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., int value )


bool
QgsProject::writeEntry(QString const &scope, const QString & key,
                       const QString & value)
{
    dirty(true);

    return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., const QString & value )


bool
QgsProject::writeEntry(QString const &scope, const QString & key,
                       const QStringList & value)
{
    dirty(true);

    return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., const QStringList & value )




QStringList
QgsProject::readListEntry(QString const & scope,
                          const QString & key,
                          bool * ok) const 
{
    QgsProperty * property = findKey_( scope, key, imp_->properties_ );

    QVariant value;

    if ( property )
    {
        value = property->value();
    }

    bool valid = QVariant::StringList == value.type();

    if (ok)
    {
        *ok = valid;
    }

    if (valid)
    {
        return value.asStringList();
    }

    return QStringList();
} // QgsProject::readListEntry


QString
QgsProject::readEntry(QString const & scope, 
                      const QString & key,
                      const QString & def,
                      bool * ok) const 
{
    QgsProperty * property = findKey_( scope, key, imp_->properties_ );

    QVariant value;

    if ( property )
    {
        value = property->value();
    }

    bool valid = value.canCast(QVariant::String);

    if (ok)
    {
        *ok = valid;
    }

    if (valid)
    {
        return value.toString();
    }

    return QString(def);
} // QgsProject::readEntry


int
QgsProject::readNumEntry(QString const &scope, const QString & key, int def,
                         bool * ok) const 
{
    QgsProperty * property = findKey_( scope, key, imp_->properties_ );

    QVariant value;

    if ( property )
    {
        value = property->value();
    }

    bool valid = value.canCast(QVariant::String);

    if (ok)
    {
        *ok = valid;
    }

    if (valid)
    {
        return value.toInt();
    }

    return def;
} // QgsProject::readNumEntry


double
QgsProject::readDoubleEntry(QString const &scope, const QString & key,
                            double def,
                            bool * ok) const 
{
    QgsProperty * property = findKey_( scope, key, imp_->properties_ );

    QVariant value;

    if ( property )
    {
        value = property->value();
    }

    bool valid = value.canCast(QVariant::Double);

    if (ok)
    {
        *ok = valid;
    }

    if (valid)
    {
        return value.toDouble();
    }

    return def;
} // QgsProject::readDoubleEntry


bool
QgsProject::readBoolEntry(QString const &scope, const QString & key, bool def,
                          bool * ok) const
{
    QgsProperty * property = findKey_( scope, key, imp_->properties_ );

    QVariant value;

    if ( property )
    {
        value = property->value();
    }

    bool valid = value.canCast(QVariant::Bool);

    if (ok)
    {
        *ok = valid;
    }

    if (valid)
    {
        return value.toBool();
    }

    return def;
} // QgsProject::readBoolEntry


bool QgsProject::removeEntry(QString const &scope, const QString & key)
{
    removeKey_( scope, key, imp_->properties_ );

    dirty(true);

    return ! findKey_(scope, key, imp_->properties_ );
} // QgsProject::removeEntry



QStringList QgsProject::entryList(QString const &scope, QString const &key) const
{
    const char * scope_str = scope.ascii();
    const char * key_str   = key.ascii();

    QgsProperty * foundProperty = findKey_( scope, key, imp_->properties_ );

    QStringList entries;

    if ( foundProperty )
    {
        QgsPropertyKey * propertyKey = dynamic_cast<QgsPropertyKey*>(foundProperty);

        if (propertyKey)
        { propertyKey->entryList( entries ); }
    }

    return entries;
} // QgsProject::entryList 


QStringList 
QgsProject::subkeyList(QString const &scope, QString const &key) const
{
    const char * scope_str = scope.ascii();
    const char * key_str   = key.ascii();

    QgsProperty * foundProperty = findKey_( scope, key, imp_->properties_ );

    QStringList entries;

    if ( foundProperty )
    {
        QgsPropertyKey * propertyKey = dynamic_cast<QgsPropertyKey*>(foundProperty);

        if (propertyKey)
        { propertyKey->subkeyList( entries ); }
    }

    return entries;

} // QgsProject::subkeyList 



void QgsProject::dumpProperties() const
{
    dump_(imp_->properties_);
} // QgsProject::dumpProperties
