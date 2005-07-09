/***************************************************************************
                     qgsvectorfilewriter.h -  description
                             -------------------
    begin                : Jun 6 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim at linfiniti.com
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
#ifndef _QGSVECTORFILEWRITER_H_
#define _QGSVECTORFILEWRITER_H_

// qgis includes
#include "qgspoint.h"
#include "qgsvectorlayer.h"

//qt includes
#include <qstring.h>
#include <qfile.h>

// OGR Includes
#include "ogr_api.h"

class QgsVectorFileWriter
{
    public:
  QgsVectorFileWriter(QString theOutputFileName, QString fileEncoding, QgsVectorLayer * theVectorLayer);
        QgsVectorFileWriter(QString theOutputFileName, QString fileEncoding, OGRwkbGeometryType theGeometryType);
        ~QgsVectorFileWriter() ;
	/**Writes a point to the file*/
        bool writePoint(QgsPoint * thePoint);
	/**Writes a line to the file
	 @param wkb well known binary char array
	 @param size size of the binary array
	 @return true in case of success and false else*/
	bool writeLine(unsigned char* wkb, int size);
	/**Writes a polygon to the file
	@param wkb well known binary char array
	@param size size of the binary array
	@return true in case of success and false else*/ 
	bool writePolygon(unsigned char* wkb, int size);
        //! Add a new field to the output attribute table
        bool createField(QString theName, OGRFieldType theType, int theWidthInt=0, int thePrecisionInt=0);
        //! creates the output file etc...
        bool initialise();
    private:
        //! current record number
        int mCurrentRecInt;    
        //! file name to be written to 
        QString mOutputFileName;
        //! file type to be written to
        QString mOutputFormat;
        //! Encodionf for the layer attributes and other properties.
        QTextCodec *mEncoding;
        //! Ogr handle to the output datasource
        OGRDataSourceH mDataSourceHandle;
        //! Ogr handle to the spatial layer (e.g. .shp) parrt of the datasource
        OGRLayerH mLayerHandle;
        //! The geometry type for the output file
        OGRwkbGeometryType mGeometryType;
        //! Whether the output gile has been initialised. Some operations require this to be true before they will run
        bool mInitialisedFlag;
	enum ENDIAN
	    {
		NDR = 1,
		XDR = 0
	    };
	/** Return endian-ness for this layer*/
	int endian();
};
#endif
