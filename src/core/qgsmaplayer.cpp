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

#include <iostream>

#include <QDateTime>
#include <QDomNode>
#include <QFileInfo>
#include <QSettings> // TODO: get rid of it [MD]

#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgsrect.h"
#include "qgssymbol.h"
#include "qgsmaplayer.h"
#include "qgsspatialrefsys.h"


QgsMapLayer::QgsMapLayer(int type,
                         QString lyrname,
                         QString source) :
        mTransparencyLevel(255), // 0 is completely transparent
        mValid(FALSE), // assume the layer is invalid
        mDataSource(source),
        mID(""),
        mLayerType(type)

{
    QgsDebugMsg("QgsMapLayer::QgsMapLayer - lyrname is '" + lyrname);
    
    mSRS = new QgsSpatialRefSys();

    // Set the display name = internal name
    mLayerName = capitaliseLayerName(lyrname);
    QgsDebugMsg("QgsMapLayer::QgsMapLayer - layerName is '" + mLayerName);

    // Generate the unique ID of this layer
    QDateTime dt = QDateTime::currentDateTime();
    mID = lyrname + dt.toString("yyyyMMddhhmmsszzz");
    // Tidy the ID up to avoid characters that may cause problems
    // elsewhere (e.g in some parts of XML). Replaces every non-word
    // character (word characters are the alphabet, numbers and
    // underscore) with an underscore. 
    // Note that the first backslashe in the regular expression is
    // there for the compiler, so the pattern is actually \W
    mID.replace(QRegExp("[\\W]"), "_");

    //set some generous  defaults for scale based visibility
    mMinScale = 0;
    mMaxScale = 100000000;
    mScaleBasedVisibility = false;
}



QgsMapLayer::~QgsMapLayer()
{
  delete mSRS;
}

int QgsMapLayer::type() const
{
    return mLayerType;
}

/** Get this layer's unique ID */
QString QgsMapLayer::getLayerID() const
{
    return mID;
}

/** Write property of QString layerName. */
void QgsMapLayer::setLayerName(const QString & _newVal)
{
  QgsDebugMsg("QgsMapLayer::setLayerName: new name is '" + _newVal);
  mLayerName = capitaliseLayerName(_newVal);
  emit layerNameChanged();
}

/** Read property of QString layerName. */
QString const & QgsMapLayer::name() const
{
  QgsDebugMsg("QgsMapLayer::name: returning name '" + mLayerName);
  return mLayerName;
}

QString QgsMapLayer::publicSource() const 
{ 
  // Redo this every time we're asked for it, as we don't know if 
  // dataSource has changed. 
  static QRegExp regexp(" password=.* "); 
  regexp.setMinimal(true); 
  QString safeName(mDataSource); 
  return safeName.replace(regexp, " "); 
}

QString const & QgsMapLayer::source() const
{
    return mDataSource;
}

const QgsRect QgsMapLayer::extent()
{
    return mLayerExtent;
}


bool QgsMapLayer::draw(QPainter *, QgsRect &, QgsMapToPixel *, QgsCoordinateTransform *, bool)
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
    return false;
}

void QgsMapLayer::drawLabels(QPainter *, QgsRect &, QgsMapToPixel *, QgsCoordinateTransform *)
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
}

