/***************************************************************************
  qgsvectorlayerutils.h
  ---------------------
  Date                 : October 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERUTILS_H
#define QGSVECTORLAYERUTILS_H

#include "qgis_core.h"
#include "qgsgeometry.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsfeaturesink.h"

class QgsFeatureRenderer;
class QgsSymbolLayer;
class QgsSymbolLayerId;

/**
 * \ingroup core
 * \class QgsVectorLayerUtils
 * \brief Contains utility methods for working with QgsVectorLayers.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUtils
{
  public:

    /**
     * \ingroup core
     * \class QgsDuplicateFeatureContext
     * \brief Contains mainly the QMap with QgsVectorLayer and QgsFeatureIds do list all the duplicated features
     *
     * \since QGIS 3.0
     */
    class CORE_EXPORT QgsDuplicateFeatureContext
    {
      public:

        //! Constructor for QgsDuplicateFeatureContext
        QgsDuplicateFeatureContext() = default;

        /**
         * Returns all the layers on which features have been duplicated
         * \since QGIS 3.0
         */
        QList<QgsVectorLayer *> layers() const;

        /**
         * Returns the duplicated features in the given layer
         * \since QGIS 3.0
         */
        QgsFeatureIds duplicatedFeatures( QgsVectorLayer *layer ) const;


      private:
        QMap<QgsVectorLayer *, QgsFeatureIds> mDuplicatedFeatures;
        friend class QgsVectorLayerUtils;

        /**
         * To set info about duplicated features to the function feedback (layout and ids)
         * \since QGIS 3.0
         */
        void setDuplicatedFeatures( QgsVectorLayer *layer, const QgsFeatureIds &ids );
    };

    /**
     * \ingroup core
     * \class QgsFeatureData
     * \brief Encapsulate geometry and attributes for new features, to be passed to createFeatures
     * \see createFeatures()
     * \since QGIS 3.6
     */
    class CORE_EXPORT QgsFeatureData
    {
      public:

        /**
         * Constructs a new QgsFeatureData with given \a geometry and \a attributes
         */
        QgsFeatureData( const QgsGeometry &geometry = QgsGeometry(), const QgsAttributeMap &attributes = QgsAttributeMap() );

        //! Returns geometry
        QgsGeometry geometry() const;

        //! Returns attributes
        QgsAttributeMap attributes() const;

      private:
        QgsGeometry mGeometry;
        QgsAttributeMap mAttributes;
    };

    // SIP does not like "using", use legacy typedef
    //! Alias for list of QgsFeatureData
    typedef QList<QgsVectorLayerUtils::QgsFeatureData> QgsFeaturesDataList;

    /**
     * Create a feature iterator for a specified field name or expression.
     * \param layer vector layer to retrieve values from
     * \param fieldOrExpression field name or an expression string
     * \param ok will be set to FALSE if field or expression is invalid, otherwise TRUE
     * \param selectedOnly set to TRUE to get values from selected features only
     * \returns feature iterator
     * \since QGIS 3.0
     */
    static QgsFeatureIterator getValuesIterator( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly );

    /**
     * Fetches all values from a specified field name or expression.
     * \param layer vector layer to retrieve values from
     * \param fieldOrExpression field name or an expression string
     * \param ok will be set to FALSE if field or expression is invalid, otherwise TRUE
     * \param selectedOnly set to TRUE to get values from selected features only
     * \param feedback optional feedback object to allow cancellation
     * \returns list of fetched values
     * \see getDoubleValues
     * \since QGIS 3.0
     */
    static QList< QVariant > getValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, QgsFeedback *feedback = nullptr );

    /**
     * Fetches all double values from a specified field name or expression. Null values or
     * invalid expression results are skipped.
     * \param layer vector layer to retrieve values from
     * \param fieldOrExpression field name or an expression string evaluating to a double value
     * \param ok will be set to FALSE if field or expression is invalid, otherwise TRUE
     * \param selectedOnly set to TRUE to get values from selected features only
     * \param nullCount optional pointer to integer to store number of null values encountered in
     * \param feedback optional feedback object to allow cancellation
     * \returns list of fetched values
     * \see getValues
     * \since QGIS 3.0
     */
    static QList< double > getDoubleValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, int *nullCount = nullptr, QgsFeedback *feedback = nullptr );

    /**
     * Returns TRUE if the specified value already exists within a field. This method can be used to test for uniqueness
     * of values inside a layer's attributes. An optional list of ignored feature IDs can be provided, if so, any features
     * with IDs within this list are ignored when testing for existence of the value.
     * \see createUniqueValue()
     */
    static bool valueExists( const QgsVectorLayer *layer, int fieldIndex, const QVariant &value, const QgsFeatureIds &ignoreIds = QgsFeatureIds() );

    /**
     * Returns a new attribute value for the specified field index which is guaranteed to be unique. The optional seed
     * value can be used as a basis for generated values.
     * \see valueExists()
     */
    static QVariant createUniqueValue( const QgsVectorLayer *layer, int fieldIndex, const QVariant &seed = QVariant() );

    /**
     * Returns a new attribute value for the specified field index which is guaranteed to
     * be unique within regard to \a existingValues.
     * The optional seed value can be used as a basis for generated values.
     * \since QGIS 3.6
     */
    static QVariant createUniqueValueFromCache( const QgsVectorLayer *layer, int fieldIndex, const QSet<QVariant> &existingValues, const QVariant &seed = QVariant() );

    /**
     * Tests a feature attribute value to check whether it passes all constraints which are present on the corresponding field.
     * Returns TRUE if the attribute value is valid for the field. Any constraint failures will be reported in the errors argument.
     * If the strength or origin parameter is set then only constraints with a matching strength/origin will be checked.
     */
    static bool validateAttribute( const QgsVectorLayer *layer, const QgsFeature &feature, int attributeIndex, QStringList &errors SIP_OUT,
                                   QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthNotSet,
                                   QgsFieldConstraints::ConstraintOrigin origin = QgsFieldConstraints::ConstraintOriginNotSet );

    /**
     * Creates a new feature ready for insertion into a layer. Default values and constraints
     * (e.g., unique constraints) will automatically be handled. An optional attribute map can be
     * passed for the new feature to copy as many attribute values as possible from the map,
     * assuming that they respect the layer's constraints. Note that the created feature is not
     * automatically inserted into the layer.
     * \see createFeatures()
     */
    static QgsFeature createFeature( const QgsVectorLayer *layer,
                                     const QgsGeometry &geometry = QgsGeometry(),
                                     const QgsAttributeMap &attributes = QgsAttributeMap(),
                                     QgsExpressionContext *context = nullptr );

    /**
     * Creates a set of new features ready for insertion into a layer. Default values and constraints
     * (e.g., unique constraints) will automatically be handled. Note that the created features are not
     * automatically inserted into the layer.
     * \see createFeature()
     * \since QGIS 3.6
     */
    static QgsFeatureList createFeatures( const QgsVectorLayer *layer,
                                          const QgsFeaturesDataList &featuresData,
                                          QgsExpressionContext *context = nullptr );

    /**
     * Duplicates a feature and it's children (one level deep). It calls CreateFeature, so
     * default values and constraints (e.g., unique constraints) will automatically be handled.
     * The duplicated feature will be automatically inserted into the layer.
     * \a duplicateFeatureContext stores all the layers and the featureids of the duplicated features (incl. children)
     * \a maxDepth the maximum depth to duplicate children in relations, 0 is unlimited depth (in any case, limited to 100)
     * \a depth the current depth, not exposed in Python
     * \a referencedLayersBranch the current branch of layers across the relations, not exposed in Python, taken by copy not reference, used to avoid infinite loop
     * \since QGIS 3.0
     */
    static QgsFeature duplicateFeature( QgsVectorLayer *layer, const QgsFeature &feature, QgsProject *project, QgsDuplicateFeatureContext &duplicateFeatureContext SIP_OUT, const int maxDepth = 0, int depth SIP_PYARGREMOVE = 0, QList<QgsVectorLayer *> referencedLayersBranch SIP_PYARGREMOVE = QList<QgsVectorLayer *>() );


    /**
     * Gets the feature source from a QgsVectorLayer pointer.
     * This method is thread-safe but will block the main thread for execution. Executing it from the main
     * thread is safe too.
     * This should be used in scenarios, where a ``QWeakPointer<QgsVectorLayer>`` is kept in a thread
     * and features should be fetched from this layer. Using the layer directly is not safe to do.
     * The result will be ``NULLPTR`` if the layer has been deleted.
     * If \a feedback is specified, the call will return if the feedback is canceled.
     * Returns a new feature source for the \a layer. The source may be NULLPTR if the layer no longer
     * exists or if the feedback is canceled.
     *
     * \note Requires Qt >= 5.10 to make use of the thread-safe implementation
     * \since QGIS 3.4
     */
    static std::unique_ptr<QgsVectorLayerFeatureSource> getFeatureSource( QPointer<QgsVectorLayer> layer, QgsFeedback *feedback = nullptr ) SIP_SKIP;

    /**
     * Matches the attributes in \a feature to the specified \a fields.
     *
     * This causes the attributes contained within the given \a feature to be rearranged (or in
     * some cases dropped) in order to match the fields and order indicated by \a fields.
     *
     * The exact behavior depends on whether or not \a feature has a valid fields container
     * set (see QgsFeature::fields()). If a fields container is set, then the names of the
     * feature's fields are matched to \a fields. In this case attributes from \a feature
     * will be rearranged or dropped in order to match the field names from \a fields.
     *
     * If the \a feature does not have a valid fields container set, then the feature's attributes
     * are simply truncated to match the number of fields present in \a fields (or if
     * less attributes are present in \a feature than in \a fields, the feature's attributes
     * are padded with NULL values to match the required length).
     * Finally, the feature's fields are set to \a fields.
     *
     * \since QGIS 3.4
     */
    static void matchAttributesToFields( QgsFeature &feature, const QgsFields &fields );

    /**
     * Converts input \a feature to be compatible with the given \a layer.
     *
     * This function returns a new list of transformed features compatible with the input
     * layer, note that the number of features returned might be greater than one when
     * converting a multi part geometry to single part
     *
     * The following operations will be performed to convert the input features:
     *
     * - convert single geometries to multi part
     * - drop additional attributes
     * - drop geometry if layer is geometry-less
     * - add missing attribute fields
     * - add back M/Z values (initialized to 0)
     * - drop Z/M
     * - convert multi part geometries to single part
     *
     * Optionally, \a sinkFlags can be specified to further refine the compatibility logic.
     *
     * \since QGIS 3.4
     */
    static QgsFeatureList makeFeatureCompatible( const QgsFeature &feature, const QgsVectorLayer *layer, QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags() );

    /**
     * Converts input \a features to be compatible with the given \a layer.
     *
     * This function returns a new list of transformed features compatible with the input
     * layer, note that the number of features returned might be greater than the number
     * of input features.
     *
     * The following operations will be performed to convert the input features:
     *
     * - convert single geometries to multi part
     * - drop additional attributes
     * - drop geometry if layer is geometry-less
     * - add missing attribute fields
     * - add back M/Z values (initialized to 0)
     * - drop Z/M
     * - convert multi part geometries to single part
     *
     * Optionally, \a sinkFlags can be specified to further refine the compatibility logic.
     *
     * \since QGIS 3.4
     */
    static QgsFeatureList makeFeaturesCompatible( const QgsFeatureList &features, const QgsVectorLayer *layer, QgsFeatureSink::SinkFlags sinkFlags = QgsFeatureSink::SinkFlags() );

    /**
     * Tests whether a field is editable for a particular \a feature.
     *
     * \returns TRUE if the field at index \a fieldIndex from \a layer
     * is editable, FALSE if the field is read only.
     *
     * \since QGIS 3.10
     */
    static bool fieldIsEditable( const QgsVectorLayer *layer, int fieldIndex, const QgsFeature &feature );

    /**
     * \returns TRUE if the field at index \a fieldIndex from \a layer
     * is editable, FALSE if the field is read only.
     *
     * If this function returns TRUE then the editability of the field may still vary feature by
     * feature. See fieldIsEditable() to determine this on a feature by feature basis.
     *
     * \since QGIS 3.18
     */
    static bool fieldIsReadOnly( const QgsVectorLayer *layer, int fieldIndex );

    /**
     * Returns TRUE if the editability of the field at index \a fieldIndex from \a layer may vary
     * feature by feature.
     *
     * I.e. if the field is taken from a joined layer, the value may or may not be editable for any individual
     * feature depending on the join's "upsert on edit" capabilities.
     *
     * \since QGIS 3.18
     */
    static bool fieldEditabilityDependsOnFeature( const QgsVectorLayer *layer, int fieldIndex );

    /**
      * Returns masks defined in labeling options of a layer.
      * The returned type associates a labeling rule identifier to a set of layers that are masked given by their layer id,
      * and a set of masked symbol layers if associated to each masked layers.
      * \note Not available in Python bindings
      * \since QGIS 3.12
      */
    static QHash<QString, QHash<QString, QSet<QgsSymbolLayerId>>> labelMasks( const QgsVectorLayer * ) SIP_SKIP;

    /**
     * Returns all masks that may be defined on symbol layers for a given vector layer.
     * The hash key is a layer id.
     * The hash value is the set of symbol layers masked in the key's layer.
     * \note Not available in Python bindings
     * \since QGIS 3.12
     */
    static QHash<QString, QSet<QgsSymbolLayerId>> symbolLayerMasks( const QgsVectorLayer * ) SIP_SKIP;

    /**
     * \returns a descriptive string for a \a feature, suitable for displaying to the user.
     *         The definition is taken from the ``displayExpression`` property of \a layer.
     * \since QGIS 3.12
     */
    static QString getFeatureDisplayString( const QgsVectorLayer *layer, const QgsFeature &feature );

    /**
     * Flags that can be used when determining cascaded features.
     *
     * \since QGIS 3.4
     */
    enum CascadedFeatureFlag
    {
      IgnoreAuxiliaryLayers = 1 << 1, //!< Ignore auxiliary layers
    };
    Q_DECLARE_FLAGS( CascadedFeatureFlags, CascadedFeatureFlag )

    /**
     * \returns TRUE if at least one feature of the \a fids on \a layer is connected as parent in at
     * least one composition relation of the \a project or contains joins, where cascade delete is set.
     * Details about cascading effects will be written to \a context.
     * \since QGIS 3.14
     */
    static bool impactsCascadeFeatures( const QgsVectorLayer *layer, const QgsFeatureIds &fids, const QgsProject *project, QgsDuplicateFeatureContext &context SIP_OUT, QgsVectorLayerUtils::CascadedFeatureFlags flags = QgsVectorLayerUtils::CascadedFeatureFlags() );

