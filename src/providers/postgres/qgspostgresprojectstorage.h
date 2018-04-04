#ifndef QGSPOSTGRESPROJECTSTORAGE_H
#define QGSPOSTGRESPROJECTSTORAGE_H

#include "qgsconfig.h"
#include "qgsprojectstorage.h"

#include "qgsdatasourceuri.h"

//! Stores information parsed from postgres project URI
typedef struct
{
  bool valid;

  QgsDataSourceUri connInfo;  // using only the bits about connection info (server, port, username, password, service, ssl mode)

  QString schemaName;
  QString projectName;

} QgsPostgresProjectUri;


//! Implements storage of QGIS projects inside a PostgreSQL table
class QgsPostgresProjectStorage : public QgsProjectStorage
{
  public:
    QgsPostgresProjectStorage();

    virtual QString type() override { return QStringLiteral( "postgresql" ); }

    virtual QStringList listProjects( const QString &uri ) override;

    virtual bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    virtual bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    virtual bool removeProject( const QString &uri ) override;

    virtual bool readProjectMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override;

#ifdef HAVE_GUI
    // GUI support
    virtual QString visibleName() override;
    virtual QString showLoadGui() override;
    virtual QString showSaveGui() override;
#endif

    static QString encodeUri( const QgsPostgresProjectUri &postUri );
    static QgsPostgresProjectUri decodeUri( const QString &uri );
};

#endif // QGSPOSTGRESPROJECTSTORAGE_H
