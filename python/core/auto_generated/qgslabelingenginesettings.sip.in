/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslabelingenginesettings.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLabelingEngineSettings
{
%Docstring
Stores global configuration for labeling engine

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslabelingenginesettings.h"
%End
  public:
    enum Flag
    {
      UseAllLabels,
      UsePartialCandidates,
      // TODO QGIS 4.0: remove
      RenderOutlineLabels,
      DrawLabelRectOnly,
      DrawCandidates,
    };
    typedef QFlags<QgsLabelingEngineSettings::Flag> Flags;


    enum Search
    {
      Chain,
      Popmusic_Tabu,
      Popmusic_Chain,
      Popmusic_Tabu_Chain,
      Falp
    };

    QgsLabelingEngineSettings();

    void clear();
%Docstring
Returns the configuration to the defaults
%End

    void setFlags( Flags flags );
%Docstring
Sets flags of the labeling engine
%End
    Flags flags() const;
%Docstring
Gets flags of the labeling engine
%End
    bool testFlag( Flag f ) const;
%Docstring
Test whether a particular flag is enabled
%End
    void setFlag( Flag f, bool enabled = true );
%Docstring
Sets whether a particual flag is enabled
%End

    void numCandidatePositions( int &candPoint, int &candLine, int &candPolygon ) const;
%Docstring
Gets number of candidate positions that will be generated for each label feature (default to 8)
%End
    void setNumCandidatePositions( int candPoint, int candLine, int candPolygon );
%Docstring
Sets number of candidate positions that will be generated for each label feature
%End

    void setSearchMethod( Search s );
%Docstring
Sets which search method to use for removal collisions between labels
%End
    Search searchMethod() const;
%Docstring
Which search method to use for removal collisions between labels
%End

    void readSettingsFromProject( QgsProject *project );
%Docstring
Read configuration of the labeling engine from a project
%End
    void writeSettingsToProject( QgsProject *project );
%Docstring
Write configuration of the labeling engine to a project
%End


    QgsRenderContext::TextRenderFormat defaultTextRenderFormat() const;
%Docstring
Returns the default text rendering format for the labels.

.. seealso:: :py:func:`setDefaultTextRenderFormat`

.. versionadded:: 3.4.3
%End

    void setDefaultTextRenderFormat( QgsRenderContext::TextRenderFormat format );
%Docstring
Sets the default text rendering ``format`` for the labels.

.. seealso:: :py:func:`defaultTextRenderFormat`

.. versionadded:: 3.4.3
%End

};

QFlags<QgsLabelingEngineSettings::Flag> operator|(QgsLabelingEngineSettings::Flag f1, QFlags<QgsLabelingEngineSettings::Flag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslabelingenginesettings.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
