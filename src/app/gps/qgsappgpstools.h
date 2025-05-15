/***************************************************************************
    qgsappgpstools.h
    ---------------------
    begin                : May 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPGPSTOOLS_H
#define QGSAPPGPSTOOLS_H

#include <qgsgpstoolsinterface.h>

/**
 * \brief QgsAppGpsTools
 * Interface to make GPS tools available to plugins.
 */
class QgsAppGpsTools : public QgsGpsToolsInterface
{
  public:
    QgsAppGpsTools();

    void setGpsPanelConnection( QgsGpsConnection *connection SIP_TRANSFER ) override;
    void createFeatureFromGpsTrack() override;
    void setGpsTrackLineSymbol( QgsLineSymbol *symbol ) override;
};

#endif // QGSAPPGPSTOOLS_H
