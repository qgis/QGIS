/***************************************************************************
    qgssingleitemmodel.h
    ---------------
    begin                : May 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLEITEMMODEL_H
#define QGSSINGLEITEMMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QAbstractItemModel>

/**
 * \ingroup core
 * \class QgsSingleItemModel
 *
 * \brief A QgsSingleItemModel subclass which contains a single read-only item.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsSingleItemModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSingleItemModel with the specified \a parent object and display \a text.
     *
     * The \a data map specifies the data which should be returned for the specified roles, where
     * map keys are Qt item roles.
     *
     * Custom \a flags can also be specified for the item.
     */
    explicit QgsSingleItemModel( QObject *parent SIP_TRANSFERTHIS = nullptr, const QString &text = QString(),
                                 const QMap< int, QVariant > &data = QMap< int, QVariant >(), Qt::ItemFlags flags = Qt::NoItemFlags );

    /**
     * Constructor for a multi-column QgsSingleItemModel with the specified \a parent object.
     *
     * The \a columnData list specifies the data which should be returned for the specified roles for each column in the model, where
     * each entry in the list must be a QMap of Qt item role to value.
     *
     * Custom \a flags can also be specified for the item.
     */
    explicit QgsSingleItemModel( QObject *parent SIP_TRANSFERTHIS,
                                 const QList< QMap< int, QVariant > > &columnData,
                                 Qt::ItemFlags flags = Qt::NoItemFlags );

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

  private:

    QString mText;
    QMap< int, QVariant > mData;
    QList< QMap< int, QVariant > > mColumnData;
    Qt::ItemFlags mFlags = Qt::NoItemFlags;
};

#endif //QGSSINGLEITEMMODEL_H
