/***************************************************************************
                             qgshtmlutils.h
                             ---------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHTMLUTILS_H
#define QGSHTMLUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QString>

/**
 * \ingroup core
 * \class QgsHtmlUtils
 * \brief Class for HTML utilities.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsHtmlUtils
{
  public:

    /**
     * Build a bullet list.
     * This will return a HTML "ul" element.
     */
    static QString buildBulletList( const QStringList &values );
};

#endif // QGSHTMLUTILS_H
