/***************************************************************************
    qgsruntimeprofiler.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRUNTIMEPROFILER_H
#define QGSRUNTIMEPROFILER_H

#include <QTime>
#include <QElapsedTimer>
#include "qgis_sip.h"
#include <QPair>
#include <QStack>
#include <QList>
#include <QAbstractItemModel>
#include <memory>
#include <deque>
#include <QSet>
#include "qgis_core.h"

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsRuntimeProfilerNode
 * A node representing an entry in a QgsRuntimeProfiler.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsRuntimeProfilerNode
{
  public:

    //! Custom node data roles
    enum Roles
    {
      Name = Qt::UserRole + 1, //!< Profile item name
      Group, //!< Node group
      Elapsed, //!< Node elapsed time
      ParentElapsed, //!< Total elapsed time for node's parent
    };

    /**
     * Constructor for QgsRuntimeProfilerNode, with the specified \a group and \a name.
     */
    QgsRuntimeProfilerNode( const QString &group, const QString &name );

    //! QgsRuntimeProfilerNode cannot be copied
    QgsRuntimeProfilerNode( const QgsRuntimeProfilerNode &other ) = delete;
    //! QgsRuntimeProfilerNode cannot be copied
    QgsRuntimeProfilerNode &operator=( const QgsRuntimeProfilerNode &other ) = delete;

    ~QgsRuntimeProfilerNode();

    /**
     * Returns the node's parent node.
     *
     * If parent is NULLPTR, the node is a root node
     */
    QgsRuntimeProfilerNode *parent() { return mParent; }

    /**
     * Returns the full path to the node's parent.
     */
    QStringList fullParentPath() const;

    /**
     * Returns the node's data for the specified model \a role.
     */
    QVariant data( int role = Qt::DisplayRole ) const;

    /**
     * Returns the number of child nodes owned by this node.
     */
    int childCount() const { return mChildren.size(); }

    /**
     * Adds a \a child node to this node.
     */
    void addChild( std::unique_ptr< QgsRuntimeProfilerNode > child );

    /**
     * Returns the index of the specified \a child node.
     *
     * \warning \a child must be a valid child of this node.
     */
    int indexOf( QgsRuntimeProfilerNode *child ) const;

    /**
     * Finds the child with matching \a group and \a name. Returns NULLPTR if
     * a matching child was not found.
     */
    QgsRuntimeProfilerNode *child( const QString &group, const QString &name );

    /**
     * Returns the child at the specified \a index.
     */
    QgsRuntimeProfilerNode *childAt( int index );

    /**
     * Clears the node, removing all its children.
     */
    void clear();

    /**
     * Removes and deletes the child at the specified \a index.
     */
    void removeChildAt( int index );

    /**
     * Starts the node timer.
     * \see stop()
     */
    void start();

    /**
     * Stops the node's timer, recording the elapsed time automatically.
     */
    void stop();

    /**
     * Manually sets the node's elapsed \a time, in seconds.
     */
    void setElapsed( double time );

    /**
     * Returns the node's elapsed time, in seconds.
     *
     * If the node is still running then 0 will be returned.
     */
    double elapsed() const;

    /**
     * Returns the total elapsed time in seconds for all children
     * of this node with matching \a group.
     */
    double totalElapsedTimeForChildren( const QString &group ) const;

  private:
    std::deque< std::unique_ptr< QgsRuntimeProfilerNode > > mChildren;
    QgsRuntimeProfilerNode *mParent = nullptr;
    QElapsedTimer mProfileTime;
    double mElapsed = 0;

    QString mName;
    QString mGroup;

};
#endif

/**
 * \ingroup core
 * \class QgsRuntimeProfiler
 *
 * Provides a method of recording run time profiles of operations, allowing
 * easy recording of their overall run time.
 *
 * QgsRuntimeProfiler is not usually instantied manually, but rather accessed
 * through QgsApplication::profiler().
 *
 * This class is thread-safe only if accessed through QgsApplication::profiler().
 * If accessed in this way, operations can be profiled from non-main threads.
 */
