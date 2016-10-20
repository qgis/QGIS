/** \ingroup core
 * This is a container for configuration of the snapping of the project
 * @note added in 3.0
 */
class QgsSnappingConfig
{
%TypeHeaderCode
#include <qgssnappingconfig.h>
%End
  public:
    /**
     * SnappingMode defines on which layer the snapping is performed
     */
    enum SnappingMode
    {
      ActiveLayer, /*!< on the active layer */
      AllLayers, /*!< on all vector layers */
      AdvancedConfiguration, /*!< on a per layer configuration basis */
    };

    /**
     * SnappingType defines on what object the snapping is performed
     */
    enum SnappingType
    {
      Vertex, /*!< on vertices only */
      VertexAndSegment, /*!< both on vertices and segments */
      Segment, /*!< on segments only */
    };

    /** \ingroup core
     * This is a container of advanced configuration (per layer) of the snapping of the project
     * @note added in 3.0
     */
    class IndividualLayerSettings
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
        //bool operator!= ( const QgsSnappingConfig::IndividualLayerSettings& other ) const;

        //bool operator== ( const QgsSnappingConfig::IndividualLayerSettings& other ) const;
    };

    /**
     * Constructor with default parameters defined in global settings
     */
    explicit QgsSnappingConfig();

    ~QgsSnappingConfig();

    //! reset to default values
    void reset();

    //! return if snaping is enbaled
    bool enabled() const;

    //! enables the snapping
    void setEnabled( bool enabled );

    //! return the mode (all layers, active layer, per layer settings)
    QgsSnappingConfig::SnappingMode mode() const;

    //! define the mode of snapping
    void setMode( QgsSnappingConfig::SnappingMode mode );

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

    //! return if the snapping on intersection is enabled
    bool intersectionSnapping() const;

    //! set if the snapping on intersection is enabled
    void setIntersectionSnapping( bool enabled );

    //! return individual snapping settings for all layers
    SIP_PYDICT individualLayerSettings() const;
%MethodCode
    // Create the dictionary.
    PyObject* d = PyDict_New();
    if (!d)
        return nullptr;
    // Set the dictionary elements.
    QHash<QgsVectorLayer*, QgsSnappingConfig::IndividualLayerSettings> container = sipCpp->individualLayerSettings();
    QHash<QgsVectorLayer*, QgsSnappingConfig::IndividualLayerSettings>::const_iterator i = container.constBegin();
    while ( i != container.constEnd() )
    {
      QgsVectorLayer* vl = i.key();
      QgsSnappingConfig::IndividualLayerSettings* ils = new QgsSnappingConfig::IndividualLayerSettings( i.value() );

      PyObject* vlobj = sipConvertFromType( vl, sipType_QgsVectorLayer, nullptr );
      PyObject* ilsobj = sipConvertFromType( ils, sipType_QgsSnappingConfig_IndividualLayerSettings, Py_None );

      if ( !vlobj || !ilsobj || PyDict_SetItem( d, vlobj, ilsobj) < 0)
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
        PyErr_SetString(PyExc_StopIteration,"");
      }
      Py_DECREF( vlobj );
      Py_DECREF( ilsobj );
      ++i;
    }
    sipRes = d;
%End

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

};
