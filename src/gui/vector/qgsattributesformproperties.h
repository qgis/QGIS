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

#include <QMimeData>
#include <QPushButton>
#include <QTableWidget>
#include <QTreeWidget>
#include <QWidget>
#include <QSpinBox>
#include <QTreeWidgetItem>
#include <QDropEvent>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QAction>
#include <QMenu>
#include <QClipboard>

#include "ui_qgsattributesformproperties.h"
#include "qgis_gui.h"
#include "qgsattributesformmodel.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsattributeeditorelement.h"
#include "qgspropertycollection.h"
#include "qgsmessagebar.h"

class QgsAttributeFormContainerEdit;
class QgsAttributeTypeDialog;
class QgsAttributeWidgetEdit;
class QgsAttributesFormBaseTreeView;

/**
 * \ingroup gui
 * \class QgsAttributesFormProperties
 * \brief A widget for configuring attribute forms.
 */
class GUI_EXPORT QgsAttributesFormProperties : public QWidget, public QgsExpressionContextGenerator, private Ui_QgsAttributesFormProperties
{
    Q_OBJECT

  public:
    enum FieldPropertiesRoles
    {
      DnDTreeRole = Qt::UserRole,
      FieldConfigRole,
      FieldNameRole,
    };

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

    void initAvailableWidgetsTree();
    void initFormLayoutTree();
    void initLayoutConfig();
    void initInitPython();
    void initSuppressCombo();

    QgsExpressionContext createExpressionContext() const override;

  protected:
    void updateButtons();

    //QList<QgsRelation> mRelations;
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

    void onInvertSelectionButtonClicked( bool checked );
    void loadAttributeSpecificEditor( QgsAttributesFormBaseTreeView *emitter, QgsAttributesFormBaseTreeView *receiver, QModelIndex &deselectedFormLayoutIndex );
    void onAttributeSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void onFormLayoutSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    //! Context menu for Fields to enable Copy&Paste
    void onContextMenuRequested( QPoint );

    void updatedFields();

  private:
    QModelIndex getPreviousIndex( const QgsAttributesFormBaseTreeView *treeView, const QItemSelection &deselected ) const;

    //! this will clean the right panel
    void clearAttributeTypeFrame();

    void loadAttributeWidgetEdit();
    void storeAttributeWidgetEdit();

    // index should come from mFormLayoutTreeView because it's there that attribute widget config is stored!
    void storeAttributeWidgetEdit( const QModelIndex &index );
    void storeAttributeContainerEdit( const QModelIndex &index );

    void loadAttributeTypeDialog();
    void loadAttributeTypeDialogFromConfiguration( const QgsAttributeFormTreeData::FieldConfig &cfg );
    void storeAttributeTypeDialog();

    void storeAttributeContainerEdit();
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
 * \ingroup gui
 * \class QgsAttributesFormBaseTreeView
 *
 * Graphical representation for the attribute drag and drop editor
 */
class GUI_EXPORT QgsAttributesFormBaseTreeView : public QTreeView, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormBaseTreeView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    // QgsExpressionContextGenerator interface
    QgsExpressionContext createExpressionContext() const override;

  public slots:
    void selectFirstMatchingItem( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId );

  protected:
    QgsVectorLayer *mLayer = nullptr;
};

class GUI_EXPORT QgsAttributesAvailableWidgetsTreeView : public QgsAttributesFormBaseTreeView
{
    Q_OBJECT

  public:
    explicit QgsAttributesAvailableWidgetsTreeView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesAvailableWidgetsModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

    //! Gets access to the QgsAttributesAvailableWidgetsModel model
    QgsAttributesAvailableWidgetsModel *availableWidgetsModel() const;

  private:
    QgsAttributesAvailableWidgetsModel *mModel;
};

class GUI_EXPORT QgsAttributesFormLayoutTreeView : public QgsAttributesFormBaseTreeView
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormLayoutTreeView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesFormLayoutModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

    //! Gets access to the QgsAttributesFormLayoutModel model
    QgsAttributesFormLayoutModel *formLayoutModel() const;

  protected:
    void dragEnterEvent( QDragEnterEvent *event ) override;

    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:
    void onItemDoubleClicked( const QModelIndex &index );
    void handleExternalDroppedNode( QModelIndex &index );
    void handleInternalDroppedNode( QModelIndex &index );

  private:
    QgsAttributesFormLayoutModel *mModel;
};

#endif // QGSATTRIBUTESFORMPROPERTIES_H
