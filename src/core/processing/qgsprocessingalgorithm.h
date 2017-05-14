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
#include <QString>
#include <QVariant>
#include <QIcon>

class QgsProcessingProvider;
class QgsProcessingContext;
class QgsProcessingFeedback;

/**
 * \class QgsProcessingAlgorithm
 * \ingroup core
 * Abstract base class for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingAlgorithm
{
  public:

    //! Flags indicating how and when an algorithm operates and should be exposed to users
    enum Flag
    {
      FlagHideFromToolbox = 1 << 1, //!< Algorithm should be hidden from the toolbox
      FlagHideFromModeler = 1 << 2, //!< Algorithm should be hidden from the modeler
      FlagSupportsBatch = 1 << 3,  //!< Algorithm supports batch mode
      FlagDeprecated = FlagHideFromToolbox | FlagHideFromModeler, //!< Algorithm is deprecated
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingAlgorithm.
     */
    QgsProcessingAlgorithm() = default;

    virtual ~QgsProcessingAlgorithm();

    //! Algorithms cannot be copied
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other ) = delete;
    //! Algorithms cannot be copied
    QgsProcessingAlgorithm &operator=( const QgsProcessingAlgorithm &other ) = delete;

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
     * \see name()
     */
    virtual QString displayName() const = 0;

    /**
     * Returns a list of tags which relate to the algorithm, and are used to assist users in searching
     * for suitable algorithms. These tags should be localised.
    */
    virtual QStringList tags() const { return QStringList(); }

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
     * \see tags()
    */
    virtual QString group() const { return QString(); }

    /**
     * Returns the flags indicating how and when the algorithm operates and should be exposed to users.
     */
    virtual Flags flags() const;

    /**
     * Returns the provider to which this algorithm belongs.
     */
    QgsProcessingProvider *provider() const;

    /**
     * Associates this algorithm with its provider. No transfer of ownership is involved.
     */
    //TEMPORARY - remove when algorithms are no longer copied in python code
    void setProvider( QgsProcessingProvider *provider );

    /**
     * Returns an ordered list of parameter definitions utilized by the algorithm.
     * \see addParameter()
     * \see parameterDefinition()
     */
    QgsProcessingParameterDefinitions parameterDefinitions() const { return mParameters; }

    /**
     * Returns a matching parameter by \a name. Matching is done in a case-insensitive
     * manner.
     * \see parameterDefinitions()
     */
    const QgsProcessingParameterDefinition *parameterDefinition( const QString &name ) const;

    /**
     * Returns the number of visible (non-hidden) parameters defined by this
     * algorithm.
     */
    int countVisibleParameters() const;

    /**
     * Runs the algorithm using the specified \a parameters. Algorithms should implement
     * their custom processing logic here.
     *
     * The \a context argument specifies the context in which the algorithm is being run.
     *
     * Algorithm progress should be reported using the supplied \a feedback object. Additionally,
     * well-behaved algorithms should periodically check \a feedback to determine whether the
     * algorithm should be canceled and exited early.
     *
     * \returns A map of algorithm outputs. These may be output layer references, or calculated
     * values such as statistical calculations.
     */
    virtual QVariantMap run( const QVariantMap &parameters,
                             QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const;

  protected:

    /**
     * Adds a parameter \a definition to the algorithm. Ownership of the definition is transferred to the algorithm.
     * Returns true if parameter could be successfully added, or false if the parameter could not be added (e.g.
     * as a result of a duplicate name).
     */
    bool addParameter( QgsProcessingParameterDefinition *parameterDefinition SIP_TRANSFER );

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
     * Evaluates the parameter with matching \a name to a enum value.
     */
    int parameterAsEnum( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to list of enum values.
     */
    QList<int> parameterAsEnums( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a static boolean value.
     */
    bool parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const;

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
     */
    QgsRectangle parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a point.
     */
    QgsPoint parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

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
     * Evaluates the parameter with matching \a name to a range of values.
     */
    QList<double> parameterAsRange( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;

    /**
     * Evaluates the parameter with matching \a name to a list of fields.
     */
    QStringList parameterAsFields( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const;


  private:

    QgsProcessingProvider *mProvider = nullptr;
    QgsProcessingParameterDefinitions mParameters;

    // friend class to access setProvider() - we do not want this public!
    friend class QgsProcessingProvider;
    friend class TestQgsProcessing;

#ifdef SIP_RUN
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other );
#endif

};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingAlgorithm::Flags )

#endif // QGSPROCESSINGALGORITHM_H


