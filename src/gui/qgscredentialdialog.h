/***************************************************************************
                          qgscredentialdialog.h  -  description
                             -------------------
    begin                : February 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSCREDENTIALDIALOG_H
#define QGSCREDENTIALDIALOG_H

#include <ui_qgscredentialdialog.h>
#include <qgisgui.h>
#include "qgscredentials.h"

#include <QString>


/** \ingroup gui
 * A generic dialog for requesting credentials
 */
class GUI_EXPORT QgsCredentialDialog : public QDialog, public QgsCredentials, private Ui_QgsCredentialDialog
{
    Q_OBJECT
  public:
    QgsCredentialDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsCredentialDialog();

  protected:
    virtual bool request( QString realm, QString &username, QString &password, QString message = QString::null );
};

#endif
