/***************************************************************************
    qgsfieldsproperties.h
    ---------------------
    begin                : September 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDSPROPERTIES_H
#define QGSFIELDSPROPERTIES_H

#include <QWidget>
#include <QPushButton>
#include <QTreeWidget>
#include <QTableWidget>

#include "qgsvectorlayer.h"
#include "ui_qgsfieldspropertiesbase.h"

class QgsAttributesTree : public QTreeWidget
{
    Q_OBJECT
  public:
    QgsAttributesTree( QWidget* parent = 0 )
        : QTreeWidget( parent )
    {}
    QTreeWidgetItem* addContainer( QTreeWidgetItem* parent , QString title );
    QTreeWidgetItem* addItem( QTreeWidgetItem* parent , QString fieldName );

  protected:
    virtual void dragMoveEvent( QDragMoveEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual bool dropMimeData( QTreeWidgetItem * parent, int index, const QMimeData * data, Qt::DropAction action );
    /* Qt::DropActions supportedDropActions() const;*/
};


class QgsFieldsProperties : public QWidget, private Ui_QgsFieldsPropertiesBase
{
    Q_OBJECT

  public:
    class FieldConfig
    {
      public:
        FieldConfig();
        FieldConfig( QgsVectorLayer* layer, int idx );

        bool mEditable;
        bool mEditableEnabled;
        bool mLabelOnTop;
        QgsVectorLayer::ValueRelationData mValueRelationData;
        QMap<QString, QVariant> mValueMap;
        QgsVectorLayer::RangeData mRange;
        QPair<QString, QString> mCheckedState;
        QgsVectorLayer::EditType mEditType;
        QPushButton* mButton;
        QString mDateFormat;
        QSize mWidgetSize;
    };

  public:
    QgsFieldsProperties( QgsVectorLayer *layer, QWidget* parent = 0 );

    ~QgsFieldsProperties();

    /**Adds an attribute to the table (but does not commit it yet)
    @param field the field to add
    @return false in case of a name conflict, true in case of success */
    bool addAttribute( const QgsField &field );

    /**Creates the a proper item to save from the tree
     * @param item The tree widget item to process
     * @return A widget definition. Containing another container or the final field
     */
    QgsAttributeEditorElement* createAttributeEditorWidget( QTreeWidgetItem* item, QObject *parent );

    void init();
    void apply();

    void updateButtons();
    void loadRows();
    void setRow( int row, int idx, const QgsField &field );

    void loadAttributeEditorTree();
    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement* const widgetDef, QTreeWidgetItem* parent );

  signals:
    void toggleEditing();

  public slots:
    void on_mAddAttributeButton_clicked();
    void on_mDeleteAttributeButton_clicked();
    void on_mCalculateFieldButton_clicked();
    void onAttributeSelectionChanged();
    void on_pbnSelectEditForm_clicked();
    void on_mEditorLayoutComboBox_currentIndexChanged( int index );

    void addAttribute();
    void attributeAdded( int idx );
    void attributeDeleted( int idx );
    void attributeTypeDialog();

    void on_mAddTabOrGroupButton_clicked();
    void on_mAddItemButton_clicked();
    void on_mRemoveTabGroupItemButton_clicked();
    void on_mMoveDownItem_clicked();
    void on_mMoveUpItem_clicked();

    void attributesListCellChanged( int row, int column );

  protected slots:
    /** editing of layer was toggled */
    void editingToggled();

  protected:
    FieldConfig configForRow( int row );
    void setConfigForRow( int row, FieldConfig cfg );

    QgsVectorLayer* mLayer;
    QgsAttributesTree* mAttributesTree;
    QTableWidget* mAttributesList;

    // Holds all the first column items (header: id) of the table.
    // The index in the list is the fieldIdx, and therefore acts as a mapping
    // between fieldIdx and QTableWidgetItem->row()
    QList<QTableWidgetItem*> mIndexedWidgets;

    enum attrColumns
    {
      attrIdCol = 0,
      attrNameCol,
      attrTypeCol,
      attrTypeNameCol,
      attrLengthCol,
      attrPrecCol,
      attrCommentCol,
      attrEditTypeCol,
      attrAliasCol,
      attrWMSCol,
      attrWFSCol,
      attrColCount,
    };

    static QMap< QgsVectorLayer::EditType, QString > editTypeMap;
    static void setupEditTypes();
    static QString editTypeButtonText( QgsVectorLayer::EditType type );
    static QgsVectorLayer::EditType editTypeFromButtonText( QString text );

};

Q_DECLARE_METATYPE( QgsFieldsProperties::FieldConfig )

#endif // QGSFIELDSPROPERTIES_H
