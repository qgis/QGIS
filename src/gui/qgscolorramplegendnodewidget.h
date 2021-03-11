/***************************************************************************
                         qgscolorramplegendnodewidget.h
                         -----------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORRAMPLEGENDNODEWIDGET_H
#define QGSCOLORRAMPLEGENDNODEWIDGET_H


#include "qgis_gui.h"
#include "ui_qgscolorramplegendnodewidgetbase.h"

#include "qgscolorramplegendnode.h"
#include <QDialog>

class QDialogButtonBox;

/**
 * \ingroup gui
 * \brief A widget for properties relating to a QgsColorRampLegendNode (QgsColorRampLegendNodeSettings).
 *
 * The current settings are set by a call to setSettings(), and the settings defined by the
 * widget are retrieved by calling settings().
 *
 * When changes are made the to settings by a user the widgetChanged() signal is emitted.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsColorRampLegendNodeWidget: public QgsPanelWidget, private Ui::QgsColorRampLegendNodeWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsColorRampLegendNodeWidget, with the specified \a parent widget.
     */
    QgsColorRampLegendNodeWidget( QWidget *parent = nullptr );

    /**
     * Returns the legend node settings as defined by the widget.
     *
     * \see setSettings()
     */
    QgsColorRampLegendNodeSettings settings() const;

    /**
     * Sets the settings to show in the widget.
     *
     * \see settings()
     */
    void setSettings( const QgsColorRampLegendNodeSettings &settings );

    /**
     * Sets visibility for the "Use Continuous Legend" checkbox to \a visible.
     *
     * This widget is visible and checked by default but in a few cases it does not
     * need to be visible because disabling it would not make sense (for instance
     * when using single band gray renderer).
     */
    void setUseContinuousRampCheckBoxVisibility( bool visible );

  private slots:

    void onChanged();
    void changeNumberFormat();
    void onOrientationChanged();

  private:

    bool mBlockSignals = false;
    QgsColorRampLegendNodeSettings mSettings;

};

/**
 * \ingroup gui
 * \brief A dialog for configuring a QgsColorRampLegendNode (QgsColorRampLegendNodeSettings).
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsColorRampLegendNodeDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsColorRampLegendNodeDialog, initially showing the specified \a settings.
     */
    QgsColorRampLegendNodeDialog( const QgsColorRampLegendNodeSettings &settings, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the legend node settings as defined by the dialog.
     */
    QgsColorRampLegendNodeSettings settings() const;

    /**
     * Returns a reference to the dialog's button box.
     */
    QDialogButtonBox *buttonBox() const;

    /**
     * Sets visibility for the "Use Continuous Legend" checkbox in the legend settings dialog to \a visible.
     *
     * This widget is visible and checked by default but in a few cases it does not
     * need to be visible because disabling it would not make sense (for instance
     * when using single band gray renderer).
     */
    void setUseContinuousRampCheckBoxVisibility( bool visible );

  private:

    QgsColorRampLegendNodeWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;

};


#endif //QGSCOLORRAMPLEGENDNODEWIDGET_H

