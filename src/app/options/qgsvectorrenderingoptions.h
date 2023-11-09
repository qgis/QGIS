/***************************************************************************
    qgsvectorrenderingeoptions.h
    -------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORRENDERINGOPTIONS_H
#define QGSVECTORRENDERINGOPTIONS_H

#include "ui_qgsvectorrenderingoptionsbase.h"
#include "qgsoptionswidgetfactory.h"

class QgsVectorRenderingOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsVectorRenderingOptionsWidgetBase
{
    Q_OBJECT

  public:

    QgsVectorRenderingOptionsWidget( QWidget *parent );

    void apply() override;
  private:
    bool mBlockStoringChanges = false;
};


class QgsVectorRenderingOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsVectorRenderingOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QStringList path() const override;

};


#endif // QGSVECTORRENDERINGOPTIONS_H
