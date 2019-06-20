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
    QgsProviderGuiMetadata( const QString &key, const QString &description );

    virtual ~QgsProviderGuiMetadata();

    //! Returns data item gui providers
    virtual QList<QgsDataItemGuiProvider *> dataItemGuiProviders();

    //! Assigns parent to widget
    virtual void registerGui( QMainWindow *widget );

    //! Returns project storage gui providers
    virtual QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders();

    //! Returns source select providers
    virtual QList<QgsSourceSelectProvider *> sourceSelectProviders();

    //! Returns unique provider key
    QString key() const;

    //! Returns provider description
    QString description() const;

  private:
    /// unique key for data provider
    QString mKey;

    /// associated terse description
    QString mDescription;
};

#endif //QGSPROVIDERGUIMETADATA_H

