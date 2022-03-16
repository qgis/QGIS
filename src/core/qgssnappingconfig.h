/***************************************************************************
  qgssnappingconfig.h - QgsSnappingConfig

 ---------------------
 begin                : 29.8.2016
 copyright            : (C) 2016 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTSNAPPINGSETTINGS_H
#define QGSPROJECTSNAPPINGSETTINGS_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgstolerance.h"

#include <QHash>
#include <QIcon>

class QDomDocument;
class QgsProject;
class QgsVectorLayer;



/**
 * \ingroup core
 * \brief This is a container for configuration of the snapping of the project
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsSnappingConfig
{
    Q_GADGET

    Q_PROPERTY( QgsProject *project READ project WRITE setProject )

  public:

    /**
     * SnappingType defines on what object the snapping is performed
     * \deprecated since QGIS 3.12 use Qgis::SnappingTypes instead.
     */
    enum SnappingType
    {
      Vertex = 1, //!< On vertices only
      VertexAndSegment = 2, //!< Both on vertices and segments
      Segment = 3, //!< On segments only
    };
    // TODO QGIS 4: remove
    // this could not be tagged with Q_DECL_DEPRECATED due to Doxygen warning
    // might be fixed in newer Doxygen (does not on 1.8.15, might be ok on 1.8.16)

    /**
     * ScaleDependencyMode the scale dependency mode of snapping
     * \since QGIS 3.14
     */
    enum ScaleDependencyMode
    {
      Disabled = 0,//!< No scale dependency
      Global = 1,//!< Scale dependency using global min max range
      PerLayer = 2//!< Scale dependency using min max range per layer
    };
    Q_ENUM( ScaleDependencyMode )

    /**
     * Convenient method to returns the translated name of the enum type
     * Qgis::SnappingTypes.
     * \since QGIS 3.26
     */
    static QString snappingTypeToString( Qgis::SnappingType type );

    /**
     * Convenient method to return the translated name of the enum type
     * Qgis::SnappingTypes.
     * \since QGIS 3.12
     * \deprecated since QGIS 3.26 use Qgis::snappingTypeToString instead
     */
    Q_DECL_DEPRECATED static QString snappingTypeFlagToString( Qgis::SnappingType type ) SIP_DEPRECATED {return snappingTypeToString( type );}


    /**
     * Convenient method to return an icon corresponding to the enum type
     * Qgis::SnappingTypes.
     * \since QGIS 3.20
     */
    static QIcon snappingTypeToIcon( Qgis::SnappingType type );

    /**
     * Convenient method to return an icon corresponding to the enum type
     * Qgis::SnappingTypes.
     * \deprecated since QGIS 3.26 use Qgis::snappingTypeToString instead
     * \since QGIS 3.20
     */
    Q_DECL_DEPRECATED static QIcon snappingTypeFlagToIcon( Qgis::SnappingType type ) SIP_DEPRECATED {return snappingTypeToIcon( type );}

    /**
     * \ingroup core
     * \brief This is a container of advanced configuration (per layer) of the snapping of the project
     * \since QGIS 3.0
     */
    class CORE_EXPORT IndividualLayerSettings
    {
      public:

        /**
         * \brief IndividualLayerSettings
         * \param enabled
         * \param type
         * \param tolerance
         * \param units
         * \deprecated since QGIS 3.12 use the method with Qgis::SnappingTypes instead.
         */
        Q_DECL_DEPRECATED IndividualLayerSettings( bool enabled, SnappingType type, double tolerance, QgsTolerance::UnitType units ) SIP_DEPRECATED;

        /**
         * \brief IndividualLayerSettings
         * \param enabled
         * \param type
         * \param tolerance
         * \param units
         * \param minScale 0.0 disable scale limit
         * \param maxScale 0.0 disable scale limit
         * \since QGIS 3.12
         */
        IndividualLayerSettings( bool enabled, Qgis::SnappingTypes type, double tolerance, QgsTolerance::UnitType units, double minScale = 0.0, double maxScale = 0.0 );

        /**
         * Constructs an invalid setting
         */
        IndividualLayerSettings() = default;

        //! Returns if settings are valid
        bool valid() const;

        //! Returns if snapping is enabled
        bool enabled() const;

        //! enables the snapping
        void setEnabled( bool enabled );

        /**
         * Returns the flags type (vertices | segments | area | centroid | middle)
         * \since QGIS 3.12
         */
        Qgis::SnappingTypes typeFlag() const;

        /**
         * Returns the flags type (vertices | segments | area | centroid | middle)
         * \deprecated since QGIS 3.12 use typeFlag instead.
         */
        Q_DECL_DEPRECATED QgsSnappingConfig::SnappingType type() const SIP_DEPRECATED;

        /**
         * define the type of snapping
        * \deprecated since QGIS 3.12 use setTypeFlag instead.
        */
        Q_DECL_DEPRECATED void setType( SnappingType type ) SIP_DEPRECATED;

        /**
         * define the type of snapping
         * \since QGIS 3.12
         */
        void setTypeFlag( Qgis::SnappingTypes type );

        //! Returns the tolerance
        double tolerance() const;

        //! Sets the tolerance
        void setTolerance( double tolerance );

        //! Returns the type of units
        QgsTolerance::UnitType units() const;

        //! Sets the type of units
        void setUnits( QgsTolerance::UnitType units );

        /**
         * Returns minimum scale on which snapping is limited
         * \since QGIS 3.14
         */
        double minimumScale() const;

        /**
         * Sets the min scale value on which snapping is used, 0.0 disable scale limit
         * \since QGIS 3.14
         */
        void setMinimumScale( double minScale );

        /**
         * Returns max scale on which snapping is limited
         * \since QGIS 3.14
         */
        double maximumScale() const;

        /**
         * Sets the max scale value on which snapping is used, 0.0 disable scale limit
         * \since QGIS 3.14
         */
        void setMaximumScale( double maxScale );

        /**
         * Compare this configuration to other.
         */
        bool operator!= ( const QgsSnappingConfig::IndividualLayerSettings &other ) const;

        // TODO c++20 - replace with = default
        bool operator== ( const QgsSnappingConfig::IndividualLayerSettings &other ) const;

      private:
        bool mValid = false;
        bool mEnabled = false;
        Qgis::SnappingTypes mType = Qgis::SnappingType::Vertex;
        double mTolerance = 0;
        QgsTolerance::UnitType mUnits = QgsTolerance::Pixels;
        double mMinimumScale = 0.0;
        double mMaximumScale = 0.0;
    };

    /**
     * Constructor with default parameters defined in global settings
     */
    explicit QgsSnappingConfig( QgsProject *project = nullptr );

    bool operator==( const QgsSnappingConfig &other ) const;

    //! reset to default values
    void reset();

    //! Returns if snapping is enabled
    bool enabled() const;

    //! enables the snapping
    void setEnabled( bool enabled );

    //! Returns the mode (all layers, active layer, per layer settings)
    Qgis::SnappingMode mode() const;

    //! define the mode of snapping
    void setMode( Qgis::SnappingMode mode );

    /**
     * Returns the flags type (vertices | segments | area | centroid | middle)
     * \since QGIS 3.12
     */
    Qgis::SnappingTypes typeFlag() const;

    /**
     * Returns the flags type (vertices | segments | area | centroid | middle)
     * \deprecated since QGIS 3.12 use typeFlag instead.
     */
    Q_DECL_DEPRECATED QgsSnappingConfig::SnappingType type() const SIP_DEPRECATED;

    /**
     * define the type of snapping
    * \deprecated since QGIS 3.12 use setTypeFlag instead.
    */
    Q_DECL_DEPRECATED void setType( QgsSnappingConfig::SnappingType type );

    /**
     * define the type of snapping
     * \since QGIS 3.12
     */
    void setTypeFlag( Qgis::SnappingTypes type );

    //! Returns the tolerance
    double tolerance() const;

    //! Sets the tolerance
    void setTolerance( double tolerance );

    /**
     * Returns the min scale (i.e. most \"zoomed out\" scale)
     * \since QGIS 3.14
     */
    double minimumScale() const;

    /**
     * Sets the min scale on which snapping is enabled, 0.0 disable scale limit
     * \since QGIS 3.14
     */
    void setMinimumScale( double minScale );

    /**
     * Returns the max scale (i.e. most \"zoomed in\" scale)
     * \since QGIS 3.14
     */
    double maximumScale() const;

    /**
     * Set the max scale on which snapping is enabled, 0.0 disable scale limit
     * \since QGIS 3.14
     */
    void setMaximumScale( double maxScale );

    /**
     * Set the scale dependency mode
     * \since QGIS 3.14
     */
    void setScaleDependencyMode( ScaleDependencyMode mode );

    /**
     * Returns the scale dependency mode
     * \since QGIS 3.14
     */
    ScaleDependencyMode scaleDependencyMode() const;

    //! Returns the type of units
    QgsTolerance::UnitType units() const;

    //! Sets the type of units
    void setUnits( QgsTolerance::UnitType units );

    //! Returns if the snapping on intersection is enabled
    bool intersectionSnapping() const;

    //! Sets if the snapping on intersection is enabled
    void setIntersectionSnapping( bool enabled );

    /**
     * Returns if self snapping (snapping to the currently digitized feature) is enabled
     *
     * \since QGIS 3.14
     */
    bool selfSnapping() const;

    /**
     * Sets if self snapping (snapping to the currently digitized feature) is enabled
     *
     * \since QGIS 3.14
     */
    void setSelfSnapping( bool enabled );

    //! Returns individual snapping settings for all layers
