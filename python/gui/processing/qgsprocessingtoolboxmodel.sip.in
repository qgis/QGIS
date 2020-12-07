/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingtoolboxmodel.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsProcessingToolboxModelNode : QObject
{
%Docstring
Abstract base class for nodes contained within a QgsProcessingToolboxModel.

.. warning::

   Not part of stable API and may change in future QGIS releases.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsprocessingtoolboxmodel.h"
%End
%ConvertToSubClassCode
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
    }
    else
      sipType = 0;
%End
  public:

    enum NodeType
    {
      NodeProvider,
      NodeGroup,
      NodeAlgorithm,
    };

    ~QgsProcessingToolboxModelNode();

    virtual NodeType nodeType() const = 0;
%Docstring
Returns the node's type.
%End

    QgsProcessingToolboxModelNode *parent();
%Docstring
Returns the node's parent. If the node's parent is a null pointer, then the node is a root node.
%End

    QList<QgsProcessingToolboxModelNode *> children();
%Docstring
Returns a list of children belonging to the node.
%End


    QgsProcessingToolboxModelNode* takeChild( QgsProcessingToolboxModelNode* node );
%Docstring
Removes the specified ``node`` from this node's children, and gives
ownership back to the caller.
%End

    QgsProcessingToolboxModelGroupNode *getChildGroupNode( const QString &id );
%Docstring
Tries to find a child node belonging to this node, which corresponds to
a group node with the given group ``id``. Returns None if no matching
child group node was found.
%End

    void addChildNode( QgsProcessingToolboxModelNode *node /Transfer/ );
%Docstring
Adds a child ``node`` to this node, transferring ownership of the node
to this node.
%End

    void deleteChildren();
%Docstring
Deletes all child nodes from this node.
%End

};

class QgsProcessingToolboxModelProviderNode : QgsProcessingToolboxModelNode
{
%Docstring
Processing toolbox model node corresponding to a Processing provider.

.. versionadded:: 3.2

.. warning::

   Not part of stable API and may change in future QGIS releases.
%End

%TypeHeaderCode
#include "qgsprocessingtoolboxmodel.h"
%End
  public:

    QgsProcessingToolboxModelProviderNode( QgsProcessingProvider *provider );
%Docstring
Constructor for QgsProcessingToolboxModelProviderNode, linked to the
specified ``provider``.
%End

    virtual NodeType nodeType() const;

    QgsProcessingProvider *provider();
%Docstring
Returns the provider associated with this node.
%End

    QString providerId() const;
%Docstring
Returns the provider ID.
%End

};

class QgsProcessingToolboxModelGroupNode : QgsProcessingToolboxModelNode
{
%Docstring
Processing toolbox model node corresponding to a group of algorithms.

.. versionadded:: 3.2

.. warning::

   Not part of stable API and may change in future QGIS releases.
%End

%TypeHeaderCode
#include "qgsprocessingtoolboxmodel.h"
%End
  public:

    QgsProcessingToolboxModelGroupNode( const QString &id, const QString &name );
%Docstring
Constructor for QgsProcessingToolboxModelGroupNode.

The ``id`` argument specifies the group ID (unique and untranslated),
and the ``name`` argument gives the translated, user-visible name
of the group.
%End

    virtual NodeType nodeType() const;

    QString id() const;
%Docstring
Returns the group's ID, which is unique and untranslated.
%End

    QString name() const;
%Docstring
Returns the group's name, which is translated and user-visible.
%End

};

class QgsProcessingToolboxModelAlgorithmNode : QgsProcessingToolboxModelNode
{
%Docstring
Processing toolbox model node corresponding to an algorithm.

.. versionadded:: 3.2

.. warning::

   Not part of stable API and may change in future QGIS releases.
%End

%TypeHeaderCode
#include "qgsprocessingtoolboxmodel.h"
%End
  public:

    QgsProcessingToolboxModelAlgorithmNode( const QgsProcessingAlgorithm *algorithm );
%Docstring
Constructor for QgsProcessingToolboxModelAlgorithmNode, associated
with the specified ``algorithm``.
%End

    virtual NodeType nodeType() const;

    const QgsProcessingAlgorithm *algorithm() const;
%Docstring
Returns the algorithm associated with this node.
%End

};


