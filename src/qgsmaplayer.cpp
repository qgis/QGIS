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
#include <cfloat>
#include <iostream>
#include <qdatetime.h>
#include "qgsrect.h"
#include "qgssymbol.h"
#include "qgsmaplayer.h"

QgsMapLayer::QgsMapLayer(int type, QString lyrname, QString source):layerName(lyrname), layerType(type), dataSource(source)
{
	// assume the layer is valid (data source exists and can be used)
	// until we learn otherwise
	valid = true;
	m_visible = true;
	// create a default symbol

	// Generate the unique ID of this layer
	QDateTime dt = QDateTime::currentDateTime();
	ID = lyrname + dt.toString("yyyyMMddhhmmsszzz");
}

QgsMapLayer::~QgsMapLayer()
{
}
const int QgsMapLayer::type()
{
	return layerType;
}

/** Get this layer's unique ID */
QString QgsMapLayer::getLayerID()
{
	return ID;
}

/** Write property of QString layerName. */
void QgsMapLayer::setlayerName(const QString & _newVal)
{
	layerName = _newVal;
}

/** Read property of QString layerName. */
const QString QgsMapLayer::name()
{
	return layerName;
}

QString QgsMapLayer::source()
{
	return dataSource;
}
const QgsRect QgsMapLayer::extent()
{
	return layerExtent;
}

QgsRect QgsMapLayer::bBoxOfSelected()
{

}

QgsRect QgsMapLayer::calculateExtent()
{
    //just to prevent any crashes
    QgsRect rect(DBL_MAX,DBL_MAX,DBL_MIN,DBL_MIN);
    return rect;
}
void QgsMapLayer::draw(QPainter *, QgsRect * viewExtent, int yTransform)
{
//  std::cout << "In QgsMapLayer::draw" << std::endl;
}

void QgsMapLayer::draw(QPainter *, QgsRect *, QgsCoordinateTransform *)
{
//  std::cout << "In QgsMapLayer::draw" << std::endl;
}


/** Read property of QgsSymbol * m_symbol. */
QgsSymbol *QgsMapLayer::symbol()
{
	return m_symbol;
}

/** Write property of QgsSymbol * m_symbol. */
void QgsMapLayer::setSymbol(QgsSymbol * _newVal)
{
	m_symbol = _newVal;
}

/** Read property of QString labelField. */
const QString & QgsMapLayer::labelField()
{
	return m_labelField;
}

/** Write property of QString labelField. */
void QgsMapLayer::setlabelField(const QString & _newVal)
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
	m_visible = vis;
	emit visibilityChanged();
}	 /** Read property of int featureType. */
const int &QgsMapLayer::featureType()
{
	return feature;
}

/** Write property of int featureType. */
void QgsMapLayer::setFeatureType(const int &_newVal)
{
	feature = _newVal;
}

void QgsMapLayer::identify(QgsRect * r)
{
}

void QgsMapLayer::table()
{
}

void QgsMapLayer::select(QgsRect *, bool lock)
{

}
