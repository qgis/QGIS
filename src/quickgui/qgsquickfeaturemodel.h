/***************************************************************************
 qgsquickfeaturemodel.h
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

#ifndef QGSQUICKFEATUREMODEL_H
#define QGSQUICKFEATUREMODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "qgsfeature.h"
#include "qgsvectorlayer.h"

#include "qgis_quick.h"

/**
 * \ingroup quick
 * Item model implementation for attributes of QgsFeature: each attribute gets a row in the model.
 *
 * \note QML Type: FeatureModel
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickFeatureModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY( QgsFeature feature READ feature WRITE setFeature NOTIFY featureChanged )
    Q_PROPERTY( QgsVectorLayer *layer READ layer WRITE setLayer NOTIFY layerChanged )
    Q_ENUMS( FeatureRoles )

  public:
    enum FeatureRoles
    {
      AttributeName = Qt::UserRole + 1,  //!< Attribute's display name (the original field name or a custom alias)
      AttributeValue,                    //!< Value of the feature's attribute
      Field,                             //!< Field definition (QgsField)
      RememberAttribute
    };

    explicit QgsQuickFeatureModel( QObject *parent = 0 );
    explicit QgsQuickFeatureModel( const QgsFeature &feat, QObject *parent = 0 );

    void setFeature( const QgsFeature &feature );

    /**
     * Return the feature wrapped in a QVariant for passing it around in QML
     */
    QgsFeature feature() const;


    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;


    QHash<int, QByteArray> roleNames() const override;
    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Will commit the edit buffer of this layer.
     * May change in the future to only commit the changes buffered in this model.
     *
     * @return Success of the operation
     */
    Q_INVOKABLE bool save();

    /**
     * Will delete the current feature from the layer and commit the changes.
     * @return Success of the operation
     */
    Q_INVOKABLE bool deleteFeature();

    /**
     * Will reset the feature to the original values and dismiss any buffered edits.
     */
    Q_INVOKABLE void reset();
    Q_INVOKABLE void create();

    Q_INVOKABLE bool suppressFeatureForm() const;

    Q_INVOKABLE void resetAttributes();

    QVector<bool> rememberedAttributes() const;

  public slots:

  signals:
    void featureChanged();
    void layerChanged();

    void warning( const QString &text );

  private:
    bool commit();
    bool startEditing();

    QgsVectorLayer *mLayer = nullptr;
    QgsFeature mFeature;
    QVector<bool> mRememberedAttributes;
};

#endif // QGSQUICKFEATUREMODEL_H
