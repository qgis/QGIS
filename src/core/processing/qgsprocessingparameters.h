/***************************************************************************
                         qgsprocessingparameters.h
                         -------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERS_H
#define QGSPROCESSINGPARAMETERS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsproperty.h"
#include "qgscoordinatereferencesystem.h"
#include <QMap>
#include <limits>

class QgsProcessingContext;
class QgsRasterLayer;
class QgsVectorLayer;
class QgsFeatureSink;
class QgsFeatureSource;
class QgsProcessingOutputDefinition;

/**
 * \class QgsProcessingFeatureSourceDefinition
 * \ingroup core
 *
 * Encapsulates settings relating to a feature source input to a processing algorithm.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingFeatureSourceDefinition
{
  public:

    /**
     * Constructor for QgsProcessingFeatureSourceDefinition, accepting a static string source.
     */
    QgsProcessingFeatureSourceDefinition( const QString &source = QString(), bool selectedFeaturesOnly = false )
      : source( QgsProperty::fromValue( source ) )
      , selectedFeaturesOnly( selectedFeaturesOnly )
    {}

    /**
     * Constructor for QgsProcessingFeatureSourceDefinition, accepting a QgsProperty source.
     */
    QgsProcessingFeatureSourceDefinition( const QgsProperty &source, bool selectedFeaturesOnly = false )
      : source( source )
      , selectedFeaturesOnly( selectedFeaturesOnly )
    {}

    /**
     * Source definition. Usually a static property set to a source layer's ID or file name.
     */
    QgsProperty source;

    /**
     * True if only selected features in the source should be used by algorithms.
     */
    bool selectedFeaturesOnly;


    //! Allows direct construction of QVariants.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

};

Q_DECLARE_METATYPE( QgsProcessingFeatureSourceDefinition )

/**
 * \class QgsProcessingOutputLayerDefinition
 * \ingroup core
 *
 * Encapsulates settings relating to a feature sink or output raster layer for a processing algorithm.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingOutputLayerDefinition
{
  public:

    /**
     * Constructor for QgsProcessingOutputLayerDefinition, accepting a static sink/layer string.
     * The \a destinationProject parameter can be set to a QgsProject instance in which
     * to automatically load the resulting sink/layer after completing processing.
     */
    QgsProcessingOutputLayerDefinition( const QString &sink = QString(), QgsProject *destinationProject = nullptr )
      : sink( QgsProperty::fromValue( sink ) )
      , destinationProject( destinationProject )
    {}

    /**
     * Constructor for QgsProcessingOutputLayerDefinition, accepting a QgsProperty sink/layer.
     * The \a destinationProject parameter can be set to a QgsProject instance in which
     * to automatically load the resulting sink/layer after completing processing.
     */
    QgsProcessingOutputLayerDefinition( const QgsProperty &sink, QgsProject *destinationProject = nullptr )
      : sink( sink )
      , destinationProject( destinationProject )
    {}

    /**
     * Sink/layer definition. Usually a static property set to the destination file name for the sink's layer.
     */
    QgsProperty sink;

    /**
     * Destination project. Can be set to a QgsProject instance in which
     * to automatically load the resulting sink/layer after completing processing.
     * The default behavior is not to load the result into any project (nullptr).
     */
    QgsProject *destinationProject;

    /**
     * Map of optional sink/layer creation options, which
     * are passed to the underlying provider when creating new layers. Known options also
     * include 'fileEncoding', which is used to specify a file encoding to use for created
     * files.
     */
    QVariantMap createOptions;


    //! Allows direct construction of QVariants.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

};

Q_DECLARE_METATYPE( QgsProcessingOutputLayerDefinition )




//
// Parameter definitions
//

