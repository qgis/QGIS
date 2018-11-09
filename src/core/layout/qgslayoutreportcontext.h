/***************************************************************************
                             qgslayoutreportcontext.h
                             -------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSLAYOUTREPORTCONTEXT_H
#define QGSLAYOUTREPORTCONTEXT_H

#include "qgis_core.h"
#include "qgsfeature.h"
#include "qgslayout.h"

#include <QtGlobal>
#include <QPointer>

/**
 * \ingroup core
 * \class QgsLayoutReportContext
 * \brief Stores information relating to the current reporting context for a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutReportContext : public QObject
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutReportContext.
     */
    QgsLayoutReportContext( QgsLayout *layout SIP_TRANSFERTHIS );

    /**
     * Sets the current \a feature for evaluating the layout. This feature may
     * be used for altering an item's content and appearance for a report
     * or atlas layout.
     *
     * Emits the changed() signal.
     *
     * \see feature()
     */
    void setFeature( const QgsFeature &feature );

    /**
     * Returns the current feature for evaluating the layout. This feature may
     * be used for altering an item's content and appearance for a report
     * or atlas layout.
     * \see currentGeometry()
     * \see setFeature()
     */
    QgsFeature feature() const { return mFeature; }

    /**
     * Returns the current feature() geometry in the given \a crs.
     * If no CRS is specified, the original feature geometry is returned.
     *
     * Reprojection only works if a valid layer is set for layer().
     *
     * \see feature()
     * \see layer()
     */
    QgsGeometry currentGeometry( const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) const;

    /**
     * Returns the vector layer associated with the layout's context.
     * \see setLayer()
     */
    QgsVectorLayer *layer() const;

    /**
     * Sets the vector \a layer associated with the layout's context.
     *
     * Emits the changed() signal.
     *
     * \see layer()
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Sets the list of predefined \a scales to use with the layout. This is used
     * for maps which are set to the predefined atlas scaling mode.
     * \see predefinedScales()
     */
    void setPredefinedScales( const QVector<qreal> &scales );

    /**
     * Returns the current list of predefined scales for use with the layout.
     * \see setPredefinedScales()
     */
    QVector<qreal> predefinedScales() const { return mPredefinedScales; }

  signals:

    /**
     * Emitted when the context's \a layer is changed.
     */
    void layerChanged( QgsVectorLayer *layer );

    /**
     * Emitted certain settings in the context is changed, e.g. by setting a new feature or vector layer
     * for the context.
     */
    void changed();

  private:

    QgsLayout *mLayout = nullptr;

    QgsFeature mFeature;
    QPointer< QgsVectorLayer > mLayer;

    // projected geometry cache
    mutable QMap<long, QgsGeometry> mGeometryCache;

    //list of predefined scales
    QVector<qreal> mPredefinedScales;

    friend class QgsLayoutExporter;
    friend class TestQgsLayout;

};

#endif //QGSLAYOUTREPORTCONTEXT_H



