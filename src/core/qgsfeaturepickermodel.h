/***************************************************************************
  qgsfeaturepickermodel.h - QgsFeaturePickerModel
 ---------------------
 begin                : 03.04.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREPICKERMODEL_H
#define QGSFEATUREPICKERMODEL_H

#include <QAbstractItemModel>

#include "qgsconditionalstyle.h"
#include "qgsfeatureexpressionvaluesgatherer.h"

/**
 * \ingroup core
 * Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeaturePickerModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull NOTIFY allowNullChanged )
    Q_PROPERTY( bool isLoading READ isLoading NOTIFY isLoadingChanged )

  public:

    /**
     * Extra roles that can be used to fetch data from this model.
     */
    enum Role
    {
      ValueRole = Qt::UserRole + 1, //!< Used to retrieve the displayExpression of a feature.
      FeatureRole, //!< Used to retrieve the feature.
      FeatureIdRole //!< Used to retrieve the id of a feature.
    };

    /**
     * Create a new QgsFeaturePickerModel, optionally specifying a \a parent.
     */
    explicit QgsFeaturePickerModel( QObject *parent = nullptr );
    ~QgsFeaturePickerModel() override;

    /**
     * The source layer from which features will be fetched.
     */
    QgsVectorLayer *sourceLayer() const;

    /**
     * The source layer from which features will be fetched.
     */
    void setSourceLayer( QgsVectorLayer *sourceLayer );

    /**
     * The display expression will be used for
     *
     *  - displaying values in the combobox
     *  - filtering based on filterValue
     */
    QString displayExpression() const;

    /**
     * The display expression will be used for
     *
     *  - displaying values in the combobox
     *  - filtering based on filterValue
     */
    void setDisplayExpression( const QString &displayExpression );

    /**
     * This value will be used to filter the features available from
     * this model. Whenever a substring of the displayExpression of a feature
     * matches the filter value, it will be accessible by this model.
     */
    QString filterValue() const;

    /**
     * This value will be used to filter the features available from
     * this model. Whenever a substring of the displayExpression of a feature
     * matches the filter value, it will be accessible by this model.
     */
    void setFilterValue( const QString &filterValue );

    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * An additional filter expression to apply, next to the filterValue.
     * Can be used for spatial filtering etc.
     */
    QString filterExpression() const;

    /**
     * An additional filter expression to apply, next to the filterValue.
     * Can be used for spatial filtering etc.
     */
    void setFilterExpression( const QString &filterExpression );

    /**
     * Indicator if the model is currently performing any feature iteration in the background.
     */
    bool isLoading() const;

    /**
     * Add a NULL entry to the list.
     */
    bool allowNull() const;

    /**
     * Add a NULL entry to the list.
     */
    void setAllowNull( bool allowNull );

    QgsFeature currentFeature() const;

    /**
     * Sets the current feature id
     */
    void setCurrentFeature( const QgsFeatureId &featureId );

    /**
     * Returns the current index
     */
    int currentIndex() const {return mCurrentIndex;}

  signals:

    /**
     * Emitted when the current index changed
     */
    void currentIndexChanged( int index );

    /**
     * Emitted when the current feature changed
     */
    void currentFeatureChanged( const QgsFeature &feature );

    /**
     * The source layer from which features will be fetched.
     */
    void sourceLayerChanged();

    /**
     * The display expression will be used for
     *
     *  - displaying values in the combobox
     *  - filtering based on filterValue
     */
    void displayExpressionChanged();

    /**
     * This value will be used to filter the features available from
     * this model. Whenever a substring of the displayExpression of a feature
     * matches the filter value, it will be accessible by this model.
     */
    void filterValueChanged();

    /**
     * An additional filter expression to apply, next to the filterValue.
     * Can be used for spatial filtering etc.
     */
    void filterExpressionChanged();

    /**
     * Indicator if the model is currently performing any feature iteration in the background.
     */
    void isLoadingChanged();

    /**
     * Indicates that a filter job has been completed and new data may be available.
     */
    void filterJobCompleted();

    /**
     * Notification that the model is about to be changed because a job was completed.
     */
    void beginUpdate();

    /**
     * Notification that the model change is finished. Will always be emitted in sync with beginUpdate.
     */
    void endUpdate();

    /**
     * Add a NULL entry to the list.
     */
    void allowNullChanged();

  private slots:
    void updateCompleter();
    void scheduledReload();

  private:
    void setCurrentIndex( int index, bool force = false );
    void reload();
    void setCurrentFeatureUnguarded( const QgsFeatureId &featureId );
    QgsConditionalStyle featureStyle( const QgsFeature &feature ) const;

    QgsVectorLayer *mSourceLayer = nullptr;
    QgsExpression mDisplayExpression;
    QString mFilterValue;
    QString mFilterExpression;

    mutable QgsExpressionContext mExpressionContext;
    mutable QMap< QgsFeatureId, QgsConditionalStyle > mEntryStylesMap;
    QVector<QgsFeatureExpressionValuesGatherer::Entry> mEntries;
    QgsFeatureExpressionValuesGatherer *mGatherer = nullptr;
    QTimer mReloadTimer;
    bool mAllowNull = false;

    int mCurrentIndex = -1;
    bool mIsSettingCurrentFeature = false;

};

#endif // QGSFEATUREPICKERMODEL_H
