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
#include "qgsoptionalexpression.h"
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

    struct TextElementEditorConfiguration
    {
        QString text;
    };

    struct SpacerElementEditorConfiguration
    {
        bool drawLine = false;
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
          WidgetType,   //!< In the widget tree, the type of widget
          Action,       //!< Layer action
          TextWidget,   //!< Text widget type, \since QGIS 3.30
          SpacerWidget, //!< Spacer widget type, \since QGIS 3.30
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

        /**
         * Returns the container type.
         *
         * \see setContainerType()
         * \since QGIS 3.32
         */
        Qgis::AttributeEditorContainerType containerType() const;

        /**
         * Sets the container type.
         *
         * \see containerType()
         * \since QGIS 3.32
         */
        void setContainerType( Qgis::AttributeEditorContainerType type );

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

        /**
         * Returns the label style.
         * \see setLabelStyle()
         * \since QGIS 3.26
         */
        const QgsAttributeEditorElement::LabelStyle labelStyle() const;

        /**
         * Sets the label style to \a labelStyle.
         * \see labelStyle()
         * \since QGIS 3.26
         */
        void setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle );

        bool showLabel() const;
        void setShowLabel( bool showLabel );

        /**
         * Returns the horizontal stretch factor for the element.
         *
         * \see setHorizontalStretch()
         * \see verticalStretch()
         *
         * \since QGIS 3.32
         */
        int horizontalStretch() const { return mHorizontalStretch; }

        /**
         * Sets the horizontal \a stretch factor for the element.
         *
         * \see horizontalStretch()
         * \see setVerticalStretch()
         *
         * \since QGIS 3.32
         */
        void setHorizontalStretch( int stretch ) { mHorizontalStretch = stretch; }

        /**
         * Returns the vertical stretch factor for the element.
         *
         * \see setVerticalStretch()
         * \see horizontalStretch()
         *
         * \since QGIS 3.32
         */
        int verticalStretch() const { return mVerticalStretch; }

        /**
         * Sets the vertical \a stretch factor for the element.
         *
         * \see verticalStretch()
         * \see setHorizontalStretch()
         *
         * \since QGIS 3.32
         */
        void setVerticalStretch( int stretch ) { mVerticalStretch = stretch; }

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

        /**
         * Returns the relation editor configuration.
         *
         * \see setRelationEditorConfiguration()
         */
        RelationEditorConfiguration relationEditorConfiguration() const;

        /**
         * Sets the relation editor configuration.
         *
         * \see relationEditorConfiguration()
         */
        void setRelationEditorConfiguration( const RelationEditorConfiguration &relationEditorConfiguration );

        /**
         * Returns the QML editor configuration.
         *
         * \see setQmlElementEditorConfiguration()
         */
        QmlElementEditorConfiguration qmlElementEditorConfiguration() const;

        /**
         * Sets the QML editor configuration.
         *
         * \see qmlElementEditorConfiguration()
         */
        void setQmlElementEditorConfiguration( const QmlElementEditorConfiguration &qmlElementEditorConfiguration );

        /**
         * Returns the HTML editor configuration.
         *
         * \see setHtmlElementEditorConfiguration()
         */
        HtmlElementEditorConfiguration htmlElementEditorConfiguration() const;

        /**
         * Sets the HTML editor configuration.
         *
         * \see htmlElementEditorConfiguration()
         */
        void setHtmlElementEditorConfiguration( const HtmlElementEditorConfiguration &htmlElementEditorConfiguration );

        /**
         * Returns the spacer element configuration
         * \since QGIS 3.30
         */
        SpacerElementEditorConfiguration spacerElementEditorConfiguration() const;

        /**
         * Sets the the spacer element configuration to \a spacerElementEditorConfiguration
         * \since QGIS 3.30
         */
        void setSpacerElementEditorConfiguration( SpacerElementEditorConfiguration spacerElementEditorConfiguration );

        QColor backgroundColor() const;
        void setBackgroundColor( const QColor &backgroundColor );

        /**
         * Returns the editor configuration for text element.
         * \since QGIS 3.30
         */
        TextElementEditorConfiguration textElementEditorConfiguration() const;

        /**
         * Sets the editor configuration for text element to \a textElementEditorConfiguration.
         * \since QGIS 3.30
         */
        void setTextElementEditorConfiguration( const TextElementEditorConfiguration &textElementEditorConfiguration );

      private:
        Type mType = Field;
        QString mName;
        QString mDisplayName;
        int mColumnCount = 1;
        Qgis::AttributeEditorContainerType mContainerType = Qgis::AttributeEditorContainerType::Tab;
        bool mShowLabel = true;
        int mHorizontalStretch = 0;
        int mVerticalStretch = 0;
        QgsOptionalExpression mVisibilityExpression;
        RelationEditorConfiguration mRelationEditorConfiguration;
        QmlElementEditorConfiguration mQmlElementEditorConfiguration;
        HtmlElementEditorConfiguration mHtmlElementEditorConfiguration;
        TextElementEditorConfiguration mTextElementEditorConfiguration;
        SpacerElementEditorConfiguration mSpacerElementEditorConfiguration;
        QColor mBackgroundColor;
        bool mCollapsed = false;
        QgsOptionalExpression mCollapsedExpression;
        QgsAttributeEditorElement::LabelStyle mLabelStyle;
    };


    /**
     * Holds the configuration for a field
     */
    struct FieldConfig
    {
        FieldConfig() = default;
        FieldConfig( QgsVectorLayer *layer, int idx );

        bool mEditable = true;
        bool mLabelOnTop = false;
        bool mReuseLastValues = false;
        QgsFieldConstraints mFieldConstraints;
        QPushButton *mButton = nullptr;
        QString mEditorWidgetType;
        QMap<QString, QVariant> mEditorWidgetConfig;
        QString mAlias;
        QgsPropertyCollection mDataDefinedProperties;
        QString mComment;
        Qgis::FieldDomainSplitPolicy mSplitPolicy = Qgis::FieldDomainSplitPolicy::Duplicate;
        Qgis::FieldDuplicatePolicy mDuplicatePolicy = Qgis::FieldDuplicatePolicy::Duplicate;

        operator QVariant();
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
    void loadAttributeSpecificEditor( QgsAttributesDnDTree *emitter, QgsAttributesDnDTree *receiver );
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
    void loadAttributeTypeDialogFromConfiguration( const FieldConfig cfg );
    void storeAttributeTypeDialog();

    void storeAttributeContainerEdit();
    void loadAttributeContainerEdit();

    void loadInfoWidget( const QString &infoText );

    void copyWidgetConfiguration();
    void pasteWidgetConfiguration();

    QTreeWidgetItem *loadAttributeEditorTreeItem( QgsAttributeEditorElement *widgetDef, QTreeWidgetItem *parent, QgsAttributesDnDTree *tree );

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
};


QDataStream &operator<<( QDataStream &stream, const QgsAttributesFormProperties::DnDTreeItemData &data );
QDataStream &operator>>( QDataStream &stream, QgsAttributesFormProperties::DnDTreeItemData &data );


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
    QTreeWidgetItem *addItem( QTreeWidgetItem *parent, const QgsAttributesFormProperties::DnDTreeItemData &data, int index = -1, const QIcon &icon = QIcon() );

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
    void selectFirstMatchingItem( const QgsAttributesFormProperties::DnDTreeItemData &data );

  protected:
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    bool dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action ) override;
    /* Qt::DropActions supportedDropActions() const;*/

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


Q_DECLARE_METATYPE( QgsAttributesFormProperties::RelationEditorConfiguration )
Q_DECLARE_METATYPE( QgsAttributesFormProperties::FieldConfig )
Q_DECLARE_METATYPE( QgsAttributesFormProperties::DnDTreeItemData )

#endif // QGSATTRIBUTESFORMPROPERTIES_H
