class QgsMapLayerDependency
{
%TypeHeaderCode
#include "qgsmaplayerdependency.h"
%End
 public:
  //! Type of dependency
  enum Type
  {
    PresenceDependency = 1, // The layer must be already present (in the registry) for this dependency to be resolved
    DataDependency     = 2  // The layer may be invalidated by data changes on another layer
  };

  //! Origin of the dependency
  enum Origin
  {
    FromProvider = 0,  // Dependency given by the provider, the user cannot change it
    FromUser     = 1   // Dependency given by the user
  };

  //! Standard constructor
  QgsMapLayerDependency( QString layerId, Type type = DataDependency, Origin origin = FromUser );

  //! Return the dependency type
  Type type() const;

  //! Return the dependency origin
  Origin origin() const;

  //! Return the ID of the layer this dependency depends on
  QString layerId() const;

  //! Comparison operator
  bool operator==( const QgsMapLayerDependency& other ) const;

  //! hash operator
  long __hash__() const;
%MethodCode
  sipRes = qHash( *sipCpp );
%End
};


