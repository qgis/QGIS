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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgsaddtaborgroup.h"
#include "qgsattributeeditorelement.h"
#include "qgsoptionalexpression.h"
#include "qgsvectorlayer.h"

#include <QAbstractItemModel>
#include <QPushButton>

/**
 * \brief Class to describe editor data contained within a QgsAttributesFormModel.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormTreeData
{
  public:
    /**
     * Custom node types.
     *
     * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties::DnDTreeItemData::Type
     * \since QGIS 3.44
     */
    enum AttributesFormTreeNodeType
    {
      Field,
      Relation,
      Container, //!< Container for the form
      QmlWidget,
      HtmlWidget,
      WidgetType,   //!< In the available widgets tree, the type of widget
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
     * \brief Main class to store and transfer editor data contained within a QgsAttributesFormModel.
     *
     * \ingroup gui
     * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties::DnDTreeItemData
     * \since QGIS 3.44
     */
    class DnDTreeNodeData
    {
      public:
        DnDTreeNodeData() = default;

        //! Allows direct construction of QVariants from DnDTreeNodeData.
        operator QVariant() { return QVariant::fromValue<DnDTreeNodeData>( *this ); }

        //! Returns the number of columns in a container.
        int columnCount() const { return mColumnCount; }

        //! Sets the number of columns for a container.
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

        //! Returns whether the widget's label is to be shown.
        bool showLabel() const;

        //! Sets whether the label for the widget should be shown.
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

        /**
         * Returns the expression to control the visibility status of a container.
         *
         * \see setVisibilityExpression()
         */
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

        /**
         * Returns the background color of a container.
         *
         * \see setBackgroundColor()
         */
        QColor backgroundColor() const;

        /**
         * Sets the background color of a container.
         *
         * \see backgroundColor()
         */
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


/**
 * \brief Class to hold parent-child relations as well as node data contained within a QgsAttributesFormModel.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormTreeNode
{
  public:
    QgsAttributesFormTreeNode() = default;

    /**
     * Constructor for QgsAttributesFormTreeNode, with the given \a nodeType and
     * the given \a name.
     *
     * If \a displayName is specified then it will be used as the preferred display
     * name for the node. If it is not specified, \a name will be displayed instead.
     *
     * If \a parent is specified, the node will be added as child of the parent node.
     * If it is not specified then it will be set when manually added to another node.
     */
    explicit QgsAttributesFormTreeNode( QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QString &name, const QString &displayName = QString(), QgsAttributesFormTreeNode *parent = nullptr );

    /**
     * Constructor for QgsAttributesFormTreeNode, with the given \a nodeType, the
     * given \a data and the given \a name.
     *
     * If \a displayName is specified then it will be used as the preferred display
     * name for the node. If it is not specified, \a name will be displayed instead.
     *
     * If \a parent is specified, the node will be added as child of the parent node.
     * If it is not specified then it will be set when manually added to another node.
     */
    explicit QgsAttributesFormTreeNode( QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QgsAttributesFormTreeData::DnDTreeNodeData &data, const QString &name, const QString &displayName = QString(), QgsAttributesFormTreeNode *parent = nullptr );

    /**
     * \brief Access the child node located at \a row position.
     *
     * If there is no child node for the given \a row position, a NULLPTR is returned.
     */
    QgsAttributesFormTreeNode *child( int row );

    /**
     * \brief Access the first top-level child node that matches \a nodeType and \a nodeId.
     *
     * If there is no top-level matching child node, a NULLPTR is returned.
     */
    QgsAttributesFormTreeNode *firstTopChild( const QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, const QString &nodeId ) const;

    /**
     * \brief Access the first child node that matches \a nodeType and \a nodeId, recursively.
     *
     * If there is no matching child node in the whole node hierarchy, a NULLPTR is returned.
     */
    QgsAttributesFormTreeNode *firstChildRecursive( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId ) const;

    /**
     * Returns the number of children nodes for the given node.
     */
    int childCount() const;

    /**
     * Returns the parent object of the current node.
     *
     * If node is a top-level node, its parent is the root object and a NULLPTR is returned.
     */
    QgsAttributesFormTreeNode *parent() { return mParent; }

    /**
     * Returns the position of the current node regarding its parent.
     */
    int row() const;

    /**
     * Returns the data stored in the current node, corresponding to the given \a role.
     */
    QVariant data( int role ) const;

    /**
     * Stores a data \a value in a given \a role inside the current node.
     */
    bool setData( int role, const QVariant &value );

    /**
     * Appends a \a child to this node. Takes ownership of the child.
     */
    void addChildNode( std::unique_ptr< QgsAttributesFormTreeNode > &&child );

    /**
     * Inserts a child \a node to the current node at a given \a position. Takes ownership of the child node.
     */
    void insertChildNode( int position, std::unique_ptr< QgsAttributesFormTreeNode > &&node );

    /**
     * Deletes the child of the current node placed at the given \a index.
     */
    void deleteChildAtIndex( int index );

    /**
     * Deletes all child nodes from this node.
     */
    void deleteChildren();

    /**
     * Returns the type of the current node.
     */
    QgsAttributesFormTreeData::AttributesFormTreeNodeType type() const { return mNodeType; }

    /**
     * Returns the id of the current node.
     */
    QString id() const { return mNodeId; }

    /**
     * Returns the name of the current node.
     */
    QString name() const { return mName; }

    /**
     * Returns the display name of the current node.
     */
    QString displayName() const { return mDisplayName; }

    /**
     * Returns the icon of the current node.
     *
     * \see setIcon()
     */
    QIcon icon() const { return mIcon; }

    /**
     * Sets an icon for the current node.
     *
     * \see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

  private:
    QString mName = QString();
    QString mDisplayName = QString();
    QIcon mIcon;
    QgsAttributesFormTreeData::AttributesFormTreeNodeType mNodeType = QgsAttributesFormTreeData::Field;
    QString mNodeId = QString();
    QgsAttributesFormTreeData::DnDTreeNodeData mNodeData;
    QgsAttributesFormTreeData::FieldConfig mFieldConfigData;

    std::vector< std::unique_ptr< QgsAttributesFormTreeNode > > mChildren;
    QgsAttributesFormTreeNode *mParent = nullptr;
};


/**
 * \brief Abstract class for tree views allowing for configuration of attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties::FieldPropertiesRoles
     * \since QGIS 3.44
     */
    enum NodeRoles
    {
      NodeDataRole = Qt::UserRole, //!< Prior to QGIS 3.44, this was available as DnDTreeRole
      NodeFieldConfigRole,         //!< Prior to QGIS 3.44, this was available as FieldConfigRole
      NodeNameRole,                //!< Prior to QGIS 3.44, this was available as FieldNameRole
      NodeIdRole,                  //!< Nodes may have ids to ease comparison. Used by Relations, fields and actions.
      NodeTypeRole,
      NodeDisplayRole,
    };

    /**
     * Constructor for QgsAttributesFormModel, with the given \a layer object and
     * the given \a parent.
     *
     * The \a layer is the data source to populate the model.
     */
    explicit QgsAttributesFormModel( QgsVectorLayer *layer, QObject *parent = nullptr );

    ~QgsAttributesFormModel() override;

    // Basic functionality
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    // Drag and drop support (common methods)
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;

    // Other methods
    /**
     * Returns the first top-level model index that matches the given \a nodeType and \a nodeId.
     *
     * If there is no matching top-level model index an invalid index is returned.
     */
    QModelIndex firstTopMatchingModelIndex( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId ) const;

    /**
     * Returns the first model index that matches the given \a nodeType and \a nodeId, recursively.
     *
     * If there is no matching model index in the whole hierarchy an invalid index is returned.
     */
    QModelIndex firstRecursiveMatchingModelIndex( const QgsAttributesFormTreeData::AttributesFormTreeNodeType &nodeType, const QString &nodeId ) const;

  public slots:
    /**
     * Populates the model with initial data read from the layer.
     */
    virtual void populate() = 0;

  protected:
    /**
     * Returns the underlying node that corresponds to the given \a index.
     *
     * If the given \a index is not valid the root node is returned.
     */
    QgsAttributesFormTreeNode *nodeForIndex( const QModelIndex &index ) const;

    std::unique_ptr< QgsAttributesFormTreeNode > mRootNode;
    QgsVectorLayer *mLayer;
};


/**
 * \brief Class for available widgets when configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesAvailableWidgetsModel : public QgsAttributesFormModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesAvailableWidgetsModel, with the given \a parent.
     *
     * The given \a layer and \a project are the data sources to populate the model.
     */
    explicit QgsAttributesAvailableWidgetsModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    // Drag and drop support
    Qt::DropActions supportedDragActions() const override;

    // Other methods

    /**
     * Returns the field container in this model, expected to be placed at the first top-level row.
     *
     * If there is no field container set, an invalid index is returned.
     */
    QModelIndex fieldContainer() const;

    /**
     * Returns the relation container in this model, expected to be placed at the second top-level row.
     *
     * If there is no relation container set, an invalid index is returned.
     */
    QModelIndex relationContainer() const;

    /**
     * Returns the model index that corresponds to the field with the given \a fieldName.
     */
    QModelIndex fieldModelIndex( const QString &fieldName ) const;

  public slots:
    void populate() override;

  private:
    QgsProject *mProject;
};


