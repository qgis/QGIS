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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfields.h"
#include "qgsfeedback.h"
#include "qgstaskmanager.h"
#include "qgsogrutils.h"
#include "qgsrenderer.h"
#include "qgsgeometryengine.h"
#include "qgsfeaturesink.h"
#include "qgsrendercontext.h"
#include <ogr_api.h>

class QgsSymbolLayer;
class QTextCodec;
class QgsFeatureIterator;

/**
 * \ingroup core
 * \brief A convenience class for writing vector layers to disk based formats (e.g. Shapefiles, GeoPackage).
 *
 * There are two possibilities how to use this class:
 *
 * 1. A static call to QgsVectorFileWriter::writeAsVectorFormat(...) which saves the whole vector layer.
 * 2. Create an instance of the class and issue calls to addFeature(...).
 */
class CORE_EXPORT QgsVectorFileWriter : public QgsFeatureSink
{
  public:
    enum OptionType
    {
      Set,
      String,
      Int,
      Hidden
    };

    /**
     * \ingroup core
     */
    class Option
    {
      public:
        Option( const QString &docString, QgsVectorFileWriter::OptionType type )
          : docString( docString )
          , type( type ) {}
        virtual ~Option() = default;

        QString docString;
        QgsVectorFileWriter::OptionType type;
    };

    /**
     * \ingroup core
     */
    class SetOption : public QgsVectorFileWriter::Option
    {
      public:
        SetOption( const QString &docString, const QStringList &values, const QString &defaultValue, bool allowNone = false )
          : Option( docString, Set )
          , values( values.begin(), values.end() )
          , defaultValue( defaultValue )
          , allowNone( allowNone )
        {}

        QSet<QString> values;
        QString defaultValue;
        bool allowNone;
    };

    /**
     * \ingroup core
     */
    class StringOption: public QgsVectorFileWriter::Option
    {
      public:
        StringOption( const QString &docString, const QString &defaultValue = QString() )
          : Option( docString, String )
          , defaultValue( defaultValue )
        {}

        QString defaultValue;
    };

    /**
     * \ingroup core
     */
    class IntOption: public QgsVectorFileWriter::Option
    {
      public:
        IntOption( const QString &docString, int defaultValue )
          : Option( docString, Int )
          , defaultValue( defaultValue )
        {}

        int defaultValue;
    };

    /**
     * \ingroup core
     */
    class BoolOption : public QgsVectorFileWriter::SetOption
    {
      public:
        BoolOption( const QString &docString, bool defaultValue )
          : SetOption( docString, QStringList() << QStringLiteral( "YES" ) << QStringLiteral( "NO" ), defaultValue ? "YES" : "NO" )
        {}
    };

    /**
     * \ingroup core
     */
    class HiddenOption : public QgsVectorFileWriter::Option
    {
      public:
        explicit HiddenOption( const QString &value )
          : Option( QString(), Hidden )
          , mValue( value )
        {}

        QString mValue;
    };

    struct MetaData
    {
      //! Constructor for MetaData
      MetaData() = default;

      MetaData( const QString &longName, const QString &trLongName, const QString &glob, const QString &ext, const QMap<QString, QgsVectorFileWriter::Option *> &driverOptions, const QMap<QString, QgsVectorFileWriter::Option *> &layerOptions, const QString &compulsoryEncoding = QString() )
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
      QMap<QString, QgsVectorFileWriter::Option *> driverOptions;
      QMap<QString, QgsVectorFileWriter::Option *> layerOptions;
      //! Some formats require a compulsory encoding, typically UTF-8. If no compulsory encoding, empty string
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
      ErrSavingMetadata, //!< Metadata saving failed
      Canceled, //!< Writing was interrupted by manual cancellation
    };

    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    /**
     * Source for exported field names.
     *
     * \since QGIS 3.18
     */
    enum FieldNameSource
    {
      Original = 0, //!< Use original field names
      PreferAlias, //!< Use the field alias as the exported field name, wherever one is set. Otherwise use the original field names.
    };

    /**
     * Options for sorting and filtering vector formats.
     * \since QGIS 3.0
     */
    enum VectorFormatOption
    {
      SortRecommended = 1 << 1, //!< Use recommended sort order, with extremely commonly used formats listed first
      SkipNonSpatialFormats = 1 << 2, //!< Filter out any formats which do not have spatial support (e.g. those which cannot save geometries)
    };
    Q_DECLARE_FLAGS( VectorFormatOptions, VectorFormatOption )

