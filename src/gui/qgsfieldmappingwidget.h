/***************************************************************************
  qgsfieldmappingwidget.h - QgsFieldMappingWidget

 ---------------------
 begin                : 16.3.2020
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
#ifndef QGSFIELDMAPPINGWIDGET_H
#define QGSFIELDMAPPINGWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QPointer>

#include "qgis_gui.h"
#include "qgsfieldmappingmodel.h"
#include "qgspanelwidget.h"

class QTableView;
class QItemSelectionModel;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief The QgsFieldMappingWidget class creates a mapping from one set of QgsFields to another,
 * for each set of "destination" fields an expression defines how to obtain the values of the
 * "destination" fields.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFieldMappingWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructs a QgsFieldMappingWidget from a set of \a sourceFields
     * and \a destinationFields, initial values for the expressions can be
     * optionally specified through \a expressions which is a map from the original
     * field name to the corresponding expression. A \a parent object
     * can also be specified.
     */
    explicit QgsFieldMappingWidget( QWidget *parent = nullptr,
                                    const QgsFields &sourceFields = QgsFields(),
                                    const QgsFields &destinationFields = QgsFields(),
                                    const QMap<QString, QString> &expressions = QMap<QString, QString>() );

    //! Sets the destination fields editable state to \a editable
    void setDestinationEditable( bool editable );

    //! Returns TRUE if the destination fields are editable in the model
    bool destinationEditable() const;

    //! Returns the underlying mapping model
    QgsFieldMappingModel *model() const;

    //! Returns a list of Field objects representing the current status of the underlying mapping model
    QList<QgsFieldMappingModel::Field> mapping() const;

    /**
     * Returns a map of destination field name to QgsProperty definition for field value,
     * representing the current status of the widget.
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
     * Set destination fields to \a destinationFields in the underlying model,
     * initial values for the expressions can be optionally specified through
     * \a expressions which is a map from the original field name to the
     * corresponding expression.
     */
    void setDestinationFields( const QgsFields &destinationFields,
                               const QMap<QString, QString> &expressions = QMap<QString, QString>() );

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
     *Emitted when the fields defined in the widget are changed.
     */
    void changed();

  public slots:

    //! Appends a new \a field to the model, with an optional \a expression
    void appendField( const QgsField &field, const QString &expression = QString() );

    //! Removes the currently selected field from the model
    bool removeSelectedFields( );

    //! Moves up currently selected field
    bool moveSelectedFieldsUp( );

    //! Moves down the currently selected field
    bool moveSelectedFieldsDown( );

  private:

    QTableView *mTableView = nullptr;
    QAbstractTableModel *mModel = nullptr;

    QPointer< QgsVectorLayer > mSourceLayer;
    void updateColumns();
    //! Returns selected row indexes in ascending order
    std::list<int> selectedRows( );

    friend class QgsAggregateMappingWidget;

};

/// @cond PRIVATE

#ifndef SIP_RUN

class QgsFieldMappingExpressionDelegate: public QStyledItemDelegate
{
    Q_OBJECT

  public:

    QgsFieldMappingExpressionDelegate( QObject *parent = nullptr );

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

class QgsFieldMappingTypeDelegate: public QStyledItemDelegate
{
    Q_OBJECT

  public:

    QgsFieldMappingTypeDelegate( QObject *parent = nullptr );

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

#endif

/// @endcond

#endif // QGSFIELDMAPPINGWIDGET_H
