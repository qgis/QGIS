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
#include "qgsexpressioncontextgenerator.h"
#include "qgsfieldconstraints.h"
#include "qgsproperty.h"
#include "qgsvectordataprovider.h"
#include "qgis_gui.h"


/**
 * \ingroup gui
 * \brief The QgsFieldMappingModel holds mapping information for mapping from one set of QgsFields to another,
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
    enum class ColumnDataIndex : int
    {
      SourceExpression,       //!< Expression
      DestinationName,        //!< Destination field name
      DestinationType,        //!< Destination field type string
      DestinationLength,      //!< Destination field length
      DestinationPrecision,   //!< Destination field precision
      DestinationConstraints, //!< Destination field constraints
    };

    Q_ENUM( ColumnDataIndex );

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
      QString expression;
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
                          const QMap<QString, QString> &expressions = QMap<QString, QString>(),
                          QObject *parent = nullptr );

    //! Returns TRUE if the destination fields are editable
    bool destinationEditable() const;

    //! Sets the destination fields editable state to \a editable
    void setDestinationEditable( bool editable );

    /**
     * Returns a static map of supported data types
     * \deprecated QGIS 3.24 use supportedDataTypes() instead
     */
    Q_DECL_DEPRECATED static const QMap<QVariant::Type, QString> dataTypes();

    /**
     * Returns a static list of supported data types
     * \since QGIS 3.24
     */
    static const QList<QgsVectorDataProvider::NativeType> supportedDataTypes();

    //! Returns a list of source fields
    QgsFields sourceFields() const;

    //! Returns a list of Field objects representing the current status of the model
    QList<QgsFieldMappingModel::Field> mapping() const;

    /**
     * Returns a map of destination field name to QgsProperty definition for field value,
     * representing the current status of the model.
     *
     * \see setFieldPropertyMap()
     */
    QMap< QString, QgsProperty > fieldPropertyMap() const;

    /**
     * Sets a map of destination field name to QgsProperty definition for field value.
     *
     * \see fieldPropertyMap()
     */
    void setFieldPropertyMap( const QMap< QString, QgsProperty > &map );

    //! Appends a new \a field to the model, with an optional \a expression
    void appendField( const QgsField &field, const QString &expression = QString() );

    //! Removes the field at \a index from the model, returns TRUE on success
    bool removeField( const QModelIndex &index );

    //! Moves down the field at \a index
    bool moveUp( const QModelIndex &index );

    //! Moves up the field at \a index
    bool moveDown( const QModelIndex &index );

    //! Set source fields to \a sourceFields
    void setSourceFields( const QgsFields &sourceFields );

    //! Returns the context generator with the source fields
    QgsExpressionContextGenerator *contextGenerator() const;

    /**
     * Sets the base expression context \a generator, which will generate the expression
     * contexts for expression based widgets used by the model.
     */
    void setBaseExpressionContextGenerator( const QgsExpressionContextGenerator *generator );

    /**
     * Set destination fields to \a destinationFields, initial values for the expressions can be
     * optionally specified through \a expressions which is a map from the original
     * field name to the corresponding expression.
     */
    void setDestinationFields( const QgsFields &destinationFields,
                               const QMap<QString, QString> &expressions = QMap<QString, QString>() );


    // QAbstractItemModel interface
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

  private:

    class ExpressionContextGenerator: public QgsExpressionContextGenerator
    {

      public:

        ExpressionContextGenerator( const QgsFields &sourceFields );

        // QgsExpressionContextGenerator interface
        QgsExpressionContext createExpressionContext() const override;
        void setBaseExpressionContextGenerator( const QgsExpressionContextGenerator *generator );
        void setSourceFields( const QgsFields &fields );

      private:

        const QgsExpressionContextGenerator *mBaseGenerator = nullptr;

        QgsFields mSourceFields;

    };


    QgsFieldConstraints::Constraints fieldConstraints( const QgsField &field ) const;

    /**
     * Returns the field type name matching the \a field settings.
     * \since QGIS 3.24
     */
    static const QString qgsFieldToTypeName( const QgsField &field );

    /**
     * Sets the \a field type and subtype based on the type \a name provided.
     * \since QGIS 3.24
     */
    static void setFieldTypeFromName( QgsField &field, const QString &name );

    bool moveUpOrDown( const QModelIndex &index, bool up = true );

    /**
     * Try to find the best expression for a destination \a field by searching in the
     * source fields for fields with:
     *
     * - the same name
     * - the same type
     *
     * Returns an expression containing a reference to the field that matches first.
     */
    QString findExpressionForDestinationField( const QgsFieldMappingModel::Field &field, QStringList &excludedFieldNames );

    QList<Field> mMapping;
    bool mDestinationEditable = false;
    QgsFields mSourceFields;
    std::unique_ptr<ExpressionContextGenerator> mExpressionContextGenerator;

    friend class QgsAggregateMappingModel;

};



#endif // QGSFIELDMAPPINGMODEL_H
