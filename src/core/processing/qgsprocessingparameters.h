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
#include "qgsprocessing.h"
#include "qgsproperty.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeaturesource.h"
#include "qgsprocessingutils.h"
#include <QMap>
#include <limits>

class QgsProcessingContext;
class QgsRasterLayer;
class QgsMeshLayer;
class QgsVectorLayer;
class QgsFeatureSink;
class QgsProcessingFeatureSource;
class QgsProcessingOutputDefinition;
class QgsProcessingFeedback;
class QgsProcessingProvider;

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

    bool operator==( const QgsProcessingFeatureSourceDefinition &other )
    {
      return source == other.source && selectedFeaturesOnly == other.selectedFeaturesOnly;
    }

    bool operator!=( const QgsProcessingFeatureSourceDefinition &other )
    {
      return !( *this == other );
    }

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
    QgsProject *destinationProject = nullptr;

    /**
     * Name to use for sink if it's to be loaded into a destination project.
     */
    QString destinationName;

    /**
     * Map of optional sink/layer creation options, which
     * are passed to the underlying provider when creating new layers. Known options also
     * include 'fileEncoding', which is used to specify a file encoding to use for created
     * files.
     */
    QVariantMap createOptions;

    /**
     * Saves this output layer definition to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     * \see loadVariant()
     * \since QGIS 3.2
     */
    QVariant toVariant() const;

    /**
     * Loads this output layer definition from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to load it from an XML document.
     * \see toVariant()
     * \since QGIS 3.2
     */
    bool loadVariant( const QVariantMap &map );

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
    if ( sipCpp->type() == QgsProcessingParameterBoolean::typeName() )
      sipType = sipType_QgsProcessingParameterBoolean;
    else if ( sipCpp->type() == QgsProcessingParameterCrs::typeName() )
      sipType = sipType_QgsProcessingParameterCrs;
    else if ( sipCpp->type() == QgsProcessingParameterMapLayer::typeName() )
      sipType = sipType_QgsProcessingParameterMapLayer;
    else if ( sipCpp->type() == QgsProcessingParameterExtent::typeName() )
      sipType = sipType_QgsProcessingParameterExtent;
    else if ( sipCpp->type() == QgsProcessingParameterPoint::typeName() )
      sipType = sipType_QgsProcessingParameterPoint;
    else if ( sipCpp->type() == QgsProcessingParameterFile::typeName() )
      sipType = sipType_QgsProcessingParameterFile;
    else if ( sipCpp->type() == QgsProcessingParameterMatrix::typeName() )
      sipType = sipType_QgsProcessingParameterMatrix;
    else if ( sipCpp->type() == QgsProcessingParameterMultipleLayers::typeName() )
      sipType = sipType_QgsProcessingParameterMultipleLayers;
    else if ( sipCpp->type() == QgsProcessingParameterNumber::typeName() )
      sipType = sipType_QgsProcessingParameterNumber;
    else if ( sipCpp->type() == QgsProcessingParameterDistance::typeName() )
      sipType = sipType_QgsProcessingParameterDistance;
    else if ( sipCpp->type() == QgsProcessingParameterRange::typeName() )
      sipType = sipType_QgsProcessingParameterRange;
    else if ( sipCpp->type() == QgsProcessingParameterRasterLayer::typeName() )
      sipType = sipType_QgsProcessingParameterRasterLayer;
    else if ( sipCpp->type() == QgsProcessingParameterMeshLayer::typeName() )
      sipType = sipType_QgsProcessingParameterMeshLayer;
    else if ( sipCpp->type() == QgsProcessingParameterEnum::typeName() )
      sipType = sipType_QgsProcessingParameterEnum;
    else if ( sipCpp->type() == QgsProcessingParameterString::typeName() )
      sipType = sipType_QgsProcessingParameterString;
    else if ( sipCpp->type() == QgsProcessingParameterExpression::typeName() )
      sipType = sipType_QgsProcessingParameterExpression;
    else if ( sipCpp->type() == QgsProcessingParameterAuthConfig::typeName() )
      sipType = sipType_QgsProcessingParameterAuthConfig;
    else if ( sipCpp->type() == QgsProcessingParameterVectorLayer::typeName() )
      sipType = sipType_QgsProcessingParameterVectorLayer;
    else if ( sipCpp->type() == QgsProcessingParameterField::typeName() )
      sipType = sipType_QgsProcessingParameterField;
    else if ( sipCpp->type() == QgsProcessingParameterFeatureSource::typeName() )
      sipType = sipType_QgsProcessingParameterFeatureSource;
    else if ( sipCpp->type() == QgsProcessingParameterFeatureSink::typeName() )
      sipType = sipType_QgsProcessingParameterFeatureSink;
    else if ( sipCpp->type() == QgsProcessingParameterVectorDestination::typeName() )
      sipType = sipType_QgsProcessingParameterVectorDestination;
    else if ( sipCpp->type() == QgsProcessingParameterRasterDestination::typeName() )
      sipType = sipType_QgsProcessingParameterRasterDestination;
    else if ( sipCpp->type() == QgsProcessingParameterFileDestination::typeName() )
      sipType = sipType_QgsProcessingParameterFileDestination;
    else if ( sipCpp->type() == QgsProcessingParameterFolderDestination::typeName() )
      sipType = sipType_QgsProcessingParameterFolderDestination;
    else if ( sipCpp->type() == QgsProcessingParameterBand::typeName() )
      sipType = sipType_QgsProcessingParameterBand;
    else
      sipType = nullptr;
    SIP_END
