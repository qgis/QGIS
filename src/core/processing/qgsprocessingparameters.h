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
#include "qgsfilefiltergenerator.h"
#include "qgsremappingproxyfeaturesink.h"
#include <QMap>
#include <limits>

class QgsProcessingContext;
class QgsProcessingAlgorithm;
class QgsRasterLayer;
class QgsMeshLayer;
class QgsVectorLayer;
class QgsFeatureSink;
class QgsProcessingFeatureSource;
class QgsProcessingOutputDefinition;
class QgsProcessingFeedback;
class QgsProcessingProvider;
class QgsPrintLayout;
class QgsLayoutItem;
class QgsPointCloudLayer;
class QgsAnnotationLayer;

/**
 * \class QgsProcessingFeatureSourceDefinition
 * \ingroup core
 *
 * \brief Encapsulates settings relating to a feature source input to a processing algorithm.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingFeatureSourceDefinition
{
  public:

    /**
     * Flags which control source behavior.
     * \since QGIS 3.14
     */
    enum Flag
    {
      FlagOverrideDefaultGeometryCheck = 1 << 0, //!< If set, the default geometry check method (as dictated by QgsProcessingContext) will be overridden for this source
      FlagCreateIndividualOutputPerInputFeature = 1 << 1, //!< If set, every feature processed from this source will be placed into its own individually created output destination. Support for this flag depends on how an algorithm is executed.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingFeatureSourceDefinition, accepting a static string \a source.
     *
     * If \a selectedFeaturesOnly is TRUE, then only selected features from the source will be used.
     *
     * The optional \a featureLimit can be set to a value > 0 to place a hard limit on the maximum number
     * of features which will be read from the source.
     *
     * The \a flags argument can be used to specify flags which dictate the source behavior.
     *
     * If the QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck is set in \a flags, then the value of \a geometryCheck will override
     * the default geometry check method (as dictated by QgsProcessingContext) for this source.
     */
    QgsProcessingFeatureSourceDefinition( const QString &source = QString(), bool selectedFeaturesOnly = false, long long featureLimit = -1,
                                          QgsProcessingFeatureSourceDefinition::Flags flags = QgsProcessingFeatureSourceDefinition::Flags(), QgsFeatureRequest::InvalidGeometryCheck geometryCheck = QgsFeatureRequest::GeometryAbortOnInvalid )
      : source( QgsProperty::fromValue( source ) )
      , selectedFeaturesOnly( selectedFeaturesOnly )
      , featureLimit( featureLimit )
      , flags( flags )
      , geometryCheck( geometryCheck )
    {}

    /**
     * Constructor for QgsProcessingFeatureSourceDefinition, accepting a QgsProperty source.
     *
     * If \a selectedFeaturesOnly is TRUE, then only selected features from the source will be used.
     *
     * The optional \a featureLimit can be set to a value > 0 to place a hard limit on the maximum number
     * of features which will be read from the source.
     *
     * The \a flags argument can be used to specify flags which dictate the source behavior.
     *
     * If the QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck is set in \a flags, then the value of \a geometryCheck will override
     * the default geometry check method (as dictated by QgsProcessingContext) for this source.
     */
    QgsProcessingFeatureSourceDefinition( const QgsProperty &source, bool selectedFeaturesOnly = false, long long featureLimit = -1,
                                          QgsProcessingFeatureSourceDefinition::Flags flags = QgsProcessingFeatureSourceDefinition::Flags(), QgsFeatureRequest::InvalidGeometryCheck geometryCheck = QgsFeatureRequest::GeometryAbortOnInvalid )
      : source( source )
      , selectedFeaturesOnly( selectedFeaturesOnly )
      , featureLimit( featureLimit )
      , flags( flags )
      , geometryCheck( geometryCheck )
    {}

    /**
     * Source definition. Usually a static property set to a source layer's ID or file name.
     */
    QgsProperty source;

    /**
     * TRUE if only selected features in the source should be used by algorithms.
     */
    bool selectedFeaturesOnly;

    /**
     * If set to a value > 0, places a limit on the maximum number of features which will be
     * read from the source.
     *
     * \since QGIS 3.14
     */
    long long featureLimit = -1;

    /**
     * Flags which dictate source behavior.
     *
     * \since QGIS 3.14
     */
    Flags flags = Flags();

    /**
     * Geometry check method to apply to this source. This setting is only
     * utilized if the QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature is
     * set in QgsProcessingFeatureSourceDefinition::flags.
     *
     * \see overrideDefaultGeometryCheck
     * \since QGIS 3.14
     */
    QgsFeatureRequest::InvalidGeometryCheck geometryCheck = QgsFeatureRequest::GeometryAbortOnInvalid;

    /**
     * Saves this source definition to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     * \see loadVariant()
     * \since QGIS 3.14
     */
    QVariant toVariant() const;

    /**
     * Loads this source definition from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to load it from an XML document.
     * \see toVariant()
     * \since QGIS 3.14
     */
    bool loadVariant( const QVariantMap &map );

    // TODO c++20 - replace with = default
    bool operator==( const QgsProcessingFeatureSourceDefinition &other ) const
    {
      return source == other.source
             && selectedFeaturesOnly == other.selectedFeaturesOnly
             && featureLimit == other.featureLimit
             && flags == other.flags
             && geometryCheck == other.geometryCheck;
    }

    bool operator!=( const QgsProcessingFeatureSourceDefinition &other ) const
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
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingFeatureSourceDefinition::Flags )

/**
 * \class QgsProcessingOutputLayerDefinition
 * \ingroup core
 *
 * \brief Encapsulates settings relating to a feature sink or output raster layer for a processing algorithm.
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
     * The default behavior is not to load the result into any project (NULLPTR).
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
     * Returns TRUE if the output uses a remapping definition.
     *
     * \see remappingDefinition()
     * \since QGIS 3.14
     */
    bool useRemapping() const { return mUseRemapping; }

    /**
     * Returns the output remapping definition, if useRemapping() is TRUE.
     *
     * \see useRemapping()
     * \see setRemappingDefinition()
     * \since QGIS 3.14
     */
    QgsRemappingSinkDefinition remappingDefinition() const { return mRemappingDefinition; }

    /**
     * Sets the remapping \a definition to use when adding features to the output layer.
     *
     * Calling this method will set useRemapping() to TRUE.
     *
     * \see remappingDefinition()
     * \see useRemapping()
     *
     * \since QGIS 3.14
     */
    void setRemappingDefinition( const QgsRemappingSinkDefinition &definition );

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

    // TODO c++20 - replace with = default
    bool operator==( const QgsProcessingOutputLayerDefinition &other ) const;
    bool operator!=( const QgsProcessingOutputLayerDefinition &other ) const;

  private:

    bool mUseRemapping = false;
    QgsRemappingSinkDefinition mRemappingDefinition;

};

Q_DECLARE_METATYPE( QgsProcessingOutputLayerDefinition )




//
// Parameter definitions
//

