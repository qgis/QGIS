/***************************************************************************
    qgsdatabasequeryloggernode.h
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDBQUERYLOGGERNODE_H
#define QGSDBQUERYLOGGERNODE_H

#include <QElapsedTimer>
#include <QVariant>
#include <QColor>
#include <QUrl>
#include <memory>
#include <deque>
#include "devtools/qgsdevtoolsmodelnode.h"
#include "qgsdbquerylog.h"

class QAction;

/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerRootNode
 * \brief Root node for the query logger model.
 */
class QgsDatabaseQueryLoggerRootNode final : public QgsDevToolsModelGroup
{
  public:

    QgsDatabaseQueryLoggerRootNode();
    QVariant data( int role = Qt::DisplayRole ) const override final;

    /**
     * Removes a \a row from the root group.
     */
    void removeRow( int row );

    QVariant toVariant() const override;
};


/**
 * \ingroup app
 * \class QgsDatabaseQueryLoggerQueryGroup
 * \brief Parent group for all database queries, showing the query id, SQL
 * and containing child groups with detailed query and result information.
 *
 * Visually, a QgsDatabaseQueryLoggerQueryGroup is structured by:
 *
 * |__ QgsDatabaseQueryLoggerQueryGroup (showing sql, uri,...)
 *   |__ QgsDevToolsModelValueNode(key-value pairs with info)
 *       ...
 */
class QgsDatabaseQueryLoggerQueryGroup final : public QgsDevToolsModelGroup
{
  public:

    //! Query status
    enum class Status
    {
      Pending, //!< Query underway
      Complete, //!< Query was successfully completed
      Error, //!< Query encountered an error
      TimeOut, //!< Query timed out
      Canceled, //!< Query was manually canceled
    };

    /**
     * Constructor for QgsDatabaseQueryLoggerQueryGroup, populated from the
     * specified \a query details.
     */
    QgsDatabaseQueryLoggerQueryGroup( const QgsDatabaseQueryLogEntry &query );
    QVariant data( int role = Qt::DisplayRole ) const override;
    QList< QAction * > actions( QObject *parent ) override final;
    QVariant toVariant() const override;

    /**
     * Called when the \a query is finished.
     *
     * Will automatically create children encapsulating the completed details.
     */
    void setFinished( const  QgsDatabaseQueryLogEntry &query );

    /**
     * Returns the query's status.
     */
    Status status() const { return mStatus; }

    /**
     * Set the query \a status
     */
    void setStatus( Status status );

    /**
     * Converts a request \a status to a translated string value.
     */
    static QString statusToString( Status status );

    /**
     * Sets the SQL to \a sql.
     */
    void setSql( const QString &sql );

    /**
     * Returns the group SQL.
     */
    const QString &sql() const;

  private:

    QString mSql;
    int mQueryId = 0;
    QByteArray mData;
    Status mStatus = Status::Pending;
};

#endif // QGSDBQUERYLOGGERNODE_H