#endif

  public:

    //! Parameter flags
    enum Flag
    {
      FlagAdvanced = 1 << 1, //!< Parameter is an advanced parameter which should be hidden from users by default
      FlagHidden = 1 << 2, //!< Parameter is hidden and should not be shown to users
      FlagOptional = 1 << 3, //!< Parameter is optional
      FlagIsModelOutput = 1 << 4, //!< Destination parameter is final output. The parameter name will be used.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingParameterDefinition.
     */
    QgsProcessingParameterDefinition( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      bool optional = false );

    virtual ~QgsProcessingParameterDefinition() = default;

    /**
     * Creates a clone of the parameter definition.
     */
    virtual QgsProcessingParameterDefinition *clone() const = 0 SIP_FACTORY;

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
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name of the parameter. This is the internal identifier by which
     * algorithms access this parameter.
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns the description for the parameter. This is the user-visible string
     * used to identify this parameter.
     * \see setDescription()
     */
    QString description() const { return mDescription; }

    /**
     * Sets the \a description for the parameter. This is the user-visible string
     * used to identify this parameter.
     * \see description()
     */
    void setDescription( const QString &description ) { mDescription = description; }

    /**
     * Returns the default value for the parameter.
     * \see setDefaultValue()
     */
    QVariant defaultValue() const { return mDefault; }

    /**
     * Sets the default \a value for the parameter. Caller takes responsibility
     * to ensure that \a value is a valid input for the parameter subclass.
     * \see defaultValue()
     */
    void setDefaultValue( const QVariant &value ) { mDefault = value; }

    /**
     * Returns any flags associated with the parameter.
     * \see setFlags()
     */
    Flags flags() const { return mFlags; }

    /**
     * Sets the \a flags associated with the parameter.
     * \see flags()
     */
    void setFlags( Flags flags ) { mFlags = flags; }

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
     * Returns the parameter definition encoded in a string which can be used within a
     * Python processing script.
     */
    virtual QString asScriptCode() const;

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
     * in order to customize their appearance and behavior.
     * \see setMetadata()
     * \note not available in Python bindings.
     */
    SIP_SKIP QVariantMap metadata() const { return mMetadata; }

    /**
     * Returns the parameter's freeform metadata. This is mostly used by parameter widget wrappers
     * in order to customize their appearance and behavior.
     * \see setMetadata()
     */
    QVariantMap &metadata() { return mMetadata; }

    /**
     * Sets the parameter's freeform \a metadata. This is mostly used by parameter widget wrappers
     * in order to customize their appearance and behavior.
     * \see metadata()
     */
    void setMetadata( const QVariantMap &metadata ) { mMetadata = metadata; }

    /**
     * Returns a list of other parameter names on which this parameter is dependent (e.g.
     * field parameters which depend on a parent layer parameter).
     */
    virtual QStringList dependsOnOtherParameters() const { return QStringList(); }

    /**
     * Returns a pointer to the algorithm which owns this parameter. May be nullptr
     * for non-owned parameters.
     * \see provider()
     */
    QgsProcessingAlgorithm *algorithm() const;

    /**
     * Returns a pointer to the provider for the algorithm which owns this parameter. May be nullptr
     * for non-owned parameters or algorithms.
     * \see algorithm()
     */
    QgsProcessingProvider *provider() const;

    /**
     * Returns a formatted tooltip for use with the parameter, which gives helpful information
     * like parameter description, ID, and extra content like default values (depending on parameter type).
     */
    virtual QString toolTip() const;

    /**
     * Returns true if the parameter supports is dynamic, and can support data-defined values
     * (i.e. QgsProperty based values).
     * \see setIsDynamic()
     * \see dynamicPropertyDefinition()
     * \see dynamicLayerParameterName()
     */
    bool isDynamic() const { return mIsDynamic; }

    /**
     * Sets whether the parameter is \a dynamic, and can support data-defined values
     * (i.e. QgsProperty based values).
     * \see isDynamic()
     * \see setDynamicPropertyDefinition()
     * \see setDynamicLayerParameterName()
     */
    void setIsDynamic( bool dynamic ) { mIsDynamic = dynamic; }

    /**
     * Returns the property definition for dynamic properties.
     * \see isDynamic()
     * \see setDynamicPropertyDefinition()
     * \see dynamicLayerParameterName()
     */
    QgsPropertyDefinition dynamicPropertyDefinition() const { return mPropertyDefinition; }

    /**
     * Sets the property \a definition for dynamic properties.
     * \see isDynamic()
     * \see dynamicPropertyDefinition()
     * \see setDynamicLayerParameterName()
     */
    void setDynamicPropertyDefinition( const QgsPropertyDefinition &definition ) { mPropertyDefinition = definition; }

    /**
     * Returns the name of the parameter for a layer linked to a dynamic parameter, or an empty string if this is not set.
     *
     * Dynamic parameters (see isDynamic()) can have an optional vector layer parameter linked to them,
     * which indicates which layer the fields and values will be available from when evaluating
     * the dynamic parameter.
     *
     * \see setDynamicLayerParameterName()
     * \see isDynamic()
     * \see dynamicPropertyDefinition()
     */
    QString dynamicLayerParameterName() const { return mDynamicLayerParameterName; }

    /**
     * Sets the \a name for the parameter for a layer linked to a dynamic parameter, or an empty string if this is not set.
     *
     * Dynamic parameters (see isDynamic()) can have an optional vector layer parameter linked to them,
     * which indicates which layer the fields and values will be available from when evaluating
     * the dynamic parameter.
     *
     * \see dynamicLayerParameterName()
     * \see isDynamic()
     * \see setDynamicPropertyDefinition()
     */
    void setDynamicLayerParameterName( const QString &name ) { mDynamicLayerParameterName = name; }

  protected:

    //! Parameter name
    QString mName;

    //! Parameter description
    QString mDescription;

    //! Default value for parameter
    QVariant mDefault;

    //! Parameter flags
    Flags mFlags;

    //! Freeform metadata for parameter. Mostly used by widget wrappers to customize their appearance and behavior.
    QVariantMap mMetadata;

    //! Pointer to algorithm which owns this parameter
    QgsProcessingAlgorithm *mAlgorithm = nullptr;

    //! True for dynamic parameters, which can have data-defined (QgsProperty) based values
    bool mIsDynamic = false;

    //! Data defined property definition
    QgsPropertyDefinition mPropertyDefinition;

    //! Linked vector layer parameter name for dynamic properties
    QString mDynamicLayerParameterName;

    // To allow access to mAlgorithm. We don't want a public setter for this!
    friend class QgsProcessingAlgorithm;

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
     * Evaluates the parameter with matching \a definition and \a value to a static string value.
     * \since QGIS 3.4
     */
    static QString parameterAsString( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to an expression.
     */
    static QString parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definitionand \a value to an expression.
     * \since QGIS 3.4
     */
    static QString parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static double value.
     */
    static double parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static double value.
     * \since QGIS 3.4
     */
    static double parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static integer value.
     */
    static int parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static integer value.
     * \since QGIS 3.4
     */
    static int parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a list of integer values.
     * \since QGIS 3.4
     */
    static QList<int> parameterAsInts( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a list of integer values.
     * \since QGIS 3.4
     */
    static QList<int> parameterAsInts( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a enum value.
     */
    static int parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a enum value.
     * \since QGIS 3.4
     */
    static int parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to list of enum values.
     */
    static QList<int> parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to list of enum values.
     * \since QGIS 3.4
     */
    static QList<int> parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static boolean value.
     */
    static bool parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static boolean value.
     * \since QGIS 3.4
     */
    static bool parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

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
                                            QgsProcessingContext &context, QString &destinationIdentifier SIP_OUT, QgsFeatureSink::SinkFlags sinkFlags = nullptr ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a definition and \a value to a feature sink.
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
     *
     * \since QGIS 3.4
     */
    static QgsFeatureSink *parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariant &value,
                                            const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                            QgsProcessingContext &context, QString &destinationIdentifier SIP_OUT, QgsFeatureSink::SinkFlags sinkFlags = nullptr ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a definition to a feature source.
     *
     * Sources will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     */
    static QgsProcessingFeatureSource *parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a definition and \a value to a feature source.
     *
     * Sources will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     *
     * \since QGIS 3.4
     */
    static QgsProcessingFeatureSource *parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a definition to a source vector layer file path of compatible format.
     *
     * If the parameter is evaluated to an existing layer, and that layer is not of the format listed in the
     * \a compatibleFormats argument, then the layer will first be exported to a compatible format
     * in a temporary location. The function will then return the path to that temporary file.
     *
     * \a compatibleFormats should consist entirely of lowercase file extensions, e.g. 'shp'.
     *
     * The \a preferredFormat argument is used to specify to desired file extension to use when a temporary
     * layer export is required. This defaults to shapefiles, because shapefiles are the future (don't believe the geopackage hype!).
     */
    static QString parameterAsCompatibleSourceLayerPath( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
        QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat = QString( "shp" ), QgsProcessingFeedback *feedback = nullptr );

    /**
     * Evaluates the parameter with matching \a definition to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsMapLayer *parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.4
     */
    static QgsMapLayer *parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a raster layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsRasterLayer *parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a raster layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.4
     */
    static QgsRasterLayer *parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a output layer destination.
     */
    static QString parameterAsOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a output layer destination.
     * \since QGIS 3.4
     */
    static QString parameterAsOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a file based output destination.
     */
    static QString parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a file based output destination.
     * \since QGIS 3.4
     */
    static QString parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a vector layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsVectorLayer *parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a vector layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.4
     */
    static QgsVectorLayer *parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a mesh layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.6
     */
    static QgsMeshLayer *parameterAsMeshLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a mesh layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.6
     */
    static QgsMeshLayer *parameterAsMeshLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );


    /**
     * Evaluates the parameter with matching \a definition to a coordinate reference system.
     */
    static QgsCoordinateReferenceSystem parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a coordinate reference system.
     * \since QGIS 3.4
     */
    static QgsCoordinateReferenceSystem parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a rectangular extent.
     *
     * If \a crs is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
     * reprojected so that it is in the specified \a crs. In this case the extent of the reproject rectangle will be returned.
     *
     * \see parameterAsExtentGeometry()
     * \see parameterAsExtentCrs()
     */
    static QgsRectangle parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
                                           const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a rectangular extent.
     *
     * If \a crs is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
     * reprojected so that it is in the specified \a crs. In this case the extent of the reproject rectangle will be returned.
     *
     * \see parameterAsExtentGeometry()
     * \see parameterAsExtentCrs()
     *
     * \since QGIS 3.4
     */
    static QgsRectangle parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context,
                                           const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Evaluates the parameter with matching \a definition to a rectangular extent, and returns a geometry covering this extent.
     *
     * If \a crs is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
     * reprojected so that it is in the specified \a crs. Unlike parameterAsExtent(), the reprojected rectangle returned by this function
     * will no longer be a rectangle itself (i.e. this method returns the geometry of the actual reprojected rectangle, while parameterAsExtent() returns
     * just the extent of the reprojected rectangle).
     *
     * \see parameterAsExtent()
     * \see parameterAsExtentCrs()
     */
    static QgsGeometry parameterAsExtentGeometry( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Returns the coordinate reference system associated with an extent parameter value.
     *
     * \see parameterAsExtent()
     */
    static QgsCoordinateReferenceSystem parameterAsExtentCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a point.
     *
     * If \a crs is set then the point will be automatically reprojected so that it is in the specified \a crs.
     *
     * \see parameterAsPointCrs()
     */
    static QgsPointXY parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
                                        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a point.
     *
     * If \a crs is set then the point will be automatically reprojected so that it is in the specified \a crs.
     *
     * \see parameterAsPointCrs()
     * \since QGIS 3.4
     */
    static QgsPointXY parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context,
                                        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Returns the coordinate reference system associated with an point parameter value.
     *
     * \see parameterAsPoint()
     */
    static QgsCoordinateReferenceSystem parameterAsPointCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a file/folder name.
     */
    static QString parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a file/folder name.
     * \since QGIS 3.4
     */
    static QString parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a matrix/table of values.
     * Tables are collapsed to a 1 dimensional list.
     */
    static QVariantList parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a matrix/table of values.
     * Tables are collapsed to a 1 dimensional list.
     * \since QGIS 3.4
     */
    static QVariantList parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a list of map layers.
     */
    static QList< QgsMapLayer *> parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a list of map layers.
     * \since QGIS 3.4
     */
    static QList< QgsMapLayer *> parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a range of values.
     */
    static QList<double> parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a range of values.
     * \since QGIS 3.4
     */
    static QList<double> parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a list of fields.
     */
    static QStringList parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a list of fields.
     * \since QGIS 3.4
     */
    static QStringList parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Creates a new QgsProcessingParameterDefinition using the configuration from a
     * supplied variant \a map.
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsProcessingParameterDefinition *parameterFromVariantMap( const QVariantMap &map ) SIP_FACTORY;

    /**
     * Creates an autogenerated parameter description from a parameter \a name.
     */
    static QString descriptionFromName( const QString &name );

    /**
     * Creates a new QgsProcessingParameterDefinition using the configuration from a
     * supplied script \a code string.
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsProcessingParameterDefinition *parameterFromScriptCode( const QString &code ) SIP_FACTORY;

  private:

    static bool parseScriptCodeParameterOptions( const QString &code, bool &isOptional, QString &name, QString &type, QString &definition );
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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "boolean" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterBoolean *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "crs" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterCrs *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "layer" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMapLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "extent" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterExtent *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "point" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterPoint *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "file" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString asScriptCode() const override;

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

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFile *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition, Behavior behavior = File ) SIP_FACTORY;

  private:

    Behavior mBehavior = File;
    QString mExtension;
};

/**
 * \class QgsProcessingParameterMatrix
 * \ingroup core
 * A table (matrix) parameter for processing algorithms.
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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "matrix" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
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
     * \see setHasFixedNumberRows()
     */
    int numberRows() const;

    /**
     * Sets the fixed number of \a rows in the table. This parameter only has an
     * effect if hasFixedNumberRows() is true.
     * \see numberRows()
     * \see setHasFixedNumberRows()
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

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMatrix *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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
    QgsProcessingParameterMultipleLayers( const QString &name, const QString &description = QString(), QgsProcessing::SourceType layerType = QgsProcessing::TypeVectorAnyGeometry,
                                          const QVariant &defaultValue = QVariant(),
                                          bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "multilayer" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;

    /**
     * Returns the layer type for layers acceptable by the parameter.
     * \see setLayerType()
     */
    QgsProcessing::SourceType layerType() const;

    /**
     * Sets the layer \a type for layers acceptable by the parameter.
     * \see layerType()
     */
    void setLayerType( QgsProcessing::SourceType type );

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

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMultipleLayers *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QgsProcessing::SourceType mLayerType = QgsProcessing::TypeVectorAnyGeometry;
    int mMinimumNumberInputs = 0;

};

/**
 * \class QgsProcessingParameterNumber
 * \ingroup core
 * A numeric parameter for processing algorithms.
 *
 * For numeric parameters with a dataType() of Double, the number of decimals places
 * shown in the parameter's widget can be specified by setting the parameter's metadata. For example:
 *
 * * \code{.py}
 *   param = QgsProcessingParameterNumber( 'VAL', 'Threshold', type=QgsProcessingParameter.Double)
 *   # only show two decimal places in parameter's widgets, not 6:
 *   param.setMetadata( {'widget_wrapper':
 *     { 'decimals': 2 }
 *   })
 * \endcode
 *
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
                                           double minValue = std::numeric_limits<double>::lowest() + 1,
                                           double maxValue = std::numeric_limits<double>::max()
                                         );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "number" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString toolTip() const override;

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
    void setDataType( Type type );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterNumber *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    double mMin = std::numeric_limits<double>::lowest() + 1;
    double mMax = std::numeric_limits<double>::max();
    Type mDataType = Integer;
};

/**
 * \class QgsProcessingParameterDistance
 * \ingroup core
 * A double numeric parameter for distance values. Linked to a source layer or CRS parameter
 * to determine what units the distance values are in.
 *
 * The number of decimals places shown in a distance parameter's widget can be specified by
 * setting the parameter's metadata. For example:
 *
 * * \code{.py}
 *   param = QgsProcessingParameterDistance( 'VAL', 'Threshold')
 *   # only show two decimal places in parameter's widgets, not 6:
 *   param.setMetadata( {'widget_wrapper':
 *     { 'decimals': 2 }
 *   })
 * \endcode
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterDistance : public QgsProcessingParameterNumber
{
  public:

    /**
     * Constructor for QgsProcessingParameterDistance.
     */
    explicit QgsProcessingParameterDistance( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        const QString &parentParameterName = QString(),
        bool optional = false,
        double minValue = std::numeric_limits<double>::lowest() + 1,
        double maxValue = std::numeric_limits<double>::max() );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "distance" ); }

    QgsProcessingParameterDistance *clone() const override SIP_FACTORY;

    QString type() const override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent parameter, or an empty string if this is not set.
     * \see setParentParameterName()
     */
    QString parentParameterName() const;

    /**
     * Sets the name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentParameterName()
     */
    void setParentParameterName( const QString &parentParameterName );

    /**
     * Returns the default distance unit for the parameter.
     *
     * \see setDefaultUnit()
     * \since QGIS 3.4.3
     */
    QgsUnitTypes::DistanceUnit defaultUnit() const { return mDefaultUnit; }

    /**
     * Sets the default distance \a unit for the parameter.
     *
     * \see defaultUnit()
     * \since QGIS 3.4.3
     */
    void setDefaultUnit( QgsUnitTypes::DistanceUnit unit ) { mDefaultUnit = unit; }

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QString mParentParameterName;
    QgsUnitTypes::DistanceUnit mDefaultUnit = QgsUnitTypes::DistanceUnknownUnit;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "range" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
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
    void setDataType( QgsProcessingParameterNumber::Type dataType );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterRange *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "raster" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterRasterLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "enum" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;

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

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterEnum *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "string" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;

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

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterString *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    bool mMultiLine = false;

};


/**
 * \class QgsProcessingParameterAuthConfig
 * \ingroup core
 * A string parameter for authentication configuration configuration ID values.
 *
 * This parameter allows for users to select from available authentication configurations,
 * or create new authentication configurations as required.
 *
 * QgsProcessingParameterAuthConfig should be evaluated by calling QgsProcessingAlgorithm::parameterAsString().
 *
  * \since QGIS 3.6
 */
class CORE_EXPORT QgsProcessingParameterAuthConfig : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterAuthConfig.
     */
    QgsProcessingParameterAuthConfig( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "authcfg" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterAuthConfig *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "expression" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent layer parameter, or an empty string if this is not set.
     * \see setParentLayerParameterName()
     */
    QString parentLayerParameterName() const;

    /**
     * Sets the name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentLayerParameterName()
     */
    void setParentLayerParameterName( const QString &parentLayerParameterName );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterExpression *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QString mParentLayerParameterName;

};


/**
 * \class QgsProcessingParameterLimitedDataTypes
 * \ingroup core
 * Can be inherited by parameters which require limits to their acceptable data types.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterLimitedDataTypes
{
  public:

    /**
     * Constructor for QgsProcessingParameterLimitedDataTypes, with a list of acceptable data \a types.
     */
    QgsProcessingParameterLimitedDataTypes( const QList< int > &types = QList< int >() );

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

  protected:

    //! List of acceptable data types for the parameter
    QList< int > mDataTypes;
};

