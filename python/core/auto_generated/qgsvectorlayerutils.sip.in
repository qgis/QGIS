/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerutils.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsVectorLayerUtils
{
%Docstring
Contains utility methods for working with QgsVectorLayers.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerutils.h"
%End
  public:

    class QgsDuplicateFeatureContext
{
%Docstring
Contains mainly the QMap with QgsVectorLayer and QgsFeatureIds do list all the duplicated features

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerutils.h"
%End
      public:

        QgsDuplicateFeatureContext();
%Docstring
Constructor for QgsDuplicateFeatureContext
%End

        QList<QgsVectorLayer *> layers() const;
%Docstring
Returns all the layers on which features have been duplicated

.. versionadded:: 3.0
%End

        QgsFeatureIds duplicatedFeatures( QgsVectorLayer *layer ) const;
%Docstring
Returns the duplicated features in the given layer

.. versionadded:: 3.0
%End


    };

    class QgsFeatureData
{
%Docstring
Encapsulate geometry and attributes for new features, to be passed to createFeatures

.. seealso:: :py:func:`createFeatures`

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsvectorlayerutils.h"
%End
      public:

        QgsFeatureData( const QgsGeometry &geometry = QgsGeometry(), const QgsAttributeMap &attributes = QgsAttributeMap() );
%Docstring
Constructs a new QgsFeatureData with given ``geometry`` and ``attributes``
%End

        QgsGeometry geometry() const;
%Docstring
Returns geometry
%End

        QgsAttributeMap attributes() const;
%Docstring
Returns attributes
%End

    };

    typedef QList<QgsVectorLayerUtils::QgsFeatureData> QgsFeaturesDataList;

    static QgsFeatureIterator getValuesIterator( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly );
%Docstring
Create a feature iterator for a specified field name or expression.

:param layer: vector layer to retrieve values from
:param fieldOrExpression: field name or an expression string
:param ok: will be set to ``False`` if field or expression is invalid, otherwise ``True``
:param selectedOnly: set to ``True`` to get values from selected features only

:return: feature iterator

.. versionadded:: 3.0
%End

    static QList< QVariant > getValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, QgsFeedback *feedback = 0 );
%Docstring
Fetches all values from a specified field name or expression.

:param layer: vector layer to retrieve values from
:param fieldOrExpression: field name or an expression string
:param ok: will be set to ``False`` if field or expression is invalid, otherwise ``True``
:param selectedOnly: set to ``True`` to get values from selected features only
:param feedback: optional feedback object to allow cancellation

:return: list of fetched values

.. seealso:: :py:func:`getDoubleValues`

.. versionadded:: 3.0
%End

    static QList< double > getDoubleValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, int *nullCount = 0, QgsFeedback *feedback = 0 );
%Docstring
Fetches all double values from a specified field name or expression. Null values or
invalid expression results are skipped.

:param layer: vector layer to retrieve values from
:param fieldOrExpression: field name or an expression string evaluating to a double value
:param ok: will be set to ``False`` if field or expression is invalid, otherwise ``True``
:param selectedOnly: set to ``True`` to get values from selected features only
:param nullCount: optional pointer to integer to store number of null values encountered in
:param feedback: optional feedback object to allow cancellation

:return: list of fetched values

.. seealso:: :py:func:`getValues`

.. versionadded:: 3.0
%End

    static bool valueExists( const QgsVectorLayer *layer, int fieldIndex, const QVariant &value, const QgsFeatureIds &ignoreIds = QgsFeatureIds() );
%Docstring
Returns ``True`` if the specified value already exists within a field. This method can be used to test for uniqueness
of values inside a layer's attributes. An optional list of ignored feature IDs can be provided, if so, any features
with IDs within this list are ignored when testing for existence of the value.

.. seealso:: :py:func:`createUniqueValue`
%End

    static QVariant createUniqueValue( const QgsVectorLayer *layer, int fieldIndex, const QVariant &seed = QVariant() );
%Docstring
Returns a new attribute value for the specified field index which is guaranteed to be unique. The optional seed
value can be used as a basis for generated values.

.. seealso:: :py:func:`valueExists`
%End

    static QVariant createUniqueValueFromCache( const QgsVectorLayer *layer, int fieldIndex, const QSet<QVariant> &existingValues, const QVariant &seed = QVariant() );
%Docstring
Returns a new attribute value for the specified field index which is guaranteed to
be unique within regard to ``existingValues``.
The optional seed value can be used as a basis for generated values.

