/***************************************************************************
    qgsmbtilesvectortilesourcewidget.cpp
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

#include "qgsmbtilesvectortilesourcewidget.h"
#include "moc_qgsmbtilesvectortilesourcewidget.cpp"
///@cond PRIVATE

#include "qgsproviderregistry.h"
#include "qgsfilewidget.h"
#include "qgsmbtilesvectortiledataprovider.h"

#include <QHBoxLayout>


QgsMbtilesVectorTileSourceWidget::QgsMbtilesVectorTileSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mFileWidget = new QgsFileWidget();
  mFileWidget->setDialogTitle( tr( "Select Mbtiles Dataset" ) );
  mFileWidget->setFilter( tr( "Mbtiles Files" ) + QStringLiteral( " (*.mbtiles *.MBTILES)" ) );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  layout->addWidget( mFileWidget );

  setLayout( layout );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsMbtilesVectorTileSourceWidget::validate );
}

void QgsMbtilesVectorTileSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri(
    QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY,
    uri
  );

  mFileWidget->setFilePath( mSourceParts.value( QStringLiteral( "path" ) ).toString() );
  mIsValid = true;
}

QString QgsMbtilesVectorTileSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;
  parts.insert( QStringLiteral( "path" ), mFileWidget->filePath() );
  return QgsProviderRegistry::instance()->encodeUri(
    QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY,
    parts
  );
}

void QgsMbtilesVectorTileSourceWidget::validate()
{
  const bool valid = !mFileWidget->filePath().isEmpty();
  if ( valid != mIsValid )
    emit validChanged( valid );
  mIsValid = valid;
}


///@endcond
