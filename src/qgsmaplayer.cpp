/***************************************************************************
                          qgsmaplayer.cpp  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <cfloat>
#include <iostream>

#include <qapplication.h>
#include <qdatetime.h>
#include <qdom.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qevent.h>

#include "qgsrect.h"
#include "qgssymbol.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"




QgsMapLayer::QgsMapLayer(int type,
                         QString lyrname,
                         QString source)
        : internalName(lyrname),
        ID(""),
        layerType(type),
        dataSource(source),
        m_legendItem(0),
        m_visible(true),
        mShowInOverview(false),
        mShowInOverviewItemId(0),
        valid(true) // assume the layer is valid (data source exists and can be
        // used) until we learn otherwise

{
    // Set the display name = internal name
    layerName = internalName;

    // Generate the unique ID of this layer
    QDateTime dt = QDateTime::currentDateTime();
    ID = lyrname + dt.toString("yyyyMMddhhmmsszzz");

#if defined(WIN32) || defined(Q_OS_MACX)

    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif

    mInOverviewPixmap.load(QString(PKGDATAPATH) + QString("/images/icons/inoverview.png"));
    mEditablePixmap.load(QString(PKGDATAPATH) + QString("/images/icons/editable.png"));

    //mActionInOverview = new QAction( "in Overview", "Ctrl+O", this );

    //set some generous  defaults for scale based visibility
    mMinScale = 0;
    mMaxScale = 100000000;
    mScaleBasedVisibility = false;

}



QgsMapLayer::~QgsMapLayer()
{}
const int QgsMapLayer::type()
{
    return layerType;
}

/** Get this layer's unique ID */
QString const & QgsMapLayer::getLayerID() const
{
    return ID;
}

/** Write property of QString layerName. */
void QgsMapLayer::setLayerName(const QString & _newVal)
{
    layerName = _newVal;
}

/** Read property of QString layerName. */
QString const & QgsMapLayer::name() const
{
    return layerName;
}

QString const & QgsMapLayer::source() const
{
    return dataSource;
}

QString const & QgsMapLayer::sourceName() const
{
    return internalName;
}

const QgsRect QgsMapLayer::extent()
{
    return layerExtent;
}

QgsRect QgsMapLayer::calculateExtent()
{
    //just to prevent any crashes
    QgsRect rect(DBL_MAX, DBL_MAX, DBL_MIN, DBL_MIN);
    return rect;
}
void QgsMapLayer::draw(QPainter *, QgsRect * viewExtent, int yTransform)
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
}

void QgsMapLayer::draw(QPainter *, QgsRect *, QgsMapToPixel *,QPaintDevice * )
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
}

void QgsMapLayer::drawLabels(QPainter *, QgsRect *, QgsMapToPixel *,QPaintDevice * )
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
}

/** Read property of QString labelField. */
const QString & QgsMapLayer::labelField()
{
    return m_labelField;
}


bool QgsMapLayer::readXML( QDomNode & layer_node )
{
    QDomElement element = layer_node.toElement();

    // XXX not needed? QString type = element.attribute("type");

    QString visible = element.attribute("visible");

    if ( "1" == visible )
    {
        setVisible( true );
    }
    else
    {
        setVisible( false );
    }

    QString showInOverview = element.attribute("showInOverviewFlag");

    if ( "1" == showInOverview )
    {
        mShowInOverview = true;
    }
    else
    {
        mShowInOverview = false;
    }

    // use scale dependent visibility flag
    QString scaleBasedVisibility = element.attribute("scaleBasedVisibilityFlag");
    if ( "1" == scaleBasedVisibility )
    {
        setScaleBasedVisibility(true);
    }
    else
    {
        setScaleBasedVisibility(false);
    }
    setMinScale(element.attribute("minScale").toFloat());
    setMaxScale(element.attribute("maxScale").toFloat());
    // set data source
    QDomNode mnl = layer_node.namedItem("datasource");
    QDomElement mne = mnl.toElement();
    dataSource = mne.text();

    const char * dataSourceStr = dataSource; // debugger probe

    // the internal name is just the data source basename
    QFileInfo dataSourceFileInfo( dataSource );
    internalName = dataSourceFileInfo.baseName();

    // set name
    mnl = layer_node.namedItem("layername");
    mne = mnl.toElement();
    setLayerName( mne.text() );

    const char * layerNameStr = mne.text(); // debugger probe

    // process zorder
    mnl = layer_node.namedItem("zorder");
    mne = mnl.toElement();
    // XXX and do what with it?

    // now let the children grab what they need from the DOM node.
    return readXML_( layer_node );

} // void QgsMapLayer::readXML


