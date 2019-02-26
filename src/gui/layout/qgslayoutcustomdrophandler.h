/***************************************************************************
    qgslayoutcustomdrophandler.h
    ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTCUSTOMDROPHANDLER_H
#define QGSLAYOUTCUSTOMDROPHANDLER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QObject>

class QgsLayoutDesignerInterface;

/**
 * \ingroup gui
 * Abstract base class that may be implemented to handle new types of data to be dropped in QGIS layouts.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutCustomDropHandler : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutCustomDropHandler.
     */
    QgsLayoutCustomDropHandler( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Called when the specified \a file has been dropped onto a QGIS layout. If TRUE
     * is returned, then the handler has accepted this file and it should not
     * be further processed (e.g. by other QgsLayoutCustomDropHandler).
     *
     * The base class implementation does nothing.
     */
    virtual bool handleFileDrop( QgsLayoutDesignerInterface *iface, const QString &file );
};

#endif // QGSLAYOUTCUSTOMDROPHANDLER_H
