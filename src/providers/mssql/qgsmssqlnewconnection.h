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
  private:
    //! Class that reprents a model to display available schemas on a database and choose which will be diplayed in QGIS
    class SchemaModel: public QAbstractListModel
    {
      public:
        SchemaModel( QObject *parent = nullptr );

        int rowCount( const QModelIndex &parent ) const override;
        QVariant data( const QModelIndex &index, int role ) const override;
        bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
        Qt::ItemFlags flags( const QModelIndex &index ) const override;

        //! Sets the schema settings (keyd : schema names, value : bool that represnts wheter the schema is checked)
        void setSchemasSetting( const QVariantMap &schemas );

        //! Returns the schema settings (keyd : schema names, value : bool that represnts wheter the schema is checked)
        QVariantMap schemasSettings() const;

        //! Returns the database nale represented by the model
        QString dataBaseName() const;

        //! Sets the database nale represented by the model
        void setDataBaseName( const QString &dataBaseName );

      private:
        QVariantMap mSchemas;
        QString mDataBaseName;

    };

    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();

    QVariantMap mSchemaSettings; //store the schema settings edited during this QDialog life time
    SchemaModel mSchemaModel;

};

#endif //  QGSMSSQLNEWCONNECTION_H
