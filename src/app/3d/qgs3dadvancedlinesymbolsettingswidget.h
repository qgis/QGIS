/***************************************************************************
    qgs3dadvancedlinesymbolsettingswidget.h
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

#ifndef QGS3DADVANCEDLINESYMBOLSETTINGSWIDGET_H
#define QGS3DADVANCEDLINESYMBOLSETTINGSWIDGET_H

#include "ui_qgs3dadvancedlinesymbolsettingswidgetbase.h"

#include "qgs3dadvancedsymbolsettingswidget.h"

class QgsVectorLayer;

/**
 * \class Qgs3DAdvancedLineSymbolSettingsWidget
 * \brief A widget which allows the user to advanced settings for 3d line symbols.
 * \since QGIS 4.4
 */
class Qgs3DAdvancedLineSymbolSettingsWidget : public Qgs3DAdvancedSymbolSettingsWidget, private Ui::Qgs3DAdvancedLineSymbolSettingsWidgetBase
{
    Q_OBJECT

  public:
    Qgs3DAdvancedLineSymbolSettingsWidget( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent = nullptr );

    std::unique_ptr<QgsAbstract3DSymbol> symbol() const final;
};

#endif // QGS3DADVANCEDLINESYMBOLSETTINGSWIDGET_H
