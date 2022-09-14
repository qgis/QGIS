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
 * \brief Abstract class to interpolate 3d stacked mesh data to 2d data
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMesh3dAveragingMethod SIP_ABSTRACT
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    QgsMesh3dAveragingMethod *averagingMethod = dynamic_cast<QgsMesh3dAveragingMethod *>( sipCpp );

    sipType = 0;

    if ( averagingMethod )
    {
      switch ( averagingMethod->method() )
      {
        case QgsMesh3dAveragingMethod::MultiLevelsAveragingMethod:
          sipType = sipType_QgsMeshMultiLevelsAveragingMethod;
          break;
        case QgsMesh3dAveragingMethod::SigmaAveragingMethod:
          sipType = sipType_QgsMeshSigmaAveragingMethod;
          break;
        case QgsMesh3dAveragingMethod::RelativeHeightAveragingMethod:
          sipType = sipType_QgsMeshRelativeHeightAveragingMethod;
          break;
        case QgsMesh3dAveragingMethod::ElevationAveragingMethod:
          sipType = sipType_QgsMeshElevationAveragingMethod;
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
      //! Method to average values from selected vertical layers
      MultiLevelsAveragingMethod = 0,
      //! Method to average values between 0 (bed level) and 1 (surface)
      SigmaAveragingMethod,
      //! Method to average values defined by range of relative length units to the surface or bed level
      RelativeHeightAveragingMethod,
      //! Method to average values defined by range of absolute length units to the model's datum
      ElevationAveragingMethod
    };

    //! Ctor
    QgsMesh3dAveragingMethod( Method method );

    //! Dtor
    virtual ~QgsMesh3dAveragingMethod() = default;

    //! Calculated 2d block values from 3d stacked mesh values
    QgsMeshDataBlock calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback = nullptr ) const;

    /**
     * Writes configuration to a new DOM element
     */
    virtual QDomElement writeXml( QDomDocument &doc ) const = 0;

    //! Creates the instance from XML by calling readXml of derived classes
    static QgsMesh3dAveragingMethod *createFromXml( const QDomElement &elem ) SIP_FACTORY;

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
    //! Returns whether the method is correctly initialized
    virtual bool hasValidInputs() const = 0;

    /**
     * For one face, Calculates average of volume values
     */
    void averageVolumeValuesForFace(
      int faceIndex,
      int volumesBelowFaceCount,
      int startVolumeIndex,
      double methodLevelTop,
      double methodLevelBottom,
      bool isVector,
      const QVector<double> &verticalLevelsForFace,
      const QVector<double> &volumeValues,
      QVector<double> &valuesFaces
    ) const;

    /**
     * For one face, calculates absolute vertical levels between which we need to average
     */
    virtual void volumeRangeForFace(
      double &startVerticalLevel,
      double &endVerticalLevel,
      int &singleVerticalLevel,
      const QVector<double> &verticalLevels ) const = 0;

    Method mMethod;
};