class CORE_EXPORT QgsRuntimeProfiler : public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor to create a new runtime profiler.
     *
     * \warning QgsRuntimeProfiler is not usually instantied manually, but rather accessed
     * through QgsApplication::profiler().
     */
    QgsRuntimeProfiler();
    ~QgsRuntimeProfiler() override;

    /**
     * \brief Begin the group for the profiler. Groups will append {GroupName}/ to the
     * front of the profile tag set using start.
     * \param name The name of the group.
     *
     * \deprecated use start() instead
     */
    Q_DECL_DEPRECATED void beginGroup( const QString &name ) SIP_DEPRECATED;

    /**
     * \brief End the current active group.
     *
     * \deprecated use end() instead
     */
    Q_DECL_DEPRECATED void endGroup() SIP_DEPRECATED;

    /**
     * Returns a list of all child groups with the specified \a parent.
     * \since QGIS 3.14
     */
    QStringList childGroups( const QString &parent = QString(), const QString &group = "startup" ) const;

    /**
     * \brief Start a profile event with the given name.
     * \param name The name of the profile event. Will have the name of
     * the active group appended after ending.
     */
    void start( const QString &name, const QString &group = "startup" );

    /**
     * \brief End the current profile event.
     */
    void end( const QString &group = "startup" );

    /**
     * Returns the profile time for the specified \a name.
     * \since QGIS 3.14
     */
    double profileTime( const QString &name, const QString &group = "startup" ) const;

    /**
     * \brief clear Clear all profile data.
     */
    void clear( const QString &group = "startup" );

    /**
     * \brief The current total time collected in the profiler.
     * \returns The current total time collected in the profiler.
     */
    double totalTime( const QString &group = "startup" );

    /**
     * Returns the set of known groups.
     */
    QSet< QString > groups() const { return mGroups; }

    /**
     * Returns TRUE if the specified \a group is currently being logged,
     * i.e. it has a entry which has started and not yet stopped.
     *
     * \since QGIS 3.14
     */
    bool groupIsActive( const QString &group ) const;

    /**
     * Returns the translated name of a standard profile \a group.
     */
    static QString translateGroupName( const QString &group );

    // Implementation of virtual functions from QAbstractItemModel

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

#ifndef SIP_RUN
    ///@cond PRIVATE
  signals:

    void started( const QString &group, const QStringList &path, const QString &name );
    void ended( const QString &group, const QStringList &path, const QString &name, double elapsed );
///@endcond
#endif

    /**
     * Emitted when a new group has started being profiled.
     */
    void groupAdded( const QString &group );

  private slots:

    void otherProfilerStarted( const QString &group, const QStringList &path, const QString &name );
    void otherProfilerEnded( const QString &group, const QStringList &path, const QString &name, double elapsed );

  private:

    static QgsRuntimeProfiler *threadLocalInstance();
    static QgsRuntimeProfiler *sMainProfiler;
    bool mInitialized = false;
    void setupConnections();

    QgsRuntimeProfilerNode *pathToNode( const QString &group, const QString &path ) const;
    QgsRuntimeProfilerNode *pathToNode( const QString &group, const QStringList &path ) const;
    QModelIndex node2index( QgsRuntimeProfilerNode *node ) const;
    QModelIndex indexOfParentNode( QgsRuntimeProfilerNode *parentNode ) const;

    /**
     * Returns node for given index. Returns root node for invalid index.
     */
    QgsRuntimeProfilerNode *index2node( const QModelIndex &index ) const;

    QMap< QString, QStack< QgsRuntimeProfilerNode * > > mCurrentStack;
    std::unique_ptr< QgsRuntimeProfilerNode > mRootNode;

    QSet< QString > mGroups;

    friend class QgsApplication;
};


/**
 * \ingroup core
 *
 * Scoped object for logging of the runtime for a single operation or group of operations.
 *
 * This class automatically takes care of registering an operation in the QgsApplication::profiler()
 * registry upon construction, and recording of the elapsed runtime upon destruction.
 *
 * Python scripts should not use QgsScopedRuntimeProfile directly. Instead, use QgsRuntimeProfiler.profile()
 * \code{.py}
 *   with QgsRuntimeProfiler.profile('My operation'):
 *     # do something
 * \endcode
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsScopedRuntimeProfile
{
  public:

    /**
     * Constructor for QgsScopedRuntimeProfile.
     *
     * Automatically registers the operation in the QgsApplication::profiler() instance
     * and starts recording the run time of the operation.
     */
    QgsScopedRuntimeProfile( const QString &name, const QString &group = "startup" );

    /**
     * Records the final runtime of the operation in the profiler instance.
     */
    ~QgsScopedRuntimeProfile();

    /**
     * Switches the current task managed by the scoped profile to a new task with the given \a name.
     * The current task will be finalised before switching.
     *
     * This is useful for reusing an existing scoped runtime profiler with multi-step processes.
     *
     * \since QGIS 3.14
     */
    void switchTask( const QString &name );

  private:

    QString mGroup;

};


#endif // QGSRUNTIMEPROFILER_H
