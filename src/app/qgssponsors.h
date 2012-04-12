/***************************************************************************
                          qgssponsors.h  -  description
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
#ifndef QGSSPONSORS_H
#define QGSSPONSORS_H

#include "ui_qgssponsorsbase.h"

class QgsSponsors : public QDialog, private Ui::QgsSponsorsBase
{
    Q_OBJECT
  public:
    QgsSponsors( QWidget *parent );
    ~QgsSponsors();

  private:
    void init();

  private slots:
};

#endif