    /**
     * \ingroup core
     * \brief Interface to convert raw field values to their user-friendly value.
     * \since QGIS 2.16
     */
    class CORE_EXPORT FieldValueConverter
    {
      public:
        //! Constructor
        FieldValueConverter() = default;

        virtual ~FieldValueConverter() = default;

        /**
         * Returns a possibly modified field definition. Default implementation will return provided field unmodified.
         * \param field original field definition
         * \returns possibly modified field definition
         */
        virtual QgsField fieldDefinition( const QgsField &field );

        /**
         * Convert the provided value, for field fieldIdxInLayer. Default implementation will return provided value unmodified.
         * \param fieldIdxInLayer field index
         * \param value original raw value
         * \returns possibly modified value.
         */
        virtual QVariant convert( int fieldIdxInLayer, const QVariant &value );

        /**
         * Creates a clone of the FieldValueConverter.
         */
        virtual QgsVectorFileWriter::FieldValueConverter *clone() const SIP_FACTORY;
    };

    /**
     * Edition capability flags
     * \since QGIS 3.0
    */
    enum EditionCapability
    {
      //! Flag to indicate that a new layer can be added to the dataset
      CanAddNewLayer                 = 1 << 0,

      //! Flag to indicate that new features can be added to an existing layer
      CanAppendToExistingLayer       = 1 << 1,

      //! Flag to indicate that new fields can be added to an existing layer. Imply CanAppendToExistingLayer
      CanAddNewFieldsToExistingLayer = 1 << 2,

      //! Flag to indicate that an existing layer can be deleted
      CanDeleteLayer                 = 1 << 3
    };

    /**
     * Combination of CanAddNewLayer, CanAppendToExistingLayer, CanAddNewFieldsToExistingLayer or CanDeleteLayer
     * \since QGIS 3.0.
    */
    Q_DECLARE_FLAGS( EditionCapabilities, EditionCapability )

    /**
     * Enumeration to describe how to handle existing files
     * \since QGIS 3.0
     */
    enum ActionOnExistingFile
    {
      //! Create or overwrite file
      CreateOrOverwriteFile,

      //! Create or overwrite layer
      CreateOrOverwriteLayer,

      //! Append features to existing layer, but do not create new fields
      AppendToLayerNoNewFields,

      //! Append features to existing layer, and create new fields if needed
      AppendToLayerAddFields
    };

#ifndef SIP_RUN

