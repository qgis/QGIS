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

class QgsAttributesList : public QTableWidget
{
    Q_OBJECT
  public:
    QgsAttributesList( QWidget* parent = 0 )
        : QTableWidget( parent )
    {}

  protected:
    // virtual void dragMoveEvent( QDragMoveEvent *event );
    //QMimeData *mimeData( const QList<QTableWidgetItem *> items ) const;
    //Qt::DropActions supportedDropActions() const;
};

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
    QgsFieldsProperties( QgsVectorLayer *layer, QWidget* parent = 0 );

    /**Adds an attribute to the table (but does not commit it yet)
    @param field the field to add
    @return false in case of a name conflict, true in case of success */
    bool addAttribute( const QgsField &field );

    /**Deletes an attribute (but does not commit it)
      @param name attribute name
      @return false in case of a non-existing attribute.*/
    bool deleteAttribute( int attr );

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
    void deleteAttribute();
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
    QgsVectorLayer* mLayer;
    QgsAttributesTree* mAttributesTree;
    QgsAttributesList* mAttributesList;

    QMap<int, bool> mFieldEditables;
    QMap<int, QgsVectorLayer::ValueRelationData> mValueRelationData;
    QMap<int, QMap<QString, QVariant> > mValueMaps;
    QMap<int, QgsVectorLayer::RangeData> mRanges;
    QMap<int, QPair<QString, QString> > mCheckedStates;
    QMap<int, QgsVectorLayer::EditType> mEditTypeMap;
    QMap<int, QPushButton*> mButtonMap;
    QMap<int, QString> mDateFormat;
    QMap<int, QSize> mWidgetSize;

    enum attrColumns
    {
      attrIdCol = 0,
      attrNameCol,
      attrTypeCol,
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

#endif // QGSFIELDSPROPERTIES_H
