/***************************************************************************
  qgis_quick.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGIS_QUICK_H
#define QGIS_QUICK_H

#ifndef QUICK_EXPORT
#  ifdef qgis_quick_EXPORTS
/* We are building this library */
#     define QUICK_EXPORT __attribute__((visibility("default")))
#  else
/* We are using this library */
#     define QUICK_EXPORT __attribute__((visibility("default")))
#  endif
#endif //QUICK_EXPORT

#ifndef QUICK_NO_EXPORT
#  define QUICK_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif // QGIS_QUICK_H