    /**
     * Write contents of vector layer to an (OGR supported) vector format
     * \param layer layer to write
     * \param fileName file name to write to
     * \param fileEncoding encoding to use
     * \param destCRS CRS to reproject exported geometries to, or invalid CRS for no reprojection
     * \param driverName OGR driver to use
     * \param onlySelected write only selected features of layer
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \param datasourceOptions list of OGR data source creation options
     * \param layerOptions list of OGR layer creation options
     * \param skipAttributeCreation only write geometries
     * \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * \param symbologyExport symbology to export
     * \param symbologyScale scale of symbology
     * \param filterExtent if not NULLPTR, only features intersecting the extent will be saved (added in QGIS 2.4)
     * \param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
     * allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
     * \param forceMulti set to TRUE to force creation of multi* geometries (added in QGIS 2.14)
     * \param includeZ set to TRUE to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
     * \param attributes attributes to export (empty means all unless skipAttributeCreation is set)
     * \param fieldValueConverter field value converter (added in QGIS 2.16)
     * \param newLayer QString pointer which will contain the new layer name created (in case it is different to the provided layer name) (added in QGIS 3.4, not available in python)
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
#else

    /**
     * Write contents of vector layer to an (OGR supported) vector format
     * \param layer layer to write
     * \param fileName file name to write to
     * \param fileEncoding encoding to use
     * \param destCRS CRS to reproject exported geometries to, or invalid CRS for no reprojection
     * \param driverName OGR driver to use
     * \param onlySelected write only selected features of layer
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \param datasourceOptions list of OGR data source creation options
     * \param layerOptions list of OGR layer creation options
     * \param skipAttributeCreation only write geometries
     * \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * \param symbologyExport symbology to export
     * \param symbologyScale scale of symbology
     * \param filterExtent if not NULLPTR, only features intersecting the extent will be saved (added in QGIS 2.4)
     * \param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
     * allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
     * \param forceMulti set to TRUE to force creation of multi* geometries (added in QGIS 2.14)
     * \param includeZ set to TRUE to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
     * \param attributes attributes to export (empty means all unless skipAttributeCreation is set)
     * \param fieldValueConverter field value converter (added in QGIS 2.16)
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
#endif
    Q_DECL_DEPRECATED static QgsVectorFileWriter::WriterError writeAsVectorFormat( QgsVectorLayer *layer,
        const QString &fileName,
        const QString &fileEncoding,
        const QgsCoordinateReferenceSystem &destCRS = QgsCoordinateReferenceSystem(),
        const QString &driverName = "GPKG",
        bool onlySelected = false,
        QString *errorMessage SIP_OUT = nullptr,
        const QStringList &datasourceOptions = QStringList(),
        const QStringList &layerOptions = QStringList(),
        bool skipAttributeCreation = false,
        QString *newFilename = nullptr,
        QgsVectorFileWriter::SymbologyExport symbologyExport = QgsVectorFileWriter::NoSymbology,
        double symbologyScale = 1.0,
        const QgsRectangle *filterExtent = nullptr,
        QgsWkbTypes::Type overrideGeometryType = QgsWkbTypes::Unknown,
        bool forceMulti = false,
        bool includeZ = false,
        const QgsAttributeList &attributes = QgsAttributeList(),
        QgsVectorFileWriter::FieldValueConverter *fieldValueConverter = nullptr
#ifndef SIP_RUN
            , QString *newLayer = nullptr );
#else
                                                                                 ) SIP_DEPRECATED;
#endif

#ifndef SIP_RUN

    /**
     * Writes a layer out to a vector file.
     * \param layer layer to write
     * \param fileName file name to write to
     * \param fileEncoding encoding to use
     * \param ct coordinate transform to reproject exported geometries with, or invalid transform
     * for no transformation
     * \param driverName OGR driver to use
     * \param onlySelected write only selected features of layer
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \param datasourceOptions list of OGR data source creation options
     * \param layerOptions list of OGR layer creation options
     * \param skipAttributeCreation only write geometries
     * \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * \param symbologyExport symbology to export
     * \param symbologyScale scale of symbology
     * \param filterExtent if not NULLPTR, only features intersecting the extent will be saved (added in QGIS 2.4)
     * \param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
     * allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
     * \param forceMulti set to TRUE to force creation of multi* geometries (added in QGIS 2.14)
     * \param includeZ set to TRUE to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
     * \param attributes attributes to export (empty means all unless skipAttributeCreation is set)
     * \param fieldValueConverter field value converter (added in QGIS 2.16)
     * \param newLayer QString pointer which will contain the new layer name created (in case it is different to the provided layer name) (added in QGIS 3.4, not available in python)
     * \since QGIS 2.2
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
#else

    /**
     * Writes a layer out to a vector file.
     * \param layer layer to write
     * \param fileName file name to write to
     * \param fileEncoding encoding to use
     * \param ct coordinate transform to reproject exported geometries with, or invalid transform
     * for no transformation
     * \param driverName OGR driver to use
     * \param onlySelected write only selected features of layer
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \param datasourceOptions list of OGR data source creation options
     * \param layerOptions list of OGR layer creation options
     * \param skipAttributeCreation only write geometries
     * \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * \param symbologyExport symbology to export
     * \param symbologyScale scale of symbology
     * \param filterExtent if not NULLPTR, only features intersecting the extent will be saved (added in QGIS 2.4)
     * \param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
     * allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
     * \param forceMulti set to TRUE to force creation of multi* geometries (added in QGIS 2.14)
     * \param includeZ set to TRUE to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
     * \param attributes attributes to export (empty means all unless skipAttributeCreation is set)
     * \param fieldValueConverter field value converter (added in QGIS 2.16)
     * \since QGIS 2.2
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
#endif
    Q_DECL_DEPRECATED static QgsVectorFileWriter::WriterError writeAsVectorFormat( QgsVectorLayer *layer,
        const QString &fileName,
        const QString &fileEncoding,
        const QgsCoordinateTransform &ct,
        const QString &driverName = "GPKG",
        bool onlySelected = false,
        QString *errorMessage SIP_OUT = nullptr,
        const QStringList &datasourceOptions = QStringList(),
        const QStringList &layerOptions = QStringList(),
        bool skipAttributeCreation = false,
        QString *newFilename = nullptr,
        QgsVectorFileWriter::SymbologyExport symbologyExport = QgsVectorFileWriter::NoSymbology,
        double symbologyScale = 1.0,
        const QgsRectangle *filterExtent = nullptr,
        QgsWkbTypes::Type overrideGeometryType = QgsWkbTypes::Unknown,
        bool forceMulti = false,
        bool includeZ = false,
        const QgsAttributeList &attributes = QgsAttributeList(),
        QgsVectorFileWriter::FieldValueConverter *fieldValueConverter = nullptr
#ifndef SIP_RUN
            , QString *newLayer = nullptr );
#else
                                                                                 ) SIP_DEPRECATED;
#endif

    /**
     * \ingroup core
     * \brief Options to pass to writeAsVectorFormat()
     * \since QGIS 3.0
     */
    class CORE_EXPORT SaveVectorOptions
    {
      public:
        //! Constructor
        SaveVectorOptions();

