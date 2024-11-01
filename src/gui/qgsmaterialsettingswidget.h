/***************************************************************************
  qgsmaterialsettingswidget.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMATERIALSETTINGSWIDGET_H
#define QGSMATERIALSETTINGSWIDGET_H

#include <QWidget>
#include "qgis_gui.h"
#include "qgis_sip.h"

#include "qgspropertycollection.h"

#define SIP_NO_FILE

class QgsAbstractMaterialSettings SIP_EXTERNAL;
enum class QgsMaterialSettingsRenderingTechnique SIP_EXTERNAL;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsMaterialSettingsWidget
 * \brief Base class for 3D material settings widgets.
 * \note Not available in Python bindings
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsMaterialSettingsWidget : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsMaterialSettingsWidget with the specified \a parent widget.
     */
    QgsMaterialSettingsWidget( QWidget *parent );

    /**
     * Sets the material \a settings to show in the widget.
     */
    virtual void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) = 0;

    /**
     * Sets the rendering technique which will be used for the symbol. Allows the widget to adapt
     * available settings for the specified \a technique.
     */
    virtual void setTechnique( QgsMaterialSettingsRenderingTechnique technique );

    /**
     * Returns a new instance of the material settings defined by the widget.
     *
     * Caller takes ownership of the returned settings.
     */
    virtual QgsAbstractMaterialSettings *settings() = 0 SIP_FACTORY;

  signals:

    /**
     * Emitted when the material definition is changed.
     */
    void changed();

  protected:
    QgsPropertyCollection mPropertyCollection;
};

#endif // QGSMATERIALSETTINGSWIDGET_H