bool QgsMapLayer::readXML_( QDomNode & layer_node )
{
    // NOP by default; children will over-ride with behavior specific to them

    return true;
} // void QgsMapLayer::readXML_



bool QgsMapLayer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    // general layer metadata
    QDomElement maplayer = document.createElement( "maplayer" );

    // visible flag
    if ( visible() )
    {
        maplayer.setAttribute( "visible", 1 );
    }
    else
    {
        maplayer.setAttribute( "visible", 0 );
    }


    // show in overview flag
    if ( showInOverviewStatus() )
    {
        maplayer.setAttribute( "showInOverviewFlag", 1 );
    }
    else
    {
        maplayer.setAttribute( "showInOverviewFlag", 0 );
    }

    // use scale dependent visibility flag
    if ( scaleBasedVisibility() )
    {
        maplayer.setAttribute( "scaleBasedVisibilityFlag", 1 );
    }
    else
    {
        maplayer.setAttribute( "scaleBasedVisibilityFlag", 0 );
    }
    maplayer.setAttribute( "minScale", minScale() );
    maplayer.setAttribute( "maxScale", maxScale() );

    // data source
    QDomElement dataSource = document.createElement( "datasource" );
    QDomText dataSourceText = document.createTextNode( source() );
    dataSource.appendChild( dataSourceText );

    maplayer.appendChild( dataSource );


    // layer name
    QDomElement layerName = document.createElement( "layername" );
    QDomText layerNameText = document.createTextNode( name() );
    layerName.appendChild( layerNameText );

    maplayer.appendChild( layerName );

    // zorder
    // This is no longer stored in the project file. It is superflous since the layers
    // are written and read in the proper order.

    // now append layer node to map layer node

    layer_node.appendChild( maplayer );

    return writeXML_( maplayer, document );

} // bool QgsMapLayer::writeXML



bool QgsMapLayer::writeXML_( QDomNode & layer_node, QDomDocument & document )
{
    // NOP by default; children will over-ride with behavior specific to them

    return true;
} // void QgsMapLayer::writeXML_




/** Write property of QString labelField. */
void QgsMapLayer::setLabelField(const QString & _newVal)
{
    m_labelField = _newVal;
}

bool QgsMapLayer::isValid()
{
    return valid;
}

bool QgsMapLayer::visible()
{
    return m_visible;
}

void QgsMapLayer::setVisible(bool vis)
{
  if (m_visible != vis)
  {
    if (m_legendItem != 0)
      m_legendItem->setOn(vis);
    m_visible = vis;
    emit visibilityChanged();
  }
}



void QgsMapLayer::inOverview( bool b )
{
    // will we have to propogate changes?
    bool updateNecessary = mShowInOverview != b;

    mShowInOverview = b;

    if ( updateNecessary ) // update the show in overview popup menu item
    {
        updateOverviewPopupItem();
        updateItemPixmap();

        emit showInOverview(this,mShowInOverview);
    }
} // QgsMapLayer::inOverview



// void QgsMapLayer::toggleShowInOverview()
// {
//   if (mShowInOverview==false)
//   {
// #ifdef QGISDEBUG
//     std::cout << "Map layer " << ID << " requested to be added to the overview " << std::endl;
// #endif
//     mShowInOverview=true;
//   }
//   else
//   {
// #ifdef QGISDEBUG
//     std::cout << "Map layer " << ID << " requested to be removed from the overview " << std::endl;
// #endif
//     mShowInOverview=false;
//   }
//   //update the show in overview popup menu item
//   updateOverviewPopupItem();
//   updateItemPixmap();
//   emit showInOverview(ID,mShowInOverview);
// }


