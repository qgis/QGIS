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

#ifndef QGSVECTORFILEWRITER_H
#define QGSVECTORFILEWRITER_H

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
 1. static call to QgsVectorFileWriter::writeAsVectorFormat(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)
 */
class CORE_EXPORT QgsVectorFileWriter
{
  public:
    enum OptionType
    {
      Set,
      String,
      Int,
      Hidden
    };

    /** \ingroup core
     */
    class Option
    {
      public:
        Option( const QString& docString, OptionType type )
            : docString( docString )
            , type( type ) {}
        virtual ~Option() {}

        QString docString;
        OptionType type;
    };

    /** \ingroup core
     */
    class SetOption : public Option
    {
      public:
        SetOption( const QString& docString, const QStringList& values, const QString& defaultValue, bool allowNone = false )
            : Option( docString, Set )
            , values( values.toSet() )
            , defaultValue( defaultValue )
            , allowNone( allowNone )
        {}

        QSet<QString> values;
        QString defaultValue;
        bool allowNone;
    };

    /** \ingroup core
     */
    class StringOption: public Option
    {
      public:
        StringOption( const QString& docString, const QString& defaultValue = QString() )
            : Option( docString, String )
            , defaultValue( defaultValue )
        {}

        QString defaultValue;
    };

    /** \ingroup core
     */
    class IntOption: public Option
    {
      public:
        IntOption( const QString& docString, int defaultValue )
            : Option( docString, Int )
            , defaultValue( defaultValue )
        {}

        int defaultValue;
    };

    /** \ingroup core
     */
    class BoolOption : public SetOption
    {
      public:
        BoolOption( const QString& docString, bool defaultValue )
            : SetOption( docString, QStringList() << "YES" << "NO", defaultValue ? "YES" : "NO" )
        {}
    };

    /** \ingroup core
     */
    class HiddenOption : public Option
    {
      public:
        explicit HiddenOption( const QString& value )
            : Option( "", Hidden )
            , mValue( value )
        {}

        QString mValue;
    };

    struct MetaData
    {
      MetaData()
      {}

      MetaData( const QString& longName, const QString& trLongName, const QString& glob, const QString& ext, const QMap<QString, Option*>& driverOptions, const QMap<QString, Option*>& layerOptions, const QString& compulsoryEncoding = QString() )
          : longName( longName )
          , trLongName( trLongName )
          , glob( glob )
          , ext( ext )
          , driverOptions( driverOptions )
          , layerOptions( layerOptions )
          , compulsoryEncoding( compulsoryEncoding )
      {}

      QString longName;
      QString trLongName;
      QString glob;
      QString ext;
      QMap<QString, Option*> driverOptions;
      QMap<QString, Option*> layerOptions;
      /** Some formats require a compulsory encoding, typically UTF-8. If no compulsory encoding, empty string */
      QString compulsoryEncoding;
    };

    enum WriterError
    {
      NoError = 0,
      ErrDriverNotFound,
      ErrCreateDataSource,
      ErrCreateLayer,
      ErrAttributeTypeUnsupported,
      ErrAttributeCreationFailed,
      ErrProjection,
      ErrFeatureWriteFailed,
      ErrInvalidLayer,
    };

    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    /** \ingroup core
     * Interface to convert raw field values to their user-friendly value.
     * @note Added in QGIS 2.16
     */
    class CORE_EXPORT FieldValueConverter
    {
      public:
        /** Constructor */
        FieldValueConverter();

        /** Destructor */
        virtual ~FieldValueConverter();

        /** Return a possibly modified field definition. Default implementation will return provided field unmodified.
         * @param field original field definition
         * @return possibly modified field definition
         */
        virtual QgsField fieldDefinition( const QgsField& field );

        /** Convert the provided value, for field fieldIdxInLayer. Default implementation will return provided value unmodified.
         * @param fieldIdxInLayer field index
         * @param value original raw value
         * @return possibly modified value.
         */
        virtual QVariant convert( int fieldIdxInLayer, const QVariant& value );
    };

