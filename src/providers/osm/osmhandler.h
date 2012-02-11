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


/**
 * The SAX handler used for parsing input OSM XML file.
 * While processing XML file it stores data to specified sqlite database.
 */
class OsmHandler: public QXmlDefaultHandler
{
  public:
    /**
     * Construction.
     * @param f input file with OSM data
     * @param database opened sqlite3 database with OSM db schema to store data in
     */
    OsmHandler( QFile *f, sqlite3 *database );

    /**
     * Destruction.
     */
    ~OsmHandler();

    // input OSM XML processing

    /**
     * Function is called after XML processing is started.
     * @return True if start of document signal is processed without problems; False otherwise
     */
    bool startDocument();

    /**
     * The reader of OSM file calls this function when it has parsed a start element tag.
     * If this function returns false the reader stops parsing and reports an error.
     * The reader uses the function errorString() to get the error message.
     *
     * @param pUri namespace URI of element, or an empty string if the element has no namespace URI or if no namespace processing is done
     * @param pLocalName local name of element (without prefix), or an empty string if no namespace processing is done
     * @param pName qualified name of element (with prefix)
     * @param pAttrs attributes attached to the element; if there are no attributes, atts is an empty attributes object.
     * @return True if start of element signal is processed without problems; False otherwise
     */
    bool startElement( const QString & pUri, const QString & pLocalName, const QString & pName, const QXmlAttributes & pAttrs );

    /**
     * The reader calls this function when it has parsed an end element tag with the qualified name pName,
     * the local name pLocalName and the namespace URI pURI.
     * If this function returns false the reader stops parsing and reports an error.
     * The reader uses the function errorString() to get the error message.
     *
     * @param pUri namespace URI of element, or an empty string if the element has no namespace URI or if no namespace processing is done
     * @param pLocalName local name of element (without prefix), or an empty string if no namespace processing is done
     * @param pName qualified name of element (with prefix)
     * @return True if end of element signal is processed without problems; False otherwise
     */
    bool endElement( const QString & pURI, const QString & pLocalName, const QString & pName );

    /**
     * Function is called after end of document is reached while XML processing.
     * @return True if end of document signal is processed without problems; False otherwise
     */
    bool endDocument();

    /**
     * Returns information on error that occures while parsing.
     * @return info on error that occures while parsing
     */
    QString errorString() const;

  public:
    int mPointCnt;
    int mLineCnt;
    int mPolygonCnt;
    double xMin, xMax, yMin, yMax;

  private:
    sqlite3_stmt *mStmtInsertNode;
    sqlite3_stmt *mStmtInsertWay;
    sqlite3_stmt *mStmtInsertTag;
    sqlite3_stmt *mStmtInsertWayMember;
    sqlite3_stmt *mStmtInsertRelation;
    sqlite3_stmt *mStmtInsertRelationMember;
    sqlite3_stmt *mStmtUpdateNode;
    sqlite3_stmt *mStmtInsertVersion;

    sqlite3 *mDatabase;
    int mPosId;
    QString firstWayMemberId;
    QString lastWayMemberId;
    int mFirstMemberAppeared;
    int mCnt;
    QString mError;
    QString mObjectId;         //last node, way or relation id while parsing file
    QString mObjectType;       //one of "node", "way", "relation"
    QString mRelationType;
};





