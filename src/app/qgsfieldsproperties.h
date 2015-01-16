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

#include <QMimeData>
#include <QPushButton>
#include <QTableWidget>
#include <QTreeWidget>
#include <QWidget>


#include "qgsvectorlayer.h"
#include "ui_qgsfieldspropertiesbase.h"

class APP_EXPORT QgsFieldsProperties : public QWidget, private Ui_QgsFieldsPropertiesBase
{
    Q_OBJECT

  public:

    enum FieldPropertiesRoles
    {
      DesignerTreeRole = Qt::UserRole,
      FieldConfigRole
    };

    class DesignerTreeItemData
    {
      public:
        enum Type
        {
          Field,
          Relation,
          Container
        };

        DesignerTreeItemData()
        {}

        DesignerTreeItemData( Type type, const QString& name )
            : mType( type )
            , mName( name ) {}

        QString name() const { return mName; }
        void setName( const QString& name ) { mName = name; }

        Type type() const { return mType; }
        void setType( const Type& type ) { mType = type; }

        QVariant asQVariant() { return QVariant::fromValue<DesignerTreeItemData>( *this ); }

      protected:
        Type mType;
        QString mName;
    };

    /**
     * This class overrides mime type handling to be able to work with
     * the drag and drop attribute editor.
     *
     * The mime type is application/x-qgsattributetablefield
     */

    class DragList : public QTableWidget
    {
      public:
        DragList( QWidget* parent = 0 )
            : QTableWidget( parent )
        {}

        // QTreeWidget interface
      protected:
        virtual QStringList mimeTypes() const override;

        virtual QMimeData* mimeData( const QList<QTableWidgetItem*> items ) const override;
    };


    /**
     * Graphical representation for the attribute editor drag and drop editor
     */
    class DesignerTree : public QTreeWidget
    {
      public:
        DesignerTree( QWidget* parent = 0 )
            : QTreeWidget( parent )
        {}
        QTreeWidgetItem* addItem( QTreeWidgetItem* parent, DesignerTreeItemData data );
        QTreeWidgetItem* addContainer( QTreeWidgetItem* parent, QString title );

      protected:
        virtual void dragMoveEvent( QDragMoveEvent *event ) override;
        virtual void dropEvent( QDropEvent *event ) override;
        virtual bool dropMimeData( QTreeWidgetItem * parent, int index, const QMimeData * data, Qt::DropAction action ) override;
        /* Qt::DropActions supportedDropActions() const;*/

        // QTreeWidget interface
      protected:
        virtual QStringList mimeTypes() const override;
        virtual QMimeData* mimeData( const QList<QTreeWidgetItem*> items ) const override;
    };

    /**
     * Holds the configuration for a field
     */
    class FieldConfig
    {
      public:
        FieldConfig();
        FieldConfig( QgsVectorLayer* layer, int idx );

        bool mEditable;
        bool mEditableEnabled;
        bool mLabelOnTop;
        QPushButton* mButton;
        QString mEditorWidgetV2Type;
        QMap<QString, QVariant> mEditorWidgetV2Config;
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

    void loadRows();
    void setRow( int row, int idx, const QgsField &field );

    void loadRelations();

    void loadAttributeEditorTree();
    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement* const widgetDef, QTreeWidgetItem* parent );

  signals:
    void toggleEditing();

  private slots:
    void on_mAddAttributeButton_clicked();
    void on_mDeleteAttributeButton_clicked();
    void on_mCalculateFieldButton_clicked();
    void onAttributeSelectionChanged();
    void on_pbnSelectEditForm_clicked();
    void on_mEditorLayoutComboBox_currentIndexChanged( int index );

    void attributeAdded( int idx );
    void attributeDeleted( int idx );
    void attributeTypeDialog();

    void on_mAddTabOrGroupButton_clicked();
    void on_mAddItemButton_clicked();
    void on_mRemoveTabGroupItemButton_clicked();
    void on_mMoveDownItem_clicked();
    void on_mMoveUpItem_clicked();

    void attributesListCellChanged( int row, int column );


    /** editing of layer was toggled */
    void editingToggled();

  protected:
    void updateButtons();

    FieldConfig configForRow( int row );
    void setConfigForRow( int row, FieldConfig cfg );

    QgsVectorLayer* mLayer;
    DesignerTree* mDesignerTree;
    DragList* mFieldsList;
    DragList* mRelationsList;

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

    enum relationColumns
    {
      RelNameCol = 0,
      RelLayerCol,
      RelFieldCol,
      RelIdCol,
      RelColCount
    };

    static QMap< QgsVectorLayer::EditType, QString > editTypeMap;
    static void setupEditTypes();
    static QString editTypeButtonText( QgsVectorLayer::EditType type );
};

QDataStream& operator<< ( QDataStream& stream, const QgsFieldsProperties::DesignerTreeItemData& data );
QDataStream& operator>> ( QDataStream& stream, QgsFieldsProperties::DesignerTreeItemData& data );

Q_DECLARE_METATYPE( QgsFieldsProperties::FieldConfig )
Q_DECLARE_METATYPE( QgsFieldsProperties::DesignerTreeItemData )

#endif // QGSFIELDSPROPERTIES_H
