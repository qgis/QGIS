/***************************************************************************
                          qgsshapefilelayer.cpp  -  description
                             -------------------
    begin                : Thu Aug 1 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include "qgsshapefilelayer.h"
#include <ogrsf_frmts.h>

QgsShapeFileLayer::QgsShapeFileLayer(){
// test ogr access to a shapefile
 	OGRDataSource *ds;
	
  ds = OGRSFDriverRegistrar::Open( "/cdrom/data/akrr.shp" );
    if( ds != NULL )
    {
			OGRLayer *lyr = ds->GetLayer(0);
			while(OGRFeature *fet = lyr->GetNextFeature()){
				fet->DumpReadable(stdout);
				OGRGeometry *geom = fet->GetGeometryRef();
				std::cout << geom->getGeometryName() << std::endl;
				
				delete fet;				
			}
			
			delete ds;
    }
    


	
}
QgsShapeFileLayer::~QgsShapeFileLayer(){
}

/** No descriptions */
void QgsShapeFileLayer::registerFormats(){
}
