/***************************************************************************
  qgsbabelformatregistry.h
   -------------------
  begin                : July 2021
  copyright            : (C) 2021 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBABELFORMATREGISTRY_H
#define QGSBABELFORMATREGISTRY_H

#include "qgis_core.h"
#include "qgis.h"

class QgsBabelSimpleImportFormat;
class QgsBabelGpsDeviceFormat;

/**
 * \ingroup core
 * \class QgsBabelFormatRegistry
 * \brief A registry for QgsAbstractBabelFormat GPSBabel formats.
 *
 * QgsBabelFormatRegistry is not usually directly created, but rather accessed through
 * QgsApplication::gpsBabelFormatRegistry().
 *
 * \since QGIS 3.22
*/
class CORE_EXPORT QgsBabelFormatRegistry
{
  public:

    /**
     * Constructor for QgsBabelFormatRegistry.
     *
     * The registry will automatically be populated with standard formats, and with
     * devices previously configured and stored in QSettings.
     */
    QgsBabelFormatRegistry();
    ~QgsBabelFormatRegistry();

    //! QgsBabelFormatRegistry cannot be copied.
    QgsBabelFormatRegistry( const QgsBabelFormatRegistry &rh ) = delete;
    //! QgsBabelFormatRegistry cannot be copied.
    QgsBabelFormatRegistry &operator=( const QgsBabelFormatRegistry &rh ) = delete;

    /**
     * Returns a list of the names of all registered import formats.
     */
    QStringList importFormatNames() const;

    /**
     * Returns a registered import format by \a name.
     *
     * \see importFormatNames()
     * \see importFormatByDescription()
     */
    QgsBabelSimpleImportFormat *importFormat( const QString &name );

    /**
     * Returns a registered import format by \a description.
     *
     * \see importFormat()
     */
    QgsBabelSimpleImportFormat *importFormatByDescription( const QString &description );

    /**
     * Returns a file filter string representing all registered import formats.
     */
    QString importFileFilter() const;

    /**
     * Returns a list of the names of all registered devices.
     */
    QStringList deviceNames() const;

    /**
     * Returns a registered device format by \a name.
     *
     * \see deviceNames().
     */
    QgsBabelGpsDeviceFormat *deviceFormat( const QString &name );

    /**
     * Returns a map of device name to device format.
     *
     * \see deviceFormat()
     * \see deviceNames()
     */
    QMap< QString, QgsBabelGpsDeviceFormat * > devices() const;

    /**
     * Reloads the registry's members from the currently stored configuration.
     */
    void reloadFromSettings();

  private:
#ifdef SIP_RUN
    QgsBabelFormatRegistry( const QgsBabelFormatRegistry &rh );
#endif

    //! Importers for external GPS data file formats
    QMap< QString, QgsBabelSimpleImportFormat *> mImporters;
    //! Upload/downloaders for GPS devices
    QMap< QString, QgsBabelGpsDeviceFormat *> mDevices;
};


#endif // QGSBABELFORMATREGISTRY_H