#ifndef SIP_RUN

    /**
     * Given a set of fields, attempts to pick the "most useful" field
     * for user-friendly identification of features.
     *
     * For instance, if a field called "name" is present, this will be returned.
     *
     * Assumes that the user has organized the data with the more "interesting" field
     * names first. As such, "name" would be selected before "oldname", "othername", etc.
     *
     * If no friendly identifier is found, the function will fallback to the
     * first available.
     *
     * An optional boolean parameter can be used to determine whether the returned
     * field name is a friendly identifier or not.
     *
     * \param fields list of fields to pick a friendly identifier from
     * \param foundFriendly set to TRUE if the returned field name is a friendly identifier (since QGIS 3.22)
     * \returns field name
     * \since QGIS 3.18
     */
#else

    /**
     * Given a set of fields, attempts to pick the "most useful" field
     * for user-friendly identification of features.
     *
     * For instance, if a field called "name" is present, this will be returned.
     *
     * Assumes that the user has organized the data with the more "interesting" field
     * names first. As such, "name" would be selected before "oldname", "othername", etc.
     *
     * If no friendly identifier is found, the function will fallback to the
     * first available.
     *
     * \param fields list of fields to pick a friendly identifier from
     * \param foundFriendly set to TRUE if the returned field name is a friendly identifier
     * \returns field name
     * \since QGIS 3.22
     */
#endif
    static QString guessFriendlyIdentifierField( const QgsFields &fields, bool *foundFriendly SIP_OUT = nullptr ) SIP_PYNAME( guessFriendlyIdentifierFieldV2 );

#ifdef SIP_RUN

    /**
     * Given a set of fields, attempts to pick the "most useful" field
     * for user-friendly identification of features.
     *
     * For instance, if a field called "name" is present, this will be returned.
     *
     * Assumes that the user has organized the data with the more "interesting" field
     * names first. As such, "name" would be selected before "oldname", "othername", etc.
     *
     * If no friendly identifier is found, the function will fallback to the
     * first available.
     *
     * \param fields list of fields to pick a friendly identifier from
     * \returns field name
     * \since QGIS 3.18
     */
    static QString guessFriendlyIdentifierField( const QgsFields &fields );
#endif

};


#endif // QGSVECTORLAYERUTILS_H