    /** Write contents of vector layer to an (OGR supported) vector formt
     * @param layer layer to write
     * @param fileName file name to write to
     * @param fileEncoding encoding to use
     * @param destCRS pointer to CRS to reproject exported geometries to
     * @param driverName OGR driver to use
     * @param onlySelected write only selected features of layer
     * @param errorMessage pointer to buffer fo error message
     * @param datasourceOptions list of OGR data source creation options
     * @param layerOptions list of OGR layer creation options
     * @param skipAttributeCreation only write geometries
     * @param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * @param symbologyExport symbology to export
     * @param symbologyScale scale of symbology
     * @param filterExtent if not a null pointer, only features intersecting the extent will be saved (added in QGIS 2.4)
     * @param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
     * allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
     * @param forceMulti set to true to force creation of multi* geometries (added in QGIS 2.14)
     * @param includeZ set to true to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
     * @param attributes attributes to export (empty means all unless skipAttributeCreation is set)
     * @param fieldValueConverter field value converter (added in QGIS 2.16)
     */
    static WriterError writeAsVectorFormat( QgsVectorLayer* layer,
                                            const QString& fileName,
                                            const QString& fileEncoding,
                                            const QgsCoordinateReferenceSystem *destCRS,
                                            const QString& driverName = "ESRI Shapefile",
                                            bool onlySelected = false,
                                            QString *errorMessage = nullptr,
                                            const QStringList &datasourceOptions = QStringList(),
                                            const QStringList &layerOptions = QStringList(),
                                            bool skipAttributeCreation = false,
                                            QString *newFilename = nullptr,
                                            SymbologyExport symbologyExport = NoSymbology,
                                            double symbologyScale = 1.0,
                                            const QgsRectangle* filterExtent = nullptr,
                                            QgsWKBTypes::Type overrideGeometryType = QgsWKBTypes::Unknown,
                                            bool forceMulti = false,
                                            bool includeZ = false,
                                            QgsAttributeList attributes = QgsAttributeList(),
                                            FieldValueConverter* fieldValueConverter = nullptr
                                          );

    /** Writes a layer out to a vector file.
     * @param layer layer to write
     * @param fileName file name to write to
     * @param fileEncoding encoding to use
     * @param ct pointer to coordinate transform to reproject exported geometries with
     * @param driverName OGR driver to use
     * @param onlySelected write only selected features of layer
     * @param errorMessage pointer to buffer fo error message
     * @param datasourceOptions list of OGR data source creation options
     * @param layerOptions list of OGR layer creation options
     * @param skipAttributeCreation only write geometries
     * @param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * @param symbologyExport symbology to export
     * @param symbologyScale scale of symbology
     * @param filterExtent if not a null pointer, only features intersecting the extent will be saved (added in QGIS 2.4)
     * @param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
     * allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
     * @param forceMulti set to true to force creation of multi* geometries (added in QGIS 2.14)
     * @param includeZ set to true to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
     * @param attributes attributes to export (empty means all unless skipAttributeCreation is set)
     * @param fieldValueConverter field value converter (added in QGIS 2.16)
     * @note added in 2.2
     */
    static WriterError writeAsVectorFormat( QgsVectorLayer* layer,
                                            const QString& fileName,
                                            const QString& fileEncoding,
                                            const QgsCoordinateTransform* ct,
                                            const QString& driverName = "ESRI Shapefile",
                                            bool onlySelected = false,
                                            QString *errorMessage = nullptr,
                                            const QStringList &datasourceOptions = QStringList(),
                                            const QStringList &layerOptions = QStringList(),
                                            bool skipAttributeCreation = false,
                                            QString *newFilename = nullptr,
                                            SymbologyExport symbologyExport = NoSymbology,
                                            double symbologyScale = 1.0,
                                            const QgsRectangle* filterExtent = nullptr,
                                            QgsWKBTypes::Type overrideGeometryType = QgsWKBTypes::Unknown,
                                            bool forceMulti = false,
                                            bool includeZ = false,
                                            QgsAttributeList attributes = QgsAttributeList(),
                                            FieldValueConverter* fieldValueConverter = nullptr
                                          );

    /** Create a new vector file writer */
    QgsVectorFileWriter( const QString& vectorFileName,
                         const QString& fileEncoding,
                         const QgsFields& fields,
                         QGis::WkbType geometryType,
                         const QgsCoordinateReferenceSystem* srs,
                         const QString& driverName = "ESRI Shapefile",
                         const QStringList &datasourceOptions = QStringList(),
                         const QStringList &layerOptions = QStringList(),
                         QString *newFilename = nullptr,
                         SymbologyExport symbologyExport = NoSymbology
                       );

    /** Create a new vector file writer */
    QgsVectorFileWriter( const QString& vectorFileName,
                         const QString& fileEncoding,
                         const QgsFields& fields,
                         QgsWKBTypes::Type geometryType,
                         const QgsCoordinateReferenceSystem* srs,
                         const QString& driverName = "ESRI Shapefile",
                         const QStringList &datasourceOptions = QStringList(),
                         const QStringList &layerOptions = QStringList(),
                         QString *newFilename = nullptr,
                         SymbologyExport symbologyExport = NoSymbology
                       );

    /** Returns map with format filter string as key and OGR format key as value*/
    static QMap< QString, QString> supportedFiltersAndFormats();

    /** Returns driver list that can be used for dialogs. It contains all OGR drivers
     * + some additional internal QGIS driver names to distinguish between more
     * supported formats of the same OGR driver
     */
    static QMap< QString, QString> ogrDriverList();

    /** Returns filter string that can be used for dialogs*/
    static QString fileFilterString();

    /** Creates a filter for an OGR driver key*/
    static QString filterForDriver( const QString& driverName );

    /** Converts codec name to string passed to ENCODING layer creation option of OGR Shapefile*/
    static QString convertCodecNameForEncodingOption( const QString &codecName );

    /** Checks whether there were any errors in constructor */
    WriterError hasError();