/**
 * \ingroup core
 *
 * \brief Multi level averaging method specifies limits of vertical layers from the top layer down or reversed.
 *
 * The limits will be truncated to the maximum number of vertical layers.
 * To pick value from a single layer, specify the upper and lower limit to be the same
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshMultiLevelsAveragingMethod: public QgsMesh3dAveragingMethod
{
  public:
    //! Constructs single level averaging method for 1st (top) vertical level
    QgsMeshMultiLevelsAveragingMethod();

    /**
     * Constructs multi level averaging method
     * \param startLevel starting vertical level index numbered from 1
     * \param endLevel ending vertical level index numbered from 1 (higher or equal than startLevel)
     * \param countedFromTop if TRUE, the startLevel index is counted from surface (index 1 is the top layer).
     *                       if FALSE, the startLevel index is counted from the bed level (index 1 is the bottom layer)
     */
    QgsMeshMultiLevelsAveragingMethod( int startLevel, int endLevel, bool countedFromTop );

    /**
     * Constructs single level averaging method
     * \param verticalLevel vertical level index numbered from 1
     * \param countedFromTop if TRUE, the startLevel index is counted from surface (index 1 is the top layer).
     *                       if FALSE, the startLevel index is counted from the bed level (index 1 is the bottom layer)
     */
    QgsMeshMultiLevelsAveragingMethod( int verticalLevel, bool countedFromTop );

    ~QgsMeshMultiLevelsAveragingMethod() override;
    QDomElement writeXml( QDomDocument &doc ) const override;
    void readXml( const QDomElement &elem ) override;
    bool equals( const QgsMesh3dAveragingMethod *other ) const override;
    QgsMesh3dAveragingMethod *clone() const override SIP_FACTORY;

    /**
     * Returns starting vertical level.
     *
     * Numbered from 1. If countedFromTop(), 1 represents the top (surface) level,
     * otherwise 1 represents the bottom (bed) level
     *
     * Always lower or equal than endVerticalLevel()
     */
    int startVerticalLevel() const;

    /**
     * Returns ending vertical level.
     *
     * Numbered from 1. If countedFromTop(), 1 represents the top (surface) level,
     * otherwise 1 represents the bottom (bed) level
     *
     * Always lower or equal than endVerticalLevel()
     */
    int endVerticalLevel() const;

    /**
     * Returns whether the start and end vertical levels are indexed from top (surface) or bottom (bed) level
     */
    bool countedFromTop() const;

    /**
     * Returns whether the averaging method selects only a single vertical level
     */
    bool isSingleLevel() const;

  private:
    bool hasValidInputs() const override;
    void volumeRangeForFace( double &startVerticalLevel,
                             double &endVerticalLevel,
                             int &singleVerticalIndex,
                             const QVector<double> &verticalLevels ) const override;
    void setLevels( int startVerticalLevel, int endVerticalLevel );
    int mStartVerticalLevel = 1;
    int mEndVerticalLevel = 1;
    bool mCountedFromTop = true;
};

/**
 * \ingroup core
 *
 * \brief Sigma averages over the values between 0 (bed level) and 1 (surface).
 *
 * The fractions will be truncated to 0-1.
 * For example: the average of between a quarter and 3 quarters of the water column - Sigma from 0.25 to 0.75
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshSigmaAveragingMethod: public QgsMesh3dAveragingMethod
{
  public:
    //! Constructs the sigma method for whole value range 0-1
    QgsMeshSigmaAveragingMethod();

    /**
     * Constructs the sigma method
     * \param startFraction starting fraction (0-1)
     * \param endFraction ending fraction, must be higher or equal than startFraction (0-1)
     */
    QgsMeshSigmaAveragingMethod( double startFraction, double endFraction );

    ~QgsMeshSigmaAveragingMethod() override;
    QDomElement writeXml( QDomDocument &doc ) const override;
    void readXml( const QDomElement &elem ) override;
    bool equals( const QgsMesh3dAveragingMethod *other ) const override;
    QgsMesh3dAveragingMethod *clone() const override SIP_FACTORY;

    /**
     * Returns starting fraction.
     *
     * In range 0-1, where 1 means the surface level and 0 bed level.
     * Always lower or equal than endFraction()
     */
    double startFraction() const;

    /**
     * Returns ending fraction.
     *
     * In range 0-1, where 1 means the surface level and 0 bed level.
     * Always higher or equal than startFraction()
     */
    double endFraction() const;

  private:
    bool hasValidInputs() const override;
    void volumeRangeForFace( double &startVerticalLevel,
                             double &endVerticalLevel,
                             int &singleVerticalIndex,
                             const QVector<double> &verticalLevels ) const override;

    double mStartFraction = 0;
    double mEndFraction = 1;
};

