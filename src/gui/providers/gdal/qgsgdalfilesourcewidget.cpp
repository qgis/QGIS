/***************************************************************************
    qgsgdalfilesourcewidget.cpp
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalfilesourcewidget.h"
#include "moc_qgsgdalfilesourcewidget.cpp"
///@cond PRIVATE

#include "qgsproviderregistry.h"
#include "qgsgdalguiutils.h"
#include "qgsfilewidget.h"

#include <QHBoxLayout>
#include <gdal.h>

QgsGdalFileSourceWidget::QgsGdalFileSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mFileWidget = new QgsFileWidget();
  mFileWidget->setDialogTitle( tr( "Open GDAL Supported Raster Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileRasterFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  layout->addWidget( mFileWidget );

  setLayout( layout );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsGdalFileSourceWidget::validate );
}

void QgsGdalFileSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );

  mFileWidget->setFilePath( mSourceParts.value( QStringLiteral( "path" ) ).toString() );
  mIsValid = true;
}

QString QgsGdalFileSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;
  parts.insert( QStringLiteral( "path" ), mFileWidget->filePath() );
  return QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts );
}

void QgsGdalFileSourceWidget::validate()
{
  const bool valid = !mFileWidget->filePath().isEmpty();
  if ( valid != mIsValid )
    emit validChanged( valid );
  mIsValid = valid;
}


///@endcond
