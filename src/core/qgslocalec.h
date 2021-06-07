/***************************************************************************
                          qgslocalec.h - temporary C numeric locale
                             -------------------
    begin                : Jun 15th 2015
    copyright            : (C) 2015 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
