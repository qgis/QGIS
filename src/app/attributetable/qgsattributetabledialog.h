/***************************************************************************
  QgsAttributeTableDialog.h - dialog for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEDIALOG_H_
#define QGSATTRIBUTETABLEDIALOG_H_

#include <QDialog>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QMutex>

#include "ui_qgsattributetabledialog.h"
#include "qgscontexthelp.h"

#include "qgsvectorlayer.h" //QgsFeatureIds

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QDockWidget;

class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;
class QgsAttributeTableView;

class QgsAttributeTableDialog : public QDialog, private Ui::QgsAttributeTableDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param theLayer layer pointer
     * @param parent parent object
     * @param flags window flags
     */
    QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window );
    ~QgsAttributeTableDialog();

  public slots:
    /**
     * Toggles editing mode
     */
    void editingToggled();

  private slots:
    /**
     * submits the data
     */
    void submit();
    /**
     * Reverts the changes
     */
    void revert();
    /**
     * Launches search
     */
    void search();
    /**
     * Launches advanced search
     */
    void on_mAdvancedSearchButton_clicked();
    /**
     * Updates the selection
     */
    void updateSelection();
    /**
     * Reads the selection from the layer
     */
    void updateSelectionFromLayer();
    /**
     * Updates selection of a row
     */
    void updateRowSelection( int index );
    /**
     * Updates the index pressed
     */
    void updateRowPressed( int index );
    /**
     * Updates selection of specified rows
     * @param first first row
     * @param last last row
     * @param clickType 0:Single click, 1:Shift, 2:Ctrl, 3:dragged click
     */
    void updateRowSelection( int first, int last, int clickType );

    /**
     * Toggle showing of selected line only
     * @param theFlag toggle on if true
     */
    void on_cbxShowSelectedOnly_toggled( bool theFlag );
    /**
     * Copies selected rows to the clipboard
     */
    void on_mCopySelectedRowsButton_clicked();

    /**
     * Toggles editing mode
     */
    void on_mToggleEditingButton_toggled();
    /**
     * Saves edits
     */
    void on_mSaveEditsButton_clicked();
    /**
     * Inverts selection
     */
    void on_mInvertSelectionButton_clicked();
    /**
     * Clears selection
     */
    void on_mRemoveSelectionButton_clicked();
    /**
     * Zooms to selected features
     */
    void on_mZoomMapToSelectedRowsButton_clicked();
    /**
     * Moves selected lines to the top
     */
    void on_mSelectedToTopButton_clicked();
    /**
     * Shows advanced actions
     */
    void showAdvanced();
    /**
     * Starts editing mode
     */
    void startEditing();
    /**Opens dialog to add new attribute*/
    void on_mAddAttribute_clicked();
    /**Opens dialog to remove attribute*/
    void on_mRemoveAttribute_clicked();
    /**
     * Opens field calculator dialog
     */
    void on_mOpenFieldCalculator_clicked();

    /**
     * deletes the selected features
     */
    void on_mDeleteSelectedButton_clicked();

    /**
     * add feature
     */
    void addFeature();

    void on_mHelpButton_clicked() { QgsContextHelp::run( metaObject()->className() ); }

  signals:
    /**
     * Informs that editing mode has been toggled
     * @param layer layer that has been toggled
     */
    void editingToggled( QgsMapLayer *layer );

    /**
     * Informs that edits should be saved
     * @param layer layer whose edits are to be saved
     */
    void saveEdits( QgsMapLayer *layer );

  protected:
    /**
     * Handle closing of the window
     * @param event unused
     */
    void closeEvent( QCloseEvent* event );

  private:
    /**
     * Initialize column box
     */
    void columnBoxInit();
    /**
     * Returns id of a column
     */
    int columnBoxColumnId();
    /**
     * Performs the search
     * @param searchString search query string
     */
    void doSearch( QString searchString );

    /**
     * update window title
     */
    void updateTitle();

    QIcon getThemeIcon( const QString theName );

    QLineEdit *mQuery;
    QComboBox *mColumnBox;
    QComboBox *mShowBox;

    QMenu* mMenuActions;
    QAction* mActionToggleEditing;

    QgsAttributeTableModel *mModel;
    QgsAttributeTableFilterModel *mFilterModel;
    QgsVectorLayer *mLayer;
    QgsFeatureIds mSelectedFeatures;
    int mIndexPressed;

    QItemSelectionModel* mSelectionModel;
    int mLastClickedHeaderIndex;

    QDockWidget *mDock;
};

#endif
