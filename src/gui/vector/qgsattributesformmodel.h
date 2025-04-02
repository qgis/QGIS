/***************************************************************************
    qgsattributesformmodel.h
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESFORMMODEL_H
#define QGSATTRIBUTESFORMMODEL_H

#include "qgsaddtaborgroup.h"
#include "qgsattributeeditorelement.h"
#include "qgsoptionalexpression.h"
#include "qgsvectorlayer.h"

#include <QAbstractItemModel>
#include <QPushButton>


class QgsAttributeFormTreeData
{
  public:
    enum AttributeFormTreeItemType
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
    class DnDTreeItemData
    {
      public:
        DnDTreeItemData() = default;

        // DnDTreeItemData( const QColor &backgroundColor = QColor() )
        //   : mBackgroundColor( backgroundColor )
        // {}

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
};


class AttributesFormTreeNode
{
  public:
    AttributesFormTreeNode() = default;
    explicit AttributesFormTreeNode( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QString &name, const QString &displayName = QString(), AttributesFormTreeNode *parent = nullptr );
    explicit AttributesFormTreeNode( QgsAttributeFormTreeData::AttributeFormTreeItemType itemType, const QgsAttributeFormTreeData::DnDTreeItemData &data, const QString &name, const QString &displayName = QString(), AttributesFormTreeNode *parent = nullptr );

    AttributesFormTreeNode *child( int number );
    AttributesFormTreeNode *firstTopChild( const QgsAttributeFormTreeData::AttributeFormTreeItemType nodeType, const QString &nodeId ) const;
    AttributesFormTreeNode *firstChildRecursive( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId ) const;

    int childCount() const;
    //bool insertChildren(int position, int count, int columns);
    AttributesFormTreeNode *parent() { return mParent; }
    //bool removeChildren(int position, int count);
    int row() const;
    QVariant data( int role ) const;
    bool setData( int role, const QVariant &value );

    /**
     * Adds a \a child to this item. Takes ownership of the child.
     */
    void addChildItem( std::unique_ptr< AttributesFormTreeNode > &&child );

    void insertChildNode( int position, std::unique_ptr< AttributesFormTreeNode > &&node );

    void deleteChildAtIndex( int index );

    /**
     * Deletes all child items from this item.
     */
    void deleteChildren();

    QgsAttributeFormTreeData::AttributeFormTreeItemType type() const { return mNodeType; }
    QString id() const { return mNodeId; }
    QString name() const { return mName; }

    QString displayName() const { return mDisplayName; }
    void setDisplayName( const QString &displayName ) { mDisplayName = displayName; }

    QIcon icon() const { return mIcon; }
    void setIcon( const QIcon &icon ) { mIcon = icon; }

  private:
    QString mName;
    QString mDisplayName;
    QIcon mIcon;
    QgsAttributeFormTreeData::AttributeFormTreeItemType mNodeType;
    QString mNodeId;
    QgsAttributeFormTreeData::DnDTreeItemData mNodeData;
    QgsAttributeFormTreeData::FieldConfig mFieldConfigData;

    std::vector< std::unique_ptr< AttributesFormTreeNode > > mChildren;
    AttributesFormTreeNode *mParent;
};


/**
 * Abstract class
 */
class QgsAttributesFormModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum NodeRoles
    {
      NodeDataRole = Qt::UserRole,
      NodeFieldConfigRole,
      NodeNameRole,
      NodeIdRole, // Items may have ids, to ease comparison. Used by Relations, fields and actions.
      NodeTypeRole,
    };

    explicit QgsAttributesFormModel( QgsVectorLayer *layer, QObject *parent = nullptr );

    //explicit QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QObject *parent = nullptr );

    ~QgsAttributesFormModel() override;

    // Header:
    //QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    // Basic functionality:
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    // Drag and drop support
    //Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    //bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    //QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    //bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    // Add data:
    //bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    //bool insertColumns( int column, int count, const QModelIndex &parent = QModelIndex() ) override;

    // Remove data:
    //bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    //bool removeColumns( int column, int count, const QModelIndex &parent = QModelIndex() ) override;

    // Other methods
    QModelIndex getFirstTopMatchingModelIndex( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId ) const;
    QModelIndex getFirstRecursiveMatchingModelIndex( const QgsAttributeFormTreeData::AttributeFormTreeItemType &nodeType, const QString &nodeId ) const;
    //QModelIndex getFieldModelIndex( const QString &fieldName ) const;

    //void loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, AttributeFormLayoutTreeItem *parent );

  public slots:
    virtual void populate() = 0;

  protected:
    AttributesFormTreeNode *getItem( const QModelIndex &index ) const;
    std::unique_ptr< AttributesFormTreeNode > mRootItem;
    QgsVectorLayer *mLayer;
};


