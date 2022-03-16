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

#include "ui_qgsattributesformproperties.h"
#include "qgis_gui.h"
#include "qgsaddattrdialog.h"
#include "qgslogger.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcalculator.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsattributesforminitcode.h"
#include "qgsgui.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgsrelationmanager.h"
#include "qgsattributeeditorrelation.h"


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

    struct RelationEditorConfiguration
    {
      operator QVariant();

      QString mRelationWidgetType;
      QVariantMap mRelationWidgetConfig;
      QVariant nmRelationId;
      bool forceSuppressFormPopup = false;
      QString label;
    };

    struct QmlElementEditorConfiguration
    {
      QString qmlCode;
    };

    struct HtmlElementEditorConfiguration
    {
      QString htmlCode;
    };

    /**
     * \ingroup gui
     * \class DnDTreeItemData
     */
    class DnDTreeItemData : public QTreeWidgetItem
    {
      public:
        enum Type
        {
          Field,
          Relation,
          Container, //!< Container for the form
          QmlWidget,
          HtmlWidget,
          WidgetType, //!< In the widget tree, the type of widget
          Action //!< Layer action
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

        /**
         * For group box containers  returns if this group box is collapsed.
         *
         * \returns TRUE if the group box is collapsed, FALSE otherwise.
         * \see collapsed()
         * \see setCollapsed()
         * \since QGIS 3.26
         */
        bool collapsed() const { return mCollapsed; };

        /**
         * For group box containers  sets if this group box is \a collapsed.
         *
         * \see collapsed()
         * \see setCollapsed()
         * \since QGIS 3.26
         */
        void setCollapsed( bool collapsed ) { mCollapsed = collapsed; };

        bool showLabel() const;
        void setShowLabel( bool showLabel );

        QgsOptionalExpression visibilityExpression() const;

        /**
         * Sets the optional \a visibilityExpression that dynamically controls the visibility status of a container.
         *
         * \see visibilityExpression()
         * \since QGIS 3.26
         */
        void setVisibilityExpression( const QgsOptionalExpression &visibilityExpression );

        /**
         * Returns the optional expression that dynamically controls the collapsed status of a group box container.
         *
         * \see collapsed()
         * \see setCollapsed()
         * \see setCollapsedExpression()
         * \since QGIS 3.26
         */
        QgsOptionalExpression collapsedExpression() const;

        /**
         * Sets the optional \a collapsedExpression that dynamically controls the collapsed status of a group box container.
         *
         * \see collapsed()
         * \see setCollapsed()
         * \see collapsedExpression()
         * \since QGIS 3.26
         */
        void setCollapsedExpression( const QgsOptionalExpression &collapsedExpression );

        RelationEditorConfiguration relationEditorConfiguration() const;
        void setRelationEditorConfiguration( RelationEditorConfiguration relationEditorConfiguration );

        QmlElementEditorConfiguration qmlElementEditorConfiguration() const;
        void setQmlElementEditorConfiguration( QmlElementEditorConfiguration qmlElementEditorConfiguration );

        HtmlElementEditorConfiguration htmlElementEditorConfiguration() const;
        void setHtmlElementEditorConfiguration( HtmlElementEditorConfiguration htmlElementEditorConfiguration );

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
        HtmlElementEditorConfiguration mHtmlElementEditorConfiguration;
        QColor mBackgroundColor;
        bool mCollapsed = false;
        QgsOptionalExpression mCollapsedExpression;
    };


    /**
     * Holds the configuration for a field
     */
    struct FieldConfig
    {
      FieldConfig() = default;
      FieldConfig( QgsVectorLayer *layer, int idx );

      bool mEditable = true;
      bool mEditableEnabled = true;
      bool mLabelOnTop = false;
      bool mReuseLastValues = false;
      QgsFieldConstraints mFieldConstraints;
      QPushButton *mButton = nullptr;
      QString mEditorWidgetType;
      QMap<QString, QVariant> mEditorWidgetConfig;
      QString mAlias;
      QgsPropertyCollection mDataDefinedProperties;
      QString mComment;

      operator QVariant();
    };

  public:
    explicit QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent = nullptr );

    QgsAttributeEditorElement *createAttributeEditorWidget( QTreeWidgetItem *item, QgsAttributeEditorElement *parent, bool forceGroup = true );

    void init();
    void apply();


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

    QgsAttributeWidgetEdit *mAttributeWidgetEdit = nullptr;
    QgsAttributeTypeDialog *mAttributeTypeDialog = nullptr;
    QgsAttributeFormContainerEdit *mAttributeContainerEdit = nullptr;
    QLabel *mInfoTextWidget = nullptr;

  private slots:

    void onInvertSelectionButtonClicked( bool checked );
    void loadAttributeSpecificEditor( QgsAttributesDnDTree *emitter, QgsAttributesDnDTree *receiver );
    void onAttributeSelectionChanged();
    void onFormLayoutSelectionChanged();

  private:
    //! this will clean the right panel
    void clearAttributeTypeFrame();

    void loadAttributeWidgetEdit();
    void storeAttributeWidgetEdit();

    void loadAttributeTypeDialog();
    void storeAttributeTypeDialog( );

    void storeAttributeContainerEdit();
    void loadAttributeContainerEdit();

    void loadInfoWidget( const QString &infoText );

    QgsEditFormConfig::PythonInitCodeSource mInitCodeSource = QgsEditFormConfig::CodeSourceNone;
    QString mInitFunction;
    QString mInitFilePath;
    QString mInitCode;

    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement *widgetDef, QTreeWidgetItem *parent, QgsAttributesDnDTree *tree );

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
class GUI_EXPORT QgsAttributesDnDTree : public QTreeWidget
{
    Q_OBJECT

  public:
    explicit QgsAttributesDnDTree( QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Adds a new item to a \a parent. If \a index is -1, the item is added to the end of the parent's existing children.
     * Otherwise it is inserted at the specified \a index.
     */
    QTreeWidgetItem *addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data, int index = -1, const QIcon &icon = QIcon() );
    QTreeWidgetItem *addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount );

    enum Type
    {
      Drag,
      Drop
    };


    Type type() const;
    void setType( QgsAttributesDnDTree::Type value );

  public slots:
    void selectFirstMatchingItem( const QgsAttributesFormProperties::DnDTreeItemData &data );

  protected:
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    bool dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action ) override;
    /* Qt::DropActions supportedDropActions() const;*/

    // QTreeWidget interface
  protected:
    QStringList mimeTypes() const override;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMimeData *mimeData( const QList<QTreeWidgetItem *> items ) const override;
#else
    QMimeData *mimeData( const QList<QTreeWidgetItem *> &items ) const override;
#endif

  private slots:
    void onItemDoubleClicked( QTreeWidgetItem *item, int column );

  private:
    QgsVectorLayer *mLayer = nullptr;
    Type mType = QgsAttributesDnDTree::Type::Drag;
};


Q_DECLARE_METATYPE( QgsAttributesFormProperties::RelationEditorConfiguration )
Q_DECLARE_METATYPE( QgsAttributesFormProperties::FieldConfig )
Q_DECLARE_METATYPE( QgsAttributesFormProperties::DnDTreeItemData )

#endif // QGSATTRIBUTESFORMPROPERTIES_H