        virtual ~SaveVectorOptions() = default;

        //! OGR driver to use
        QString driverName;

        //! Layer name. If let empty, it will be derived from the filename
        QString layerName;

        //! Action on existing file
        QgsVectorFileWriter::ActionOnExistingFile actionOnExistingFile = CreateOrOverwriteFile;

        //! Encoding to use
        QString fileEncoding;

        /**
         * Transform to reproject exported geometries with, or invalid transform
         * for no transformation
        */
        QgsCoordinateTransform ct;

        //! Write only selected features of layer
        bool onlySelectedFeatures = false;

        //! List of OGR data source creation options
        QStringList datasourceOptions;

        //! List of OGR layer creation options
        QStringList layerOptions;

        //! Only write geometries
        bool skipAttributeCreation = false;

        //! Attributes to export (empty means all unless skipAttributeCreation is set)
        QgsAttributeList attributes;

        //! Attributes export names
        QStringList attributesExportNames;

        //! Symbology to export
        QgsVectorFileWriter::SymbologyExport symbologyExport = NoSymbology;

        //! Scale of symbology
        double symbologyScale = 1.0;

        //! If not empty, only features intersecting the extent will be saved
        QgsRectangle filterExtent;

        /**
         * Set to a valid geometry type to override the default geometry type for the layer. This parameter
         * allows for conversion of geometryless tables to null geometries, etc.
        */
        QgsWkbTypes::Type overrideGeometryType = QgsWkbTypes::Unknown;

        //! Sets to TRUE to force creation of multi* geometries
        bool forceMulti = false;

        //! Sets to TRUE to include z dimension in output. This option is only valid if overrideGeometryType is set
        bool includeZ = false;

        /**
         * Field value converter.
         *
         * Ownership is not transferred and callers must ensure that the lifetime of fieldValueConverter
         * exceeds the lifetime of the QgsVectorFileWriter object.
         */
        QgsVectorFileWriter::FieldValueConverter *fieldValueConverter = nullptr;

        //! Optional feedback object allowing cancellation of layer save
        QgsFeedback *feedback = nullptr;

        /**
         * Source for exported field names.
         *
         * \since QGIS 3.18
         */
        FieldNameSource fieldNameSource = Original;

        /**
         * Set to TRUE to save layer metadata for the exported vector file.
         *
         * \see layerMetadata
         * \since QGIS 3.20
         */
        bool saveMetadata = false;

        /**
         * Layer metadata to save for the exported vector file. This will only be used if saveMetadata is TRUE.
         *
         * \see saveMetadata
         * \since QGIS 3.20
         */
        QgsLayerMetadata layerMetadata;
    };

#ifndef SIP_RUN

    /**
     * Writes a layer out to a vector file.
     * \param layer source layer to write
     * \param fileName file name to write to
     * \param options options.
     * \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \param newLayer QString pointer which will contain the new layer name created (in case it is different to the provided layer name) (added in QGIS 3.4, not available in python)
     * \since QGIS 3.0
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
#else

    /**
     * Writes a layer out to a vector file.
     * \param layer source layer to write
     * \param fileName file name to write to
     * \param options options.
     * \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \since QGIS 3.0
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
#endif
    Q_DECL_DEPRECATED static QgsVectorFileWriter::WriterError writeAsVectorFormat( QgsVectorLayer *layer,
        const QString &fileName,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        QString *newFilename = nullptr,
        QString *errorMessage SIP_OUT = nullptr
#ifndef SIP_RUN
                                        , QString *newLayer = nullptr );
#else
                                                                                 ) SIP_DEPRECATED;
#endif

    /**
     * Create a new vector file writer
     * \deprecated Use create() instead.
     */
    Q_DECL_DEPRECATED QgsVectorFileWriter( const QString &vectorFileName,
                                           const QString &fileEncoding,
                                           const QgsFields &fields,
                                           QgsWkbTypes::Type geometryType,
                                           const QgsCoordinateReferenceSystem &srs = QgsCoordinateReferenceSystem(),
                                           const QString &driverName = "GPKG",
                                           const QStringList &datasourceOptions = QStringList(),
                                           const QStringList &layerOptions = QStringList(),
                                           QString *newFilename = nullptr,
                                           QgsVectorFileWriter::SymbologyExport symbologyExport = QgsVectorFileWriter::NoSymbology,
                                           QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags()
#ifndef SIP_RUN
                                               , QString *newLayer = nullptr,
                                           const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext(),
                                           FieldNameSource fieldNameSource = Original
#endif
                                         ) SIP_DEPRECATED;

