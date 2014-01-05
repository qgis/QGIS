/***************************************************************************
                          qgsabout.h  -  description
                             -------------------
    begin                : Sat Aug 10 2002
    copyright            : (C) 2002 by Gary E.Sherman
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
#ifndef QGSABOUT_H
#define QGSABOUT_H

#include "ui_qgsabout.h"
#include "qgsoptionsdialogbase.h"

class APP_EXPORT QgsAbout : public QgsOptionsDialogBase , private Ui::QgsAbout
{
    Q_OBJECT
  public:
    QgsAbout( QWidget *parent );
    ~QgsAbout();
    void setVersion( QString v );
    static QString fileSystemSafe( QString string );

  private:
    void setWhatsNew( );
    void setLicence();
    void setPluginInfo();
    void init();
    void openUrl( QString url );

  private slots:
    void on_btnQgisUser_clicked();
    void on_btnQgisHome_clicked();
};

#endif
