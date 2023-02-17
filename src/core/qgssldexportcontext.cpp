/***************************************************************************
  qgssldexportcontext.cpp - QgsSldExportContext

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
#include "qgssldexportcontext.h"

QgsSldExportContext::QgsSldExportContext( const Qgis::SldExportOptions &options, const Qgis::SldExportVendorExtension &vendorExtension, const QString &filePath )
  : mExportOptions( options )
  , mVendorExtensions( vendorExtension )
  , mExportFilePath( filePath )
{

}

Qgis::SldExportOptions QgsSldExportContext::exportOptions() const
{
  return mExportOptions;
}

void QgsSldExportContext::setExportOptions( const Qgis::SldExportOptions &exportOptions )
{
  mExportOptions = exportOptions;
}

Qgis::SldExportVendorExtension QgsSldExportContext::vendorExtensions() const
{
  return mVendorExtensions;
}

void QgsSldExportContext::setVendorExtension( const Qgis::SldExportVendorExtension &vendorExtension )
{
  mVendorExtensions = vendorExtension;
}

QString QgsSldExportContext::exportFilePath() const
{
  return mExportFilePath;
}

void QgsSldExportContext::setExportFilePath( const QString &exportFilePath )
{
  mExportFilePath = exportFilePath;
}