    /**
     * Create a new vector file writer.
     * \param vectorFileName file name to write to
     * \param fileEncoding encoding to use
     * \param fields fields to write
     * \param geometryType geometry type of output file
     * \param srs spatial reference system of output file
     * \param driverName OGR driver to use
     * \param datasourceOptions list of OGR data source creation options
     * \param layerOptions list of OGR layer creation options
     * \param newFilename potentially modified file name (output parameter)
     * \param symbologyExport symbology to export
     * \param fieldValueConverter field value converter (added in QGIS 2.16)
     * \param layerName layer name. If let empty, it will be derived from the filename (added in QGIS 3.0)
     * \param action action on existing file (added in QGIS 3.0)
     * \param newLayer potentially modified layer name (output parameter) (added in QGIS 3.4)
     * \param transformContext transform context, needed if the output file srs is forced to specific crs (added in QGIS 3.10.3)
     * \param sinkFlags feature sink flags (added in QGIS 3.10.3)
     * \param fieldNameSource source for field names (since QGIS 3.18)
     * \note not available in Python bindings
     * \deprecated Use create() instead.
     */
    Q_DECL_DEPRECATED QgsVectorFileWriter( const QString &vectorFileName,
                                           const QString &fileEncoding,
                                           const QgsFields &fields,
                                           QgsWkbTypes::Type geometryType,
                                           const QgsCoordinateReferenceSystem &srs,
                                           const QString &driverName,
                                           const QStringList &datasourceOptions,
                                           const QStringList &layerOptions,
                                           QString *newFilename,
                                           QgsVectorFileWriter::SymbologyExport symbologyExport,
                                           QgsVectorFileWriter::FieldValueConverter *fieldValueConverter,
                                           const QString &layerName,
                                           QgsVectorFileWriter::ActionOnExistingFile action,
                                           QString *newLayer = nullptr,
                                           const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext(),
                                           QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags(),
                                           FieldNameSource fieldNameSource = Original
                                         ) SIP_SKIP;

    //! QgsVectorFileWriter cannot be copied.
    QgsVectorFileWriter( const QgsVectorFileWriter &rh ) = delete;
    //! QgsVectorFileWriter cannot be copied.
    QgsVectorFileWriter &operator=( const QgsVectorFileWriter &rh ) = delete;

    /**
     * Create a new vector file writer.
     * \param fileName file name to write to
     * \param fields fields to write
     * \param geometryType geometry type of output file
     * \param srs spatial reference system of output file
     * \param transformContext coordinate transform context
     * \param options save options
     * \param sinkFlags feature sink flags
     * \param newFilename potentially modified file name (output parameter)
     * \param newLayer potentially modified layer name (output parameter)
     * \since QGIS 3.10.3
     */
    static QgsVectorFileWriter *create( const QString &fileName,
                                        const QgsFields &fields,
                                        QgsWkbTypes::Type geometryType,
                                        const QgsCoordinateReferenceSystem &srs,
                                        const QgsCoordinateTransformContext &transformContext,
                                        const QgsVectorFileWriter::SaveVectorOptions &options,
                                        QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags(),
                                        QString *newFilename = nullptr,
                                        QString *newLayer = nullptr ) SIP_FACTORY;

    /**
     * Writes a layer out to a vector file.
     * \param layer source layer to write
     * \param fileName file name to write to
     * \param transformContext coordinate transform context
     * \param options save options
     * \param newFilename potentially modified file name (output parameter)
     * \param newLayer potentially modified layer name (output parameter)
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \returns Error message code, or QgsVectorFileWriter.NoError if the write operation was successful
     * \deprecated since QGIS 3.20, use writeAsVectorFormatV3 instead
     */
    Q_DECL_DEPRECATED static QgsVectorFileWriter::WriterError writeAsVectorFormatV2( QgsVectorLayer *layer,
        const QString &fileName,
        const QgsCoordinateTransformContext &transformContext,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        QString *newFilename = nullptr,
        QString *newLayer = nullptr,
        QString *errorMessage SIP_OUT = nullptr ) SIP_DEPRECATED;

