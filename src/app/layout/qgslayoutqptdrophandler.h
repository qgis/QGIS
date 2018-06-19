/***************************************************************************
    qgslayoutqptdrophandler.h
    -------------------------
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

#ifndef QGSLAYOUTQPTDROPHANDLER_H
#define QGSLAYOUTQPTDROPHANDLER_H

#include "qgslayoutcustomdrophandler.h"

class QgsLayoutQptDropHandler : public QgsLayoutCustomDropHandler
{
    Q_OBJECT

  public:

    QgsLayoutQptDropHandler( QObject *parent = nullptr );

    bool handleFileDrop( QgsLayoutDesignerInterface *iface, const QString &file ) override;
};

#endif // QGSLAYOUTQPTDROPHANDLER_H
