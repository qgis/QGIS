/***************************************************************************
    qgsprocessingtoolboxmodel.h
    ---------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGTOOLBOXMODEL_H
#define QGSPROCESSINGTOOLBOXMODEL_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QPointer>

class QgsVectorLayer;
class QgsProcessingRegistry;
class QgsProcessingProvider;
class QgsProcessingAlgorithm;
class QgsProcessingToolboxModelGroupNode;
class QgsProcessingRecentAlgorithmLog;

///@cond PRIVATE

/**
 * \brief Abstract base class for nodes contained within a QgsProcessingToolboxModel.
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxModelNode : public QObject
{
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsProcessingToolboxModelNode" ) )
    {
      sipType = sipType_QgsProcessingToolboxModelNode;
      QgsProcessingToolboxModelNode *node = qobject_cast<QgsProcessingToolboxModelNode *>( sipCpp );
      if ( node->nodeType() == QgsProcessingToolboxModelNode::NodeProvider )
        sipType = sipType_QgsProcessingToolboxModelProviderNode;
      else if ( node->nodeType() == QgsProcessingToolboxModelNode::NodeGroup )
        sipType = sipType_QgsProcessingToolboxModelGroupNode;
      else if ( node->nodeType() == QgsProcessingToolboxModelNode::NodeAlgorithm )
        sipType = sipType_QgsProcessingToolboxModelAlgorithmNode;
      else if ( node->nodeType() == QgsProcessingToolboxModelNode::NodeRecent )
        sipType = sipType_QgsProcessingToolboxModelRecentNode;
    }
    else
      sipType = 0;
    SIP_END
#endif

  public:

    //! Enumeration of possible model node types
    enum NodeType
    {
      NodeProvider = 0, //!< Provider node
      NodeGroup, //!< Group node
      NodeAlgorithm, //!< Algorithm node
      NodeRecent, //!< Recent algorithms node
    };

    ~QgsProcessingToolboxModelNode() override;

    /**
     * Returns the node's type.
     */
    virtual NodeType nodeType() const = 0;

    /**
     * Returns the node's parent. If the node's parent is a null pointer, then the node is a root node.
     */
    QgsProcessingToolboxModelNode *parent() { return mParent; }

    /**
     * Returns a list of children belonging to the node.
     */
    QList<QgsProcessingToolboxModelNode *> children() { return mChildren; }

    /**
     * Returns a list of children belonging to the node.
     * \note Not available in Python bindings
     */
    QList<QgsProcessingToolboxModelNode *> children() const { return mChildren; } SIP_SKIP

    /**
     * Removes the specified \a node from this node's children, and gives
     * ownership back to the caller.
     */
    QgsProcessingToolboxModelNode *takeChild( QgsProcessingToolboxModelNode *node );

    /**
     * Tries to find a child node belonging to this node, which corresponds to
     * a group node with the given group \a id. Returns nullptr if no matching
     * child group node was found.
     */
    QgsProcessingToolboxModelGroupNode *getChildGroupNode( const QString &id );

    /**
     * Adds a child \a node to this node, transferring ownership of the node
     * to this node.
     */
    void addChildNode( QgsProcessingToolboxModelNode *node SIP_TRANSFER );

    /**
     * Deletes all child nodes from this node.
     */
    void deleteChildren();

  private:

    NodeType mNodeType;
    QgsProcessingToolboxModelNode *mParent = nullptr;
    QList<QgsProcessingToolboxModelNode *> mChildren;

};

/**
 * \brief Processing toolbox model node corresponding to the recent algorithms group
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxModelRecentNode : public QgsProcessingToolboxModelNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingToolboxModelRecentNode.
     */
    QgsProcessingToolboxModelRecentNode() = default;

    NodeType nodeType() const override { return NodeRecent; }

};

