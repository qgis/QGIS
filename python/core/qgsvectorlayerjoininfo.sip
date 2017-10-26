/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerjoininfo.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsVectorLayerJoinInfo
{
%Docstring
 Defines left outer join from our vector layer to some other vector layer.
 The join is done based on [our layer].targetField = [join layer].joinField

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerjoininfo.h"
%End
  public:

    QgsVectorLayerJoinInfo();
%Docstring
 Constructor for QgsVectorLayerJoinInfo.
%End

    void setJoinLayer( QgsVectorLayer *layer );
%Docstring
Sets weak reference to the joined layer
%End
    QgsVectorLayer *joinLayer() const;
%Docstring
Returns joined layer (may be null if the reference was set by layer ID and not resolved yet)
 :rtype: QgsVectorLayer
%End

    void setJoinLayerId( const QString &layerId );
%Docstring
Sets ID of the joined layer. It will need to be overwritten by setJoinLayer() to a reference to real layer
%End
    QString joinLayerId() const;
%Docstring
ID of the joined layer - may be used to resolve reference to the joined layer
 :rtype: str
%End

    void setTargetFieldName( const QString &fieldName );
%Docstring
Sets name of the field of our layer that will be used for join
%End
    QString targetFieldName() const;
%Docstring
Returns name of the field of our layer that will be used for join
 :rtype: str
%End

    void setJoinFieldName( const QString &fieldName );
%Docstring
Sets name of the field of joined layer that will be used for join
%End
    QString joinFieldName() const;
%Docstring
Returns name of the field of joined layer that will be used for join
 :rtype: str
%End

    void setPrefix( const QString &prefix );
%Docstring
Sets prefix of fields from the joined layer. If null, joined layer's name will be used.
%End
    QString prefix() const;
%Docstring
Returns prefix of fields from the joined layer. If null, joined layer's name will be used.
 :rtype: str
%End

    void setUsingMemoryCache( bool enabled );
%Docstring
Sets whether values from the joined layer should be cached in memory to speed up lookups
%End
    bool isUsingMemoryCache() const;
%Docstring
Returns whether values from the joined layer should be cached in memory to speed up lookups
 :rtype: bool
%End

    bool isDynamicFormEnabled() const;
%Docstring
 Returns whether the form has to be dynamically updated with joined fields
  when  a feature is being created in the target layer.
.. versionadded:: 3.0
 :rtype: bool
%End

    void setDynamicFormEnabled( bool enabled );
%Docstring
 Sets whether the form has to be dynamically updated with joined fields
  when a feature is being created in the target layer.
.. versionadded:: 3.0
%End

    bool isEditable() const;
%Docstring
 Returns whether joined fields may be edited through the form of
  the target layer.
.. versionadded:: 3.0
 :rtype: bool
%End

    void setEditable( bool enabled );
%Docstring
 Sets whether the form of the target layer allows editing joined fields.
.. versionadded:: 3.0
%End

    bool hasUpsertOnEdit() const;
%Docstring
 Returns whether a feature created on the target layer has to impact
  the joined layer by creating a new feature if necessary.
.. versionadded:: 3.0
 :rtype: bool
%End

    void setUpsertOnEdit( bool enabled );
%Docstring
 Sets whether a feature created on the target layer has to impact
  the joined layer by creating a new feature if necessary.
.. versionadded:: 3.0
%End

    bool hasCascadedDelete() const;
%Docstring
 Returns whether a feature deleted on the target layer has to impact the
  joined layer by deleting the corresponding joined feature.
.. versionadded:: 3.0
 :rtype: bool
%End

    void setCascadedDelete( bool enabled );
%Docstring
 Sets whether a feature deleted on the target layer has to impact the
  joined layer by deleting the corresponding joined feature.
.. versionadded:: 3.0
%End

    QString prefixedFieldName( const QgsField &field ) const;
%Docstring
 Returns the prefixed name of the field.
 \param field the field
 :return: the prefixed name of the field
.. versionadded:: 3.0
 :rtype: str
%End

    QgsFeature extractJoinedFeature( const QgsFeature &feature ) const;
%Docstring
 Extract the join feature from the target feature for the current
  join layer information.
 \param feature A feature from the target layer
 :return: the corresponding joined feature
.. versionadded:: 3.0
 :rtype: QgsFeature
%End

    void setJoinFieldNamesBlackList( const QStringList &blackList );
%Docstring
 Sets a list of fields to ignore whatever happens.

.. versionadded:: 3.0
%End

    QStringList joinFieldNamesBlackList() const;
%Docstring
 Returns the list of fields to ignore.

.. versionadded:: 3.0
 :rtype: list of str
%End

    bool hasSubset( bool blacklisted = true ) const;
%Docstring
 Returns true if blacklisted fields is not empty or if a subset of names
 has been set.

.. versionadded:: 3.0
 :rtype: bool
%End

    static QStringList joinFieldNamesSubset( const QgsVectorLayerJoinInfo &info, bool blacklisted = true );
%Docstring
 Returns the list of field names to use for joining considering
 blacklisted fields and subset.

.. versionadded:: 3.0
 :rtype: list of str
%End

    bool operator==( const QgsVectorLayerJoinInfo &other ) const;

    void setJoinFieldNamesSubset( QStringList *fieldNamesSubset /Transfer/ );
%Docstring
 Set subset of fields to be used from joined layer. Takes ownership of the passed pointer. Null pointer tells to use all fields.
.. versionadded:: 2.6
%End

    QStringList *joinFieldNamesSubset() const;
%Docstring
 Get subset of fields to be used from joined layer. All fields will be used if null is returned.
.. versionadded:: 2.6
 :rtype: list of str
%End

  protected:













};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerjoininfo.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
