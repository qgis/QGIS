/***************************************************************************
  qgsanimatedicon.cpp - QgsAnimatedIcon

 ---------------------
 begin                : 13.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsanimatedicon.h"
#include "moc_qgsanimatedicon.cpp"

QgsAnimatedIcon::QgsAnimatedIcon( const QString &iconPath, QObject *parent )
  : QObject( parent )
  , mMovie( new QMovie( this ) )
{
  if ( !iconPath.isEmpty() )
  {
    mMovie->setFileName( iconPath );
  }
  mMovie->setCacheMode( QMovie::CacheAll );
  connect( mMovie, &QMovie::frameChanged, this, &QgsAnimatedIcon::onFrameChanged );
}

QString QgsAnimatedIcon::iconPath() const
{
  return mMovie->fileName();
}

void QgsAnimatedIcon::setIconPath( const QString &iconPath )
{
  mMovie->setFileName( iconPath );
}

QIcon QgsAnimatedIcon::icon() const
{
  return mIcon;
}

bool QgsAnimatedIcon::connectFrameChanged( const QObject *receiver, const char *method )
{
  if ( connect( this, SIGNAL( frameChanged() ), receiver, method ) )
  {
    mMovie->setPaused( false );
    return true;
  }
  else
    return false;
}

bool QgsAnimatedIcon::disconnectFrameChanged( const QObject *receiver, const char *method )
{
  return disconnect( this, SIGNAL( frameChanged() ), receiver, method );
}

int QgsAnimatedIcon::width() const
{
  return mMovie->currentPixmap().width();
}

int QgsAnimatedIcon::height() const
{
  return mMovie->currentPixmap().height();
}
void QgsAnimatedIcon::onFrameChanged()
{
  if ( !isSignalConnected( QMetaMethod::fromSignal( &QgsAnimatedIcon::frameChanged ) ) )
    mMovie->setPaused( true );

  mIcon = QIcon( mMovie->currentPixmap() );
  emit frameChanged();
}