/**
 * \brief Processing toolbox model node corresponding to a Processing provider.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxModelProviderNode : public QgsProcessingToolboxModelNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingToolboxModelProviderNode, linked to the
     * specified \a provider.
     */
    QgsProcessingToolboxModelProviderNode( QgsProcessingProvider *provider );

    NodeType nodeType() const override { return NodeProvider; }

    /**
     * Returns the provider associated with this node.
     */
    QgsProcessingProvider *provider();

    /**
     * Returns the provider ID.
     */
    QString providerId() const { return mProviderId; }

  private:

    // NOTE: we store both the provider ID and a pointer to the provider here intentionally.
    // We store the provider pointer to avoid having to lookup the provider from the registry
    // every time the node is used (which kills performance in the filter proxy model), but
    // we also store the provider id string in order to identify the provider that the node
    // is linked to for cleanups after the provider is removed.
    QString mProviderId;
    QPointer< QgsProcessingProvider > mProvider;

};

/**
 * \brief Processing toolbox model node corresponding to a group of algorithms.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxModelGroupNode : public QgsProcessingToolboxModelNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingToolboxModelGroupNode.
     *
     * The \a id argument specifies the group ID (unique and untranslated),
     * and the \a name argument gives the translated, user-visible name
     * of the group.
     */
    QgsProcessingToolboxModelGroupNode( const QString &id, const QString &name );

    NodeType nodeType() const override { return NodeGroup; }

    /**
     * Returns the group's ID, which is unique and untranslated.
     */
    QString id() const { return mId; }

    /**
     * Returns the group's name, which is translated and user-visible.
     */
    QString name() const { return mName; }

  private:
    QString mId;
    QString mName;

};

/**
 * \brief Processing toolbox model node corresponding to an algorithm.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxModelAlgorithmNode : public QgsProcessingToolboxModelNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingToolboxModelAlgorithmNode, associated
     * with the specified \a algorithm.
     */
    QgsProcessingToolboxModelAlgorithmNode( const QgsProcessingAlgorithm *algorithm );

    NodeType nodeType() const override { return NodeAlgorithm; }

    /**
     * Returns the algorithm associated with this node.
     */
    const QgsProcessingAlgorithm *algorithm() const;

  private:

    const QgsProcessingAlgorithm *mAlgorithm = nullptr;

};

///@endcond

