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

    /// set of plug-in (and possibly qgis) related properties
    QgsProject::Properties properties_;
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


void QgsProject::name( QString const & name )
{
    imp_->file.setName( name );
} // void QgsProject::name( QString const & name )


QString QgsProject::name() const
{
    return imp_->file.name();
} // QString QgsProject::name() const



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
    QDomNodeList nl = doc.elementsByTagName("maplayer");

    // XXX what is this used for? QString layerCount( QString::number(nl.count()) );
    
    QString wk;
    
    // process the map layer nodes

    for (int i = 0; i < nl.count(); i++)
    {
	QDomNode    node    = nl.item(i);
        QDomElement element = node.toElement();

        QString type = element.attribute("type");

	// QString myNewLayerId="";

	if (type == "vector")
	{

	    QgsVectorLayer * vectorLayer = new QgsVectorLayer;

	    Q_CHECK_PTR( vectorLayer );

	    if ( ! vectorLayer )
	    {
#ifdef QGISDEBUG
		std::cerr << __FILE__ << " : " << __LINE__
			  << " unable to create vector layer\n";
#endif                  
		return false;
	    }
            
            // have the vector layer restore state that is stored in DOM node
            vectorLayer->readXML( node );

	    QgsMapLayerRegistry::instance()->addMapLayer( vectorLayer );

            // XXX set z order here?  Or in readXML()?  Leaning to latter.
            // XXX Or, how about Z order being implicit in order of layer
            // XXX information stored in project file?
	} 
	else if (type == "raster")
	{
#ifdef TEMPORARILYDISABLED
	    QgsRasterLayer *myRasterLayer = new QgsRasterLayer(dataSource, layerName);
	    // myNewLayerId=myRasterLayer->getLayerID();

	    myRasterLayer->setVisible(visible == "1");
	    if (showInOverview == "1")
	    {
		myRasterLayer->toggleShowInOverview();
	    }

	    mnl = node.namedItem("rasterproperties");

	    QDomNode snode = mnl.namedItem("showDebugOverlayFlag");
	    QDomElement myElement = snode.toElement();
	    QVariant myQVariant = (QVariant) myElement.attribute("boolean");
	    myRasterLayer->setShowDebugOverlayFlag(myQVariant.toBool());

	    snode = mnl.namedItem("drawingStyle");
	    myElement = snode.toElement();
	    myRasterLayer->setDrawingStyle(myElement.text());

	    snode = mnl.namedItem("invertHistogramFlag");
	    myElement = snode.toElement();
	    myQVariant = (QVariant) myElement.attribute("boolean");
	    myRasterLayer->setInvertHistogramFlag(myQVariant.toBool());

	    snode = mnl.namedItem("stdDevsToPlotDouble");
	    myElement = snode.toElement();
	    myRasterLayer->setStdDevsToPlot(myElement.text().toDouble());

	    snode = mnl.namedItem("transparencyLevelInt");
	    myElement = snode.toElement();
	    myRasterLayer->setTransparency(myElement.text().toInt());

	    snode = mnl.namedItem("redBandNameQString");
	    myElement = snode.toElement();
	    myRasterLayer->setRedBandName(myElement.text());
	    snode = mnl.namedItem("greenBandNameQString");
	    myElement = snode.toElement();
	    myRasterLayer->setGreenBandName(myElement.text());

	    snode = mnl.namedItem("blueBandNameQString");
	    myElement = snode.toElement();
	    myRasterLayer->setBlueBandName(myElement.text());

	    snode = mnl.namedItem("grayBandNameQString");
	    myElement = snode.toElement();
	    myRasterLayer->setGrayBandName(myElement.text());

	    QgsMapLayerRegistry::instance()->addMapLayer(myRasterLayer);
	    // XXX myZOrder.push_back(myRasterLayer->getLayerID());
#endif
	}

    }

    return true;

} // _getMapLayers




bool
QgsProject::read( )
{
    // purge current state first
    std::auto_ptr<QDomDocument> doc = 
	std::auto_ptr<QDomDocument>(new QDomDocument("qgisdocument"));

    if ( ! imp_->file.open(IO_ReadOnly) )
    {
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

    // enable the hourglass -- no, this should be done be the caller

    qWarning("opened document" + imp_->file.name());


    // first get the map layers
    if ( ! _getMapLayers( *doc ) )
    {
        qDebug( "Unable to get map layers from project file." );

        throw QgsException( "Cannot get map layers from " + imp_->file.name() );

        return false;
    }

    // XXX insert code for handling z order


    // restore the canvas' area of interest

                                // first find the canonical map canvas

    QgsMapCanvas * theMapCanvas;

    QWidgetList  * list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );  // iterate over the widgets
    QWidget * w;

    while ( (w=it.current()) != 0 ) 
    {   // for each top level widget...
        ++it;
        theMapCanvas = dynamic_cast<QgsMapCanvas*>(w->child( "theMapCanvas", 0, true ));

        if ( theMapCanvas )
        { break; }
    }
    delete list;                // delete the list, not the widgets


    if( ! theMapCanvas )
    {
        qDebug( "Unable to find main canvas widget" );

        return false;
    }

    // restor the area of interest, or extent
    QgsRect savedExtent;

    if ( ! _getExtents( *doc, savedExtent ) )
    {
        qDebug( "Unable to get extents from project file." );

        throw QgsException( "Cannot get extents from " + imp_->file.name() );

        return false;
    }

    theMapCanvas->setExtent( savedExtent );



    // XXX insert code for setting the properties

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
    // XXX add guts from QgsProjectIO write

    return true;
} // QgsProject::write



QgsProject::Properties & 
QgsProject::properties()
{
    return imp_->properties_;
} // QgsProject::properties


    
