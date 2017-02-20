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
#include "qgis_analysis.h"

class QXmlStreamReader;

/** \ingroup analysis
 * @brief The QgsOSMXmlImport class imports OpenStreetMap XML format to our topological representation
 * in a SQLite database (see QgsOSMDatabase for details).
 *
 * How to use the class:
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

    /**
     * Sets the filename for the output database.
     * @see outputDatabaseFileName()
     */
    void setOutputDatabaseFileName( const QString& fileName ) { mDbFileName = fileName; }

    /**
     * Returns the filename for the output database.
     * @see setOutputDatabaseFileName()
     */
    QString outputDatabaseFileName() const { return mDbFileName; }

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

    //! @note not available in Python bindings
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

    sqlite3* mDatabase = nullptr;
    sqlite3_stmt* mStmtInsertNode = nullptr;
    sqlite3_stmt* mStmtInsertNodeTag = nullptr;
    sqlite3_stmt* mStmtInsertWay = nullptr;
    sqlite3_stmt* mStmtInsertWayNode = nullptr;
    sqlite3_stmt* mStmtInsertWayTag = nullptr;
};



#endif // OSMIMPORT_H
