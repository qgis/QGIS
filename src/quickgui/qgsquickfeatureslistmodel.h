#ifndef QGSQUICKFEATURESMODEL_H
#define QGSQUICKFEATURESMODEL_H

#include <QAbstractListModel>

#include "qgsvectorlayer.h"
#include "qgsquickfeaturelayerpair.h"
#include "qgsvaluerelationfieldformatter.h"

class QUICK_EXPORT QgsQuickFeaturesListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY( int featuresCount READ featuresCount NOTIFY featuresCountChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( int featuresLimit READ featuresLimit NOTIFY featuresLimitChanged )
    Q_PROPERTY( modelTypes modelType READ modelType WRITE setModelType )

    enum roleNames
    {
      FeatureTitle = Qt::UserRole + 1,
      FeatureId,
      Feature,
      Description, // secondary text in list view
      EmitableIndex, // key in value relation
      FoundPair // pair of attribute and its value by which the feature was found, empty if mFilterExpression is empty
    };

  public:

    enum modelTypes
    {
      FeatureListing,
      ValueRelation
    };
    Q_ENUM( modelTypes );

    explicit QgsQuickFeaturesListModel( QObject *parent = nullptr );
    ~QgsQuickFeaturesListModel() override {};

    //! Function to get QgsQuickFeatureLayerPair by feature id
    Q_INVOKABLE QgsQuickFeatureLayerPair featureLayerPair( const int &featureId );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QHash<int, QByteArray> roleNames() const override;

    //! Features count represents real number of features in layer being browsed
    int featuresCount() const;

    QString filterExpression() const;
    void setFilterExpression( const QString &filterExpression );

    int featuresLimit() const;

    Q_INVOKABLE void populate( const QVariantMap &config );

    Q_INVOKABLE void populateFromLayer( QgsVectorLayer *layer );

    Q_INVOKABLE void loadFeaturesFromLayer( QgsVectorLayer *layer = nullptr );

    //! Returns row number
    Q_INVOKABLE int rowIndexFromKey( const QVariant &key ) const;

    Q_INVOKABLE int rowIndexFromKeyModel( const QVariant &key ) const;

    modelTypes modelType() const;

  public slots:
    void setModelType( modelTypes modelType );

  signals:
    void featuresCountChanged( int featuresCount );
    void featuresLimitChanged( int featuresLimit );
    void filterExpressionChanged( QString filterExpression );

  private:
    //! Empty data when changing map theme or project
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

    //! Search string, change of string results in reloading features from layer with this text
    QString mFilterExpression;

    //! Pointer to layer that is currently parsed
    QgsVectorLayer *mCurrentLayer = nullptr;

    //! Data from config for value relations
    QgsValueRelationFieldFormatter::ValueRelationCache mCache;

    //! Type of a model - Listing (browsing) features or use in Value relation widget
    modelTypes mModelType;

    QString mKeyFieldName;
};

#endif // QGSQUICKFEATURESMODEL_H
