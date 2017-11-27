/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsconfigcache.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsConfigCache : QObject
{
%Docstring
*************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgsconfigcache.h"
%End
  public:
    static QgsConfigCache *instance();
%Docstring
 :rtype: QgsConfigCache
%End

    void removeEntry( const QString &path );

    const QgsProject *project( const QString &path );
%Docstring
 If the project is not cached yet, then the project is read thank to the
  path. If the project is not available, then a None is returned.
 \param path the filename of the QGIS project
 :return: the project or None if an error happened
.. versionadded:: 3.0
 :rtype: QgsProject
%End

  private:
    QgsConfigCache() ;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsconfigcache.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
