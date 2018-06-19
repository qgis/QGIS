/***************************************************************************
    qgspresetcolorrampdialog.h
    ---------------------
    begin                : September 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPRESETCOLORRAMPDIALOG_H
#define QGSPRESETCOLORRAMPDIALOG_H

#include <QDialog>
#include "qgis.h"
#include "qgspanelwidget.h"
#include "qgscolorramp.h"
#include "ui_qgspresetcolorrampwidgetbase.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsPresetColorRampWidget
 * A widget which allows users to modify the properties of a QgsPresetSchemeColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsPresetColorRampWidget : public QgsPanelWidget, private Ui::QgsPresetColorRampWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsPresetSchemeColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsPresetColorRampWidget.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsPresetColorRampWidget( const QgsPresetSchemeColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsPresetSchemeColorRamp ramp() const;

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsPresetSchemeColorRamp &ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();

  private slots:
    void setColors();

    void mButtonAddColor_clicked();

    void newColorChanged( const QColor &color );
    void schemeChanged();

  private:

    void updatePreview();
    QgsPresetSchemeColorRamp mRamp;
};

/**
 * \ingroup gui
 * \class QgsPresetColorRampDialog
 * A dialog which allows users to modify the properties of a QgsPresetSchemeColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsPresetColorRampDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QgsPresetSchemeColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsPresetColorRampDialog.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsPresetColorRampDialog( const QgsPresetSchemeColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsPresetSchemeColorRamp ramp() const { return mWidget->ramp(); }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsPresetSchemeColorRamp &ramp ) { mWidget->setRamp( ramp ); }

  signals:

    //! Emitted when the dialog settings change
    void changed();

  private:

    QgsPresetColorRampWidget *mWidget = nullptr;

  private slots:

    void showHelp();

};

#endif //QGSPRESETCOLORRAMPDIALOG_H
