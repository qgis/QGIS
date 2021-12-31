/***************************************************************************
                            qgsprocessinghistoryprovider.h
                            --------------------------
    begin                : December 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROCESSINGHISTORYPROVIDER_H
#define QGSPROCESSINGHISTORYPROVIDER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include "qgshistoryprovider.h"

/**
 * History provider for operations performed through the Processing framework.
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsProcessingHistoryProvider : public QgsAbstractHistoryProvider
{
  public:

    QgsProcessingHistoryProvider();

    QString id() const override;

    /**
     * Ports the old text log to the history framework.
     *
     * This should only be called once -- calling multiple times will result in duplicate log entries
     */
    void portOldLog();

  private:

    //! Returns the path to the old log file
    QString oldLogPath() const;

};

#endif //QGSHISTORYPROVIDER_H



