/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmdatabase.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






typedef QPair<QString, int> QgsOSMTagCountPair;

class QgsOSMDatabase
{
%Docstring
 Class that encapsulates access to OpenStreetMap data stored in a database
 previously imported from XML file.

 Internal database structure consists of the following tables:
 - nodes
 - nodes_tags
 - ways
 - ways_tags
 - ways_nodes

 The topology representation can be translated to simple features representation
 using exportSpatiaLite() method into SpatiaLite layers (tables). These can be
 easily used in QGIS like any other layers.
%End

%TypeHeaderCode
#include "qgsosmdatabase.h"
%End
  public:
    explicit QgsOSMDatabase( const QString &dbFileName = QString() );
    ~QgsOSMDatabase();


    void setFileName( const QString &dbFileName );
%Docstring
Setter for the SpatiaLite database.
%End
    QString filename() const;
%Docstring
 :rtype: str
%End
    bool isOpen() const;
%Docstring
 :rtype: bool
%End

    bool open();
%Docstring
 :rtype: bool
%End
    bool close();
%Docstring
 :rtype: bool
%End

    QString errorString() const;
%Docstring
 :rtype: str
%End


    int countNodes() const;
%Docstring
 :rtype: int
%End
    int countWays() const;
%Docstring
 :rtype: int
%End



    QgsOSMNode node( QgsOSMId id ) const;
%Docstring
 :rtype: QgsOSMNode
%End
    QgsOSMWay way( QgsOSMId id ) const;
%Docstring
 :rtype: QgsOSMWay
%End

    QgsOSMTags tags( bool way, QgsOSMId id ) const;
%Docstring
 :rtype: QgsOSMTags
%End


    QgsPolyline wayPoints( QgsOSMId id ) const;
%Docstring
 :rtype: QgsPolyline
%End


    enum ExportType { Point, Polyline, Polygon };
    bool exportSpatiaLite( ExportType type, const QString &tableName,
                           const QStringList &tagKeys = QStringList(),
                           const QStringList &noNullTagKeys = QStringList() );
%Docstring
 :rtype: bool
%End

  protected:
    bool prepareStatements();
%Docstring
 :rtype: bool
%End
    int runCountStatement( const char *sql ) const;
%Docstring
 :rtype: int
%End


    void exportSpatiaLiteNodes( const QString &tableName, const QStringList &tagKeys, const QStringList &notNullTagKeys = QStringList() );
    void exportSpatiaLiteWays( bool closed, const QString &tableName, const QStringList &tagKeys, const QStringList &notNullTagKeys = QStringList() );
    bool createSpatialTable( const QString &tableName, const QString &geometryType, const QStringList &tagKeys );
%Docstring
 :rtype: bool
%End
    bool createSpatialIndex( const QString &tableName );
%Docstring
 :rtype: bool
%End

    QString quotedIdentifier( QString id );
%Docstring
 :rtype: str
%End
    QString quotedValue( QString value );
%Docstring
 :rtype: str
%End

  private:
    QgsOSMDatabase( const QgsOSMDatabase &rh );
};








/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmdatabase.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