/**
 * \brief Class for form layouts when configuring attributes forms via drag and drop designer.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormLayoutModel : public QgsAttributesFormModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormLayoutModel, with the given \a parent.
     *
     * The given \a layer is the data source to populate the model.
     */
    explicit QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QObject *parent = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    // Basic functionality
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    // Add/remove data
    /**
     * Removes the index located at \a row within the given \a parent.
     */
    bool removeRow( int row, const QModelIndex &parent = QModelIndex() );

    // Drag and drop support
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    // Other methods
    /**
     * Returns a list of containers stored in the model, structured as pairs (name, container model index).
     */
    QList< QgsAddAttributeFormContainerDialog::ContainerPair > listOfContainers() const;

    /**
     * Adds a new container to \a parent.
     *
     * If no \a parent is set then the container will be forced to be a tab widget.
     */
    void addContainer( QModelIndex &parent, const QString &title, int columnCount, Qgis::AttributeEditorContainerType type );

    /**
     * Updates the aliases of all matching fields in the model.
     *
     * Required because a field might appear several times in the form layout.
     *
     * \param fieldName Name of the field to search
     * \param fieldAlias Alias to be set to matching fields
     */
    void updateAliasForFieldNodes( const QString &fieldName, const QString &fieldAlias );

    /**
     * Inserts a new child to \a parent model index at the given \a row position.
     *
     * The child is constructed from the given \a nodeId, \a nodeType, \a nodeName and \a nodeData.
     */
    void insertChild( const QModelIndex &parent, int row, QString &nodeId, QgsAttributesFormTreeData::AttributesFormTreeNodeType nodeType, QString &nodeName, QgsAttributesFormTreeData::DnDTreeNodeData nodeData );

    /**
     * Returns whether field aliases are preferred over field names as item text.
     *
     * \see setShowAliases()
     */
    bool showAliases() const;

    /**
     * Sets whether field aliases should be preferred over field names as item text.
     *
     * \see showAliases()
     */
    void setShowAliases( bool show );

  public slots:
    void populate() override;

  signals:
    //! Informs that nodes were inserted (via drop) in the model from another model.
    void externalNodeDropped( QModelIndex &index );
    //! Informs that nodes were moved (via drop) in the model from the same model.
    void internalNodeDropped( QModelIndex &index );

  private:
    void updateAliasForFieldNodesRecursive( QgsAttributesFormTreeNode *parent, const QString &fieldName, const QString &fieldAlias );
    QList< QgsAddAttributeFormContainerDialog::ContainerPair > recursiveListOfContainers( QgsAttributesFormTreeNode *parent ) const;
    void loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, QgsAttributesFormTreeNode *parent );

    bool mShowAliases = false;
};


QDataStream &operator<<( QDataStream &stream, const QgsAttributesFormTreeData::DnDTreeNodeData &data );
QDataStream &operator>>( QDataStream &stream, QgsAttributesFormTreeData::DnDTreeNodeData &data );


Q_DECLARE_METATYPE( QgsAttributesFormTreeData::RelationEditorConfiguration )
Q_DECLARE_METATYPE( QgsAttributesFormTreeData::FieldConfig )
Q_DECLARE_METATYPE( QgsAttributesFormTreeData::DnDTreeNodeData )

#endif // QGSATTRIBUTESFORMMODEL_H
