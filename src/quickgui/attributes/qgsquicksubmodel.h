/***************************************************************************
 qgsquicksubmodel.h
  --------------------------------------
  Date                 : 16.9.2016
  Copyright            : (C) 2016 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSQUICKSUBMODEL_H
#define QGSQUICKSUBMODEL_H

#include "qgis_quick.h"
#include "qgsquickattributeformmodel.h"
#include <QAbstractItemModel>

/**
 * \ingroup quick
 *
 * Helper class for submodels (e.g. tabs within feature model).
 *
 * It uses internal mapping from internal indexes to indexes in the parent model.
 *
 * \note QML Type: SubModel
 *
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickSubModel : public QAbstractItemModel
{
    Q_OBJECT

    //! Parent model (e.g QgsQuickAttributeFormModel)
    Q_PROPERTY( QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged )

    //! Root index of parent model
    Q_PROPERTY( QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged )

  public:
    //! Creates new sub model
    QgsQuickSubModel( QObject *parent = nullptr );

    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    QHash<int, QByteArray> roleNames() const override;

    //! \copydoc QgsQuickSubModel::model
    QAbstractItemModel *model() const;

    //! \copydoc QgsQuickSubModel::model
    void setModel( QAbstractItemModel *model );

    //! \copydoc QgsQuickSubModel::rootIndex
    QModelIndex rootIndex() const;

    //! \copydoc QgsQuickSubModel::rootIndex
    void setRootIndex( const QModelIndex &rootIndex );

  signals:
    //! \copydoc QgsQuickSubModel::model
    void modelChanged();

    //! \copydoc QgsQuickSubModel::rootIndex
    void rootIndexChanged();

  private slots:
    void onRowsAboutToBeInserted( const QModelIndex &parent, int first, int last );
    void onRowsInserted( const QModelIndex &parent, int first, int last );
    void onRowsAboutToBeRemoved( const QModelIndex &parent, int first, int last );
    void onRowsRemoved( const QModelIndex &parent, int first, int last );
    void onModelAboutToBeReset();
    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>() );

  private:
    QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const;
    QModelIndex mapToSource( const QModelIndex &index ) const;

    QAbstractItemModel *mModel = nullptr; // not owned
    QPersistentModelIndex mRootIndex;

    // Map internal id to parent index
    mutable QHash<qintptr, QModelIndex> mMappings;
};

#endif // QGSQUICKSUBMODEL_H