class QgsAttributesAvailableWidgetsModel : public QgsAttributesFormModel
{
    Q_OBJECT

  public:
    explicit QgsAttributesAvailableWidgetsModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent = nullptr );

    //~QgsAttributesAvailableWidgetsModel() override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    // Basic functionality:
    // QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    // QModelIndex parent( const QModelIndex &index ) const override;

    // int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    // int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    // Drag and drop support
    //Qt::DropActions supportedDropActions() const override;
    //QStringList mimeTypes() const override;
    //QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    //bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    // Add data:
    //bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    //bool insertColumns( int column, int count, const QModelIndex &parent = QModelIndex() ) override;

    // Remove data:
    //bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    //bool removeColumns( int column, int count, const QModelIndex &parent = QModelIndex() ) override;

    // Other methods
    QModelIndex getFieldContainer() const;
    QModelIndex getRelationContainer() const;
    QModelIndex getFieldModelIndex( const QString &fieldName ) const;

  public slots:
    void populate() override;

  private:
    QgsProject *mProject;
};

class QgsAttributesFormLayoutModel : public QgsAttributesFormModel
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QObject *parent = nullptr );

    //~QgsAttributesFormLayoutModel() override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    // Basic functionality:
    // QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    // QModelIndex parent( const QModelIndex &index ) const override;

    // int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    // int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    // Add/remove data:
    //bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRow( int row, const QModelIndex &parent = QModelIndex() );

    // Drag and drop support
    Qt::DropActions supportedDropActions() const override;
    //QStringList mimeTypes() const override;
    //QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    // Other methods
    QList< QgsAddAttributeFormContainerDialog::ContainerPair > getListOfContainers() const;

    /**
     * Adds a new container to \a parent.
     *
     * If no \a parent is set then the container will be forced to be a tab widget.
     */
    void addContainer( QModelIndex &parent, const QString &title, int columnCount, Qgis::AttributeEditorContainerType type );

    void insertNode( const QModelIndex &parent, int row, QString &nodeId, QgsAttributeFormTreeData::AttributeFormTreeItemType nodeType, QString &nodeName, QgsAttributeFormTreeData::DnDTreeItemData nodeData );

  public slots:
    void populate() override;

  signals:
    //! Informs that nodes were inserted (via drop) in the model.
    void nodeDropped( QModelIndex &index );

  private:
    QList< QgsAddAttributeFormContainerDialog::ContainerPair > getRecursiveListOfContainers( AttributesFormTreeNode *parent ) const;
    void loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, AttributesFormTreeNode *parent );
    //AttributeFormLayoutTreeItem *getItem( const QModelIndex &index ) const;

    //QgsVectorLayer *mLayer;
    //QgsProject *mProject;
    //std::unique_ptr< AttributeFormLayoutTreeItem > mRootItem;
};


QDataStream &operator<<( QDataStream &stream, const QgsAttributeFormTreeData::DnDTreeItemData &data );
QDataStream &operator>>( QDataStream &stream, QgsAttributeFormTreeData::DnDTreeItemData &data );


Q_DECLARE_METATYPE( QgsAttributeFormTreeData::RelationEditorConfiguration )
Q_DECLARE_METATYPE( QgsAttributeFormTreeData::FieldConfig )
Q_DECLARE_METATYPE( QgsAttributeFormTreeData::DnDTreeItemData )

#endif // QGSATTRIBUTESFORMMODEL_H
