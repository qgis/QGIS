/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscrscache.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsCoordinateTransformCache
{
%Docstring
 Cache coordinate transform by authid of source/dest transformation to avoid the
overhead of initialization for each redraw*
%End

%TypeHeaderCode
#include "qgscrscache.h"
%End
  public:
    static QgsCoordinateTransformCache *instance();
%Docstring
 :rtype: QgsCoordinateTransformCache
%End


    QgsCoordinateTransform transform( const QString &srcAuthId, const QString &destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );
%Docstring
 Returns coordinate transformation. Cache keeps ownership
\param srcAuthId auth id string of source crs
\param destAuthId auth id string of dest crs
\param srcDatumTransform id of source's datum transform
\param destDatumTransform id of destinations's datum transform
:return: matching transform, or an invalid transform if none could be created
 :rtype: QgsCoordinateTransform
%End

    void invalidateCrs( const QString &crsAuthId );
%Docstring
Removes transformations where a changed crs is involved from the cache
%End

  private:
    QgsCoordinateTransformCache( const QgsCoordinateTransformCache &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscrscache.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
