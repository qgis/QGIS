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
#include <iostream>

#include "qgisapp.h"
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
#include <qmessagebox.h>
#include <qwidgetlist.h>



static const char * const ident_ = "$Id$";




/// canonical project instance
QgsProject * QgsProject::theProject_;


struct QgsProject::Imp
{
    /// current physical project file
    QFile file;

    /** set of plug-in (and possibly qgis) related properties

        String key is scope; i.e., QMap<Scope,Properties>
     */
    QMap< QString, QgsProject::Properties > properties_;

    /// project title
    QString title;

    /// true if project has been modified since it has been read or saved
    bool dirty;

    /// map units for current project
    QgsScaleCalculator::units mapUnits;

    Imp()
        : title(""), dirty(false), mapUnits(QgsScaleCalculator::METERS)
    {}

}; // struct QgsProject::Imp




QgsProject::QgsProject()
    : imp_( new QgsProject::Imp )
{
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

    QDomNode titleTextNode = titleNode.firstChild(); // should only have on child

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
static
QgisApp *
_findQgisApp()
{
    QgisApp * qgisApp;

    QWidgetList  * list = QApplication::allWidgets();
    QWidgetListIt it( *list );  // iterate over the widgets
    QWidget * w;

    while ( (w=it.current()) != 0 ) 
    {   // for each top level widget...

        if ( "QgisApp" == w->name() )
        { 
            qgisApp = dynamic_cast<QgisApp*>(w);
            break;
        }
                                // "QgisApp" canonical name assigned in main.cpp
        qgisApp = dynamic_cast<QgisApp*>(w->child( "QgisApp", 0, true ));

        if ( qgisApp )
        { break; }

        ++it;
    }
    delete list;                // delete the list, not the widgets


//     if ( ! qgisApp )            // another tactic for finding qgisapp
//     {
//         if ( "QgisApp" == QApplication::mainWidget().name() )
//         { qgisApp = QApplication::mainWidget(); }
//     }

    if( ! qgisApp )
    {
        qDebug( "Unable to find QgisApp" );

        return 0x0;             // XXX some sort of error value?  Exception?
    }

    return qgisApp;
} // _findQgisApp



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

    for (size_t i = 0; i < nl.count(); i++)
    {
	QDomNode    node    = nl.item(i);
        QDomElement element = node.toElement();

        QString type = element.attribute("type");


        QgsMapLayer * mapLayer;

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

	return false;		// XXX raise exception? Ok now superfluous 
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

        qDebug( errorString );

	imp_->file.close();

        throw QgsException( errorString + " for file " + imp_->file.name().ascii() );

	return false;           // XXX superfluous because of exception
    }

    imp_->file.close();


#ifdef QGISDEBUG
    qWarning("opened document " + imp_->file.name());
#endif


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


    // XXX insert code for setting the properties


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

	return false;		// XXX raise exception? Ok now superfluous 
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

    qDebug( "there are %d property scopes", imp_->properties_.count() );

    if ( ! imp_->properties_.empty() ) // only worry about properties if we
                                       // actually have any
    {
        // <properties>
        QDomElement propertiesElement = doc->createElement( "properties" );
        qgisNode.appendChild( propertiesElement );

        for ( QMap< QString, QgsProject::Properties >::iterator curr_scope =
                  imp_->properties_.begin();
              curr_scope != imp_->properties_.end();
              curr_scope++ )
        {
            qDebug( "scope ``%s'' has %d entries", curr_scope.key().ascii(), curr_scope.data().count() );

            // <$scope>

            QDomElement scopeElement = doc->createElement( curr_scope.key() );
            propertiesElement.appendChild( scopeElement );

            for ( Properties::iterator curr_property = curr_scope.data().begin();
                  curr_property != curr_scope.data().end();
                  curr_property ++ )
            {
                qDebug( "storing %s -> %s: %s", 
                        curr_scope.key().ascii(), 
                        (*curr_property).first.ascii(),
                        (*curr_property).second.toString().ascii() );

                // <property type="$type">
                QDomElement propertyElement = doc->createElement( (*curr_property).first );

                // we store the QVariant type so that we can properly decode it later in read()
                propertyElement.setAttribute( "type", (*curr_property).second.typeName() );
                scopeElement.appendChild( propertyElement );

                // XXX Of course this assumes that the true underlying value of QVariant can be
                // XXX converted to string form; should add QVariant::canCast(T) sanity checks
                QDomText propertyText = doc->createTextNode( (*curr_property).second.toString() );
                propertyElement.appendChild( propertyText );

                // </property>

            } // </$scope>

        } // </properties>

    } // if any properties

    // now wrap it up and ship it to the project file


    doc->normalize();           // XXX I'm not entirely sure what this does

    QString xml = doc->toString( 4 ); // write to string with indentation of four characters
                                      // (yes, four is arbitrary)

    // const char * xmlString = xml.ascii(); // debugger probe point
    // qDebug( "project file output:\n\n" + xml );

    QTextStream projectFileStream( &imp_->file );

    projectFileStream << xml << endl;

    imp_->file.close();


    dirty( false );             // reset to pristine state

    return true;
} // QgsProject::write



QgsProject::Properties & 
QgsProject::properties( QString const & scope )
{
    return imp_->properties_[scope];
} // QgsProject::properties