/**
 * \brief A model for providers and algorithms shown within the Processing toolbox.
 *
 * See QgsProcessingToolboxProxyModel for a sorted, filterable version
 * of this model.
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Custom roles used by the model
    enum Roles
    {
      RoleNodeType = Qt::UserRole, //!< Corresponds to the node's type
      RoleAlgorithmFlags, //!< Returns the node's algorithm flags, for algorithm nodes
      RoleAlgorithmId, //!< Algorithm ID, for algorithm nodes
      RoleAlgorithmName, //!< Untranslated algorithm name, for algorithm nodes
      RoleAlgorithmShortDescription, //!< Short algorithm description, for algorithm nodes
      RoleAlgorithmTags, //!< List of algorithm tags, for algorithm nodes
    };

    /**
     * Constructor for QgsProcessingToolboxModel, with the given \a parent object.
     *
     * If \a registry is specified then the model will show providers and algorithms
     * from the given registry. If no registry is specified, then the processing
     * registry attached to QgsApplication::processingRegistry() will be used
     * by the model.
     *
     * If \a recentLog is specified then it will be used to create a "Recently used" top
     * level group containing recently used algorithms.
     */
    QgsProcessingToolboxModel( QObject *parent SIP_TRANSFERTHIS = nullptr, QgsProcessingRegistry *registry = nullptr,
                               QgsProcessingRecentAlgorithmLog *recentLog = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;

    /**
     * Returns the model node corresponding to the given \a index.
     * \see node2index()
     */
    QgsProcessingToolboxModelNode *index2node( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to the given \a node.
     * \see index2node()
     */
    QModelIndex node2index( QgsProcessingToolboxModelNode *node ) const;

    /**
     * Returns the provider which corresponds to a given \a index, or
     * a nullptr if the index does not represent a provider.
     *
     * \see algorithmForIndex()
     * \see indexForProvider()
     */
    QgsProcessingProvider *providerForIndex( const QModelIndex &index ) const;

    /**
     * Returns the provider ID which corresponds to a given \a index, or
     * an empty string if the index does not represent a provider.
     *
     * \see algorithmForIndex()
     * \see indexForProvider()
     */
    QString providerIdForIndex( const QModelIndex &index ) const;

    /**
     * Returns the algorithm which corresponds to a given \a index, or
     * a nullptr if the index does not represent an algorithm.
     *
     * \see isAlgorithm()
     * \see providerForIndex()
     */
    const QgsProcessingAlgorithm *algorithmForIndex( const QModelIndex &index ) const;

    /**
     * Returns true if \a index corresponds to an algorithm.
     *
     * \see algorithmForIndex()
     */
    bool isAlgorithm( const QModelIndex &index ) const;

    /**
     * Returns the index corresponding to the specified \a providerId.
     * \see providerForIndex()
     */
    QModelIndex indexForProvider( const QString &providerId ) const;

    /**
     * Returns the index corresponding to the parent of a given node.
     */
    QModelIndex indexOfParentTreeNode( QgsProcessingToolboxModelNode *parentNode ) const;

  signals:

    /**
     * Emitted whenever recent algorithms are added to the model.
     */
    void recentAlgorithmAdded();

  private slots:

    void rebuild();
    void repopulateRecentAlgorithms( bool resetting = false );
    void providerAdded( const QString &id );
    void providerRemoved( const QString &id );

  private:

    QPointer< QgsProcessingRegistry > mRegistry;
    QPointer< QgsProcessingRecentAlgorithmLog > mRecentLog;

    std::unique_ptr< QgsProcessingToolboxModelGroupNode > mRootNode;
    QgsProcessingToolboxModelRecentNode *mRecentNode = nullptr;

    void addProvider( QgsProcessingProvider *provider );

    /**
     * Returns true if \a providerId is a "top-level" provider, which shows
     * groups directly under the root node and not under a provider child node.
     */
    static bool isTopLevelProvider( const QString &providerId );

    /**
     * Returns a formatted tooltip for an \a algorithm.
     */
    static QString toolTipForAlgorithm( const QgsProcessingAlgorithm *algorithm );

};


/**
 * \brief A sort/filter proxy model for providers and algorithms shown within the Processing toolbox,
 * which automatically sorts the toolbox in a logical fashion and supports filtering
 * the results.
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingToolboxProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    //! Available filter flags for filtering the model
    enum Filter
    {
      FilterToolbox = 1 << 1, //!< Filters out any algorithms and content which should not be shown in the toolbox
      FilterModeler = 1 << 2, //!< Filters out any algorithms and content which should not be shown in the modeler
      FilterInPlace = 1 << 3, //!< Only show algorithms which support in-place edits
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * Constructor for QgsProcessingToolboxProxyModel, with the given \a parent object.
     *
     * If \a registry is specified then the model will show providers and algorithms
     * from the given registry. If no registry is specified, then the processing
     * registry attached to QgsApplication::processingRegistry() will be used
     * by the model.
     *
     * If \a recentLog is specified then it will be used to create a "Recently used" top
     * level group containing recently used algorithms.
     */
    explicit QgsProcessingToolboxProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr,
        QgsProcessingRegistry *registry = nullptr,
        QgsProcessingRecentAlgorithmLog *recentLog = nullptr );

    /**
     * Returns the underlying source Processing toolbox model.
     */
    QgsProcessingToolboxModel *toolboxModel();

    /**
     * Set \a filters that affect how toolbox content is filtered.
     * \see filters()
     */
    void setFilters( QgsProcessingToolboxProxyModel::Filters filters );

    /**
     * Returns any filters that affect how toolbox content is filtered.
     * \see setFilters()
     */
    Filters filters() const { return mFilters; }

    /**
     * Sets geometry \type for the in-place algorithms
     * \param layer the vector layer for in-place algorithm filter
     */
    void setInPlaceLayer( QgsVectorLayer *layer );

    /**
     * Sets a \a filter string, such that only algorithms matching the
     * specified string will be shown.
     *
     * Matches are performed using a variety of tests, including checking
     * against the algorithm name, short description, tags, etc.
     *
     * \see filterString()
    */
    void setFilterString( const QString &filter );

    /**
     * Returns the current filter string, if set.
     *
     * \see setFilterString()
     */
    QString filterString() const { return mFilterString; }

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:

    QgsProcessingToolboxModel *mModel = nullptr;
    Filters mFilters = nullptr;
    QString mFilterString;
    QPointer<QgsVectorLayer> mInPlaceLayer;
};


#endif // QGSPROCESSINGTOOLBOXMODEL_H
