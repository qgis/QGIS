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
#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFormLayout>

#include "qgsvectorlayer.h"
#include "ui_qgsattributesformproperties.h"
#include "qgis_app.h"
#include "qgsaddattrdialog.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsaddtaborgroup.h"

class DnDTree;

class APP_EXPORT QgsAttributesFormProperties : public QWidget, private Ui_QgsAttributesFormProperties
{
    Q_OBJECT
  public:
    enum FieldPropertiesRoles
    {
      DnDTreeRole = Qt::UserRole,
      FieldConfigRole
    };

    struct RelationEditorConfiguration
    {
      RelationEditorConfiguration()
        : showLinkButton( true )
        , showUnlinkButton( true )
      {}
      bool showLinkButton;
      bool showUnlinkButton;
    };

    class DnDTreeItemData
    {
      public:
        enum Type
        {
          Field,
          Relation,
          Container
        };

        DnDTreeItemData()
          : mType( Field )
          , mColumnCount( 1 )
          , mShowAsGroupBox( false )
          , mShowLabel( true )
        {}

        DnDTreeItemData( Type type, const QString &name )
          : mType( type )
          , mName( name )
          , mColumnCount( 1 )
          , mShowAsGroupBox( false )
          , mShowLabel( true )
        {}

        QString name() const { return mName; }
        void setName( const QString &name ) { mName = name; }

        Type type() const { return mType; }
        void setType( Type type ) { mType = type; }

        QVariant asQVariant() { return QVariant::fromValue<DnDTreeItemData>( *this ); }

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

      private:
        Type mType;
        QString mName;
        int mColumnCount;
        bool mShowAsGroupBox;
        bool mShowLabel;
        QgsOptionalExpression mVisibilityExpression;
        RelationEditorConfiguration mRelationEditorConfiguration;
    };

  public:
    explicit QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent = nullptr );
    ~QgsAttributesFormProperties();

    void init();
    void apply();
    void onAttributeSelectionChanged();

    void loadAttributeEditorTree( DnDTree *mTree );
    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement *const widgetDef, QTreeWidgetItem *parent, DnDTree* mTree);

  protected:
    QgsVectorLayer *mLayer = nullptr;

    DnDTree *mDragTree = nullptr;
    DnDTree *mDropTree = nullptr;

  private:

  signals:

  private slots:
  void addTabOrGroupButton();
  void removeTabOrGroupButton();
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
    QTreeWidgetItem *addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data );
    QTreeWidgetItem *addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount );

    enum Type
    {
      Drag,
      Drop
    };

    Type type;

  protected:
    virtual void dragMoveEvent( QDragMoveEvent *event ) override;
    virtual void dropEvent( QDropEvent *event ) override;
    virtual bool dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action ) override;
    /* Qt::DropActions supportedDropActions() const;*/

    // QTreeWidget interface
  protected:
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData( const QList<QTreeWidgetItem *> items ) const override;

  private slots:
    void onItemDoubleClicked( QTreeWidgetItem *item, int column );
    void attributeTypeDialog();

  private:
    QgsVectorLayer *mLayer = nullptr;
};


Q_DECLARE_METATYPE( QgsAttributesFormProperties::DnDTreeItemData )

#endif // QGSATTRIBUTESFORMPROPERTIES_H