void QgsMapLayer::updateItemPixmap()
{
    if (m_legendItem)             // XXX should we know about our legend?
    {
        QPixmap pix=*(this->legendPixmap());
        if(mShowInOverview)
        {
            //add overview glasses to the pixmap
            QPainter p(&pix);
            p.drawPixmap(0,0,mInOverviewPixmap);
        }
        if(isEditable())
        {
            //add editing icon to the pixmap
            QPainter p(&pix);
            p.drawPixmap(30,0,mEditablePixmap);
        }
        ((QCheckListItem *) m_legendItem)->setPixmap(0,pix);
    }
}

void QgsMapLayer::updateOverviewPopupItem()
{
    if (mShowInOverviewItemId != 0)
    {
        popMenu->setItemChecked(mShowInOverviewItemId,mShowInOverview);
    }

}

const int &QgsMapLayer::featureType()
{
    return geometryType;
}

/** Write property of int featureType. */
void QgsMapLayer::setFeatureType(const int &_newVal)
{
    geometryType = _newVal;
}

QPixmap *QgsMapLayer::legendPixmap()
{
    return &m_legendPixmap;
}

QgsLegendItem *QgsMapLayer::legendItem()
{
    return m_legendItem;
}

void QgsMapLayer::setLegendItem(QgsLegendItem * li)
{
    m_legendItem = li;
}

QPopupMenu *QgsMapLayer::contextMenu()
{
    return 0;
}


QgsFeature * QgsMapLayer::getFirstFeature(bool fetchAttributes) const
{
    return 0x0;                 // by default return NULL
} // QgsMapLayer::getFirstFeature


QgsFeature * QgsMapLayer::getNextFeature(bool fetchAttributes) const
{
    return 0x0;                 // by default return NULL
} // QgsMapLayer::getNextFeature


bool QgsMapLayer::getNextFeature(QgsFeature &feature, bool fetchAttributes) const
{
    return false;
} // QgsMapLayer::getNextFeature


long QgsMapLayer::featureCount() const
{
    return 0;
} // QgsMapLayer::featureCount



int QgsMapLayer::fieldCount() const
{
    return 0;
} // QgsMapLayer::fieldCount



std::vector<QgsField> const & QgsMapLayer::fields() const
{
    static std::vector<QgsField> bogus; // bogus empty container

    return bogus;
} // QgsMapLayer::fields()



void QgsMapLayer::connectNotify( const char * signal )
{
#ifdef QGISDEBUG
    std::cerr << "QgsMapLayer connected to " << signal << "\n";
#endif
} //  QgsMapLayer::connectNotify


void QgsMapLayer::initContextMenu(QgisApp * app)
{
    popMenu = new QPopupMenu();
    myPopupLabel = new QLabel( popMenu );

    myPopupLabel->setFrameStyle( QFrame::Panel | QFrame::Raised );

    // now set by children
    // myPopupLabel->setText( tr("<center><b>Vector Layer</b></center>") );

    popMenu->insertItem(myPopupLabel,0);

    popMenu->insertItem(tr("&Zoom to extent of selected layer"), app, SLOT(zoomToLayerExtent()));
    popMenu->insertSeparator();



    app->actionInOverview->addTo( popMenu );

    popMenu->insertSeparator();
    popMenu->insertItem(tr("&Remove"), app, SLOT(removeLayer()));

    // now give the sub-classes a chance to tailor the context menu
    initContextMenu_( app );
    //properties goes on bottom of menu for consistency with normal ui standards
    //e.g. kde stuff
    popMenu->insertItem(tr("&Properties"), this, SLOT(showLayerProperties()));
} // QgsMapLayer::initContextMenu(QgisApp * app)

void QgsMapLayer::keyPressed ( QKeyEvent * e )
{
  if (e->key()==Qt::Key_Escape) mDrawingCancelled = true;
  std::cout << e->ascii() << " pressed in maplayer !" << std::endl;
}
