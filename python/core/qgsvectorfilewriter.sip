/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorfilewriter.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsVectorFileWriter : QgsFeatureSink
{
%Docstring
 A convenience class for writing vector files to disk.
There are two possibilities how to use this class:
1. static call to QgsVectorFileWriter.writeAsVectorFormat(...) which saves the whole vector layer
2. create an instance of the class and issue calls to addFeature(...)
%End

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
  public:
    enum OptionType
    {
      Set,
      String,
      Int,
      Hidden
    };

    class Option
{

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        Option( const QString &docString, QgsVectorFileWriter::OptionType type );

        QString docString;
        QgsVectorFileWriter::OptionType type;
    };

    class SetOption : QgsVectorFileWriter::Option
{

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        SetOption( const QString &docString, const QStringList &values, const QString &defaultValue, bool allowNone = false );

        QSet<QString> values;
        QString defaultValue;
        bool allowNone;
    };

    class StringOption: QgsVectorFileWriter::Option
{

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        StringOption( const QString &docString, const QString &defaultValue = QString() );

        QString defaultValue;
    };

    class IntOption: QgsVectorFileWriter::Option
{

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        IntOption( const QString &docString, int defaultValue );

        int defaultValue;
    };

    class BoolOption : QgsVectorFileWriter::SetOption
{

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        BoolOption( const QString &docString, bool defaultValue );
    };

    class HiddenOption : QgsVectorFileWriter::Option
{

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        explicit HiddenOption( const QString &value );

        QString mValue;
    };

    struct MetaData
    {
      MetaData();
%Docstring
Constructor for MetaData
%End

      MetaData( const QString &longName, const QString &trLongName, const QString &glob, const QString &ext, const QMap<QString, QgsVectorFileWriter::Option *> &driverOptions, const QMap<QString, QgsVectorFileWriter::Option *> &layerOptions, const QString &compulsoryEncoding = QString() );

      QString longName;
      QString trLongName;
      QString glob;
      QString ext;
      QMap<QString, QgsVectorFileWriter::Option *> driverOptions;
      QMap<QString, QgsVectorFileWriter::Option *> layerOptions;
      QString compulsoryEncoding;
%Docstring
Some formats require a compulsory encoding, typically UTF-8. If no compulsory encoding, empty string
%End
    };

    enum WriterError
    {
      NoError,
      ErrDriverNotFound,
      ErrCreateDataSource,
      ErrCreateLayer,
      ErrAttributeTypeUnsupported,
      ErrAttributeCreationFailed,
      ErrProjection,
      ErrFeatureWriteFailed,
      ErrInvalidLayer,
      Canceled,
    };

    enum SymbologyExport
    {
      NoSymbology,
      FeatureSymbology,
      SymbolLayerSymbology
    };


    enum VectorFormatOption
    {
      SortRecommended,
      SkipNonSpatialFormats,
    };
    typedef QFlags<QgsVectorFileWriter::VectorFormatOption> VectorFormatOptions;


    class FieldValueConverter
{
%Docstring
 Interface to convert raw field values to their user-friendly value.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        FieldValueConverter();
%Docstring
Constructor
%End

        virtual ~FieldValueConverter();

        virtual QgsField fieldDefinition( const QgsField &field );
%Docstring
 Return a possibly modified field definition. Default implementation will return provided field unmodified.
 \param field original field definition
 :return: possibly modified field definition
 :rtype: QgsField
%End

        virtual QVariant convert( int fieldIdxInLayer, const QVariant &value );
%Docstring
 Convert the provided value, for field fieldIdxInLayer. Default implementation will return provided value unmodified.
 \param fieldIdxInLayer field index
 \param value original raw value
 :return: possibly modified value.
 :rtype: QVariant
%End

        virtual QgsVectorFileWriter::FieldValueConverter *clone() const /Factory/;
%Docstring
 Creates a clone of the FieldValueConverter.
 :rtype: QgsVectorFileWriter.FieldValueConverter
%End
    };

    enum EditionCapability
    {
      //! Flag
      CanAddNewLayer,

      //! Flag
      CanAppendToExistingLayer,

      //! Flag
      CanAddNewFieldsToExistingLayer,

      //! Flag
      CanDeleteLayer
    };

    typedef QFlags<QgsVectorFileWriter::EditionCapability> EditionCapabilities;


    enum ActionOnExistingFile
    {
      //! Create
      CreateOrOverwriteFile,

      //! Create
      CreateOrOverwriteLayer,

      //! Append
      AppendToLayerNoNewFields,

      //! Append
      AppendToLayerAddFields
    };

    static QgsVectorFileWriter::WriterError writeAsVectorFormat( QgsVectorLayer *layer,
        const QString &fileName,
        const QString &fileEncoding,
        const QgsCoordinateReferenceSystem &destCRS = QgsCoordinateReferenceSystem(),
        const QString &driverName = "GPKG",
        bool onlySelected = false,
        QString *errorMessage /Out/ = 0,
        const QStringList &datasourceOptions = QStringList(),
        const QStringList &layerOptions = QStringList(),
        bool skipAttributeCreation = false,
        QString *newFilename = 0,
        QgsVectorFileWriter::SymbologyExport symbologyExport = QgsVectorFileWriter::NoSymbology,
        double symbologyScale = 1.0,
        const QgsRectangle *filterExtent = 0,
        QgsWkbTypes::Type overrideGeometryType = QgsWkbTypes::Unknown,
        bool forceMulti = false,
        bool includeZ = false,
        const QgsAttributeList &attributes = QgsAttributeList(),
        QgsVectorFileWriter::FieldValueConverter *fieldValueConverter = 0
                                                               );
%Docstring
 Write contents of vector layer to an (OGR supported) vector formt
 \param layer layer to write
 \param fileName file name to write to
 \param fileEncoding encoding to use
 \param destCRS CRS to reproject exported geometries to, or invalid CRS for no reprojection
 \param driverName OGR driver to use
 \param onlySelected write only selected features of layer
 \param errorMessage pointer to buffer fo error message
 \param datasourceOptions list of OGR data source creation options
 \param layerOptions list of OGR layer creation options
 \param skipAttributeCreation only write geometries
 \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
 \param symbologyExport symbology to export
 \param symbologyScale scale of symbology
 \param filterExtent if not a null pointer, only features intersecting the extent will be saved (added in QGIS 2.4)
 \param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
 allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
 \param forceMulti set to true to force creation of multi* geometries (added in QGIS 2.14)
 \param includeZ set to true to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
 \param attributes attributes to export (empty means all unless skipAttributeCreation is set)
 \param fieldValueConverter field value converter (added in QGIS 2.16)
 :rtype: QgsVectorFileWriter.WriterError
%End

    static QgsVectorFileWriter::WriterError writeAsVectorFormat( QgsVectorLayer *layer,
        const QString &fileName,
        const QString &fileEncoding,
        const QgsCoordinateTransform &ct,
        const QString &driverName = "GPKG",
        bool onlySelected = false,
        QString *errorMessage /Out/ = 0,
        const QStringList &datasourceOptions = QStringList(),
        const QStringList &layerOptions = QStringList(),
        bool skipAttributeCreation = false,
        QString *newFilename = 0,
        QgsVectorFileWriter::SymbologyExport symbologyExport = QgsVectorFileWriter::NoSymbology,
        double symbologyScale = 1.0,
        const QgsRectangle *filterExtent = 0,
        QgsWkbTypes::Type overrideGeometryType = QgsWkbTypes::Unknown,
        bool forceMulti = false,
        bool includeZ = false,
        const QgsAttributeList &attributes = QgsAttributeList(),
        QgsVectorFileWriter::FieldValueConverter *fieldValueConverter = 0
                                                               );
%Docstring
 Writes a layer out to a vector file.
 \param layer layer to write
 \param fileName file name to write to
 \param fileEncoding encoding to use
 \param ct coordinate transform to reproject exported geometries with, or invalid transform
 for no transformation
 \param driverName OGR driver to use
 \param onlySelected write only selected features of layer
 \param errorMessage pointer to buffer fo error message
 \param datasourceOptions list of OGR data source creation options
 \param layerOptions list of OGR layer creation options
 \param skipAttributeCreation only write geometries
 \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
 \param symbologyExport symbology to export
 \param symbologyScale scale of symbology
 \param filterExtent if not a null pointer, only features intersecting the extent will be saved (added in QGIS 2.4)
 \param overrideGeometryType set to a valid geometry type to override the default geometry type for the layer. This parameter
 allows for conversion of geometryless tables to null geometries, etc (added in QGIS 2.14)
 \param forceMulti set to true to force creation of multi* geometries (added in QGIS 2.14)
 \param includeZ set to true to include z dimension in output. This option is only valid if overrideGeometryType is set. (added in QGIS 2.14)
 \param attributes attributes to export (empty means all unless skipAttributeCreation is set)
 \param fieldValueConverter field value converter (added in QGIS 2.16)
.. versionadded:: 2.2
 :rtype: QgsVectorFileWriter.WriterError
%End


    class SaveVectorOptions
{
%Docstring
 Options to pass to writeAsVectorFormat()
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorfilewriter.h"
%End
      public:
        SaveVectorOptions();
%Docstring
Constructor
%End

        virtual ~SaveVectorOptions();

        QString driverName;
%Docstring
OGR driver to use
%End

        QString layerName;
%Docstring
Layer name. If let empty, it will be derived from the filename
%End

        QgsVectorFileWriter::ActionOnExistingFile actionOnExistingFile;
%Docstring
Action on existing file
%End

        QString fileEncoding;
%Docstring
Encoding to use
%End

        QgsCoordinateTransform ct;
%Docstring
 Transform to reproject exported geometries with, or invalid transform
 for no transformation *
%End

        bool onlySelectedFeatures;
%Docstring
Write only selected features of layer
%End

        QStringList datasourceOptions;
%Docstring
List of OGR data source creation options
%End

        QStringList layerOptions;
%Docstring
List of OGR layer creation options
%End

        bool skipAttributeCreation;
%Docstring
Only write geometries
%End

        QgsAttributeList attributes;
%Docstring
Attributes to export (empty means all unless skipAttributeCreation is set)
%End

        QgsVectorFileWriter::SymbologyExport symbologyExport;
%Docstring
Symbology to export
%End

        double symbologyScale;
%Docstring
Scale of symbology
%End

        QgsRectangle filterExtent;
%Docstring
If not empty, only features intersecting the extent will be saved
%End

        QgsWkbTypes::Type overrideGeometryType;
%Docstring
 Set to a valid geometry type to override the default geometry type for the layer. This parameter
 allows for conversion of geometryless tables to null geometries, etc *
%End

        bool forceMulti;
%Docstring
Set to true to force creation of multi* geometries
%End

        bool includeZ;
%Docstring
Set to true to include z dimension in output. This option is only valid if overrideGeometryType is set
%End

        QgsVectorFileWriter::FieldValueConverter *fieldValueConverter;
%Docstring
 Field value converter.

 Ownership is not transferred and callers must ensure that the lifetime of fieldValueConverter
 exceeds the lifetime of the QgsVectorFileWriter object.
%End

        QgsFeedback *feedback;
%Docstring
Optional feedback object allowing cancelation of layer save
%End
    };

    static QgsVectorFileWriter::WriterError writeAsVectorFormat( QgsVectorLayer *layer,
        const QString &fileName,
        const QgsVectorFileWriter::SaveVectorOptions &options,
        QString *newFilename = 0,
        QString *errorMessage /Out/ = 0 );
%Docstring
 Writes a layer out to a vector file.
 \param layer source layer to write
 \param fileName file name to write to
 \param options options.
 \param newFilename QString pointer which will contain the new file name created (in case it is different to fileName).
 \param errorMessage pointer to buffer fo error message
.. versionadded:: 3.0
 :rtype: QgsVectorFileWriter.WriterError
%End

    QgsVectorFileWriter( const QString &vectorFileName,
                         const QString &fileEncoding,
                         const QgsFields &fields,
                         QgsWkbTypes::Type geometryType,
                         const QgsCoordinateReferenceSystem &srs = QgsCoordinateReferenceSystem(),
                         const QString &driverName = "GPKG",
                         const QStringList &datasourceOptions = QStringList(),
                         const QStringList &layerOptions = QStringList(),
                         QString *newFilename = 0,
                         QgsVectorFileWriter::SymbologyExport symbologyExport = QgsVectorFileWriter::NoSymbology
                       );
%Docstring
Create a new vector file writer
%End



    struct FilterFormatDetails
    {
      QString driverName;
%Docstring
Unique driver name
%End

      QString filterString;
%Docstring
Filter string for file picker dialogs
%End
    };

    static QList< QgsVectorFileWriter::FilterFormatDetails > supportedFiltersAndFormats( VectorFormatOptions options = SortRecommended );
%Docstring
 Returns a list or pairs, with format filter string as first element and OGR format key as second element.

 The ``options`` argument can be used to control the sorting and filtering of
 returned formats.

.. seealso:: supportedOutputVectorLayerExtensions()
 :rtype: list of QgsVectorFileWriter.FilterFormatDetails
%End

    static QStringList supportedFormatExtensions( VectorFormatOptions options = SortRecommended );
%Docstring
 Returns a list of file extensions for supported formats.

 The ``options`` argument can be used to control the sorting and filtering of
 returned formats.

.. versionadded:: 3.0
.. seealso:: supportedFiltersAndFormats()
 :rtype: list of str
%End

    struct DriverDetails
    {
      QString longName;
%Docstring
Descriptive, user friendly name for the driver
%End

      QString driverName;
%Docstring
Unique driver name
%End
    };

    static QList< QgsVectorFileWriter::DriverDetails > ogrDriverList( VectorFormatOptions options = SortRecommended );
%Docstring
 Returns the driver list that can be used for dialogs. It contains all OGR drivers
 plus some additional internal QGIS driver names to distinguish between more
 supported formats of the same OGR driver.

 The returned list consists of structs containing the driver long name (e.g. user-friendly
 display name for the format) and internal driver short name.

 The ``options`` argument can be used to control the sorting and filtering of
 returned drivers.
 :rtype: list of QgsVectorFileWriter.DriverDetails
%End

    static QString driverForExtension( const QString &extension );
%Docstring
 Returns the OGR driver name for a specified file ``extension``. E.g. the
 driver name for the ".shp" extension is "ESRI Shapefile".
 If no suitable drivers are found then an empty string is returned.
.. versionadded:: 3.0
 :rtype: str
%End

    static QString fileFilterString( VectorFormatOptions options = SortRecommended );
%Docstring
 Returns filter string that can be used for dialogs.

 The ``options`` argument can be used to control the sorting and filtering of
 returned drivers.
 :rtype: str
%End

    static QString filterForDriver( const QString &driverName );
%Docstring
Creates a filter for an OGR driver key
 :rtype: str
%End

    static QString convertCodecNameForEncodingOption( const QString &codecName );
%Docstring
Converts codec name to string passed to ENCODING layer creation option of OGR Shapefile
 :rtype: str
%End

    QgsVectorFileWriter::WriterError hasError();
%Docstring
Checks whether there were any errors in constructor
 :rtype: QgsVectorFileWriter.WriterError
%End

    QString errorMessage();
%Docstring
Retrieves error message
 :rtype: str
%End

    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = 0 );

    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = 0 );


    bool addFeatureWithStyle( QgsFeature &feature, QgsFeatureRenderer *renderer, QgsUnitTypes::DistanceUnit outputUnit = QgsUnitTypes::DistanceMeters );