bool QgsMapLayer::readXML( QDomNode & layer_node )
{
    QDomElement element = layer_node.toElement();

    // XXX not needed? QString type = element.attribute("type");

    // set data source
    QDomNode mnl = layer_node.namedItem("datasource");
    QDomElement mne = mnl.toElement();
    mDataSource = mne.text();

    // Set a SRS (any will do) so that we don't ask the user.
    // We will overwrite whatever GDAL etc picks up anway
    // further down this function.
    mSRS->createFromSrsId(GEOSRS_ID);

    // now let the children grab what they need from the DOM node.
    if (!readXML_( layer_node ))
    {
      return false;
    }

    // the internal name is just the data source basename
    QFileInfo dataSourceFileInfo( mDataSource );
    //internalName = dataSourceFileInfo.baseName();

    // set ID
    mnl = layer_node.namedItem("id");
    if ( ! mnl.isNull() ) 
    {
        mne = mnl.toElement();
        if ( ! mne.isNull() && mne.text().length() > 10 ) // should be at least 17 (yyyyMMddhhmmsszzz)
        {
            mID = mne.text();
        }
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

    // set name
    mnl = layer_node.namedItem("layername");
    mne = mnl.toElement();
    setLayerName( mne.text() );

    //read srs
    QDomNode srsNode = layer_node.namedItem("srs");
    mSRS->readXML(srsNode);
    
    //read transparency level
    QDomNode transparencyNode = layer_node.namedItem("transparencyLevelInt");
    if ( ! transparencyNode.isNull() )
    {
      // set transparency level only if it's in project
      // (otherwise it sets the layer transparent)
      QDomElement myElement = transparencyNode.toElement();
      setTransparency(myElement.text().toInt());
    }

    return true;
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

    // ID 
    QDomElement id = document.createElement( "id" );
    QDomText idText = document.createTextNode( getLayerID() );
    id.appendChild( idText );

    maplayer.appendChild( id );

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

    // spatial reference system id
    QDomElement mySrsElement = document.createElement( "srs" );
    mSRS->writeXML(mySrsElement, document);
    maplayer.appendChild(mySrsElement);
    
    // <transparencyLevelInt>
    QDomElement transparencyLevelIntElement = document.createElement( "transparencyLevelInt" );
    QDomText    transparencyLevelIntText    = document.createTextNode( QString::number(getTransparency()) );
    transparencyLevelIntElement.appendChild( transparencyLevelIntText );
    maplayer.appendChild( transparencyLevelIntElement );
    // now append layer node to map layer node

    layer_node.appendChild( maplayer );

    return writeXML_( maplayer, document );

} // bool QgsMapLayer::writeXML



bool QgsMapLayer::writeXML_( QDomNode & layer_node, QDomDocument & document )
{
    // NOP by default; children will over-ride with behavior specific to them

    return true;
} // void QgsMapLayer::writeXML_




bool QgsMapLayer::isValid()
{
    return mValid;
}


void QgsMapLayer::invalidTransformInput()
{
  QgsLogger::warning("QgsMapLayer::invalidTransformInput() called");
  // TODO: emit a signal - it will be used to update legend
}


QString QgsMapLayer::errorCaptionString()
{
  return QString();
}

QString QgsMapLayer::errorString()
{
  return QString();
}

void QgsMapLayer::connectNotify( const char * signal )
{
    QgsDebugMsg("QgsMapLayer connected to " + QString(signal));
} //  QgsMapLayer::connectNotify



void QgsMapLayer::setScaleBasedVisibility(bool theVisibilityFlag)
{
  mScaleBasedVisibility = theVisibilityFlag;
}

bool QgsMapLayer::scaleBasedVisibility()
{
  return mScaleBasedVisibility;
}

void QgsMapLayer::setMinScale(float theMinScale)
{
  mMinScale = theMinScale;
}
    
float QgsMapLayer::minScale()
{
  return mMinScale;
}

    
void QgsMapLayer::setMaxScale(float theMaxScale)
{
  mMaxScale = theMaxScale;
}

float QgsMapLayer::maxScale()
{
  return mMaxScale;
}


QStringList QgsMapLayer::subLayers()
{
  return QStringList();  // Empty
}
    
void QgsMapLayer::setLayerOrder(QStringList layers)
{
      // NOOP
}

void QgsMapLayer::setSubLayerVisibility(QString name, bool vis)
{
      // NOOP
}

const QgsSpatialRefSys& QgsMapLayer::srs()
{
  return *mSRS;
}

void QgsMapLayer::setSrs(const QgsSpatialRefSys& srs)
{
  *mSRS = srs;
}

unsigned int QgsMapLayer::getTransparency()
{
  return mTransparencyLevel;
}

void QgsMapLayer::setTransparency(unsigned int theInt)
{
  mTransparencyLevel = theInt;
}

QString QgsMapLayer::capitaliseLayerName(const QString name)
{
  // Capitalise the first letter of the layer name if requested
  QSettings settings;
  bool capitaliseLayerName = 
         settings.value("qgis/capitaliseLayerName", QVariant(false)).toBool();

  QString layerName(name);

  if (capitaliseLayerName)
    layerName = layerName.left(1).upper() + layerName.mid(1);

  return layerName;
}
