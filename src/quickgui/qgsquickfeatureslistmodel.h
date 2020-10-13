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
 * \note QML Type: FeaturesListModel
 *
 * \since QGIS 3.16
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

  public:

    //! Roles for FeaturesListModel
    enum modelRoles
    {
      FeatureTitle = Qt::UserRole + 1,
      FeatureId,
      Feature,
      Description, //! secondary text in list view
      KeyColumn, //! key in value relation
      FoundPair //! pair of attribute and its value by which the feature was found, empty if mFilterExpression is empty
    };
    Q_ENUM( modelRoles );

    //! Create features list model
    explicit QgsQuickFeaturesListModel( QObject *parent = nullptr );
    ~QgsQuickFeaturesListModel() override {};

    //! Function to get QgsQuickFeatureLayerPair by feature id
    Q_INVOKABLE QgsQuickFeatureLayerPair featureLayerPair( const int &featureId );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * \brief setupValueRelation populates model with value relation data from config
     * \param config to be used
     */
    Q_INVOKABLE void setupValueRelation( const QVariantMap &config );

    /**
     * \brief populateFromLayer populates model with features from layer
     * \param layer to be used
     */
    Q_INVOKABLE void populateFromLayer( QgsVectorLayer *layer );

    /**
     * \brief reloadFeatures reloads features from current layer
     */
    Q_INVOKABLE void reloadFeatures();

    /**
     * \brief rowFromAttribute finds feature with requested role and value, returns its row
     * \param role to find from modelRoles
     * \param value to find
     * \return Row index for found feature, returns -1 if no feature is found. If more features
     * match requested role and value, index of first is returned.
     */
    Q_INVOKABLE int rowFromAttribute( const int role, const QVariant &value ) const;

    /**
     * \brief keyFromAttribute finds feature with requested role and value, returns keycolumn
     * \param role role to find from modelRoles
     * \param value value to find
     * \return KeyColumn role for found feature, returns -1 if no feature is found. If more features
     * match requested role and value, KeyColumn for first is returned.
     */
    Q_INVOKABLE int keyFromAttribute( const int role, const QVariant &value ) const;

    //! Returns maximum amount of features that can be queried from layer
    int featuresLimit() const;
    //! Returns number of features in layer, not number of loaded features
    int featuresCount() const;
    //! Returns filter expression, empty string represents no filter
    QString filterExpression() const;

    /**
     * \brief setFilterExpression Sets filter expression, upon setting also reloads features from current layer with new filter
     * \param filterExpression QString to set, empty string represents no filter
     */
    void setFilterExpression( const QString &filterExpression );

    /**
     * \brief setFeatureTitleField Sets name of attribute that will be used for FeatureTitle and Qt::DisplayRole
     * \param attribute Name of attribute to use. If empty, displayExpression will be used.
     */
    void setFeatureTitleField( const QString &attribute );

    //! Sets name of attribute used as "key" in value relation
    void setKeyField( const QString &attribute );

  signals:

    /**
     * \brief featuresCountChanged Signal emitted when features are reloaded or layer is changed
     * \param featuresCount number of features in layer, not number of loaded features
     */
    void featuresCountChanged( int featuresCount );

    //! Signal emitted when maximum number of features that can be loaded changes
    void featuresLimitChanged( int featuresLimit );

    //! Signal emitted when filter expression has changed
    void filterExpressionChanged( QString filterExpression );

  private:

    //! Reloads features from layer, if layer is not provided, uses current layer, If layer is provided, saves it as current.
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
    QgsQuickFeatureLayerPairs mFeatures;

    //! Number of maximum features loaded from layer
    const int FEATURES_LIMIT = 10000;

    //! Search string, change of string results in reloading features from mCurrentLayer
    QString mFilterExpression;

    //! Pointer to layer that is currently loaded
    QgsVectorLayer *mCurrentLayer = nullptr;

    //! Field that is used as a "key" in value relation
    QString mKeyField;

    //! Field that represents field used as a feature title, if not set, display expression is used
    QString mFeatureTitleField;
};

#endif // QGSQUICKFEATURESMODEL_H
