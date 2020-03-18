/***************************************************************************
  qgsfieldmappingmodel.h - QgsFieldMappingModel

 ---------------------
 begin                : 17.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDMAPPINGMODEL_H
#define QGSFIELDMAPPINGMODEL_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>

#include "qgsfields.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgis_gui.h"


/**
 * \ingroup gui
 * The QgsFieldMappingModel holds mapping information for mapping from one set of QgsFields to another,
 * for each set of "destination" fields an expression defines how to obtain the values of the
 * "destination" fields.
 * The model can be optionally set "editable" allowing to modify all the fields, by default only
 * the mapping expression is editable.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFieldMappingModel: public QAbstractTableModel
{

    Q_OBJECT

  public:

    /**
     * The ColumnDataIndex enum represents the column index for the view
     */
    enum ColumnDataIndex
    {
      SourceExpression,     //!< Expression
      DestinationName,      //!< Destination field name
      DestinationType,      //!< Destination field QVariant::Type casted to (int)
      DestinationLength,    //!< Destination field length
      DestinationPrecision  //!< Destination field precision
    };

    /**
     * The Field struct holds information about a mapped field
     */
    struct Field
    {
      //! The original name of the field
      QString originalName;
      //! The field in its current status (it might have been renamed)
      QgsField field;
      //! The expression for the mapped field from the source fields
      QgsExpression expression;
    };

    /**
     * Constructs a QgsFieldMappingModel from a set of \a sourceFields
     * and \a destinationFields, initial values for the expressions can be
     * optionally specified through \a expressions which is a map from the original
     * field name to the corresponding expression. A \a parent object
     * can be also specified.
     */
    QgsFieldMappingModel( const QgsFields &sourceFields = QgsFields(),
                          const QgsFields &destinationFields = QgsFields(),
                          const QMap<QString, QgsExpression> &expressions = QMap<QString, QgsExpression>(),
                          QObject *parent = nullptr );

    //! Returns the context generator with the source fields
    QgsExpressionContextGenerator *contextGenerator() const;

    //! Returns TRUE if the destination fields are editable
    bool destinationEditable() const;

    //! Sets the destination fields editable state to \a editable
    void setDestinationEditable( bool editable );

    //! Returns a static map of supported data types
    const QMap<QVariant::Type, QString> dataTypes() const;

    //! Returns a list of source fields
    QgsFields sourceFields() const;

    //! Returns a list of Field objects representing the current status of the model
    QList<QgsFieldMappingModel::Field> mapping() const;

    //! Appends a new \a field to the model, with an optional \a expression
    void appendField( const QgsField &field, const QgsExpression &expression = QgsExpression() );

    //! Removes the field at \a index from the model, returns TRUE on success
    bool removeField( const QModelIndex &index );

    //! Moves down the field at \a index
    bool moveUp( const QModelIndex &index );

    //! Moves up the field at \a index
    bool moveDown( const QModelIndex &index );

    //! Set source fields to \a sourceFields
    void setSourceFields( const QgsFields &sourceFields );

    /**
     * Set destination fields to \a destinationFields, initial values for the expressions can be
     * optionally specified through \a expressions which is a map from the original
     * field name to the corresponding expression.
     */
    void setDestinationFields( const QgsFields &destinationFields,
                               const QMap<QString, QgsExpression> &expressions = QMap<QString, QgsExpression>() );

    // QAbstractItemModel interface
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

  private:

    class ExpressionContextGenerator: public QgsExpressionContextGenerator
    {

      public:

        ExpressionContextGenerator( const QgsFields *sourceFields );

        // QgsExpressionContextGenerator interface
        QgsExpressionContext createExpressionContext() const override;

      private:

        const QgsFields *mSourceFields;

    };

    bool moveUpOrDown( const QModelIndex &index, bool up = true );

    QList<Field> mMapping;
    bool mDestinationEditable = false;
    QgsFields mSourceFields;
    std::unique_ptr<ExpressionContextGenerator> mExpressionContextGenerator;
};

#endif // QGSFIELDMAPPINGMODEL_H
