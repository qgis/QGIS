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
#include "qgsattributesformmodel.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsattributeeditorelement.h"
#include "qgspropertycollection.h"
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
class QgsAttributesFormBaseTreeView;

/**
 * \brief Class to create a panel to configure attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAttributesFormProperties : public QWidget, public QgsExpressionContextGenerator, private Ui_QgsAttributesFormProperties
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Creates a new attribute editor element based on the definition stored in a form layout model \a index.
     */
    QgsAttributeEditorElement *createAttributeEditorWidget( const QModelIndex &index, QgsAttributeEditorElement *parent, bool isTopLevel = false );

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
    void initAvailableWidgetsTreeView();

    /**
     * Initializes the form layout tree view, repopulating the underlying model.
     */
    void initFormLayoutTreeView();

    void initLayoutConfig();
    void initInitPython();
    void initSuppressCombo();

    QgsExpressionContext createExpressionContext() const override;

  protected:
    void updateButtons();

    QgsVectorLayer *mLayer = nullptr;

    QgsAttributesFormBaseTreeView *mAvailableWidgetsTreeView = nullptr;
    QgsAttributesFormBaseTreeView *mFormLayoutTreeView = nullptr;

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

    /**
     * Inverts selection of top-level nodes.
     */
    void onInvertSelectionButtonClicked( bool checked );

    void loadAttributeSpecificEditor( QgsAttributesFormBaseTreeView *emitter, QgsAttributesFormBaseTreeView *receiver, QModelIndex &deselectedFormLayoutIndex );
    void onAttributeSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void onFormLayoutSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    //! Context menu for Fields to enable Copy&Paste
    void onContextMenuRequested( QPoint );

    void updatedFields();

  private:
    /**
     * Gets the index that was previously selected to store configuration when changing node selection.
     */
    QModelIndex previousIndex( const QgsAttributesFormBaseTreeView *treeView, const QItemSelection &deselected ) const;

    //! this will clean the right panel
    void clearAttributeTypeFrame();

    void loadAttributeWidgetEdit();

    /**
     * Stores attribute widget edit for the selected node in form layout tree view.
     */
    void storeAttributeWidgetEdit();

    /**
     * Stores attribute container edit for the selected node in form layout tree view.
     */
    void storeAttributeContainerEdit();

    //! Index should come from mFormLayoutTreeView because it's there that attribute widget config is stored!
    void storeAttributeWidgetEdit( const QModelIndex &index );
    void storeAttributeContainerEdit( const QModelIndex &index );

    void loadAttributeTypeDialog();
    void loadAttributeTypeDialogFromConfiguration( const QgsAttributesFormTreeData::FieldConfig &cfg );
    void storeAttributeTypeDialog();

    void loadAttributeContainerEdit();

    void loadInfoWidget( const QString &infoText );

    void copyWidgetConfiguration();
    void pasteWidgetConfiguration();

    QgsAttributesAvailableWidgetsModel *mAvailableWidgetsModel;
    QgsAttributesFormLayoutModel *mFormLayoutModel;

    QgsMessageBar *mMessageBar = nullptr;

    Qgis::AttributeFormPythonInitCodeSource mInitCodeSource = Qgis::AttributeFormPythonInitCodeSource::NoSource;
    QString mInitFunction;
    QString mInitFilePath;
    QString mInitCode;
    int mBlockUpdates = 0;

    //! Context menu for Fields
    QMenu *mAvailableWidgetsTreeContextMenu = nullptr;
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
class GUI_EXPORT QgsAttributesFormBaseTreeView : public QTreeView, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormBaseTreeView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesFormBaseTreeView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    // QgsExpressionContextGenerator interface
    QgsExpressionContext createExpressionContext() const override;

  public slots:
    /**
     * Selects the first node that matches a \a nodeType and a \a nodeId.
     *
     * Helps to keep in sync selection from both Attribute Widget view and Form Layout view.
     */
    void selectFirstMatchingNode( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId );

  protected:
    QgsVectorLayer *mLayer = nullptr;
};


/**
 * \brief Graphical representation for the available widgets while configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesAvailableWidgetsTreeView : public QgsAttributesFormBaseTreeView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesAvailableWidgetsTreeView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesAvailableWidgetsTreeView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesAvailableWidgetsModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

    //! Access the underlying QgsAttributesAvailableWidgetsModel model
    QgsAttributesAvailableWidgetsModel *availableWidgetsModel() const;

  private:
    QgsAttributesAvailableWidgetsModel *mModel = nullptr;
};


/**
 * \brief Graphical representation for the form layout while configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormLayoutTreeView : public QgsAttributesFormBaseTreeView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormLayoutTreeView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesFormLayoutTreeView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesFormLayoutModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

    //! Access the underlying QgsAttributesFormLayoutModel model
    QgsAttributesFormLayoutModel *formLayoutModel() const;

  protected:
    // Drag and drop support (to handle internal moves)
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:
    void onNodeDoubleClicked( const QModelIndex &index );
    void handleExternalDroppedNode( QModelIndex &index );
    void handleInternalDroppedNode( QModelIndex &index );

  private:
    QgsAttributesFormLayoutModel *mModel = nullptr;
};

#endif // QGSATTRIBUTESFORMPROPERTIES_H