#ifndef SIP_RUN
    QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> individualLayerSettings() const;
#else
    SIP_PYDICT individualLayerSettings() const;
    % MethodCode
    // Create the dictionary.
    PyObject *d = PyDict_New();
    if ( !d )
      return nullptr;
    // Set the dictionary elements.
    QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> container = sipCpp->individualLayerSettings();
    QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings>::const_iterator i = container.constBegin();
    while ( i != container.constEnd() )
    {
      QgsVectorLayer *vl = i.key();
      QgsSnappingConfig::IndividualLayerSettings *ils = new QgsSnappingConfig::IndividualLayerSettings( i.value() );

      PyObject *vlobj = sipConvertFromType( vl, sipType_QgsVectorLayer, nullptr );
      PyObject *ilsobj = sipConvertFromType( ils, sipType_QgsSnappingConfig_IndividualLayerSettings, Py_None );

      if ( !vlobj || !ilsobj || PyDict_SetItem( d, vlobj, ilsobj ) < 0 )
      {
        Py_DECREF( d );
        if ( vlobj )
        {
          Py_DECREF( vlobj );
        }
        if ( ilsobj )
        {
          Py_DECREF( ilsobj );
        }
        else
        {
          delete ils;
        }
        PyErr_SetString( PyExc_StopIteration, "" );
      }
      Py_DECREF( vlobj );
      Py_DECREF( ilsobj );
      ++i;
    }
    sipRes = d;
    % End
