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

#include <QHash>

#include "qgis_core.h"
#include "qgstolerance.h"

class QDomDocument;
class QgsProject;
class QgsVectorLayer;


/**
 * \ingroup core
 * This is a container for configuration of the snapping of the project
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsSnappingConfig
{
    Q_GADGET

    Q_PROPERTY( QgsProject *project READ project WRITE setProject )

  public:

    /**
     * SnappingMode defines on which layer the snapping is performed
     */
    enum SnappingMode
    {
      ActiveLayer = 1, //!< On the active layer
      AllLayers = 2, //!< On all vector layers
      AdvancedConfiguration = 3, //!< On a per layer configuration basis
    };
    Q_ENUM( SnappingMode )

    /**
     * SnappingType defines on what object the snapping is performed
     */
    enum SnappingType
    {
      Vertex = 1, //!< On vertices only
      VertexAndSegment = 2, //!< Both on vertices and segments
      Segment = 3, //!< On segments only
    };
    Q_ENUM( SnappingType )

    /**
     * \ingroup core
     * This is a container of advanced configuration (per layer) of the snapping of the project
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
         */
        IndividualLayerSettings( bool enabled, QgsSnappingConfig::SnappingType type, double tolerance, QgsTolerance::UnitType units );

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

        //! Returns the type (vertices and/or segments)
        QgsSnappingConfig::SnappingType type() const;

        //! define the type of snapping
        void setType( QgsSnappingConfig::SnappingType type );

        //! Returns the tolerance
        double tolerance() const;

        //! Sets the tolerance
        void setTolerance( double tolerance );

        //! Returns the type of units
        QgsTolerance::UnitType units() const;

        //! Sets the type of units
        void setUnits( QgsTolerance::UnitType units );

        /**
         * Compare this configuration to other.
         */
        bool operator!= ( const QgsSnappingConfig::IndividualLayerSettings &other ) const;

        bool operator== ( const QgsSnappingConfig::IndividualLayerSettings &other ) const;

      private:
        bool mValid = false;
        bool mEnabled = false;
        SnappingType mType = Vertex;
        double mTolerance = 0;
        QgsTolerance::UnitType mUnits = QgsTolerance::Pixels;
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
    SnappingMode mode() const;

    //! define the mode of snapping
    void setMode( SnappingMode mode );

    //! Returns the type (vertices and/or segments)
    SnappingType type() const;

    //! define the type of snapping
    void setType( SnappingType type );

    //! Returns the tolerance
    double tolerance() const;

    //! Sets the tolerance
    void setTolerance( double tolerance );

    //! Returns the type of units
    QgsTolerance::UnitType units() const;

    //! Sets the type of units
    void setUnits( QgsTolerance::UnitType units );

    //! Returns if the snapping on intersection is enabled
    bool intersectionSnapping() const;

    //! Sets if the snapping on intersection is enabled
    void setIntersectionSnapping( bool enabled );

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
    SnappingMode mMode = ActiveLayer;
    SnappingType mType = Vertex;
    double mTolerance = 0.0;
    QgsTolerance::UnitType mUnits = QgsTolerance::ProjectUnits;
    bool mIntersectionSnapping = false;

    QHash<QgsVectorLayer *, IndividualLayerSettings> mIndividualLayerSettings;

};

#endif // QGSPROJECTSNAPPINGSETTINGS_H
