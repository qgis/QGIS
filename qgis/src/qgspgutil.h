/***************************************************************************
                qgspgutil.h - PostgreSQL Utility Functions
                     --------------------------------------
               Date                 : 2004-11-21
               Copyright            : (C) 2004 by Gary E.Sherman
               Email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSPGUTIL_H
#define QGSPGUTIL_H
#include <map>
#include <qstringlist.h>
/*!
 * \class QgsPgUtil
 * \brief Class containing utility functions for working with PostgreSQL
 * tables/databases
 */
class QgsPgUtil
{
  public:
    //! Instance function to return a pointer to the one and
    //only QgsPgUtil object (QgsPgUtil is a Singleton)
    static QgsPgUtil* instance();
    /*! Checks to see if word is a PG reserved word.
     * The comparison is case-insensitive.
     * @param word Word to check
     * @return True if word is a PG reserved word
     */
    bool isReserved(QString word);
  protected:
    //! Protected constructor
    QgsPgUtil();
    //! Destructor
    ~QgsPgUtil();
  private:
    //! Initialize the list of reserved words
    void initReservedWords();
    //! Instance member
    static QgsPgUtil* mInstance;
    //! Reserved word list
    QStringList mReservedWords;
};
#endif // QGSPGUTIL_H
