/***************************************************************************
    qgslabelingenginesettings.h
    ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELINGENGINESETTINGS_H
#define QGSLABELINGENGINESETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QColor>

class QgsProject;

/**
 * \ingroup core
 * \brief Stores global configuration for labeling engine
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLabelingEngineSettings
{
  public:

    // TODO QGIS 4 - remove

    /**
     * Search methods in the PAL library to remove colliding labels
     * (methods have different processing speed and number of labels placed)
     */
    enum Search
    {
      Chain,
      Popmusic_Tabu,
      Popmusic_Chain,
      Popmusic_Tabu_Chain,
      Falp
    };

    QgsLabelingEngineSettings();

    //! Returns the configuration to the defaults
    void clear();

    //! Sets flags of the labeling engine
    void setFlags( Qgis::LabelingFlags flags ) { mFlags = flags; }
    //! Gets flags of the labeling engine
    Qgis::LabelingFlags flags() const { return mFlags; }
    //! Test whether a particular flag is enabled
    bool testFlag( Qgis::LabelingFlag f ) const { return mFlags.testFlag( f ); }
    //! Sets whether a particual flag is enabled
    void setFlag( Qgis::LabelingFlag f, bool enabled = true ) { if ( enabled ) mFlags |= f; else mFlags &= ~static_cast< int >( f ); }

    /**
     * Returns the maximum number of line label candidate positions per centimeter.
     *
     * \see setMaximumLineCandidatesPerCm()
     * \since QGIS 3.12
     */
    double maximumLineCandidatesPerCm() const { return mMaxLineCandidatesPerCm; }

    /**
     * Sets the maximum number of line label \a candidates per centimeter.
     *
     * \see maximumLineCandidatesPerCm()
     * \since QGIS 3.12
     */
    void setMaximumLineCandidatesPerCm( double candidates ) { mMaxLineCandidatesPerCm = candidates; }

    /**
     * Returns the maximum number of polygon label candidate positions per centimeter squared.
     *
     * \see setMaximumPolygonCandidatesPerCmSquared()
     * \since QGIS 3.12
     */
    double maximumPolygonCandidatesPerCmSquared() const { return mMaxPolygonCandidatesPerCmSquared; }

    /**
     * Sets the maximum number of polygon label \a candidates per centimeter squared.
     *
     * \see maximumPolygonCandidatesPerCmSquared()
     * \since QGIS 3.12
     */
    void setMaximumPolygonCandidatesPerCmSquared( double candidates ) { mMaxPolygonCandidatesPerCmSquared = candidates; }

    /**
     * Gets number of candidate positions that will be generated for each label feature.
     * \deprecated since QGIS 3.12 use maximumPolygonCandidatesPerCmSquared() and maximumLineCandidatesPerCm() instead.
     */
    Q_DECL_DEPRECATED void numCandidatePositions( int &candPoint, int &candLine, int &candPolygon ) const SIP_DEPRECATED
    {
      Q_UNUSED( candPoint )
      Q_UNUSED( candLine )
      Q_UNUSED( candPolygon )
    }

    /**
     * Sets the number of candidate positions that will be generated for each label feature.
     * \deprecated since QGIS 3.12 use setMaximumPolygonCandidatesPerCmSquared() and setMaximumLineCandidatesPerCm() instead.
     */
    Q_DECL_DEPRECATED void setNumCandidatePositions( int candPoint, int candLine, int candPolygon ) SIP_DEPRECATED
    {
      Q_UNUSED( candPoint )
      Q_UNUSED( candLine )
      Q_UNUSED( candPolygon )
    }

    /**
     * Used to set which search method to use for removal collisions between labels
     * \deprecated since QGIS 3.10 - Chain is always used.
     */
    Q_DECL_DEPRECATED void setSearchMethod( Search s ) SIP_DEPRECATED { Q_UNUSED( s ) }

    /**
     * Which search method to use for removal collisions between labels
     * \deprecated since QGIS 3.10 - Chain is always used.
     */
    Q_DECL_DEPRECATED Search searchMethod() const SIP_DEPRECATED { return Chain; }

    //! Read configuration of the labeling engine from a project
    void readSettingsFromProject( QgsProject *project );
    //! Write configuration of the labeling engine to a project
    void writeSettingsToProject( QgsProject *project );

    // TODO QGIS 4.0: In reality the text render format settings don't only apply to labels, but also
    // ANY text rendered using QgsTextRenderer (including some non-label text items in layouts).
    // These methods should possibly be moved out of here and into the general QgsProject settings.

    /**
     * Returns the default text rendering format for the labels.
     *
     * \see setDefaultTextRenderFormat()
     * \since QGIS 3.4.3
     */
    Qgis::TextRenderFormat defaultTextRenderFormat() const
    {
      return mDefaultTextRenderFormat;
    }

    /**
     * Sets the default text rendering \a format for the labels.
     *
     * \see defaultTextRenderFormat()
     * \since QGIS 3.4.3
     */
    void setDefaultTextRenderFormat( Qgis::TextRenderFormat format )
    {
      mDefaultTextRenderFormat = format;
    }

    /**
     * Returns the color to use when rendering unplaced labels.
     *
     * \see setUnplacedLabelColor()
     * \since QGIS 3.10
     */
    QColor unplacedLabelColor() const;

    /**
     * Sets the \a color to use when rendering unplaced labels.
     *
     * \see unplacedLabelColor()
     * \since QGIS 3.10
     */
    void setUnplacedLabelColor( const QColor &color );

    /**
     * Returns the placement engine version, which dictates how the label placement problem is solved.
     *
     * \see setPlacementVersion()
     * \since QGIS 3.10.2
     */
    Qgis::LabelPlacementEngineVersion placementVersion() const;

    /**
     * Sets the placement engine \a version, which dictates how the label placement problem is solved.
     *
     * \see placementVersion()
     * \since QGIS 3.10.2
     */
    void setPlacementVersion( Qgis::LabelPlacementEngineVersion version );

  private:
    //! Flags
    Qgis::LabelingFlags mFlags = Qgis::LabelingFlag::UsePartialCandidates;
    //! search method to use for removal collisions between labels
    Search mSearchMethod = Chain;

    // maximum density of line/polygon candidates per mm
    double mMaxLineCandidatesPerCm = 5;
    double mMaxPolygonCandidatesPerCmSquared = 2.5;

    QColor mUnplacedLabelColor = QColor( 255, 0, 0 );

    Qgis::LabelPlacementEngineVersion mPlacementVersion = Qgis::LabelPlacementEngineVersion::Version2;

    Qgis::TextRenderFormat mDefaultTextRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;

};

#endif // QGSLABELINGENGINESETTINGS_H
