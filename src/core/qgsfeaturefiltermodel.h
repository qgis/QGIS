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

#include "qgsvectorlayer.h"

class QgsFieldExpressionValuesGatherer;

/**
 * Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 */
class CORE_EXPORT QgsFeatureFilterModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
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
    enum Role
    {
      IdentifierValueRole = Qt::UserRole,
      ValueRole
    };

    QgsFeatureFilterModel( QObject *parent = nullptr );
    ~QgsFeatureFilterModel();

    QgsVectorLayer *sourceLayer() const;
    void setSourceLayer( QgsVectorLayer *sourceLayer );

    QString displayExpression() const;
    void setDisplayExpression( const QString &displayExpression );

    QString filterValue() const;
    void setFilterValue( const QString &filterValue );

    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    virtual QModelIndex parent( const QModelIndex &child ) const override;
    virtual int rowCount( const QModelIndex &parent ) const override;
    virtual int columnCount( const QModelIndex &parent ) const override;
    virtual QVariant data( const QModelIndex &index, int role ) const override;

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

    bool isLoading() const;

    QString identifierField() const;
    void setIdentifierField( const QString &identifierField );

    QVariant extraIdentifierValue() const;
    void setExtraIdentifierValue( const QVariant &extraIdentifierValue );

    int extraIdentifierValueIndex() const;

    bool extraValueDoesNotExist() const;

  signals:
    void sourceLayerChanged();
    void displayExpressionChanged();
    void filterValueChanged();
    void filterExpressionChanged();
    void isLoadingChanged();
    void identifierFieldChanged();
    void filterJobCompleted();
    void extraIdentifierValueChanged();
    void extraIdentifierValueIndexChanged( int index );
    void extraValueDoesNotExistChanged();
    void beginUpdate();
    void endUpdate();

  private slots:
    void updateCompleter();
    void gathererThreadFinished();
    void scheduledReload();

  private:
    void setExtraIdentifierValueIndex( int index );
    void setExtraValueDoesNotExist( bool extraValueDoesNotExist );
    void reload();
    void reloadCurrentFeature();
    void setExtraIdentifierValueUnguarded( const QVariant &extraIdentifierValue );
    struct Entry
    {
      Entry()
      {}

      Entry( QVariant _identifierValue, const QString &_value )
        : identifierValue( _identifierValue )
        , value( _value )
      {}

      QVariant identifierValue;
      QString value;

      bool operator()( const Entry &lhs, const Entry &rhs ) const;
    };

    QgsVectorLayer *mSourceLayer;
    QString mDisplayExpression;
    QString mFilterValue;
    QString mFilterExpression;

    QVector<Entry> mEntries;
    QgsFieldExpressionValuesGatherer *mGatherer = nullptr;
    QTimer mReloadTimer;
    bool mShouldReloadCurrentFeature;
    bool mExtraValueDoesNotExist = false;

    QString mIdentifierField;

    QVariant mExtraIdentifierValue;

    int mExtraIdentifierValueIndex = -1;

    friend class QgsFieldExpressionValuesGatherer;
};

#endif // QGSFEATUREFILTERMODEL_H