    /**
     * Writes a layer out to a vector file.
     * \param layer source layer to write
     * \param fileName file name to write to
     * \param transformContext coordinate transform context
     * \param options save options
     * \param newFilename potentially modified file name (output parameter)
     * \param newLayer potentially modified layer name (output parameter)
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \returns Error message code, or QgsVectorFileWriter.NoError if the write operation was successful
     * \since QGIS 3.20
     */
    static QgsVectorFileWriter::WriterError writeAsVectorFormatV3( QgsVectorLayer *layer,
        const QString &fileName,
        const QgsCoordinateTransformContext &transformContext,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        QString *errorMessage SIP_OUT = nullptr,
        QString *newFilename SIP_OUT = nullptr,
        QString *newLayer SIP_OUT = nullptr );

    /**
     * Details of available filters and formats.
     * \since QGIS 3.0
     */
    struct FilterFormatDetails
    {
      //! Unique driver name
      QString driverName;

      //! Filter string for file picker dialogs
      QString filterString;

      /**
       * Matching glob patterns for format, e.g. *.shp.
       * \since QGIS 3.2
       */
      QStringList globs;
    };

    /**
     * Returns a list or pairs, with format filter string as first element and OGR format key as second element.
     *
     * The \a options argument can be used to control the sorting and filtering of
     * returned formats.
     *
     * \see supportedFormatExtensions()
     */
    static QList< QgsVectorFileWriter::FilterFormatDetails > supportedFiltersAndFormats( VectorFormatOptions options = SortRecommended );

    /**
     * Returns a list of file extensions for supported formats, e.g "shp", "gpkg".
     *
     * The \a options argument can be used to control the sorting and filtering of
     * returned formats.
     *
     * \see supportedFiltersAndFormats()
     * \since QGIS 3.0
     */
    static QStringList supportedFormatExtensions( VectorFormatOptions options = SortRecommended );

    /**
     * Returns TRUE if the specified \a driverName supports feature styles.
     *
     * The \a driverName argument must be a valid GDAL driver name.
     *
     * \since QGIS 3.0
     */
    static bool supportsFeatureStyles( const QString &driverName );

    /**
     * Details of available driver formats.
     * \since QGIS 3.0
     */
    struct DriverDetails
    {
      //! Descriptive, user friendly name for the driver
      QString longName;

      //! Unique driver name
      QString driverName;
    };

    /**
     * Returns the driver list that can be used for dialogs. It contains all OGR drivers
     * plus some additional internal QGIS driver names to distinguish between more
     * supported formats of the same OGR driver.
     *
     * The returned list consists of structs containing the driver long name (e.g. user-friendly
     * display name for the format) and internal driver short name.
     *
     * The \a options argument can be used to control the sorting and filtering of
     * returned drivers.
     */
    static QList< QgsVectorFileWriter::DriverDetails > ogrDriverList( VectorFormatOptions options = SortRecommended );

    /**
     * Returns the OGR driver name for a specified file \a extension. E.g. the
     * driver name for the ".shp" extension is "ESRI Shapefile".
     * If no suitable drivers are found then an empty string is returned.
     * \since QGIS 3.0
     */
    static QString driverForExtension( const QString &extension );

    /**
     * Returns filter string that can be used for dialogs.
     *
     * The \a options argument can be used to control the sorting and filtering of
     * returned drivers.
     */
    static QString fileFilterString( VectorFormatOptions options = SortRecommended );

    //! Creates a filter for an OGR driver key
    static QString filterForDriver( const QString &driverName );

    //! Converts codec name to string passed to ENCODING layer creation option of OGR Shapefile
    static QString convertCodecNameForEncodingOption( const QString &codecName );

    //! Checks whether there were any errors in constructor
    QgsVectorFileWriter::WriterError hasError() const;

    //! Retrieves error message
    QString errorMessage() const;

    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    QString lastError() const override;

    /**
     * Adds a \a feature to the currently opened data source, using the style from a specified \a renderer.
     * \since QGIS 3.0
     */
    bool addFeatureWithStyle( QgsFeature &feature, QgsFeatureRenderer *renderer, QgsUnitTypes::DistanceUnit outputUnit = QgsUnitTypes::DistanceMeters );