/**
 * \class QgsProcessingParameterVectorLayer
 * \ingroup core
 * A vector layer (with or without geometry) parameter for processing algorithms. Consider using
 * the more versatile QgsProcessingParameterFeatureSource wherever possible.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterVectorLayer : public QgsProcessingParameterDefinition, public QgsProcessingParameterLimitedDataTypes
{
  public:

    /**
     * Constructor for QgsProcessingParameterVectorLayer.
     */
    QgsProcessingParameterVectorLayer( const QString &name,
                                       const QString &description = QString(),
                                       const QList< int > &types = QList< int >(),
                                       const QVariant &defaultValue = QVariant(),
                                       bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "vector" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterVectorLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

};

/**
 * \class QgsProcessingParameterMeshLayer
 * \ingroup core
 * A mesh layer parameter for processing algorithms.
  * \since QGIS 3.6
 */
class CORE_EXPORT QgsProcessingParameterMeshLayer : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterMeshLayer.
     */
    QgsProcessingParameterMeshLayer( const QString &name,
                                     const QString &description = QString(),
                                     const QVariant &defaultValue = QVariant(),
                                     bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "mesh" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMeshLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
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

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "field" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent layer parameter, or an empty string if this is not set.
     * \see setParentLayerParameterName()
     */
    QString parentLayerParameterName() const;

    /**
     * Sets the name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentLayerParameterName()
     */
    void setParentLayerParameterName( const QString &parentLayerParameterName );

    /**
     * Returns the acceptable data type for the field.
     * \see setDataType()
     */
    DataType dataType() const;

    /**
     * Sets the acceptable data \a type for the field.
     * \see dataType()
     */
    void setDataType( DataType type );

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

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterField *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QString mParentLayerParameterName;
    DataType mDataType = Any;
    bool mAllowMultiple = false;

};


