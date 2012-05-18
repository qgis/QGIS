/***************************************************************************
    qgsbrushstylecombobox.h
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

#ifndef QGSBRUSHSTYLECOMBOBOX_H
#define QGSBRUSHSTYLECOMBOBOX_H

#include <QComboBox>

class GUI_EXPORT QgsBrushStyleComboBox : public QComboBox
{
    Q_OBJECT

  public:
    QgsBrushStyleComboBox( QWidget* parent = NULL );

    Qt::BrushStyle brushStyle() const;

    void setBrushStyle( Qt::BrushStyle style );

  protected:
    QIcon iconForBrush( Qt::BrushStyle style );

};

#endif
