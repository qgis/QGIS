/***************************************************************************
    qgssymbolbuffersettingswidget.h
    ---------------------
    begin                : July 2024
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

#ifndef QGSSYMBOLBUFFERSETTINGSWIDGET_H
#define QGSSYMBOLBUFFERSETTINGSWIDGET_H

#include "ui_qgssymbolbuffersettingswidgetbase.h"

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"

#include <QDialog>

class QgsSymbolBufferSettings;


/**
 * \ingroup gui
 * \brief A widget for customising buffer settings for a symbol.
 * \since QGIS 3.40
*/
class GUI_EXPORT QgsSymbolBufferSettingsWidget : public QgsPanelWidget, private Ui::QgsSymbolBufferSettingsWidgetBase
{
    Q_OBJECT
  public:
    //! Constructor for QgsSymbolBufferSettingsWidget
    QgsSymbolBufferSettingsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the buffer \a settings to show in the widget.
     *
     * \see bufferSettings()
     */
    void setBufferSettings( const QgsSymbolBufferSettings &settings );

    /**
     * Returns the buffer settings as defined in the widget.
     *
     * \see setBufferSettings()
     */
    QgsSymbolBufferSettings bufferSettings() const;

  private slots:

    void onWidgetChanged();

  private:
    bool mBlockUpdates = false;
};

/**
 * \ingroup gui
 * \brief A dialog for customising buffer settings for a symbol.
 * \since QGIS 3.40
*/
class GUI_EXPORT QgsSymbolBufferSettingsDialog : public QDialog
{
    Q_OBJECT
  public:
    //! Constructor for QgsSymbolBufferSettingsDialog
    QgsSymbolBufferSettingsDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /**
     * Sets the buffer \a settings to show in the dialog.
     *
     * \see bufferSettings()
     */
    void setBufferSettings( const QgsSymbolBufferSettings &settings );

    /**
     * Returns the buffer settings as defined in the dialog.
     *
     * \see setBufferSettings()
     */
    QgsSymbolBufferSettings bufferSettings() const;

  private:
    QgsSymbolBufferSettingsWidget *mWidget = nullptr;
};

#endif // QGSSYMBOLBUFFERSETTINGSWIDGET_H
