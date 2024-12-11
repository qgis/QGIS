/***************************************************************************
                             qgscoordinatereferencesystemmodel.h
                             -------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATEREFERENCESYSTEMMODEL_H
#define QGSCOORDINATEREFERENCESYSTEMMODEL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgscoordinatereferencesystemregistry.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QIcon>

class QgsCoordinateReferenceSystem;
class QgsCoordinateReferenceSystemModelGroupNode;

///@cond PRIVATE
#ifndef SIP_RUN

/**
 * \brief Abstract base class for nodes contained within a QgsCoordinateReferenceSystemModel.
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsCoordinateReferenceSystemModelNode
{
  public:
    //! Enumeration of possible model node types
    enum NodeType
    {
      NodeGroup, //!< Group node
      NodeCrs,   //!< CRS node
    };

    virtual ~QgsCoordinateReferenceSystemModelNode();

    /**
     * Returns the node's type.
     */
    virtual NodeType nodeType() const = 0;

    /**
     * Returns the node's parent. If the node's parent is NULLPTR, then the node is a root node.
     */
    QgsCoordinateReferenceSystemModelNode *parent() { return mParent; }

    /**
     * Returns a list of children belonging to the node.
     */
    QList<QgsCoordinateReferenceSystemModelNode *> children() { return mChildren; }

    /**
     * Returns a list of children belonging to the node.
     */
    QList<QgsCoordinateReferenceSystemModelNode *> children() const { return mChildren; }

    /**
    * Removes the specified \a node from this node's children, and gives
    * ownership back to the caller.
    */
    QgsCoordinateReferenceSystemModelNode *takeChild( QgsCoordinateReferenceSystemModelNode *node );

    /**
     * Adds a child \a node to this node, transferring ownership of the node
     * to this node.
     */
    void addChildNode( QgsCoordinateReferenceSystemModelNode *node );

    /**
     * Deletes all child nodes from this node.
     */
    void deleteChildren();

    /**
     * Tries to find a child node belonging to this node, which corresponds to
     * a group node with the given group \a id. Returns NULLPTR if no matching
     * child group node was found.
     */
    QgsCoordinateReferenceSystemModelGroupNode *getChildGroupNode( const QString &id );

  private:
    QgsCoordinateReferenceSystemModelNode *mParent = nullptr;
    QList<QgsCoordinateReferenceSystemModelNode *> mChildren;
};

/**
 * \brief Coordinate reference system model node corresponding to a group
 * \ingroup gui
 * \warning Not available in Python bindings
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsCoordinateReferenceSystemModelGroupNode : public QgsCoordinateReferenceSystemModelNode
{
  public:
    /**
     * Constructor for QgsCoordinateReferenceSystemModelGroupNode.
     */
    QgsCoordinateReferenceSystemModelGroupNode( const QString &name, const QIcon &icon, const QString &id );

    /**
    * Returns the group's ID, which is non-translated.
    */
    QString id() const { return mId; }

    /**
     * Returns the group's name, which is translated and user-visible.
     */
    QString name() const { return mName; }

    /**
     * Returns the group's icon.
     */
    QIcon icon() const { return mIcon; }

    NodeType nodeType() const override { return NodeGroup; }

  private:
    QString mId;
    QString mName;
    QIcon mIcon;
};

/**
 * \brief Coordinate reference system model node corresponding to a CRS.
 * \ingroup gui
 * \warning Not available in Python bindings.
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsCoordinateReferenceSystemModelCrsNode : public QgsCoordinateReferenceSystemModelNode
{
  public:
    /**
     * Constructor for QgsCoordinateReferenceSystemModelCrsNode, associated
     * with the specified \a record.
     */
    QgsCoordinateReferenceSystemModelCrsNode( const QgsCrsDbRecord &record );

    NodeType nodeType() const override { return NodeCrs; }

    /**
     * Returns the record associated with this node.
     */
    const QgsCrsDbRecord &record() const { return mRecord; }

    /**
     * Sets the \a wkt representation of the CRS.
     *
     * This is only used for non-standard CRS (i.e. those not present in the database).
     *
     * \see wkt()
     */
    void setWkt( const QString &wkt ) { mWkt = wkt; }

    /**
     * Returns the WKT representation of the CRS.
     *
     * This is only used for non-standard CRS (i.e. those not present in the database).
     *
     * \see setWkt()
     */
    QString wkt() const { return mWkt; }

    /**
     * Sets the \a proj representation of the CRS.
     *
     * This is only used for non-standard CRS (i.e. those not present in the database).
     *
     * \see proj()
     */
    void setProj( const QString &proj ) { mProj = proj; }

    /**
     * Returns the PROJ representation of the CRS.
     *
     * This is only used for non-standard CRS (i.e. those not present in the database).
     *
     * \see setProj()
     */
    QString proj() const { return mProj; }

    /**
     * Sets the CRS's group name.
     *
     * \see group()
     * \since QGIS 3.42
     */
    void setGroup( const QString &group ) { mGroup = group; }

    /**
     * Returns the CRS's group name.
     *
     * \see setGroup()
     * \since QGIS 3.42
     */
    QString group() const { return mGroup; }

    /**
     * Sets the CRS's projection name.
     *
     * \see projection()
     * \since QGIS 3.42
     */
    void setProjection( const QString &projection ) { mProjection = projection; }

    /**
     * Returns the CRS's projection name.
     *
     * \see setProjection()
     * \since QGIS 3.42
     */
    QString projection() const { return mProjection; }

  private:
    const QgsCrsDbRecord mRecord;
    QString mWkt;
    QString mProj;
    QString mGroup;
    QString mProjection;
};

#endif
///@endcond

