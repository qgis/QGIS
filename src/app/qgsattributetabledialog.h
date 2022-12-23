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

#include "qgis_app.h"
#include "ui_qgsattributetabledialog.h"

class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;
class QgsRubberBand;
struct QgsStoredExpression;
class QgsDockableWidgetHelper;

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
    QgsAttributeTableDialog( QgsVectorLayer *layer,
                             QgsAttributeTableFilterModel::FilterMode initialMode = QgsAttributeTableFilterModel::ShowAll,
                             QWidget *parent = nullptr,
                             Qt::WindowFlags flags = Qt::Window,
                             bool *initiallyDocked = nullptr,
                             const QString &filterExpression = QString() );
    ~QgsAttributeTableDialog() override;

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Writes the dialog's state to an XML element.
     */
    QDomElement writeXml( QDomDocument &document );

    /**
     * Reads the dialog's state from an XML element.
     */
    void readXml( const QDomElement &element );

    QgsDockableWidgetHelper *dockableWidgetHelper() { return mDockableWidgetHelper; }

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

    /**
     * Set the view \a mode (e.g. attribute table or attribute editor).
     * \since QGIS 3.24
     */
    void setView( QgsDualView::ViewMode mode );

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
     * Opens organize columns dialog
     */
    void mActionOrganizeColumns_triggered();

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
    void mActionAddFeatureViaAttributeTable_triggered();
    void mActionAddFeatureViaAttributeForm_triggered();

    void mActionExpressionSelect_triggered();

    void openConditionalStyles();

    /**
     * update window title
     */
    void updateTitle();

    void updateButtonStatus( const QString &fieldName, bool isValid );

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

  private slots:

    void runFieldCalculation( QgsVectorLayer *layer, const QString &fieldName, const QString &expression, const QgsFeatureIds &filteredIds = QgsFeatureIds() );
    void updateFieldFromExpression();
    void updateFieldFromExpressionSelected();
    void viewModeChanged( QgsAttributeEditorContext::Mode mode );
    void formFilterSet( const QString &filter, QgsAttributeForm::FilterType type );
    void showContextMenu( QgsActionMenu *menu, QgsFeatureId fid );
    void updateLayerModifiedActions();

  private:
    QMenu *mMenuActions = nullptr;
    QToolButton *mActionFeatureActions = nullptr;

    QDialog *mDialog = nullptr;

    QPointer< QgsVectorLayer > mLayer = nullptr;
    void updateMultiEditButtonState();
    void deleteFeature( QgsFeatureId fid );

    QList< QPointer< QgsVectorLayer> > mReferencingLayers;

    QAction *mActionDockUndock = nullptr;
    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;

    friend class TestQgsAttributeTable;
};


#endif
