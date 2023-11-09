/***************************************************************************
    qgssymbolanimationsettingswidget.h
    ---------------------
    begin                : April 2022
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

#ifndef QGSSYMBOLANIMATIONSETTINGSWIDGET_H
#define QGSSYMBOLANIMATIONSETTINGSWIDGET_H

#include "ui_qgssymbolanimationsettingswidgetbase.h"

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"

#include <QDialog>

class QgsSymbolAnimationSettings;


/**
 * \ingroup gui
 * \brief A widget for customising animation settings for a symbol.
 * \since QGIS 3.26
*/
class GUI_EXPORT QgsSymbolAnimationSettingsWidget: public QgsPanelWidget, private Ui::QgsSymbolAnimationSettingsWidgetBase
{
    Q_OBJECT
  public:

    //! Constructor for QgsSymbolAnimationSettingsWidget
    QgsSymbolAnimationSettingsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the animation \a settings to show in the widget.
     *
     * \see animationSettings()
     */
    void setAnimationSettings( const QgsSymbolAnimationSettings &settings );

    /**
     * Returns the animation settings as defined in the widget.
     *
     * \see setAnimationSettings()
     */
    QgsSymbolAnimationSettings animationSettings() const;

  private:

    bool mBlockUpdates = false;

};

/**
 * \ingroup gui
 * \brief A dialog for customising animation settings for a symbol.
 * \since QGIS 3.26
*/
class GUI_EXPORT QgsSymbolAnimationSettingsDialog : public QDialog
{
    Q_OBJECT
  public:

    //! Constructor for QgsSymbolAnimationSettingsDialog
    QgsSymbolAnimationSettingsDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /**
     * Sets the animation \a settings to show in the dialog.
     *
     * \see animationSettings()
     */
    void setAnimationSettings( const QgsSymbolAnimationSettings &settings );

    /**
     * Returns the animation settings as defined in the dialog.
     *
     * \see setAnimationSettings()
     */
    QgsSymbolAnimationSettings animationSettings() const;

  private:

    QgsSymbolAnimationSettingsWidget *mWidget = nullptr;

};

#endif // QGSSYMBOLANIMATIONSETTINGSWIDGET_H
