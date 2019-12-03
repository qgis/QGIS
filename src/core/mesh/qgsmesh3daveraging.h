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

#include <QDomElement>

class QgsMeshLayer;
class QgsMesh3dDataBlock;
class QgsMeshDataBlock;
class QgsMeshDatasetIndex;
class QgsFeedback;
class QgsMeshRenderer3dAveragingSettings;

/**
 * \ingroup core
 * Abstract class to interpolate 3d stacked mesh data to 2d data
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMesh3dAveragingMethod
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    QgsMesh3dAveragingMethod *averagingMethod = dynamic_cast<QgsMesh3dAveragingMethod *>( sipCpp );

    sipType = 0;

    if ( averagingMethod )
    {
      switch ( averagingMethod->method() )
      {
        case QgsMesh3dAveragingMethod::SingleLevelAverageMethod:
          sipType = sipType_QgsMeshSingleLevelAveragingMethod;
          break;
        default:
          sipType = nullptr;
          break;
      }
    }
    SIP_END
#endif

  public:
    //! Type of averaging method
    enum Method
    {
      //! Method to pick single layer
      SingleLevelAverageMethod = 0
    };

    //! Ctor
    QgsMesh3dAveragingMethod( Method method );

    //! Dtor
    virtual ~QgsMesh3dAveragingMethod() = default;

    //! Calculated 2d block values from 3d stacked mesh values
    virtual QgsMeshDataBlock calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback = nullptr ) const = 0;

    /**
     * Writes configuration to a new DOM element
     */
    virtual QDomElement writeXml( QDomDocument &doc ) const = 0;
    //! Reads configuration from the given DOM element
    virtual void readXml( const QDomElement &elem ) = 0;

    //! Returns whether two methods equal
    static bool equals( const QgsMesh3dAveragingMethod *a, const QgsMesh3dAveragingMethod *b );

    //! Returns whether method equals to other
    virtual bool equals( const QgsMesh3dAveragingMethod *other ) const = 0;

    //! Clone the instance
    virtual QgsMesh3dAveragingMethod *clone() const = 0 SIP_FACTORY;

    //! Returns type of averaging method
    Method method() const;

  private:
    Method mMethod;
};

/**
 * \ingroup core
 *
 * No averaging, takes values from one vertical level only
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshSingleLevelAveragingMethod: public QgsMesh3dAveragingMethod
{
  public:
    //! Ctor
    QgsMeshSingleLevelAveragingMethod();
    //! Ctor
    QgsMeshSingleLevelAveragingMethod( int verticalLevel );
    ~QgsMeshSingleLevelAveragingMethod() override;

    QgsMeshDataBlock calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback = nullptr ) const override;
    QDomElement writeXml( QDomDocument &doc ) const override;
    void readXml( const QDomElement &elem ) override;
    bool equals( const QgsMesh3dAveragingMethod *other ) const override;
    QgsMesh3dAveragingMethod *clone() const override SIP_FACTORY;

    //! Returns vertical level. Numbered from 0
    int verticalLevel() const;

  private:
    int mVerticalLevel;
};

#endif // QGSMESH3DAVERAGING_H
