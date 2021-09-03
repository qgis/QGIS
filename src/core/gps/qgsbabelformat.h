/***************************************************************************
  qgsbabelformat.h - import/export formats for GPSBabel
   -------------------
  begin                : Oct 20, 2004
  copyright            : (C) 2004 by Lars Luthman
  email                : larsl@users.sourceforge.net

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBABELFORMAT_H
#define QGSBABELFORMAT_H

#include "qgis_core.h"
#include "qgis.h"
#include <QStringList>

class QString;

/**
 * \ingroup core
 * \class QgsAbstractBabelFormat
 * \brief Abstract base class for babel GPS formats.
 *
 * \since QGIS 3.22
*/
class CORE_EXPORT QgsAbstractBabelFormat
{
  public:

    virtual ~QgsAbstractBabelFormat() = default;

    /**
     * Returns the format's name.
     */
    QString name() const;

    /**
     * Returns the format's capabilities.
     */
    Qgis::BabelFormatCapabilities capabilities() const;

    /**
     * Generates a command for importing data into a GPS format using babel.
     *
     * \param babel babel path to the babel executable
     * \param featureType type of GPS feature to import
     * \param input input data path
     * \param output output path
     * \param flags optional flags to control how babel command is generated
     *
     * Returns an empty list if the format does not support imports (see capabilities()).
     */
    virtual QStringList importCommand( const QString &babel,
                                       Qgis::GpsFeatureType featureType,
                                       const QString &input,
                                       const QString &output,
                                       Qgis::BabelCommandFlags flags = Qgis::BabelCommandFlags() ) const;

    /**
     * Generates a command for exporting GPS data into a different format using babel.
     *
     * \param babel path to the babel executable
     * \param featureType type of GPS feature to export
     * \param input input data path
     * \param output output path
     * \param flags optional flags to control how babel command is generated
     *
     * Returns an empty list if the format does not support exports (see capabilities()).
     */
    virtual QStringList exportCommand( const QString &babel,
                                       Qgis::GpsFeatureType featureType,
                                       const QString &input,
                                       const QString &output,
                                       Qgis::BabelCommandFlags flags = Qgis::BabelCommandFlags() ) const;

  protected:

    /**
     * Constructor for QgsAbstractBabelFormat, with the specified \a name.
     */
    explicit QgsAbstractBabelFormat( const QString &name = QString() );

    /**
     * Converts a GPS feature type to the equivalent babel argument.
     */
    static QString featureTypeToArgument( Qgis::GpsFeatureType type );

    //! Name of format
    QString mName;

    Qgis::BabelFormatCapabilities mCapabilities = Qgis::BabelFormatCapabilities();
};


/**
 * \ingroup core
 * \class QgsBabelSimpleImportFormat
 * \brief A babel format capable of converting input files to GPX files.
 *
 * \since QGIS 3.22
*/
class CORE_EXPORT QgsBabelSimpleImportFormat : public QgsAbstractBabelFormat
{
  public:

    /**
     * Constructor for QgsBabelSimpleImportFormat.
     *
     * The \a format argument specifies the babel identifier for the input format.

     * The \a description argument specified a descriptive name for the format.
     *
     * The \a capabilities argument must reflect whether the format is capable of handling
     * waypoints, routes and/or tracks.
     *
     * The optional \a extensions argument can be used to populate a list of known file
     * extensions for the format (e.g. "csv", "txt").
     */
    QgsBabelSimpleImportFormat( const QString &format, const QString &description, Qgis::BabelFormatCapabilities capabilities,
                                const QStringList extensions = QStringList() );

    /**
     * Returns the friendly description for the format.
     */
    QString description() const { return mDescription; }

    /**
     * Returns the list of known extensions for the format, e.g. "csv", "txt".
     */
    QStringList extensions() const { return mExtensions; }

    QStringList importCommand( const QString &babel,
                               Qgis::GpsFeatureType featureType,
                               const QString &input,
                               const QString &output,
                               Qgis::BabelCommandFlags flags = Qgis::BabelCommandFlags() ) const override;
  private:
    QString mDescription;
    QStringList mExtensions;
};

#endif
