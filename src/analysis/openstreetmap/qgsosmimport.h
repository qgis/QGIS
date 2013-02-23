#ifndef OSMIMPORT_H
#define OSMIMPORT_H

#include <QFile>
#include <QObject>

#include "qgsosmbase.h"

class QXmlStreamReader;


class ANALYSIS_EXPORT QgsOSMXmlImport : public QObject
{
    Q_OBJECT
  public:
    explicit QgsOSMXmlImport( const QString& xmlFileName = QString(), const QString& dbFileName = QString() );

    void setInputXmlFileName( const QString& xmlFileName ) { mXmlFileName = xmlFileName; }
    QString inputXmlFileName() const { return mXmlFileName; }

    void setOutputDbFileName( const QString& dbFileName ) { mDbFileName = dbFileName; }
    QString outputDbFileName() const { return mDbFileName; }

    bool import();

    bool hasError() const { return !mError.isEmpty(); }
    QString errorString() const { return mError; }

  signals:
    void progress( int percent );

  protected:

    bool createDatabase();
    bool closeDatabase();
    void deleteStatement( sqlite3_stmt*& stmt );

    bool createIndexes();

    void readRoot( QXmlStreamReader& xml );
    void readNode( QXmlStreamReader& xml );
    void readWay( QXmlStreamReader& xml );
    void readTag( bool way, QgsOSMId id, QXmlStreamReader& xml );

  private:
    QString mXmlFileName;
    QString mDbFileName;

    QString mError;

    QFile mInputFile;

    sqlite3* mDatabase;
    sqlite3_stmt* mStmtInsertNode;
    sqlite3_stmt* mStmtInsertNodeTag;
    sqlite3_stmt* mStmtInsertWay;
    sqlite3_stmt* mStmtInsertWayNode;
    sqlite3_stmt* mStmtInsertWayTag;
};



#endif // OSMIMPORT_H
