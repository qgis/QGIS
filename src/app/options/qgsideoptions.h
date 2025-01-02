/***************************************************************************
    qgsideoptions.h
    -------------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSIDEOPTIONS_H
#define QGSIDEOPTIONS_H

#include "ui_qgsideoptionswidgetbase.h"
#include "qgsoptionswidgetfactory.h"

/**
 * \ingroup app
 * \class QgsIdeOptionsWidget
 * \brief An options widget showing IDE settings.
 */
class QgsIdeOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsIdeOptionsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsIdeOptionsWidget with the specified \a parent widget.
     */
    QgsIdeOptionsWidget( QWidget *parent );
    ~QgsIdeOptionsWidget() override;

    QString helpKey() const override;

    void apply() override;

  private slots:

    void generateGitHubToken();
};


class QgsIdeOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:
    QgsIdeOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;
};

#endif // QGSIDEOPTIONS_H
