/***************************************************************************
                         qgsdecorationoverlay.h
                         ----------------------
    begin                : July 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDECORATIONOVERLAY_H
#define QGSDECORATIONOVERLAY_H


#include <QWidget>

class QgsDecorationOverlay : public QWidget
{
    Q_OBJECT
  public:
    QgsDecorationOverlay( QWidget *parent );
    void paintEvent( QPaintEvent *e ) override;
    bool eventFilter( QObject *obj, QEvent *event ) override;
};


#endif