%Docstring
 Adds a ``feature`` to the currently opened data source, using the style from a specified ``renderer``.
.. versionadded:: 3.0
 :rtype: bool
%End


    ~QgsVectorFileWriter();
%Docstring
Close opened shapefile for writing
%End

    static bool deleteShapeFile( const QString &fileName );
%Docstring
 Delete a shapefile (and its accompanying shx / dbf / prf)
 \param fileName /path/to/file.shp
 :return: bool true if the file was deleted successfully
 :rtype: bool
%End

    QgsVectorFileWriter::SymbologyExport symbologyExport() const;
%Docstring
 :rtype: QgsVectorFileWriter.SymbologyExport
%End
    void setSymbologyExport( QgsVectorFileWriter::SymbologyExport symExport );

    double symbologyScale() const;
%Docstring
 Returns the reference scale for output.
 The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
.. versionadded:: 3.0
.. seealso:: setSymbologyScale()
 :rtype: float
%End

    void setSymbologyScale( double scale );
%Docstring
 Set reference ``scale`` for output.
 The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
.. versionadded:: 3.0
.. seealso:: symbologyScale()
%End

    static bool driverMetadata( const QString &driverName, MetaData &driverMetadata );
%Docstring
 :rtype: bool
%End

    static QStringList defaultDatasetOptions( const QString &driverName );