/**
 * \class QgsProcessingParameterFeatureSource
 * \ingroup core
 * An input feature source (such as vector layers) parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFeatureSource : public QgsProcessingParameterDefinition, public QgsProcessingParameterLimitedDataTypes
{
  public:

    /**
    * Constructor for QgsProcessingParameterFeatureSource.
    */
    QgsProcessingParameterFeatureSource( const QString &name, const QString &description = QString(),
                                         const QList< int > &types = QList< int >(),
                                         const QVariant &defaultValue = QVariant(), bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "source" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFeatureSource *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

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
     *
     * If \a createByDefault is false and the parameter is \a optional, then the destination
     * output will not be created by default.
     */
    QgsProcessingDestinationParameter( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false, bool createByDefault = true );

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
     * \see setSupportsNonFileBasedOutput()
     */
    bool supportsNonFileBasedOutput() const { return mSupportsNonFileBasedOutputs; }

    /**
     * Sets whether the destination parameter supports non filed-based outputs,
     * such as memory layers or direct database outputs.
     * \see supportsNonFileBasedOutput()
     */
    void setSupportsNonFileBasedOutput( bool supportsNonFileBasedOutput ) { mSupportsNonFileBasedOutputs = supportsNonFileBasedOutput; }

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

    /**
     * Returns true if the destination should be created by default. For optional parameters,
     * a return value of false indicates that the destination should not be created by default.
     * \see setCreateByDefault()
     */
    bool createByDefault() const;

    /**
     * Sets whether the destination should be created by default. For optional parameters,
     * a value of false indicates that the destination should not be created by default.
     * \see createByDefault()
     */
    void setCreateByDefault( bool createByDefault );

  protected:

    /**
     * Original (source) provider which this parameter has been derived from.
     * In the case of destination parameters which are part of model algorithms, this
     * will reflect the child algorithm's provider which actually generates the
     * parameter, as opposed to the provider which this parameter belongs to (i.e.
     * the model provider)
     * \since QGIS 3.2
     */
    QgsProcessingProvider *originalProvider() const { return mOriginalProvider; }

  private:

    /**
     * Original (source) provider which this parameter has been derived from.
     * In the case of destination parameters which are part of model algorithms, this
     * will reflect the child algorithm's provider which actually generates the
     * parameter, as opposed to the provider which this parameter belongs to (i.e.
     * the model provider)
     */
    QgsProcessingProvider *mOriginalProvider = nullptr;

    bool mSupportsNonFileBasedOutputs = true;
    bool mCreateByDefault = true;

    friend class QgsProcessingModelAlgorithm;
    friend class TestQgsProcessing;
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
     *
     * If \a createByDefault is false and the parameter is \a optional, then this destination
     * output will not be created by default.
     */
    QgsProcessingParameterFeatureSink( const QString &name, const QString &description = QString(), QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry, const QVariant &defaultValue = QVariant(),
                                       bool optional = false, bool createByDefault = true );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "sink" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns a list of the vector format file extensions supported by this parameter.
     * \see defaultFileExtension()
     * \since QGIS 3.2
     */
    virtual QStringList supportedOutputVectorLayerExtensions() const;

    /**
     * Returns the layer type for sinks associated with the parameter.
     * \see setDataType()
     */
    QgsProcessing::SourceType dataType() const;

    /**
     * Returns true if sink is likely to include geometries. In cases were presence of geometry
     * cannot be reliably determined in advance, this method will default to returning true.
     */
    bool hasGeometry() const;

    /**
     * Sets the layer \a type for the sinks associated with the parameter.
     * \see dataType()
     */
    void setDataType( QgsProcessing::SourceType type );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QString generateTemporaryDestination() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFeatureSink *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QgsProcessing::SourceType mDataType = QgsProcessing::TypeVectorAnyGeometry;
};


