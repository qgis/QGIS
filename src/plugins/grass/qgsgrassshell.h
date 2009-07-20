/***************************************************************************
     qgsgrassshell.h
     --------------------------------------
    Date                 : Thu Apr 23 08:35:43 CEST 2009
    Copyright            : (C) 2009 by Lorenzo "Il Rugginoso" Masini
    Email                : lorenxo86@gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QFrame>

class QgsGrassTools;
class QTabWidget;
class QTermWidget;

class QgsGrassShell : public QFrame
{
    Q_OBJECT

  public:
    QgsGrassShell( QgsGrassTools *tools, QTabWidget *parent = 0, const char *name = 0 );
    virtual ~QgsGrassShell();

  private slots:
    void closeShell();

  private:
    void initTerminal( QTermWidget *terminal );

    QTermWidget *mTerminal;
    QgsGrassTools *mTools;
    QTabWidget *mTabWidget;
    QString mLockFilename;
};
