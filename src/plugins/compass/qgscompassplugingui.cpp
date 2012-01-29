/***************************************************************************
 *   Copyright (C) 2012 by Marco Bernasocchi                               *
 *   marco at bernawebdesign.ch                                            *
 *                                                                         *
 *   GUI for showing a QtSensors compass reading                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "qgscompassplugingui.h"

#include "qgisinterface.h"
#include "qgscontexthelp.h"
#include "qgslogger.h"
#include <QDebug>

QgsCompassPluginGui::QgsCompassPluginGui( const QString &title , QWidget * parent, Qt::WFlags fl )
    : QDockWidget( title, parent, fl )
{
  setupUi( this );
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(updateReading()));
  mTimer->start(1000);
}

QgsCompassPluginGui::~QgsCompassPluginGui()
{
  mTimer->stop();
}

void QgsCompassPluginGui::updateReading()
{
  qDebug() << mCompass.azimuth();
  this->mAzimutDisplay->setText( QString::number( mCompass.azimuth() ) );
  this->mCalibrationDisplay->setText( QString::number( mCompass.calibrationLevel() ) );
}
