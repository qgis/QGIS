/***************************************************************************
  qgsosmimport.h
  --------------------------------------
  Date                 : January 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OSMIMPORT_H
#define OSMIMPORT_H

#include <QFile>
#include <QObject>

#include "qgsosmbase.h"

class QXmlStreamReader;

/**
 * @brief The QgsOSMXmlImport class imports OpenStreetMap XML format to our topological representation
 * in a SQLite database (see QgsOSMDatabase for details).
 *
 * How to use the classs:
 * 1. set input XML file name and output DB file name (in constructor or with respective functions)
 * 2. run import()
 * 3. check errorString() if the import failed
 */
class ANALYSIS_EXPORT QgsOSMXmlImport : public QObject
{
    Q_OBJECT
  public:
    explicit QgsOSMXmlImport( const QString& xmlFileName = QString(), const QString& dbFileName = QString() );

    void setInputXmlFileName( const QString& xmlFileName ) { mXmlFileName = xmlFileName; }
    QString inputXmlFileName() const { return mXmlFileName; }

    void setOutputDbFileName( const QString& dbFileName ) { mDbFileName = dbFileName; }
    QString outputDbFileName() const { return mDbFileName; }

    /**
     * Run import. This will parse the XML file and store the data in a SQLite database.
     * @return true on success, false when import failed (see errorString() for the error)
     */
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
