/***************************************************************************
 qgssvgbrowserwidget.h

 ---------------------
 begin                : October 2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE


#ifndef QGSSVGBROWSERWIDGET_H
#define QGSSVGBROWSERWIDGET_H

#include <QWidget>

#include "qgis_gui.h"
#include "qgis_sip.h"

#include "ui_qgssvgbrowserwidget.h"

/**
 * \ingroup gui
 * A widget to browse available SVGs
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsSvgBrowserWidget : public QWidget, private Ui::QgsSvgBrowserWidget
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsSvgBrowserWidget( QWidget *parent = nullptr );

    //! Sets the current path of SVG
    void setPath( const QString &path );

    //! Returns the current path
    QString path() const {return mPath;}

    //! Populates the list of SVG locations
    void populateList() SIP_SKIP; // TODO QGIS 4: make private

  signals:
    void pathChanged( const QString &path );

  protected:
    void showEvent( QShowEvent *event ) override;

  private slots:
    void onSelectionChange( const QModelIndex index );

  private:
    void updateSelection();
    void populateIcons( const QModelIndex &idx );

    // store the path to be used to update selection
    QString mPath;
    bool mListPopulated = false;
    bool mSelectionDirty = true;
    int mIconSize = 30;

};

#endif // QGSSVGBROWSERWIDGET_H
