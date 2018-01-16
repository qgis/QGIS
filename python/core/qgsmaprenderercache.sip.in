/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprenderercache.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapRendererCache : QObject
{
%Docstring
This class is responsible for keeping cache of rendered images resulting from
a map rendering job.

Once a job has a rendered image stored in the cache (using setCacheImage(...)),
the cache listens to repaintRequested() signals from dependent layers.
If triggered, the cache removes the rendered image (and disconnects from the
layers).

The class is thread-safe (multiple classes can access the same instance safely).

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsmaprenderercache.h"
%End
  public:

    QgsMapRendererCache();

    void clear();
%Docstring
Invalidates the cache contents, clearing all cached images.

.. seealso:: :py:func:`clearCacheImage`
%End

    bool init( const QgsRectangle &extent, double scale );
%Docstring
Initialize cache: set new parameters and clears the cache if any
parameters have changed since last initialization.

:return: flag whether the parameters are the same as last time
%End

    void setCacheImage( const QString &cacheKey, const QImage &image, const QList< QgsMapLayer * > &dependentLayers = QList< QgsMapLayer * >() );
%Docstring
Set the cached ``image`` for a particular ``cacheKey``. The ``cacheKey`` usually
matches the QgsMapLayer.id() which the image is a render of.
A list of ``dependentLayers`` should be passed containing all layer
on which this cache image is dependent. If any of these layers triggers a
repaint then the cache image will be cleared.

.. seealso:: :py:func:`cacheImage`
%End

    bool hasCacheImage( const QString &cacheKey ) const;
%Docstring
Returns true if the cache contains an image with the specified ``cacheKey``.

.. versionadded:: 3.0

.. seealso:: :py:func:`cacheImage`
%End

    QImage cacheImage( const QString &cacheKey ) const;
%Docstring
Returns the cached image for the specified ``cacheKey``. The ``cacheKey`` usually
matches the QgsMapLayer.id() which the image is a render of.
Returns a null image if it is not cached.

.. seealso:: :py:func:`setCacheImage`

.. seealso:: :py:func:`hasCacheImage`
%End

    QList< QgsMapLayer * > dependentLayers( const QString &cacheKey ) const;
%Docstring
Returns a list of map layers on which an image in the cache depends.

.. versionadded:: 3.0
%End

    void clearCacheImage( const QString &cacheKey );
%Docstring
Removes an image from the cache with matching ``cacheKey``.

.. seealso:: :py:func:`clear`
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprenderercache.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