%Docstring
 Returns a list of the default dataset options for a specified driver.
 \param driverName name of OGR driver
.. versionadded:: 3.0
.. seealso:: defaultLayerOptions()
 :rtype: list of str
%End

    static QStringList defaultLayerOptions( const QString &driverName );
%Docstring
 Returns a list of the default layer options for a specified driver.
 \param driverName name of OGR driver
.. versionadded:: 3.0
.. seealso:: defaultDatasetOptions()
 :rtype: list of str
%End


    static QgsVectorFileWriter::EditionCapabilities editionCapabilities( const QString &datasetName );
%Docstring
 Return edition capabilities for an existing dataset name.
.. versionadded:: 3.0
 :rtype: QgsVectorFileWriter.EditionCapabilities
%End

    static bool targetLayerExists( const QString &datasetName,
                                   const QString &layerName );
%Docstring
 Returns whether the target layer already exists.
.. versionadded:: 3.0
 :rtype: bool
%End

    static bool areThereNewFieldsToCreate( const QString &datasetName,
                                           const QString &layerName,
                                           QgsVectorLayer *layer,
                                           const QgsAttributeList &attributes );
%Docstring
 Returns whether there are among the attributes specified some that do not exist yet in the layer
.. versionadded:: 3.0
 :rtype: bool
%End

  protected:












  private:
    QgsVectorFileWriter( const QgsVectorFileWriter &rh );
};

QFlags<QgsVectorFileWriter::EditionCapability> operator|(QgsVectorFileWriter::EditionCapability f1, QFlags<QgsVectorFileWriter::EditionCapability> f2);

QFlags<QgsVectorFileWriter::VectorFormatOption> operator|(QgsVectorFileWriter::VectorFormatOption f1, QFlags<QgsVectorFileWriter::VectorFormatOption> f2);



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorfilewriter.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
