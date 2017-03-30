/***************************************************************************
                          qgsexception.h
                             -------------------
  begin                : August 31, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXCEPTION_H
#define QGSEXCEPTION_H

#include <QStringList>
#include "qgsstacktrace.h"

#include "qgis_core.h"

/** \ingroup core
  * Defines a qgis exception class.
 */
class CORE_EXPORT QgsException
{
  public:
    QgsException( QString const &what )
      : mWhat( what )
#ifdef QGISDEBUG
      , mStack( QgsStacktrace::trace() )
#endif
    {}

    virtual ~QgsException() throw()
    {}

    //! @note not available in Python bindings
    QString what() const throw()
    {
      return mWhat;
    }

#ifndef QGISDEBUG
    QString stack() { return QStringLiteral( "Stack information not available in release." ); }
#else
    QString stack()
    {
      QString stack;
      if ( mStack.isEmpty() )
      {
        stack = QStringLiteral( "Stack not available. Run a Debug build of QGIS to get more information." );
      }
      Q_FOREACH ( const QString &entry, mStack )
      {
        if ( entry.isNull() )
        {
          stack += QStringLiteral( "\n   Frame not available (possibly corrupted)" );
        }
        else
        {
          stack += entry;
        }
      }

      return stack;
    }
#endif

  private:

    /// description of exception
    QString mWhat;
#ifdef QGISDEBUG
    QStringList mStack;
#endif

}; // class QgsException


#endif
