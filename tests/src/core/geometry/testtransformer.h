/***************************************************************************
     testtransformer.h
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrytransformer.h"

class TestTransformer : public QgsAbstractGeometryTransformer
{
  public:

    bool transformPoint( double &x, double &y, double &z, double &m ) override
    {
      x *= 3;
      y += 14;
      z += 5;
      m -= 1;
      return true;
    }
};


class TestFailTransformer : public QgsAbstractGeometryTransformer
{
  public:

    bool transformPoint( double &x, double &y, double &z, double &m ) override
    {
      x *= 3;
      y += 14;
      z += 5;
      m -= 1;
      return false;
    }
};

