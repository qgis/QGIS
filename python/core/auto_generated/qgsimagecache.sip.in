/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsimagecache.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsImageCache : QgsAbstractContentCacheBase
{
%Docstring
A cache for images derived from raster files.

QgsImageCache stores pre-rendered resampled versions of raster image files, allowing efficient
reuse without incurring the cost of resampling on every render.

QgsImageCache is not usually directly created, but rather accessed through
:py:func:`QgsApplication.imageCache()`

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsimagecache.h"
%End
  public:

    QgsImageCache( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsImageCache, with the specified ``parent`` object.
%End

    QImage pathAsImage( const QString &path, const QSize size, const bool keepAspectRatio, const double opacity, bool &fitsInCache /Out/ );
%Docstring
Returns the specified ``path`` rendered as an image. If possible, a pre-existing cached
version of the image will be used. If not, the image is fetched and resampled to the desired
size, and then the result cached for subsequent lookups.

``path`` may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).

The ``size`` parameter dictates the target size of the image. An invalid size indicates the
original raster image size (with no resampling). A size in which the width or height is
set to zero will have the zeroed value automatically computed when keepAspectRatio is ``True``.

If ``keepAspectRatio`` is ``True``, then the original raster aspect ratio will be maintained during
any resampling operations.

An ``opacity`` parameter dictates the opacity of the image.

If the resultant raster was of a sufficiently small size to store in the cache, then ``fitsInCache``
will be set to ``True``.
%End

    QSize originalSize( const QString &path ) const;
%Docstring
Returns the original size (in pixels) of the image at the specified ``path``.

``path`` may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).

If ``path`` is a remote file, then an invalid size may be returned while the image is in the process
of being fetched.

If the image could not be read then an invalid QSize is returned.
%End

  signals:

    void remoteImageFetched( const QString &url );
%Docstring
Emitted when the cache has finished retrieving an image file from a remote ``url``.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsimagecache.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
