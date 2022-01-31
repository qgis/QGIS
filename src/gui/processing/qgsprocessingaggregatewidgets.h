/***************************************************************************
  qgsprocessingaggregatewidgets.h
  ---------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGAGGREGATEWIDGETS_H
#define QGSPROCESSINGAGGREGATEWIDGETS_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QPointer>

#include "qgsfields.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsfieldmappingmodel.h"
#include "qgspanelwidget.h"

class QLineEdit;
class QToolButton;
class QItemSelectionModel;
class QTableView;


/**
 * \ingroup gui
 * \brief The QgsAggregateMappingModel holds mapping information for defining sets of aggregates of
 * fields from a QgsFields object.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAggregateMappingModel: public QAbstractTableModel
{

    Q_OBJECT

  public:

    /**
     * The ColumnDataIndex enum represents the column index for the view
     */
    enum class ColumnDataIndex : int
    {
      SourceExpression,       //!< Expression
      Aggregate,              //!< Aggregate name
      Delimiter,              //!< Delimiter
      DestinationName,        //!< Destination field name
      DestinationType,        //!< Destination field type string
      DestinationLength,      //!< Destination field length
      DestinationPrecision,   //!< Destination field precision
    };

    Q_ENUM( ColumnDataIndex );

    /**
     * The Aggregate struct holds information about an aggregate column
     */
    struct Aggregate
    {
      //! The source expression used as the input for the aggregate calculation
      QString source;

      //! Aggregate name
      QString aggregate;

      //! Delimiter string
      QString delimiter;

      //! The field in its current status (it might have been renamed)
      QgsField field;

    };

    /**
     * Constructs a QgsAggregateMappingModel from a set of \a sourceFields.
     * A \a parent object can be also specified.
     */
    QgsAggregateMappingModel( const QgsFields &sourceFields = QgsFields(),
                              QObject *parent = nullptr );

    //! Returns a list of source fields
    QgsFields sourceFields() const;

    //! Returns a list of Aggregate objects representing the current status of the model
    QList<QgsAggregateMappingModel::Aggregate> mapping() const;

    /**
     * Sets the \a mapping to show in the model.
     */
    void setMapping( const QList<QgsAggregateMappingModel::Aggregate> &mapping );

    //! Appends a new \a field to the model, with an optional \a source and \a aggregate
    void appendField( const QgsField &field, const QString &source = QString(), const QString &aggregate = QString() );

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

    // QAbstractItemModel interface
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

  private:

    bool moveUpOrDown( const QModelIndex &index, bool up = true );

    QList<Aggregate> mMapping;
    QgsFields mSourceFields;
    std::unique_ptr< QgsFieldMappingModel::ExpressionContextGenerator> mExpressionContextGenerator;

};

/**
 * \ingroup gui
 * \brief The QgsAggregateMappingWidget class creates a mapping for defining sets of aggregates of
 * fields from a QgsFields object.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAggregateMappingWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructs a QgsAggregateMappingWidget from a set of \a sourceFields. A \a parent object
     * can also be specified.
     */
    explicit QgsAggregateMappingWidget( QWidget *parent = nullptr,
                                        const QgsFields &sourceFields = QgsFields() );

    //! Returns the underlying mapping model
    QgsAggregateMappingModel *model() const;

    //! Returns a list of Aggregate objects representing the current status of the underlying mapping model
    QList<QgsAggregateMappingModel::Aggregate> mapping() const;

    /**
     * Sets the \a mapping to show in the model.
     */
    void setMapping( const QList<QgsAggregateMappingModel::Aggregate> &mapping );

    //! Returns the selection model
    QItemSelectionModel *selectionModel();

    //! Set source fields of the underlying mapping model to \a sourceFields
    void setSourceFields( const QgsFields &sourceFields );

    /**
     * Sets a source \a layer to use when generating expression previews in the widget.
     *
     * \since QGIS 3.16
     */
    void setSourceLayer( QgsVectorLayer *layer );

    /**
     * Returns the source layer for use when generating expression previews.
     *
     * Returned value may be NULLPTR.
     *
     * \since QGIS 3.16
     */
    QgsVectorLayer *sourceLayer();

    /**
     * Scroll the fields view to \a index
     */
    void scrollTo( const QModelIndex &index ) const;

    /**
     * Register an expression context \a generator class that will be used to retrieve
     * an expression context for the widget.
     */
    void registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator );

  signals:

    /**
     *Emitted when the aggregates defined in the widget are changed.
     */
    void changed();

  public slots:

    //! Appends a new \a field to the model, with an optional \a source and \a aggregate
    void appendField( const QgsField &field, const QString &source = QString(), const QString &aggregate = QString() );

    //! Removes the currently selected field from the model
    bool removeSelectedFields( );

    //! Moves up currently selected field
    bool moveSelectedFieldsUp( );

    //! Moves down currently selected field
    bool moveSelectedFieldsDown( );

  private:

    QTableView *mTableView = nullptr;
    QAbstractTableModel *mModel = nullptr;
    QPointer< QgsVectorLayer > mSourceLayer;
    void updateColumns();
    //! Returns selected row indexes in ascending order
    std::list<int> selectedRows( );


    class AggregateDelegate: public QStyledItemDelegate
    {

      public:

        AggregateDelegate( QObject *parent = nullptr );

        // QAbstractItemDelegate interface
        QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
        void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
        void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

      private:
        //! Returns a static list of supported aggregates
        static const QStringList aggregates();
    };

};


#endif // QGSPROCESSINGAGGREGATEWIDGETS_H
