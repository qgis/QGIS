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
 * If allowed, expressions might be added to the end of the model.
 * It can be associated with a QgsMapLayerModel to dynamically display a layer and its fields.
 * @note added in 2.3
 */
class GUI_EXPORT QgsFieldModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    enum  FieldRoles
    {
      FieldNameRole = Qt::UserRole + 1,
      FieldIndexRole = Qt::UserRole + 2,
      ExpressionRole = Qt::UserRole + 3
    };

    /**
     * @brief QgsFieldModel creates a model to display the fields of a given layer
     */
    explicit QgsFieldModel( QObject *parent = 0 );

    /**
     * @brief indexFromName returns the index corresponding to a given fieldName
     */
    QModelIndex indexFromName( QString fieldName );

    /**
     * @brief setAllowExpression determines if expressions are allowed to be added to the model
     */
    void setAllowExpression( bool allowExpression );
    bool allowExpression() {return mAllowExpression;}

    /**
     * @brief setExpression sets a single expression to be added after the fields at the end of the model
     * @return the model index of the newly added expression
     */
    QModelIndex setExpression( QString expression );

    /**
     * @brief layer returns the currently used layer
     */
    QgsMapLayer* layer() {return mLayer;}

  public slots:
    /**
     * @brief setLayer sets the layer of whch fields are displayed
     */
    void setLayer( QgsMapLayer *layer );

  protected slots:
    virtual void updateModel();

  private slots:
    void layerDeleted();

  protected:
    QgsFields mFields;
    QList<QString> mExpression;

    QgsVectorLayer* mLayer;
    bool mAllowExpression;

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;
};

#endif // QGSFIELDMODEL_H
