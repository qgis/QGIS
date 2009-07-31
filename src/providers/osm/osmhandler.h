/***************************************************************************
    osmhandler.h - handler for parsing OSM data
    ------------------
    begin                : October 2008
    copyright            : (C) 2008 by Lukas Berka
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFile>
#include <QProgressDialog>
#include <QString>
#include <QXmlDefaultHandler>
#include <QXmlAttributes>

#include <sqlite3.h>

#include <iostream>
using namespace std;

/**
 * XML SAX handler -> while processing XML file,
 * stores data to specified sqlite database
 */
class OsmHandler: public QXmlDefaultHandler
{
  public:
// member variables

    QFile mFile;
    sqlite3 *mDatabase;
    int mCnt;

    int mFileSize;
    QString mError;
    // xml processing information
    QString mObjectId;     //last node, way or relation id while parsing file
    QString mObjectType;   //one of strings "node", "way", "relation"
    QString mRelationType;
    float mLat;
    float mLon;
    double xMin, xMax, yMin, yMax;

    int mPointCnt;
    int mLineCnt;
    int mPolygonCnt;

    int mCurrent_way_id;
    int mPosId;
    QString firstWayMemberId;
    QString lastWayMemberId;
    int mFirstMemberAppeared;

//functions

    // object construction
    OsmHandler( QFile *f, sqlite3 *database );
    ~OsmHandler();
    // xml processing

    bool startDocument();
    QString errorString();
    bool startElement( const QString & pUri, const QString & pLocalName, const QString & pName, const QXmlAttributes & pAttrs );
    bool endElement( const QString & pURI, const QString & pLocalName, const QString & pName );
    bool endDocument();

  private:
    sqlite3_stmt *mStmtInsertNode;
    sqlite3_stmt *mStmtInsertWay;
    sqlite3_stmt *mStmtInsertTag;
    sqlite3_stmt *mStmtInsertWayMember;
    sqlite3_stmt *mStmtInsertRelation;
    sqlite3_stmt *mStmtInsertRelationMember;
    sqlite3_stmt *mStmtUpdateNode;
    sqlite3_stmt *mStmtInsertVersion;
};





