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
#include "qgspropertycollection.h"
#include "qgssettingstree.h"
#include "qgssettingstreenode.h"
#include "qgsmessagebar.h"

#include <QMimeData>
#include <QPushButton>
#include <QWidget>
#include <QTreeView>
#include <QSpinBox>
#include <QDropEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QAction>
#include <QMenu>
#include <QClipboard>

class QgsAttributeFormContainerEdit;
class QgsAttributeTypeDialog;
class QgsAttributeWidgetEdit;
class QgsAttributesFormBaseView;

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
    static inline QgsSettingsTreeNode *sTreeAttributesForm = QgsSettingsTree::sTreeApp->createChildNode( QStringLiteral( "attributes-form" ) );
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

    QgsAttributesAvailableWidgetsModel *mAvailableWidgetsModel;
    QgsAttributesFormLayoutModel *mFormLayoutModel;
    QgsAttributesFormProxyModel *mAvailableWidgetsProxyModel;
    QgsAttributesFormProxyModel *mFormLayoutProxyModel;

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

    friend class TestQgsAttributesFormProperties;
};


/**
 * \brief Graphical representation for the attribute drag and drop editor.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormBaseView : public QTreeView, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormBaseView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesFormBaseView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Returns the source model index corresponding to the first selected row.
     *
     * \note The first selected row is the first one the user selected, and not necessarily the one closer to the header.
     */
    QModelIndex firstSelectedIndex() const;

    // QgsExpressionContextGenerator interface
    QgsExpressionContext createExpressionContext() const override;

  public slots:
    /**
     * Selects the first item that matches a \a itemType and a \a itemId.
     *
     * Helps to keep in sync selection from both Attribute Widget view and Form Layout view.
     */
    void selectFirstMatchingItem( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId );

    /**
     * Sets the filter text to the underlying proxy model.
     *
     * \param text Filter text to be used to filter source model items.
     */
    void setFilterText( const QString &text );

  protected:
    QgsVectorLayer *mLayer = nullptr;
    QgsAttributesFormProxyModel *mModel = nullptr;
};


/**
 * \brief Graphical representation for the available widgets while configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesAvailableWidgetsView : public QgsAttributesFormBaseView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesAvailableWidgetsView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesAvailableWidgetsView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesFormProxyModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

    //! Access the underlying QgsAttributesAvailableWidgetsModel source model
    QgsAttributesAvailableWidgetsModel *availableWidgetsModel() const;
};


/**
 * \brief Graphical representation for the form layout while configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormLayoutView : public QgsAttributesFormBaseView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormLayoutView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesFormLayoutView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesFormProxyModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

  protected:
    // Drag and drop support (to handle internal moves)
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:
    void onItemDoubleClicked( const QModelIndex &index );
    void handleExternalDroppedItem( QModelIndex &index );
    void handleInternalDroppedItem( QModelIndex &index );
};

#endif // QGSATTRIBUTESFORMPROPERTIES_H
