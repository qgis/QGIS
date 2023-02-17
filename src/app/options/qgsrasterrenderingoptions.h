/***************************************************************************
    qgsrasterrenderingeoptions.h
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
#ifndef QGSRASTERRENDERINGOPTIONS_H
#define QGSRASTERRENDERINGOPTIONS_H

#include "ui_qgsrasterrenderingoptionsbase.h"
#include "qgsoptionswidgetfactory.h"

class QgsRasterRenderingOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsRasterRenderingOptionsWidgetBase
{
    Q_OBJECT

  public:

    QgsRasterRenderingOptionsWidget( QWidget *parent );

    void apply() override;
  private:
    bool mBlockStoringChanges = false;

    void initContrastEnhancement( QComboBox *cbox, const QString &name, const QString &defaultVal );
    void saveContrastEnhancement( QComboBox *cbox, const QString &name );
    void initMinMaxLimits( QComboBox *cbox, const QString &name, const QString &defaultVal );
    void saveMinMaxLimits( QComboBox *cbox, const QString &name );

};


class QgsRasterRenderingOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsRasterRenderingOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QStringList path() const override;

};

#endif // QGSRASTERRENDERINGOPTIONS_H
