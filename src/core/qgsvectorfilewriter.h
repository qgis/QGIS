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

#ifndef _QGSVECTORFILEWRITER_H_
#define _QGSVECTORFILEWRITER_H_

#include "qgsvectorlayer.h"
#include "qgsfield.h"
#include "qgssymbolv2.h"
#include <ogr_api.h>

#include <QPair>


class QgsSymbolLayerV2;
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
      ErrProjection,  // added in 1.5
      ErrFeatureWriteFailed, // added in 1.6
      ErrInvalidLayer, // added in 2.0
    };

    //added in 2.0
    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    /** Write contents of vector layer to an (OGR supported) vector formt
        @note: this method was added in version 1.5
    @param layer layer to write
    @param fileName file name to write to
    @param fileEncoding encoding to use
    @param destCRS pointer to CRS to reproject exported geometries to
    @param driverName OGR driver to use
    @param onlySelected write only selected features of layer
    @param errorMessage pointer to buffer fo error message
    @param datasourceOptions list of OGR data source creation options
    @param layerOptions list of OGR layer creation options
    @param skipAttributeCreation only write geometries
    @param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
    @param symbologyExport symbology to export
    @param symbologyScale scale of symbology
    */
    static WriterError writeAsVectorFormat( QgsVectorLayer* layer,
                                            const QString& fileName,
                                            const QString& fileEncoding,
                                            const QgsCoordinateReferenceSystem *destCRS,
                                            const QString& driverName = "ESRI Shapefile",
                                            bool onlySelected = false,
                                            QString *errorMessage = 0,
                                            const QStringList &datasourceOptions = QStringList(),  // added in 1.6
                                            const QStringList &layerOptions = QStringList(),  // added in 1.6
                                            bool skipAttributeCreation = false, // added in 1.6
                                            QString *newFilename = 0, // added in 1.9
                                            SymbologyExport symbologyExport = NoSymbology, //added in 2.0
                                            double symbologyScale = 1.0 // added in 2.0
                                          );

    /** create shapefile and initialize it */
    QgsVectorFileWriter( const QString& vectorFileName,
                         const QString& fileEncoding,
                         const QgsFields& fields,
                         QGis::WkbType geometryType,
                         const QgsCoordinateReferenceSystem* srs,
                         const QString& driverName = "ESRI Shapefile",
                         const QStringList &datasourceOptions = QStringList(), // added in 1.6
                         const QStringList &layerOptions = QStringList(), // added in 1.6
                         QString *newFilename = 0, // added in 1.9
                         SymbologyExport symbologyExport = NoSymbology//added in 2.0
                       );

    /**Returns map with format filter string as key and OGR format key as value*/
    static QMap< QString, QString> supportedFiltersAndFormats();

    /**Returns driver list that can be used for dialogs. It contains all OGR drivers
     * + some additional internal QGIS driver names to distinguish between more
     * supported formats of the same OGR driver
     */
    static QMap< QString, QString> ogrDriverList();

    /**Returns filter string that can be used for dialogs*/
    static QString fileFilterString();

    /**Creates a filter for an OGR driver key*/
    static QString filterForDriver( const QString& driverName );

    /**Converts codec name to string passed to ENCODING layer creation option of OGR Shapefile*/
    static QString convertCodecNameForEncodingOption( const QString &codecName );

    /** checks whether there were any errors in constructor */
    WriterError hasError();

    /** retrieves error message
     * @note added in 1.5
     */
    QString errorMessage();

    /** add feature to the currently opened shapefile */
    bool addFeature( QgsFeature& feature, QgsFeatureRendererV2* renderer = 0, QGis::UnitType outputUnit = QGis::Meters );

    //! @note not available in python bindings
    QMap<int, int> attrIdxToOgrIdx() { return mAttrIdxToOgrIdx; }

    /** close opened shapefile for writing */
    ~QgsVectorFileWriter();

    /** Delete a shapefile (and its accompanying shx / dbf / prf)
     * @param theFileName /path/to/file.shp
     * @return bool true if the file was deleted successfully
     */
    static bool deleteShapeFile( QString theFileName );

    SymbologyExport symbologyExport() const { return mSymbologyExport; }
    void setSymbologyExport( SymbologyExport symExport ) { mSymbologyExport = symExport; }

    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }
    void setSymbologyScaleDenominator( double d ) { mSymbologyScaleDenominator = d; }
    
    /**Returns OGRLayer used to begin and commit transactions on database*/
    OGRLayerH getLayer() { return mLayer; }

  protected:
    //! @note not available in python bindings
    OGRGeometryH createEmptyGeometry( QGis::WkbType wkbType );

    OGRDataSourceH mDS;
    OGRLayerH mLayer;
    OGRGeometryH mGeom;

    QgsFields mFields;

    /** contains error value if construction was not successful */
    WriterError mError;
    QString mErrorMessage;

    QTextCodec *mCodec;

    /** geometry type which is being used */
    QGis::WkbType mWkbType;

    /** map attribute indizes to OGR field indexes */
    QMap<int, int> mAttrIdxToOgrIdx;

    SymbologyExport mSymbologyExport;

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1700
    QMap< QgsSymbolLayerV2*, QString > mSymbolLayerTable;
#endif

    /**Scale for symbology export (e.g. for symbols units in map units)*/
    double mSymbologyScaleDenominator;

  private:
    static bool driverMetadata( QString driverName, QString &longName, QString &trLongName, QString &glob, QString &ext );
    void createSymbolLayerTable( QgsVectorLayer* vl,  const QgsCoordinateTransform* ct, OGRDataSourceH ds );
    OGRFeatureH createFeature( QgsFeature& feature );
    bool writeFeature( OGRLayerH layer, OGRFeatureH feature );

    /**Writes features considering symbol level order*/
    WriterError exportFeaturesSymbolLevels( QgsVectorLayer* layer, QgsFeatureIterator& fit, const QgsCoordinateTransform* ct, QString* errorMessage = 0 );
    double mmScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );
    double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );
    QgsRenderContext renderContext() const;
    void startRender( QgsVectorLayer* vl ) const;
    void stopRender( QgsVectorLayer* vl ) const;
    QgsFeatureRendererV2* symbologyRenderer( QgsVectorLayer* vl ) const;
    /**Adds attributes needed for classification*/
    void addRendererAttributes( QgsVectorLayer* vl, QgsAttributeList& attList );

};

#endif
