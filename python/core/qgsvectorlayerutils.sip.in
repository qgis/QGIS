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

    static bool valueExists( const QgsVectorLayer *layer, int fieldIndex, const QVariant &value, const QgsFeatureIds &ignoreIds = QgsFeatureIds() );
%Docstring
Returns true if the specified value already exists within a field. This method can be used to test for uniqueness
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

    static bool validateAttribute( const QgsVectorLayer *layer, const QgsFeature &feature, int attributeIndex, QStringList &errors /Out/,
                                   QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthNotSet,
                                   QgsFieldConstraints::ConstraintOrigin origin = QgsFieldConstraints::ConstraintOriginNotSet );
%Docstring
Tests an attribute value to check whether it passes all constraints which are present on the corresponding field.
Returns true if the attribute value is valid for the field. Any constraint failures will be reported in the errors argument.
If the strength or origin parameter is set then only constraints with a matching strength/origin will be checked.
%End

    static QgsFeature createFeature( QgsVectorLayer *layer,
                                     const QgsGeometry &geometry = QgsGeometry(),
                                     const QgsAttributeMap &attributes = QgsAttributeMap(),
                                     QgsExpressionContext *context = 0 );
%Docstring
Creates a new feature ready for insertion into a layer. Default values and constraints
(e.g., unique constraints) will automatically be handled. An optional attribute map can be
passed for the new feature to copy as many attribute values as possible from the map,
assuming that they respect the layer's constraints. Note that the created feature is not
automatically inserted into the layer.
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

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerutils.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
