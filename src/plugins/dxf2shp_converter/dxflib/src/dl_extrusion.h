/****************************************************************************
** $Id: dl_extrusion.h 273 2005-02-28 18:14:39Z andrew $
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

#ifndef DL_EXTRUSION_H
#define DL_EXTRUSION_H

#include <math.h>


/**
 * Storing and passing around attributes. Attributes
 * are the layer name, color, width and line type.
 *
 * @author Andrew Mustun
 */
class DL_Extrusion
{

  public:

    /**
     * Default constructor.
     */
    DL_Extrusion()
    {
      direction = new double[3];
      setDirection( 0.0, 0.0, 1.0 );
      setElevation( 0.0 );
    }


    /**
     * Destructor.
     */
    ~DL_Extrusion()
    {
      delete direction ;
    }


    /**
     * Constructor for DXF extrusion.
     *
     * @param direction Vector of axis along which the entity shall be extruded
    *                  this is also the Z axis of the Entity coordinate system
     * @param elevation Distance of the entities XY plane from the origin of the
    *                  world coordinate system
     */
    DL_Extrusion( double dx, double dy, double dz, double elevation )
    {
      direction = new double[3];
      setDirection( dx, dy, dz );
      setElevation( elevation );
    }



    /**
     * Sets the direction vector.
     */
    void setDirection( double dx, double dy, double dz )
    {
      direction[0] = dx;
      direction[1] = dy;
      direction[2] = dz;
    }



    /**
     * @return direction vector.
     */
    double* getDirection() const
    {
      return direction;
    }



    /**
     * @return direction vector.
     */
    void getDirection( double dir[] ) const
    {
      dir[0] = direction[0];
      dir[1] = direction[1];
      dir[2] = direction[2];
    }



    /**
     * Sets the elevation.
     */
    void setElevation( double elevation )
    {
      this->elevation = elevation;
    }



    /**
     * @return Elevation.
     */
    double getElevation() const
    {
      return elevation;
    }



    /**
     * Copies extrusion (deep copies) from another extrusion object.
     */
    DL_Extrusion &operator= ( const DL_Extrusion& extru )
    {
      setDirection( extru.direction[0], extru.direction[1], extru.direction[2] );
      setElevation( extru.elevation );

      return *this;
    }



  private:
    double *direction;
    double elevation;
};

#endif

// EOF
