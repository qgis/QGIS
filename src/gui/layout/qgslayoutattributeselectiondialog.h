/***************************************************************************
                         qgslayoutattributeselectiondialog.h
                         -----------------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTATTRIBUTESELECTIONDIALOG_H
#define QGSLAYOUTATTRIBUTESELECTIONDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QDialog>
#include <QMap>
#include <QSet>
#include <QItemDelegate>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgslayoutattributeselectiondialogbase.h"
#include "qgsexpressioncontextgenerator.h"

class QGridLayout;
class QgsVectorLayer;
class QPushButton;
class QgsLayoutItemAttributeTable;
class QgsLayoutAttributeTableColumnModel;
class QgsLayoutTableSortModel;
class QgsLayoutTableAvailableSortProxyModel;
class QgsLayoutObject;
class QgsLayoutTableColumn;
class QgsLayoutTableColumn;

/**
 * \ingroup gui
 * \brief A base model to hold the displaying or sorting columns used in a QgsLayoutAttributeTable
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLayoutAttributeTableColumnModelBase: public QAbstractTableModel
{
    Q_OBJECT

  public:

    /**
     * Controls whether a row/column is shifted up or down
     */
    enum ShiftDirection
    {
      ShiftUp, //!< Shift the row/column up
      ShiftDown //!< Shift the row/column down
    };

    /**
     * Available columns for the configuration table to be used by the model
     */
    enum Column
    {
      Attribute, //!< Attribute for a field or an expression
      Heading, //!< Defines the title of the column
      Alignment, //!< Defines the alignment of the column
      Width, //!< Defines the width of the column
      SortOrder //!< Defines the sort order
    };

    /**
     * Constructor for QgsLayoutAttributeTableColumnModel.
     * \param table QgsLayoutItemAttributeTable the model is attached to
     * \param parent optional parent
     */
    QgsLayoutAttributeTableColumnModelBase( QgsLayoutItemAttributeTable *table, QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! To be reimplemented to provide the display or the sort columns
    virtual QVector<QgsLayoutTableColumn> &columns() const = 0;

    //! To be reimplemented to choose which column should be used by the model
    virtual QList<Column> displayedColumns() const = 0;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;

    /**
     * Moves the specified row up or down in the model. Used for rearranging the attribute tables
     * columns.
     * \returns TRUE if the move is allowed
     * \param row row in model representing attribute table column to move
     * \param direction direction to move the attribute table column
     */
    bool moveRow( int row, ShiftDirection direction );

  protected:
    QgsLayoutItemAttributeTable *mTable = nullptr;

};

/**
 * \ingroup gui
 * \brief A model for displaying columns shown in a QgsLayoutAttributeTable
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutAttributeTableColumnModel: public QgsLayoutAttributeTableColumnModelBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutAttributeTableColumnModel.
     * \param table QgsLayoutItemAttributeTable the model is attached to
     * \param parent optional parent
     */
    QgsLayoutAttributeTableColumnModel( QgsLayoutItemAttributeTable *table, QObject *parent SIP_TRANSFERTHIS = nullptr )
      : QgsLayoutAttributeTableColumnModelBase( table, parent )
    {}

    QVector<QgsLayoutTableColumn> &columns() const override;

    QList<Column> displayedColumns() const override
    {
      return {Attribute, Heading, Alignment, Width};
    }

    /**
     * Resets the attribute table's columns to match the source layer's fields. Remove all existing
     * attribute table columns and column customizations.
     */
    void resetToLayer();
};

/**
 * \ingroup gui
 * \brief Allows for filtering QgsComposerAttributeTable columns by columns which are sorted or unsorted
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutTableSortModel: public QgsLayoutAttributeTableColumnModelBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutTableSortColumnsProxyModel.
     * \param table QgsLayoutItemAttributeTable the model is attached to
     * \param parent optional parent
     */
    QgsLayoutTableSortModel( QgsLayoutItemAttributeTable *table, QObject *parent SIP_TRANSFERTHIS = nullptr )
      : QgsLayoutAttributeTableColumnModelBase( table, parent )
    {}

    QVector<QgsLayoutTableColumn> &columns() const override;

    QList<Column> displayedColumns() const override
    {
      return {Attribute, SortOrder};
    }
};

/**
 * \ingroup gui
 * \brief A delegate for showing column alignment as a combo box
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutColumnAlignmentDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutColumnAlignmentDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/**
 * \ingroup gui
 * \brief A delegate for showing column attribute source as a QgsFieldExpressionWidget
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutColumnSourceDelegate : public QItemDelegate, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutColumnSourceDelegate.
     *
     * If \a forceExpressionValues is TRUE then the values generated by the delegate will always be complete
     * QGIS expressions, i.e. field references will be quoted accordingly.
     */
    QgsLayoutColumnSourceDelegate( QgsVectorLayer *vlayer, QObject *parent = nullptr, const QgsLayoutObject *layoutObject = nullptr, bool forceExpressions = false );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
  private slots:
    void commitAndCloseEditor();
  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    const QgsLayoutObject *mLayoutObject = nullptr;
    bool mForceExpressions = false;
    QgsExpressionContext createExpressionContext() const override;
};

/**
 * \ingroup gui
 * \brief A delegate for showing column width as a spin box
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutColumnWidthDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutColumnWidthDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};

/**
 * \ingroup gui
 * \brief A delegate for showing column sort order as a combo box
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutColumnSortOrderDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutColumnSortOrderDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


/**
 * \ingroup gui
 * \brief A dialog to select what attributes to display (in the table item), set the column properties and specify a sort order
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutAttributeSelectionDialog: public QDialog, private Ui::QgsLayoutAttributeSelectionDialogBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsLayoutAttributeSelectionDialog( QgsLayoutItemAttributeTable *table, QgsVectorLayer *vLayer, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

  private slots:
    void mRemoveColumnPushButton_clicked();
    void mAddColumnPushButton_clicked();
    void mColumnUpPushButton_clicked();
    void mColumnDownPushButton_clicked();
    void mResetColumnsPushButton_clicked();
    void mClearColumnsPushButton_clicked();
    void mAddSortColumnPushButton_clicked();
    void mRemoveSortColumnPushButton_clicked();
    void mSortColumnUpPushButton_clicked();
    void mSortColumnDownPushButton_clicked();
    void showHelp();

  private:
    QgsLayoutItemAttributeTable *mTable = nullptr;

    const QgsVectorLayer *mVectorLayer = nullptr;

    QgsLayoutAttributeTableColumnModel *mColumnModel = nullptr;
    QgsLayoutColumnAlignmentDelegate *mColumnAlignmentDelegate = nullptr;
    QgsLayoutColumnSourceDelegate *mColumnSourceDelegate = nullptr;
    QgsLayoutColumnWidthDelegate *mColumnWidthDelegate = nullptr;

    QgsLayoutTableSortModel *mSortColumnModel = nullptr;
    QgsLayoutColumnSourceDelegate *mSortColumnSourceDelegate = nullptr;
    QgsLayoutColumnSortOrderDelegate *mSortColumnOrderDelegate = nullptr;


};

#endif // QGSLAYOUTATTRIBUTESELECTIONDIALOG_H
