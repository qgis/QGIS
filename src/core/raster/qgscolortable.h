/***************************************************************************
                              qgscolortable.h
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSCOLORTABLE_H
#define QGSCOLORTABLE_H

#include <QVector>

/*
 * Because of performance, this class can store color rules in 2 forms:
 *   1) discrete - vector of descrete values and their colors; can be accessed by index (from 0)
 *   2) ramp - vector of ramps defined by min and max value and a color for min amd max value,
 *             colors between min and max are interpolated linearly.
 *
 * Only one type should be used for one instance.
 *
 */


typedef struct
{
  unsigned char c1, c2, c3, c4;
} DISCRETE;

typedef struct
{
  double min, max;
  unsigned char min_c1, min_c2, min_c3, min_c4;
  unsigned char max_c1, max_c2, max_c3, max_c4;
} RAMP;
class CORE_EXPORT QgsColorTable
{
  public:
    /**
    *  \brief Constructor.
    *  \param interp color table interpretation
    */
    QgsColorTable( int interp = Rgb );

    /** \brief The destuctor.  */
    ~QgsColorTable();

    /** \brief Color table interpretation. */
    // This is taken from GDAL, GPI_RGB is mapped to Rgba
    enum Interp
    {
      Gray = 0, // Use c1 as grayscale value.
      Rgb,      // Use c1 as red, c2 as green, c3 as blue
      Rgba,     // Use c1 as red, c2 as green, c3 as blue and c4 as alpha.
      Cmyk,     // Use c1 as cyan, c2 as magenta, c3 as yellow and c4 as black.
      Hls       // c1 hue, c2 lightness, c3 saturation; should be His ?
    };

    /**
    *  \brief Color table is defined
    *  \return true if at least one rule is defined
    *  \false no rule defined
    */
    bool defined( void );

    /** \brief Get color table interpretation */
    int interpretation( void );

    /** \brief Add a discrete color */
    void add( int index, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4 = 0 );

    /** \brief Add a ramp rule */
    void add( double min,  double max,
              unsigned char min_c1, unsigned char min_c2, unsigned char min_c3, unsigned char min_c4,
              unsigned char max_c1, unsigned char max_c2, unsigned char max_c3, unsigned char max_c4 );

    /** \brief Get color
    *  \return true color was found
    *  \return false color was no found
    */
    bool color( double value, int *c1, int *c2, int *c3 );

    /** \brief Sort ramp rules */
    void sort( void );

    /** \brief Print to stderr - for debuging */
    void print( void );

    /** \brief Minimum value */
    double rmin();

    /** \brief Maximum value */
    double rmax();

    /** \brief Clear the color table */
    void clear();

  private:
    /** \brief vector of discrete values */
    QVector<DISCRETE> mDiscrete;

    /** \brief vector of ramp rules */
    QVector<RAMP> mRamp;

    /** \brief color table interpretation */
    int  mInterp;

    /** \brief min value */
    double mMin;

    /** \brief max value */
    double mMax;
};

#endif