class QgsProcessingToolboxModel : QAbstractItemModel
{
%Docstring
A model for providers and algorithms shown within the Processing toolbox.

See QgsProcessingToolboxProxyModel for a sorted, filterable version
of this model.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsprocessingtoolboxmodel.h"
%End
  public:

    enum Roles
    {
      RoleNodeType,
      RoleAlgorithmFlags,
      RoleAlgorithmId,
      RoleAlgorithmName,
      RoleAlgorithmTags,
    };

    QgsProcessingToolboxModel( QObject *parent /TransferThis/ = 0, QgsProcessingRegistry *registry = 0 );
%Docstring
Constructor for QgsProcessingToolboxModel, with the given ``parent`` object.

If ``registry`` is specified then the model will show providers and algorithms
from the given registry. If no registry is specified, then the processing
registry attached to QgsApplication.processingRegistry() will be used
by the model.
%End

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex & = QModelIndex() ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;


    QgsProcessingToolboxModelNode *index2node( const QModelIndex &index ) const;
%Docstring
Returns the model node corresponding to the given ``index``.

.. seealso:: :py:func:`node2index`
%End

    QModelIndex node2index( QgsProcessingToolboxModelNode *node ) const;
%Docstring
Returns the model index corresponding to the given ``node``.

.. seealso:: :py:func:`index2node`
%End

    QgsProcessingProvider *providerForIndex( const QModelIndex &index ) const;
%Docstring
Returns the provider which corresponds to a given ``index``, or
a None if the index does not represent a provider.

.. seealso:: :py:func:`algorithmForIndex`

.. seealso:: :py:func:`indexForProvider`
%End

    QString providerIdForIndex( const QModelIndex &index ) const;
%Docstring
Returns the provider ID which corresponds to a given ``index``, or
an empty string if the index does not represent a provider.

.. seealso:: :py:func:`algorithmForIndex`

.. seealso:: :py:func:`indexForProvider`
%End

    const QgsProcessingAlgorithm *algorithmForIndex( const QModelIndex &index ) const;
%Docstring
Returns the algorithm which corresponds to a given ``index``, or
a None if the index does not represent an algorithm.

.. seealso:: :py:func:`isAlgorithm`

.. seealso:: :py:func:`providerForIndex`
%End

    bool isAlgorithm( const QModelIndex &index ) const;
%Docstring
Returns true if ``index`` corresponds to an algorithm.

.. seealso:: :py:func:`algorithmForIndex`
%End

    QModelIndex indexForProvider(const QString &providerId ) const;
%Docstring
Returns the index corresponding to the specified ``providerId``.

.. seealso:: :py:func:`providerForIndex`
%End

    QModelIndex indexOfParentTreeNode( QgsProcessingToolboxModelNode *parentNode ) const;
%Docstring
Returns the index corresponding to the parent of a given node.
%End

};


class QgsProcessingToolboxProxyModel: QSortFilterProxyModel
{
%Docstring
A sort/filter proxy model for providers and algorithms shown within the Processing toolbox,
which automatically sorts the toolbox in a logical fashion and supports filtering
the results.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsprocessingtoolboxmodel.h"
%End
  public:

    enum Filter
    {
      FilterToolbox,
      FilterModeler,
    };
    typedef QFlags<QgsProcessingToolboxProxyModel::Filter> Filters;


    explicit QgsProcessingToolboxProxyModel( QObject *parent /TransferThis/ = 0, QgsProcessingRegistry *registry = 0 );
%Docstring
Constructor for QgsProcessingToolboxProxyModel, with the given ``parent`` object.

If ``registry`` is specified then the model will show providers and algorithms
from the given registry. If no registry is specified, then the processing
registry attached to QgsApplication.processingRegistry() will be used
by the model.
%End

    void setFilters( QgsProcessingToolboxProxyModel::Filters filters );
%Docstring
Set ``filters`` that affect how toolbox content is filtered.

.. seealso:: :py:func:`filters`
%End

    Filters filters() const;
%Docstring
Returns any filters that affect how toolbox content is filtered.

.. seealso:: :py:func:`setFilters`
%End

    void setFilterString(const QString &filter);
%Docstring
Sets a ``filter`` string, such that only algorithms matching the
specified string will be shown.

.. seealso:: :py:func:`filterString`
%End

    QString filterString() const;

    virtual bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingtoolboxmodel.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
