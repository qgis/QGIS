/***************************************************************************
    qgs3dadvancedsymbolsettingswidget.h
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

#ifndef QGS3DADVANCEDSYMBOLSETTINGSWIDGET_H
#define QGS3DADVANCEDSYMBOLSETTINGSWIDGET_H

#include "qgsabstract3dsymbol.h"
#include "qgspanelwidget.h"

#include <QDialog>

class QgsVectorLayer;

/**
 * \class Qgs3DAdvancedSymbolSettingsWidget
 * \brief Base class for a widget which allows the user to advanced settings for 3d symbols.
 * \since QGIS 4.4
 */
class Qgs3DAdvancedSymbolSettingsWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:
    Qgs3DAdvancedSymbolSettingsWidget( const QgsAbstract3DSymbol *symbol, QWidget *parent = nullptr );

    virtual std::unique_ptr<QgsAbstract3DSymbol> symbol() const;

  protected:
    std::unique_ptr<QgsAbstract3DSymbol> mBaseSymbol;
};

/**
 * \brief A dialog for customising animation settings for a symbol.
 * \since QGIS 4.4
*/
class Qgs3DAdvancedSymbolSettingsDialog : public QDialog
{
    Q_OBJECT
  public:
    //! Constructor for Qgs3DAdvancedSymbolSettingsDialog
    Qgs3DAdvancedSymbolSettingsDialog(
      const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags()
    );

    std::unique_ptr<QgsAbstract3DSymbol> symbol() const;

  private:
    Qgs3DAdvancedSymbolSettingsWidget *mWidget = nullptr;
};

#endif // QGS3DADVANCEDSYMBOLSETTINGSWIDGET_H
