/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgssnappingconfig.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSnappingConfig
{
%Docstring
This is a container for configuration of the snapping of the project

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssnappingconfig.h"
%End
  public:
    static const QMetaObject staticMetaObject;

  public:

    enum SnappingMode
    {
      ActiveLayer,
      AllLayers,
      AdvancedConfiguration,
    };

    enum SnappingType
    {
      Vertex,
      VertexAndSegment,
      Segment,
    };

    class IndividualLayerSettings
{
%Docstring
This is a container of advanced configuration (per layer) of the snapping of the project

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssnappingconfig.h"
%End
      public:

        IndividualLayerSettings( bool enabled, QgsSnappingConfig::SnappingType type, double tolerance, QgsTolerance::UnitType units );
%Docstring
IndividualLayerSettings

:param enabled:
:param type:
:param tolerance:
:param units:
%End

        IndividualLayerSettings();
%Docstring
Constructs an invalid setting
%End

        bool valid() const;
%Docstring
Returns if settings are valid
%End

        bool enabled() const;
%Docstring
Returns if snapping is enabled
%End

        void setEnabled( bool enabled );
%Docstring
enables the snapping
%End

        QgsSnappingConfig::SnappingType type() const;
%Docstring
Returns the type (vertices and/or segments)
%End

        void setType( QgsSnappingConfig::SnappingType type );
%Docstring
define the type of snapping
%End

        double tolerance() const;
%Docstring
Returns the tolerance
%End

        void setTolerance( double tolerance );
%Docstring
Sets the tolerance
%End

        QgsTolerance::UnitType units() const;
%Docstring
Returns the type of units
%End

        void setUnits( QgsTolerance::UnitType units );
%Docstring
Sets the type of units
%End

        bool operator!= ( const QgsSnappingConfig::IndividualLayerSettings &other ) const;

        bool operator== ( const QgsSnappingConfig::IndividualLayerSettings &other ) const;

    };

    explicit QgsSnappingConfig( QgsProject *project = 0 );
%Docstring
Constructor with default parameters defined in global settings
%End

    bool operator==( const QgsSnappingConfig &other ) const;

    void reset();
%Docstring
reset to default values
%End

    bool enabled() const;
%Docstring
Returns if snapping is enabled
%End

    void setEnabled( bool enabled );
%Docstring
enables the snapping
%End

    SnappingMode mode() const;
%Docstring
Returns the mode (all layers, active layer, per layer settings)
%End

    void setMode( SnappingMode mode );
%Docstring
define the mode of snapping
%End

    SnappingType type() const;
%Docstring
Returns the type (vertices and/or segments)
%End

    void setType( SnappingType type );
%Docstring
define the type of snapping
%End

    double tolerance() const;
%Docstring
Returns the tolerance
%End

    void setTolerance( double tolerance );
%Docstring
Sets the tolerance
%End

    QgsTolerance::UnitType units() const;
%Docstring
Returns the type of units
%End

    void setUnits( QgsTolerance::UnitType units );
%Docstring
Sets the type of units
%End

    bool intersectionSnapping() const;
%Docstring
Returns if the snapping on intersection is enabled
%End

    void setIntersectionSnapping( bool enabled );
%Docstring
Sets if the snapping on intersection is enabled
%End

    SIP_PYDICT individualLayerSettings() const;
%Docstring
Returns individual snapping settings for all layers
%End
%MethodCode
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
%End

    QgsSnappingConfig::IndividualLayerSettings individualLayerSettings( QgsVectorLayer *vl ) const;
%Docstring
Returns individual layer snappings settings (applied if mode is AdvancedConfiguration)
%End

    void setIndividualLayerSettings( QgsVectorLayer *vl, const QgsSnappingConfig::IndividualLayerSettings &individualLayerSettings );
%Docstring
Sets individual layer snappings settings (applied if mode is AdvancedConfiguration)
%End

    bool operator!= ( const QgsSnappingConfig &other ) const;

    void readProject( const QDomDocument &doc );
%Docstring
Reads the configuration from the specified QGIS project document.

.. versionadded:: 3.0
%End

    void writeProject( QDomDocument &doc );
%Docstring
Writes the configuration to the specified QGIS project document.

.. versionadded:: 3.0
%End

    bool addLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Adds the specified layers as individual layers to the configuration
with standard configuration.
When implementing a long-living QgsSnappingConfig (like the one in QgsProject)
it is best to directly feed this with information from the layer registry.

:return: ``True`` if changes have been done.

.. versionadded:: 3.0
%End


    bool removeLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Removes the specified layers from the individual layer configuration.
When implementing a long-living QgsSnappingConfig (like the one in QgsProject)
it is best to directly feed this with information from the layer registry.

:return: ``True`` if changes have been done.

.. versionadded:: 3.0
%End

    QgsProject *project() const;
%Docstring
The project from which the snapped layers should be retrieved

.. versionadded:: 3.0
%End

    void setProject( QgsProject *project );
%Docstring
The project from which the snapped layers should be retrieved

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgssnappingconfig.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
