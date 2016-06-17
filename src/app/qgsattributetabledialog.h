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

#include <time.h>

#include "ui_qgsattributetabledialog.h"
#include "qgscontexthelp.h"

#include "qgsattributedialog.h"
#include "qgsvectorlayer.h" //QgsFeatureIds
#include "qgssearchwidgetwrapper.h"
#include "qgsdockwidget.h"

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QSignalMapper;
class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;
class QgsRubberBand;

class APP_EXPORT QgsAttributeTableDialog : public QDialog, private Ui::QgsAttributeTableDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param theLayer layer pointer
     * @param parent parent object
     * @param flags window flags
     */
    QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Window );
    ~QgsAttributeTableDialog();

  public slots:
    /**
     * Toggles editing mode
     */
    void editingToggled();

    /**
     * Sets the filter expression to filter visible features
     * @param filterString filter query string. QgsExpression compatible.
     */
    void setFilterExpression( const QString& filterString,
                              QgsAttributeForm::FilterType type = QgsAttributeForm::ReplaceFilter,
                              bool alwaysShowFilter = false );

  private slots:
    /**
     * Copies selected rows to the clipboard
     */
    void on_mActionCopySelectedRows_triggered();
    /**
     * Paste features from the clipboard
     */
    void on_mActionPasteFeatures_triggered();
    /**
     * Toggles editing mode
     */
    void on_mActionToggleEditing_toggled( bool );
    /**
     * Saves edits
     */
    void on_mActionSaveEdits_triggered();
    /**
     * Reload the data
     */
    void on_mActionReload_triggered();
    /**
     * Inverts selection
     */
    void on_mActionInvertSelection_triggered();
    /**
     * Clears selection
     */
    void on_mActionRemoveSelection_triggered();
    /**
     * Select all
     */
    void on_mActionSelectAll_triggered();
    /**
     * Zooms to selected features
     */
    void on_mActionZoomMapToSelectedRows_triggered();
    /**
     * Pans to selected features
     */
    void on_mActionPanMapToSelectedRows_triggered();
    /**
     * Moves selected lines to the top
     */
    void on_mActionSelectedToTop_toggled( bool );

    /**
     * Opens dialog to add new attribute
     */
    void on_mActionAddAttribute_triggered();

    /**
     * Opens dialog to remove attribute
     */
    void on_mActionRemoveAttribute_triggered();
    /**
     * Opens field calculator dialog
     */
    void on_mActionOpenFieldCalculator_triggered();

    /**
     * deletes the selected features
     */
    void on_mActionDeleteSelected_triggered();

    /**
     * Called when the current index changes in the main view
     * i.e. when the view mode is switched from table to form view
     * or vice versa.
     *
     * Will adjust the button state
     */
    void on_mMainView_currentChanged( int );

    /**
     * add feature
     */
    void on_mActionAddFeature_triggered();

    void on_mActionExpressionSelect_triggered();
    void filterColumnChanged( QObject* filterAction );
    void filterExpressionBuilder();
    void filterShowAll();
    void filterSelected();
    void filterVisible();
    void filterEdited();
    void filterQueryChanged( const QString& query );
    void filterQueryAccepted();
    void openConditionalStyles();

    /**
     * update window title
     */
    void updateTitle();

    void updateButtonStatus( const QString& fieldName, bool isValid );

    /* replace the search widget with a new one */
    void replaceSearchWidget( QWidget* oldw, QWidget* neww );

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
    void closeEvent( QCloseEvent* event ) override;

    /*
     * Handle KeyPress event of the window
     * @param event
     */
    void keyPressEvent( QKeyEvent* event ) override;

  private slots:
    /**
     * Initialize column box
     */
    void columnBoxInit();

    void runFieldCalculation( QgsVectorLayer* layer, const QString& fieldName, const QString& expression, const QgsFeatureIds& filteredIds = QgsFeatureIds() );
    void updateFieldFromExpression();
    void updateFieldFromExpressionSelected();
    void viewModeChanged( QgsAttributeForm::Mode mode );
    void formFilterSet( const QString& filter, QgsAttributeForm::FilterType type );

  private:
    QMenu* mMenuActions;

    QgsDockWidget* mDock;
    QgsDistanceArea* myDa;


    QMenu* mFilterColumnsMenu;
    QSignalMapper* mFilterActionMapper;

    QgsVectorLayer* mLayer;
    QgsRubberBand* mRubberBand;
    QgsSearchWidgetWrapper* mCurrentSearchWidgetWrapper;
    QStringList mVisibleFields;
    QgsAttributeEditorContext mEditorContext;

    void updateMultiEditButtonState();

    friend class TestQgsAttributeTable;
};


class QgsAttributeTableDock : public QgsDockWidget
{
    Q_OBJECT

  public:
    QgsAttributeTableDock( const QString & title, QWidget * parent = nullptr, Qt::WindowFlags flags = nullptr );

    virtual void closeEvent( QCloseEvent * ev ) override;
};


#endif
