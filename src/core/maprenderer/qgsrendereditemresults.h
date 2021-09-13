/***************************************************************************
  qgsrendereditemresults.h
  -------------------
   begin                : August 2021
   copyright            : (C) Nyall Dawson
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

#ifndef QGSRENDEREDITEMRESULTS_H
#define QGSRENDEREDITEMRESULTS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"
#include <memory>
#include <QList>
#include <vector>
#include <unordered_map>

class QgsRenderedItemDetails;
class QgsRenderContext;
class QgsRenderedAnnotationItemDetails;

///@cond PRIVATE
class QgsRenderedItemResultsSpatialIndex;
///@endcond

/**
 * \ingroup core
 * \brief Stores collated details of rendered items during a map rendering operation.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsRenderedItemResults
{
  public:

    /**
     * Constructor for QgsRenderedItemResults.
     *
     * The \a extent argument can be used to specify an expected maximal extent for items which
     * will be stored in the results. This helps to optimise the spatial indices used by the object.
     */
    QgsRenderedItemResults( const QgsRectangle &extent = QgsRectangle() );
    ~QgsRenderedItemResults();

    //! QgsRenderedItemResults cannot be copied.
    QgsRenderedItemResults( const QgsRenderedItemResults & ) = delete;
    //! QgsRenderedItemResults cannot be copied.
    QgsRenderedItemResults &operator=( const QgsRenderedItemResults &rh ) = delete;

    /**
     * Returns a list of all rendered items.
     */
    QList< QgsRenderedItemDetails * > renderedItems() const;

    /**
     * Returns a list with details of the rendered annotation items within the specified \a bounds.
     *
     * \since QGIS 3.22
     */
    QList<const QgsRenderedAnnotationItemDetails *> renderedAnnotationItemsInBounds( const QgsRectangle &bounds ) const;

    /**
     * Appends rendered item details to the results object.
     *
     * Ownership of \a results is transferred to the this object.
     *
     * The render \a context argument is used to specify the render context used to render the items. It will be used
     * to transform the details to the destination map CRS.
     */
    void appendResults( const QList< QgsRenderedItemDetails * > &results SIP_TRANSFER, const QgsRenderContext &context );

    /**
     * Transfers all results from an \a other QgsRenderedItemResults object where the items
     * have layer IDs matching the specified list.
     *
     * Items are removed from \a other and transferred to this object.
     *
     * \warning After calling this method the \a other results will be left in an undefined state.
     */
    void transferResults( QgsRenderedItemResults *other, const QStringList &layerIds );

    /**
     * Transfers all results from an \a other QgsRenderedItemResults object to this one.
     *
     * Items are removed from \a other and transferred to this object.
     *
     * \warning After calling this method the \a other results will be left in an undefined state.
     */
    void transferResults( QgsRenderedItemResults *other );

    /**
     * Erases results from layers matching those in the specified list of layers IDs.
     */
    void eraseResultsFromLayers( const QStringList &layerIds );

  private:
#ifdef SIP_RUN
    QgsRenderedItemResults( const QgsRenderedItemResults & );
#endif

    QgsRectangle mExtent;

    std::unordered_map< QString, std::vector< std::unique_ptr< QgsRenderedItemDetails > > > mDetails;
    std::unique_ptr< QgsRenderedItemResultsSpatialIndex > mAnnotationItemsIndex;

};

#endif // QGSRENDEREDITEMRESULTS_H