.. versionadded:: 3.6
%End

    static bool validateAttribute( const QgsVectorLayer *layer, const QgsFeature &feature, int attributeIndex, QStringList &errors /Out/,
                                   QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthNotSet,
                                   QgsFieldConstraints::ConstraintOrigin origin = QgsFieldConstraints::ConstraintOriginNotSet );
%Docstring
Tests an attribute value to check whether it passes all constraints which are present on the corresponding field.
Returns ``True`` if the attribute value is valid for the field. Any constraint failures will be reported in the errors argument.
If the strength or origin parameter is set then only constraints with a matching strength/origin will be checked.
%End

    static QgsFeature createFeature( const QgsVectorLayer *layer,
                                     const QgsGeometry &geometry = QgsGeometry(),
                                     const QgsAttributeMap &attributes = QgsAttributeMap(),
                                     QgsExpressionContext *context = 0 );
%Docstring
Creates a new feature ready for insertion into a layer. Default values and constraints
(e.g., unique constraints) will automatically be handled. An optional attribute map can be
passed for the new feature to copy as many attribute values as possible from the map,
assuming that they respect the layer's constraints. Note that the created feature is not
automatically inserted into the layer.

.. seealso:: :py:func:`createFeatures`
%End

    static QgsFeatureList createFeatures( const QgsVectorLayer *layer,
                                          const QgsFeaturesDataList &featuresData,
                                          QgsExpressionContext *context = 0 );
%Docstring
Creates a set of new features ready for insertion into a layer. Default values and constraints
(e.g., unique constraints) will automatically be handled. Note that the created features are not
automatically inserted into the layer.

.. seealso:: :py:func:`createFeature`

.. versionadded:: 3.6
%End

    static QgsFeature duplicateFeature( QgsVectorLayer *layer, const QgsFeature &feature, QgsProject *project, int depth, QgsDuplicateFeatureContext &duplicateFeatureContext /Out/ );
%Docstring
Duplicates a feature and it's children (one level deep). It calls CreateFeature, so
default values and constraints (e.g., unique constraints) will automatically be handled.
The duplicated feature will be automatically inserted into the layer.
``depth`` the higher this number the deeper the level - With depth > 0 the children of the feature are not duplicated
``duplicateFeatureContext`` stores all the layers and the featureids of the duplicated features (incl. children)

.. versionadded:: 3.0
%End


    static void matchAttributesToFields( QgsFeature &feature, const QgsFields &fields );
%Docstring
Matches the attributes in ``feature`` to the specified ``fields``.

This causes the attributes contained within the given ``feature`` to be rearranged (or in
some cases dropped) in order to match the fields and order indicated by ``fields``.

The exact behavior depends on whether or not ``feature`` has a valid fields container
set (see QgsFeature.fields()). If a fields container is set, then the names of the
feature's fields are matched to ``fields``. In this case attributes from ``feature``
will be rearranged or dropped in order to match the field names from ``fields``.

If the ``feature`` does not have a valid fields container set, then the feature's attributes
are simply truncated to match the number of fields present in ``fields`` (or if
less attributes are present in ``feature`` than in ``fields``, the feature's attributes
are padded with NULL values to match the required length).
Finally, the feature's fields are set to ``fields``.

.. versionadded:: 3.4
%End

    static QgsFeatureList makeFeatureCompatible( const QgsFeature &feature, const QgsVectorLayer *layer );
%Docstring
Converts input ``feature`` to be compatible with the given ``layer``.

This function returns a new list of transformed features compatible with the input
layer, note that the number of features returned might be greater than one when
converting a multi part geometry to single part

The following operations will be performed to convert the input features:
- convert single geometries to multi part
- drop additional attributes
- drop geometry if layer is geometry-less
- add missing attribute fields
- add back M/Z values (initialized to 0)
- drop Z/M
- convert multi part geometries to single part

.. versionadded:: 3.4
%End

    static QgsFeatureList makeFeaturesCompatible( const QgsFeatureList &features, const QgsVectorLayer *layer );
%Docstring
Converts input ``features`` to be compatible with the given ``layer``.

This function returns a new list of transformed features compatible with the input
layer, note that the number of features returned might be greater than the number
of input features.

The following operations will be performed to convert the input features:
- convert single geometries to multi part
- drop additional attributes
- drop geometry if layer is geometry-less
- add missing attribute fields
- add back M/Z values (initialized to 0)
- drop Z/M
- convert multi part geometries to single part

.. versionadded:: 3.4
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerutils.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
