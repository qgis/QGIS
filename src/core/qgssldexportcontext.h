/***************************************************************************
  qgssldexportcontext.h - QgsSldExportContext

 ---------------------
 begin                : 21.12.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSLDEXPORTCONTEXT_H
#define QGSSLDEXPORTCONTEXT_H

#include "qgis.h"
#include "qgis_core.h"

/**
 * \ingroup core
 * \brief Holds SLD export options and other information related to SLD export of a QGIS layer style.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSldExportContext
{
  public:

    /**
     * Constructs a default SLD export context
     */
    QgsSldExportContext() = default;

    ~QgsSldExportContext() = default;

    QgsSldExportContext( const QgsSldExportContext &other ) = default;
    QgsSldExportContext &operator=( const QgsSldExportContext &other ) = default;

    /**
     * /brief Create a new QgsSldExportContext
     * /param options SLD export options
     * /param vendorExtension SLD export vendor extension
     * /param filePath SLD export file path
     */
    QgsSldExportContext( const Qgis::SldExportOptions &options, const Qgis::SldExportVendorExtension &vendorExtension, const QString &filePath );

    /**
     * Returns the export options
     */
    Qgis::SldExportOptions exportOptions() const;

    /**
     * Set export options to \a exportOptions
     */
    void setExportOptions( const Qgis::SldExportOptions &exportOptions );

    /**
     * Returns the vendor extension enabled for the SLD export
     */
    Qgis::SldExportVendorExtension vendorExtensions() const;

    /**
     * Sets the vendor extensions to \a vendorExtension
     */
    void setVendorExtension( const Qgis::SldExportVendorExtension &vendorExtension );

    /**
     * Returns the export file path for the SLD
     */
    QString exportFilePath() const;

    /**
     * Sets the export file path for the SLD to \a exportFilePath
     */
    void setExportFilePath( const QString &exportFilePath );

    /**
     * Returns a list of errors which occurred during the conversion.
     *
     * \see pushError()
     * \see warnings()
     * \since QGIS 3.44
     */
    QStringList errors() const { return mErrors; }

    /**
     * Pushes a \a error message generated during the conversion.
     *
     * \see errors()
     * \see pushWarning()
     * \since QGIS 3.44
     */
    void pushError( const QString &error ) { mErrors << error; }

    /**
     * Returns a list of warnings which occurred during the conversion.
     *
     * \see pushWarning()
     * \see errors()
     * \since QGIS 3.44
     */
    QStringList warnings() const { return mWarnings; }

    /**
     * Pushes a \a warning message generated during the conversion.
     *
     * \see warnings()
     * \see pushError()
     * \since QGIS 3.44
     */
    void pushWarning( const QString &warning ) { mWarnings << warning; }

    /**
     * Returns the open ended set of properties that can drive/inform the SLD encoding.
     *
     * \see setExtraProperties()
     * \since QGIS 3.44
     */
    QVariantMap extraProperties() const;

    /**
     * Sets the open ended set of properties that can drive/inform the SLD encoding.
     *
     * \see extraProperties()
     * \since QGIS 3.44
     */
    void setExtraProperties( const QVariantMap &properties );

  private:

    Qgis::SldExportOptions mExportOptions = Qgis::SldExportOption::NoOptions;
    Qgis::SldExportVendorExtension mVendorExtensions = Qgis::SldExportVendorExtension::NoVendorExtension;
    QString mExportFilePath;
    QStringList mErrors;
    QStringList mWarnings;
    QVariantMap mExtraProperties;

};

Q_DECLARE_METATYPE( QgsSldExportContext );

#endif // QGSSLDEXPORTCONTEXT_H
