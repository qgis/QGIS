#ifndef GEOS_C_EXTRAS_H_INCLUDED
#define GEOS_C_EXTRAS_H_INCLUDED

#include <geos_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef GEOSPrecisionModel
  typedef struct GEOSPrecisionModel_t GEOSPrecisionModel;
  typedef enum { GEOS_PRECISION_FIXED,
                 GEOS_PRECISION_FLOATING,
                 GEOS_PRECISION_FLOATING_SINGLE
               } GEOSPrecisionModelType;
  typedef struct GEOSGeometryPrecisionReducer_t GEOSGeometryPrecisionReducer;
#endif



  CORE_EXPORT GEOSPrecisionModel* GEOSPrecisionModel_create( GEOSPrecisionModelType type );

  CORE_EXPORT GEOSPrecisionModel* GEOSPrecisionModel_createFixed( double scale );

  CORE_EXPORT void GEOSPrecisionModel_destroy( GEOSPrecisionModel* model );


// Does not take ownership of model, model needs to stay valid
  CORE_EXPORT GEOSGeometryPrecisionReducer* GEOSGeometryPrecisionReducer_create( GEOSPrecisionModel* model );

  CORE_EXPORT GEOSGeometry* GEOSGeometryPrecisionReducer_reduce( GEOSGeometryPrecisionReducer* reducer, const GEOSGeometry* geometry );

  CORE_EXPORT void GEOSGeometryPrecisionReducer_destroy( GEOSGeometryPrecisionReducer* reducer );

#ifdef __cplusplus
}
#endif

#endif // GEOS_C_EXTRAS_H_INCLUDED
