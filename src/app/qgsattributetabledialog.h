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
#include "qgsfieldmodel.h"
#include "qgseditorwidgetwrapper.h"

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QDockWidget;
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
    QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window );
    ~QgsAttributeTableDialog();

    /**
     * Sets the filter expression to filter visible features
     * @param filterString filter query string. QgsExpression compatible.
     */
    void setFilterExpression( QString filterString );

  public slots:
    /**
     * Toggles editing mode
     */
    void editingToggled();

  private slots:
    /**
     * Copies selected rows to the clipboard
     */
    void on_mCopySelectedRowsButton_clicked();
    /**
     * Paste features from the clipboard
     */
    void on_mPasteFeatures_clicked();
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
     * Pans to selected features
     */
    void on_mPanMapToSelectedRowsButton_clicked();
    /**
     * Moves selected lines to the top
     */
    void on_mSelectedToTopButton_toggled();

    /**
     * Opens dialog to add new attribute
     */
    void on_mAddAttribute_clicked();

    /**
     * Opens dialog to remove attribute
     */
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
    void on_mAddFeature_clicked();

    void on_mHelpButton_clicked() { QgsContextHelp::run( metaObject()->className() ); }

    void on_mExpressionSelectButton_clicked();
    void filterColumnChanged( QObject* filterAction );
    void filterExpressionBuilder();
    void filterShowAll();
    void filterSelected();
    void filterVisible();
    void filterEdited();
    void filterQueryChanged( const QString& query );
    void filterQueryAccepted();

    /**
     * update window title
     */
    void updateTitle();

    void updateButtonStatus( QString fieldName, bool isValid );

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

    void runFieldCalculation( QgsVectorLayer* layer, QString fieldName, QString expression, QgsFeatureIds filteredIds = QgsFeatureIds() );
    void updateFieldFromExpression();
    void updateFieldFromExpressionSelected();

  private:
    QMenu* mMenuActions;
    QAction* mActionToggleEditing;

    QDockWidget* mDock;
    QgsDistanceArea* myDa;

    QMenu* mFilterColumnsMenu;
    QSignalMapper* mFilterActionMapper;

    QgsVectorLayer* mLayer;
    QgsFieldModel* mFieldModel;

    QgsRubberBand* mRubberBand;
    QgsEditorWidgetWrapper* mCurrentSearchWidgetWrapper;
};

#endif
