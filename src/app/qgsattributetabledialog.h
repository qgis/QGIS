/***************************************************************************
  QgsAttributeTableDialog.h - dialog for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : (C) 2009 by Vita Cizek
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

#include <ctime>

#include "ui_qgsattributetabledialog.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsdockwidget.h"
#include "qgis_app.h"
#include "qgsstoredexpressionmanager.h"

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QSignalMapper;
class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;
class QgsRubberBand;
struct QgsStoredExpression;

class APP_EXPORT QgsAttributeTableDialog : public QDialog, private Ui::QgsAttributeTableDialog, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param layer layer pointer
     * \param initialMode initial filter for dialog
     * \param parent parent object
     * \param flags window flags
     */
    QgsAttributeTableDialog( QgsVectorLayer *layer, QgsAttributeTableFilterModel::FilterMode initialMode = QgsAttributeTableFilterModel::ShowAll, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Window );

    QgsExpressionContext createExpressionContext() const override;

  public slots:

    /**
     * Toggles editing mode
     */
    void editingToggled();

    /**
     * Sets the filter expression to filter visible features
     * \param filterString filter query string. QgsExpression compatible.
     */
    void setFilterExpression( const QString &filterString,
                              QgsAttributeForm::FilterType type = QgsAttributeForm::ReplaceFilter,
                              bool alwaysShowFilter = false );

  private slots:

    /**
     * Cut selected rows to the clipboard
     */
    void mActionCutSelectedRows_triggered();

    /**
     * Copies selected rows to the clipboard
     */
    void mActionCopySelectedRows_triggered();

    /**
     * Paste features from the clipboard
     */
    void mActionPasteFeatures_triggered();

    /**
     * Toggles editing mode
     */
    void mActionToggleEditing_toggled( bool );

    /**
     * Saves edits
     */
    void mActionSaveEdits_triggered();

    /**
     * Reload the data
     */
    void mActionReload_triggered();

    /**
     * Inverts selection
     */
    void mActionInvertSelection_triggered();

    /**
     * Clears selection
     */
    void mActionRemoveSelection_triggered();

    /**
     * Select all
     */
    void mActionSelectAll_triggered();

    /**
     * Zooms to selected features
     */
    void mActionZoomMapToSelectedRows_triggered();

    /**
     * Pans to selected features
     */
    void mActionPanMapToSelectedRows_triggered();

    /**
     * Moves selected lines to the top
     */
    void mActionSelectedToTop_toggled( bool );

    /**
     * Opens dialog to add new attribute
     */
    void mActionAddAttribute_triggered();

    /**
     * Opens dialog to remove attribute
     */
    void mActionRemoveAttribute_triggered();

    /**
     * Opens field calculator dialog
     */
    void mActionOpenFieldCalculator_triggered();

    /**
     * deletes the selected features
     */
    void mActionDeleteSelected_triggered();

    /**
     * Called when the current index changes in the main view
     * i.e. when the view mode is switched from table to form view
     * or vice versa.
     *
     * Will adjust the button state
     */
    void mMainView_currentChanged( int );

    /**
     * add feature
     */
    void mActionAddFeature_triggered();

    void mActionExpressionSelect_triggered();
    void filterColumnChanged( QObject *filterAction );
    void filterExpressionBuilder();
    void filterShowAll();
    void filterSelected();
    void filterVisible();
    void filterEdited();
    void filterQueryChanged( const QString &query );
    void filterQueryAccepted();

    /**
    * Handles the expression (save or delete) when the bookmark button for stored
    * filter expressions is triggered.
    */
    void handleStoreFilterExpression();

    /**
    * Opens dialog and give the possibility to save the expression with a name.
    */
    void saveAsStoredFilterExpression();

    /**
    * Opens dialog and give the possibility to edit the name and the expression
    * of the stored expression.
    */
    void editStoredFilterExpression();

    /**
     * Updates the bookmark button and it's actions regarding the stored filter
     * expressions according to the values
     */
    void updateCurrentStoredFilterExpression( );

    /**
     * Starts timer with timeout 300 ms.
     */
    void onFilterQueryTextChanged( const QString &value );

    void openConditionalStyles();

    /**
     * update window title
     */
    void updateTitle();

    void updateButtonStatus( const QString &fieldName, bool isValid );

    /* replace the search widget with a new one */
    void replaceSearchWidget( QWidget *oldw, QWidget *neww );

    void layerActionTriggered();
  signals:

    /**
     * Informs that edits should be saved
     * \param layer layer whose edits are to be saved
     */
    void saveEdits( QgsMapLayer *layer );

  protected:

    /*
     * Handle KeyPress event of the window
     * \param event
     */
    void keyPressEvent( QKeyEvent *event ) override;

    bool eventFilter( QObject *object, QEvent *ev ) override;

  private slots:

    //! Initialize column box
    void columnBoxInit();

    //! Initialize storedexpression box e.g after adding/deleting/edditing stored expression
    void storedFilterExpressionBoxInit();
    //! Functionalities of store expression button changes regarding the status of it
    void storeExpressionButtonInit();

    void runFieldCalculation( QgsVectorLayer *layer, const QString &fieldName, const QString &expression, const QgsFeatureIds &filteredIds = QgsFeatureIds() );
    void updateFieldFromExpression();
    void updateFieldFromExpressionSelected();
    void viewModeChanged( QgsAttributeEditorContext::Mode mode );
    void formFilterSet( const QString &filter, QgsAttributeForm::FilterType type );
    void showContextMenu( QgsActionMenu *menu, QgsFeatureId fid );
    void toggleDockMode( bool docked );

  private:
    QMenu *mMenuActions = nullptr;
    QToolButton *mActionFeatureActions = nullptr;

    QgsDockWidget *mDock = nullptr;
    QDialog *mDialog = nullptr;

    QMenu *mFilterColumnsMenu = nullptr;
    QSignalMapper *mFilterActionMapper = nullptr;
    QMenu *mStoredFilterExpressionMenu = nullptr;

    QPointer< QgsVectorLayer > mLayer = nullptr;
    QgsSearchWidgetWrapper *mCurrentSearchWidgetWrapper = nullptr;
    QStringList mVisibleFields;
    QgsAttributeEditorContext mEditorContext;

    QTimer mFilterQueryTimer;

    void updateMultiEditButtonState();
    void deleteFeature( QgsFeatureId fid );

    friend class TestQgsAttributeTable;
};


class QgsAttributeTableDock : public QgsDockWidget
{
    Q_OBJECT

  public:
    QgsAttributeTableDock( const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

    void closeEvent( QCloseEvent *ev ) override;
};


#endif
