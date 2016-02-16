/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Licensees holding valid dxflib Professional Edition licenses may use 
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DL_ATTRIBUTES_H
#define DL_ATTRIBUTES_H

#include "dl_global.h"

#include <string>
#include <vector>

#include "dl_codes.h"

/**
 * Storing and passing around attributes. Attributes
 * are the layer name, color, width and line type.
 *
 * @author Andrew Mustun
 */
class DXFLIB_EXPORT DL_Attributes {

public:

    /**
     * Default constructor.
     */
    DL_Attributes() :
        layer(""),
        color(0),
        color24(-1),
        width(0),
        linetype("BYLAYER"),
        linetypeScale(1.0),
        handle(-1),
        inPaperSpace(false) {
    }

    /**
     * Constructor for DXF attributes.
     *
     * @param layer Layer name for this entity or NULL for no layer
     *              (every entity should be on a named layer!).
     * @param color Color number (0..256). 0 = BYBLOCK, 256 = BYLAYER.
     * @param width Line thickness. Defaults to zero. -1 = BYLAYER, 
     *               -2 = BYBLOCK, -3 = default width
     * @param linetype Line type name or "BYLAYER" or "BYBLOCK". Defaults
     *              to "BYLAYER"
     */
    DL_Attributes(const std::string& layer,
                  int color, int width,
                  const std::string& linetype,
                  double linetypeScale) :
        layer(layer),
        color(color),
        color24(-1),
        width(width),
        linetype(linetype),
        linetypeScale(linetypeScale),
        handle(-1),
        inPaperSpace(false) {

    }
    
    /**
     * Constructor for DXF attributes.
     *
     * @param layer Layer name for this entity or NULL for no layer
     *              (every entity should be on a named layer!).
     * @param color Color number (0..256). 0 = BYBLOCK, 256 = BYLAYER.
     * @param color24 24 bit color (see DXF reference).
     * @param width Line thickness. Defaults to zero. -1 = BYLAYER, 
     *               -2 = BYBLOCK, -3 = default width
     * @param linetype Line type name or "BYLAYER" or "BYBLOCK". Defaults
     *              to "BYLAYER"
     */
    DL_Attributes(const std::string& layer,
                  int color, int color24, int width,
                  const std::string& linetype,
                  int handle=-1)  :
        layer(layer),
        color(color),
        color24(color24),
        width(width),
        linetype(linetype),
        linetypeScale(1.0),
        handle(handle),
        inPaperSpace(false) {
    }

    /**
     * Sets the layer. If the given pointer points to NULL, the
     *  new layer name will be an empty but valid string.
     */
    void setLayer(const std::string& layer) {
        this->layer = layer;
    }

    /**
     * @return Layer name.
     */
    std::string getLayer() const {
        return layer;
    }

    /**
     * Sets the color.
     *
     * @see DL_Codes, dxfColors
     */
    void setColor(int color) {
        this->color = color;
    }
    
    /**
     * Sets the 24bit color.
     *
     * @see DL_Codes, dxfColors
     */
    void setColor24(int color) {
        this->color24 = color;
    }

    /**
     * @return Color.
     *
     * @see DL_Codes, dxfColors
     */
    int getColor() const {
        return color;
    } 

    /**
     * @return 24 bit color or -1 if no 24bit color is defined.
     *
     * @see DL_Codes, dxfColors
     */
    int getColor24() const {
        return color24;
    }

    /**
     * Sets the width.
     */
    void setWidth(int width) {
        this->width = width;
    }

    /**
     * @return Width.
     */
    int getWidth() const {
        return width;
    }

    /**
     * Sets the line type. This can be any string and is not
     *  checked to be a valid line type. 
     */
    void setLinetype(const std::string& linetype) {
        this->linetype = linetype;
    }

    /**
     * Sets the entity specific line type scale.
     */
    void setLinetypeScale(double linetypeScale) {
        this->linetypeScale = linetypeScale;
    }

    double getLinetypeScale() const {
        return linetypeScale;
    }

    /**
     * @return Line type.
     */
    std::string getLinetype() const {
        if (linetype.length()==0) {
            return "BYLAYER";
        } else {
            return linetype;
        }
    }

    void setHandle(int h) {
        handle = h;
    }

    int getHandle() const {
        return handle;
    }

    void setInPaperSpace(bool on) {
        inPaperSpace = on;
    }

    bool isInPaperSpace() {
        return inPaperSpace;
    }

private:
    std::string layer;
    int color;
    int color24;
    int width;
    std::string linetype;
    double linetypeScale;
    int handle;

    // DXF code 67 (true: entity in paper space, false: entity in model space (default):
    bool inPaperSpace;
};

#endif

// EOF
