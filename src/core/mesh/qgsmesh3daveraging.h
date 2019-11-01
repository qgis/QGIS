/***************************************************************************
                         qgsmesh3daveraging.h
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESH3DAVERAGING_H
#define QGSMESH3DAVERAGING_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsMeshLayer;
class QgsMesh3dDataBlock;
class QgsMeshDataBlock;
class QgsMeshDatasetIndex;
class QgsFeedback;
class QgsMeshRenderer3dAveragingSettings;

#define SIP_NO_FILE
///@cond PRIVATE

/**
 * \ingroup core
 * Abstract class to interpolate 3d stacked mesh data to 2d data
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class QgsMesh3dAveragingMethod
{
  public:
    enum Method
    {
      SingleLevelAverageMethod = 0
    };

    QgsMesh3dAveragingMethod( Method method );
    virtual ~QgsMesh3dAveragingMethod() = default;
    virtual QgsMeshDataBlock calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback = nullptr ) const = 0;

    static QgsMesh3dAveragingMethod *create( const QgsMeshRenderer3dAveragingSettings &settings );
    static bool equals( QgsMesh3dAveragingMethod *a, QgsMesh3dAveragingMethod *b );

    Method method() const;

  private:
    const Method mMethod;
};

/**
 * \ingroup core
 *
 * no averaging, takes values from one vertical level only
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshSingleLevelAverageMethod: public QgsMesh3dAveragingMethod
{
  public:
    QgsMeshSingleLevelAverageMethod( int verticalLevel );
    ~QgsMeshSingleLevelAverageMethod() override;

    QgsMeshDataBlock calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback = nullptr ) const override;

    bool operator==( const QgsMeshSingleLevelAverageMethod &rhs ) const;
    int verticalLevel() const;

  private:
    // numbered from 0
    int mVerticalLevel;
};

///@endcond

#endif // QGSMESH3DAVERAGING_H
