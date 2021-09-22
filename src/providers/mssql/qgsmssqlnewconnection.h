/***************************************************************************
                          qgsmssqlnewconnection.h  -  description
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMSSQLNEWCONNECTION_H
#define QGSMSSQLNEWCONNECTION_H
#include "ui_qgsmssqlnewconnectionbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include <QAbstractListModel>

#include <QSqlDatabase>

class QgsMssqlDatabase;

/**
 * \class QgsMssqlNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for an MSSQL database
 */
class QgsMssqlNewConnection : public QDialog, private Ui::QgsMssqlNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMssqlNewConnection( QWidget *parent = nullptr, const QString &connName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Tests the connection using the parameters supplied
    bool testConnection( const QString &testDatabase = QString() );

    /**
     * \brief List all databases found for the given server.
     */
    void listDatabases();
  public slots:
    void accept() override;
    void btnListDatabase_clicked();
    void btnConnect_clicked();
    void cb_trustedConnection_clicked();

  private slots:
    //! Updates state of the OK button depending of the filled fields
    void updateOkButtonState();
    void onCurrentDataBaseChange();

    void onExtentFromGeometryToggled( bool checked );
    void onPrimaryKeyFromGeometryToggled( bool checked );

  private:
    //! Class that reprents a model to display available schemas on a database and choose which will be displayed in QGIS
    class SchemaModel: public QAbstractListModel
    {
      public:
        SchemaModel( QObject *parent = nullptr );

        int rowCount( const QModelIndex &parent ) const override;
        QVariant data( const QModelIndex &index, int role ) const override;
        bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
        Qt::ItemFlags flags( const QModelIndex &index ) const override;

        //! Returns the unchecked schemas
        QStringList uncheckedSchemas() const;

        //! Returns the database name represented by the model
        QString dataBaseName() const;

        //! Sets the database nale represented by the model
        void setDataBaseName( const QString &dataBaseName );

        //! Sets the settings for \a database
        void setSettings( const QString &database, const QStringList &schemas, const QStringList &excludedSchemas );

        //! Checks all schemas
        void checkAll();

        //! Unchecks all schemas
        void unCheckAll();

      private:
        QString mDataBaseName;
        QStringList mSchemas;
        QStringList mExcludedSchemas;

    };

    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();

    QVariantMap mSchemaSettings; //store the schema settings edited during this QDialog life time
    SchemaModel mSchemaModel;

    std::shared_ptr<QgsMssqlDatabase> getDatabase( const QString &name = QString() ) const;

    bool testExtentInGeometryColumns() const;

    bool testPrimaryKeyInGeometryColumns() const;
};

#endif //  QGSMSSQLNEWCONNECTION_H
