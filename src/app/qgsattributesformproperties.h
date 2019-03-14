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

#include "ui_qgsattributesformproperties.h"
#include "qgis_app.h"
#include "qgsaddattrdialog.h"
#include "qgslogger.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsaddtaborgroup.h"
#include "qgsattributetypedialog.h"
#include "qgsattributerelationedit.h"
#include "qgsattributesforminitcode.h"
#include "qgsgui.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgsrelationmanager.h"

class DnDTree;

class APP_EXPORT QgsAttributesFormProperties : public QWidget, private Ui_QgsAttributesFormProperties
{
    Q_OBJECT

  public:

    enum FieldPropertiesRoles
    {
      DnDTreeRole = Qt::UserRole,
      FieldConfigRole,
      RelationConfigRole,
      FieldNameRole
    };

    struct RelationEditorConfiguration
    {
      bool showLinkButton = true;
      bool showUnlinkButton = true;
    };

    struct QmlElementEditorConfiguration
    {
      QString qmlCode;
    };

    class DnDTreeItemData : public QTreeWidgetItem
    {
      public:
        enum Type
        {
          Field,
          Relation,
          Container,
          QmlWidget
        };

        //do we need that
        DnDTreeItemData() = default;

        DnDTreeItemData( Type type, const QString &name, const QString &displayName, const QColor &backgroundColor = QColor() )
          : mType( type )
          , mName( name )
          , mDisplayName( displayName )
          , mBackgroundColor( backgroundColor )
        {}

        QString name() const { return mName; }
        void setName( const QString &name ) { mName = name; }

        QString displayName() const { return mDisplayName; }
        void setDisplayName( const QString &displayName ) { mDisplayName = displayName; }

        Type type() const { return mType; }
        void setType( Type type ) { mType = type; }

        operator QVariant() { return QVariant::fromValue<DnDTreeItemData>( *this ); }

        int columnCount() const { return mColumnCount; }
        void setColumnCount( int count ) { mColumnCount = count; }

        bool showAsGroupBox() const;
        void setShowAsGroupBox( bool showAsGroupBox );

        bool showLabel() const;
        void setShowLabel( bool showLabel );

        QgsOptionalExpression visibilityExpression() const;
        void setVisibilityExpression( const QgsOptionalExpression &visibilityExpression );

        RelationEditorConfiguration relationEditorConfiguration() const;
        void setRelationEditorConfiguration( RelationEditorConfiguration relationEditorConfiguration );

        QmlElementEditorConfiguration qmlElementEditorConfiguration() const;
        void setQmlElementEditorConfiguration( QmlElementEditorConfiguration qmlElementEditorConfiguration );

        QColor backgroundColor() const;
        void setBackgroundColor( const QColor &backgroundColor );

      private:
        Type mType = Field;
        QString mName;
        QString mDisplayName;

        int mColumnCount = 1;
        bool mShowAsGroupBox = false;
        bool mShowLabel = true;
        QgsOptionalExpression mVisibilityExpression;
        RelationEditorConfiguration mRelationEditorConfiguration;
        QmlElementEditorConfiguration mQmlElementEditorConfiguration;
        QColor mBackgroundColor;
    };


    /**
     * Holds the configuration for a field
     */
    struct FieldConfig
    {
      FieldConfig() = default;
      FieldConfig( QgsVectorLayer *layer, int idx );

      bool mEditable =  true ;
      bool mEditableEnabled =  true ;
      bool mLabelOnTop =  false ;
      QgsFieldConstraints mFieldConstraints;
      QPushButton *mButton = nullptr;
      QString mEditorWidgetType;
      QMap<QString, QVariant> mEditorWidgetConfig;
      QString mAlias;
      QString mComment;

      operator QVariant();
    };


    /**
     * Holds the configuration for a relation
     */
    struct RelationConfig
    {
      RelationConfig();
      RelationConfig( QgsVectorLayer *layer, const QString &relationId );

      QVariant mCardinality;

      operator QVariant();
    };

  public:
    explicit QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent = nullptr );

    QgsAttributeEditorElement *createAttributeEditorWidget( QTreeWidgetItem *item, QgsAttributeEditorElement *parent, bool forceGroup = true );

    void init();
    void apply();

    void onAttributeSelectionChanged();

    void loadRelations();

    void initAvailableWidgetsTree();
    void initFormLayoutTree();
    void initLayoutConfig();
    void initInitPython();
    void initSuppressCombo();

  protected:
    void updateButtons();

    RelationConfig configForRelation( const QString &relationName );

    //QList<QgsRelation> mRelations;
    QgsVectorLayer *mLayer = nullptr;

    DnDTree *mAvailableWidgetsTree = nullptr;
    DnDTree *mFormLayoutTree = nullptr;

    QgsAttributeTypeDialog *mAttributeTypeDialog = nullptr;
    QgsAttributeRelationEdit *mAttributeRelationEdit = nullptr;

  private:
    void loadAttributeTypeDialog();
    void storeAttributeTypeDialog( );

    void loadAttributeRelationEdit();
    void storeAttributeRelationEdit( );

    QgsEditFormConfig::PythonInitCodeSource mInitCodeSource = QgsEditFormConfig::CodeSourceNone;
    QString mInitFunction;
    QString mInitFilePath;
    QString mInitCode;

    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement *widgetDef, QTreeWidgetItem *parent, DnDTree *tree );

  private slots:
    void addTabOrGroupButton();
    void removeTabOrGroupButton();
    void mEditorLayoutComboBox_currentIndexChanged( int index );
    void pbnSelectEditForm_clicked();
    void mTbInitCode_clicked();
};


QDataStream &operator<< ( QDataStream &stream, const QgsAttributesFormProperties::DnDTreeItemData &data );
QDataStream &operator>> ( QDataStream &stream, QgsAttributesFormProperties::DnDTreeItemData &data );


/**
 * This class overrides mime type handling to be able to work with
 * the drag and drop attribute editor.
 *
 * The mime type is application/x-qgsattributetablefield
 *
 * Graphical representation for the attribute editor drag and drop editor
 */

class DnDTree : public QTreeWidget
{
    Q_OBJECT

  public:
    explicit DnDTree( QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Adds a new item to a \a parent. If \a index is -1, the item is added to the end of the parent's existing children.
     * Otherwise it is inserted at the specified \a index.
     */
    QTreeWidgetItem *addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data, int index = -1 );
    QTreeWidgetItem *addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount );

    enum Type
    {
      Drag,
      Drop
    };


    Type type() const;
    void setType( DnDTree::Type value );

  protected:
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    bool dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action ) override;
    /* Qt::DropActions supportedDropActions() const;*/

    // QTreeWidget interface
  protected:
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QList<QTreeWidgetItem *> items ) const override;


  private slots:
    void onItemDoubleClicked( QTreeWidgetItem *item, int column );

  private:
    QgsVectorLayer *mLayer = nullptr;
    Type mType = DnDTree::Type::Drag;
};


Q_DECLARE_METATYPE( QgsAttributesFormProperties::FieldConfig )
Q_DECLARE_METATYPE( QgsAttributesFormProperties::RelationConfig )
Q_DECLARE_METATYPE( QgsAttributesFormProperties::DnDTreeItemData )

#endif // QGSATTRIBUTESFORMPROPERTIES_H
