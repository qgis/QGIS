/***************************************************************************
                          qgsdlgbuffer.cpp
                          Buffer dialog - Subclasses qgsdlgbufferbase
 Part of the Geoprocessing plugin for PostgreSQL/PostGIS layers
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */
#include <QValidator>

#include "qgsdlgpgbuffer.h"
#include "qgscontexthelp.h"
#include "qgisinterface.h"

QgsDlgPgBuffer::QgsDlgPgBuffer( QgisInterface * _qI, QWidget * parent, Qt::WFlags fl )
    : QDialog( parent, fl ), qI( _qI )
{
  setupUi( this );
  // set the validator
  distanceValidator = new QDoubleValidator( 0, 9e9, 6, this );
  txtBufferDistance->setValidator( distanceValidator );
}

QgsDlgPgBuffer::~QgsDlgPgBuffer()
{
}
void QgsDlgPgBuffer::setBufferLabel( QString & lbl )
{
  lblBufferInfo->setText( "<h2>" + lbl + "</h2>" );
}

QString QgsDlgPgBuffer::bufferDistance()
{
  return txtBufferDistance->text();
}

QString QgsDlgPgBuffer::bufferLayerName()
{
  return txtBufferedLayerName->text();
}

bool QgsDlgPgBuffer::addLayerToMap()
{
  return chkAddToMap->isChecked();
}

QString QgsDlgPgBuffer::geometryColumn()
{
  return txtGeometryColumn->text();
}

QString QgsDlgPgBuffer::srid()
{
  return txtSrid->text();
}

QString QgsDlgPgBuffer::objectIdColumn()
{
  return cmbFields->currentText();
}

QString QgsDlgPgBuffer::schema()
{
  return cmbSchema->currentText();
}

void QgsDlgPgBuffer::addFieldItem( QString field )
{
  cmbFields->addItem( field );
}

void QgsDlgPgBuffer::addSchema( QString schema )
{
  cmbSchema->addItem( schema );
}

void QgsDlgPgBuffer::setSrid( QString srid )
{
  txtSrid->setText( srid );
}

void QgsDlgPgBuffer::setBufferLayerName( QString name )
{
  txtBufferedLayerName->setText( name );
}

void QgsDlgPgBuffer::setGeometryColumn( QString name )
{
  txtGeometryColumn->setText( name );
}
void QgsDlgPgBuffer::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
// Old call to open help in browser  qI->openURL("plugins/geoprocessing/buffer/index.html",true);
}
