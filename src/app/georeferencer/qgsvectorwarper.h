/***************************************************************************
     qgsvectorwarper.h
     --------------------------------------
    Date                 :
    Copyright            :
    Email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORWARPER_H
#define QGSVECTORWARPER_H

#include <QCoreApplication>
#include <QString>

#include <vector>

class QWidget;

/**
 * Base implementation of QgsVectorWarper.
 * Creates the process to perform the ogr2ogr call.
 * \since QGIS 3.20
 */
class QgsVectorWarper
{
  public:
    explicit QgsVectorWarper();

    /**
     * Functions that creates a QgsBlockingProcess to call ogr2ogr.
     * \param command command string to use for the process.
     * \return True if operation finished properly, otherwise false.
     * \since QGIS 3.20
     */
    bool executeGDALCommand( const QString &fused_command );

};


#endif
