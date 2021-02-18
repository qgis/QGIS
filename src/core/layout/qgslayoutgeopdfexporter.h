/***************************************************************************
                              qgslayoutgeopdfexporter.h
                             --------------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
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
#ifndef QGSLAYOUTGEOPDFEXPORTER_H
#define QGSLAYOUTGEOPDFEXPORTER_H

#include "qgis_core.h"
#include "qgsabstractgeopdfexporter.h"
#include "qgslayoutitemmap.h"
#include <QList>
#include <QTemporaryDir>

#define SIP_NO_FILE

class QgsLayout;
class QgsGeoPdfRenderedFeatureHandler;

/**
 * \class QgsLayoutGeoPdfExporter
 * \ingroup core
 *
 * \brief Handles GeoPDF export specific setup, cleanup and processing steps.
 *
 * This class is a low level implementation detail only. Generally, you should use the high level interface exposed by
 * QgsLayoutExporter instead.
 *
 * \warning QgsLayoutGeoPdfExporter is designed to be a short lived object. It should be created for a
 * single layout export operation only, and then immediately destroyed. Failure to correctly
 * destroy the object after exporting a layout will leave the layout in an inconsistent, unstable state.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsLayoutGeoPdfExporter : public QgsAbstractGeoPdfExporter
{
  public:

    /**
     * Constructor for QgsLayoutGeoPdfExporter, associated with the specified \a layout.
     */
    QgsLayoutGeoPdfExporter( QgsLayout *layout );

    ~QgsLayoutGeoPdfExporter() override;

    /**
     * Returns any custom layer tree groups defined in the layer's settings.
     */
    QMap< QString, QString > customLayerTreeGroups() const { return mCustomLayerTreeGroups; }

    /**
     * Optional map of map layer ID to initial visibility state. If a layer ID is not present in this,
     * it will default to being initially visible when opening the PDF.
     *
     * \since QGIS 3.14
     */
    QMap< QString, bool > initialLayerVisibility() const { return mInitialLayerVisibility; }

    /**
     * Optional list of map layer IDs in the order they should be shown in the generated GeoPDF layer tree.
     * Layer IDs earlier in the list will appear higher in the GeoPDF layer tree.
     *
     * \since QGIS 3.14
     */
    QStringList layerOrder() const { return mLayerOrder; }

  private:

    VectorComponentDetail componentDetailForLayerId( const QString &layerId ) override;

    QgsLayout *mLayout = nullptr;
    QHash< QgsLayoutItemMap *, QgsGeoPdfRenderedFeatureHandler * > mMapHandlers;

    QMap< QString, bool > mInitialLayerVisibility;
    QMap< QString, QString > mCustomLayerTreeGroups;
    QStringList mLayerOrder;

    friend class TestQgsLayoutGeoPdfExport;
};

#endif //QGSLAYOUTGEOPDFEXPORTER_H



