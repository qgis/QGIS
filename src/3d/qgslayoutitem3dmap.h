/***************************************************************************
  qgslayoutitem3dmap.h
  --------------------------------------
  Date                 : August 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEM3DMAP_H
#define QGSLAYOUTITEM3DMAP_H

#include "qgis_3d.h"

#include "qgslayoutitem.h"

#include "qgscamerapose.h"


class Qgs3DMapSettings;

/**
 * \ingroup 3d
 *
 * Implements support of 3D map views in print layouts
 *
 * \since QGIS 3.4
 */
class _3D_EXPORT QgsLayoutItem3DMap : public QgsLayoutItem
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    // the conversions have to be static, because they're using multiple inheritance
    // (seen in PyQt4 .sip files for some QGraphicsItem classes)
    switch ( sipCpp->type() )
    {
      // really, these *should* use the constants from QgsLayoutItemRegistry, but sip doesn't like that!
      case QGraphicsItem::UserType + 115:
        sipType = sipType_QgsLayoutItem3DMap;
        *sipCppRet = static_cast<QgsLayoutItem3DMap *>( sipCpp );
        break;
      default:
        sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsLayoutItem3DMap, with the specified parent \a layout.
     *
     * Ownership is transferred to the layout.
     */
    QgsLayoutItem3DMap( QgsLayout *layout SIP_TRANSFERTHIS );

    /**
     * Returns a new 3D map item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItem3DMap *create( QgsLayout *layout ) SIP_FACTORY;

    virtual int type() const override;

    virtual void draw( QgsLayoutItemRenderContext &context ) override;

    //! Configures camera view
    void setCameraPose( const QgsCameraPose &pose ) { mCameraPose = pose; }
    //! Returns camera view
    QgsCameraPose cameraPose() const { return mCameraPose; }

    /**
     * Configures map scene
     *
     * Ownership is transferred to the item.
     */
    void setMapSettings( Qgs3DMapSettings *settings SIP_TRANSFER );
    //! Returns map scene. May be a null pointer if not yet configured.
    Qgs3DMapSettings *mapSettings() const { return mSettings.get(); }

  private:
    std::unique_ptr<Qgs3DMapSettings> mSettings;
    QgsCameraPose mCameraPose;
};

#endif // QGSLAYOUTITEM3DMAP_H
