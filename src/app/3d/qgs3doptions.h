/***************************************************************************
    qgs3doptions.h
    -------------------------
    begin                : January 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGS3DOPTIONS_H
#define QGS3DOPTIONS_H

#include "ui_qgs3doptionsbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgscodeeditor.h"

/**
 * \ingroup app
 * \class Qgs3DOptionsWidget
 * \brief An options widget showing 3D settings.
 *
 * \since QGIS 3.18
 */
class Qgs3DOptionsWidget : public QgsOptionsPageWidget, private Ui::Qgs3DOptionsBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for Qgs3DOptionsWidget with the specified \a parent widget.
     */
    Qgs3DOptionsWidget( QWidget *parent );
    QString helpKey() const override;
    void apply() override;
};


class Qgs3DOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:
    Qgs3DOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;
};


#endif // QGS3DOPTIONS_H
