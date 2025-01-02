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
#ifndef QGSFEATUREFILTERMODELBASE_H
#define QGSFEATUREFILTERMODELBASE_H

#include "qgsconditionalstyle.h"
#include "qgsfeatureexpressionvaluesgatherer.h"

#include <QAbstractItemModel>
#include <QTimer>

/**
 * \ingroup core
 * \brief Provides a list of features based on filter conditions.
 *
 * Features are fetched asynchronously.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsFeaturePickerModelBase : public QAbstractItemModel SIP_ABSTRACT
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull NOTIFY allowNullChanged )
    Q_PROPERTY( bool fetchGeometry READ fetchGeometry WRITE setFetchGeometry NOTIFY fetchGeometryChanged )
    Q_PROPERTY( int fetchLimit READ fetchLimit WRITE setFetchLimit NOTIFY fetchLimitChanged )
    Q_PROPERTY( int extraIdentifierValueIndex READ extraIdentifierValueIndex NOTIFY extraIdentifierValueIndexChanged )

  public:

    // *INDENT-OFF*

    /**
     * Extra roles that can be used to fetch data from this model.
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeaturePickerModelBase::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsFeaturePickerModelBase, Role ) : int
    {
      IdentifierValue SIP_MONKEYPATCH_COMPAT_NAME(IdentifierValueRole) = Qt::UserRole, //!< Used to retrieve the identifier value (primary key) of a feature. \deprecated QGIS 3.40. Use IdentifierValuesRole instead.
      IdentifierValues SIP_MONKEYPATCH_COMPAT_NAME(IdentifierValuesRole), //!< Used to retrieve the identifierValues (primary keys) of a feature.
      Value SIP_MONKEYPATCH_COMPAT_NAME(ValueRole), //!< Used to retrieve the displayExpression of a feature.
      Feature SIP_MONKEYPATCH_COMPAT_NAME(FeatureRole), //!< Used to retrieve the feature, it might be incomplete if the request doesn't fetch all attributes or geometry.
      FeatureId SIP_MONKEYPATCH_COMPAT_NAME(FeatureIdRole) //!< Used to retrieve the id of a feature.
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Create a new QgsFeaturePickerModelBase, optionally specifying a \a parent.
     */
    explicit QgsFeaturePickerModelBase( QObject *parent = nullptr );
    ~QgsFeaturePickerModelBase() override;

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
     * - displaying values in the combobox
     * - filtering based on filterValue
     */
    QString displayExpression() const;

    /**
     * The display expression will be used for
     *
     * - displaying values in the combobox
     * - filtering based on filterValue
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
    int columnCount( const QModelIndex &parent ) const override
    {
      Q_UNUSED( parent )
      return 1;
    }
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
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model as NULL value(s).
     */
    virtual void setExtraIdentifierValueToNull() = 0;

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

    /**
     * Returns if the geometry is fetched
     */
    bool fetchGeometry() const;

    /**
     * Defines if the geometry will be fetched
     */
    void setFetchGeometry( bool fetchGeometry );

    /**
     * Returns the feature request fetch limit
     */
    int fetchLimit() const;

    /**
     * Defines the feature request fetch limit
     * If set to 0, no limit is applied when fetching
     */
    void setFetchLimit( int fetchLimit );

  signals:

    /**
     * Emitted when the current feature in the model has changed
     * This emitted both when the extra value changes and when the extra value status changes.
     * It allows being notified when the feature is fetched after the extra value has been set.
     * \since QGIS 3.16.5
     */
    void currentFeatureChanged();

    /**
     * The source layer from which features will be fetched.
     */
    void sourceLayerChanged();

    /**
     * The display expression will be used for
     *
     * - displaying values in the combobox
     * - filtering based on filterValue
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
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     */
    void extraIdentifierValueChanged();

    /**
     * The index at which the extra identifier value is available within the model.
     */
    void extraIdentifierValueIndexChanged( int index );

    /**
     * Notification whether the model has \a found a feature tied to the extraIdentifierValue or not.
     */
    void extraValueDoesNotExistChanged( bool found );

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

    /**
     * Emitted when the fetching of the geometry changes
     */
    void fetchGeometryChanged();

    /**
     * Emitted when the fetching limit for the feature request changes
     */
    void fetchLimitChanged();


  private slots:
    void updateCompleter();
    void scheduledReload();

  protected:

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

    //! Update the request to match the current feature to be reloaded
    virtual void requestToReloadCurrentFeature( QgsFeatureRequest &request ) = 0;

    //! This will set the identifier value to be set in the model even if it doesn't exist currently in the data
    void setExtraIdentifierValueUnguarded( const QVariant &identifierValue );

#ifndef SIP_RUN

    /**
     * Returns the attributes to be fetched in the request.
     * Returns an empty set if all attributes should be fetched.
     */
    virtual QSet<QString> requestedAttributes() const {return {};}

    //! Creates the value gatherer
    virtual QgsFeatureExpressionValuesGatherer *createValuesGatherer( const QgsFeatureRequest &request ) const = 0;

    //! Creates an entry with just the identifier so the feature can be retrieved in a next iteration
    virtual QgsFeatureExpressionValuesGatherer::Entry createEntry( const QVariant &identifier ) const = 0;

    //! Returns the identifier of the given entry
    virtual QVariant entryIdentifier( const QgsFeatureExpressionValuesGatherer::Entry &entry ) const = 0;

    //! Returns TRUE if the 2 entries refers to the same feature
    virtual bool compareEntries( const QgsFeatureExpressionValuesGatherer::Entry &a, const QgsFeatureExpressionValuesGatherer::Entry &b ) const = 0;

    //! Returns a null identifier
    virtual QVariant nullIdentifier() const = 0;

    /**
     * Returns TRUE if the entry is null
     * The identifier can be either the feature ID or the list of identifier fields
     */
    virtual bool identifierIsNull( const QVariant &identifier ) const = 0;

    QVector<QgsFeatureExpressionValuesGatherer::Entry> mEntries;
#endif

    //! The current identifier value
    QVariant mExtraIdentifierValue;

    //! The current index
    int mExtraValueIndex = -1;

  private:
    void setExtraIdentifierValueIndex( int index, bool force = false );
    void setExtraValueDoesNotExist( bool extraValueDoesNotExist );
    void reload();
    void reloadCurrentFeature();
    QSet<QString> requestedAttributesForStyle() const;

    QgsConditionalStyle featureStyle( const QgsFeature &feature ) const;

    QgsVectorLayer *mSourceLayer = nullptr;
    QgsExpression mDisplayExpression;
    QString mFilterValue;
    QString mFilterExpression;

    mutable QgsExpressionContext mExpressionContext;
    mutable QMap< QgsFeatureId, QgsConditionalStyle > mEntryStylesMap;

    QgsFeatureExpressionValuesGatherer *mGatherer = nullptr;
    bool mFetchGeometry = true;
    int mFetchLimit = 100;

    QTimer mReloadTimer;
    bool mShouldReloadCurrentFeature = false;
    bool mKeepCurrentEntry = false; // need to keep the current value after a reload or if the value does not exist
    bool mExtraValueDoesNotExist = false;
    bool mAllowNull = false;
    bool mIsSettingExtraIdentifierValue = false;

    friend class TestQgsFeatureListComboBox;
};

#endif // QGSFEATUREFILTERMODELBASE_H
