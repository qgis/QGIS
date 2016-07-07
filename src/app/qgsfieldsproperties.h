/***************************************************************************
    qgsfieldsproperties.h
    ---------------------
    begin                : September 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias at opengis dot ch
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
#include <QSpinBox>


#include "qgsvectorlayer.h"
#include "ui_qgsfieldspropertiesbase.h"

class DesignerTree;
class DragList;

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
            : mType( Field )
            , mColumnCount( 1 )
            , mShowAsGroupBox( false )
        {}

        DesignerTreeItemData( Type type, const QString& name )
            : mType( type )
            , mName( name )
            , mColumnCount( 1 )
            , mShowAsGroupBox( false )
        {}

        QString name() const { return mName; }
        void setName( const QString& name ) { mName = name; }

        Type type() const { return mType; }
        void setType( Type type ) { mType = type; }

        QVariant asQVariant() { return QVariant::fromValue<DesignerTreeItemData>( *this ); }

        int columnCount() const { return mColumnCount; }
        void setColumnCount( int count ) { mColumnCount = count; }

        bool showAsGroupBox() const;
        void setShowAsGroupBox( bool showAsGroupBox );

      private:
        Type mType;
        QString mName;
        int mColumnCount;
        bool mShowAsGroupBox;
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
        bool mNotNull;
        QString mConstraint;
        QString mConstraintDescription;
        QPushButton* mButton;
        QString mEditorWidgetV2Type;
        QMap<QString, QVariant> mEditorWidgetV2Config;
    };

  public:
    QgsFieldsProperties( QgsVectorLayer *layer, QWidget* parent = nullptr );

    ~QgsFieldsProperties();

    /** Adds an attribute to the table (but does not commit it yet)
    @param field the field to add
    @return false in case of a name conflict, true in case of success */
    bool addAttribute( const QgsField &field );

    /** Creates the a proper item to save from the tree
     * @return A widget definition. Containing another container or the final field
     */
    QgsAttributeEditorElement* createAttributeEditorWidget( QTreeWidgetItem* item, QObject *parent, bool forceGroup = true );

    void init();
    void apply();

    void loadRows();
    void setRow( int row, int idx, const QgsField &field );

    void loadRelations();

    void loadAttributeEditorTree();
    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement* const widgetDef, QTreeWidgetItem* parent );

    /**
     * @brief setEditFormInit set the private ui fields
     * @param editForm
     * @param initFunction
     * @param initCode
     * @param initFilePath
     * @param codeSource
     */
    void setEditFormInit( const QString &editForm,
                          const QString &initFunction,
                          const QString &initCode,
                          const QString &initFilePath,
                          QgsEditFormConfig::PythonInitCodeSource codeSource );

  signals:
    void toggleEditing();

  private slots:
    void on_mAddAttributeButton_clicked();
    void on_mDeleteAttributeButton_clicked();
    void on_mCalculateFieldButton_clicked();
    void onAttributeSelectionChanged();
    void on_pbtnSelectInitFilePath_clicked();
    void on_pbnSelectEditForm_clicked();
    void on_mEditorLayoutComboBox_currentIndexChanged( int index );
    void on_mInitCodeSourceComboBox_currentIndexChanged( int codeSource );
    void attributeAdded( int idx );
    void attributeDeleted( int idx );
    void attributeTypeDialog();

    void on_mAddTabOrGroupButton_clicked();
    void on_mAddItemButton_clicked();
    void on_mRemoveTabGroupItemButton_clicked();
    void on_mMoveDownItem_clicked();
    void on_mMoveUpItem_clicked();

    void attributesListCellChanged( int row, int column );

    void updateExpression();

    /** Editing of layer was toggled */
    void editingToggled();

  protected:
    void updateButtons();

    FieldConfig configForRow( int row );
    void setConfigForRow( int row, const FieldConfig& cfg );

    QList<QgsRelation> mRelations;

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
      attrEditTypeCol,
      attrAliasCol,
      attrTypeCol,
      attrTypeNameCol,
      attrLengthCol,
      attrPrecCol,
      attrCommentCol,
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
      RelNmCol,
      RelColCount
    };

    static QMap< QgsVectorLayer::EditType, QString > editTypeMap;
    static void setupEditTypes();
    static QString editTypeButtonText( QgsVectorLayer::EditType type );

  private:

    void updateFieldRenamingStatus();

};

QDataStream& operator<< ( QDataStream& stream, const QgsFieldsProperties::DesignerTreeItemData& data );
QDataStream& operator>> ( QDataStream& stream, QgsFieldsProperties::DesignerTreeItemData& data );


/**
 * This class overrides mime type handling to be able to work with
 * the drag and drop attribute editor.
 *
 * The mime type is application/x-qgsattributetablefield
 */

class DragList : public QTableWidget
{
    Q_OBJECT

  public:
    explicit DragList( QWidget* parent = nullptr )
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
    Q_OBJECT

  public:
    explicit DesignerTree( QWidget* parent = nullptr );
    QTreeWidgetItem* addItem( QTreeWidgetItem* parent, QgsFieldsProperties::DesignerTreeItemData data );
    QTreeWidgetItem* addContainer( QTreeWidgetItem* parent, const QString& title , int columnCount );

  protected:
    virtual void dragMoveEvent( QDragMoveEvent *event ) override;
    virtual void dropEvent( QDropEvent *event ) override;
    virtual bool dropMimeData( QTreeWidgetItem * parent, int index, const QMimeData * data, Qt::DropAction action ) override;
    /* Qt::DropActions supportedDropActions() const;*/

    // QTreeWidget interface
  protected:
    virtual QStringList mimeTypes() const override;
    virtual QMimeData* mimeData( const QList<QTreeWidgetItem*> items ) const override;

  private slots:
    void onItemDoubleClicked( QTreeWidgetItem* item, int column );
};

Q_DECLARE_METATYPE( QgsFieldsProperties::FieldConfig )
Q_DECLARE_METATYPE( QgsFieldsProperties::DesignerTreeItemData )

#endif // QGSFIELDSPROPERTIES_H

