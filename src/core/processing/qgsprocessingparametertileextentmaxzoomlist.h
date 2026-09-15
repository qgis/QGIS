/***************************************************************************
  qgsprocessingparametertileextentmaxzoomlist.h
  ---------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERTILEEXTENTMAXZOOMLIST_H
#define QGSPROCESSINGPARAMETERTILEEXTENTMAXZOOMLIST_H

#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingparametertype.h"
#include "qgsreferencedgeometry.h"

#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup core
 * \brief Represents a spatial region paired with a custom maximum zoom level.
 *
 * \since QGIS 4.4
 */
struct CORE_EXPORT QgsTileExtentMaxZoomRegion
{
    //! Bounding box extent and associated CRS
    QgsReferencedRectangle extent;

    //! Maximum zoom level applied within this extent
    int maxZoom = 0;
};

/**
 * \ingroup core
 * \brief A Processing algorithm parameter for defining a list of spatial extents,
 * each with an associated maximum zoom level override.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingParameterTileExtentMaxZoomList : public QgsProcessingParameterDefinition
{
  public:
    /**
     * Constructor for QgsProcessingParameterTileExtentMaxZoomList.
     */
    explicit QgsProcessingParameterTileExtentMaxZoomList( const QString &name, const QString &description = QString(), const QVariant &defaultValue = QVariant(), bool optional = false );

    QgsProcessingParameterTileExtentMaxZoomList *clone() const override SIP_FACTORY;
    QString type() const override;
    bool checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context = nullptr ) const override;
    QString valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const override;

    QString valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok SIP_OUT ) const override;
    QVariant valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const override;

    //! Returns the type name for the parameter class.
    static QString typeName() { return u"tileextentmaxzoomlist"_s; } // cppcheck-suppress duplInheritedMember

    /**
     * Converts a parameter \a variant value to a list of QgsTileExtentMaxZoomRegion objects.
     */
    QList<QgsTileExtentMaxZoomRegion> parameterAsRegionList( const QVariant &variant, QgsProcessingContext &context ) const;

    /**
     * Converts a list of \a regions to variant value.
     */
    static QVariant toVariant( const QList<QgsTileExtentMaxZoomRegion> &regions );

    /**
     * Converts a variant to a region value.
     */
    QgsTileExtentMaxZoomRegion regionFromVariant( const QVariant &variant, QgsProcessingContext &context ) const;

    /**
     * Evaluates the effective maximum zoom level for a given \a tileExtent.
     *
     * If the tile intersects one or more regions, the LARGEST maxZoom among all intersecting
     * regions is returned. If no region covers the tile, \a defaultMaxZoom is returned.
     *
     * \warning This method requires that the CRS for \a tileExtent and ALL \a regions is equal. The caller
     * must take care to transform all \a regions to a matching CRS in advance.
     */
    static int maxZoomForTile( const QgsRectangle &tileExtent, const QList<QgsTileExtentMaxZoomRegion> &regions, int defaultMaxZoom );

  private:
    static QVariantMap regionToVariantMap( const QgsTileExtentMaxZoomRegion &region );

    QgsTileExtentMaxZoomRegion regionFromVariantMap( const QVariantMap &map, QgsProcessingContext &context ) const;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \brief Parameter type definition for QgsProcessingParameterTileExtentMaxZoomList.
 *
 * \ingroup core
 * \note This class is not a part of public API.
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingParameterTypeTileExtentMaxZoomList : public QgsProcessingParameterType
{
  public:
    QgsProcessingParameterDefinition *create( const QString &name ) const override SIP_FACTORY { return new QgsProcessingParameterTileExtentMaxZoomList( name ); }

    QString description() const override
    {
      return QCoreApplication::translate( "Processing", "An input for specifying defining a list of spatial extents, each with an associated maximum zoom level override." );
    }
    QString name() const override { return QCoreApplication::translate( "Processing", "Tile Extent Maximum Zoom List" ); }
    QString id() const override { return QgsProcessingParameterTileExtentMaxZoomList::typeName(); }
    QString pythonImportString() const override { return u"from qgis.core import QgsProcessingParameterTileExtentMaxZoomList"_s; }
    QString className() const override { return u"QgsProcessingParameterTileExtentMaxZoomList"_s; }
    QStringList acceptedParameterTypes() const override { return { QgsProcessingParameterTileExtentMaxZoomList::typeName() }; }
    QStringList acceptedOutputTypes() const override { return {}; }
};

///@endcond
#endif

#endif // QGSPROCESSINGPARAMETERTILEEXTENTMAXZOOMLIST_H
