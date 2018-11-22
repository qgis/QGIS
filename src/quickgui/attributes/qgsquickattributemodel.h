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
 * Basic item model for attributes of QgsFeature associated
 * from feature layer pair. Each attribute of the feature
 * gets a row in the model. Also supports CRUD operations
 * related to layer and feature pair.
 *
 * On top of the QgsFeature attributes, support for "remember"
 * attribute is added. Remember attribute is used for
 * quick addition of the multiple features to the same layer.
 * A new feature can use "remembered" attribute values from
 * the last feature added.
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

    //! Gets remembered attributes
    QVector<bool> rememberedAttributes() const;

    //!\copydoc QgsQuickAttributeModel::featureLayerPair
    QgsQuickFeatureLayerPair featureLayerPair() const;

    //!\copydoc QgsQuickAttributeModel::featureLayerPair
    void setFeatureLayerPair( const QgsQuickFeatureLayerPair &pair );

  public slots:

  signals:
    //! Feature or layer changed in feature layer pair
    void featureLayerPairChanged();

    //! Feature updated, layer is the same
    void featureChanged();

    //! Layer changed, feature is the same
    void layerChanged();

  protected:
    //! Commits model changes
    bool commit();
    //! Starts editing model
    bool startEditing();

    QgsQuickFeatureLayerPair mFeatureLayerPair;
    QVector<bool> mRememberedAttributes;
  private:
    void setFeature( const QgsFeature &feature );
    void setVectorLayer( QgsVectorLayer *layer );
};

#endif // QGSQUICKATTRIBUTEMODEL_H