/**
 * \class QgsProcessingParameterDefinition
 * \ingroup core
 *
 * Base class for the definition of processing parameters.
 *
 * Parameter definitions encapsulate properties regarding the behavior of parameters,
 * their acceptable ranges, defaults, etc.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingParameterDefinition
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == "boolean" )
      sipType = sipType_QgsProcessingParameterBoolean;
    else if ( sipCpp->type() == "crs" )
      sipType = sipType_QgsProcessingParameterCrs;
    else if ( sipCpp->type() == "layer" )
      sipType = sipType_QgsProcessingParameterMapLayer;
    else if ( sipCpp->type() == "extent" )
      sipType = sipType_QgsProcessingParameterExtent;
    else if ( sipCpp->type() == "point" )
      sipType = sipType_QgsProcessingParameterPoint;
    else if ( sipCpp->type() == "file" )
      sipType = sipType_QgsProcessingParameterFile;
    else if ( sipCpp->type() == "matrix" )
      sipType = sipType_QgsProcessingParameterMatrix;
    else if ( sipCpp->type() == "multilayer" )
      sipType = sipType_QgsProcessingParameterMultipleLayers;
    else if ( sipCpp->type() == "number" )
      sipType = sipType_QgsProcessingParameterNumber;
    else if ( sipCpp->type() == "range" )
      sipType = sipType_QgsProcessingParameterRange;
    else if ( sipCpp->type() == "raster" )
      sipType = sipType_QgsProcessingParameterRasterLayer;
    else if ( sipCpp->type() == "enum" )
      sipType = sipType_QgsProcessingParameterEnum;
    else if ( sipCpp->type() == "string" )
      sipType = sipType_QgsProcessingParameterString;
    else if ( sipCpp->type() == "expression" )
      sipType = sipType_QgsProcessingParameterExpression;
    else if ( sipCpp->type() == "vector" )
      sipType = sipType_QgsProcessingParameterVectorLayer;
    else if ( sipCpp->type() == "field" )
      sipType = sipType_QgsProcessingParameterField;
    else if ( sipCpp->type() == "source" )
      sipType = sipType_QgsProcessingParameterFeatureSource;
    else if ( sipCpp->type() == "sink" )
      sipType = sipType_QgsProcessingParameterFeatureSink;
    else if ( sipCpp->type() == "vectorOut" )
      sipType = sipType_QgsProcessingParameterVectorOutput;
    else if ( sipCpp->type() == "rasterOut" )
      sipType = sipType_QgsProcessingParameterRasterOutput;
    else if ( sipCpp->type() == "fileOut" )
      sipType = sipType_QgsProcessingParameterFileOutput;
    else if ( sipCpp->type() == "folderOut" )
      sipType = sipType_QgsProcessingParameterFolderOutput;
    SIP_END
#endif

  public:

    //! Parameter flags
    enum Flag
    {
      FlagAdvanced = 1 << 1, //!< Parameter is an advanced parameter which should be hidden from users by default
      FlagHidden = 1 << 2, //!< Parameter is hidden and should not be shown to users
      FlagOptional = 1 << 3, //!< Parameter is optional
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! Layer types enum
    enum LayerType
    {
      TypeAny = -2, //!< Any layer
      TypeVectorAny = -1, //!< Any vector layer with geometry
      TypeVectorPoint = 0, //!< Vector point layers
      TypeVectorLine = 1, //!< Vector line layers
      TypeVectorPolygon = 2, //!< Vector polygon layers
      TypeRaster = 3, //!< Raster layers
      TypeFile = 4, //!< Files
      TypeTable = 5, //!< Tables (i.e. vector layers with or without geometry)
    };

    /**
     * Constructor for QgsProcessingParameterDefinition.
     */
    QgsProcessingParameterDefinition( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      bool optional = false );

    virtual ~QgsProcessingParameterDefinition() = default;

    /**
     * Unique parameter type name.
     */
    virtual QString type() const = 0;

    /**
     * Returns true if this parameter represents a file or layer destination, e.g. parameters
     * which are used for the destination for layers output by an algorithm will return
     * true.
     */
    virtual bool isDestination() const { return false; }

    /**
     * Returns the name of the parameter. This is the internal identifier by which
     * algorithms access this parameter.
     * @see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name of the parameter. This is the internal identifier by which
     * algorithms access this parameter.
     * @see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns the description for the parameter. This is the user-visible string
     * used to identify this parameter.
     * @see setDescription()
     */
    QString description() const { return mDescription; }

    /**
     * Sets the \a description for the parameter. This is the user-visible string
     * used to identify this parameter.
     * @see description()
     */
    void setDescription( const QString &description ) { mDescription = description; }

    /**
     * Returns the default value for the parameter.
     * @see setDefaultValue()
     */
    QVariant defaultValue() const { return mDefault; }

    /**
     * Sets the default \a value for the parameter. Caller takes responsibility
     * to ensure that \a value is a valid input for the parameter subclass.
     * @see defaultValue()
     */
    void setDefaultValue( const QVariant &value ) { mDefault = value; }

    /**
     * Returns any flags associated with the parameter.
     * @see setFlags()
     */
    Flags flags() const { return mFlags; }

    /**
     * Sets the \a flags associated with the parameter.
     * @see flags()
     */
    void setFlags( const Flags &flags ) { mFlags = flags; }

    /**
     * Checks whether the specified \a input value is acceptable for the
     * parameter. Returns true if the value can be accepted.
     * The optional \a context parameter can be specified to allow a more stringent
     * check to be performed, capable of checking for the presence of required
     * layers and other factors within the context.
     */
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const;

    /**
     * Returns a string version of the parameter input \a value, which is suitable for use as an input
     * parameter value when running an algorithm directly from a Python command.
     * The returned value must be correctly escaped - e.g. string values must be wrapped in ' 's.
     */
    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    /**
     * Saves this parameter to a QVariantMap. Subclasses should ensure that they call the base class
     * method and then extend the result with additional properties.
     * \see fromVariantMap()
     */
    virtual QVariantMap toVariantMap() const;

    /**
     * Restores this parameter to a QVariantMap. Subclasses should ensure that they call the base class
     * method.
     * \see toVariantMap()
     */
    virtual bool fromVariantMap( const QVariantMap &map );

    /**
     * Returns the parameter's freeform metadata. This is mostly used by parameter widget wrappers
     * in order to customise their appearance and behavior.
     * \see setMetadata()
     * \note not available in Python bindings.
     */
    SIP_SKIP QVariantMap metadata() const { return mMetadata; }

    /**
     * Returns the parameter's freeform metadata. This is mostly used by parameter widget wrappers
     * in order to customise their appearance and behavior.
     * \see setMetadata()
     */
    QVariantMap &metadata() { return mMetadata; }

    /**
     * Sets the parameter's freeform \a metadata. This is mostly used by parameter widget wrappers
     * in order to customise their appearance and behavior.
     * \see metadata()
     */
    void setMetadata( const QVariantMap &metadata ) { mMetadata = metadata; }

  protected:

    //! Parameter name
    QString mName;

    //! Parameter description
    QString mDescription;

    //! Default value for parameter
    QVariant mDefault;

    //! Parameter flags
    Flags mFlags;

    //! Freeform metadata for parameter. Mostly used by widget wrappers to customise their appearance and behavior.
    QVariantMap mMetadata;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingParameterDefinition::Flags )

