/***************************************************************************
    qgs3dadvancedpointsymbolsettingswidget.h
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DADVANCEDPOINTSYMBOLSETTINGSWIDGET_H
#define QGS3DADVANCEDPOINTSYMBOLSETTINGSWIDGET_H

#include "ui_qgs3dadvancedpointsymbolsettingswidgetbase.h"

#include "qgs3dadvancedsymbolsettingswidget.h"

class QgsVectorLayer;

/**
 * \class Qgs3DAdvancedPointSymbolSettingsWidget
 * \brief A widget which allows the user to advanced settings for 3d point symbols.
 * \since QGIS 4.4
 */
class Qgs3DAdvancedPointSymbolSettingsWidget : public Qgs3DAdvancedSymbolSettingsWidget, private Ui::Qgs3DAdvancedPointSymbolSettingsWidgetBase
{
    Q_OBJECT

  public:
    Qgs3DAdvancedPointSymbolSettingsWidget( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent = nullptr );

    std::unique_ptr<QgsAbstract3DSymbol> symbol() const final;
};

#endif // QGS3DADVANCEDPOINTSYMBOLSETTINGSWIDGET_H
