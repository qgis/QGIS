/***************************************************************************
    qgsrenderingeoptions.h
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
#ifndef QGSRENDERINGOPTIONS_H
#define QGSRENDERINGOPTIONS_H

#include "ui_qgsrenderingoptionsbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgscodeeditor.h"

class QgsBabelGpsDeviceFormat;

class QgsRenderingOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsRenderingOptionsWidgetBase
{
    Q_OBJECT

  public:

    QgsRenderingOptionsWidget( QWidget *parent );

    void apply() override;
  private:
    bool mBlockStoringChanges = false;
};


class QgsRenderingOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsRenderingOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;


};


#endif // QGSRENDERINGOPTIONS_H
