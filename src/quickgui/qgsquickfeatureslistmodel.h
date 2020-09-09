/***************************************************************************
  qgsquickfeatureslistmodel.h
 ---------------------------
  Date                 : Sep 2020
  Copyright            : (C) 2020 by Tomas Mizera
  Email                : tomas.mizera2 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKFEATURESMODEL_H
#define QGSQUICKFEATURESMODEL_H

#include <QAbstractListModel>

#include "qgsvectorlayer.h"
#include "qgsquickfeaturelayerpair.h"
#include "qgsvaluerelationfieldformatter.h"

/**
 * \ingroup quick
 * List Model holding features of specific layer.
 *
 * Model allows searching by any string or number attribute.
 *
 * Note that the model can run in several modes:
 *  (1) as a features list - usable in listing features from specifig layer
 *  (2) as a value relation model - filling value-relation widget with data from config
 *
 * \note QML Type: FeaturesListModel
 *
 * \since QGIS 3.14
 */
class QUICK_EXPORT QgsQuickFeaturesListModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * Read only property holding true number of features in layer - not only requested features
     * Changing filter expression does not result in changing this number
     */
    Q_PROPERTY( int featuresCount READ featuresCount NOTIFY featuresCountChanged )

    /**
     * Filter Expression represents filter used when querying for data in current layer.
     * String and numerical attributes are compared with filterExpression
     */
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )

    /**
     * Property limiting maximum number of features queried from layer
     * Read only property
     */
    Q_PROPERTY( int featuresLimit READ featuresLimit NOTIFY featuresLimitChanged )

    /**
     * Property determining behaviour of Feature Model.
     *
     * \note ValueRelation type provides different attribute when opting for data with EmitableIndex role, it returns "key" attribute
     */
    Q_PROPERTY( modelTypes modelType READ modelType WRITE setModelType )

    enum roleNames
    {
      FeatureTitle = Qt::UserRole + 1,
      FeatureId,
      Feature,
      Description, //! secondary text in list view
      EmitableIndex, //! key in value relation
      FoundPair //! pair of attribute and its value by which the feature was found, empty if mFilterExpression is empty
    };

  public:

    enum modelTypes
    {
      FeatureListing,
      ValueRelation
    };
    Q_ENUM( modelTypes );

    //! Create features list model
    explicit QgsQuickFeaturesListModel( QObject *parent = nullptr );
    ~QgsQuickFeaturesListModel() override {};

    //! Function to get QgsQuickFeatureLayerPair by feature id
    Q_INVOKABLE QgsQuickFeatureLayerPair featureLayerPair( const int &featureId );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief populate populates model with value relation data from config
     * @param config to be used
     */
    Q_INVOKABLE void populate( const QVariantMap &config );

    /**
     * @brief populateFromLayer populates model with features from layer
     * @param layer to be used
     */
    Q_INVOKABLE void populateFromLayer( QgsVectorLayer *layer );

    /**
     * @brief reloadFeatures reloads features from current layer
     */
    Q_INVOKABLE void reloadFeatures();

    //! Methods to translate value relation key into model index
    Q_INVOKABLE int rowIndexFromKey( const QVariant &key ) const;
    Q_INVOKABLE int rowModelIndexFromKey( const QVariant &key ) const;

    int featuresLimit() const;
    int featuresCount() const;
    modelTypes modelType() const;
    QString filterExpression() const;
    void setFilterExpression( const QString &filterExpression );

  public slots:
    void setModelType( modelTypes modelType );

  signals:
    void featuresCountChanged( int featuresCount );
    void featuresLimitChanged( int featuresLimit );
    void filterExpressionChanged( QString filterExpression );

  private:

    /**
     * @brief loadFeaturesFromLayer
     * @param layer
     */
    void loadFeaturesFromLayer( QgsVectorLayer *layer = nullptr );

    //! Empty data when resetting model
    void emptyData();

    //! Builds feature title in list
    QVariant featureTitle( const QgsQuickFeatureLayerPair &featurePair ) const;

    //! Builds filter qgis expression from mFilterExpression
    QString buildFilterExpression();

    //! Returns found attribute and its value from mFilterExpression
    QString foundPair( const QgsQuickFeatureLayerPair &feat ) const;

    /**
     * QList of loaded features from layer
     * Hold maximum of FEATURES_LIMIT features
     * \note mFeatures.size() is not always the same as mFeaturesCount
     */
    QList<QgsQuickFeatureLayerPair> mFeatures;

    //! Number of maximum features loaded from layer
    const int FEATURES_LIMIT = 10000;

    //! Search string, change of string results in reloading features from mCurrentLayer
    QString mFilterExpression;

    //! Pointer to layer that is currently loaded
    QgsVectorLayer *mCurrentLayer = nullptr;

    //! Data from config for value relations
    QgsValueRelationFieldFormatter::ValueRelationCache mCache;

    //! Type of a model - Listing (browsing) features or use in value relation widget
    modelTypes mModelType;

    //! Field that is used as a "key" in value relation
    QString mKeyFieldName;
};

#endif // QGSQUICKFEATURESMODEL_H
