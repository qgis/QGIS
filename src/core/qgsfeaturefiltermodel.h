/***************************************************************************
  qgsfeaturefiltermodel.h - QgsFeatureFilterModel

 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREFILTERMODEL_H
#define QGSFEATUREFILTERMODEL_H

#include <QAbstractItemModel>

#include "qgsconditionalstyle.h"

class QgsFieldExpressionValuesGatherer;

/**
 * \ingroup core
 * Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeatureFilterModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull NOTIFY allowNullChanged )
    Q_PROPERTY( bool isLoading READ isLoading NOTIFY isLoadingChanged )

    /**
     * A field of sourceLayer that is unique and should be used to identify features.
     * Normally the primary key field.
     * Needs to match the identifierValue.
     */
    Q_PROPERTY( QString identifierField READ identifierField WRITE setIdentifierField NOTIFY identifierFieldChanged )

    /**
     * The value that identifies the current feature.
     */
    Q_PROPERTY( QVariant extraIdentifierValue READ extraIdentifierValue WRITE setExtraIdentifierValue NOTIFY extraIdentifierValueChanged )

    Q_PROPERTY( int extraIdentifierValueIndex READ extraIdentifierValueIndex NOTIFY extraIdentifierValueIndexChanged )

  public:

    /**
     * Extra roles that can be used to fetch data from this model.
     */
    enum Role
    {
      IdentifierValueRole = Qt::UserRole, //!< Used to retrieve the identifierValue (primary key) of a feature.
      ValueRole //!< Used to retrieve the displayExpression of a feature.
    };

    /**
     * Create a new QgsFeatureFilterModel, optionally specifying a \a parent.
     */
    explicit QgsFeatureFilterModel( QObject *parent = nullptr );
    ~QgsFeatureFilterModel() override;

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
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     */
    QString identifierField() const;

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     */
    void setIdentifierField( const QString &identifierField );

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     */
    QVariant extraIdentifierValue() const;

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     */
    void setExtraIdentifierValue( const QVariant &extraIdentifierValue );

    /**
     * The index at which the extra identifier value is available within the model.
     */
    int extraIdentifierValueIndex() const;

    /**
     * Flag indicating that the extraIdentifierValue does not exist in the data.
     */
    bool extraValueDoesNotExist() const;

    /**
     * Add a NULL entry to the list.
     */
    bool allowNull() const;

    /**
     * Add a NULL entry to the list.
     */
    void setAllowNull( bool allowNull );

  signals:

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
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     */
    void identifierFieldChanged();

    /**
     * Indicates that a filter job has been completed and new data may be available.
     */
    void filterJobCompleted();

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     */
    void extraIdentifierValueChanged();

    /**
     * The index at which the extra identifier value is available within the model.
     */
    void extraIdentifierValueIndexChanged( int index );

    /**
     * Flag indicating that the extraIdentifierValue does not exist in the data.
     */
    void extraValueDoesNotExistChanged();

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
    void gathererThreadFinished();
    void scheduledReload();

  private:
    QSet<QString> requestedAttributes() const;
    void setExtraIdentifierValueIndex( int index, bool force = false );
    void setExtraValueDoesNotExist( bool extraValueDoesNotExist );
    void reload();
    void reloadCurrentFeature();
    void setExtraIdentifierValueUnguarded( const QVariant &extraIdentifierValue );
    struct Entry
    {
      Entry() = default;

      Entry( const QVariant &_identifierValue, const QString &_value, const QgsFeature &_feature )
        : identifierValue( _identifierValue )
        , value( _value )
        , feature( _feature )
      {}

      QVariant identifierValue;
      QString value;
      QgsFeature feature;

      bool operator()( const Entry &lhs, const Entry &rhs ) const;
    };

    QgsConditionalStyle featureStyle( const QgsFeature &feature ) const;

    QgsVectorLayer *mSourceLayer = nullptr;
    QgsExpression mDisplayExpression;
    QString mFilterValue;
    QString mFilterExpression;

    mutable QgsExpressionContext mExpressionContext;
    mutable QMap< QgsFeatureId, QgsConditionalStyle > mEntryStylesMap;
    QVector<Entry> mEntries;
    QgsFieldExpressionValuesGatherer *mGatherer = nullptr;
    QTimer mReloadTimer;
    bool mShouldReloadCurrentFeature = false;
    bool mExtraValueDoesNotExist = false;
    bool mAllowNull = false;
    bool mIsSettingExtraIdentifierValue = false;

    QString mIdentifierField;

    QVariant mExtraIdentifierValue;

    int mExtraIdentifierValueIndex = -1;

    friend class QgsFieldExpressionValuesGatherer;
};

#endif // QGSFEATUREFILTERMODEL_H
