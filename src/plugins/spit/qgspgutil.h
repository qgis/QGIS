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
#include <QStringList>
extern "C"
{
#include <libpq-fe.h>
}

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

    /*!
     * Set the connection to be used in database operations
     * @param con Pointer to an active PostgreSQL connection
     */
    void setConnection( PGconn *con );
    /*!
     * Get the connection currently in use for database operations
     * @return Pointer to the PostgreSQL connection object
     */
    PGconn *connection();

    /** Double quote a PostgreSQL identifier for placement in a SQL string.
    */
    static QString quotedIdentifier( QString ident );

    /** Quote a value for placement in a SQL string.
     */
    static QString quotedValue( QString value );

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
    //! PostgreSQL connection
    PGconn *mPgConnection;
};
#endif // QGSPGUTIL_H
