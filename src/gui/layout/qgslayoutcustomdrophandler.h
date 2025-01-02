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
class QMimeData;
class QgsLayoutItem;

/**
 * \ingroup gui
 * \brief Abstract base class that may be implemented to handle new types of data to be dropped or pasted in QGIS layouts.
 *
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
     *
     * \deprecated QGIS 3.40. Use the version which specifies a drop location instead.
     */
    Q_DECL_DEPRECATED virtual bool handleFileDrop( QgsLayoutDesignerInterface *iface, const QString &file ) SIP_DEPRECATED;

    /**
     * Called when the specified \a file has been dropped onto a QGIS layout. If TRUE
     * is returned, then the handler has accepted this file and it should not
     * be further processed (e.g. by other QgsLayoutCustomDropHandler).
     *
     * The \a dropPoint point specifies the location (in layout coordinates) at which
     * the drop occurred.
     *
     * The base class implementation does nothing.
     *
     * \since QGIS 3.12
     */
    virtual bool handleFileDrop( QgsLayoutDesignerInterface *iface, QPointF dropPoint, const QString &file );

    /**
     * Called when the specified MIME \a data has been pasted onto a QGIS layout. If TRUE
     * is returned, then the handler has accepted this data and it should not
     * be further processed (e.g. by other QgsLayoutCustomDropHandler).
     *
     * The \a pastePoint point specifies the location (in layout coordinates) at which
     * the paste occurred.
     *
     * The base class implementation does nothing.
     *
     * \param iface pointer to the layout designer interface
     * \param pastePoint layout point at which the paste should occur
     * \param data MIME data to paste
     * \param pastedItems should be filled with any newly created items as a result of the paste
     *
     * \returns TRUE if the handler accepted and processed the paste operation
     *
     * \since QGIS 3.14
     */
    virtual bool handlePaste( QgsLayoutDesignerInterface *iface, QPointF pastePoint, const QMimeData *data, QList<QgsLayoutItem *> &pastedItems SIP_OUT );
};


#endif // QGSLAYOUTCUSTOMDROPHANDLER_H
