/***************************************************************************
                            qgssearchstring.h
          interface for parsing and evaluation of search strings
                          --------------------
    begin                : 2005-07-26
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSSEARCHSTRING_H
#define QGSSEARCHSTRING_H

#include <QString>

class QgsSearchTreeNode;

/** \ingroup core
 * A class to represent a search string.
 * - interface for 'search string' parser
 * - when a string is set, it parses it and creates parsed tree of it
 * - owns node tree and coresponding search string
 * - keeps string and node tree always in sync
 *
 */
class CORE_EXPORT QgsSearchString
{
  public:
    //! construct an empty string
    QgsSearchString();

    //! construct and parse a string
    //! @note added in v1.6
    QgsSearchString( const QString & str );

    //! copy constructor - makes also copy of search tree
    QgsSearchString( const QgsSearchString& str );

    //! destructor - deletes node tree
    ~QgsSearchString();

    //! assignment operator takes care to copy search tree correctly
    QgsSearchString& operator=( const QgsSearchString& str );

    /** sets search string and parses search tree
        on success returns true and sets member variables to the new values */
    bool setString( QString str );

    /* copies tree and makes search string for it
       on success returns true and sets member variables to the new values */
    bool setTree( QgsSearchTreeNode* tree );

    //! getter functions
    QgsSearchTreeNode* tree() { return mTree; }
    QString string() { return mString; }

    //! returns parser error message - valid only after unsuccessfull parsing
    const QString& parserErrorMsg() { return mParserErrorMsg; }

    //! returns true if no string is set
    bool isEmpty();

    //! clear search string
    void clear();

  private:
    //! search string and coresponding tree
    QgsSearchTreeNode*    mTree;
    QString               mString;

    //! error message from parser
    QString               mParserErrorMsg;
};

#endif
