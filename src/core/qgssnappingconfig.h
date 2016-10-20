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

#include "qgstolerance.h"

class QDomDocument;
class QgsVectorLayer;


/** \ingroup core
 * This is a container for configuration of the snapping of the project
 * @note added in 3.0
 */
class CORE_EXPORT QgsSnappingConfig
{
  public:
    /**
     * SnappingMode defines on which layer the snapping is performed
     */
    enum SnappingMode
    {
      ActiveLayer = 1, /*!< on the active layer */
      AllLayers = 2, /*!< on all vector layers */
      AdvancedConfiguration = 3, /*!< on a per layer configuration basis */
    };

    /**
     * SnappingType defines on what object the snapping is performed
     */
    enum SnappingType
    {
      Vertex = 1, /*!< on vertices only */
      VertexAndSegment = 2, /*!< both on vertices and segments */
      Segment = 3, /*!< on segments only */
    };

    /** \ingroup core
     * This is a container of advanced configuration (per layer) of the snapping of the project
     * @note added in 3.0
     */
    class CORE_EXPORT IndividualLayerSettings
    {
      public:
        /**
         * @brief IndividualLayerSettings
         * @param enabled
         * @param type
         * @param tolerance
         * @param units
         */
        IndividualLayerSettings( bool enabled, QgsSnappingConfig::SnappingType type, double tolerance, QgsTolerance::UnitType units );

        /**
         * Constructs an invalid setting
         */
        IndividualLayerSettings();

        //! return if settings are valid
        bool valid() const;

        //! return if snaping is enbaled
        bool enabled() const;

        //! enables the snapping
        void setEnabled( bool enabled );

        //! return the type (vertices and/or segments)
        QgsSnappingConfig::SnappingType type() const;

        //! define the type of snapping
        void setType( QgsSnappingConfig::SnappingType type );

        //! return the tolerance
        double tolerance() const;

        //! set the tolerance
        void setTolerance( double tolerance );

        //! return the type of units
        QgsTolerance::UnitType units() const;

        //! set the type of units
        void setUnits( QgsTolerance::UnitType units );

        /**
         * Compare this configuration to other.
         */
        bool operator!= ( const IndividualLayerSettings& other ) const;

        bool operator== ( const IndividualLayerSettings& other ) const;

      private:
        bool mValid;
        bool mEnabled;
        SnappingType mType;
        double mTolerance;
        QgsTolerance::UnitType mUnits;
    };

    /**
     * Constructor with default parameters defined in global settings
     */
    explicit QgsSnappingConfig();

    ~QgsSnappingConfig();

    bool operator==( const QgsSnappingConfig& other ) const;

    //! reset to default values
    void reset();

    //! return if snapping is enbaled
    bool enabled() const;

    //! enables the snapping
    void setEnabled( bool enabled );

    //! return the mode (all layers, active layer, per layer settings)
    SnappingMode mode() const;

    //! define the mode of snapping
    void setMode( SnappingMode mode );

    //! return the type (vertices and/or segments)
    SnappingType type() const;

    //! define the type of snapping
    void setType( SnappingType type );

    //! return the tolerance
    double tolerance() const;

    //! set the tolerance
    void setTolerance( double tolerance );

    //! return the type of units
    QgsTolerance::UnitType units() const;

    //! set the type of units
    void setUnits( QgsTolerance::UnitType units );

    //! return if the snapping on intersection is enabled
    bool intersectionSnapping() const;

    //! set if the snapping on intersection is enabled
    void setIntersectionSnapping( bool enabled );

    //! return individual snapping settings for all layers
    QHash<QgsVectorLayer*, QgsSnappingConfig::IndividualLayerSettings> individualLayerSettings() const;

    //! return individual layer snappings settings (applied if mode is AdvancedConfiguration)
    QgsSnappingConfig::IndividualLayerSettings individualLayerSettings( QgsVectorLayer* vl ) const;

    //! set individual layer snappings settings (applied if mode is AdvancedConfiguration)
    void setIndividualLayerSettings( QgsVectorLayer* vl, QgsSnappingConfig::IndividualLayerSettings individualLayerSettings );

    /**
     * Compare this configuration to other.
     */
    bool operator!= ( const QgsSnappingConfig& other ) const;

  public:
    /**
     * Reads the configuration from the specified QGIS project document.
     *
     * @note Added in QGIS 3.0
     */
    void readProject( const QDomDocument& doc );

    /**
     * Writes the configuration to the specified QGIS project document.
     *
     * @note Added in QGIS 3.0
     */
    void writeProject( QDomDocument& doc );

    /**
     * Adds the specified layers as individual layers to the configuration
     * with standard configuration.
     * When implementing a long-living QgsSnappingConfig (like the one in QgsProject)
     * it is best to directly feed this with information from the layer registry.
     *
     * @return True if changes have been done.
     *
     * @note Added in QGIS 3.0
     */
    bool addLayers( const QList<QgsMapLayer*>& layers );


    /**
     * Removes the specified layers from the individual layer configuration.
     * When implementing a long-living QgsSnappingConfig (like the one in QgsProject)
     * it is best to directly feed this with information from the layer registry.
     *
     * @return True if changes have been done.
     *
     * @note Added in QGIS 3.0
     */
    bool removeLayers( const QList<QgsMapLayer*>& layers );

  private:
    void readLegacySettings();

    bool mEnabled;
    SnappingMode mMode;
    SnappingType mType;
    double mTolerance;
    QgsTolerance::UnitType mUnits;
    bool mIntersectionSnapping;

    QHash<QgsVectorLayer*, IndividualLayerSettings> mIndividualLayerSettings;

};

#endif // QGSPROJECTSNAPPINGSETTINGS_H
