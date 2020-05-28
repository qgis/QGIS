/***************************************************************************
   qgsproviderguimetadata.h
                             -------------------
    begin                : June 4th 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERGUIMETADATA_H
#define QGSPROVIDERGUIMETADATA_H

#include <QList>
#include <QMainWindow>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsDataItemGuiProvider;
class QgsSourceSelectProvider;
class QgsProjectStorageGuiProvider;

/**
 * \ingroup gui
 * Holds data for GUI part of the data providers
 *
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsProviderGuiMetadata
{
  public:

    /**
     * Constructor for provider gui metadata
     */
    explicit QgsProviderGuiMetadata( const QString &key );

    virtual ~QgsProviderGuiMetadata();

    /**
     * Called during GUI initialization - allows provider to do its internal initialization
     * of GUI components, possibly making use of the passed pointer to the QGIS main window.
     */
    virtual void registerGui( QMainWindow *widget );

    /**
     * Returns data item gui providers
     * \note Ownership of created data item gui providers is passed to the caller.
     */
    virtual QList<QgsDataItemGuiProvider *> dataItemGuiProviders() SIP_FACTORY;

    /**
     * Returns project storage gui providers
     * \note Ownership of created project storage gui providers is passed to the caller.
     */
    virtual QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() SIP_FACTORY;

    /**
     * Returns source select providers
     * \note Ownership of created source select providers is passed to the caller.
     */
    virtual QList<QgsSourceSelectProvider *> sourceSelectProviders() SIP_FACTORY;

    //! Returns unique provider key
    QString key() const;

  private:
    //! unique key for data provider
    QString mKey;
};

#endif //QGSPROVIDERGUIMETADATA_H