/**
 * \class QgsProcessingParameterVectorDestination
 * \ingroup core
 * A vector layer destination parameter, for specifying the destination path for a vector layer
 * created by the algorithm.
 *
 * \note Consider using the more flexible QgsProcessingParameterFeatureSink wherever
 * possible.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterVectorDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterVectorDestination.
     *
     * If \a createByDefault is false and the parameter is \a optional, then this destination
     * output will not be created by default.
     */
    QgsProcessingParameterVectorDestination( const QString &name, const QString &description = QString(), QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry, const QVariant &defaultValue = QVariant(),
        bool optional = false, bool createByDefault = true );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "vectorDestination" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns a list of the vector format file extensions supported by this parameter.
     * \see defaultFileExtension()
     * \since QGIS 3.2
     */
    virtual QStringList supportedOutputVectorLayerExtensions() const;

    /**
     * Returns the layer type for this created vector layer.
     * \see setDataType()
     */
    QgsProcessing::SourceType dataType() const;

    /**
     * Returns true if the created layer is likely to include geometries. In cases were presence of geometry
     * cannot be reliably determined in advance, this method will default to returning true.
     */
    bool hasGeometry() const;

    /**
     * Sets the layer \a type for the created vector layer.
     * \see dataType()
     */
    void setDataType( QgsProcessing::SourceType type );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterVectorDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;


  private:

    QgsProcessing::SourceType mDataType = QgsProcessing::TypeVectorAnyGeometry;
};

