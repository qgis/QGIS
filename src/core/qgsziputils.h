/***************************************************************************
                                  qgsziputils.h
                              ---------------------
    begin                : Jul 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSZIPUTILS_H
#define QGSZIPUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QStringList>

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsziputils.h"
% End
#endif

namespace QgsZipUtils
{

  /**
   * Returns TRUE if the file name is a zipped file ( i.e with a '.qgz'
   *  extension, FALSE otherwise.
   * \param filename The name of the file
   * \returns TRUE if the file is zipped, FALSE otherwise
   */
  CORE_EXPORT bool isZipFile( const QString &filename );

  /**
   * Unzip a zip file in an output directory.
   * \param zip The zip filename
   * \param dir The output directory
   * \param files The absolute path of unzipped files
   * \returns FALSE if the zip filename does not exist, the output directory
   * does not exist or is not writable.
   * \since QGIS 3.0
   */
  CORE_EXPORT bool unzip( const QString &zip, const QString &dir, QStringList &files SIP_OUT );

  /**
   * Zip the list of files in the zip file. If the zip file already exists or is
   *  empty, an error is returned. If an input file does not exist, an error is
   *  also returned.
   * \param zip The zip filename
   * \param files The absolute path to files to embed within the zip
   * \since QGIS 3.0
   */
  CORE_EXPORT bool zip( const QString &zip, const QStringList &files );
};

#endif //QGSZIPUTILS_H
