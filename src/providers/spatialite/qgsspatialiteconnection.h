#ifndef QGSSPATIALITECONNECTION_H
#define QGSSPATIALITECONNECTION_H

#include <QStringList>
#include <QObject>

extern "C"
{
#include <sqlite3.h>
}

class QgsSpatiaLiteConnection : public QObject
{
  public:
    /** construct a connection. Name can be either stored connection name or a path to the database file */
    QgsSpatiaLiteConnection( QString name );

    QString path() { return mPath; }

    static QStringList connectionList();
    static void deleteConnection( QString name );
    static QString connectionPath( QString name );

    typedef struct TableEntry
    {
      TableEntry( QString _tableName, QString _column, QString _type )
          : tableName( _tableName ), column( _column ), type( _type ) {}
      QString tableName;
      QString column;
      QString type;
    } TableEntry;

    enum Error
    {
      NoError,
      NotExists,
      FailedToOpen,
      FailedToCheckMetadata,
      FailedToGetTables,
    };

    Error fetchTables( bool loadGeometrylessTables );

    /** return list of tables. fetchTables() function has to be called before */
    QList<TableEntry> tables() { return mTables; }

    /** return additional error message (if an error occurred before) */
    QString errorMessage() { return mErrorMsg; }

  protected:
    // SpatiaLite DB open / close
    sqlite3 *openSpatiaLiteDb( QString path );
    void closeSpatiaLiteDb( sqlite3 * handle );

    /**Checks if geometry_columns and spatial_ref_sys exist and have expected layout*/
    bool checkHasMetadataTables( sqlite3* handle );

    /**Inserts information about the spatial tables into mTables*/
    bool getTableInfo( sqlite3 * handle, bool loadGeometrylessTables );

    /**cleaning well-formatted SQL strings*/
    QString quotedValue( QString value ) const;

    /**Checks if geometry_columns_auth table exists*/
    bool checkGeometryColumnsAuth( sqlite3 * handle );

    /**Checks if views_geometry_columns table exists*/
    bool checkViewsGeometryColumns( sqlite3 * handle );

    /**Checks if virts_geometry_columns table exists*/
    bool checkVirtsGeometryColumns( sqlite3 * handle );

    /**Checks if this layer has been declared HIDDEN*/
    bool isDeclaredHidden( sqlite3 * handle, QString table, QString geom );

    /**Checks if this layer is a RasterLite-1 datasource*/
    bool isRasterlite1Datasource( sqlite3 * handle, const char * table );

    QString mErrorMsg;
    QString mPath; // full path to the database

    QList<TableEntry> mTables;
};

#endif // QGSSPATIALITECONNECTION_H
