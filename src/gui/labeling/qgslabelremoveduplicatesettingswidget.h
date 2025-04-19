/***************************************************************************
    qgslabelremoveduplicatessettingswidget.h
    ----------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELREMOVEDUPLICATESSETTINGSWIDGET_H
#define QGSLABELREMOVEDUPLICATESSETTINGSWIDGET_H

#include "ui_qgslabelremoveduplicatesettingswidgetbase.h"
#include "qgslabelsettingswidgetbase.h"
#include "qgspallabeling.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 * \class QgsLabelRemoveDuplicatesSettingsWidget
 * A widget for customising label duplicate removal settings.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLabelRemoveDuplicatesSettingsWidget : public QgsLabelSettingsWidgetBase, private Ui::QgsLabelRemoveDuplicateSettingsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelRemoveDuplicatesSettingsWidget.
     * \param parent parent widget
     * \param layer associated layer
     */
    QgsLabelRemoveDuplicatesSettingsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsMapLayer *layer = nullptr );

    /**
     * Sets the thinning \a settings to show in the widget.
     *
     * \see settings()
     */
    void setSettings( const QgsLabelThinningSettings &settings );

    /**
     * Returns the thinning settings defined by the widget.
     *
     * \see setSettings()
     */
    QgsLabelThinningSettings settings() const;

    void setGeometryType( Qgis::GeometryType type ) override;

    void updateDataDefinedProperties( QgsPropertyCollection &properties ) override;

  private:
    bool mBlockSignals = false;
    mutable QgsLabelThinningSettings mSettings;
};

#endif // QGSLABELREMOVEDUPLICATESSETTINGSWIDGET_H