    //! \note not available in Python bindings
    QMap<int, int> attrIdxToOgrIdx() const { return mAttrIdxToOgrIdx; } SIP_SKIP

    //! Close opened shapefile for writing
    ~QgsVectorFileWriter() override;

    /**
     * Delete a shapefile (and its accompanying shx / dbf / prj / qix / qpj / cpg / sbn / sbx / idm / ind)
     * \param fileName /path/to/file.shp
     * \returns bool TRUE if the file was deleted successfully
     */
    static bool deleteShapeFile( const QString &fileName );

    QgsVectorFileWriter::SymbologyExport symbologyExport() const { return mSymbologyExport; }
    void setSymbologyExport( QgsVectorFileWriter::SymbologyExport symExport ) { mSymbologyExport = symExport; }

    /**
     * Returns the reference scale for output.
     * The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setSymbologyScale()
     * \since QGIS 3.0
     */
    double symbologyScale() const { return mSymbologyScale; }

    /**
     * Set reference \a scale for output.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see symbologyScale()
     * \since QGIS 3.0
     */
    void setSymbologyScale( double scale );

    static bool driverMetadata( const QString &driverName, MetaData &driverMetadata );

    /**
     * Returns a list of the default dataset options for a specified driver.
     * \param driverName name of OGR driver
     * \see defaultLayerOptions()
     * \since QGIS 3.0
     */
    static QStringList defaultDatasetOptions( const QString &driverName );

    /**
     * Returns a list of the default layer options for a specified driver.
     * \param driverName name of OGR driver
     * \see defaultDatasetOptions()
     * \since QGIS 3.0
     */
    static QStringList defaultLayerOptions( const QString &driverName );

    /**
     * Gets the ogr geometry type from an internal QGIS wkb type enum.
     *
     * Will drop M values and convert Z to 2.5D where required.
     * \note not available in Python bindings
     */
    static OGRwkbGeometryType ogrTypeFromWkbType( QgsWkbTypes::Type type ) SIP_SKIP;

    /**
     * Returns edition capabilities for an existing dataset name.
     * \since QGIS 3.0
     */
    static QgsVectorFileWriter::EditionCapabilities editionCapabilities( const QString &datasetName );

    /**
     * Returns whether the target layer already exists.
     * \since QGIS 3.0
     */
    static bool targetLayerExists( const QString &datasetName,
                                   const QString &layerName );

    /**
     * Returns whether there are among the attributes specified some that do not exist yet in the layer
     * \since QGIS 3.0
     */
    static bool areThereNewFieldsToCreate( const QString &datasetName,
                                           const QString &layerName,
                                           QgsVectorLayer *layer,
                                           const QgsAttributeList &attributes );

  protected:
    //! \note not available in Python bindings
    OGRGeometryH createEmptyGeometry( QgsWkbTypes::Type wkbType ) SIP_SKIP;

    gdal::ogr_datasource_unique_ptr mDS;
    OGRLayerH mLayer = nullptr;
    OGRSpatialReferenceH mOgrRef = nullptr;

    QgsFields mFields;

    //! Contains error value if construction was not successful
    WriterError mError;
    QString mErrorMessage;

    QTextCodec *mCodec = nullptr;

    //! Geometry type which is being used
    QgsWkbTypes::Type mWkbType;

    //! Map attribute indizes to OGR field indexes
    QMap<int, int> mAttrIdxToOgrIdx;

    SymbologyExport mSymbologyExport;

    QMap< QgsSymbolLayer *, QString > mSymbolLayerTable;

    //! Scale for symbology export (e.g. for symbols units in map units)
    double mSymbologyScale;

    QString mOgrDriverName;

    //! Field value converter
    FieldValueConverter *mFieldValueConverter = nullptr;

  private:
#ifdef SIP_RUN
    QgsVectorFileWriter( const QgsVectorFileWriter &rh );
#endif

    struct PreparedWriterDetails
    {
      std::unique_ptr< QgsFeatureRenderer > renderer;
      QgsCoordinateReferenceSystem sourceCrs;
      QgsWkbTypes::Type sourceWkbType = QgsWkbTypes::Unknown;
      QgsFields sourceFields;
      QString providerType;
      long long featureCount = 0;
      QgsFeatureIds selectedFeatureIds;
      QString dataSourceUri;
      QString storageType;
      QgsFeatureIterator geometryTypeScanIterator;
      QgsExpressionContext expressionContext;
      QSet< int > fieldsToConvertToInt;
      QgsRenderContext renderContext;
      bool shallTransform = false;
      QgsCoordinateReferenceSystem outputCrs;
      QgsWkbTypes::Type destWkbType = QgsWkbTypes::Unknown;
      QgsAttributeList attributes;
      QgsFields outputFields;
      QgsFeatureIterator sourceFeatureIterator;
      QgsGeometry filterRectGeometry;
      std::unique_ptr< QgsGeometryEngine  > filterRectEngine;
      QVariantMap providerUriParams;
    };

