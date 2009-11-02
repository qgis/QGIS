/***************************************************************************
                          qgsvectorfilewriter.h
                          generic vector file writer
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

#include "qgsvectorlayer.h"
#include "qgsfield.h"

typedef void *OGRDataSourceH;
typedef void *OGRLayerH;
typedef void *OGRGeometryH;

class QTextCodec;

/** \ingroup core
  * A convenience class for writing vector files to disk.
 There are two possibilities how to use this class:
 1. static call to QgsVectorFileWriter::writeAsShapefile(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)

 Currently supports only writing to shapefiles, but shouldn't be a problem to add capability
 to support other OGR-writable formats.
 */
class CORE_EXPORT QgsVectorFileWriter
{
  public:

    enum WriterError
    {
      NoError = 0,
      ErrDriverNotFound,
      ErrCreateDataSource,
      ErrCreateLayer,
      ErrAttributeTypeUnsupported,
      ErrAttributeCreationFailed
    };

    /** Write contents of vector layer to a shapefile */
    static WriterError writeAsShapefile( QgsVectorLayer* layer,
                                         const QString& shapefileName,
                                         const QString& fileEncoding,
                                         const QgsCoordinateReferenceSystem *destCRS,
                                         bool onlySelected = FALSE );


    /** create shapefile and initialize it */
    QgsVectorFileWriter( const QString& shapefileName,
                         const QString& fileEncoding,
                         const QgsFieldMap& fields,
                         QGis::WkbType geometryType,
                         const QgsCoordinateReferenceSystem* srs );

    /** checks whether there were any errors in constructor */
    WriterError hasError();

    /** add feature to the currently opened shapefile */
    bool addFeature( QgsFeature& feature );

    /** close opened shapefile for writing */
    ~QgsVectorFileWriter();

    /** Delete a shapefile (and its accompanying shx / dbf / prf)
     * @param QString theFileName - /path/to/file.shp
     * @return bool true if the file was deleted successfully
     */
    static bool deleteShapeFile( QString theFileName );
  protected:

    OGRGeometryH createEmptyGeometry( QGis::WkbType wkbType );

    OGRDataSourceH mDS;
    OGRLayerH mLayer;
    OGRGeometryH mGeom;

    QgsFieldMap mFields;

    /** contains error value if construction was not successful */
    WriterError mError;

    QTextCodec* mCodec;

    /** geometry type which is being used */
    QGis::WkbType mWkbType;
};

#endif
