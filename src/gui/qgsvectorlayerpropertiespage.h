/***************************************************************************
    qgsvectorlayerpropertiespage.h
     --------------------------------------
    Date                 : 8.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERPROPERTIESPAGE_H
#define QGSVECTORLAYERPROPERTIESPAGE_H

#include <QWidget>

class QgsVectorLayer;

class GUI_EXPORT QgsVectorLayerPropertiesPage : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsVectorLayerPropertiesPage( QWidget *parent = 0 );

  signals:

  public slots:
    virtual void apply() = 0;
};

#endif // QGSVECTORLAYERPROPERTIESPAGE_H
