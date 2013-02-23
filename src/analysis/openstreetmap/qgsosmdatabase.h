#ifndef OSMDATABASE_H
#define OSMDATABASE_H

#include <QString>
#include <QStringList>

#include "qgsosmbase.h"

#include "qgsgeometry.h"

class QgsOSMNodeIterator;
class QgsOSMWayIterator;

typedef QPair<QString, int> QgsOSMTagCountPair;

/**
 * Class that encapsulates access to OpenStreetMap data stored in a database
 * previously imported from XML file.
 */
class ANALYSIS_EXPORT QgsOSMDatabase
{
  public:
    explicit QgsOSMDatabase( const QString& dbFileName = QString() );
    ~QgsOSMDatabase();

    void setFileName( const QString& dbFileName ) { mDbFileName = dbFileName; }
    QString filename() const { return mDbFileName; }
    bool isOpen() const;

    bool open();
    bool close();

    QString errorString() const { return mError; }

    // data access

    int countNodes() const;
    int countWays() const;

    QgsOSMNodeIterator listNodes() const;
    QgsOSMWayIterator listWays() const;

    QgsOSMNode node( QgsOSMId id ) const;
    QgsOSMWay way( QgsOSMId id ) const;
    //OSMRelation relation( OSMId id ) const;

    QgsOSMTags tags( bool way, QgsOSMId id ) const;

    QList<QgsOSMTagCountPair> usedTags( bool ways ) const;

    QgsPolyline wayPoints( QgsOSMId id ) const;

    // export to spatialite

    enum ExportType { Point, Polyline, Polygon };
    bool exportSpatiaLite( ExportType type, const QString& tableName, const QStringList& tagKeys = QStringList() );

  protected:
    bool prepareStatements();
    int runCountStatement( const char* sql ) const;
    void deleteStatement( sqlite3_stmt*& stmt );

    void exportSpatiaLiteNodes( const QString& tableName, const QStringList& tagKeys );
    void exportSpatiaLiteWays( bool closed, const QString& tableName, const QStringList& tagKeys );
    bool createSpatialTable( const QString& tableName, const QString& geometryType, const QStringList& tagKeys );
    bool createSpatialIndex( const QString& tableName );

    QString quotedIdentifier( QString id );
    QString quotedValue( QString value );

  private:
    //! database file name
    QString mDbFileName;

    QString mError;

    //! pointer to sqlite3 database that keeps OSM data
    sqlite3* mDatabase;

    sqlite3_stmt* mStmtNode;
    sqlite3_stmt* mStmtNodeTags;
    sqlite3_stmt* mStmtWay;
    sqlite3_stmt* mStmtWayNode;
    sqlite3_stmt* mStmtWayNodePoints;
    sqlite3_stmt* mStmtWayTags;
};


/** Encapsulate iteration over table of nodes */
class ANALYSIS_EXPORT QgsOSMNodeIterator
{
  public:
    ~QgsOSMNodeIterator();

    QgsOSMNode next();
    void close();

  protected:
    QgsOSMNodeIterator( sqlite3* handle );

    sqlite3_stmt* mStmt;

    friend class QgsOSMDatabase;
};



/** Encapsulate iteration over table of ways */
class ANALYSIS_EXPORT QgsOSMWayIterator
{
  public:
    ~QgsOSMWayIterator();

    QgsOSMWay next();
    void close();

  protected:
    QgsOSMWayIterator( sqlite3* handle );

    sqlite3_stmt* mStmt;

    friend class QgsOSMDatabase;
};



#endif // OSMDATABASE_H
