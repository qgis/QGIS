/***************************************************************************
    qgsuserprofileoptions.h
    ---------------
    begin                : February 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSUSERPROFILEOPTIONS_H
#define QGSUSERPROFILEOPTIONS_H

#include "ui_qgsuserprofileoptionswidgetbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgis_app.h"

/**
 * \ingroup app
 * \class QgsUserProfileOptionsWidget
 * \brief An options widget showing USERPROFILE settings.
 *
 * \since QGIS 3.32
 */
class APP_EXPORT QgsUserProfileOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsUserProfileOptionsWidgetBase
{
    Q_OBJECT

  public:
    //! Constructor for QgsUserProfileOptionsWidget with the specified \a parent widget.
    QgsUserProfileOptionsWidget( QWidget *parent );
    QString helpKey() const override;
    void apply() override;

  private slots:

    //! Change the default profile icon
    void onChangeIconClicked();

    //! Reset the profile icon to default
    void onResetIconClicked();

    // Show or hide user selector dialog icon setting
    void onAskUserChanged();
};


class QgsUserProfileOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:
    QgsUserProfileOptionsFactory();

    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;
};


#endif // QGSUSERPROFILEOPTIONS_H
