/***************************************************************************
                         qgsprocessingalgorithm.h
                         ------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSPROCESSINGALGORITHM_H
#define QGSPROCESSINGALGORITHM_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingcontext.h"
#include "qgsfeaturesource.h"
#include "qgsprocessingutils.h"
#include <QString>
#include <QVariant>
#include <QIcon>

class QgsProcessingProvider;
class QgsProcessingFeedback;
class QgsFeatureSink;
class QgsProcessingModelAlgorithm;
class QgsProcessingAlgorithmConfigurationWidget;
class QgsMeshLayer;
class QgsPointCloudLayer;

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsprocessingmodelalgorithm.h>
% End
#endif

/**
 * \class QgsProcessingAlgorithm
 * \ingroup core
 * \brief Abstract base class for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingAlgorithm
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsProcessingModelAlgorithm * >( sipCpp ) != NULL )
      sipType = sipType_QgsProcessingModelAlgorithm;
    else if ( dynamic_cast< QgsProcessingFeatureBasedAlgorithm * >( sipCpp ) != NULL )
      sipType = sipType_QgsProcessingFeatureBasedAlgorithm;
    else
      sipType = sipType_QgsProcessingAlgorithm;
    SIP_END
#endif

  public:

    //! Flags indicating how and when an algorithm operates and should be exposed to users
    enum Flag
    {
      FlagHideFromToolbox = 1 << 1, //!< Algorithm should be hidden from the toolbox
      FlagHideFromModeler = 1 << 2, //!< Algorithm should be hidden from the modeler
      FlagSupportsBatch = 1 << 3,  //!< Algorithm supports batch mode
      FlagCanCancel = 1 << 4, //!< Algorithm can be canceled
      FlagRequiresMatchingCrs = 1 << 5, //!< Algorithm requires that all input layers have matching coordinate reference systems
      FlagNoThreading = 1 << 6, //!< Algorithm is not thread safe and cannot be run in a background thread, e.g. for algorithms which manipulate the current project, layer selections, or with external dependencies which are not thread-safe.
      FlagDisplayNameIsLiteral = 1 << 7, //!< Algorithm's display name is a static literal string, and should not be translated or automatically formatted. For use with algorithms named after commands, e.g. GRASS 'v.in.ogr'.
      FlagSupportsInPlaceEdits = 1 << 8, //!< Algorithm supports in-place editing
      FlagKnownIssues = 1 << 9, //!< Algorithm has known issues
      FlagCustomException = 1 << 10, //!< Algorithm raises custom exception notices, don't use the standard ones
      FlagPruneModelBranchesBasedOnAlgorithmResults = 1 << 11, //!< Algorithm results will cause remaining model branches to be pruned based on the results of running the algorithm
      FlagSkipGenericModelLogging = 1 << 12, //!< When running as part of a model, the generic algorithm setup and results logging should be skipped
      FlagNotAvailableInStandaloneTool = 1 << 13, //!< Algorithm should not be available from the standalone "qgis_process" tool. Used to flag algorithms which make no sense outside of the QGIS application, such as "select by..." style algorithms.
      FlagRequiresProject = 1 << 14, //!< The algorithm requires that a valid QgsProject is available from the processing context in order to execute
      FlagDeprecated = FlagHideFromToolbox | FlagHideFromModeler, //!< Algorithm is deprecated
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingAlgorithm.
     *
     * initAlgorithm() should be called after creating an algorithm to ensure it can correctly configure
     * its parameterDefinitions() and outputDefinitions(). Alternatively, calling create() will return
     * a pre-initialized copy of the algorithm.
     */
    QgsProcessingAlgorithm() = default;

    virtual ~QgsProcessingAlgorithm();

    //! Algorithms cannot be copied - create() should be used instead
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other ) = delete;
    //! Algorithms cannot be copied- create() should be used instead
    QgsProcessingAlgorithm &operator=( const QgsProcessingAlgorithm &other ) = delete;

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotation here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a copy of the algorithm, ready for execution.
     *
     * This method returns a new, preinitialized copy of the algorithm, ready for
     * executing.
     *
     * The \a configuration argument allows passing of a map of configuration settings
     * to the algorithm, allowing it to dynamically adjust its initialized parameters
     * and outputs according to this configuration. This is generally used only for
     * algorithms in a model, allowing them to adjust their behavior at run time
     * according to some user configuration.
     *
     * Raises a QgsProcessingException if a new algorithm instance could not be created,
     * e.g. if there is an issue with the subclass' createInstance() method.
     *
     * \see initAlgorithm()
     */
    QgsProcessingAlgorithm *create( const QVariantMap &configuration = QVariantMap() ) const SIP_THROW( QgsProcessingException ) SIP_TRANSFERBACK;

    /**
     * Returns the algorithm name, used for identifying the algorithm. This string
     * should be fixed for the algorithm, and must not be localised. The name should
     * be unique within each provider. Names should contain lowercase alphanumeric characters
     * only and no spaces or other formatting characters.
     * \see displayName()
     * \see group()
     * \see tags()
    */
    virtual QString name() const = 0;

    /**
     * Returns the unique ID for the algorithm, which is a combination of the algorithm
     * provider's ID and the algorithms unique name (e.g. "qgis:mergelayers" ).
     * \see name()
     * \see provider()
     */
    QString id() const;

    /**
     * Returns the translated algorithm name, which should be used for any user-visible display
     * of the algorithm name.
     *
     * Algorithm display names should be short, e.g. ideally no more than 3 or 4 words.
     * The name should use sentence case (e.g. "Raster layer statistics", not "Raster Layer Statistics").
     *
     * \see name()
     * \see shortDescription()
     */
    virtual QString displayName() const = 0;

    /**
     * Returns an optional translated short description of the algorithm. This should be
     * at most a single sentence, e.g. "Converts 2D features to 3D by sampling a DEM raster."
     * \since QGIS 3.2
     */
    virtual QString shortDescription() const;

    /**
     * Returns a list of tags which relate to the algorithm, and are used to assist users in searching
     * for suitable algorithms. These tags should be localised.
    */
    virtual QStringList tags() const { return QStringList(); }

    /**
     * Returns a localised short helper string for the algorithm. This string should provide a basic description
     * about what the algorithm does and the parameters and outputs associated with it.
     * \see helpString()
     * \see helpUrl()
     */
    virtual QString shortHelpString() const;

    /**
     * Returns a localised help string for the algorithm. Algorithm subclasses should implement either
     * helpString() or helpUrl().
     * \see helpUrl()
     * \see shortHelpString()
     * \deprecated Unused, will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED virtual QString helpString() const SIP_DEPRECATED;

    /**
     * Returns a url pointing to the algorithm's help page.
     * \see helpString()
     * \see shortHelpString()
     */
    virtual QString helpUrl() const;

    /**
     * Returns an icon for the algorithm.
     * \see svgIconPath()
    */
    virtual QIcon icon() const;

    /**
     * Returns a path to an SVG version of the algorithm's icon.
     * \see icon()
     */
    virtual QString svgIconPath() const;

    /**
     * Returns the name of the group this algorithm belongs to. This string
     * should be localised.
     * \see groupId()
     * \see tags()
    */
    virtual QString group() const { return QString(); }

    /**
     * Returns the unique ID of the group this algorithm belongs to. This string
     * should be fixed for the algorithm, and must not be localised. The group id
     * should be unique within each provider. Group id should contain lowercase
     * alphanumeric characters only and no spaces or other formatting characters.
     * \see group()
     */
    virtual QString groupId() const { return QString(); }

    /**
     * Returns the flags indicating how and when the algorithm operates and should be exposed to users.
     * Default flags are FlagSupportsBatch and FlagCanCancel.
     */
    virtual Flags flags() const;

    /**
     * Returns TRUE if the algorithm can execute. Algorithm subclasses can return FALSE
     * here to indicate that they are not able to execute, e.g. as a result of unmet
     * external dependencies. If specified, the \a errorMessage argument will be filled
     * with a localised error message describing why the algorithm cannot execute.
     */
    virtual bool canExecute( QString *errorMessage SIP_OUT = nullptr ) const;

    /**
     * Checks the supplied \a parameter values to verify that they satisfy the requirements
     * of this algorithm in the supplied \a context. The \a message parameter will be
     * filled with explanatory text if validation fails.
     * Overridden implementations should also check this base class implementation.
     * \returns TRUE if parameters are acceptable for the algorithm.
     */
    virtual bool checkParameterValues( const QVariantMap &parameters,
                                       QgsProcessingContext &context, QString *message SIP_OUT = nullptr ) const;

    /**
     * Pre-processes a set of \a parameters, allowing the algorithm to clean their
     * values.
     *
     * This method is automatically called after users enter parameters, e.g. via the algorithm
     * dialog. This method should NOT be called manually by algorithms.
     */
    virtual QVariantMap preprocessParameters( const QVariantMap &parameters );

    /**
     * Returns the provider to which this algorithm belongs.
     */
    QgsProcessingProvider *provider() const;

    /**
     * Returns an ordered list of parameter definitions utilized by the algorithm.
     * \see addParameter()
     * \see parameterDefinition()
     * \see destinationParameterDefinitions()
     */
    QgsProcessingParameterDefinitions parameterDefinitions() const { return mParameters; }

    /**
     * Returns a matching parameter by \a name. Matching is done in a case-insensitive
     * manner, but exact case matches will be preferred.
     * \see parameterDefinitions()
     */
    const QgsProcessingParameterDefinition *parameterDefinition( const QString &name ) const;

    /**
     * Returns the number of visible (non-hidden) parameters defined by this
     * algorithm.
     */
    int countVisibleParameters() const;

    /**
     * Returns a list of destination parameters definitions utilized by the algorithm.
     * \see QgsProcessingParameterDefinition::isDestination()
     * \see parameterDefinitions()
     */
    QgsProcessingParameterDefinitions destinationParameterDefinitions() const;

    /**
     * Returns an ordered list of output definitions utilized by the algorithm.
     * \see addOutput()
     * \see outputDefinition()
     */
    QgsProcessingOutputDefinitions outputDefinitions() const { return mOutputs; }

    /**
     * Returns a matching output by \a name. Matching is done in a case-insensitive
     * manner.
     * \see outputDefinitions()
     */
    const QgsProcessingOutputDefinition *outputDefinition( const QString &name ) const;

    /**
     * Returns TRUE if this algorithm generates HTML outputs.
     */
    bool hasHtmlOutputs() const;

    /**
     * Property availability, used for QgsProcessingAlgorithm::VectorProperties
     * in order to determine if properties are available or not
     */
    enum PropertyAvailability
    {
      NotAvailable, //!< Properties are not available
      Available, //!< Properties are available
    };

    /**
     * Properties of a vector source or sink used in an algorithm.
     *
     * \since QGIS 3.14
     */
    struct VectorProperties
    {
      //! Fields
      QgsFields fields;

      //! Geometry (WKB) type
      QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown;

      //! Coordinate Reference System
      QgsCoordinateReferenceSystem crs;

      //! Availability of the properties. By default properties are not available.
      QgsProcessingAlgorithm::PropertyAvailability availability = QgsProcessingAlgorithm::NotAvailable;
    };

    /**
     * Returns the vector properties which will be used for the \a sink with matching name.
     *
     * The \a parameters argument specifies the values of all parameters which would be used to generate
     * the sink. These can be used alongside the provided \a context in order to pre-evaluate inputs
     * when required in order to determine the sink's properties.
     *
     * The \a sourceProperties map will contain the vector properties of the various sources used
     * as inputs to the algorithm. These will only be available in certain circumstances (e.g. when the
     * algorithm is used within a model), so implementations will need to be adaptable to circumstances
     * when either \a sourceParameters is empty or \a parameters is empty, and use whatever information
     * is passed in order to make a best guess determination of the output properties.
     *
     * \since QGIS 3.14
     */
    virtual QgsProcessingAlgorithm::VectorProperties sinkProperties( const QString &sink,
        const QVariantMap &parameters,
        QgsProcessingContext &context,
        const QMap< QString, QgsProcessingAlgorithm::VectorProperties > &sourceProperties ) const;

    /**
     * Executes the algorithm using the specified \a parameters. This method internally
     * creates a copy of the algorithm before running it, so it is safe to call
     * on algorithms directly retrieved from QgsProcessingRegistry and QgsProcessingProvider.
     *
     * The \a context argument specifies the context in which the algorithm is being run.
     *
     * Algorithm progress should be reported using the supplied \a feedback object.
     *
     * If specified, \a ok will be set to TRUE if algorithm was successfully run.
     *
     * If \a catchExceptions is set to FALSE, then QgsProcessingException raised during
     * the algorithm run will not be automatically caught and will be raised instead.
     *
     * \returns A map of algorithm outputs. These may be output layer references, or calculated
     * values such as statistical calculations.
     *
     * \note this method can only be called from the main thread. Use prepare(), runPrepared() and postProcess()
     * if you need to run algorithms from a background thread, or use the QgsProcessingAlgRunnerTask class.
     */
    QVariantMap run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, bool *ok SIP_OUT = nullptr, const QVariantMap &configuration = QVariantMap(),
                     bool catchExceptions = true ) const SIP_THROW( QgsProcessingException );

    /**
     * Prepares the algorithm for execution. This must be run in the main thread, and allows the algorithm
     * to pre-evaluate input parameters in a thread-safe manner. This must be called before
     * calling runPrepared() (which is safe to do in any thread).
     * \see runPrepared()
     * \see postProcess()
     * \note This method modifies the algorithm instance, so it is not safe to call
     * on algorithms directly retrieved from QgsProcessingRegistry and QgsProcessingProvider. Instead, a copy
     * of the algorithm should be created with clone() and prepare()/runPrepared() called on the copy.
     */
    bool prepare( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    /**
     * Runs the algorithm, which has been prepared by an earlier call to prepare().
     * This method is safe to call from any thread. Returns TRUE if the algorithm was successfully executed.
     * After runPrepared() has finished, the postProcess() method should be called from the main thread
     * to allow the algorithm to perform any required cleanup tasks and return its final result.
     * \see prepare()
     * \see postProcess()
     * \note This method modifies the algorithm instance, so it is not safe to call
     * on algorithms directly retrieved from QgsProcessingRegistry and QgsProcessingProvider. Instead, a copy
     * of the algorithm should be created with clone() and prepare()/runPrepared() called on the copy.
     */
    QVariantMap runPrepared( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) SIP_THROW( QgsProcessingException );

    /**
     * Should be called in the main thread following the completion of runPrepared(). This method
     * allows the algorithm to perform any required cleanup tasks. The returned variant map
     * includes the results evaluated by the algorithm.
     * \note This method modifies the algorithm instance, so it is not safe to call
     * on algorithms directly retrieved from QgsProcessingRegistry and QgsProcessingProvider. Instead, a copy
     * of the algorithm should be created with clone() and prepare()/runPrepared() called on the copy.
     */
    QVariantMap postProcess( QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    /**
     * If an algorithm subclass implements a custom parameters widget, a copy of this widget
     * should be constructed and returned by this method.
     * The base class implementation returns NULLPTR, which indicates that an autogenerated
     * parameters widget should be used.
     */
    virtual QWidget *createCustomParametersWidget( QWidget *parent = nullptr ) const SIP_FACTORY;

    /**
     * Creates an expression context relating to the algorithm. This can be called by algorithms
     * to create a new expression context ready for evaluating expressions within the algorithm.
     * Optionally, a \a source can be specified which will be used to populate the context if it
     * implements the QgsExpressionContextGenerator interface.
     */
    virtual QgsExpressionContext createExpressionContext( const QVariantMap &parameters,
        QgsProcessingContext &context, QgsProcessingFeatureSource *source = nullptr ) const;

    /**
     * Checks whether the coordinate reference systems for the specified set of \a parameters
     * are valid for the algorithm. For instance, the base implementation performs
     * checks to ensure that all input CRS are equal
     * Returns TRUE if \a parameters have passed the CRS check.
     */
    virtual bool validateInputCrs( const QVariantMap &parameters,
                                   QgsProcessingContext &context ) const;

    /**
     * Returns a Python command string which can be executed to run the algorithm
     * using the specified \a parameters.
     *
     * Algorithms which cannot be run from a Python command should return an empty
     * string.
     */
    virtual QString asPythonCommand( const QVariantMap &parameters, QgsProcessingContext &context ) const;

    /**
     * Returns a command string which will execute the algorithm using the specified \a parameters
     * via the command line qgis_process tool.
     *
     * Note that some combinations of parameter types and values cannot be represented as a qgis_process string.
     *
     * \param parameters algorithm parameters
     * \param context processing context
     * \param ok will be set to TRUE if the command was successfully generated
     *
     * \returns equivalent qgis_process command
     *
     * \since QGIS 3.24
     */
    virtual QString asQgisProcessCommand( const QVariantMap &parameters, QgsProcessingContext &context, bool &ok SIP_OUT ) const;

    /**
     * Returns a JSON serializable variant map containing the specified \a parameters and \a context settings.
     *
     * \since QGIS 3.24
     */
    virtual QVariantMap asMap( const QVariantMap &parameters, QgsProcessingContext &context ) const;

    /**
     * Associates this algorithm with its provider. No transfer of ownership is involved.
     */
    void setProvider( QgsProcessingProvider *provider );

  protected:

    /**
     * Creates a new instance of the algorithm class.
     *
     * This method should return a 'pristine' instance of the algorithm class.
     */
    virtual QgsProcessingAlgorithm *createInstance() const = 0 SIP_FACTORY SIP_VIRTUALERRORHANDLER( processing_exception_handler );

    /**
     * Initializes the algorithm using the specified \a configuration.
     *
     * This should be called directly after creating algorithms and before retrieving
     * any parameterDefinitions() or outputDefinitions().
     *
     * Subclasses should use their implementations to add all required input parameter and output
     * definitions (which can be dynamically adjusted according to \a configuration).
     *
     * Dynamic configuration can be used by algorithms which alter their behavior
     * when used inside processing models. For instance, a "feature router" type
     * algorithm which sends input features to one of any number of outputs sinks
     * based on some preconfigured filter parameters can use the init method to
     * create these outputs based on the specified \a configuration.
     *
     * \see addParameter()
     * \see addOutput()
     */
    virtual void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) = 0;

    /**
     * Adds a parameter \a definition to the algorithm. Ownership of the definition is transferred to the algorithm.
     * Returns TRUE if parameter could be successfully added, or FALSE if the parameter could not be added (e.g.
     * as a result of a duplicate name).
     *
     * This should usually be called from a subclass' initAlgorithm() implementation.
     *
     * If the \a createOutput argument is TRUE, then a corresponding output definition will also be created
     * (and added to the algorithm) where appropriate. E.g. when adding a QgsProcessingParameterVectorDestination
     * and \a createOutput is TRUE, then a QgsProcessingOutputVectorLayer output will be created and
     * added to the algorithm. There is no need to call addOutput() to manually add a corresponding output
     * for this vector. If \a createOutput is FALSE then this automatic output creation will not
     * occur.
     *
     * \see initAlgorithm()
     * \see addOutput()
     */
    bool addParameter( QgsProcessingParameterDefinition *parameterDefinition SIP_TRANSFER, bool createOutput = true );

    /**
     * Removes the parameter with matching \a name from the algorithm, and deletes any existing
     * definition.
     */
    void removeParameter( const QString &name );

    /**
     * Adds an output \a definition to the algorithm. Ownership of the definition is transferred to the algorithm.
     * Returns TRUE if the output could be successfully added, or FALSE if the output could not be added (e.g.
     * as a result of a duplicate name).
     *
     * This should usually be called from a subclass' initAlgorithm() implementation.
     *
     * Note that in some cases output creation can be automatically performed when calling addParameter().
     * See the notes in addParameter() for a description of when this occurs.
     *
     * \see addParameter()
     * \see initAlgorithm()
     */
    bool addOutput( QgsProcessingOutputDefinition *outputDefinition SIP_TRANSFER );

    /**
     * Prepares the algorithm to run using the specified \a parameters. Algorithms should implement
     * their logic for evaluating parameter values here. The evaluated parameter results should
     * be stored in member variables ready for a call to processAlgorithm().
     *
     * The \a context argument specifies the context in which the algorithm is being run.
     *
     * prepareAlgorithm should be used to handle any thread-sensitive preparation which is required
     * by the algorithm. It will always be called from the same thread that \a context has thread
     * affinity with. While this will generally be the main thread, it is not guaranteed. For instance,
     * algorithms which are run as a step in a larger model or as a subcomponent of a script-based algorithm
     * will call prepareAlgorithm from the same thread as that model/script it being executed in.
     *
     * Note that the processAlgorithm step uses a temporary context with affinity for the thread in
     * which the algorithm is executed, making it safe for processAlgorithm implementations to load
     * sources and sinks without issue. Implementing prepareAlgorithm is only required if special
     * thread safe handling is required by the algorithm.
     *
     * Algorithm preparation progress should be reported using the supplied \a feedback object. Additionally,
     * well-behaved algorithms should periodically check \a feedback to determine whether the
     * algorithm should be canceled and exited early.
     *
     * If the preparation was successful algorithms must return TRUE. If a FALSE value is returned
     * this indicates that the preparation could not be completed, and the algorithm execution
     * will be canceled.
     *
     * \returns TRUE if preparation was successful.
     * \see processAlgorithm()
     * \see postProcessAlgorithm()
     */
    virtual bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) SIP_THROW( QgsProcessingException ) SIP_VIRTUALERRORHANDLER( processing_exception_handler );

    /**
     * Runs the algorithm using the specified \a parameters. Algorithms should implement
     * their custom processing logic here.
     *
     * The \a context argument gives a temporary context with thread affinity matching the thread
     * in which the algorithm is being run. This is a cut-back copy of the context passed to
     * the prepareAlgorithm() and postProcessAlgorithm() steps, but it is generally safe
     * for most algorithms to utilize this context for loading layers and creating sinks.
     * Any loaded layers or sinks created within this temporary context will be transferred
     * back to the main execution context upon successful completion of the processAlgorithm()
     * step.
     *
     * Algorithm progress should be reported using the supplied \a feedback object. Additionally,
     * well-behaved algorithms should periodically check \a feedback to determine whether the
     * algorithm should be canceled and exited early.
     *
     * This method will not be called if the prepareAlgorithm() step failed (returned FALSE).
     *
     * Implementations of processAlgorithm can throw the QgsProcessingException exception
     * to indicate that a fatal error occurred within the execution.
     *
     * \returns A map of algorithm outputs. These may be output layer references, or calculated
     * values such as statistical calculations. Unless the algorithm subclass overrides
     * the postProcessAlgorithm() step this returned map will be used as the output for the
     * algorithm.
     *
     * \see prepareAlgorithm()
     * \see postProcessAlgorithm()
     */
    virtual QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) SIP_THROW( QgsProcessingException ) = 0 SIP_VIRTUALERRORHANDLER( processing_exception_handler );

    /**
     * Allows the algorithm to perform any required cleanup tasks. The returned variant map
     * includes the results evaluated by the algorithm. These may be output layer references, or calculated
     * values such as statistical calculations.
     *
     * The \a context argument specifies the context in which the algorithm was run.
     *
     * Postprocess progress should be reported using the supplied \a feedback object. Additionally,
     * well-behaved algorithms should periodically check \a feedback to determine whether the
     * post processing should be canceled and exited early.
     *
     * postProcessAlgorithm should be used to handle any thread-sensitive cleanup which is required
     * by the algorithm. It will always be called from the same thread that \a context has thread
     * affinity with. While this will generally be the main thread, it is not guaranteed. For instance,
     * algorithms which are run as a step in a larger model or as a subcomponent of a script-based algorithm
     * will call postProcessAlgorithm from the same thread as that model/script it being executed in.
     *
     * postProcessAlgorithm will not be called if the prepareAlgorithm() step failed (returned FALSE),
     * or if an exception was raised by the processAlgorithm() step.
     *
     * \returns A map of algorithm outputs. These may be output layer references, or calculated
     * values such as statistical calculations. Implementations which return a non-empty
     * map will override any results returned by processAlgorithm().
     *
     * \see prepareAlgorithm()
     * \see processAlgorithm()
     */
    virtual QVariantMap postProcessAlgorithm( QgsProcessingContext &context, QgsProcessingFeedback *feedback ) SIP_THROW( QgsProcessingException ) SIP_VIRTUALERRORHANDLER( processing_exception_handler );

    /**
     * Evaluates the parameter with matching \a name to a static string value.
     */
    QString parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to an expression.
     */
    QString parameterAsExpression( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a static double value.
     */
    double parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a static integer value.
     */
    int parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a list of integer values.
     * \since QGIS 3.4
     */
    QList<int> parameterAsInts( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a enum value.
     */
    int parameterAsEnum( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to list of enum values.
     */
    QList<int> parameterAsEnums( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a static enum string.
     * \since QGIS 3.18
     */
    QString parameterAsEnumString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to list of static enum strings.
     * \since QGIS 3.18
     */
    QStringList parameterAsEnumStrings( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a static boolean value.
     */
    bool parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a static boolean value.
     * \since QGIS 3.8
     */
    bool parameterAsBoolean( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a feature sink.
     *
     * Sinks will either be taken from \a context's active project, or created from external
     * providers and stored temporarily in the \a context.
     *
     * The \a fields, \a geometryType and \a crs parameters dictate the properties
     * of the resulting feature sink.
     *
     * The \a destinationIdentifier argument will be set to a string which can be used to retrieve the layer corresponding
     * to the sink, e.g. via calling QgsProcessingUtils::mapLayerFromString().
     *
     * The \a createOptions argument is used to pass on creation options such as layer name.
     *
     * The \a datasourceOptions and \a layerOptions arguments is used to pass on GDAL-specific format driver options.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     *
     * \throws QgsProcessingException
     */
    QgsFeatureSink *parameterAsSink( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, QString &destinationIdentifier SIP_OUT,
                                     const QgsFields &fields, QgsWkbTypes::Type geometryType = QgsWkbTypes::NoGeometry, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(), QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags(), const QVariantMap &createOptions = QVariantMap(), const QStringList &datasourceOptions = QStringList(), const QStringList &layerOptions = QStringList() ) const SIP_THROW( QgsProcessingException ) SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a name to a feature source.
     *
     * Sources will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context.
     *
     * This function creates a new object and the caller takes responsibility for deleting the returned object.
     */
    QgsProcessingFeatureSource *parameterAsSource( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const SIP_FACTORY;

    /**
     * Evaluates the parameter with matching \a name to a source vector layer file path of compatible format.
     *
     * If the parameter is evaluated to an existing layer, and that layer is not of the format listed in the
     * \a compatibleFormats argument, then the layer will first be exported to a compatible format
     * in a temporary location. The function will then return the path to that temporary file.
     *
     * \a compatibleFormats should consist entirely of lowercase file extensions, e.g. 'shp'.
     *
     * The \a preferredFormat argument is used to specify to desired file extension to use when a temporary
     * layer export is required.
     *
     * When an algorithm is capable of handling multi-layer input files (such as Geopackage), it is preferable
     * to use parameterAsCompatibleSourceLayerPathAndLayerName() which may avoid conversion in more situations.
     */
    QString parameterAsCompatibleSourceLayerPath( const QVariantMap &parameters, const QString &name,
        QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat = QString( "shp" ), QgsProcessingFeedback *feedback = nullptr ) const;

    /**
     * Evaluates the parameter with matching \a name to a source vector layer file path and layer name of compatible format.
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
     * \param parameters input parameter value map
     * \param name name of target parameter
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
    QString parameterAsCompatibleSourceLayerPathAndLayerName( const QVariantMap &parameters, const QString &name,
        QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat = QString( "shp" ), QgsProcessingFeedback *feedback = nullptr, QString *layerName SIP_OUT = nullptr ) const;

    /**
     * Evaluates the parameter with matching \a name to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    QgsMapLayer *parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a raster layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    QgsRasterLayer *parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a mesh layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.6
     */
    QgsMeshLayer *parameterAsMeshLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a output layer destination.
     */
    QString parameterAsOutputLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a file based output destination.
     */
    QString parameterAsFileOutput( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a vector layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    QgsVectorLayer *parameterAsVectorLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a coordinate reference system.
     */
    QgsCoordinateReferenceSystem parameterAsCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a rectangular extent.
     *
     * If \a crs is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
     * reprojected so that it is in the specified \a crs. In this case the extent of the reproject rectangle will be returned.
     *
     * \see parameterAsExtentGeometry()
     */
    QgsRectangle parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context,
                                    const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) const;

    /**
     * Evaluates the parameter with matching \a name to a rectangular extent, and returns a geometry covering this extent.
     *
     * If \a crs is set, and the original coordinate reference system of the parameter can be determined, then the extent will be automatically
     * reprojected so that it is in the specified \a crs. Unlike parameterAsExtent(), the reprojected rectangle returned by this function
     * will no longer be a rectangle itself (i.e. this method returns the geometry of the actual reprojected rectangle, while parameterAsExtent() returns
     * just the extent of the reprojected rectangle).
     *
     * \see parameterAsExtent()
     */
    QgsGeometry parameterAsExtentGeometry( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context,
                                           const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) const;

    /**
     * Returns the coordinate reference system associated with an extent parameter value.
     *
     * \see parameterAsExtent()
     */
    QgsCoordinateReferenceSystem parameterAsExtentCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a point.
     *
     * If \a crs is set then the point will be automatically
     * reprojected so that it is in the specified \a crs.
     *
     * \see parameterAsPointCrs()
     */
    QgsPointXY parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context,
                                 const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) const;

    /**
     * Returns the coordinate reference system associated with an point parameter value.
     *
     * \see parameterAsPoint()
     */
    QgsCoordinateReferenceSystem parameterAsPointCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a geometry.
     *
     * If \a crs is set then the geometry will be automatically
     * reprojected so that it is in the specified \a crs.
     *
     * \see parameterAsGeometryCrs()
     */
    QgsGeometry parameterAsGeometry( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context,
                                     const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) const;

    /**
     * Returns the coordinate reference system associated with a geometry parameter value.
     *
     * \see parameterAsGeometry()
     */
    QgsCoordinateReferenceSystem parameterAsGeometryCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a file/folder name.
     */
    QString parameterAsFile( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a matrix/table of values.
     * Tables are collapsed to a 1 dimensional list.
     */
    QVariantList parameterAsMatrix( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a list of map layers.
     */
    QList< QgsMapLayer *> parameterAsLayerList( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a list of files (for QgsProcessingParameterMultipleLayers in QgsProcessing:TypeFile mode).
     *
     * \since QGIS 3.10
     */
    QStringList parameterAsFileList( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a range of values.
     */
    QList<double> parameterAsRange( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a list of fields.
     */
    QStringList parameterAsFields( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a print layout.
     *
     * \warning This method is not safe to run in a background thread, so it must either be used within a prepareAlgorithm
     * implementation (which runs in the main thread), or the algorithm must return the FlagNoThreading flag.
     *
     * \since QGIS 3.8
     */
    QgsPrintLayout *parameterAsLayout( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a name to a print layout item, taken from the specified \a layout.
     *
     * \warning This method is not safe to run in a background thread, so it must either be used within a prepareAlgorithm
     * implementation (which runs in the main thread), or the algorithm must return the FlagNoThreading flag.
     *
     * \since QGIS 3.8
     */
    QgsLayoutItem *parameterAsLayoutItem( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, QgsPrintLayout *layout );

    /**
     * Evaluates the parameter with matching \a name to a color, or returns an invalid color if the parameter was not set.
     *
     * \since QGIS 3.10
     */
    QColor parameterAsColor( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a connection name string.
     *
     * \since QGIS 3.14
     */
    QString parameterAsConnectionName( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a database schema name string.
     *
     * \since QGIS 3.14
     */
    QString parameterAsSchema( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a database table name string.
     *
     * \since QGIS 3.14
     */
    QString parameterAsDatabaseTableName( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a DateTime, or returns an invalid date time if the parameter was not set.
     *
     * \since QGIS 3.14
     */
    QDateTime parameterAsDateTime( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a point cloud layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     *
     * \since QGIS 3.22
     */
    QgsPointCloudLayer *parameterAsPointCloudLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to an annotation layer.
     *
     * Annotation layers will be taken from \a context's active project. Callers do not
     * need to handle deletion of the returned layer.
     *
     * \warning Working with annotation layers is generally not thread safe (unless the layers are from
     * a QgsProject loaded directly in a background thread). Ensure your algorithm returns the
     * QgsProcessingAlgorithm::FlagNoThreading flag or only accesses annotation layers from a prepareAlgorithm()
     * or postProcessAlgorithm() step.
     *
     * \since QGIS 3.22
     */
    QgsAnnotationLayer *parameterAsAnnotationLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Returns a user-friendly string to use as an error when a source parameter could
     * not be loaded.
     *
     * The \a parameters argument should give the algorithms parameter map, and the \a name
     * should correspond to the invalid source parameter name.
     *
     *
     * \see invalidRasterError()
     * \see invalidSinkError()
     * \since QGIS 3.2
     */
    static QString invalidSourceError( const QVariantMap &parameters, const QString &name );

    /**
     * Returns a user-friendly string to use as an error when a raster layer input could
     * not be loaded.
     *
     * The \a parameters argument should give the algorithms parameter map, and the \a name
     * should correspond to the invalid source parameter name.
     *
     *
     * \see invalidSourceError()
     * \see invalidSinkError()
     * \since QGIS 3.2
     */
    static QString invalidRasterError( const QVariantMap &parameters, const QString &name );

    /**
     * Returns a user-friendly string to use as an error when a sink parameter could
     * not be created.
     *
     * The \a parameters argument should give the algorithms parameter map, and the \a name
     * should correspond to the invalid source parameter name.
     *
     *
     * \see invalidSourceError()
     * \see invalidRasterError()
     * \since QGIS 3.2
     */
    static QString invalidSinkError( const QVariantMap &parameters, const QString &name );

    /**
     * Returns a user-friendly string to use as an error when a feature cannot be
     * written into a sink.
     *
     * The \a sink argument is the sink into which the feature cannot be written.
     *
     * The \a parameters argument should give the algorithms parameter map, and the \a name
     * should correspond to the sink parameter name.
     *
     * \since QGIS 3.22
     */
    static QString writeFeatureError( QgsFeatureSink *sink, const QVariantMap &parameters, const QString &name );

    /**
     * Checks whether this algorithm supports in-place editing on the given \a layer
     * Default implementation returns FALSE.
     *
     * \return TRUE if the algorithm supports in-place editing
     * \since QGIS 3.4
     */
    virtual bool supportInPlaceEdit( const QgsMapLayer *layer ) const;

  private:

    QgsProcessingProvider *mProvider = nullptr;
    QgsProcessingParameterDefinitions mParameters;
    QgsProcessingOutputDefinitions mOutputs;
    bool mHasPrepared = false;
    bool mHasExecuted = false;
    bool mHasPostProcessed = false;
    std::unique_ptr< QgsProcessingContext > mLocalContext;

    bool createAutoOutputForParameter( QgsProcessingParameterDefinition *parameter );


    friend class QgsProcessingProvider;
    friend class TestQgsProcessing;
    friend class QgsProcessingModelAlgorithm;
    friend class QgsProcessingToolboxProxyModel;

#ifdef SIP_RUN
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other );
#endif

};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingAlgorithm::Flags )



/**
 * \class QgsProcessingFeatureBasedAlgorithm
 * \ingroup core
 * \brief An abstract QgsProcessingAlgorithm base class for processing algorithms which operate "feature-by-feature".
 *
 * Feature based algorithms are algorithms which operate on individual features in isolation. These
 * are algorithms where one feature is output for each input feature, and the output feature result
 * for each input feature is not dependent on any other features present in the source.
 *
 * For instance, algorithms like "centroids" and "buffers" are feature based algorithms since the centroid
 * or buffer of a feature is calculated for each feature in isolation. An algorithm like "dissolve"
 * is NOT suitable for a feature based algorithm as the dissolved output depends on multiple input features
 * and these features cannot be processed in isolation.
 *
 * Using QgsProcessingFeatureBasedAlgorithm as the base class for feature based algorithms allows
 * shortcutting much of the common algorithm code for handling iterating over sources and pushing
 * features to output sinks. It also allows the algorithm execution to be optimised in future
 * (for instance allowing automatic multi-thread processing of the algorithm, or use of the
 * algorithm in "chains", avoiding the need for temporary outputs in multi-step models).
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingFeatureBasedAlgorithm : public QgsProcessingAlgorithm
{
  public:

    /**
      * Constructor for QgsProcessingFeatureBasedAlgorithm.
      */
    QgsProcessingFeatureBasedAlgorithm() = default;

    QgsProcessingAlgorithm::Flags flags() const override;

    /**
     * Processes an individual input \a feature from the source. Algorithms should implement their
     * logic in this method for performing the algorithm's operation (e.g. replacing the feature's
     * geometry with the centroid of the original feature geometry for a 'centroid' type
     * algorithm).
     *
     * Implementations should return a list containing the modified feature. Returning an empty an list
     * will indicate that this feature should be 'skipped', and will not be added to the algorithm's output.
     * Subclasses can use this approach to filter the incoming features as desired.
     *
     * Additionally, multiple features can be returned for a single input feature. Each returned feature
     * will be added to the algorithm's output. This allows for "explode" type algorithms where a single
     * input feature results in multiple output features.
     *
     * The provided \a feedback object can be used to push messages to the log and for giving feedback
     * to users. Note that handling of progress reports and algorithm cancellation is handled by
     * the base class and subclasses do not need to reimplement this logic.
     *
     * Algorithms can throw a QgsProcessingException if a fatal error occurred which should
     * prevent the algorithm execution from continuing. This can be annoying for users though as it
     * can break valid model execution - so use with extreme caution, and consider using
     * \a feedback to instead report non-fatal processing failures for features instead.
     */
    virtual QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) SIP_THROW( QgsProcessingException ) = 0 SIP_VIRTUALERRORHANDLER( processing_exception_handler );

  protected:

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

    /**
     * Returns the name of the parameter corresponding to the input layer.
     *
     * By default this is the standard "INPUT" parameter name.
     *
     * \since QGIS 3.12
     */
    virtual QString inputParameterName() const;

    /**
     * Returns the translated description of the parameter corresponding to the input layer.
     *
     * By default this is a translated "Input layer" string.
     *
     * \since QGIS 3.12
     */
    virtual QString inputParameterDescription() const;

    /**
     * Returns the translated, user visible name for any layers created by this algorithm.
     * This name will be used as the default name when loading the resultant layer into a
     * QGIS project.
     */
    virtual QString outputName() const = 0;

    /**
     * Returns the valid input layer types for the source layer for this algorithm.
     * By default vector layers with any geometry types (excluding non-spatial, geometryless layers)
     * are accepted.
     */
    virtual QList<int> inputLayerTypes() const;

    /**
     * Returns the layer type for layers generated by this algorithm, if
     * this is possible to determine in advance.
     */
    virtual QgsProcessing::SourceType outputLayerType() const;

    /**
     * Returns the processing feature source flags to be used in the algorithm.
     */
    virtual QgsProcessingFeatureSource::Flag sourceFlags() const;

    /**
     * Returns the feature sink flags to be used for the output.
     *
     * \since QGIS 3.4.1
     */
    virtual QgsFeatureSink::SinkFlags sinkFlags() const;

    /**
     * Maps the input WKB geometry type (\a inputWkbType) to the corresponding
     * output WKB type generated by the algorithm. The default behavior is that the algorithm maintains
     * the same WKB type.
     * This is called once by the base class when creating the output sink for the algorithm (i.e. it is
     * not called once per feature processed).
     */
    virtual QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const;

    /**
     * Maps the input source fields (\a inputFields) to corresponding
     * output fields generated by the algorithm. The default behavior is that the algorithm maintains
     * the same fields as are input.
     * Algorithms which add, remove or modify existing fields should override this method and
     * implement logic here to indicate which fields are output by the algorithm.
     *
     * This is called once by the base class when creating the output sink for the algorithm (i.e. it is
     * not called once per feature processed).
     */
    virtual QgsFields outputFields( const QgsFields &inputFields ) const;

    /**
     * Maps the input source coordinate reference system (\a inputCrs) to a corresponding
     * output CRS generated by the algorithm. The default behavior is that the algorithm maintains
     * the same CRS as the input source.
     *
     * This is called once by the base class when creating the output sink for the algorithm (i.e. it is
     * not called once per feature processed).
     */
    virtual QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const;

    /**
     * Initializes any extra parameters added by the algorithm subclass. There is no need
     * to declare the input source or output sink, as these are automatically created by
     * QgsProcessingFeatureBasedAlgorithm.
     */
    virtual void initParameters( const QVariantMap &configuration = QVariantMap() );

    /**
     * Returns the source's coordinate reference system. This will only return a valid CRS when
     * called from a subclasses' processFeature() implementation.
     */
    QgsCoordinateReferenceSystem sourceCrs() const;


    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override SIP_THROW( QgsProcessingException );

    /**
     * Returns the feature request used for fetching features to process from the
     * source layer. The default implementation requests all attributes and geometry.
     */
    virtual QgsFeatureRequest request() const;

    /**
     * Checks whether this algorithm supports in-place editing on the given \a layer
     * Default implementation for feature based algorithms run some basic compatibility
     * checks based on the geometry type of the layer.
     *
     * \return TRUE if the algorithm supports in-place editing
     * \since QGIS 3.4
     */
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

    /**
     * Read the source from \a parameters and \a context and set it
     *
     * \since QGIS 3.4
     */
    void prepareSource( const QVariantMap &parameters, QgsProcessingContext &context );

    QgsProcessingAlgorithm::VectorProperties sinkProperties( const QString &sink,
        const QVariantMap &parameters,
        QgsProcessingContext &context,
        const QMap< QString, QgsProcessingAlgorithm::VectorProperties > &sourceProperties ) const override;

  private:

    std::unique_ptr< QgsProcessingFeatureSource > mSource;

};

// clazy:excludeall=qstring-allocations

#endif // QGSPROCESSINGALGORITHM_H
