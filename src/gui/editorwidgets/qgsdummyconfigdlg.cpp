/***************************************************************************
    qgsdummyconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsdummyconfigdlg.h"

QgsDummyConfigDlg::QgsDummyConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent, const QString &description )
  :    QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  mDummyTextLabel->setText( description );
}


QVariantMap QgsDummyConfigDlg::config()
{
  return QVariantMap();
}

void QgsDummyConfigDlg::setConfig( const QVariantMap &config )
{
  Q_UNUSED( config )
}
