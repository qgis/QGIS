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

#include "qgsaction.h"
#include "qgsaddtaborgroup.h"
#include "qgsattributeeditorelement.h"
#include "qgsoptionalexpression.h"
#include "qgsvectorlayer.h"

#include <QAbstractItemModel>
#include <QPushButton>
#include <QSortFilterProxyModel>

/**
 * \brief Describes editor data contained in a QgsAttributesFormModel.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormData
{
  public:
    /**
     * Custom item types.
     *
     * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties::DnDTreeItemData::Type
     * \since QGIS 3.44
     */
    enum AttributesFormItemType
    {
      Field,        //!< Vector layer field
      Relation,     //!< Relation between two vector layers
      Container,    //!< Container for the form, which may be tab, group or row
      QmlWidget,    //!< QML widget
      HtmlWidget,   //!< HTML widget
      WidgetType,   //!< In the available widgets tree, the type of widget
      Action,       //!< Layer action
      TextWidget,   //!< Text widget type, \since QGIS 3.30
      SpacerWidget, //!< Spacer widget type, \since QGIS 3.30
    };

    /**
     * Holds the configuration for a field
     */
    struct GUI_EXPORT FieldConfig
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
        Qgis::FieldDomainMergePolicy mMergePolicy = Qgis::FieldDomainMergePolicy::DefaultValue;

        operator QVariant();
    };

    struct GUI_EXPORT RelationEditorConfiguration
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
     * \brief Main class to store and transfer editor data contained in a QgsAttributesFormModel.
     *
     * \ingroup gui
     * \note Prior to QGIS 3.44 this was available as QgsAttributesFormProperties::DnDTreeItemData
     * \since QGIS 3.44
     */
    class GUI_EXPORT AttributeFormItemData
    {
      public:
        AttributeFormItemData() = default;

        //! Allows direct construction of QVariants from AttributeFormItemData.
        operator QVariant() { return QVariant::fromValue<AttributeFormItemData>( *this ); }

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
 * \brief Holds parent-child relations as well as item data contained in a QgsAttributesFormModel.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormItem : public QObject
{
    Q_OBJECT

  public:
    QgsAttributesFormItem() = default;

    /**
     * Constructor for QgsAttributesFormItem, with the given \a itemType and
     * the given \a name.
     *
     * If \a displayName is specified then it will be used as the preferred display
     * name for the item. If it is not specified, \a name will be displayed instead.
     *
     * If \a parent is specified, the item will be added as child of the parent item.
     * If it is not specified then it will be set when manually added to another item.
     */
    explicit QgsAttributesFormItem( QgsAttributesFormData::AttributesFormItemType itemType, const QString &name, const QString &displayName = QString(), QgsAttributesFormItem *parent = nullptr );

    /**
     * Constructor for QgsAttributesFormItem, with the given \a itemType, the
     * given \a data and the given \a name.
     *
     * If \a displayName is specified then it will be used as the preferred display
     * name for the item. If it is not specified, \a name will be displayed instead.
     *
     * If \a parent is specified, the item will be added as child of the parent item.
     * If it is not specified then it will be set when manually added to another item.
     */
    explicit QgsAttributesFormItem( QgsAttributesFormData::AttributesFormItemType itemType, const QgsAttributesFormData::AttributeFormItemData &data, const QString &name, const QString &displayName = QString(), QgsAttributesFormItem *parent = nullptr );

    /**
     * \brief Access the child item located at \a row position.
     *
     * If there is no child item for the given \a row position, a NULLPTR is returned.
     */
    QgsAttributesFormItem *child( int row );

    /**
     * \brief Access the first top-level child item that matches \a itemType and \a itemId.
     *
     * If there is no top-level matching child item, a NULLPTR is returned.
     */
    QgsAttributesFormItem *firstTopChild( const QgsAttributesFormData::AttributesFormItemType itemType, const QString &itemId ) const;

    /**
     * \brief Access the first child item that matches \a itemType and \a itemId, recursively.
     *
     * If there is no matching child item in the whole item hierarchy, a NULLPTR is returned.
     */
    QgsAttributesFormItem *firstChildRecursive( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId ) const;

    /**
     * Returns the number of children items for the given item.
     */
    int childCount() const;

    /**
     * Returns the parent object of the item.
     *
     * If item is a top-level item, its parent is the root object and a NULLPTR is returned.
     */
    QgsAttributesFormItem *parent() { return mParent; }

    /**
     * Returns the position of the item regarding its parent.
     */
    int row() const;

    /**
     * Returns the data stored in the item, corresponding to the given \a role.
     */
    QVariant data( int role ) const;

    /**
     * Stores a data \a value in a given \a role inside the item.
     */
    bool setData( int role, const QVariant &value );

    /**
     * Appends a \a child to this item. Takes ownership of the child.
     */
    void addChild( std::unique_ptr< QgsAttributesFormItem > &&child );

    /**
     * Inserts a child \a item to the item at a given \a position. Takes ownership of the child item.
     */
    void insertChild( int position, std::unique_ptr< QgsAttributesFormItem > &&item );

    /**
     * Deletes the child of the item placed at the given \a index.
     */
    void deleteChildAtIndex( int index );

    /**
     * Deletes all child items from this item.
     */
    void deleteChildren();

    /**
     * Returns the type of the item.
     */
    QgsAttributesFormData::AttributesFormItemType type() const { return mType; }

    /**
     * Returns the id of the item.
     */
    QString id() const { return mId; }

    /**
     * Returns the name of the item.
     */
    QString name() const { return mName; }

    /**
     * Returns the display name of the item.
     */
    QString displayName() const { return mDisplayName; }

    /**
     * Returns the icon of the item.
     *
     * \see setIcon()
     */
    QIcon icon() const { return mIcon; }

    /**
     * Sets an icon for the item.
     *
     * \see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

  private:
    QString mName = QString();
    QString mDisplayName = QString();
    QIcon mIcon;
    QgsAttributesFormData::AttributesFormItemType mType = QgsAttributesFormData::Field;
    QString mId = QString();
    QgsAttributesFormData::AttributeFormItemData mData;
    QgsAttributesFormData::FieldConfig mFieldConfigData;

    std::vector< std::unique_ptr< QgsAttributesFormItem > > mChildren;
    QgsAttributesFormItem *mParent = nullptr;

    Q_DISABLE_COPY( QgsAttributesFormItem )
};


/**
 * \brief Abstract class for tree models allowing for configuration of attributes forms.
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
    enum ItemRoles
    {
      ItemDataRole = Qt::UserRole, //!< Prior to QGIS 3.44, this was available as DnDTreeRole
      ItemFieldConfigRole,         //!< Prior to QGIS 3.44, this was available as FieldConfigRole
      ItemNameRole,                //!< Prior to QGIS 3.44, this was available as FieldNameRole
      ItemIdRole,                  //!< Items may have ids to ease comparison. Used by Relations, fields, actions and containers.
      ItemTypeRole,                //!< Type of the item
      ItemDisplayRole,             //!< Display text for the item
    };

    /**
     * Constructor for QgsAttributesFormModel, with the given \a parent.
     *
     * The given \a layer and \a project are data sources to populate the model.
     */
    explicit QgsAttributesFormModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent = nullptr );

    ~QgsAttributesFormModel() override;

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Returns the first top-level model index that matches the given \a itemType and \a itemId.
     *
     * If there is no matching top-level model index an invalid index is returned.
     */
    QModelIndex firstTopMatchingModelIndex( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId ) const;

    /**
     * Returns the first model index that matches the given \a itemType and \a itemId, recursively.
     *
     * If there is no matching model index in the whole hierarchy an invalid index is returned.
     */
    QModelIndex firstRecursiveMatchingModelIndex( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId ) const;

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
    /**
     * Populates the model with initial data read from the layer.
     */
    virtual void populate() = 0;

  protected:
    /**
     * Returns the underlying item that corresponds to the given \a index.
     *
     * If the given \a index is not valid the root item is returned.
     */
    QgsAttributesFormItem *itemForIndex( const QModelIndex &index ) const;

    /**
     * Auxiliary function to sort indexes, returning true if index \a a is less than index \a b.
     *
     * Regardless of items depth, an index nearer to the root (imagine all items in a top-down
     * flat list) should be returned first when sorting.
     *
     * For instance, index 0-19-2 (where 0 is the grandparent position and 19 the parent position)
     * will be less than index 1-0.
     *
     * \see rootToLeafPath()
     */
    bool indexLessThan( const QModelIndex &a, const QModelIndex &b ) const;

    /**
     * Returns a QVector of iterative positions from root item to the given \a item.
     *
     * \see indexLessThan()
     */
    QVector<int> rootToLeafPath( QgsAttributesFormItem *item ) const;


    /**
     * Emits dataChanged signal for all parent items in a model.
     *
     * In practice, this lets views know that the whole model has changed.
     *
     * \param parent  Model index representing the parent item.
     * \param roles   List of roles that have changed in the model.
     */
    void emitDataChangedRecursively( const QModelIndex &parent = QModelIndex(), const QVector<int> &roles = QVector<int>() );

    std::unique_ptr< QgsAttributesFormItem > mRootItem;
    QgsVectorLayer *mLayer;
    QgsProject *mProject;

    bool mShowAliases = false;
};


