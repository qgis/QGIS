/***************************************************************************
    qgsgrassprovider.cpp -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <string.h>
#include <iostream>
#include <vector>
#include <cfloat>

#include <qpixmap.h>
#include <qiconset.h>
#include <qdir.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qmessagebox.h>

#include "../../src/qgis.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"

extern "C" {
#include <gis.h>
#include <dbmi.h>
#include <Vect.h>
}

#include "qgsgrass.h"
#include "qgsgrassprovider.h"

/**
* Class factory to return a pointer to a newly created 
* QgsGrassProvider object
*/
extern "C" QgsGrassProvider * classFactory(const QString *uri)
{
    return new QgsGrassProvider(*uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
extern "C" QString providerKey(){
    return QString("grass");
}
/**
* Required description function 
*/
extern "C" QString description(){
    return QString("GRASS data provider");
} 
/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
extern "C" bool isProvider(){
    return true;
}

