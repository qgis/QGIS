/****************************************************************************
** $Id: dl_attributes.h 2334 2005-03-27 23:37:52Z andrew $
**
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This program is free software; you can redistribute it and/or modify  
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; version 2 of the License
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

#include <string>
using std::string;

#include "dl_codes.h"

/**
 * Storing and passing around attributes. Attributes
 * are the layer name, color, width and line type.
 *
 * @author Andrew Mustun
 */
class DL_Attributes
{

  public:

    /**
     * Default constructor.
     */
    DL_Attributes()
    {
      setLayer( "" );
      setColor( 0 );
      setWidth( 0 );
      setLineType( "BYLAYER" );
    }



    /**
     * Constructor for DXF attributes.
     *
     * @param layer Layer name for this entity or NULL for no layer
     *              (every entity should be on a named layer!).
     * @param color Color number (0..256). 0 = BYBLOCK, 256 = BYLAYER.
     * @param width Line thickness. Defaults to zero. -1 = BYLAYER,
     *               -2 = BYBLOCK, -3 = default width
     * @param lineType Line type name or "BYLAYER" or "BYBLOCK". Defaults
     *              to "BYLAYER"
     */
    DL_Attributes( const string& layer,
                   int color, int width,
                   const string& lineType )
    {
      setLayer( layer );
      setColor( color );
      setWidth( width );
      setLineType( lineType );
    }



    /**
     * Sets the layer. If the given pointer points to NULL, the
     *  new layer name will be an empty but valid string.
     */
    void setLayer( const string& layer )
    {
      this->layer = layer;
    }



    /**
     * @return Layer name.
     */
    string getLayer() const
    {
      return layer;
    }



    /**
     * Sets the color.
     *
     * @see DL_Codes, dxfColors
     */
    void setColor( int color )
    {
      this->color = color;
    }



    /**
     * @return Color.
     *
     * @see DL_Codes, dxfColors
     */
    int getColor() const
    {
      return color;
    }



    /**
     * Sets the width.
     */
    void setWidth( int width )
    {
      this->width = width;
    }



    /**
     * @return Width.
     */
    int getWidth() const
    {
      return width;
    }



    /**
     * Sets the line type. This can be any string and is not
     *  checked to be a valid line type.
     */
    void setLineType( const string& lineType )
    {
      this->lineType = lineType;
    }



    /**
     * @return Line type.
     */
    string getLineType() const
    {
      if ( lineType.length() == 0 )
      {
        return "BYLAYER";
      }
      else
      {
        return lineType;
      }
    }



    /**
     * Copies attributes (deep copies) from another attribute object.
     */
    DL_Attributes &operator= ( const DL_Attributes& attrib )
    {
      setLayer( attrib.layer );
      setColor( attrib.color );
      setWidth( attrib.width );
      setLineType( attrib.lineType );

      return *this;
    }

  private:
    string layer;
    int color;
    int width;
    string lineType;
};

#endif

// EOF
