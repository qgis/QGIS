/***************************************************************************
    qgsvtpkvectortilesourcewidget.cpp
     --------------------------------------
    Date                 : March 2023
    Copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsvtpkvectortilesourcewidget.h"
#include "moc_qgsvtpkvectortilesourcewidget.cpp"
///@cond PRIVATE

#include "qgsproviderregistry.h"
#include "qgsfilewidget.h"
#include "qgsvtpkvectortiledataprovider.h"

#include <QHBoxLayout>


QgsVtpkVectorTileSourceWidget::QgsVtpkVectorTileSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mFileWidget = new QgsFileWidget();
  mFileWidget->setDialogTitle( tr( "Select VTPK Dataset" ) );
  mFileWidget->setFilter( tr( "VTPK Files" ) + QStringLiteral( " (*.vtpk *.VTPK)" ) );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  layout->addWidget( mFileWidget );

  setLayout( layout );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsVtpkVectorTileSourceWidget::validate );
}

void QgsVtpkVectorTileSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri(
    QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY,
    uri
  );

  mFileWidget->setFilePath( mSourceParts.value( QStringLiteral( "path" ) ).toString() );
  mIsValid = true;
}

QString QgsVtpkVectorTileSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;
  parts.insert( QStringLiteral( "path" ), mFileWidget->filePath() );
  return QgsProviderRegistry::instance()->encodeUri(
    QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY,
    parts
  );
}

void QgsVtpkVectorTileSourceWidget::validate()
{
  const bool valid = !mFileWidget->filePath().isEmpty();
  if ( valid != mIsValid )
    emit validChanged( valid );
  mIsValid = valid;
}


///@endcond
