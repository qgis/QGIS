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
template <class T>
class CORE_EXPORT QgsFeaturePickerModelBase : public QAbstractItemModel
{
    using GathererType = T;
    using IdentifierType = typename GathererType::IdentifierType;

    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull NOTIFY allowNullChanged )
    Q_PROPERTY( bool isLoading READ isLoading NOTIFY isLoadingChanged )

    /**
     * The value that identifies the current feature.
     */
    Q_PROPERTY( IdentifierType extraIdentifierValue READ extraIdentifierValue WRITE setExtraIdentifierValue NOTIFY extraIdentifierValueChanged )

    Q_PROPERTY( int extraIdentifierValueIndex READ extraIdentifierValueIndex NOTIFY extraIdentifierValueIndexChanged )

  public:

    /**
     * Extra roles that can be used to fetch data from this model.
     */
    enum Role
    {
      IdentifierValueRole = Qt::UserRole, //!< \deprecated Use IdentifierValuesRole instead
      IdentifierValuesRole, //!< Used to retrieve the identifierValues (primary keys) of a feature.
      ValueRole, //!< Used to retrieve the displayExpression of a feature.
      FeatureRole, //!< Used to retrieve the feature, it might be incomplete if the request doesn't fetch all attributes or geometry.
      FeatureIdRole //!< Used to retrieve the id of a feature.
    };

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
     * still be available in the model.
     */
    virtual IdentifierType extraIdentifierValue() const {return mExtraIdentifierValue;}

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     */
    virtual void setExtraIdentifierValue( const IdentifierType &extraIdentifierValue ) = 0;

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model as NULL value(s).
     * \since QGIS 3.10
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
    void scheduledReload();

  protected:
    //! Update the request to match the current feature to be reloaded
    virtual void requestToReloadCurrentFeature( QgsFeatureRequest &request ) = 0;

    /**
     * Returns the attributes to be fetched in the request.
     * Returns an empty set if all attributes should be fetched.
     */
    virtual QSet<QString> requestedAttributes() const {return {};}

    virtual T createValuesGatherer( const QgsFeatureRequest &request ) const;

    void setExtraIdentifierValueUnguarded( const IdentifierType &identifierValue );

    //! The current identifier value
    IdentifierType mExtraIdentifierValue;


  private:
    using EntryType = typename GathererType::Entry;

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

    QVector<EntryType> mEntries;

    T *mGatherer = nullptr;

    QTimer mReloadTimer;
    bool mShouldReloadCurrentFeature = false;
    bool mExtraValueDoesNotExist = false;
    bool mAllowNull = false;
    bool mIsSettingExtraIdentifierValue = false;

    //! The current index
    int mExtraValueIndex = -1;

};

#endif // QGSFEATUREFILTERMODELBASE_H
