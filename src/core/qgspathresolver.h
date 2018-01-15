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

  private:
    //! path to a file that is the base for relative path resolution
    QString mBaseFileName;
};

#endif // QGSPATHRESOLVER_H
