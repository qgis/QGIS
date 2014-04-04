/***************************************************************************
   qgsfieldmodel.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFIELDMODEL_H
#define QGSFIELDMODEL_H

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QComboBox>

#include "qgsvectorlayer.h"

/**
 * @brief The QgsFieldModel class is a model to display the list of fields of a layer in widgets.
 * It can be associated with a QgsMapLayerModel to dynamically display a layer and its fields.
 * @note added in 2.3
 */

class GUI_EXPORT QgsFieldModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    enum { FieldNameRole = Qt::UserRole + 1, FieldIndexRole = Qt::UserRole + 2 };

    /**
     * @brief QgsFieldModel creates a model to display the fields of a given layer
     */
    explicit QgsFieldModel( QObject *parent = 0 );

    /**
     * @brief indexFromName returns the index corresponding to a given fieldName
     */
    QModelIndex indexFromName( QString fieldName );

  public slots:
    /**
     * @brief setLayer sets the layer of whch fields are displayed
     */
    void setLayer( QgsMapLayer *layer );

  protected slots:
    void updateFields();
    void layerDeleted();

  protected:
    QgsFields mFields;
    QgsVectorLayer* mLayer;

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent ) const;
    int columnCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;


};

#endif // QGSFIELDMODEL_H