/**
 * \class QgsProcessingParameterDefinition
 * \ingroup core
 *
 * \brief Base class for the definition of processing parameters.
 *
 * Parameter definitions encapsulate properties regarding the behavior of parameters,
 * their acceptable ranges, defaults, etc.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingParameterDefinition
{

#ifdef SIP_RUN
    % TypeHeaderCode
#include "qgsprocessingparameteraggregate.h"
#include "qgsprocessingparameterdxflayers.h"
#include "qgsprocessingparameterfieldmap.h"
#include "qgsprocessingparametertininputlayers.h"
#include "qgsprocessingparametervectortilewriterlayers.h"
#include "qgsprocessingparametermeshdataset.h"
    % End
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
    else if ( sipCpp->type() == QgsProcessingParameterGeometry::typeName() )
      sipType = sipType_QgsProcessingParameterGeometry;
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
    else if ( sipCpp->type() == QgsProcessingParameterDuration::typeName() )
      sipType = sipType_QgsProcessingParameterDuration;
    else if ( sipCpp->type() == QgsProcessingParameterScale::typeName() )
      sipType = sipType_QgsProcessingParameterScale;
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
    else if ( sipCpp->type() == QgsProcessingParameterPointCloudDestination::typeName() )
      sipType = sipType_QgsProcessingParameterPointCloudDestination;
    else if ( sipCpp->type() == QgsProcessingParameterFileDestination::typeName() )
      sipType = sipType_QgsProcessingParameterFileDestination;
    else if ( sipCpp->type() == QgsProcessingParameterFolderDestination::typeName() )
      sipType = sipType_QgsProcessingParameterFolderDestination;
    else if ( sipCpp->type() == QgsProcessingParameterBand::typeName() )
      sipType = sipType_QgsProcessingParameterBand;
    else if ( sipCpp->type() == QgsProcessingParameterLayout::typeName() )
      sipType = sipType_QgsProcessingParameterLayout;
    else if ( sipCpp->type() == QgsProcessingParameterLayoutItem::typeName() )
      sipType = sipType_QgsProcessingParameterLayoutItem;
    else if ( sipCpp->type() == QgsProcessingParameterColor::typeName() )
      sipType = sipType_QgsProcessingParameterColor;
    else if ( sipCpp->type() == QgsProcessingParameterCoordinateOperation::typeName() )
      sipType = sipType_QgsProcessingParameterCoordinateOperation;
    else if ( sipCpp->type() == QgsProcessingParameterMapTheme::typeName() )
      sipType = sipType_QgsProcessingParameterMapTheme;
    else if ( sipCpp->type() == QgsProcessingParameterDateTime::typeName() )
      sipType = sipType_QgsProcessingParameterDateTime;
    else if ( sipCpp->type() == QgsProcessingParameterProviderConnection::typeName() )
      sipType = sipType_QgsProcessingParameterProviderConnection;
    else if ( sipCpp->type() == QgsProcessingParameterDatabaseSchema::typeName() )
      sipType = sipType_QgsProcessingParameterDatabaseSchema;
    else if ( sipCpp->type() == QgsProcessingParameterDatabaseTable::typeName() )
      sipType = sipType_QgsProcessingParameterDatabaseTable;
    else if ( sipCpp->type() == QgsProcessingParameterFieldMapping::typeName() )
      sipType = sipType_QgsProcessingParameterFieldMapping;
    else if ( sipCpp->type() == QgsProcessingParameterTinInputLayers::typeName() )
      sipType = sipType_QgsProcessingParameterTinInputLayers;
    else if ( sipCpp->type() == QgsProcessingParameterVectorTileWriterLayers::typeName() )
      sipType = sipType_QgsProcessingParameterVectorTileWriterLayers;
    else if ( sipCpp->type() == QgsProcessingParameterDxfLayers::typeName() )
      sipType = sipType_QgsProcessingParameterDxfLayers;
    else if ( sipCpp->type() == QgsProcessingParameterMeshDatasetGroups::typeName() )
      sipType = sipType_QgsProcessingParameterMeshDatasetGroups;
    else if ( sipCpp->type() == QgsProcessingParameterMeshDatasetTime::typeName() )
      sipType = sipType_QgsProcessingParameterMeshDatasetTime;
    else if ( sipCpp->type() == QgsProcessingParameterPointCloudLayer::typeName() )
      sipType = sipType_QgsProcessingParameterPointCloudLayer;
    else if ( sipCpp->type() == QgsProcessingParameterAnnotationLayer::typeName() )
      sipType = sipType_QgsProcessingParameterAnnotationLayer;
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
                                      bool optional = false, const QString &help = QString() );

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
     * Returns TRUE if this parameter represents a file or layer destination, e.g. parameters
     * which are used for the destination for layers output by an algorithm will return
     * TRUE.
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
     * Returns the help for the parameter.
     *
     * This is a descriptive (possibly lengthy), translated string explaining
     * the parameter's behavior and use in depth.
     *
     * \see setHelp()
     * \since QGIS 3.16
     */
    QString help() const { return mHelp; }

    /**
     * Sets the \a help for the parameter.
     *
     * The \a help string should be a descriptive, translated string explaining
     * the parameter's behavior and use in depth.
     *
     * \see help()
     * \since QGIS 3.16
     */
    void setHelp( const QString &help ) { mHelp = help; }

    /**
     * Returns the default value for the parameter.
     * \see setDefaultValue()
     * \see defaultValueForGui()
     * \see guiDefaultValueOverride()
     */
    QVariant defaultValue() const { return mDefault; }

    /**
     * Sets the default \a value for the parameter. Caller takes responsibility
     * to ensure that \a value is a valid input for the parameter subclass.
     * \see defaultValue()
     * \see setGuiDefaultValueOverride()
     */
    void setDefaultValue( const QVariant &value ) { mDefault = value; }

    /**
     * Returns the default value to use in the GUI for the parameter.
     *
     * Usually this will return an invalid variant, which indicates that the standard defaultValue()
     * will be used in the GUI.
     *
     * \see defaultValue()
     * \see setGuiDefaultValueOverride()
     * \see defaultValueForGui()
     *
     * \since QGIS 3.18
     */
    QVariant guiDefaultValueOverride() const { return mGuiDefault; }

    /**
     * Sets the default \a value to use for the parameter in GUI widgets. Caller takes responsibility
     * to ensure that \a value is a valid input for the parameter subclass.
     *
     * Usually the guiDefaultValueOverride() is a invalid variant, which indicates that the standard defaultValue()
     * should be used in the GUI. In cases where it is decided that a previous default value was inappropriate,
     * setting a non-invalid default GUI value can be used to change the default value for the parameter shown
     * to users when running algorithms without changing the actual defaultValue() and potentially breaking
     * third party scripts.
     *
     * \see guiDefaultValueOverride()
     * \see setDefaultValue()
     *
     * \since QGIS 3.18
     */
    void setGuiDefaultValueOverride( const QVariant &value ) { mGuiDefault = value; }

    /**
     * Returns the default value to use for the parameter in a GUI.
     *
     * This will be the parameter's defaultValue(), unless a guiDefaultValueOverride() is set to
     * override that.
     *
     * \since QGIS 3.18
     */
    QVariant defaultValueForGui() const { return mGuiDefault.isValid() ? mGuiDefault : mDefault; }

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
     * parameter. Returns TRUE if the value can be accepted.
     * The optional \a context parameter can be specified to allow a more stringent
     * check to be performed, capable of checking for the presence of required
     * layers and other factors within the context.
     */
    virtual bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const;

    /**
     * Returns a string version of the parameter input \a value, which is suitable for use as an input
     * parameter value when running an algorithm directly from a Python command.
     *
     * \see valueAsJsonObject()
     * \see valueAsString()
     */
    virtual QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const;

    /**
     * Returns a version of the parameter input \a value, which is suitable for use in a JSON object.
     *
     * This method must return only simple values which can be losslessly encapsulated in a serialized
     * JSON map. For instance, any QGIS class values (such as QgsCoordinateReferenceSystem) must be
     * converted to a simple string or numeric value equivalent.
     *
     * \see valueAsPythonString()
     * \see valueAsString()
     * \since QGIS 3.24
     */
    virtual QVariant valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const;

    /**
     * Returns a string version of the parameter input \a value (if possible).
     *
     * \param value value to convert
     * \param context processing context
     * \param ok will be set to TRUE if value could be represented as a string.
     * \returns value converted to string
     *
     * \see valueAsStringList()
     * \see valueAsJsonObject()
     * \see valueAsPythonString()
     * \since QGIS 3.24
     */
    virtual QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const;

    /**
     * Returns a string list version of the parameter input \a value (if possible).
     *
     * \param value value to convert
     * \param context processing context
     * \param ok will be set to TRUE if value could be represented as a string list
     * \returns value converted to string list
     *
     * \see valueAsString()
     * \see valueAsJsonObject()
     * \see valueAsPythonString()
     * \since QGIS 3.24
     */
    virtual QStringList valueAsStringList( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const;

    /**
     * Returns a Python comment explaining a parameter \a value, or an empty string if no comment is required.
     *
     * \since QGIS 3.20
     */
    virtual QString valueAsPythonComment( const QVariant &value, QgsProcessingContext &context ) const;

    /**
     * Returns the parameter definition encoded in a string which can be used within a
     * Processing script.
     */
    virtual QString asScriptCode() const;

    /**
     * Returns the parameter definition as a Python command which can be used within a
     * Python Processing script.
     *
     * The \a outputType argument specifies the desired output format for the Python string,
     * i.e. the intended end use of the generated Python code.
     *
     * \since QGIS 3.6
     */
    virtual QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const;

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
     * Returns a pointer to the algorithm which owns this parameter. May be NULLPTR
     * for non-owned parameters.
     * \see provider()
     */
    QgsProcessingAlgorithm *algorithm() const;

    /**
     * Returns a pointer to the provider for the algorithm which owns this parameter. May be NULLPTR
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
     * Returns TRUE if the parameter supports is dynamic, and can support data-defined values
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

    /**
     * Returns a list of additional expression context variables which are available for use when evaluating
     * this parameter.
     *
     * The additional variables will be added to the variables exposed from the usual expression
     * context available to the parameter. They can be used to expose variables which are ONLY available
     * to this parameter.
     *
     * The returned list should contain the variable names only, without the usual "@" prefix.
     *
     * \see setAdditionalExpressionContextVariables()
     * \since QGIS 3.8
     */
    QStringList additionalExpressionContextVariables() const { return mAdditionalExpressionVariables; }

    /**
     * Sets a list of additional expression context \a variables which are available for use when evaluating
     * this parameter.
     *
     * The additional variables will be added to the variables exposed from the usual expression
     * context available to the parameter. They can be used to expose variables which are ONLY available
     * to this parameter.
     *
     * The \a variables list should contain the variable names only, without the usual "@" prefix.
     *
     * \note Specifying variables via this method is for metadata purposes only. It is the algorithm's responsibility
     * to correctly set the value of these additional variables in all expression context used when evaluating the parameter,
     * in whichever way is appropriate for that particular variable.
     *
     * \see additionalExpressionContextVariables()
     * \since QGIS 3.8
     */
    void setAdditionalExpressionContextVariables( const QStringList &variables ) { mAdditionalExpressionVariables = variables; }

  protected:

    //! Parameter name
    QString mName;

    //! Parameter description
    QString mDescription;

    //! Parameter help
    QString mHelp;

    //! Default value for parameter
    QVariant mDefault;

    //! Default value for parameter in GUI
    QVariant mGuiDefault;

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

    //! Additional expression context variables exposed for use by this parameter
    QStringList mAdditionalExpressionVariables;

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
 * \brief A collection of utilities for working with parameters when running a processing algorithm.
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
     * Returns TRUE if the parameter with matching \a name is a dynamic parameter, and must
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
     * Evaluates the parameter with matching \a definition to a static datetime value.
     *
     * \see parameterAsDate()
     * \see parameterAsTime()
     *
     * \since QGIS 3.14
     */
    static QDateTime parameterAsDateTime( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static datetime value.
     *
     * \see parameterAsDate()
     * \see parameterAsTime()
     *
     * \since QGIS 3.14
     */
    static QDateTime parameterAsDateTime( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static date value.
     *
     * \see parameterAsDateTime()
     * \see parameterAsTime()
     *
     * \since QGIS 3.14
     */
    static QDate parameterAsDate( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static date value.
     *
     * \see parameterAsDateTime()
     * \see parameterAsTime()
     *
     * \since QGIS 3.14
     */
    static QDate parameterAsDate( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static time value.
     *
     * \see parameterAsDateTime()
     * \see parameterAsDate()
     *
     * \since QGIS 3.14
     */
    static QTime parameterAsTime( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static time value.
     *
     * \see parameterAsDateTime()
     * \see parameterAsDate()
     *
     * \since QGIS 3.14
     */
    static QTime parameterAsTime( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

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
     * Evaluates the parameter with matching \a definition to a static enum string.
     * \since QGIS 3.18
     */
    static QString parameterAsEnumString( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static enum string.
     * \since QGIS 3.18
     */
    static QString parameterAsEnumString( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to list of static enum strings.
     * \since QGIS 3.18
     */
    static QStringList parameterAsEnumStrings( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to list of static enum strings.
     * \since QGIS 3.18
     */
    static QStringList parameterAsEnumStrings( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static boolean value.
     */
    static bool parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a static boolean value.
     *
     * \since QGIS 3.8
     */
    static bool parameterAsBoolean( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static boolean value.
     * \since QGIS 3.4
     */
    static bool parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a static boolean value.
     * \since QGIS 3.8
     */
    static bool parameterAsBoolean( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

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
     * The \a createOptions argument is used to pass on creation options such as layer name.
     *
     * The \a datasourceOptions and \a layerOptions arguments is used to pass on GDAL-specific format driver options.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     */
    static QgsFeatureSink *parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
                                            const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                            QgsProcessingContext &context, QString &destinationIdentifier SIP_OUT, QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags(), const QVariantMap &createOptions = QVariantMap(), const QStringList &datasourceOptions = QStringList(), const QStringList &layerOptions = QStringList() ) SIP_FACTORY;

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
     * The \a createOptions argument is used to pass on creation options such as layer name.
     *
     * The \a datasourceOptions and \a layerOptions arguments is used to pass on GDAL-specific format driver options.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     * \throws QgsProcessingException
     * \since QGIS 3.4
     */
    static QgsFeatureSink *parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariant &value,
                                            const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
                                            QgsProcessingContext &context, QString &destinationIdentifier SIP_OUT, QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags(), const QVariantMap &createOptions = QVariantMap(), const QStringList &datasourceOptions = QStringList(), const QStringList &layerOptions = QStringList() ) SIP_THROW( QgsProcessingException ) SIP_FACTORY;

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
     *
     * When an algorithm is capable of handling multi-layer input files (such as Geopackage), it is preferable
     * to use parameterAsCompatibleSourceLayerPathAndLayerName() which may avoid conversion in more situations.
     */
    static QString parameterAsCompatibleSourceLayerPath( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
        QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat = QString( "shp" ), QgsProcessingFeedback *feedback = nullptr );

    /**
     * Evaluates the parameter with matching \a definition to a source vector layer file path and layer name of compatible format.
     *
     * If the parameter is evaluated to an existing layer, and that layer is not of the format listed in the
     * \a compatibleFormats argument, then the layer will first be exported to a compatible format
     * in a temporary location. The function will then return the path to that temporary file.
     *
     * \a compatibleFormats should consist entirely of lowercase file extensions, e.g. 'shp'.
     *
     * The \a preferredFormat argument is used to specify to desired file extension to use when a temporary
     * layer export is required. This defaults to shapefiles, because shapefiles are the future (don't believe the geopackage hype!).
     *
     * This method should be preferred over parameterAsCompatibleSourceLayerPath() when an algorithm is able
     * to correctly handle files with multiple layers. Unlike parameterAsCompatibleSourceLayerPath(), it will not force
     * a conversion in this case and will return the target layer name in the \a layerName argument.
     *
     * \param definition associated parameter definition
     * \param parameters input parameter value map
     * \param context processing context
     * \param compatibleFormats a list of lowercase file extensions compatible with the algorithm
     * \param preferredFormat preferred format extension to use if conversion if required
     * \param feedback feedback object
     * \param layerName will be set to the target layer name for multi-layer sources (e.g. Geopackage)
     *
     * \returns path to source layer, or nearly converted compatible layer
     *
     * \see parameterAsCompatibleSourceLayerPath()
     * \since QGIS 3.10
     */
    static QString parameterAsCompatibleSourceLayerPathAndLayerName( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters,
        QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat = QString( "shp" ), QgsProcessingFeedback *feedback = nullptr, QString *layerName SIP_OUT = nullptr );

    /**
     * Evaluates the parameter with matching \a definition to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsMapLayer *parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingUtils::LayerHint layerHint = QgsProcessingUtils::LayerHint::UnknownType );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.4
     */
    static QgsMapLayer *parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsProcessingUtils::LayerHint layerHint = QgsProcessingUtils::LayerHint::UnknownType );

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
     * Returns the coordinate reference system associated with an extent parameter value.
     *
     * \see parameterAsExtent()
     */
    static QgsCoordinateReferenceSystem parameterAsExtentCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );


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
     * Returns the coordinate reference system associated with an point parameter value.
     *
     * \see parameterAsPoint()
     * \since QGIS 3.8
     */
    static QgsCoordinateReferenceSystem parameterAsPointCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a geometry.
     *
     * \since QGIS 3.16
     */
    static QgsGeometry parameterAsGeometry( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a geometry.
     *
     * \since QGIS 3.16
     */
    static QgsGeometry parameterAsGeometry( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Returns the coordinate reference system associated with a geometry parameter value.
     *
     * \see parameterAsGeometry()
     * \since QGIS 3.16
     */
    static QgsCoordinateReferenceSystem parameterAsGeometryCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Returns the coordinate reference system associated with an point parameter value.
     *
     * \see parameterAsGeometry()
     * \since QGIS 3.16
     */
    static QgsCoordinateReferenceSystem parameterAsGeometryCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

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
     * Evaluates the parameter with matching \a definition to a list of files (for QgsProcessingParameterMultipleLayers in QgsProcessing:TypeFile mode).
     *
     * \since QGIS 3.10
     */
    static QStringList parameterAsFileList( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a list of files (for QgsProcessingParameterMultipleLayers in QgsProcessing:TypeFile mode).
     *
     * \since QGIS 3.10
     */
    static QStringList parameterAsFileList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

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
     * Evaluates the parameter with matching \a definition to a print layout.
     *
     * \warning This method is not safe to run in a background thread, so it must either be used within a prepareAlgorithm
     * implementation (which runs in the main thread), or the algorithm must return the FlagNoThreading flag.
     *
     * \since QGIS 3.8
     */
    static QgsPrintLayout *parameterAsLayout( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a print layout.
     *
     * \warning This method is not safe to run in a background thread, so it must either be used within a prepareAlgorithm
     * implementation (which runs in the main thread), or the algorithm must return the FlagNoThreading flag.
     *
     * \since QGIS 3.8
     */
    static QgsPrintLayout *parameterAsLayout( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a print layout item, taken from the specified \a layout.
     *
     * \warning This method is not safe to run in a background thread, so it must either be used within a prepareAlgorithm
     * implementation (which runs in the main thread), or the algorithm must return the FlagNoThreading flag.
     *
     * \since QGIS 3.8
     */
    static QgsLayoutItem *parameterAsLayoutItem( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsPrintLayout *layout );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a print layout, taken from the specified \a layout.
     *
     * \warning This method is not safe to run in a background thread, so it must either be used within a prepareAlgorithm
     * implementation (which runs in the main thread), or the algorithm must return the FlagNoThreading flag.
     *
     * \since QGIS 3.8
     */
    static QgsLayoutItem *parameterAsLayoutItem( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsPrintLayout *layout );

    /**
     * Returns the color associated with an point parameter value, or an invalid color if the parameter was not set.
     *
     * \since QGIS 3.10
     */
    static QColor parameterAsColor( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Returns the color associated with an color parameter value, or an invalid color if the parameter was not set.
     *
     * \since QGIS 3.10
     */
    static QColor parameterAsColor( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a connection name string.
     *
     * \since QGIS 3.14
     */
    static QString parameterAsConnectionName( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a connection name string.
     *
     * \since QGIS 3.14
     */
    static QString parameterAsConnectionName( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a database schema name.
     *
     * \since QGIS 3.14
     */
    static QString parameterAsSchema( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a database schema name.
     *
     * \since QGIS 3.14
     */
    static QString parameterAsSchema( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a database table name.
     *
     * \since QGIS 3.14
     */
    static QString parameterAsDatabaseTableName( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a database table name.
     *
     * \since QGIS 3.14
     */
    static QString parameterAsDatabaseTableName( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to a point cloud layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.22
     */
    static QgsPointCloudLayer *parameterAsPointCloudLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to a point cloud layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.22
     */
    static QgsPointCloudLayer *parameterAsPointCloudLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition to an annotation layer.
     *
     * Layers will be taken from \a context's active project. Callers do not
     * need to handle deletion of the returned layer.
     *
     * \warning Working with annotation layers is generally not thread safe (unless the layers are from
     * a QgsProject loaded directly in a background thread). Ensure your algorithm returns the
     * QgsProcessingAlgorithm::FlagNoThreading flag or only accesses annotation layers from a prepareAlgorithm()
     * or postProcessAlgorithm() step.
     *
     * \since QGIS 3.22
     */
    static QgsAnnotationLayer *parameterAsAnnotationLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a definition and \a value to an annotation layer.
     *
     * Layers will be taken from \a context's active project. Callers do not
     * need to handle deletion of the returned layer.
     *
     * \warning Working with annotation layers is generally not thread safe (unless the layers are from
     * a QgsProject loaded directly in a background thread). Ensure your algorithm returns the
     * QgsProcessingAlgorithm::FlagNoThreading flag or only accesses annotation layers from a prepareAlgorithm()
     * or postProcessAlgorithm() step.
     *
     * \since QGIS 3.22
     */
    static QgsAnnotationLayer *parameterAsAnnotationLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context );

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
 * \brief A boolean parameter for processing algorithms.
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
 * \brief A coordinate reference system parameter for processing algorithms.
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
 * \class QgsProcessingParameterExtent
 * \ingroup core
 * \brief A rectangular map extent parameter for processing algorithms.
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
 * \brief A point parameter for processing algorithms.
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
 * \class QgsProcessingParameterGeometry
 * \ingroup core
 * \brief A geometry parameter for processing algorithms.
  * \since QGIS 3.16
 */
class CORE_EXPORT QgsProcessingParameterGeometry : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterGeometry.
     *
     * The \a geometryTypes argument allows for specifying a list of geometry types (see QgsWkbTypes::GeometryType) acceptable for this
     * parameter. Passing a empty list will allow for any type of geometry.
     * The \a allowMultiPart argument allows specifying a multi part geometry
     */
    QgsProcessingParameterGeometry( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(), bool optional = false, const QList< int > &geometryTypes = QList< int >(), bool allowMultipart = true );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "geometry" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Returns the parameter allowed geometries, as a list of QgsWkbTypes::GeometryType values.
     * \see setGeometryTypes()
     */
    QList<int>  geometryTypes() const { return mGeomTypes; }

    /**
     * Sets the allowed  \a geometryTypes, as a list of QgsWkbTypes::GeometryType values.
     * \see geometryTypes()
     */
    void setGeometryTypes( const QList<int> &geometryTypes ) { mGeomTypes = geometryTypes; }

    /**
    * Returns the parameter allow multipart geometries.
    * \see setAllowMultipart()
    */
    bool  allowMultipart() const { return mAllowMultipart; }

    /**
     * Sets the allow multipart geometries
     * \see allowMultipart()
     */
    void setAllowMultipart( bool allowMultipart ) { mAllowMultipart = allowMultipart; }



    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterGeometry *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QList<int> mGeomTypes;
    bool mAllowMultipart;

};

/**
 * \class QgsProcessingParameterFile
 * \ingroup core
 * \brief An input file or folder parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFile : public QgsProcessingParameterDefinition, public QgsFileFilterGenerator
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
     *
     * The \a extension argument allows for specifying a file extension associated with the parameter (e.g. "html"). Use \a fileFilter
     * for a more flexible approach which allows for multiple file extensions. Only one of \a extension or \a fileFilter should be specified,
     * if both are specified then \a fileFilter takes precedence.
     */
    QgsProcessingParameterFile( const QString &name, const QString &description = QString(), Behavior behavior = File, const QString &extension = QString(), const QVariant &defaultValue = QVariant(),
                                bool optional = false, const QString &fileFilter = QString() );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "file" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
     *
     * \note See fileFilter() for a more flexible approach.
     *
     * \see setExtension()
     */
    QString extension() const { return mExtension; }

    /**
     * Sets a file \a extension for the parameter.
     *
     * Calling this method resets any existing fileFilter().
     *
     * \note See setFileFilter() for a more flexible approach.
     *
     * \see extension()
     */
    void setExtension( const QString &extension );

    /**
     * Returns the file filter string for file destinations compatible with this parameter.
     * \see setFileFilter()
     * \see extension()
     * \since QGIS 3.10
     */
    QString fileFilter() const;

    /**
     * Sets the file \a filter string for file destinations compatible with this parameter.
     *
     * Calling this method resets any existing extension() setting.
     *
     * \see fileFilter()
     * \see setExtension()
     * \since QGIS 3.10
     */
    void setFileFilter( const QString &filter );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFile *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition, Behavior behavior = File ) SIP_FACTORY;

  private:

    Behavior mBehavior = File;
    QString mExtension;
    QString mFileFilter;
};

/**
 * \class QgsProcessingParameterMatrix
 * \ingroup core
 * \brief A table (matrix) parameter for processing algorithms.
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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
     * effect if hasFixedNumberRows() is TRUE.
     * \see setNumberRows()
     * \see setHasFixedNumberRows()
     */
    int numberRows() const;

    /**
     * Sets the fixed number of \a rows in the table. This parameter only has an
     * effect if hasFixedNumberRows() is TRUE.
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
 * \brief A parameter for processing algorithms which accepts multiple map layers.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterMultipleLayers : public QgsProcessingParameterDefinition, public QgsFileFilterGenerator
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
 * \brief A numeric parameter for processing algorithms.
 *
 * For numeric parameters with a dataType() of Double, the number of decimals places
 * shown in the parameter's widget can be specified by setting the parameter's metadata. For example:
 *
 * \code{.py}
 *   param = QgsProcessingParameterNumber( 'VAL', 'Threshold', type=QgsProcessingParameterNumber.Double)
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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
 * \brief A double numeric parameter for distance values. Linked to a source layer or CRS parameter
 * to determine what units the distance values are in.
 *
 * The number of decimals places shown in a distance parameter's widget can be specified by
 * setting the parameter's metadata. For example:
 *
 * \code{.py}
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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
 * \class QgsProcessingParameterDuration
 * \ingroup core
 * \brief A double numeric parameter for duration values. The returned
 * value will always be in milliseconds.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterDuration : public QgsProcessingParameterNumber
{
  public:

    /**
     * Constructor for QgsProcessingParameterDuration.
     */
    explicit QgsProcessingParameterDuration( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        bool optional = false,
        double minValue = std::numeric_limits<double>::lowest() + 1,
        double maxValue = std::numeric_limits<double>::max() );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "duration" ); }

    QgsProcessingParameterDuration *clone() const override SIP_FACTORY;

    QString type() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Returns the default duration unit for the parameter.
     *
     * \see setDefaultUnit()
     */
    QgsUnitTypes::TemporalUnit defaultUnit() const { return mDefaultUnit; }

    /**
     * Sets the default duration \a unit for the parameter.
     *
     * \see defaultUnit()
     */
    void setDefaultUnit( QgsUnitTypes::TemporalUnit unit ) { mDefaultUnit = unit; }

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

  private:

    QgsUnitTypes::TemporalUnit mDefaultUnit = QgsUnitTypes::TemporalMilliseconds;

};

/**
 * \class QgsProcessingParameterScale
 * \ingroup core
 * \brief A double numeric parameter for map scale values.
 *
 * QgsProcessingParameterScale should be evaluated by calling QgsProcessingAlgorithm::parameterAsDouble(),
 * which will return a numeric value representing the scale denominator.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterScale : public QgsProcessingParameterNumber
{
  public:

    /**
     * Constructor for QgsProcessingParameterScale.
     */
    explicit QgsProcessingParameterScale( const QString &name, const QString &description = QString(),
                                          const QVariant &defaultValue = QVariant(),
                                          bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "scale" ); }

    QgsProcessingParameterScale *clone() const override SIP_FACTORY;

    QString type() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterScale *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

};

/**
 * \class QgsProcessingParameterRange
 * \ingroup core
 * \brief A numeric range parameter for processing algorithms.
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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
 * \brief A raster layer parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRasterLayer : public QgsProcessingParameterDefinition, public QgsFileFilterGenerator
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
    QString createFileFilter() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterRasterLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

};

/**
 * \class QgsProcessingParameterEnum
 * \ingroup core
 * \brief An enum based parameter for processing algorithms, allowing for selection from predefined values.
 *
 * Since QGIS 3.24 a list of icons corresponding to the enum values can be specified by setting the
 * widget wrapper metadata "icons" option, as demonstrated below. The "icons" value should be
 * set to a list of QIcon values.
 *
 * \code{.py}
 *   param = QgsProcessingParameterEnum( 'FIELD_TYPE', 'Field type', ['Integer', 'String'])
 *   param.setMetadata( {'widget_wrapper':
 *     { 'icons': [QIcon('integer.svg'), QIcon('string.svg')] }
 *   })
 * \endcode
 *
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
                                bool optional = false,
                                bool usesStaticStrings = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "enum" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString valueAsPythonComment( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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
     * Returns TRUE if the parameter allows multiple selected values.
     * \see setAllowMultiple()
     */
    bool allowMultiple() const;

    /**
     * Sets whether the parameter allows multiple selected values.
     * \see allowMultiple()
     */
    void setAllowMultiple( bool allowMultiple );

    /**
     * Returns TRUE if the parameter uses static (non-translated) string
     * values for its enumeration choice list.
     * \see setUsesStaticStrings()
     * \since QGIS 3.18
     */
    bool usesStaticStrings() const;

    /**
     * Sets whether the parameter uses static (non-translated) string
     * values for its enumeration choice list.
     * \see usesStaticStrings()
     * \since QGIS 3.18
     */
    void setUsesStaticStrings( bool usesStaticStrings );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterEnum *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QStringList mOptions;
    bool mAllowMultiple = false;
    bool mUsesStaticStrings = false;
};

/**
 * \class QgsProcessingParameterString
 * \ingroup core
 * \brief A string parameter for processing algorithms.
 *
 * A parameter type which allows users to enter any string value.
 *
 * In some circumstances it is desirable to restrict the values available
 * when a user is asked to enter a string parameter to a list of predetermined
 * "valid" values. Since QGIS 3.22 this can be done by setting the widget wrapper metadata
 * "value_hints" option, as demonstrated below. (While this provides a mechanism
 * for guiding users to select from valid string values when running a Processing
 * algorithm through the GUI, it does not place any limits on the string values
 * accepted via PyQGIS codes or when running the algorithm via other non-gui
 * means. Algorithms should gracefully handle other values accordingly.)
 *
 * \code{.py}
 *   param = QgsProcessingParameterString( 'PRINTER_NAME', 'Printer name')
 *   # show only printers which are available on the current system as options
 *   # for the string input.
 *   param.setMetadata( {'widget_wrapper':
 *     { 'value_hints': ['Inkjet printer', 'Laser printer'] }
 *   })
 * \endcode
 *
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Returns TRUE if the parameter allows multiline strings.
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
 * \brief A string parameter for authentication configuration ID values.
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
 * \brief An expression parameter for processing algorithms.
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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
 * \brief Can be inherited by parameters which require limits to their acceptable data types.
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
 * \brief A vector layer (with or without geometry) parameter for processing algorithms. Consider using
 * the more versatile QgsProcessingParameterFeatureSource wherever possible.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterVectorLayer : public QgsProcessingParameterDefinition, public QgsProcessingParameterLimitedDataTypes, public QgsFileFilterGenerator
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
 * \brief A mesh layer parameter for processing algorithms.
  * \since QGIS 3.6
 */
class CORE_EXPORT QgsProcessingParameterMeshLayer : public QgsProcessingParameterDefinition, public QgsFileFilterGenerator
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
    QString createFileFilter() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMeshLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
};

/**
 * \class QgsProcessingParameterMapLayer
 * \ingroup core
 * \brief A map layer parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterMapLayer : public QgsProcessingParameterDefinition, public QgsProcessingParameterLimitedDataTypes, public QgsFileFilterGenerator
{
  public:

    /**
     * Constructor for QgsProcessingParameterMapLayer.
     */
    QgsProcessingParameterMapLayer( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                    bool optional = false,
                                    const QList< int > &types = QList< int >() );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "layer" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMapLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

};

/**
 * \class QgsProcessingParameterField
 * \ingroup core
 * \brief A vector layer or feature source field parameter for processing algorithms.
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
                                 bool optional = false,
                                 bool defaultToAllFields = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "field" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
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

    /**
     * Returns whether a parameter which allows multiple selections (see allowMultiple()) should automatically
     * select all fields as the default value.
     *
     * If TRUE, this will override any existing defaultValue() set on the parameter.
     *
     * \see setDefaultToAllFields()
     * \since QGIS 3.12
     */
    bool defaultToAllFields() const;

    /**
     * Sets whether a parameter which allows multiple selections (see allowMultiple()) should automatically
     * select all fields as the default value.
     *
     * If TRUE, this will override any existing defaultValue() set on the parameter.
     *
     * \see defaultToAllFields()
     * \since QGIS 3.12
     */
    void setDefaultToAllFields( bool enabled );

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
    bool mDefaultToAllFields = false;

};


/**
 * \class QgsProcessingParameterFeatureSource
 * \ingroup core
 * \brief An input feature source (such as vector layers) parameter for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFeatureSource : public QgsProcessingParameterDefinition, public QgsProcessingParameterLimitedDataTypes, public QgsFileFilterGenerator
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
 * \brief Base class for all parameter definitions which represent file or layer destinations, e.g. parameters
 * which are used for the destination for layers output by an algorithm.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingDestinationParameter : public QgsProcessingParameterDefinition, public QgsFileFilterGenerator
{
  public:

    /**
     * Constructor for QgsProcessingDestinationParameter.
     *
     * If \a createByDefault is FALSE and the parameter is \a optional, then the destination
     * output will not be created by default.
     */
    QgsProcessingDestinationParameter( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                       bool optional = false, bool createByDefault = true );

    bool isDestination() const override { return true; }
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

    /**
     * Returns a new QgsProcessingOutputDefinition corresponding to the definition of the destination
     * parameter.
     */
    virtual QgsProcessingOutputDefinition *toOutputDefinition() const = 0 SIP_FACTORY;

    /**
     * Returns TRUE if the destination parameter supports non filed-based outputs,
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
     * Tests whether a \a value is a supported value for this parameter.
     *
     * Will return FALSE when a \a value with an unsupported file extension is specified. The default implementation
     * calls QgsProcessingProvider::isSupportedOutputValue() to test compatibility.
     *
     * \param value value to test
     * \param context Processing context
     * \param error will be set to a descriptive error string
     *
     * \returns TRUE if \a value is supported.
     *
     * \since QGIS 3.14
     */
    virtual bool isSupportedOutputValue( const QVariant &value, QgsProcessingContext &context, QString &error SIP_OUT ) const;

    /**
     * Returns TRUE if the destination should be created by default. For optional parameters,
     * a return value of FALSE indicates that the destination should not be created by default.
     * \see setCreateByDefault()
     */
    bool createByDefault() const;

    /**
     * Sets whether the destination should be created by default. For optional parameters,
     * a value of FALSE indicates that the destination should not be created by default.
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
    friend class TestQgsProcessingModelAlgorithm;
};


/**
 * \class QgsProcessingParameterFeatureSink
 * \ingroup core
 * \brief A feature sink output for processing algorithms.
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
     * If \a createByDefault is FALSE and the parameter is \a optional, then this destination
     * output will not be created by default.
     */
    QgsProcessingParameterFeatureSink( const QString &name, const QString &description = QString(), QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry, const QVariant &defaultValue = QVariant(),
                                       bool optional = false, bool createByDefault = true, bool supportsAppend = false );

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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
     * Returns TRUE if sink is likely to include geometries. In cases were presence of geometry
     * cannot be reliably determined in advance, this method will default to returning TRUE.
     */
    bool hasGeometry() const;

    /**
     * Sets the layer \a type for the sinks associated with the parameter.
     * \see dataType()
     */
    void setDataType( QgsProcessing::SourceType type );

    /**
     * Returns TRUE if the sink supports appending features to an existing table.
     *
     * A sink only supports appending if the algorithm implements QgsProcessingAlgorithm::sinkProperties for the sink parameter.
     *
     * \see setSupportsAppend()
     * \since QGIS 3.14
     */
    bool supportsAppend() const;

    /**
     * Sets whether the sink supports appending features to an existing table.
     *
     * \warning A sink only supports appending if the algorithm implements QgsProcessingAlgorithm::sinkProperties for the sink parameter.
     *
     * \see supportsAppend()
     * \since QGIS 3.14
     */
    void setSupportsAppend( bool supportsAppend );

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QString generateTemporaryDestination() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterFeatureSink *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QgsProcessing::SourceType mDataType = QgsProcessing::TypeVectorAnyGeometry;
    bool mSupportsAppend = false;
};


/**
 * \class QgsProcessingParameterVectorDestination
 * \ingroup core
 * \brief A vector layer destination parameter, for specifying the destination path for a vector layer
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
     * If \a createByDefault is FALSE and the parameter is \a optional, then this destination
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
     * Returns TRUE if the created layer is likely to include geometries. In cases were presence of geometry
     * cannot be reliably determined in advance, this method will default to returning TRUE.
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
 * \brief A raster layer destination parameter, for specifying the destination path for a raster layer
 * created by the algorithm.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRasterDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterRasterDestination.
     *
     * If \a createByDefault is FALSE and the parameter is \a optional, then this destination
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
    QString createFileFilter() const override;

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
 * \brief A generic file based destination parameter, for specifying the destination path for a file (non-map layer)
 * created by the algorithm.
 *
 * In some circumstances it is desirable to avoid the usual file overwriting confirmation prompt when
 * users select an existing destination file for this parameter type (e.g., for algorithms which
 * append to an existing destination file instead of overwriting them.). This can be done by setting
 * the widget wrapper metadata "dontconfirmoverwrite" option:
 *
 * \code{.py}
 *   param = QgsProcessingParameterFileDestination( 'OUTPUT', 'Destination file')
 *   # don't show the file overwrite warning when users select a destination file:
 *   param.setMetadata( {'widget_wrapper':
 *     { 'dontconfirmoverwrite': True }
 *   })
 * \endcode
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterFileDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterFileDestination.
     *
     * If \a createByDefault is FALSE and the parameter is \a optional, then this destination
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QString createFileFilter() const override;

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
 * \brief A folder destination parameter, for specifying the destination path for a folder created
 * by the algorithm or used for creating new files within the algorithm.
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
        bool optional = false,
        bool createByDefault = true );

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
 * \brief A raster band parameter for Processing algorithms.
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
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

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

/**
 * \class QgsProcessingParameterLayout
 * \ingroup core
 * \brief A print layout parameter, allowing users to select a print layout.
 *
 * QgsProcessingParameterLayout should be evaluated by calling QgsProcessingAlgorithm::parameterAsLayout().
 * This will return the matching layout from the context's current project. Alternatively, calling
 * QgsProcessingAlgorithm::parameterAsString() will return the name of the target print layout.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterLayout : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterLayout.
     */
    QgsProcessingParameterLayout( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                  bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "layout" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterLayout *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

};

/**
 * \class QgsProcessingParameterLayoutItem
 * \ingroup core
 * \brief A print layout item parameter, allowing users to select a particular item from a print layout.
 *
 * QgsProcessingParameterLayoutItem should be evaluated by calling QgsProcessingAlgorithm::parameterAsLayoutItem().
 * Internally, QgsProcessingParameterLayoutItems are string parameters, storing references to items either by
 * their UUID (QgsLayoutItem::uuid()) or ID (QgsLayoutItem::id()).
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsProcessingParameterLayoutItem : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterLayoutItem.
     */
    QgsProcessingParameterLayoutItem( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                      const QString &parentLayoutParameterName = QString(),
                                      int itemType = -1,
                                      bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "layoutitem" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterLayoutItem *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

    /**
     * Returns the name of the parent layout parameter, or an empty string if this is not set.
     * \see setParentLayoutParameterName()
     */
    QString parentLayoutParameterName() const;

    /**
     * Sets the \a name of the parent layout parameter. Use an empty string if this is not required.
     * \see parentLayoutParameterName()
     */
    void setParentLayoutParameterName( const QString &name );

    /**
     * Returns the acceptable item type, or -1 if any item type is allowed.
     *
     * These values correspond to the registered item types from QgsLayoutItemRegistry.
     *
     * \see setItemType()
     */
    int itemType() const;

    /**
     * Sets the acceptable item \a type, or -1 if any item type is allowed.
     *
     * These values correspond to the registered item types from QgsLayoutItemRegistry.
     *
     * \see itemType()
     */
    void setItemType( int type );

  private:
    QString mParentLayoutParameterName;
    int mItemType = -1;
};

/**
 * \class QgsProcessingParameterColor
 * \ingroup core
 * \brief A color parameter for processing algorithms.
 *
 * QgsProcessingParameterColor should be evaluated by calling QgsProcessingAlgorithm::parameterAsColor().
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsProcessingParameterColor : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterColor.
     *
     * If \a opacityEnabled is TRUE, then users will have the option of varying color opacity.
     */
    QgsProcessingParameterColor( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                 bool opacityEnabled = true,
                                 bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "color" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Returns TRUE if the parameter allows opacity control.
     *
     * The default behavior is to allow users to set opacity for the color.
     * \see setOpacityEnabled()
     */
    bool opacityEnabled() const;

    /**
     * Sets whether the parameter allows opacity control.
     *
     * The default behavior is to allow users to set opacity for the color.
     *
     * \see opacityEnabled()
     */
    void setOpacityEnabled( bool enabled );

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterColor *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    bool mAllowOpacity = true;

};


/**
 * \class QgsProcessingParameterCoordinateOperation
 * \ingroup core
 * \brief A coordinate operation parameter for processing algorithms, for selection between available
 * coordinate operations to use when projecting between a source and destination coordinate reference system.
 *
 * QgsProcessingParameterCoordinateOperation should be evaluated by calling QgsProcessingAlgorithm::parameterAsString().
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsProcessingParameterCoordinateOperation : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterCoordinateOperation.
     */
    QgsProcessingParameterCoordinateOperation( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
        const QString &sourceCrsParameterName = QString(), const QString &destinationCrsParameterName = QString(),
        const QVariant &staticSourceCrs = QVariant(), const QVariant &staticDestinationCrs = QVariant(),
        bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "coordinateoperation" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QStringList dependsOnOtherParameters() const override;

    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterCoordinateOperation *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

    /**
     * Returns the name of the source CRS parameter, or an empty string if this is not set.
     * \see setSourceCrsParameterName()
     * \see destinationCrsParameterName()
     */
    QString sourceCrsParameterName() const { return mSourceParameterName; }

    /**
     * Sets the \a name of the source CRS parameter. Use an empty string if this is not required.
     * \see sourceCrsParameterName()
     * \see setDestinationCrsParameterName()
     */
    void setSourceCrsParameterName( const QString &name ) { mSourceParameterName = name; }

    /**
     * Returns the name of the destination CRS parameter, or an empty string if this is not set.
     * \see setDestinationCrsParameterName()
     * \see sourceCrsParameterName()
     */
    QString destinationCrsParameterName() const { return mDestParameterName; }

    /**
     * Sets the \a name of the destination CRS parameter. Use an empty string if this is not required.
     * \see destinationCrsParameterName()
     * \see setSourceCrsParameterName()
     */
    void setDestinationCrsParameterName( const QString &name ) { mDestParameterName = name; }

    /**
     * Returns the static source CRS, or an invalid value if this is not set.
     * \see setSourceCrs()
     * \see destinationCrs()
     */
    QVariant sourceCrs() const { return mSourceCrs; }

    /**
     * Sets the static source \a crs.
     * \see sourceCrs()
     * \see setDestinationCrs()
     */
    void setSourceCrs( const QVariant &crs ) { mSourceCrs = crs; }

    /**
     * Returns the static destination CRS, or an invalid value if this is not set.
     * \see setDestinationCrs()
     * \see sourceCrs()
     */
    QVariant destinationCrs() const { return mDestCrs; }

    /**
     * Sets the static destination \a crs.
     * \see destinationCrs()
     * \see setSourceCrs()
     */
    void setDestinationCrs( const QVariant &crs ) { mDestCrs = crs; }

  private:

    QString mSourceParameterName;
    QString mDestParameterName;
    QVariant mSourceCrs;
    QVariant mDestCrs;

};


/**
 * \class QgsProcessingParameterMapTheme
 * \ingroup core
 * \brief A map theme parameter for processing algorithms, allowing users to select an existing map theme from a project.
 *
 * QgsProcessingParameterMapTheme should be evaluated by calling QgsProcessingAlgorithm::parameterAsString().
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsProcessingParameterMapTheme : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterMapTheme.
     */
    QgsProcessingParameterMapTheme( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(),
                                    bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "maptheme" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterMapTheme *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

};


/**
 * \class QgsProcessingParameterDateTime
 * \ingroup core
 * \brief A datetime (or pure date or time) parameter for processing algorithms.
 *
 * QgsProcessingParameterDateTime should be evaluated by calling QgsProcessingAlgorithm::parameterAsDateTime(),
 * which will return a date time value.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterDateTime : public QgsProcessingParameterDefinition
{
  public:

    //! Datetime data type
    enum Type
    {
      DateTime, //!< Datetime values
      Date, //!< Date values
      Time, //!< Time values
    };

    /**
     * Constructor for QgsProcessingParameterDateTime.
     */
    explicit QgsProcessingParameterDateTime( const QString &name, const QString &description = QString(),
        Type type = DateTime,
        const QVariant &defaultValue = QVariant(),
        bool optional = false,
        const QDateTime &minValue = QDateTime(),
        const QDateTime &maxValue = QDateTime()
                                           );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "datetime" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString toolTip() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;

    /**
     * Returns the minimum value acceptable by the parameter.
     *
     * An invalid QDateTime value indicates no minimum value.
     *
     * \see setMinimum()
     */
    QDateTime minimum() const;

    /**
     * Sets the \a minimum value acceptable by the parameter.
     *
     * An invalid QDateTime value indicates no minimum value.
     *
     * If the dataType() is QgsProcessingParameterDateTime::Time, then the date component of \a minimum
     * must be set to any valid date (but this date will not actually be considered when comparing parameter
     * values to the specified minimum value, only the time component will be considered).
     *
     * \see minimum()
     */
    void setMinimum( const QDateTime &minimum );

    /**
     * Returns the maximum value acceptable by the parameter.
     *
     * An invalid QDateTime value indicates no maximum value.
     *
     * \see setMaximum()
     */
    QDateTime maximum() const;

    /**
     * Sets the \a maximum value acceptable by the parameter.
     *
     * An invalid QDateTime value indicates no maximum value.
     *
     * If the dataType() is QgsProcessingParameterDateTime::Time, then the date component of \a maximum
     * must be set to any valid date (but this date will not actually be considered when comparing parameter
     * values to the specified maximum value, only the time component will be considered).
     *
     * \see maximum()
     */
    void setMaximum( const QDateTime &maximum );

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
    static QgsProcessingParameterDateTime *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QDateTime mMin;
    QDateTime mMax;
    Type mDataType = DateTime;
};


/**
 * \class QgsProcessingParameterProviderConnection
 * \ingroup core
 * \brief A data provider connection parameter for processing algorithms, allowing users to select from available registered
 * connections for a particular data provider.
 *
 * QgsProcessingParameterProviderConnection should be evaluated by calling QgsProcessingAlgorithm::parameterAsConnectionName().
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterProviderConnection : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterProviderConnection, for the specified \a provider type.
     *
     * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
     * in order for the model to work correctly. This is only implemented for a subset of current data providers.
     */
    QgsProcessingParameterProviderConnection( const QString &name, const QString &description, const QString &provider, const QVariant &defaultValue = QVariant(),
        bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "providerconnection" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;

    /**
     * Returns the ID of the provider associated with the connections.
     * \see setProviderId()
     */
    QString providerId() const { return mProviderId; }

    /**
     * Sets the ID of the \a provider associated with the connections.
     * \see providerId()
     */
    void setProviderId( const QString &provider ) { mProviderId = provider; }

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterProviderConnection *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QString mProviderId;
};


/**
 * \class QgsProcessingParameterDatabaseSchema
 * \ingroup core
 * \brief A database schema parameter for processing algorithms, allowing users to select from existing schemas
 * on a registered database connection.
 *
 * QgsProcessingParameterDatabaseSchema should be evaluated by calling QgsProcessingAlgorithm::parameterAsSchema().
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterDatabaseSchema : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterDatabaseSchema.
     *
     * The \a connectionParameterName specifies the name of the parent QgsProcessingParameterProviderConnection parameter.
     *
     * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
     * in order for the model to work correctly. This is only implemented for a subset of current data providers.
     */
    QgsProcessingParameterDatabaseSchema( const QString &name, const QString &description, const QString &connectionParameterName = QString(), const QVariant &defaultValue = QVariant(),
                                          bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "databaseschema" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent connection parameter, or an empty string if this is not set.
     * \see setParentConnectionParameterName()
     */
    QString parentConnectionParameterName() const;

    /**
     * Sets the \a name of the parent connection parameter. Use an empty string if this is not required.
     * \see parentConnectionParameterName()
     */
    void setParentConnectionParameterName( const QString &name );

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterDatabaseSchema *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

  private:

    QString mParentConnectionParameterName;
};


/**
 * \class QgsProcessingParameterDatabaseTable
 * \ingroup core
 * \brief A database table name parameter for processing algorithms, allowing users to select from existing database tables
 * on a registered database connection (or optionally to enter a new table name).
 *
 * QgsProcessingParameterDatabaseTable should be evaluated by calling QgsProcessingAlgorithm::parameterAsDatabaseTableName().
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingParameterDatabaseTable : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterDatabaseTable.
     *
     * The \a connectionParameterName specifies the name of the parent QgsProcessingParameterProviderConnection parameter.
     * The \a schemaParameterName specifies the name of the parent QgsProcessingParameterDatabaseSchema parameter.
     *
     * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
     * in order for the model to work correctly. This is only implemented for a subset of current data providers.
     */
    QgsProcessingParameterDatabaseTable( const QString &name, const QString &description,
                                         const QString &connectionParameterName = QString(),
                                         const QString &schemaParameterName = QString(),
                                         const QVariant &defaultValue = QVariant(),
                                         bool optional = false,
                                         bool allowNewTableNames = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "databasetable" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString asScriptCode() const override;
    QString asPythonString( QgsProcessing::PythonOutputType outputType = QgsProcessing::PythonQgsProcessingAlgorithmSubclass ) const override;
    QVariantMap toVariantMap() const override;
    bool fromVariantMap( const QVariantMap &map ) override;
    QStringList dependsOnOtherParameters() const override;

    /**
     * Returns the name of the parent connection parameter, or an empty string if this is not set.
     * \see setParentConnectionParameterName()
     */
    QString parentConnectionParameterName() const;

    /**
     * Sets the \a name of the parent connection parameter. Use an empty string if this is not required.
     * \see parentConnectionParameterName()
     */
    void setParentConnectionParameterName( const QString &name );

    /**
     * Returns the name of the parent schema parameter, or an empty string if this is not set.
     * \see setParentSchemaParameterName()
     */
    QString parentSchemaParameterName() const;

    /**
     * Sets the \a name of the parent schema parameter. Use an empty string if this is not required.
     * \see parentSchemaParameterName()
     */
    void setParentSchemaParameterName( const QString &name );

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterDatabaseTable *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;

    /**
     * Returns TRUE if the parameter allows users to enter names for
     * a new (non-existing) tables.
     *
     * \see setAllowNewTableNames()
     */
    bool allowNewTableNames() const;

    /**
     * Sets whether the parameter allows users to enter names for
     * a new (non-existing) tables.
     *
     * \see allowNewTableNames()
     */
    void setAllowNewTableNames( bool allowed );

  private:

    QString mParentConnectionParameterName;
    QString mParentSchemaParameterName;
    bool mAllowNewTableNames = false;
};


/**
 * \class QgsProcessingParameterPointCloudLayer
 * \ingroup core
 * \brief A point cloud layer parameter for processing algorithms.
  * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterPointCloudLayer : public QgsProcessingParameterDefinition, public QgsFileFilterGenerator
{
  public:

    /**
     * Constructor for QgsProcessingParameterPointCloudLayer.
     */
    QgsProcessingParameterPointCloudLayer( const QString &name, const QString &description = QString(),
                                           const QVariant &defaultValue = QVariant(), bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "pointcloud" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QString createFileFilter() const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterPointCloudLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
};


/**
 * \class QgsProcessingParameterAnnotationLayer
 * \ingroup core
 * \brief An annotation layer parameter for processing algorithms.
  * \since QGIS 3.22
 */
class CORE_EXPORT QgsProcessingParameterAnnotationLayer : public QgsProcessingParameterDefinition
{
  public:

    /**
     * Constructor for QgsProcessingParameterAnnotationLayer.
     */
    QgsProcessingParameterAnnotationLayer( const QString &name, const QString &description = QString(),
                                           const QVariant &defaultValue = QVariant(), bool optional = false );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "annotation" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterAnnotationLayer *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
};

/**
 * \class QgsProcessingParameterPointCloudDestination
 * \ingroup core
 * \brief A point cloud layer destination parameter, for specifying the destination path for a point cloud layer
 * created by the algorithm.
  * \since QGIS 3.24
 */
class CORE_EXPORT QgsProcessingParameterPointCloudDestination : public QgsProcessingDestinationParameter
{
  public:

    /**
     * Constructor for QgsProcessingParameterPointCloudDestination.
     *
     * If \a createByDefault is FALSE and the parameter is \a optional, then this destination
     * output will not be created by default.
     */
    QgsProcessingParameterPointCloudDestination( const QString &name, const QString &description = QString(),
        const QVariant &defaultValue = QVariant(),
        bool optional = false,
        bool createByDefault = true );

    /**
     * Returns the type name for the parameter class.
     */
    static QString typeName() { return QStringLiteral( "pointCloudDestination" ); }
    QgsProcessingParameterDefinition *clone() const override SIP_FACTORY;
    QString type() const override { return typeName(); }
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;
    QgsProcessingOutputDefinition *toOutputDefinition() const override SIP_FACTORY;
    QString defaultFileExtension() const override;
    QString createFileFilter() const override;

    /**
     * Returns a list of the point cloud format file extensions supported for this parameter.
     * \see defaultFileExtension()
     */
    virtual QStringList supportedOutputPointCloudLayerExtensions() const;

    /**
     * Creates a new parameter using the definition from a script code.
     */
    static QgsProcessingParameterPointCloudDestination *fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) SIP_FACTORY;
};

// clazy:excludeall=qstring-allocations

#endif // QGSPROCESSINGPARAMETERS_H


