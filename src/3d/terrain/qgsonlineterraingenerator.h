/***************************************************************************
  qgsonlineterraingenerator.h
  --------------------------------------
  Date                 : March 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSONLINETERRAINGENERATOR_H
#define QGSONLINETERRAINGENERATOR_H

#include "qgis_3d.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsdemterraingenerator.h"
#include "qgsterraingenerator.h"

#define SIP_NO_FILE

class QgsDemHeightMapGenerator;


/**
 * \ingroup qgis_3d
 * \brief Implementation of terrain generator that uses online resources to
 * download heightmaps.
 *
 * QgsDemTerrainGenerator does that automatically just by forcing mLayer =
 * nullptr.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.8
 */
class _3D_EXPORT QgsOnlineTerrainGenerator : public QgsDemTerrainGenerator
{
    Q_OBJECT
  public:
    // cppcheck-suppress duplInheritedMember
    /**
     * Creates a new instance of a QgsOnlineTerrainGenerator object.
     */
    static QgsTerrainGenerator *create() SIP_FACTORY;

    QgsOnlineTerrainGenerator();
    ~QgsOnlineTerrainGenerator() override;

    Type type() const override;

  protected:
    void updateGenerator() override;

  private:
    // Hide layer getter/setter, mLayer is always nullptr with online terrain.
    using QgsDemTerrainGenerator::layer;
    using QgsDemTerrainGenerator::setLayer;
};

#endif // QGSONLINETERRAINGENERATOR_H
