/***************************************************************************
                        qgsvectoroverlay.h  -  description
                        ------------------
   begin                : January 2007
   copyright            : (C) 2007 by Marco Hugentobler
   email                : marco dot hugentobler at karto dot baug dot ethz dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTOROVERLAY_H
#define QGSVECTOROVERLAY_H

#include <QString>
#include "qgsvectorlayer.h"

class QgsOverlayObject;
class QgsRect;
class QgsRenderContext;

/**Base class for vector layer overlays (e.g. diagrams, labels, etc.). For each
 * object, the position of the bounding box is stored in a QgsOverlayObject.
 * The vector overlays are drawn on top of all layers.
 * \note This class has been added in version 1.1
 */
class CORE_EXPORT QgsVectorOverlay
{
  public:
    QgsVectorOverlay( QgsVectorLayer* vl );
    virtual ~QgsVectorOverlay();

    /**Create the overlay objects contained in a view extent. Subclasses need
     * to implement this method and assign width/height information to the
     * overlay objects
     */

    virtual void createOverlayObjects( const QgsRenderContext& renderContext ) = 0;

    /**Remove the overlay objects and release their memory*/
    void removeOverlayObjects();

    /**Draw the overlay objects*/
    virtual void drawOverlayObjects( QgsRenderContext& context ) const = 0;

    /**Gives direct access to oberlay objects*/
    QMap<int, QgsOverlayObject*>* overlayObjects() {return &mOverlayObjects;}

    /**Describes the overlay type (e.g. "diagram" or "label")*/
    virtual QString typeName() const = 0;

    /**Set attribute indices necessary to fetch*/
    void setAttributes( const QgsAttributeList& list ) {mAttributes = list;}

    bool displayFlag() const {return mDisplayFlag;}

    /**Display yes/no*/
    void setDisplayFlag( bool flag ) {mDisplayFlag = flag;}

    /**Restore from project file*/
    virtual bool readXML( const QDomNode& overlayNode ) = 0;

    /**Save to project file*/
    virtual bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const = 0;

  protected:
    /**Pointer to the vector layer for this overlay*/
    QgsVectorLayer* mVectorLayer;

    /**True if overlay should be displayed*/
    bool mDisplayFlag;

    /**A list with attribute indexes that are needed for overlay rendering*/
    QgsAttributeList mAttributes;

    /**Key: feature ids, value: the corresponding overlay objects. Normally, they are created for each redraw and deleted before the next redraw*/
    QMap<int, QgsOverlayObject*> mOverlayObjects;

    /**Position constraints that may be set to be persistent after redraws. Key is the feature id, value the map point
        where the feature should be placed*/
    QMap<int, QgsPoint> mPositionConstraints;

  private:
    /**Default constructor forbidden*/
    QgsVectorOverlay();
};

#endif