    /**
     * Prepares a write by populating a PreparedWriterDetails object.
     * This MUST be called in the main thread.
     */
    static QgsVectorFileWriter::WriterError prepareWriteAsVectorFormat( QgsVectorLayer *layer,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        PreparedWriterDetails &details );

    /**
     * Writes a previously prepared PreparedWriterDetails \a details object.
     * This is safe to call in a background thread.
     * \param details writer details
     * \param fileName file name to write to
     * \param transformContext coordinate transform context
     * \param options save options
     * \param newFilename potentially modified file name (output parameter)
     * \param newLayer potentially modified layer name (output parameter)
     * \param errorMessage will be set to the error message text, if an error occurs while writing the layer
     * \returns Error message code, or QgsVectorFileWriter.NoError if the write operation was successful
     * \since QGIS 3.10.3
     */
    static QgsVectorFileWriter::WriterError writeAsVectorFormatV2( PreparedWriterDetails &details,
        const QString &fileName,
        const QgsCoordinateTransformContext &transformContext,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        QString *newFilename = nullptr,
        QString *newLayer = nullptr,
        QString *errorMessage SIP_OUT = nullptr );

    /**
     * Writes a previously prepared PreparedWriterDetails \a details object.
     * This is safe to call in a background thread.
     * \deprecated Use writeAsVectorFormatV2() instead.
     */
    Q_DECL_DEPRECATED static QgsVectorFileWriter::WriterError writeAsVectorFormat( PreparedWriterDetails &details,
        const QString &fileName,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        QString *newFilename = nullptr,
        QString *errorMessage SIP_OUT = nullptr,
        QString *newLayer = nullptr ) SIP_DEPRECATED;

    void init( QString vectorFileName, QString fileEncoding, const QgsFields &fields,
               QgsWkbTypes::Type geometryType, QgsCoordinateReferenceSystem srs,
               const QString &driverName, QStringList datasourceOptions,
               QStringList layerOptions, QString *newFilename,
               QgsVectorFileWriter::FieldValueConverter *fieldValueConverter,
               const QString &layerName,
               QgsVectorFileWriter::ActionOnExistingFile action, QString *newLayer, QgsFeatureSink::SinkFlags sinkFlags,
               const QgsCoordinateTransformContext &transformContext,
               FieldNameSource fieldNameSource );
    void resetMap( const QgsAttributeList &attributes );

    std::unique_ptr< QgsFeatureRenderer > mRenderer;
    QgsRenderContext mRenderContext;


    std::unique_ptr< QgsCoordinateTransform > mCoordinateTransform;

    bool mUsingTransaction = false;
    QSet< QVariant::Type > mSupportedListSubTypes;

    void createSymbolLayerTable( QgsVectorLayer *vl, const QgsCoordinateTransform &ct, OGRDataSourceH ds );
    gdal::ogr_feature_unique_ptr createFeature( const QgsFeature &feature );
    bool writeFeature( OGRLayerH layer, OGRFeatureH feature );

    //! Writes features considering symbol level order
    QgsVectorFileWriter::WriterError exportFeaturesSymbolLevels( const PreparedWriterDetails &details, QgsFeatureIterator &fit, const QgsCoordinateTransform &ct, QString *errorMessage = nullptr );
    double mmScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits );
    double mapUnitScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits );

    void startRender( QgsFeatureRenderer *sourceRenderer, const QgsFields &fields );
    void stopRender();
    std::unique_ptr< QgsFeatureRenderer > createSymbologyRenderer( QgsFeatureRenderer *sourceRenderer ) const;
    //! Adds attributes needed for classification
    static void addRendererAttributes( QgsFeatureRenderer *renderer, QgsRenderContext &context, const QgsFields &fields, QgsAttributeList &attList );

    //! Concatenates a list of options using their default values
    static QStringList concatenateOptions( const QMap<QString, Option *> &options );

    friend class QgsVectorFileWriterTask;
    friend class TestQgsVectorFileWriter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsVectorFileWriter::EditionCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsVectorFileWriter::VectorFormatOptions )

// clazy:excludeall=qstring-allocations

#endif
