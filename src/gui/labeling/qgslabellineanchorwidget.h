/***************************************************************************
    qgslabellineanchorwidget.h
    ----------------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELLINEANCHORWIDGET_H
#define QGSLABELLINEANCHORWIDGET_H

#include "ui_qgslabellineanchorwidgetbase.h"
#include "qgslabelsettingswidgetbase.h"
#include "qgspallabeling.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 * \class QgsLabelLineAnchorWidget
 * \brief A widget for customising label line anchor settings.
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsLabelLineAnchorWidget : public QgsLabelSettingsWidgetBase, private Ui::QgsLabelLineAnchorWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLabelLineAnchorWidget.
     * \param parent parent widget
     * \param vl associated vector layer
     */
    QgsLabelLineAnchorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsVectorLayer *vl = nullptr );

    /**
     * Sets the line \a settings to show in the widget.
     *
     * \see settings()
     */
    void setSettings( const QgsLabelLineSettings &settings );

    /**
     * Returns the line settings defined by the widget.
     *
     * \see setSettings()
     */
    QgsLabelLineSettings settings() const;

    void updateDataDefinedProperties( QgsPropertyCollection &properties ) override;

  private:

    bool mBlockSignals = false;

    void updateAnchorTypeHint();
    void updateAnchorTextPointHint();
};

#endif // QGSLABELLINEANCHORWIDGET_H
