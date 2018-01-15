/***************************************************************************
                          qgslocalec.h - temporary C numeric locale
                             -------------------
    begin                : Jun 15th 2015
    copyright            : (C) 2015 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCALENUMC_H
#define QGSLOCALENUMC_H

#define SIP_NO_FILE

#include <QMutex>

#include "qgis_core.h"

/**
 * \ingroup core
 */
class CORE_EXPORT QgsLocaleNumC
{
    char *mOldlocale = nullptr;
    static QMutex sLocaleLock;

  public:
    QgsLocaleNumC();
    ~QgsLocaleNumC();

    //! QgsLocaleNumC cannot be copied
    QgsLocaleNumC( const QgsLocaleNumC &rh ) = delete;
    //! QgsLocaleNumC cannot be copied
    QgsLocaleNumC &operator=( const QgsLocaleNumC &rh ) = delete;

};

#endif // QGSLOCALENUMC_H
