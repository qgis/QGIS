/***************************************************************************
    qgspostgresprojectversionsmodel.h
    ---------------------
    begin                : October 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESPROJECTVERSIONSMODEL_H
#define QGSPOSTGRESPROJECTVERSIONSMODEL_H

#include "qgspostgresconn.h"
#include "qgspostgresprojectstorage.h"

#include <QAbstractItemModel>
#include <QVector>

/**
 * A table model for displaying PostgreSQL project versions
 *
 * This model displays project version information including:
 *
 * - Modified Time
 * - Modified User
 * - Comment
 *
 * The model stores the date_saved internally for version identification.
 */
class QgsPostgresProjectVersionsModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Column
    {
      ModifiedTime = 0, //!< Modified timestamp
      ModifiedUser = 1, //!< User who modified the project
      Comment = 2       //!< Comment for the version
    };

    explicit QgsPostgresProjectVersionsModel( const QString &connectionName, QObject *parent = nullptr );

    ~QgsPostgresProjectVersionsModel() override;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;

    /**
     * Populate the model with versions for a given schema and project
     */
    void populateVersions( const QString &schema, const QString &project );

    /**
     * Get the project URI for a specific row
     */
    QgsPostgresProjectUri projectUriForRow( int row ) const;

    /**
     * Set the database connection
     */
    void setConnection( const QString &connectionName );

    /**
     * Clear all data from the model
     */
    void clear();

  private:
    /**
     * Structure to hold version data
     */
    struct PgProjectVersionData
    {
        QString dateSaved;    //!< Date saved (empty for latest version)
        QString modifiedTime; //!< Display modified time
        QString modifiedUser; //!< User who modified
        QString comment;      //!< Comment
    };

    QgsPostgresConn *mConn = nullptr;
    QString mConnectionName;
    QString mSchema;
    QString mProject;
    QVector<PgProjectVersionData> mVersions;
};

#endif // QGSPOSTGRESPROJECTVERSIONSMODEL_H
