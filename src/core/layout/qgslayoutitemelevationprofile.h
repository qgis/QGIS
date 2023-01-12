/***************************************************************************
                             qgslayoutitemelevationprofile.h
                             -------------------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#ifndef QGSLAYOUTITEMELEVATIONPROFILE_H
#define QGSLAYOUTITEMELEVATIONPROFILE_H

#include "qgis_core.h"
#include "qgslayoutitem.h"

/**
 * \ingroup core
 * \brief A layout item subclass for elevation profile plots.
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsLayoutItemElevationProfile: public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemElevationProfile, with the specified parent \a layout.
     */
    QgsLayoutItemElevationProfile( QgsLayout *layout );

    /**
     * Returns a new elevation profile item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemElevationProfile *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QIcon icon() const override;

  public slots:

    void refresh() override;

  protected:
    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

};

#endif //QGSLAYOUTITEMELEVATIONPROFILE_H