/**
 * \ingroup core
 *
 * \brief Relative height averaging method averages the values based on range defined relative to bed elevation or surface (when countedFromTop())
 * The range is defined in the same length units as defined by model (e.g. meters)
 *
 * if countedFromTop(), the method represents averaging based on depth below surface.
 * For example one can pull out results for between 6 to 12 meters below the water surface - depth from 6m to 12m.
 * The depth will be truncated at the bed level.
 *
 * if not countedFromTop(), the method represents averaging based on height above bed level.
 * For example one can pull out results for between 6 to 12 meters above the bed - height from 6m to 12m.
 * The height will be truncated at the bed level.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshRelativeHeightAveragingMethod: public QgsMesh3dAveragingMethod
{
  public:

    //! Constructs default depth averaging method
    QgsMeshRelativeHeightAveragingMethod();

    /**
     * Constructs the depth/height averaging method
     * \param startHeight starting depth/height, higher or equal than 0
     * \param endHeight ending depth/height, higher or equal than startDepth
     * \param countedFromTop if TRUE, the startLength and endLength is relative to surface (0 is surface level).
     *                       if FALSE, the startLength and endLength is relative to bed (0 is bed level).
     */
    QgsMeshRelativeHeightAveragingMethod( double startHeight, double endHeight, bool countedFromTop );

    ~QgsMeshRelativeHeightAveragingMethod() override;
    QDomElement writeXml( QDomDocument &doc ) const override;
    void readXml( const QDomElement &elem ) override;
    bool equals( const QgsMesh3dAveragingMethod *other ) const override;
    QgsMesh3dAveragingMethod *clone() const override SIP_FACTORY;

    /**
     * Returns starting depth/height.
     *
     * Always lower or equal than endLength()
     */
    double startHeight() const;

    /**
     * Returns ending depth/height.
     *
     * Always higher or equal than startLength()
     */
    double endHeight() const;

    /**
     * Returns whether the start and end vertical levels are relative to top (surface) or bottom (bed) level
     */
    bool countedFromTop() const;

  private:
    bool hasValidInputs() const override;
    void volumeRangeForFace( double &startVerticalLevel,
                             double &endVerticalLevel,
                             int &singleVerticalIndex,
                             const QVector<double> &verticalLevels ) const override;
    double mStartHeight = 0;
    double mEndHeight = 0;
    bool mCountedFromTop = true;
};

/**
 * \ingroup core
 *
 * \brief Elevation averaging method averages the values based on range defined absolute value to the model's datum
 * The range is defined in the same length units as defined by model (e.g. meters)
 *
 * For example one can pull out results irrespective of water level change such as between -16m and -10m.
 * The elevation will be truncated at the surface and bed levels.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMeshElevationAveragingMethod: public QgsMesh3dAveragingMethod
{
  public:

    //! Ctor
    QgsMeshElevationAveragingMethod();

    /**
     * Constructs elevation averaging method
     * \param startElevation start elevation (absolute to model's datum)
     * \param endElevation end elevation (absolute to model's datum)
     */
    QgsMeshElevationAveragingMethod( double startElevation, double endElevation );
    ~QgsMeshElevationAveragingMethod() override;

    QDomElement writeXml( QDomDocument &doc ) const override;
    void readXml( const QDomElement &elem ) override;
    bool equals( const QgsMesh3dAveragingMethod *other ) const override;
    QgsMesh3dAveragingMethod *clone() const override SIP_FACTORY;

    /**
     * Returns start elevation
     */
    double startElevation() const;

    /**
     * Returns end elevation
     */
    double endElevation() const;

  private:
    bool hasValidInputs() const override;
    void volumeRangeForFace( double &startVerticalLevel,
                             double &endVerticalLevel,
                             int &singleVerticalIndex,
                             const QVector<double> &verticalLevels ) const override;
    double mStartElevation = 0;
    double mEndElevation = 0;
};

#endif // QGSMESH3DAVERAGING_H