/**
 * \brief Manages available widgets when configuring attributes forms.
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

    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;

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
     * Returns the action container in this model, expected to be placed at the third top-level row.
     *
     * If there is no action container set, an invalid index is returned.
     */
    QModelIndex actionContainer() const;

    /**
     * Returns the model index that corresponds to the field with the given \a fieldName.
     */
    QModelIndex fieldModelIndex( const QString &fieldName ) const;

  public slots:
    void populate() override;

    /**
     * Refresh layer actions in the model to keep an updated action list.
     *
     * \param actions Up-to-date list of actions
     */
    void populateLayerActions( const QList< QgsAction > actions );

  private:
    /**
     * Refresh action items in the model.
     *
     * \param actions Up-to-date list of actions
     */
    void populateActionItems( const QList<QgsAction> actions );
};


/**
 * \brief Manages form layouts when configuring attributes forms via drag and drop designer.
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
     * The given \a project is used to extract information about relations.
     */
    explicit QgsAttributesFormLayoutModel( QgsVectorLayer *layer, QgsProject *project, QObject *parent = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Removes the index located at \a row within the given \a parent.
     */
    bool removeRow( int row, const QModelIndex &parent = QModelIndex() );

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    /**
     * Creates a new attribute editor element based on the definition stored in a form layout model \a index.
     */
    QgsAttributeEditorElement *createAttributeEditorWidget( const QModelIndex &index, QgsAttributeEditorElement *parent ) const;

    /**
     * Returns a list of containers stored in the model, structured as pairs (name, container model index).
     */
    QList< QgsAddAttributeFormContainerDialog::ContainerPair > listOfContainers() const;

    /**
     * Adds a new container to \a parent.
     *
     * If no \a parent is set then the container will be forced to be a tab widget.
     */
    void addContainer( QModelIndex &parent, const QString &name, int columnCount, Qgis::AttributeEditorContainerType type );

    /**
     * Updates the aliases of all matching fields in the model.
     *
     * Required because a field might appear several times in the form layout.
     *
     * \param fieldName Name of the field to search
     * \param fieldAlias Alias to be set to matching fields
     */
    void updateAliasForFieldItems( const QString &fieldName, const QString &fieldAlias );

    /**
     * Inserts a new child to \a parent model index at the given \a row position.
     *
     * The child is constructed from the given \a itemId, \a itemType and \a itemName.
     */
    void insertChild( const QModelIndex &parent, int row, const QString &itemId, QgsAttributesFormData::AttributesFormItemType itemType, const QString &itemName );

  public slots:
    void populate() override;

  signals:
    //! Informs that items were inserted (via drop) in the model from another model.
    void externalItemDropped( QModelIndex &index );
    //! Informs that items were moved (via drop) in the model from the same model.
    void internalItemDropped( QModelIndex &index );

  private:
    void updateAliasForFieldItemsRecursive( QgsAttributesFormItem *parent, const QString &fieldName, const QString &fieldAlias );
    QList< QgsAddAttributeFormContainerDialog::ContainerPair > recursiveListOfContainers( QgsAttributesFormItem *parent ) const;
    void loadAttributeEditorElementItem( QgsAttributeEditorElement *const editorElement, QgsAttributesFormItem *parent, const int position = -1 );

    /**
     * Creates a list of indexes filtering out children whose parents are already included.
     *
     * This discards redundant indexes before creating MimeData, because a parent will
     * include all its children, grandchildren, great-grandchildren, etc.
     *
     * \param indexes Input list of indexes, potentially with redundant indexes.
     */
    QModelIndexList curateIndexesForMimeData( const QModelIndexList &indexes ) const;
};


/**
 * \brief Proxy model to filter items in the tree views of the drag and drop designer.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormProxyModel, with the given \a parent.
     */
    QgsAttributesFormProxyModel( QObject *parent = nullptr );

    /**
     * Sets the source \a model for the proxy model.
     */
    void setAttributesFormSourceModel( QgsAttributesFormModel *model );

    /**
     * Returns the text used to filter source model items.
     */
    const QString filterText() const;

  public slots:
    //! Sets the filter text
    void setFilterText( const QString &filterText = QString() );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:
    QgsAttributesFormModel *mModel = nullptr;
    QString mFilterText;
};


Q_DECLARE_METATYPE( QgsAttributesFormData::RelationEditorConfiguration )
Q_DECLARE_METATYPE( QgsAttributesFormData::FieldConfig )
Q_DECLARE_METATYPE( QgsAttributesFormData::AttributeFormItemData )

#endif // QGSATTRIBUTESFORMMODEL_H
