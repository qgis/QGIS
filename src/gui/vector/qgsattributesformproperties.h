/***************************************************************************
    qgsattributesformproperties.h
    ---------------------
    begin                : August 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESFORMPROPERTIES_H
#define QGSATTRIBUTESFORMPROPERTIES_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsattributesformproperties.h"

#include "qgis_gui.h"
#include "qgsaction.h"
#include "qgsattributesformmodel.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsmessagebar.h"
#include "qgspropertycollection.h"
#include "qgssettingstree.h"
#include "qgssettingstreenode.h"

#include <QAction>
#include <QClipboard>
#include <QDropEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QWidget>

class QgsAttributeFormContainerEdit;
class QgsAttributeTypeDialog;
class QgsAttributeWidgetEdit;
class QgsAttributesFormBaseView;
class QgsFieldConstraintIndicatorProvider;
class QgsFieldDefaultValueIndicatorProvider;

/**
 * \brief Creates panels to configure attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAttributesFormProperties : public QWidget, public QgsExpressionContextGenerator, private Ui_QgsAttributesFormProperties
{
    Q_OBJECT

  public:
    static inline QgsSettingsTreeNode *sTreeAttributesForm = QgsSettingsTree::sTreeApp->createChildNode( u"attributes-form"_s );
    static const QgsSettingsEntryBool *settingShowAliases;

    explicit QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent = nullptr );

    void init();

    /**
     * Applies the attribute from properties to the vector layer.
     */
    void apply();

    /**
     * Stores currently opened widget configuration.
     * \since QGIS 3.36
     */
    void store();

    void loadRelations();

    /**
     * Initializes the available widgets tree view, repopulating the underlying model.
     */
    void initAvailableWidgetsView();

    /**
     * Initializes the form layout tree view, repopulating the underlying model.
     */
    void initFormLayoutView();

    void initLayoutConfig();
    void initInitPython();
    void initSuppressCombo();

    /**
     * Refresh layer actions in the Available Widgets view
     *
     * \param actions Up-to-date list of actions
     */
    void initAvailableWidgetsActions( const QList< QgsAction > actions );

    QgsExpressionContext createExpressionContext() const override;

  protected:
    void updateButtons();

    QgsVectorLayer *mLayer = nullptr;

    QgsAttributesFormBaseView *mAvailableWidgetsView = nullptr;
    QgsAttributesFormBaseView *mFormLayoutView = nullptr;

    QgsAttributeWidgetEdit *mAttributeWidgetEdit = nullptr;
    QgsAttributeTypeDialog *mAttributeTypeDialog = nullptr;
    QgsAttributeFormContainerEdit *mAttributeContainerEdit = nullptr;
    QLabel *mInfoTextWidget = nullptr;

  private slots:
    void addContainer();
    void removeTabOrGroupButton();
    void mEditorLayoutComboBox_currentIndexChanged( int index );
    void pbnSelectEditForm_clicked();
    void mTbInitCode_clicked();
    void toggleShowAliases( bool checked );

    /**
     * Inverts selection of top-level items.
     */
    void onInvertSelectionButtonClicked( bool checked );

    void loadAttributeSpecificEditor( QgsAttributesFormBaseView *emitter, QgsAttributesFormBaseView *receiver, QModelIndex &deselectedFormLayoutIndex );
    void onAttributeSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void onFormLayoutSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    //! Context menu for Fields to enable Copy&Paste
    void onContextMenuRequested( QPoint );

    void updatedFields();

    void updateFilteredItems( const QString &filterText );

  private:
    //! this will clean the right panel
    void clearAttributeTypeFrame();

    void loadAttributeWidgetEdit();

    /**
     * Stores attribute widget edit for the selected item in form layout tree view.
     */
    void storeAttributeWidgetEdit();

    /**
     * Stores attribute container edit for the selected item in form layout tree view.
     */
    void storeAttributeContainerEdit();

    //! Index should come from mFormLayoutView because it's there that attribute widget config is stored!
    void storeAttributeWidgetEdit( const QModelIndex &index );
    void storeAttributeContainerEdit( const QModelIndex &index );

    void loadAttributeTypeDialog();
    void loadAttributeTypeDialogFromConfiguration( const QgsAttributesFormData::FieldConfig &cfg );
    void storeAttributeTypeDialog();

    void loadAttributeContainerEdit();

    void loadInfoWidget( const QString &infoText );

    void copyWidgetConfiguration();
    void pasteWidgetConfiguration();

    void setAvailableWidgetsIndicatorProvidersEnabled( bool enabled );
    void setFormLayoutIndicatorProvidersEnabled( bool enabled );

    QgsAttributesAvailableWidgetsModel *mAvailableWidgetsModel = nullptr;
    QgsAttributesFormLayoutModel *mFormLayoutModel = nullptr;
    QgsAttributesFormProxyModel *mAvailableWidgetsProxyModel = nullptr;
    QgsAttributesFormProxyModel *mFormLayoutProxyModel = nullptr;

    QgsMessageBar *mMessageBar = nullptr;

    Qgis::AttributeFormPythonInitCodeSource mInitCodeSource = Qgis::AttributeFormPythonInitCodeSource::NoSource;
    QString mInitFunction;
    QString mInitFilePath;
    QString mInitCode;
    int mBlockUpdates = 0;

    //! Context menu for Fields
    QMenu *mAvailableWidgetsContextMenu = nullptr;
    QAction *mActionCopyWidgetConfiguration = nullptr;
    QAction *mActionPasteWidgetConfiguration = nullptr;

    //! Indicator providers for both views
    QgsFieldConstraintIndicatorProvider *mConstraintIndicatorProviderAvailableWidgets = nullptr;
    QgsFieldDefaultValueIndicatorProvider *mDefaultValueIndicatorProviderAvailableWidgets = nullptr;
    QgsFieldConstraintIndicatorProvider *mConstraintIndicatorProviderFormLayout = nullptr;
    QgsFieldDefaultValueIndicatorProvider *mDefaultValueIndicatorProviderFormLayout = nullptr;

    friend class TestQgsAttributesFormProperties;
};

#endif // QGSATTRIBUTESFORMPROPERTIES_H
