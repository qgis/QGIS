/***************************************************************************
 QgsQuickAttributeModel.h
  --------------------------------------
  Date                 : 10.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKATTRIBUTEMODEL_H
#define QGSQUICKATTRIBUTEMODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "qgsfeature.h"

#include "qgis_quick.h"
#include "qgsquickfeaturelayerpair.h"

/**
 * \ingroup quick
 *
 * \brief Basic item model for attributes of QgsFeature associated
 * from feature layer pair. Each attribute of the feature
 * gets a row in the model. Also supports CRUD operations
 * related to layer and feature pair.
 *
 * On top of the QgsFeature attributes, support for "remember"
 * values is added. When model is allowed to remember values, it reuses
 * values for last created features. However, this only work for attributes
 * that passes filter (\see RememberedValues struct). This feature is used for quick addition
 * of multiple features to the same layer.
 *
 *
 * \note QML Type: AttributeModel
 *
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickAttributeModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * QgsQuickFeatureLayerPair for the model. Input for attributes model.
     */
    Q_PROPERTY( QgsQuickFeatureLayerPair featureLayerPair READ featureLayerPair WRITE setFeatureLayerPair NOTIFY featureLayerPairChanged )

    /**
     * Feature roles enum.
     */
    Q_ENUMS( FeatureRoles )

  public:
    //! Feature roles
    enum FeatureRoles
    {
      AttributeName = Qt::UserRole + 1,  //!< Attribute's display name (the original field name or a custom alias)
      AttributeValue,                    //!< Value of the feature's attribute
      Field,                             //!< Field definition (QgsField)
      RememberAttribute                  //!< Remember attribute value for next feature
    };

    //! Remembered values struct contains last created feature instance and a boolean vector masking attributes that should be remembered
    struct RememberedValues
    {
      QgsFeature feature;
      QVector<bool> attributeFilter;
    };

    //! Creates a new feature attribute model
    explicit QgsQuickAttributeModel( QObject *parent = nullptr );

    /**
     * Creates a new feature attribute model
     * \param feat Feature set to model
     * \param parent Parent object.
     */
    explicit QgsQuickAttributeModel( const QgsFeature &feat, QObject *parent = nullptr );


    QHash<int, QByteArray> roleNames() const override;
    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Commits the edit buffer of this layer.
     * May change in the future to only commit the changes buffered in this model.
     *
     * @return Success of the operation.
     */
    Q_INVOKABLE bool save();

    /**
     * Deletes the current feature from the layer and commit the changes.
     * @return Success of the operation.
     */
    Q_INVOKABLE bool deleteFeature();

    /**
     * Resets the feature to the original values and dismiss any buffered edits.
     */
    Q_INVOKABLE void reset();

    //! Adds feature from featureLayerPair to the layer
    Q_INVOKABLE void create();

    /**
     * Suppress layer's QgsEditFormConfig
     *
     * \sa QgsEditFormConfig::suppress
     */
    Q_INVOKABLE bool suppressFeatureForm() const;

    //! Resets remembered attributes
    Q_INVOKABLE virtual void resetAttributes();

    //! Gives information whether field with given index is remembered or not
    bool isFieldRemembered( const int fieldIndex ) const;

    //! Gets current featureLayerPair
    QgsQuickFeatureLayerPair featureLayerPair() const;

    //! Sets current featureLayerPair
    void setFeatureLayerPair( const QgsQuickFeatureLayerPair &pair );

    //! Resets the model
    Q_INVOKABLE void forceClean();

    //! Allows or forbids model to reuse last entered values
    void setRememberValuesAllowed( bool allowed );

    //! Returns whether model is remembering last entered values
    bool rememberValuesAllowed() const;

  public slots:

    //! Handles feature creation
    void onFeatureCreated( const QgsFeature &feature );

    //! Handles changing allowance of reusing last entered values
    void onRememberValuesAllowChanged();

  signals:
    //! Feature or layer changed in feature layer pair
    void featureLayerPairChanged();

    //! Feature updated, layer is the same
    void featureChanged();

    //! Layer changed, feature is the same
    void layerChanged();

    //! Feature has been created
    void featureCreated( const QgsFeature &feature );

    //! Emitted when user allows reusing last entered values
    void rememberValuesAllowChanged();

  protected:
    //! Commits model changes
    bool commit();
    //! Starts editing model
    bool startEditing();

    QgsQuickFeatureLayerPair mFeatureLayerPair;

  private:
    void setFeature( const QgsFeature &feature );
    void setVectorLayer( QgsVectorLayer *layer );

    //! Fills remembered attributes from last created feature (mRememberedValues) to current feature (mFeatureLayerPair)
    void prefillRememberedValues();

    //! Remembered last created feature for each layer (key)
    QHash<QString, RememberedValues> mRememberedValues;

    bool mRememberValuesAllowed;
};

#endif // QGSQUICKATTRIBUTEMODEL_H
