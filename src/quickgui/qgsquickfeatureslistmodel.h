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
 * \brief List Model holding features of specific layer.
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
     * Changing search expression does not result in changing this number
     */
    Q_PROPERTY( int featuresCount READ featuresCount NOTIFY featuresCountChanged )

    /**
     * Search expression represents a filter used when querying for data in current layer.
     * Changing this property results in reloading features from current layer with new search expression.
     */
    Q_PROPERTY( QString searchExpression READ searchExpression WRITE setSearchExpression NOTIFY searchExpressionChanged )

    /**
     * Property limiting maximum number of features queried from layer
     * Read only property
     */
    Q_PROPERTY( int featuresLimit READ featuresLimit NOTIFY featuresLimitChanged )

    /**
      * Feature that has opened feature form.
      * This property needs to be set before opening feature form to be able to evaulate filter expressions that contain form scope.
      */
    Q_PROPERTY( QgsFeature currentFeature READ currentFeature WRITE setCurrentFeature NOTIFY currentFeatureChanged )

  public:

    //! Roles for FeaturesListModel
    enum modelRoles
    {
      FeatureTitle = Qt::UserRole + 1,
      FeatureId,
      Feature,
      Description, //! secondary text in list view
      KeyColumn, //! key in value relation
      FoundPair //! pair of attribute and its value by which the feature was found, empty if search expression is empty
    };
    Q_ENUM( modelRoles );

    //! Create features list model
    explicit QgsQuickFeaturesListModel( QObject *parent = nullptr );
    ~QgsQuickFeaturesListModel() override;

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
     * \brief attributeFromValue finds feature with role and value, returns value for requested role
     * \param role role to find from modelRoles
     * \param value value to find
     * \param requestedRole a role whose value is returned
     * \return If feature is found by role and value, method returns value for requested role. Returns empty QVariant if no feature is found. If more features
     * match requested role and value, value for first is returned.
     */
    Q_INVOKABLE QVariant attributeFromValue( const int role, const QVariant &value, const int requestedRole ) const;

    /**
     * \brief convertMultivalueFormat converts postgres string like string to an array of variants with requested role.
     * Array {1,2,3} with requested role FeatureId results in list of QVariant ints [1, 2, 3]
     * \param multivalue string to convert
     * \param requestedRole role to convert keys from string, default value is Qt::DisplayRole
     * \return array of QVariants with values for requested role. If model can not find value for requested role, this key is omitted.
     */
    Q_INVOKABLE QVariant convertMultivalueFormat( const QVariant &multivalue, const int requestedRole = Qt::DisplayRole );

    //! Returns maximum amount of features that can be queried from layer
    int featuresLimit() const;
    //! Returns number of features in layer, not number of loaded features
    int featuresCount() const;
    //! Returns search expression
    QString searchExpression() const;

    /**
     * \brief setSearchExpression Sets search expression, upon setting also reloads features from current layer with new expression
     * \param searchExpression QString to set, empty string represents no filter
     */
    void setSearchExpression( const QString &searchExpression );

    /**
     * \brief setFeatureTitleField Sets name of attribute that will be used for FeatureTitle and Qt::DisplayRole
     * \param attribute Name of attribute to use. If empty, displayExpression will be used.
     */
    void setFeatureTitleField( const QString &attribute );

    //! Sets name of attribute used as "key" in value relation
    void setKeyField( const QString &attribute );

    /**
     * \brief setFilterExpression Sets filter expression for current layer that will be used when querying for data
     * \param filterExpression to be set
     */
    void setFilterExpression( const QString &filterExpression );

    //! Sets current feature property
    void setCurrentFeature( QgsFeature feature );

    //! Gets current feature property
    QgsFeature currentFeature() const;

  signals:

    /**
     * \brief featuresCountChanged Signal emitted when features are reloaded or layer is changed
     * \param featuresCount number of features in layer, not number of loaded features
     */
    void featuresCountChanged( int featuresCount );

    //! Signal emitted when maximum number of features that can be loaded changes
    void featuresLimitChanged( int featuresLimit );

    //! Signal emitted when search expression has changed
    void searchExpressionChanged( QString searchExpression );

    //! Signal emitted when current feature has changed
    void currentFeatureChanged( QgsFeature feature );

  private:

    //! Sets maximum limit and filter expression for request.
    void setupFeatureRequest( QgsFeatureRequest &request );

    //! Reloads features from layer, if layer is not provided, uses current layer, If layer is provided, saves it as current.
    void loadFeaturesFromLayer( QgsVectorLayer *layer = nullptr );

    //! Empty data when resetting model
    void emptyData();

    //! Builds feature title in list
    QVariant featureTitle( const QgsQuickFeatureLayerPair &featurePair ) const;

    //! Builds qgis filter expression from search expression
    QString buildSearchExpression();

    //! Returns found attribute and its value from search expression
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
    QString mSearchExpression;

    //! Contains filter expression of value relation field
    QString mFilterExpression;

    //! Pointer to layer that is currently loaded
    QgsVectorLayer *mCurrentLayer = nullptr;

    //! Pointer to a feature that has currently opened feature form, if null - feature form is not opened
    QgsFeature mCurrentFeature;

    //! Field that is used as a "key" in value relation
    QString mKeyField;

    //! Field that represents field used as a feature title, if not set, display expression is used
    QString mFeatureTitleField;
};

#endif // QGSQUICKFEATURESMODEL_H