//! List of processing parameters
typedef QList< const QgsProcessingParameterDefinition * > QgsProcessingParameterDefinitions;

/**
 * \class QgsProcessingParameters
 * \ingroup core
 *
 * A collection of utilities for working with parameters when running a processing algorithm.
 *
 * Parameters are stored in a QVariantMap and referenced by a unique string key.
 * The QVariants in parameters are not usually accessed
 * directly, and instead the high level API provided through QgsProcessingParameters
 * parameterAsString(), parameterAsDouble() are used instead.
 *
 * Parameters are evaluated using a provided QgsProcessingContext, allowing
 * the evaluation to understand available map layers and expression contexts
 * (for expression based parameters).
 *
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsProcessingParameters
{

  public:

    /**
     * Returns true if the parameter with matching \a name is a dynamic parameter, and must
     * be evaluated once for every input feature processed.
     */
    static bool isDynamic( const QVariantMap &parameters, const QString &name );

    /**
     * Evaluates the parameter with matching \a definition to a static string value.
     */
    static QString parameterAsString( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to an expression.
     */
    static QString parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static double value.
     */
    static double parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static integer value.
     */
    static int parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a enum value.
     */
    static int parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to list of enum values.
     */
    static QList<int> parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static boolean value.
     */
    static bool parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a feature sink.
     *
     * The \a fields, \a geometryType and \a crs parameters dictate the properties
     * of the resulting feature sink.
     *
     * Sinks will either be taken from \a context's active project, or created from external
     * providers and stored temporarily in the \a context. The \a destinationIdentifier
     * argument will be set to a string which can be used to retrieve the layer corresponding
     * to the sink, e.g. via calling QgsProcessingUtils::mapLayerFromString().
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     */
    static QgsFeatureSink *parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
                                            const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                            QgsProcessingContext &context, QString &destinationIdentifier SIP_OUT ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a definition to a feature source.
     *
     * Sources will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     */
    static QgsFeatureSource *parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a definition to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsMapLayer *parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a raster layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsRasterLayer *parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a raster output layer destination.
     */
    static QString parameterAsRasterOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a file based output destination.
     */
    static QString parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a vector layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsVectorLayer *parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a coordinate reference system.
     */
    static QgsCoordinateReferenceSystem parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a rectangular extent.
     */
    static QgsRectangle parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a point.
     */
    static QgsPointXY parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a file/folder name.
     */
    static QString parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a matrix/table of values.
     * Tables are collapsed to a 1 dimensional list.
     */
    static QVariantList parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a list of map layers.
     */
    static QList< QgsMapLayer *> parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a range of values.
     */
    static QList<double> parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a list of fields.
     */
    static QStringList parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Creates a new QgsProcessingParameterDefinition using the configuration from a
     * supplied variant \a map.
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsProcessingParameterDefinition *parameterFromVariantMap( const QVariantMap &map ) SIP_FACTORY;

};