#endif

    //! Returns individual layer snappings settings (applied if mode is AdvancedConfiguration)
    QgsSnappingConfig::IndividualLayerSettings individualLayerSettings( QgsVectorLayer *vl ) const;

    //! Sets individual layer snappings settings (applied if mode is AdvancedConfiguration)
    void setIndividualLayerSettings( QgsVectorLayer *vl, const QgsSnappingConfig::IndividualLayerSettings &individualLayerSettings );

    /**
     * Removes all individual layer snapping settings
     *
     * \since QGIS 3.16
     */
    void clearIndividualLayerSettings();

    /**
     * Compare this configuration to other.
     */
    bool operator!= ( const QgsSnappingConfig &other ) const;

    /**
     * Reads the configuration from the specified QGIS project document.
     *
     * \since QGIS 3.0
     */
    void readProject( const QDomDocument &doc );

    /**
     * Writes the configuration to the specified QGIS project document.
     *
     * \since QGIS 3.0
     */
    void writeProject( QDomDocument &doc );

    /**
     * Adds the specified layers as individual layers to the configuration
     * with standard configuration.
     * When implementing a long-living QgsSnappingConfig (like the one in QgsProject)
     * it is best to directly feed this with information from the layer registry.
     *
     * \returns TRUE if changes have been done.
     *
     * \since QGIS 3.0
     */
    bool addLayers( const QList<QgsMapLayer *> &layers );


    /**
     * Removes the specified layers from the individual layer configuration.
     * When implementing a long-living QgsSnappingConfig (like the one in QgsProject)
     * it is best to directly feed this with information from the layer registry.
     *
     * \returns TRUE if changes have been done.
     *
     * \since QGIS 3.0
     */
    bool removeLayers( const QList<QgsMapLayer *> &layers );

    /**
     * The project from which the snapped layers should be retrieved
     *
     * \since QGIS 3.0
     */
    QgsProject *project() const;

    /**
     * The project from which the snapped layers should be retrieved
     *
     * \since QGIS 3.0
     */
    void setProject( QgsProject *project );

  private:
    void readLegacySettings();

    //! associated project for this snapping configuration
    QgsProject *mProject = nullptr;
    bool mEnabled = false;
    Qgis::SnappingMode mMode = Qgis::SnappingMode::ActiveLayer;
    Qgis::SnappingTypes mType = Qgis::SnappingType::Vertex;
    double mTolerance = 0.0;
    ScaleDependencyMode mScaleDependencyMode = Disabled;
    double mMinimumScale = 0.0;
    double mMaximumScale = 0.0;
    QgsTolerance::UnitType mUnits = QgsTolerance::ProjectUnits;
    bool mIntersectionSnapping = false;
    bool mSelfSnapping = false;

    QHash<QgsVectorLayer *, IndividualLayerSettings> mIndividualLayerSettings;

};

#endif // QGSPROJECTSNAPPINGSETTINGS_H
