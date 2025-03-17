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

class QgsAttributesDnDTree;
class QgsAttributeFormContainerEdit;
class QgsAttributeTypeDialog;
class QgsAttributeWidgetEdit;

/**
 * \ingroup gui
 * \class QgsAttributesFormProperties
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
     * Creates a new attribute editor element based on the definition stored in \a item.
     */
    QgsAttributeEditorElement *createAttributeEditorWidget( QTreeWidgetItem *item, QgsAttributeEditorElement *parent, bool isTopLevel = false );

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

    QgsAttributesDnDTree *mAvailableWidgetsTree = nullptr;
    QgsAttributesDnDTree *mFormLayoutTree = nullptr;

    QTreeView *mAvailableWidgetsTreeView = nullptr;

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
    void loadAttributeSpecificEditor( QTreeView *emitter, QgsAttributesDnDTree *receiver );
    void onAttributeSelectionChanged();
    void onFormLayoutSelectionChanged();

    //! Context menu for Fields to enable Copy&Paste
    void onContextMenuRequested( QPoint );

    void updatedFields();

  private:
    //! this will clean the right panel
    void clearAttributeTypeFrame();

    void loadAttributeWidgetEdit();
    void storeAttributeWidgetEdit();

    void loadAttributeTypeDialog();
    void loadAttributeTypeDialogFromConfiguration( const QgsAttributeFormTreeData::FieldConfig &cfg );
    void storeAttributeTypeDialog();

    void storeAttributeContainerEdit();
    void loadAttributeContainerEdit();

    void loadInfoWidget( const QString &infoText );

    void copyWidgetConfiguration();
    void pasteWidgetConfiguration();

    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement *widgetDef, QTreeWidgetItem *parent, QgsAttributesDnDTree *tree );

    QgsAttributesAvailableWidgetsModel *mAvailableWidgetsModel;

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


//QDataStream &operator<<( QDataStream &stream, const QgsAttributesFormProperties::DnDTreeItemData &data );
//QDataStream &operator>>( QDataStream &stream, QgsAttributesFormProperties::DnDTreeItemData &data );


/**
 * \ingroup gui
 * \class QgsAttributesDnDTree
 *
 * \brief This class overrides mime type handling to be able to work with
 * the drag and drop attribute editor.
 *
 * The mime type is application/x-qgsattributetablefield
 *
 * Graphical representation for the attribute editor drag and drop editor
 */
class GUI_EXPORT QgsAttributesDnDTree : public QTreeWidget, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    explicit QgsAttributesDnDTree( QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Adds a new item to a \a parent. If \a index is -1, the item is added to the end of the parent's existing children.
     * Otherwise it is inserted at the specified \a index.
     */
    //QTreeWidgetItem *addItem( QTreeWidgetItem *parent, const QgsAttributesFormProperties::DnDTreeItemData &data, int index = -1, const QIcon &icon = QIcon() );

    /**
     * Adds a new container to \a parent.
     *
     * If no \a parent is set then the container will be forced to a tab widget.
     */
    QTreeWidgetItem *addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount, Qgis::AttributeEditorContainerType type );

    enum Type
    {
      Drag,
      Drop
    };


    Type type() const;
    void setType( QgsAttributesDnDTree::Type value );

  public slots:
    void selectFirstMatchingItem( const QgsAttributeFormTreeData::DnDTreeItemData &data );

  protected:
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    bool dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action ) override;

    // QTreeWidget interface
  protected:
    QStringList mimeTypes() const override;

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    QMimeData *mimeData( const QList<QTreeWidgetItem *> items ) const override;
#else
    QMimeData *mimeData( const QList<QTreeWidgetItem *> &items ) const override;
#endif

  private slots:
    void onItemDoubleClicked( QTreeWidgetItem *item, int column );

  private:
    QgsVectorLayer *mLayer = nullptr;
    Type mType = QgsAttributesDnDTree::Type::Drag;

    // QgsExpressionContextGenerator interface
  public:
    QgsExpressionContext createExpressionContext() const override;
};


#endif // QGSATTRIBUTESFORMPROPERTIES_H
