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
#include <QComboBox>
#include <QItemSelectionModel>

#include "qgsfields.h"
#include "qgis_core.h"

#include "qgis_sip.h"

class QgsVectorLayer;

/**
 * \ingroup core
 * \brief The QgsFieldModel class is a model to display the list of fields of a layer in widgets.
 * If allowed, expressions might be added to the end of the model.
 * It can be associated with a QgsMapLayerModel to dynamically display a layer and its fields.
 * \since QGIS 2.3
 */
class CORE_EXPORT QgsFieldModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY( bool allowExpression READ allowExpression WRITE setAllowExpression )
    Q_PROPERTY( bool allowEmptyFieldName READ allowEmptyFieldName WRITE setAllowEmptyFieldName )
    Q_PROPERTY( QgsVectorLayer *layer READ layer WRITE setLayer )

  public:

    //! Roles utilized by the model
    enum FieldRoles
    {
      FieldNameRole = Qt::UserRole + 1,  //!< Return field name if index corresponds to a field
      FieldIndexRole = Qt::UserRole + 2, //!< Return field index if index corresponds to a field
      ExpressionRole = Qt::UserRole + 3, //!< Return field name or expression
      IsExpressionRole = Qt::UserRole + 4, //!< Return if index corresponds to an expression
      ExpressionValidityRole = Qt::UserRole + 5, //!< Return if expression is valid or not
      FieldTypeRole = Qt::UserRole + 6, //!< Return the field type (if a field, return QVariant if expression)
      FieldOriginRole = Qt::UserRole + 7, //!< Return the field origin (if a field, returns QVariant if expression)
      IsEmptyRole = Qt::UserRole + 8, //!< Return if the index corresponds to the empty value
    };

    /**
     * Constructor for QgsFieldModel - creates a model to display the fields of a given layer.
     */
    explicit QgsFieldModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the index corresponding to a given fieldName.
     */
    QModelIndex indexFromName( const QString &fieldName );

    /**
     * Sets whether custom expressions are accepted and displayed in the model.
     * \see allowExpression()
     * \see setExpression()
     */
    void setAllowExpression( bool allowExpression );

    /**
     * Returns TRUE if the model allows custom expressions to be created and displayed.
     * \see setAllowExpression()
     */
    bool allowExpression() { return mAllowExpression; }

    /**
     * Sets whether an optional empty field ("not set") option is present in the model.
     * \see allowEmptyFieldName()
     * \since QGIS 3.0
     */
    void setAllowEmptyFieldName( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty field ("not set") choice.
     * \see setAllowEmptyFieldName()
     * \since QGIS 3.0
     */
    bool allowEmptyFieldName() const { return mAllowEmpty; }

    /**
     * Returns TRUE if a string represents a field reference, or FALSE if it is an
     * expression consisting of more than direct field reference.
     */
    bool isField( const QString &expression ) const;

    /**
     * Sets a single expression to be added after the fields at the end of the model.
     * \see setAllowExpression()
     * \see allowExpression()
     * \see removeExpression()
     */
    void setExpression( const QString &expression );

    /**
     * Removes any custom expression from the model.
     * \see setExpression()
     * \see allowExpression()
     */
    void removeExpression();

    /**
     * Returns the layer associated with the model.
     * \see setLayer()
     */
    QgsVectorLayer *layer() { return mLayer; }

    // QAbstractItemModel interface
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * Returns a HTML formatted tooltip string for a \a field, containing details
     * like the field name, alias and type.
     * \since QGIS 3.0
     */
    static QString fieldToolTip( const QgsField &field );

  public slots:

    /**
     * Set the layer from which fields are displayed.
     * \see layer()
     */
    void setLayer( QgsVectorLayer *layer );

  protected slots:

    /**
     * Called when the model must be updated.
     */
    virtual void updateModel();

  private slots:
    void layerDeleted();

  protected:
    QgsFields mFields;
    QList<QString> mExpression;

    QgsVectorLayer *mLayer = nullptr;
    bool mAllowExpression = false;
    bool mAllowEmpty = false;

  private:
    void fetchFeature();
};

#endif // QGSFIELDMODEL_H
