/***************************************************************************
  qgsquickvaluerelationlistmodel.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKVALUERELATIONLISTMODEL_H
#define QGSQUICKVALUERELATIONLISTMODEL_H

#include "qgis_quick.h"

#include <QAbstractListModel>

#include "qgsvaluerelationfieldformatter.h"

/**
 * \ingroup quick
 * List model for combo box in Value Relation widget.
 * The model keeps a list of key-value pairs fetched from a vector layer. "Values" are the human readable
 * descriptions (QString), "keys" are unique identifiers of items (QVariant).
 *
 * \note QML Type: ValueRelationListModel
 *
 * \since QGIS 3.14
 */
class QUICK_EXPORT QgsQuickValueRelationListModel : public QAbstractListModel
{
    Q_OBJECT
  public:
    //! Constructs an empty list model
    QgsQuickValueRelationListModel( QObject *parent = nullptr );

    //! Populates the model from vector layer's widget configuration
    Q_INVOKABLE void populate( const QVariantMap &config, const QgsFeature &formFeature = QgsFeature() );

    //! Returns key for the given rown number (invalid variant if outside of the valid range)
    Q_INVOKABLE QVariant keyForRow( int row ) const;
    //! Returns row number
    Q_INVOKABLE int rowForKey( const QVariant &key ) const;

    int rowCount( const QModelIndex & ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  private:
    QgsValueRelationFieldFormatter::ValueRelationCache mCache;
};

#endif // QGSQUICKVALUERELATIONLISTMODEL_H
