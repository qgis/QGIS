/***************************************************************************
    qgsfeature.cpp - Spatial Feature Implementation
     --------------------------------------
    Date                 : 09-Sep-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <iostream>
#include <qstring.h>
#include "qgsfeature.h"
/** \class QgsFeature
* \brief Encapsulates a spatial feature with attributes
*/
//! Constructor
QgsFeature::QgsFeature():fId(0), geometry(0), wkt(0)
{

}

QgsFeature::QgsFeature(int id):fId(id), geometry(0), wkt(0)
{

}

//! Destructor
QgsFeature::~QgsFeature()
{
  #ifdef QGISDEBUG
  std::cerr << "In QgsFeature destructor" << std::endl;
  #endif
  
  delete[]geometry;
  delete[]wkt;
}

/**
* Get the feature id for this feature
* @return Feature id
*/
int QgsFeature::featureId()
{
  return fId;
}

/**
* Get the attributes for this feature.
* @return A std::map containing the field name/value mapping
*/
const std::vector < QgsFeatureAttribute > &QgsFeature::attributeMap()
{
  return attributes;
}

/**
* Add an attribute to the map
*/
void QgsFeature::addAttribute(QString field, QString value)
{
  attributes.push_back(QgsFeatureAttribute(field, value));
}

/**
* Get the fields for this feature
* @return A std::map containing field position (index) and field name
*/
const std::map < int, QString > &QgsFeature::fields()
{
  return fieldNames;
}

/**
* Get the pointer to the feature geometry
*/
unsigned char *QgsFeature::getGeometry()
{
  return geometry;
}

/**
* Return well known text representation of this feature
*/
char *QgsFeature::wellKnownText()
{
  return wkt;
}

/** Set the pointer to the feature geometry
*/
void QgsFeature::setGeometry(unsigned char *geom)
{
  geometry = geom;

}
void QgsFeature::setWellKnownText(char *wkText)
{
  wkt = wkText;
}
