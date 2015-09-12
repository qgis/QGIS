#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/index/strtree/STRtree.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/precision/GeometryPrecisionReducer.h>
#include <geos/util/GEOSException.h>

#define GEOSGeometry geos::geom::Geometry
#define GEOSPreparedGeometry geos::geom::prep::PreparedGeometry
#define GEOSSTRtree geos::index::strtree::STRtree
#define GEOSCoordSequence geos::geom::CoordinateSequence
#define GEOSPrecisionModel geos::geom::PrecisionModel
#define GEOSPrecisionModelType geos::geom::PrecisionModel::Type
#define GEOSGeometryPrecisionReducer geos::precision::GeometryPrecisionReducer
typedef struct GEOSBufParams_t GEOSBufferParams;

#include "geos_c_extra.h"

extern "C"
{

  GEOSPrecisionModel* GEOSPrecisionModel_create( GEOSPrecisionModelType type )
  {
    return new GEOSPrecisionModel( type );
  }

  GEOSPrecisionModel* GEOSPrecisionModel_createFixed( double scale )
  {
    return new GEOSPrecisionModel( scale );
  }

  void GEOSPrecisionModel_destroy( GEOSPrecisionModel* model )
  {
    delete model;
  }

  GEOSGeometryPrecisionReducer* GEOSGeometryPrecisionReducer_create( GEOSPrecisionModel* model )
  {
    return new GEOSGeometryPrecisionReducer( *model );
  }

  GEOSGeometry* GEOSGeometryPrecisionReducer_reduce( GEOSGeometryPrecisionReducer* reducer, const GEOSGeometry* geometry )
  {
    try
    {
      return reducer->reduce( *geometry ).release();
    }
    catch ( const geos::util::GEOSException& )
    {
      return 0;
    }
  }

  void GEOSGeometryPrecisionReducer_destroy( GEOSGeometryPrecisionReducer* reducer )
  {
    delete reducer;
  }

}