/**
 * \class QgsProcessingParameterRasterDestination
 * \ingroup core
 * A raster layer destination parameter, for specifying the destination path for a raster layer
 * created by the algorithm.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRasterDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterRasterDestination.
     *
     * If \a createByDefault is false and the parameter is \a optional, then this destination
     * output will not be created by default.
     */
    QgsProcessingParameterRasterDestination( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        bool optional = false,
        bool createByDefault = true );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "rasterDestination" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns a list of the raster format file extensions supported for this parameter.
     * \see defaultFileExtension()
     * \since QGIS 3.2
     */
    virtual QStringList supportedOutputRasterLayerExtensions() const;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterRasterDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
};

/**
 * \class QgsProcessingParameterFileDestination
 * \ingroup core
 * A generic file based destination parameter, for specifying the destination path for a file (non-map layer)
 * created by the algorithm.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFileDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterFileDestination.
     *
     * If \a createByDefault is false and the parameter is \a optional, then this destination
     * output will not be created by default.
     */
    QgsProcessingParameterFileDestination( const QString &name, const QString &description = QString(),
                                           const QString &fileFilter = QString(),
                                           const QVariant &defaultValue = QVariant(),
                                           bool optional = false,
                                           bool createByDefault = true );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "fileDestination" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Returns the file filter string for file destinations compatible with this parameter.
     * \see setFileFilter()
     */
    QString fileFilter() const;

    /**
     * Sets the file \a filter string for file destinations compatible with this parameter.
     * \see fileFilter()
     */
    void setFileFilter( const QString &filter );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFileDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;


  private:

    QString mFileFilter;
};

