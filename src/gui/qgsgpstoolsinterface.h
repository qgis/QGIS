/***************************************************************************
    qgsgpstoolsinterface.h
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
#ifndef QGSGPSTOOLSINTERFACE_H
#define QGSGPSTOOLSINTERFACE_H

#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsGpsConnection;
class QgsLineSymbol;

/**
 * \ingroup gui
 * \class QgsGpsToolsInterface
 * \brief Abstract interface class for the QGIS GPS tools.
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsGpsToolsInterface
{
  public:
    virtual ~QgsGpsToolsInterface() = default;

    /**
     * Sets a GPS \a connection to use within the GPS Panel widget.
     *
     * Any existing GPS connection used by the widget will be disconnected and replaced with this connection. The connection
     * is automatically registered within the QgsApplication::gpsConnectionRegistry().
     */
    virtual void setGpsPanelConnection( QgsGpsConnection *connection SIP_TRANSFER ) = 0;

    /**
     * Creates a feature from the current GPS track.
     *
     * The geometry type of the feature is determined by the layer set via
     * QgsProjectGpsSettings::destinationLayer().
     *
     * The created geometry will be automatically commited depending on the
     * status of QgsProjectGpsSettings::automaticallyCommitFeatures().
     */
    virtual void createFeatureFromGpsTrack() = 0;

    /**
     * Sets the line \a symbol of the GPS track and changes the QgsAppGpsDigitizing::settingTrackLineSymbol setting.
     *
     * If there is a current GPS track, its appearance is updated according to the symbol.
     */
    virtual void setGpsTrackLineSymbol( QgsLineSymbol *symbol ) = 0;
};

#endif // QGSGPSTOOLSINTERFACE_H
