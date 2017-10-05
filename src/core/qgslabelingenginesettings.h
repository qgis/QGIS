#ifndef QGSLABELINGENGINESETTINGS_H
#define QGSLABELINGENGINESETTINGS_H

#include "qgis_core.h"

#include <QFlags>

class QgsProject;

/**
 * \ingroup core
 * Stores global configuration for labeling engine
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLabelingEngineSettings
{
  public:
    //! Various flags that affect drawing and placement of labels
    enum Flag
    {
      UseAllLabels          = 1 << 1,  //!< Whether to draw all labels even if there would be collisions
      UsePartialCandidates  = 1 << 2,  //!< Whether to use also label candidates that are partially outside of the map view
      RenderOutlineLabels   = 1 << 3,  //!< Whether to render labels as text or outlines
      DrawLabelRectOnly     = 1 << 4,  //!< Whether to only draw the label rect and not the actual label text (used for unit tests)
      DrawCandidates        = 1 << 5,  //!< Whether to draw rectangles of generated candidates (good for debugging)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

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

    //! Return the configuration to the defaults
    void clear();

    //! Set flags of the labeling engine
    void setFlags( Flags flags ) { mFlags = flags; }
    //! Get flags of the labeling engine
    Flags flags() const { return mFlags; }
    //! Test whether a particular flag is enabled
    bool testFlag( Flag f ) const { return mFlags.testFlag( f ); }
    //! Set whether a particual flag is enabled
    void setFlag( Flag f, bool enabled = true ) { if ( enabled ) mFlags |= f; else mFlags &= ~f; }

    //! Get number of candidate positions that will be generated for each label feature (default to 8)
    void numCandidatePositions( int &candPoint, int &candLine, int &candPolygon ) const { candPoint = mCandPoint; candLine = mCandLine; candPolygon = mCandPolygon; }
    //! Set number of candidate positions that will be generated for each label feature
    void setNumCandidatePositions( int candPoint, int candLine, int candPolygon ) { mCandPoint = candPoint; mCandLine = candLine; mCandPolygon = candPolygon; }

    //! Set which search method to use for removal collisions between labels
    void setSearchMethod( Search s ) { mSearchMethod = s; }
    //! Which search method to use for removal collisions between labels
    Search searchMethod() const { return mSearchMethod; }

    //! Read configuration of the labeling engine from a project
    void readSettingsFromProject( QgsProject *project );
    //! Write configuration of the labeling engine to a project
    void writeSettingsToProject( QgsProject *project );

  private:
    //! Flags
    Flags mFlags;
    //! search method to use for removal collisions between labels
    Search mSearchMethod = Chain;
    //! Number of candedate positions that will be generated for features
    int mCandPoint = 16, mCandLine = 50, mCandPolygon = 30;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLabelingEngineSettings::Flags )

#endif // QGSLABELINGENGINESETTINGS_H
