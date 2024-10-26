/***************************************************************************
    qgsogrfilesourcewidget.cpp
     --------------------------------------
    Date                 : January 2024
    Copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsogrfilesourcewidget.h"
#include "moc_qgsogrfilesourcewidget.cpp"
///@cond PRIVATE

#include "qgsproviderregistry.h"
#include "qgsgdalguiutils.h"
#include "qgsfilewidget.h"

#include <QHBoxLayout>
#include <gdal.h>

QgsOgrFileSourceWidget::QgsOgrFileSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mFileWidget = new QgsFileWidget();
  mFileWidget->setDialogTitle( tr( "Open OGR Supported Vector Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileVectorFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  layout->addWidget( mFileWidget );

  setLayout( layout );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsOgrFileSourceWidget::validate );
}

void QgsOgrFileSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), uri );

  mFileWidget->setFilePath( mSourceParts.value( QStringLiteral( "path" ) ).toString() );
  mIsValid = true;
}

QString QgsOgrFileSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;
  parts.insert( QStringLiteral( "path" ), mFileWidget->filePath() );
  return QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts );
}

void QgsOgrFileSourceWidget::validate()
{
  const bool valid = !mFileWidget->filePath().isEmpty();
  if ( valid != mIsValid )
    emit validChanged( valid );
  mIsValid = valid;
}


///@endcond
