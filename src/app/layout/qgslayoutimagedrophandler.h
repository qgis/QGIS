/***************************************************************************
    qgslayoutimagedrophandler.h
    -------------------------
    begin                : November 2019
    copyright            : (C) 2019 by nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTIMAGEDROPHANDLER_H
#define QGSLAYOUTIMAGEDROPHANDLER_H

#include "qgslayoutcustomdrophandler.h"

class QgsLayoutImageDropHandler : public QgsLayoutCustomDropHandler
{
    Q_OBJECT

  public:
    QgsLayoutImageDropHandler( QObject *parent = nullptr );

    bool handleFileDrop( QgsLayoutDesignerInterface *iface, QPointF point, const QString &file ) override;
    bool handlePaste( QgsLayoutDesignerInterface *iface, QPointF pastePoint, const QMimeData *data, QList<QgsLayoutItem *> &pastedItems ) override;
};

#endif // QGSLAYOUTIMAGEDROPHANDLER_H
