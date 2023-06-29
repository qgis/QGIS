/***************************************************************************
  qgslayerpropertiesguiutils.h
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERPROPERTIESGUIUTILS_H
#define QGSLAYERPROPERTIESGUIUTILS_H

#define SIP_NO_FILE

#include "qgsgui.h"

#include <QObject>
#include <QPointer>

class QgsMapLayer;
class QgsMetadataWidget;

/**
 * \ingroup gui
 * \class QgsLayerPropertiesGuiUtils
 * \brief Contains common utilities for handling functionality in a layer properties dialog.
 * \note Not available in Python bindings
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsLayerPropertiesGuiUtils : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayerPropertiesGuiUtils.
     *
     * \param parent parent widget (usually the layer properties dialog)
     * \param layer associated map layer
     * \param metadataWidget linked QgsMetadataWidget
     */
    QgsLayerPropertiesGuiUtils( QWidget *parent, QgsMapLayer *layer, QgsMetadataWidget *metadataWidget );

  public slots:

    /**
     * Triggers the option to load layer metadata from a file.
     */
    void loadMetadata();

  private:
    QWidget *mParentWidget = nullptr;

    QPointer< QgsMapLayer> mLayer;

    QgsMetadataWidget *mMetadataWidget = nullptr;

};

#endif // QGSLAYERPROPERTIESGUIUTILS_H