/**
 * \class QgsProcessingParameterFolderDestination
 * \ingroup core
 * A folder destination parameter, for specifying the destination path for a folder created
 * by the algorithm or used for creating new files within the algorithm.
 * A folder output parameter.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFolderDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterFolderDestination.
     */
    QgsProcessingParameterFolderDestination( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "folderDestination" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFolderDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

};

/**
 * \class QgsProcessingParameterBand
 * \ingroup core
 * A raster band parameter for Processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterBand : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterBand.
     */
    QgsProcessingParameterBand( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                const QString &parentLayerParameterName = QString(),
                                bool optional = false,
                                bool allowMultiple = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "band" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent layer parameter, or an empty string if this is not set.
     * \see setParentLayerParameterName()
     */
    QString parentLayerParameterName() const;

    /**
     * Sets the name of the parent layer parameter. Use an empty string if this is not required.
     * \see parentLayerParameterName()
     */
    void setParentLayerParameterName( const QString &parentLayerParameterName );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterBand *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

    /**
     * Returns whether multiple band selections are permitted.
     * \see setAllowMultiple()
     * \since QGIS 3.4
     */
    bool allowMultiple() const;

    /**
     * Sets whether multiple band selections are permitted.
     * \see allowMultiple()
     * \since QGIS 3.4
     */
    void setAllowMultiple( bool allowMultiple );

  private:

    QString mParentLayerParameterName;
    bool mAllowMultiple = false;
};

// clazy:excludeall=qstring-allocations

#endif // QGSPROCESSINGPARAMETERS_H


