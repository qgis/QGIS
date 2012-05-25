/***************************************************************************
    qgspenstylecombobox.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPENSTYLECOMBOBOX_H
#define QGSPENSTYLECOMBOBOX_H

#include <QComboBox>

class GUI_EXPORT QgsPenStyleComboBox : public QComboBox
{
    Q_OBJECT

  public:
    QgsPenStyleComboBox( QWidget* parent = NULL );

    Qt::PenStyle penStyle() const;

    void setPenStyle( Qt::PenStyle style );

  protected:
    QIcon iconForPen( Qt::PenStyle style );

};

class GUI_EXPORT QgsPenJoinStyleComboBox : public QComboBox
{
    Q_OBJECT

  public:
    QgsPenJoinStyleComboBox( QWidget* parent = NULL );

    Qt::PenJoinStyle penJoinStyle() const;

    void setPenJoinStyle( Qt::PenJoinStyle style );
};

class GUI_EXPORT QgsPenCapStyleComboBox : public QComboBox
{
    Q_OBJECT

  public:
    QgsPenCapStyleComboBox( QWidget* parent = NULL );

    Qt::PenCapStyle penCapStyle() const;

    void setPenCapStyle( Qt::PenCapStyle style );
};

#endif
