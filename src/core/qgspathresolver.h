/***************************************************************************
  qgspathresolver.h
  --------------------------------------
  Date                 : February 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPATHRESOLVER_H
#define QGSPATHRESOLVER_H

#include "qgis_core.h"

#include <QString>
#include <functional>

/**
 * \ingroup core
 * Resolves relative paths into absolute paths and vice versa. Used for writing
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPathResolver
{
  public:
    //! Initialize path resolver with a base filename. Null filename means no conversion between relative/absolute path
    explicit QgsPathResolver( const QString &baseFileName = QString() );

    /**
     * Prepare a filename to save it to the project file.
     * Creates an absolute or relative path according to the project settings.
     * Paths written to the project file should be prepared with this method.
    */
    QString writePath( const QString &filename ) const;

    //! Turn filename read from the project file to an absolute path
    QString readPath( const QString &filename ) const;

#ifndef SIP_RUN

    /**
     * Sets a path pre-processor function, which allows for manipulation of paths and data sources prior
     * to resolving them to file references or layer sources.
     *
     * The \a processor function must accept a single string argument (representing the original file path
     * or data source), and return a processed version of this path.
     *
     * The path pre-processor function is called before any bad layer handler.
     *
     * \note Setting a new \a processor replaces any existing processor.
     *
     * \since QGIS 3.10
     */
    static void setPathPreprocessor( const std::function< QString( const QString &filename )> &processor );
#else

    /**
     * Sets a path pre-processor function, which allows for manipulation of paths and data sources prior
     * to resolving them to file references or layer sources.
     *
     * The \a processor function must accept a single string argument (representing the original file path
     * or data source), and return a processed version of this path.
     *
     * The path pre-processor function is called before any bad layer handler.
     *
     * \note Setting a new \a processor replaces any existing processor.
     *
     * Example - replace an outdated folder path with a new one:
     * \code{.py}
     *   def my_processor(path):
     *      return path.replace('c:/Users/ClintBarton/Documents/Projects', 'x:/Projects/')
     *
     *   QgsPathResolver.setPathPreprocessor(my_processor)
     * \endcode
     *
     * Example - replace a stored database host with a new one:
     * \code{.py}
     *   def my_processor(path):
     *      return path.replace('host=10.1.1.115', 'host=10.1.1.116')
     *
     *   QgsPathResolver.setPathPreprocessor(my_processor)
     * \endcode
     *
     * Example - replace stored database credentials with new ones:
     * \code{.py}
     *   def my_processor(path):
     *      path = path.replace("user='gis_team'", "user='team_awesome'")
     *      path = path.replace("password='cats'", "password='g7as!m*'")
     *      return path
     *
     *   QgsPathResolver.setPathPreprocessor(my_processor)
     * \endcode
     *
     * \since QGIS 3.10
     */
    static void setPathPreprocessor( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS
    Py_XINCREF( a0 );
    QgsPathResolver::setPathPreprocessor( [a0]( const QString &arg )->QString
    {
      QString res;
      SIP_BLOCK_THREADS
      PyObject *s = sipCallMethod( NULL, a0, "D", &arg, sipType_QString, NULL );
      int state;
      int sipIsError = 0;
      QString *t1 = reinterpret_cast<QString *>( sipConvertToType( s, sipType_QString, 0, SIP_NOT_NONE, &state, &sipIsError ) );
      if ( sipIsError == 0 )
      {
        res = QString( *t1 );
      }
      sipReleaseType( t1, sipType_QString, state );
      SIP_UNBLOCK_THREADS
      return res;
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

  private:
    //! path to a file that is the base for relative path resolution
    QString mBaseFileName;

    static std::function< QString( const QString & ) > sCustomResolver;
};

#endif // QGSPATHRESOLVER_H