/**
 * \class QgsCoordinateReferenceSystemModel
 * \ingroup core
 * \brief A tree model for display of known coordinate reference systems.
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsCoordinateReferenceSystemModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsCoordinateReferenceSystemModel::Roles
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCoordinateReferenceSystemModel, Roles ) : int
    {
      NodeType SIP_MONKEYPATCH_COMPAT_NAME( RoleNodeType ) = Qt::UserRole,         //!< Corresponds to the node's type
      Name SIP_MONKEYPATCH_COMPAT_NAME( RoleName ) = Qt::UserRole + 1,             //!< The coordinate reference system name
      AuthId SIP_MONKEYPATCH_COMPAT_NAME( RoleAuthId ) = Qt::UserRole + 2,         //!< The coordinate reference system authority name and id
      Deprecated SIP_MONKEYPATCH_COMPAT_NAME( RoleDeprecated ) = Qt::UserRole + 3, //!< TRUE if the CRS is deprecated
      Type SIP_MONKEYPATCH_COMPAT_NAME( RoleType ) = Qt::UserRole + 4,             //!< The coordinate reference system type
      GroupId SIP_MONKEYPATCH_COMPAT_NAME( RoleGroupId ) = Qt::UserRole + 5,       //!< The node ID (for group nodes)
      Wkt SIP_MONKEYPATCH_COMPAT_NAME( RoleWkt ) = Qt::UserRole + 6,               //!< The coordinate reference system's WKT representation. This is only used for non-standard CRS (i.e. those not present in the database).
      Proj SIP_MONKEYPATCH_COMPAT_NAME( RoleProj ) = Qt::UserRole + 7,             //!< The coordinate reference system's PROJ representation. This is only used for non-standard CRS (i.e. those not present in the database).
      Group = Qt::UserRole + 8,                                                    //!< Group name. \since QGIS 3.42
      Projection = Qt::UserRole + 9,                                               //!< Projection name. \since QGIS 3.42
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsCoordinateReferenceSystemModel, with the specified \a parent object.
     */
    QgsCoordinateReferenceSystemModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    /**
     * Adds a custom \a crs to the model.
     *
     * This method can be used to add CRS which aren't present in either the standard PROJ SRS database or the
     * user's custom CRS database to the model.
     */
    QModelIndex addCustomCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Retrieves the model index corresponding to a CRS with the specified \a authId.
     *
     * Returns an invalid index if the CRS was not found.
     */
    QModelIndex authIdToIndex( const QString &authId ) const;

  private slots:

    void rebuild();
    void userCrsAdded( const QString &id );
    void userCrsRemoved( long id );
    void userCrsChanged( const QString &id );

  private:
    QgsCoordinateReferenceSystemModelCrsNode *addRecord( const QgsCrsDbRecord &record );

    QgsCoordinateReferenceSystemModelNode *index2node( const QModelIndex &index ) const;
    QModelIndex node2index( QgsCoordinateReferenceSystemModelNode *node ) const;
    QModelIndex indexOfParentTreeNode( QgsCoordinateReferenceSystemModelNode *parentNode ) const;


    std::unique_ptr<QgsCoordinateReferenceSystemModelGroupNode> mRootNode;

    QList<QgsCrsDbRecord> mCrsDbRecords;
};


/**
 * \brief A sort/filter proxy model for coordinate reference systems.
 *
 * \ingroup gui
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsCoordinateReferenceSystemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    //! Available filter flags for filtering the model
    enum Filter SIP_ENUM_BASETYPE( IntFlag )
    {
      FilterHorizontal = 1 << 1, //!< Include horizontal CRS (excludes compound CRS containing a horizontal component)
      FilterVertical = 1 << 2,   //!< Include vertical CRS (excludes compound CRS containing a vertical component)
      FilterCompound = 1 << 3,   //!< Include compound CRS
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * Constructor for QgsCoordinateReferenceSystemProxyModel, with the given \a parent object.
     */
    explicit QgsCoordinateReferenceSystemProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the underlying source model.
     */
    QgsCoordinateReferenceSystemModel *coordinateReferenceSystemModel();

    /**
     * Returns the underlying source model.
     * \note Not available in Python bindings
     */
    const QgsCoordinateReferenceSystemModel *coordinateReferenceSystemModel() const SIP_SKIP;

    /**
     * Set \a filters that affect how CRS are filtered.
     * \see filters()
     */
    void setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters );

    /**
     * Returns any filters that affect how CRS are filtered.
     * \see setFilters()
     */
    Filters filters() const { return mFilters; }

    /**
     * Sets a \a filter string, such that only coordinate reference systems matching the
     * specified string will be shown.
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

    /**
     * Sets a \a filter list of CRS auth ID strings, such that only coordinate reference systems matching the
     * specified auth IDs will be shown.
     *
     * \see filterAuthIds()
    */
    void setFilterAuthIds( const QSet<QString> &filter );

    /**
     * Returns the current filter list of auth ID strings, if set.
     *
     * \see setFilterString()
     */
    QSet<QString> filterAuthIds() const { return mFilterAuthIds; }

    /**
     * Sets whether deprecated CRS should be filtered from the results.
     *
     * \see filterDeprecated()
    */
    void setFilterDeprecated( bool filter );

    /**
     * Returns whether deprecated CRS will be filtered from the results.
     *
     * \see setFilterDeprecated()
     */
    bool filterDeprecated() const { return mFilterDeprecated; }

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:
    QgsCoordinateReferenceSystemModel *mModel = nullptr;
    QString mFilterString;
    QSet<QString> mFilterAuthIds;
    bool mFilterDeprecated = false;
    Filters mFilters = Filters();
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsCoordinateReferenceSystemProxyModel::Filters )


#endif // QGSCOORDINATEREFERENCESYSTEMMODEL_H
