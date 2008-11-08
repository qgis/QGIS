/***************************************************************************
                          qgsattributetable.h  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSATTRIBUTETABLE_H
#define QGSATTRIBUTETABLE_H

#include "qgsattributeaction.h"
#include "qgsvectorlayer.h"
#include "qgsfield.h"

#include <QItemDelegate>
#include <QTableWidget>

#include <set>

/**
 *@author Gary E.Sherman
 */

class QgsAttributeTable;

class QgsAttributeTableItemDelegate: public QItemDelegate
{
    Q_OBJECT

  public:
    QgsAttributeTableItemDelegate( QgsAttributeTable *table, QObject * parent = 0 );
    QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

  private:
    QgsAttributeTable *mTable;
};

class QgsAttributeTable : public QTableWidget
{
    Q_OBJECT

  public:
    QgsAttributeTable( QWidget * parent = 0 );
    ~QgsAttributeTable();

    enum
    {
      AttributeIndex = Qt::UserRole,
      AttributeName = Qt::UserRole + 1,
      AttributeType = Qt::UserRole + 2
    };

    void setReadOnly( bool b );
    void setColumnReadOnly( int col, bool ro );

    /* Inserts the feature with the specified id into rowIdMap. This function has to be called
       (e.g. from QgsShapeFileLayer) when a row is inserted into the table */
    void insertFeatureId( int id, int row );
    /**Selects the row which belongs to the feature with the specified id*/
    void selectRowWithId( int id );
    /**Sorts a column. If the first entry contains a letter, sort alphanumerically, otherwise numerically.*/
    void sortColumn( int col, bool ascending );
    /* Use this to give this class the current attribute actions,
       which are used when the user requests a popup menu */
    void setAttributeActions( const QgsAttributeAction& actions )
    { mActions = actions; }
    /**Returns if the table contains uncommited changes*/
    bool edited() const {return mEdited;}
    /**Switches editing mode on and off*/
    void setEditable( bool enabled ) {mEditable = enabled;}

    /** Copies the selected rows to the clipboard
        Deprecated: See QgisApp::editCopy() instead */
    void copySelectedRows();

    /**Swaps the selected rows such that the selected ones are on the top of the table*/
    void bringSelectedToTop();
    /** Selects rows with chosen feature IDs */
    void selectRowsWithId( const QgsFeatureIds& ids, QgsVectorLayer *layer = 0 );
    /** Shows only rows with chosen feature IDs, others get hidden */
    void showRowsWithId( const QgsFeatureIds& ids );
    /** Shows all rows */
    void showAllRows();

    /**Fills the contents of a provider into this table*/
    void fillTable( QgsVectorLayer *layer );

    /** adds a feature to the current table */
    void addFeatureToTable( QgsVectorLayer *layer, int id );

    void addAttribute( int idx, const QgsField &fld );
    void deleteAttribute( int idx );

  public slots:
    void columnClicked( int col );
    void rowClicked( int row );

    // Called when the user chooses an item on the popup menu
    void popupItemSelected( QAction * menuAction );

    void attributeValueChanged( int fid, int idx, const QVariant &value );
    void featureDeleted( int fid );

  protected slots:
    void handleChangedSelections();

  protected:
    /**Flag telling if the ctrl-button or the shift-button is pressed*/
    bool lockKeyPressed;
    /**Search tree to find a row corresponding to a feature id*/
    QMap<int, int> rowIdMap;
    /**Map attribute index to columns*/
    QMap<int, int> mAttrIdxMap;
    bool mEditable;
    /**True if table has been edited and contains uncommited changes*/
    bool mEdited;

    /**Stors the numbers of the last selected rows. This is used to check for selection changes before emit repaintRequested()*/
    std::set<int> mLastSelectedRows;

    /* Compares the content of two cells either alphanumeric or numeric.
       If 'ascending' is true, -1 means s1 is less, 0 equal, 1 greater.
       If 'ascending' is false, -1 means s1 is more, 0 equal, 1 greater.
       This method is used mainly to sort a column*/
    int compareItems( QString s1, QString s2, bool ascending, bool alphanumeric );
    void keyPressEvent( QKeyEvent* ev );
    void keyReleaseEvent( QKeyEvent* ev );
    /**Method used by sortColumn (implementation of a quicksort)*/
    void qsort( int lower, int upper, int col, bool ascending, bool alphanumeric );
    /**Called when the user requests a popup menu*/
    void contextMenuEvent( QContextMenuEvent* event );
    /**Removes the column belonging to an attribute from the table
      @name attribut name*/
    void removeAttrColumn( const QString& name );
    /** puts attributes of feature to the chosen table row */
    void putFeatureInTable( int row, const QgsFeature& fet );
    void mouseReleaseEvent( QMouseEvent* e );
    /**This function compares the current selection and the selection of the last repaint.
       Returns true if there are differences in the selection.
       Also, mLastSelectedRows is updated*/
    bool checkSelectionChanges();

  signals:
    /**Is emitted when a row was selected*/
    void selected( int, bool );
    /**Is emitted when all rows have been deselected*/
    void selectionRemoved( bool );
    /**Is emitted when a set of related selection and deselection signals have been emitted*/
    void repaintRequested();

  private:
    void swapRows( int row1, int row2 );

    // Data to do with providing a popup menu of actions that
    std::vector<std::pair<QString, QString> > mActionValues;
    int mClickedOnValue;
    QMenu* mActionPopup;
    QgsAttributeAction mActions;

    QgsAttributeTableItemDelegate *mDelegate;

    // Track previous columm for QTableView sortIndicator wrong direction workaround
    int mPreviousSortIndicatorColumn;

    QgsVectorLayer *mLayer;
};

#endif
