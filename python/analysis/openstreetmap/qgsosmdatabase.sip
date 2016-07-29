typedef QPair<QString, int> QgsOSMTagCountPair;

/**
 * Class that encapsulates access to OpenStreetMap data stored in a database
 * previously imported from XML file.
 *
 * Internal database structure consists of the following tables:
 * - nodes
 * - nodes_tags
 * - ways
 * - ways_tags
 * - ways_nodes
 *
 * The topology representation can be translated to simple features representation
 * using exportSpatiaLite() method into SpatiaLite layers (tables). These can be
 * easily used in QGIS like any other layers.
 */
class QgsOSMDatabase
{
%TypeHeaderCode
#include <qgsosmdatabase.h>
%End
  public:
    explicit QgsOSMDatabase( const QString& dbFileName = QString() );
    ~QgsOSMDatabase();

    void setFileName( const QString& dbFileName );
    QString filename() const;
    bool isOpen() const;

    bool open();
    bool close();

    QString errorString() const;

    // data access

    int countNodes() const;
    int countWays() const;

    //! @note not available in Python bindings
    //QgsOSMNodeIterator listNodes() const;
    //! @note not available in Python bindings
    //QgsOSMWayIterator listWays() const;

    QgsOSMNode node( qint64 id ) const;
    QgsOSMWay way( qint64 id ) const;
    //OSMRelation relation( OSMId id ) const;

    QgsOSMTags tags( bool way, qint64 id ) const;

    //! @note available in Python bindings
    //QList<QPair<QString, int>> usedTags( bool ways ) const;

    QgsPolyline wayPoints( qint64 id ) const;

    // export to spatialite

    enum ExportType { Point, Polyline, Polygon };
    bool exportSpatiaLite( ExportType type, const QString& tableName,
                           const QStringList& tagKeys = QStringList(),
                           const QStringList& noNullTagKeys = QStringList() );

  protected:
    bool prepareStatements();
    int runCountStatement( const char* sql ) const;

    /** @note not available in Python bindings
     */
    //void deleteStatement( sqlite3_stmt*& stmt );

    void exportSpatiaLiteNodes( const QString& tableName, const QStringList& tagKeys, const QStringList& notNullTagKeys = QStringList() );
    void exportSpatiaLiteWays( bool closed, const QString& tableName, const QStringList& tagKeys, const QStringList& notNullTagKeys = QStringList() );
    bool createSpatialTable( const QString& tableName, const QString& geometryType, const QStringList& tagKeys );
    bool createSpatialIndex( const QString& tableName );

    QString quotedIdentifier( QString id );
    QString quotedValue( QString value );

  private:

    QgsOSMDatabase( const QgsOSMDatabase& rh );
};