/**
 * \class QgsProcessingParameterBoolean
 * \ingroup core
 * A boolean parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterBoolean : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterBoolean.
     */
    QgsProcessingParameterBoolean( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                   bool optional = false );

    QString type() const override { return QStringLiteral( "boolean" ); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
};

/**
 * \class QgsProcessingParameterCrs
 * \ingroup core
 * A coordinate reference system parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterCrs : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterCrs.
     */
    QgsProcessingParameterCrs( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                               bool optional = false );

    QString type() const override { return QStringLiteral( "crs" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
};

/**
 * \class QgsProcessingParameterMapLayer
 * \ingroup core
 * A map layer parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterMapLayer : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterMapLayer.
     */
    QgsProcessingParameterMapLayer( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                    bool optional = false );

    QString type() const override { return QStringLiteral( "layer" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
};

/**
 * \class QgsProcessingParameterExtent
 * \ingroup core
 * A rectangular map extent parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterExtent : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterExtent.
     */
    QgsProcessingParameterExtent( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                  bool optional = false );

    QString type() const override { return QStringLiteral( "extent" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

};


/**
 * \class QgsProcessingParameterPoint
 * \ingroup core
 * A point parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterPoint : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterPoint.
     */
    QgsProcessingParameterPoint( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                 bool optional = false );

    QString type() const override { return QStringLiteral( "point" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;

};

/**
 * \class QgsProcessingParameterFile
 * \ingroup core
 * An input file or folder parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFile : public QgsProcessingParameterDefinition
{
  public:

    //! Parameter behavior
    enum Behavior
    {
      File = 0, //!< Parameter is a single file
      Folder, //!< Parameter is a folder
    };

    /**
     * Constructor for QgsProcessingParameterFile.
     */
    QgsProcessingParameterFile( const QString &name, const QString &description = QString(), Behavior behavior = File, const QString &extension = QString(), const QVariant &defaultValue = QVariant(),
                                bool optional = false );

    QString type() const override { return QStringLiteral( "file" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;

    /**
     * Returns the parameter behavior (e.g. File or Folder).
     * \see setBehavior()
     */
    Behavior behavior() const { return mBehavior; }

    /**
     * Sets the parameter \a behavior (e.g. File or Folder).
     * \see behavior()
     */
    void setBehavior( Behavior behavior ) { mBehavior = behavior; }

    /**
     * Returns any specified file extension for the parameter.
     * \see setExtension()
     */
    QString extension() const { return mExtension; }

    /**
     * Sets a file \a extension for the parameter.
     * \see extension()
     */
    void setExtension( const QString &extension ) { mExtension = extension; }

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    Behavior mBehavior = File;
    QString mExtension;
};

/**
 * \class QgsProcessingParameterMatrix
 * \ingroup core
 * An table (matrix) parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterMatrix : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterMatrix.
     */
    QgsProcessingParameterMatrix( const QString &name, const QString &description = QString(), int numberRows = 3,
                                  bool hasFixedNumberRows = false, const QStringList &headers = QStringList(),
                                  const QVariant &defaultValue = QVariant(),
                                  bool optional = false );

    QString type() const override { return QStringLiteral( "matrix" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns a list of column headers (if set).
     * \see setHeaders()
     */
    QStringList headers() const;

    /**
     * Sets the list of column \a headers.
     * \see headers()
     */
    void setHeaders( const QStringList &headers );

    /**
     * Returns the fixed number of rows in the table. This parameter only has an
     * effect if hasFixedNumberRows() is true.
     * \see setNumberRows()
     * \see setFixedNumberRows()
     */
    int numberRows() const;

    /**
     * Sets the fixed number of \a rows in the table. This parameter only has an
     * effect if hasFixedNumberRows() is true.
     * \see numberRows()
     * \see setFixedNumberRows()
     */
    void setNumberRows( int rows );

    /**
     * Returns whether the table has a fixed number of rows.
     * \see numberRows()
     * \see setHasFixedNumberRows()
     */
    bool hasFixedNumberRows() const;

    /**
     * Sets whether the table has a fixed number of rows.
     * \see setNumberRows()
     * \see hasFixedNumberRows()
     */
    void setHasFixedNumberRows( bool hasFixedNumberRows );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QStringList mHeaders;
    int mNumberRows = 3;
    bool mFixedNumberRows = false;

};

/**
 * \class QgsProcessingParameterMultipleLayers
 * \ingroup core
 * A parameter for processing algorithms which accepts multiple map layers.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterMultipleLayers : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterMultipleLayers.
     */
    QgsProcessingParameterMultipleLayers( const QString &name, const QString &description = QString(), QgsProcessingParameterDefinition::LayerType layerType = QgsProcessingParameterDefinition::TypeVectorAny,
                                          const QVariant &defaultValue = QVariant(),
                                          bool optional = false );

    QString type() const override { return QStringLiteral( "multilayer" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the layer type for layers acceptable by the parameter.
     * \see setLayerType()
     */
    QgsProcessingParameterDefinition::LayerType layerType() const;

    /**
     * Sets the layer \a type for layers acceptable by the parameter.
     * \see layerType()
     */
    void setLayerType( QgsProcessingParameterDefinition::LayerType type );

    /**
     * Returns the minimum number of layers required for the parameter. If the return value is < 1
     * then the parameter accepts any number of layers.
     * \see setMinimumNumberInputs()
     */
    int minimumNumberInputs() const;

    /**
     * Sets the \a minimum number of layers required for the parameter. The minimum must be >= 1
     * if the parameter is not optional.
     * \see minimumNumberInputs()
     */
    void setMinimumNumberInputs( int minimum );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    LayerType mLayerType = TypeVectorAny;
    int mMinimumNumberInputs = 0;

};

/**
 * \class QgsProcessingParameterNumber
 * \ingroup core
 * A numeric parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterNumber : public QgsProcessingParameterDefinition
{
  public:

    //! Numeric data type
    enum Type
    {
      Integer, //!< Integer values
      Double, //!< Double/float values
    };

    /**
     * Constructor for QgsProcessingParameterNumber.
     */
    explicit QgsProcessingParameterNumber( const QString &name, const QString &description = QString(),
                                           Type type = Integer,
                                           const QVariant &defaultValue = QVariant(),
                                           bool optional = false,
                                           double minValue = -DBL_MAX,
                                           double maxValue = DBL_MAX
                                         );

    QString type() const override { return QStringLiteral( "number" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the minimum value acceptable by the parameter.
     * \see setMinimum()
     */
    double minimum() const;

    /**
     * Sets the \a minimum value acceptable by the parameter.
     * \see minimum()
     */
    void setMinimum( double minimum );

    /**
     * Returns the maximum value acceptable by the parameter.
     * \see setMaximum()
     */
    double maximum() const;

    /**
     * Sets the \a maximum value acceptable by the parameter.
     * \see maximum()
     */
    void setMaximum( double maximum );

    /**
     * Returns the acceptable data type for the parameter.
     * \see setDataType()
     */
    Type dataType() const;

    /**
     * Sets the acceptable data \a type for the parameter.
     * \see dataType()
     */
    void setDataType( const Type &type );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    double mMin = -DBL_MAX;
    double mMax = DBL_MAX;
    Type mDataType = Integer;
};

/**
 * \class QgsProcessingParameterRange
 * \ingroup core
 * A numeric range parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRange : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterRange.
     */
    QgsProcessingParameterRange( const QString &name, const QString &description = QString(),
                                 QgsProcessingParameterNumber::Type type = QgsProcessingParameterNumber::Integer,
                                 const QVariant &defaultValue = QVariant(),
                                 bool optional = false );

    QString type() const override { return QStringLiteral( "range" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the acceptable data type for the range.
     * \see setDataType()
     */
    QgsProcessingParameterNumber::Type dataType() const;

    /**
     * Sets the acceptable data \a type for the range.
     * \see dataType()
     */
    void setDataType( const QgsProcessingParameterNumber::Type &dataType );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QgsProcessingParameterNumber::Type mDataType = QgsProcessingParameterNumber::Integer;
};

/**
 * \class QgsProcessingParameterRasterLayer
 * \ingroup core
 * A raster layer parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRasterLayer : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterRasterLayer.
     */
    QgsProcessingParameterRasterLayer( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false );

    QString type() const override { return QStringLiteral( "raster" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

};

/**
 * \class QgsProcessingParameterEnum
 * \ingroup core
 * An enum based parameter for processing algorithms, allowing for selection from predefined values.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterEnum : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterEnum.
     */
    QgsProcessingParameterEnum( const QString &name, const QString &description = QString(), const QStringList &options = QStringList(),
                                bool allowMultiple = false,
                                const QVariant &defaultValue = QVariant(),
                                bool optional = false );

    QString type() const override { return QStringLiteral( "enum" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the list of acceptable options for the parameter.
     * \see setOptions()
     */
    QStringList options() const;

    /**
     * Sets the list of acceptable \a options for the parameter.
     * \see options()
     */
    void setOptions( const QStringList &options );

    /**
     * Returns true if the parameter allows multiple selected values.
     * \see setAllowMultiple()
     */
    bool allowMultiple() const;

    /**
     * Sets whether the parameter allows multiple selected values.
     * \see allowMultiple()
     */
    void setAllowMultiple( bool allowMultiple );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QStringList mOptions;
    bool mAllowMultiple = false;

};

/**
 * \class QgsProcessingParameterString
 * \ingroup core
 * A string parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterString : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterString.
     */
    QgsProcessingParameterString( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                  bool multiLine = false,
                                  bool optional = false );

    QString type() const override { return QStringLiteral( "string" ); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns true if the parameter allows multiline strings.
     * \see setMultiLine()
     */
    bool multiLine() const;

    /**
     * Sets whether the parameter allows multiline strings.
     * \see multiLine()
     */
    void setMultiLine( bool multiLine );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    bool mMultiLine = false;

};

/**
 * \class QgsProcessingParameterExpression
 * \ingroup core
 * An expression parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterExpression : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterExpression.
     */
    QgsProcessingParameterExpression( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      const QString &parentLayerParameterName = QString(),
                                      bool optional = false );

    QString type() const override { return QStringLiteral( "expression" ); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the name of the parent layer parameter, or an empty string if this is not set.
     * \see setParentLayerParameter()
     */
    QString parentLayerParameter() const;

    /**
     * Sets the name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentLayerParameter()
     */
    void setParentLayerParameter( const QString &parentLayerParameter );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QString mParentLayerParameter;

};

/**
 * \class QgsProcessingParameterVectorLayer
 * \ingroup core
 * A vector layer (with or without geometry) parameter for processing algorithms. Consider using
 * the more versatile QgsProcessingParameterFeatureSource wherever possible.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterVectorLayer : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterVectorLayer.
     */
    QgsProcessingParameterVectorLayer( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false );

    QString type() const override { return QStringLiteral( "vector" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

};

/**
 * \class QgsProcessingParameterField
 * \ingroup core
 * A vector layer or feature source field parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterField : public QgsProcessingParameterDefinition
{
  public:

    //! Field data types
    enum DataType
    {
      Any = -1, //!< Accepts any field
      Numeric = 0, //!< Accepts numeric fields
      String = 1, //!< Accepts string fields
      DateTime = 2 //!< Accepts datetime fields
    };

    /**
     * Constructor for QgsProcessingParameterField.
     */
    QgsProcessingParameterField( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                 const QString &parentLayerParameterName = QString(),
                                 DataType type = Any,
                                 bool allowMultiple = false,
                                 bool optional = false );

    QString type() const override { return QStringLiteral( "field" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the name of the parent layer parameter, or an empty string if this is not set.
     * \see setParentLayerParameter()
     */
    QString parentLayerParameter() const;

    /**
     * Sets the name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentLayerParameter()
     */
    void setParentLayerParameter( const QString &parentLayerParameter );

    /**
     * Returns the acceptable data type for the field.
     * \see setDataType()
     */
    DataType dataType() const;

    /**
     * Sets the acceptable data \a type for the field.
     * \see dataType()
     */
    void setDataType( const DataType &type );

    /**
     * Returns whether multiple field selections are permitted.
     * \see setAllowMultiple()
     */
    bool allowMultiple() const;

    /**
     * Sets whether multiple field selections are permitted.
     * \see allowMultiple()
     */
    void setAllowMultiple( bool allowMultiple );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QString mParentLayerParameter;
    DataType mDataType = Any;
    bool mAllowMultiple = false;

};

/**
 * \class QgsProcessingParameterFeatureSource
 * \ingroup core
 * An input feature source (such as vector layers) parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFeatureSource : public QgsProcessingParameterDefinition
{
  public:

    /**
    * Constructor for QgsProcessingParameterFeatureSource.
    */
    QgsProcessingParameterFeatureSource( const QString &name, const QString &description = QString(),
                                         const QList< int > &types = QList< int >(),
                                         const QVariant &defaultValue = QVariant(), bool optional = false );

    QString type() const override { return QStringLiteral( "source" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Returns the geometry types for sources acceptable by the parameter.
     * \see setDataTypes()
     */
    QList< int > dataTypes() const;

    /**
     * Sets the geometry \a types for sources acceptable by the parameter.
     * \see dataTypes()
     */
    void setDataTypes( const QList< int > &types );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QList< int > mDataTypes = QList< int >() << QgsProcessingParameterDefinition::TypeVectorAny;

};

/**
 * \class QgsProcessingDestinationParameter
 * \ingroup core
 * Base class for all parameter definitions which represent file or layer destinations, e.g. parameters
 * which are used for the destination for layers output by an algorithm.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingDestinationParameter : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingDestinationParameter.
     */
    QgsProcessingDestinationParameter( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false );

    bool isDestination() const override { return true; }
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Returns a new QgsProcessingOutputDefinition corresponding to the definition of the destination
     * parameter.
     */
    virtual QgsProcessingOutputDefinition *toOutputDefinition() const = 0 SIP_FACTORY;

    /**
     * Returns true if the destination parameter supports non filed-based outputs,
     * such as memory layers or direct database outputs.
     * \see setSupportsNonFileBasedOutputs()
     */
    bool supportsNonFileBasedOutputs() const { return mSupportsNonFileBasedOutputs; }

    /**
     * Sets whether the destination parameter supports non filed-based outputs,
     * such as memory layers or direct database outputs.
     * \see supportsNonFileBasedOutputs()
     */
    void setSupportsNonFileBasedOutputs( bool supportsNonFileBasedOutputs ) { mSupportsNonFileBasedOutputs = supportsNonFileBasedOutputs; }

    /**
     * Returns the default file extension for destination file paths
     * associated with this parameter.
     */
    virtual QString defaultFileExtension() const = 0;

    /**
     * Generates a temporary destination value for this parameter. The returned
     * value will be a file path or QGIS data provider URI suitable for
     * temporary storage of created layers and files.
     */
    virtual QString generateTemporaryDestination() const;

  private:

    bool mSupportsNonFileBasedOutputs = true;

};


/**
 * \class QgsProcessingParameterFeatureSink
 * \ingroup core
 * A feature sink output for processing algorithms.
 *
 * A parameter which represents the destination feature sink for features created by an algorithm.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFeatureSink : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterFeatureSink.
     */
    QgsProcessingParameterFeatureSink( const QString &name, const QString &description = QString(), QgsProcessingParameterDefinition::LayerType type = QgsProcessingParameterDefinition::TypeVectorAny, const QVariant &defaultValue = QVariant(),
                                       bool optional = false );

    QString type() const override { return QStringLiteral( "sink" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns the layer type for sinks associated with the parameter.
     * \see setDataType()
     */
    QgsProcessingParameterDefinition::LayerType dataType() const;

    /**
     * Returns true if sink is likely to include geometries. In cases were presence of geometry
     * cannot be reliably determined in advance, this method will default to returning true.
     */
    bool hasGeometry() const;

    /**
     * Sets the layer \a type for the sinks associated with the parameter.
     * \see dataType()
     */
    void setDataType( QgsProcessingParameterDefinition::LayerType type );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QString generateTemporaryDestination() const override;

  private:

    QgsProcessingParameterDefinition::LayerType mDataType = QgsProcessingParameterDefinition::TypeVectorAny;
};


/**
 * \class QgsProcessingParameterVectorOutput
 * \ingroup core
 * A vector layer output parameter. Consider using the more flexible QgsProcessingParameterFeatureSink wherever
 * possible.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterVectorOutput : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterVectorOutput.
     */
    QgsProcessingParameterVectorOutput( const QString &name, const QString &description = QString(), QgsProcessingParameterDefinition::LayerType type = QgsProcessingParameterDefinition::TypeVectorAny, const QVariant &defaultValue = QVariant(),
                                        bool optional = false );

    QString type() const override { return QStringLiteral( "vectorOut" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns the layer type for layers associated with the parameter.
     * \see setDataType()
     */
    QgsProcessingParameterDefinition::LayerType dataType() const;

    /**
     * Returns true if the layer is likely to include geometries. In cases were presence of geometry
     * cannot be reliably determined in advance, this method will default to returning true.
     */
    bool hasGeometry() const;

    /**
     * Sets the layer \a type for the layers associated with the parameter.
     * \see dataType()
     */
    void setDataType( QgsProcessingParameterDefinition::LayerType type );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QgsProcessingParameterDefinition::LayerType mDataType = QgsProcessingParameterDefinition::TypeVectorAny;
};

/**
 * \class QgsProcessingParameterRasterOutput
 * \ingroup core
 * A raster layer output parameter.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRasterOutput : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterRasterOutput.
     */
    QgsProcessingParameterRasterOutput( const QString &name, const QString &description = QString(),
                                        const QVariant &defaultValue = QVariant(),
                                        bool optional = false );

    QString type() const override { return QStringLiteral( "rasterOut" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;
};

/**
 * \class QgsProcessingParameterFileOutput
 * \ingroup core
 * A generic file based output parameter.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFileOutput : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterFileOutput.
     */
    QgsProcessingParameterFileOutput( const QString &name, const QString &description = QString(),
                                      const QString &fileFilter = QString(),
                                      const QVariant &defaultValue = QVariant(),
                                      bool optional = false );

    QString type() const override { return QStringLiteral( "fileOut" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns the file filter string for files compatible with this output.
     * \see setFileFilter()
     */
    QString fileFilter() const;

    /**
     * Sets the file \a filter string for files compatible with this output.
     * \see fileFilter()
     */
    void setFileFilter( const QString &filter );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QString mFileFilter;
};

/**
 * \class QgsProcessingParameterFolderOutput
 * \ingroup core
 * A folder output parameter.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFolderOutput : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterFolderOutput.
     */
    QgsProcessingParameterFolderOutput( const QString &name, const QString &description = QString(),
                                        const QVariant &defaultValue = QVariant(),
                                        bool optional = false );

    QString type() const override { return QStringLiteral( "folderOut" ); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

};

#endif // QGSPROCESSINGPARAMETERS_H


