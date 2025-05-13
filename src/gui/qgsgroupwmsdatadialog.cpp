/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsgroupwmsdatadialog.h"
#include "moc_qgsgroupwmsdatadialog.cpp"
#include "qgsmaplayerserverproperties.h"

#include <QRegularExpressionValidator>

QgsGroupWmsDataDialog::QgsGroupWmsDataDialog( QWidget *parent, Qt::WindowFlags fl )
  : QgsGroupWmsDataDialog( QgsMapLayerServerProperties(), parent, fl )
{
}

QgsGroupWmsDataDialog::QgsGroupWmsDataDialog( const QgsMapLayerServerProperties &serverProperties, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mServerProperties( std::make_unique<QgsMapLayerServerProperties>() )
{
  setupUi( this );

  serverProperties.copyTo( mServerProperties.get() );

  mMapLayerServerPropertiesWidget->setHasWfsTitle( false );
  mMapLayerServerPropertiesWidget->setServerProperties( mServerProperties.get() );
}

QString QgsGroupWmsDataDialog::groupShortName() const
{
  mMapLayerServerPropertiesWidget->save();
  return mServerProperties->shortName();
}

void QgsGroupWmsDataDialog::setGroupShortName( const QString &shortName )
{
  mServerProperties->setShortName( shortName );
  mMapLayerServerPropertiesWidget->sync();
}

QString QgsGroupWmsDataDialog::groupTitle() const
{
  mMapLayerServerPropertiesWidget->save();
  return mServerProperties->title();
}

void QgsGroupWmsDataDialog::setGroupTitle( const QString &title )
{
  mServerProperties->setTitle( title );
  mMapLayerServerPropertiesWidget->sync();
}

QString QgsGroupWmsDataDialog::groupAbstract() const
{
  mMapLayerServerPropertiesWidget->save();
  return mServerProperties->abstract();
}

void QgsGroupWmsDataDialog::setGroupAbstract( const QString &abstract )
{
  mServerProperties->setAbstract( abstract );
  mMapLayerServerPropertiesWidget->sync();
}

QgsMapLayerServerProperties *QgsGroupWmsDataDialog::serverProperties()
{
  return mServerProperties.get();
}

const QgsMapLayerServerProperties *QgsGroupWmsDataDialog::serverProperties() const
{
  return mServerProperties.get();
}

void QgsGroupWmsDataDialog::accept()
{
  mMapLayerServerPropertiesWidget->save();
  QDialog::accept();
}

bool QgsGroupWmsDataDialog::hasTimeDimension() const
{
  return mComputeTimeDimension->checkState() == Qt::Checked;
}

void QgsGroupWmsDataDialog::setHasTimeDimension( bool hasTimeDimension )
{
  mComputeTimeDimension->setCheckState( hasTimeDimension ? Qt::Checked : Qt::Unchecked );
}
