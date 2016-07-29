/***************************************************************************
    qgsdummyconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdummyconfigdlg.h"

QgsDummyConfigDlg::QgsDummyConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent, const QString& description )
    :    QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  mDummyTextLabel->setText( description );
}


QgsEditorWidgetConfig QgsDummyConfigDlg::config()
{
  return QgsEditorWidgetConfig();
}

void QgsDummyConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  Q_UNUSED( config );
}
