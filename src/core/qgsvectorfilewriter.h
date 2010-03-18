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
      ErrAttributeCreationFailed,
      ErrProjection // added in 1.5
    };

    /** Write contents of vector layer to a shapefile
        @note: deprecated. Use writeAsVectorFormat instead*/
    static WriterError writeAsShapefile( QgsVectorLayer* layer,
                                         const QString& shapefileName,
                                         const QString& fileEncoding,
                                         const QgsCoordinateReferenceSystem *destCRS,
                                         bool onlySelected = FALSE,
                                         QString *errorMessage = 0 );

    /** Write contents of vector layer to an (OGR supported) vector formt
        @note: this method was added in version 1.5*/
    static WriterError writeAsVectorFormat( QgsVectorLayer* layer,
                                            const QString& fileName,
                                            const QString& fileEncoding,
                                            const QgsCoordinateReferenceSystem *destCRS,
                                            const QString& driverName = "ESRI Shapefile",
                                            bool onlySelected = FALSE,
                                            QString *errorMessage = 0 );

    /** create shapefile and initialize it */
    QgsVectorFileWriter( const QString& vectorFileName,
                         const QString& fileEncoding,
                         const QgsFieldMap& fields,
                         QGis::WkbType geometryType,
                         const QgsCoordinateReferenceSystem* srs,
                         const QString& driverName = "ESRI Shapefile" );

    /**Returns map with format filter string as key and OGR format key as value*/
    static QMap< QString, QString> supportedFiltersAndFormats();

    /**Returns filter string that can be used for dialogs*/
    static QString fileFilterString();

    /**Creates a filter for an OGR driver key*/
    static QString filterForDriver( const QString& driverName );

    /** checks whether there were any errors in constructor */
    WriterError hasError();

    /** retrieves error message
     * @note added in 1.5
     */
    QString errorMessage();

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
    QString mErrorMessage;

    QTextCodec *mCodec;

    /** geometry type which is being used */
    QGis::WkbType mWkbType;

    /** map attribute indizes to OGR field indexes */
    QMap<int, int> mAttrIdxToOgrIdx;
};

#endif
