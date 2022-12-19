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
 * /ingroup core
 * /brief The QgsSldExportContext class holds SLD export options and other information related to SLD export of a QGIS layer style.
 *
 * /since QGIS 3.30
 */
class CORE_EXPORT QgsSldExportContext
{
  public:

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

    Qgis::SldExportOptions exportOptions() const;
    void setExportOptions( const Qgis::SldExportOptions &exportOptions );

    Qgis::SldExportVendorExtension vendorExtensions() const;
    void setVendorExtensions( const Qgis::SldExportVendorExtension &vendorExtensions );

    QString exportFilePath() const;
    void setExportFilePath( const QString &exportFilePath );

  private:

    Qgis::SldExportOptions mExportOptions = Qgis::SldExportOption::NoOptions;
    Qgis::SldExportVendorExtension mVendorExtensions = Qgis::SldExportVendorExtension::NoVendorExtension;
    QString mExportFilePath;

};

Q_DECLARE_METATYPE( QgsSldExportContext );

#endif // QGSSLDEXPORTCONTEXT_H
