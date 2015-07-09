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
    enum FieldRoles
    {
      FieldNameRole = Qt::UserRole + 1,  /*!< return field name if index corresponds to a field */
      FieldIndexRole = Qt::UserRole + 2, /*!< return field index if index corresponds to a field */
      ExpressionRole = Qt::UserRole + 3, /*!< return field name or expression */
      IsExpressionRole = Qt::UserRole + 4, /*!< return if index corresponds to an expression */
      ExpressionValidityRole = Qt::UserRole + 5, /*!< return if expression is valid or not */
      FieldTypeRole = Qt::UserRole + 6 /*!< return the field type (if a field, return QVariant if expression) */
    };

    /**
     * @brief QgsFieldModel creates a model to display the fields of a given layer
     */
    explicit QgsFieldModel( QObject *parent = 0 );

    //! return the index corresponding to a given fieldName
    QModelIndex indexFromName( const QString &fieldName );

    //! returns the currently used layer
    void setAllowExpression( bool allowExpression );
    bool allowExpression() { return mAllowExpression; }

    bool isField( const QString& expression );

    /**
     * @brief setExpression sets a single expression to be added after the fields at the end of the model
     */
    void setExpression( const QString &expression );

    //! remove expressions from the model
    void removeExpression();

    //! returns the currently used layer
    QgsVectorLayer* layer() { return mLayer; }

  public slots:
    //! set the layer of whch fields are displayed
    void setLayer( QgsVectorLayer *layer );

  protected slots:
    virtual void updateModel();

  private slots:
    void layerDeleted();

  protected:
    QgsFields mFields;
    QList<QString> mExpression;

    QgsVectorLayer* mLayer;
    bool mAllowExpression;

  private:
    void fetchFeature();

    // QAbstractItemModel interface
  public:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
};

#endif // QGSFIELDMODEL_H