    /** Retrieves error message */
    QString errorMessage();

    /** Add feature to the currently opened data source */
    bool addFeature( QgsFeature& feature, QgsFeatureRendererV2* renderer = nullptr, QGis::UnitType outputUnit = QGis::Meters );

    //! @note not available in python bindings
    QMap<int, int> attrIdxToOgrIdx() { return mAttrIdxToOgrIdx; }

    /** Close opened shapefile for writing */
    ~QgsVectorFileWriter();

    /** Delete a shapefile (and its accompanying shx / dbf / prf)
     * @param theFileName /path/to/file.shp
     * @return bool true if the file was deleted successfully
     */
    static bool deleteShapeFile( const QString& theFileName );

    SymbologyExport symbologyExport() const { return mSymbologyExport; }
    void setSymbologyExport( SymbologyExport symExport ) { mSymbologyExport = symExport; }

    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }
    void setSymbologyScaleDenominator( double d );

    static bool driverMetadata( const QString& driverName, MetaData& driverMetadata );

    /**
     * Get the ogr geometry type from an internal QGIS wkb type enum.
     *
     * Will drop M values and convert Z to 2.5D where required.
     * @note not available in python bindings
     */
    static OGRwkbGeometryType ogrTypeFromWkbType( QgsWKBTypes::Type type );

  protected:
    //! @note not available in python bindings
    OGRGeometryH createEmptyGeometry( QgsWKBTypes::Type wkbType );

    OGRDataSourceH mDS;
    OGRLayerH mLayer;
    OGRSpatialReferenceH mOgrRef;
    OGRGeometryH mGeom;

    QgsFields mFields;

    /** Contains error value if construction was not successful */
    WriterError mError;
    QString mErrorMessage;

    QTextCodec *mCodec;

    /** Geometry type which is being used */
    QgsWKBTypes::Type mWkbType;

    /** Map attribute indizes to OGR field indexes */
    QMap<int, int> mAttrIdxToOgrIdx;

    SymbologyExport mSymbologyExport;

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1700
    QMap< QgsSymbolLayerV2*, QString > mSymbolLayerTable;
#endif

    /** Scale for symbology export (e.g. for symbols units in map units)*/
    double mSymbologyScaleDenominator;

    QString mOgrDriverName;

    /** Field value converter */
    FieldValueConverter* mFieldValueConverter;

  private:

    /** Create a new vector file writer.
     * @param vectorFileName file name to write to
     * @param fileEncoding encoding to use
     * @param fields fields to write
     * @param geometryType geometry type of output file
     * @param srs spatial reference system of output file
     * @param driverName OGR driver to use
     * @param datasourceOptions list of OGR data source creation options
     * @param layerOptions list of OGR layer creation options
     * @param newFilename potentially modified file name (output parameter)
     * @param symbologyExport symbology to export
     * @param fieldValueConverter field value converter (added in QGIS 2.16)
     */
    QgsVectorFileWriter( const QString& vectorFileName,
                         const QString& fileEncoding,
                         const QgsFields& fields,
                         QgsWKBTypes::Type geometryType,
                         const QgsCoordinateReferenceSystem* srs,
                         const QString& driverName,
                         const QStringList &datasourceOptions,
                         const QStringList &layerOptions,
                         QString *newFilename,
                         SymbologyExport symbologyExport,
                         FieldValueConverter* fieldValueConverter
                       );

    void init( QString vectorFileName, QString fileEncoding, const QgsFields& fields,
               QgsWKBTypes::Type geometryType, const QgsCoordinateReferenceSystem* srs,
               const QString& driverName, QStringList datasourceOptions,
               QStringList layerOptions, QString* newFilename,
               FieldValueConverter* fieldValueConverter );
    void resetMap( const QgsAttributeList &attributes );

    QgsRenderContext mRenderContext;

    static QMap<QString, MetaData> initMetaData();
    void createSymbolLayerTable( QgsVectorLayer* vl,  const QgsCoordinateTransform* ct, OGRDataSourceH ds );
    OGRFeatureH createFeature( QgsFeature& feature );
    bool writeFeature( OGRLayerH layer, OGRFeatureH feature );

    /** Writes features considering symbol level order*/
    WriterError exportFeaturesSymbolLevels( QgsVectorLayer* layer, QgsFeatureIterator& fit, const QgsCoordinateTransform* ct, QString* errorMessage = nullptr );
    double mmScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );
    double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );

    void startRender( QgsVectorLayer* vl );
    void stopRender( QgsVectorLayer* vl );
    QgsFeatureRendererV2* symbologyRenderer( QgsVectorLayer* vl ) const;
    /** Adds attributes needed for classification*/
    void addRendererAttributes( QgsVectorLayer* vl, QgsAttributeList& attList );
    static QMap<QString, MetaData> sDriverMetadata;

    QgsVectorFileWriter( const QgsVectorFileWriter& rh );
    QgsVectorFileWriter& operator=( const QgsVectorFileWriter& rh );
};

#endif
