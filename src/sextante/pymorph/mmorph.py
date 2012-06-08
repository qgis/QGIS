"""
==============
Module pymorph
==============

pymorph a powerful collection of state-of-the-art gray-scale morphological
tools that can be applied to image segmentation, non-linear filtering,
pattern recognition and image analysis.

- `add4dilate()`   : Addition for dilation
- `addm()`         : Addition of two images, with saturation.
- `areaclose()`    : Area closing
- `areaopen()`     : Area opening
- `asf()`          : Alternating Sequential Filtering
- `asfrec()`       : Reconstructive Alternating Sequential Filtering
- `bench()`        : benchmarking main functions of the toolbox.
- `binary()`       : Convert a gray-scale image into a binary image
- `blob()`         : Blob measurements from a labeled image.
- `bshow()`        : Generate a graphical representation of overlaid binary images.
- `cbisector()`    : N-Conditional bisector.
- `cdilate()`      : Dilate an image conditionally.
- `center()`       : Center filter.
- `cerode()`       : Erode an image conditionally.
- `close_holes()`  : Close holes of binary and gray-scale images.
- `close()`        : Morphological closing.
- `closerec()`     : Closing by reconstruction.
- `closerecth()`   : Close-by-Reconstruction Top-Hat.
- `closeth()`      : Closing Top Hat.
- `concat()`       : Concatenate two or more images along width, height or depth.
- `cthick()`       : Image transformation by conditional thickening.
- `cthin()`        : Image transformation by conditional thinning.
- `cwatershed()`   : Detection of watershed from markers.
- `datatype()`     : Return the image datatype string
- `dilate()`       : Dilate an image by a structuring element.
- `dist()`         : Distance transform.
- `drawv()`        : Superpose points, rectangles and lines on an image.
- `edgeoff()`      : Eliminate the objects that hit the image frame.
- `endpoints()`    : Interval to detect end-points.
- `erode()`        : Erode an image by a structuring element.
- `flood()`        : Flooding filter- h,v,a-basin and dynamics (depth, area, volume)
- `frame()`        : Create a frame image.
- `freedom()`      : Control automatic data type conversion.
- `gdist()`        : Geodesic Distance Transform.
- `gradm()`        : Morphological gradient.
- `grain()`        : Gray-scale statistics for each labeled region.
- `gray()`         : Convert a binary image into a gray-scale image.
- `gshow()`        : Apply binary overlays as color layers on a binary or gray-scale image
- `histogram()`    : Find the histogram of the image f.
- `hmax()`         : Remove peaks with contrast less than h.
- `hmin()`         : Remove basins with contrast less than h.
- `homothick()`    : Interval for homotopic thickening.
- `homothin()`     : Interval for homotopic thinning.
- `img2se()`       : Create a structuring element from a pair of images.
- `infcanon()`     : Intersection of inf-generating operators.
- `infgen()`       : Inf-generating.
- `infrec()`       : Inf-reconstruction.
- `inpos()`        : Minima imposition.
- `interot()`      : Rotate an interval
- `intersec()`     : Intersection of images.
- `intershow()`    : Visualize an interval.
- `isbinary()`     : Check for binary image
- `isolines()`     : Apply an iso-line color table to a gray-scale image.
- `label()`        : Label a binary image.
- `labelflat()`    : Label the flat zones of gray-scale images.
- `lastero()`      : Last erosion.
- `lblshow()`      : Display a labeled image assigning a random color for each label.
- `limits()`       : Get the possible minimum and maximum of an image.
- `mat2set()`      : Converts image representation from matrix to set
- `maxleveltype()` : Returns the maximum value associated to an image datatype
- `neg()`          : Negate an image.
- `open()`         : Morphological opening.
- `openrec()`      : Opening by reconstruction.
- `openrecth()`    : Open-by-Reconstruction Top-Hat.
- `openth()`       : Opening Top Hat.
- `opentransf()`   : Open transform.
- `pad4n()`        : pad4n
- `patspec()`      : Pattern spectrum (also known as granulometric size density).
- `plot()`         : Plot a function.
- `randomcolor()`  : Apply a random color table to a gray-scale image.
- `regmax()`       : Regional Maximum.
- `regmin()`       : Regional Minimum (with generalized dynamics).
- `se2hmt()`       : Create a Hit-or-Miss Template (or interval) from a pair of structuring elements.
- `se2interval()`  : Create an interval from a pair of structuring elements.
- `sebox()`        : Create a box structuring element.
- `secross()`      : Diamond structuring element and elementary 3x3 cross.
- `sedilate()`     : Dilate one structuring element by another
- `sedisk()`       : Create a disk or a semi-sphere structuring element.
- `seline()`       : Create a line structuring element.
- `sereflect()`    : Reflect a structuring element
- `serot()`        : Rotate a structuring element.
- `seshow()`       : Display a structuring element as an image.
- `sesum()`        : N-1 iterative Minkowski additions
- `set2mat()`      : Converts image representation from set to matrix
- `setrans()`      : Translate a structuring element
- `seunion()`      : Union of structuring elements
- `skelm()`        : Morphological skeleton (Medial Axis Transform).
- `skelmrec()`     : Morphological skeleton reconstruction (Inverse Medial Axis Transform).
- `skiz()`         : Skeleton of Influence Zone - also know as Generalized Voronoi Diagram
- `subm()`         : Subtraction of two images, with saturation.
- `supcanon()`     : Union of sup-generating or hit-miss operators.
- `supgen()`       : Sup-generating (hit-miss).
- `suprec()`       : Sup-reconstruction.
- `symdiff()`       : Symmetric difference between two images
- `text()`         : Create a binary image of a text.
- `thick()`        : Image transformation by thickening.
- `thin()`         : Image transformation by thinning.
- `threshad()`     : Threshold (adaptive)
- `toggle()`       : Image contrast enhancement or classification by the toggle operator.
- `union()`        : Union of images.
- `watershed()`    : Watershed detection.
- `to_int32()`     : Convert an image to an int32 image.
- `to_uint16()`    : Convert an image to a uint16 image.
- `to_uint8()`     : Convert an image to an uint8 image.

"""
from __future__ import division
from pymorph_version import __version__, __version_info__

import sys, os
from PyQt4.uic.Compiler.qtproxies import QtGui

mydir = os.path.dirname(__file__)
try:
    sys.imagepath += [os.path.join(mydir, 'data')]
except:
    sys.imagepath = [os.path.join(mydir, 'data')]

__all__ = \
    ['concat'
    ,'limits'
    ,'center'
    ,'close_holes'
    ,'dist'
    ,'cdist'
    ,'edgeoff'
    ,'frame'
    ,'randomcolor'
    ,'isolines'
    ,'overlay'
    ,'histogram'
    ,'label'
    ,'neg'
    ,'threshad'
    ,'toggle'
    ,'addm'
    ,'areaclose'
    ,'areaopen'
    ,'asf'
    ,'asfrec'
    ,'binary'
    ,'blob'
    ,'cbisector'
    ,'cdilate'
    ,'cerode'
    ,'close'
    ,'closerec'
    ,'closerecth'
    ,'closeth'
    ,'cthick'
    ,'cthin'
    ,'cwatershed'
    ,'dilate'
    ,'drawv'
    ,'endpoints'
    ,'erode'
    ,'gdist'
    ,'gradm'
    ,'grain'
    ,'gray'
    ,'hmin'
    ,'hmax'
    ,'homothick'
    ,'homothin'
    ,'img2se'
    ,'infcanon'
    ,'infgen'
    ,'infrec'
    ,'inpos'
    ,'interot'
    ,'intersec'
    ,'intershow'
    ,'isbinary'
    ,'asbinary'
    ,'isequal'
    ,'labelflat'
    ,'lastero'
    ,'open'
    ,'openrec'
    ,'openrecth'
    ,'openth'
    ,'opentransf'
    ,'patspec'
    ,'regmax'
    ,'regmin'
    ,'se2interval'
    ,'se2hmt'
    ,'se2flatidx'
    ,'sebox'
    ,'secross'
    ,'sedisk'
    ,'seline'
    ,'serot'
    ,'seshow'
    ,'sesum'
    ,'setrans'
    ,'sereflect'
    ,'sedilate'
    ,'seunion'
    ,'skelm'
    ,'skelmrec'
    ,'skiz'
    ,'subm'
    ,'supcanon'
    ,'supgen'
    ,'suprec'
    ,'bshow'
    ,'symdif'
    ,'symdiff'
    ,'thick'
    ,'thin'
    ,'union'
    ,'watershed'
    ,'maxleveltype'
    ,'to_int32'
    ,'to_uint8'
    ,'to_uint16'
    ,'to_gray'
    ,'datatype'
    ,'add4dilate'
    ,'mat2set'
    ,'set2mat'
    ,'pad4n'
    ]


def concat(dim, *imgs):
    """
    img = concat(dim, img0, img1, ...)

    Concatenate two or more images along width, height or depth.

    Concatenate two or more images in any of the dimensions: width,
    height or depth. If the images do not match the dimension, a
    larger image is create with zero pixels to accommodate them. The
    images must have the same datatype.

    Parameters
    ----------
    dim : {'width', 'heigh', 'depth', 'w', 'h', 'd' }
        Dimension along which to concatenate.
    img0, img1, ... : Images to concatenate

    Returns
    -------
    img : resulting image (of the same type as inputs).

    """
    import numpy as np
    import string
    dimcode = string.lower(dim)[0]
    if not imgs:
        raise ValueError, 'pymorph.concat: received empty image list'
    shapes = np.array([img.shape for img in imgs])
    varies = shapes.var(0).astype(bool)
    if  (dimcode == 'w' and varies[0]) or \
        (dimcode == 'h' and varies[1]) or \
        (dimcode == 'd' and varies.any()):
        max0,max1 = shapes.max(1)
        imgs = list(imgs)
        for (i,img),(s0,s1) in zip(enumerate(imgs), shapes):
            if dimcode == 'w':
                imgs[i] = np.zeros((s0,max1), img.dtype)
            elif dimcode == 'h':
                imgs[i] = np.zeros((max0,s1), img.dtype)
            else:
                imgs[i] = np.zeros((max0,max1), img.dtype)
            imgs[i][:s0,:s1] = img
    if dimcode == 'w':
        return np.hstack(imgs)
    elif dimcode == 'h':
        return np.vstack(imgs)
    elif dimcode == 'd':
        return np.dstack(imgs)
    else:
        raise ValueError, 'pymorph.concat: Unknown direction "%s"' % dim


def limits(f):
    """
    y = limits(f)

    Get the possible minimum and maximum of an image.
    The possible minimum and the possible maximum of an image depend
    on its data type. These values are important to compute many
    morphological operators (for instance, negate of an image). The
    output is a vector, where the first element is the possible
    minimum and the second, the possible maximum.

    Parameters
    ----------
    f : Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.

    Returns
    -------
    y : Two element vector
        the first element is the infimum, the second, the supremum.
    """
    from numpy import array, bool, uint8, uint16, int32, int64
    code = f.dtype
    if code == bool: return array([0,1])
    if code == uint8: return array([0,255])
    if code == uint16: return array([0,65535])
    if code == int32: return array([-2147483647,2147483647])
    if code == int64: return array([-2**63,2**63-1])

    raise ValueError('pymorph.limits: does not accept this typecode: %s' % code)


def center(f, b=None):
    """
    centered = center(f, b={3x3 cross})

    Center filter.

    center() computes the morphological center of f w.r.t. to the
    structuring element b.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    b : Structuring element (default: 3x3 cross).

    Returns
    -------
    y : Image
    """

    if b is None: b = secross()
    while True:
        beta1 = asf(f,'COC',b,1)
        beta2 = asf(f,'OCO',b,1)

        prev = f
        f = union(intersec(y,beta1),beta2)
        if isequal(prev,f):
            return f


def close_holes(f, Bc=None):
    """
    y = close_holes(f, Bc={3x3 cross})

    Close holes of binary and gray-scale images.

    `close_holes` creates the image `y` by closing the holes of the image
    `f`, according with the connectivity defined by the structuring
    element `Bc`.The images can be either binary or gray-scale.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) or binary image.
    Bc : Connectivity (default: 3x3 cross).

    Returns
    -------
    y : image (ndarray) of same type as `f`.
    """

    if Bc is None: Bc = secross()
    delta_f = frame(f)
    return neg(infrec(delta_f, neg(f), Bc))


def dist(f, Bc=None, metric='euclidean'):
    """
    y = dist(f, Bc={3x3 cross}, metric='euclidean')

    Distance transform.

    `dist` creates the distance image y of the binary image `f`. The
    value of `y` at the pixel `x` is the distance of x to the complement
    of f, that is, the distance of `x` to nearest point in the
    complement of `f`. The distances available are based on the
    Euclidean metrics and on metrics generated by a a regular graph,
    that is characterized by a connectivity rule defined by the
    structuring element `Bc`. The implementation of the Euclidean
    algorithm is based on LotuZamp:01 .

    Parameters
    ----------
    f : ndarray of type bool
        Input image
    Bc : ndarray, optional
        Connectivity structure element (default: 3x3 cross)
    metric : {'euclidean' [default], 'euclidean2'}, optional
        Metric to use.

    Returns
    -------
    y : ndarray
        distance image in uint16, or, if metric=='euclidean2', in int32

    See Also
    --------
    cdist : conditional distance transform
    mahotas.distance : in mahotas package, faster C++ implementation
    scipy.ndimage.distance_transform_edt : in scipy, faster C implementation
    """
    return cdist(f=f,g=None,Bc=Bc,metric=metric)

def cdist(f, g=None, Bc=None,metric=None):
    """
    y = cdist(f, g, Bc=None, metric=None)

    Conditional (geodesic) distance transform.

    `cdist` creates the conditional distance image `y` of the binary image `f`.

    y[y,x] = distance of (y,x) to foreground in a path through complement(c)
    y[y,x] = 0, if f[y,x] == 0 or c[y,x] == 1

    The distances available are based on the
    Euclidean metrics and on metrics generated by a a regular graph,
    that is characterized by a connectivity rule defined by the
    structuring element Bc. The implementation of the Euclidean
    algorithm is based on LotuZamp:01 .

    Parameters
    ----------
    f :      Binary image.
    g :      Binary image.
    Bc :     Structuring Element Default: None (3x3 elementary
            cross). (connectivity)
    metric : Metric to use, one of ('euclidean', 'euclidean2'), 'euclidean' by default.

    Returns
    -------
    y : ndarray
        distance image in uint16, or, if metric=='euclidean2', in int32
    """
    from string import lower
    import numpy
    from numpy import zeros, sqrt
    if Bc is None: Bc = secross()
    if metric is not None:
       metric = lower(metric)
    f = gray(f,'uint16')
    y = intersec(f,0)
    if g is not None:
        g = (~g)*255
    if (metric == 'euclidean') or (metric == 'euc2') or (metric == 'euclidean2'):
        i=1
        while not isequal(f,y):
            a4 = -4*i+2
            a2 = -2*i+1
            b = to_int32([[a4,a2,a4],
                          [a2, 0,a2],
                          [a4,a2,a4]])
            y=f
            i+=1
            if g is not None:
                f = union(erode(f,b),g)
            else:
                f = erode(f,b)
        if metric == 'euclidean':
            f = to_uint16(sqrt(f)+0.5)
    else:
        if isequal(Bc, secross()):
            b = to_int32([[-2147483647,  -1, -2147483647],
                          [         -1,   0,          -1],
                          [-2147483647,  -1, -2147483647]])
        elif isequal(Bc, sebox()):
            b = to_int32([[-1,-1,-1],
                          [-1, 0,-1],
                          [-1,-1,-1]])
        else: b = Bc # This seems wrong, but it's the original code
        while not isequal(f,y):
            y=f
            f = union(erode(f,b),g)
    if g is not None:
        return y * (g==0)
    return y


def edgeoff(f, Bc=None):
    """
    y = edgeoff(f, Bc={3x3 cross})

    Eliminate the objects that hit the image frame.

    `edgeoff` creates the binary image `y` by eliminating the objects
    (connected components) of the binary image `f` that hit the image
    frame, according to the connectivity defined by the structuring
    element `Bc`.

    Parameters
    ----------
    f :  Binary image.
    Bc : Structuring element (default: 3x3 cross)

    Returns
    -------
    y : Binary image.
    """

    if Bc is None: Bc = secross()
    edge = frame(f)
    return subm(f, infrec(edge, f, Bc))


def frame(f, Ht=1, Wt=1, Dt=0, framevalue=None, bgvalue=None):
    """
    y = frame(f, Dt=1, Wt=1, Dt=0, framevalue={max of f.dtype}, bgvalue={min of f.dtype})

    Create a frame image.

    `frame creates an image `y`, with the same dimensions (W,H,D)
    and same pixel type of the image `f`, such that the value of the
    pixels in the image frame is `framevalue` and the value of the other
    pixels is k2 . The thickness of the image frame is given by the
    `HT`, `WT`, and `DT` parameters.

    Parameters
    ----------
    f :  Input image
    Ht : Height thickness (default 1)
    Wt : Width thickness (default 1)
    Dt : Depth thickness (default 0)
    framevalue : Frame grey-level (default: max of f.dtype)
    bgvalue : Background grey-level (default: min of f.dtype)

    Returns
    -------
    y : image of same type as f .
    """
    minf, maxf = limits(f)
    if framevalue is None: framevalue = maxf
    if bgvalue is None: bgvalue  = minf
    y = union(intersec(f, minf), bgvalue)
    y[  : , :Wt] = framevalue
    y[  : ,-Wt:] = framevalue
    y[ :Ht,  : ] = framevalue
    y[-Ht:,  : ] = framevalue
    if len(f.shape) == 3:
        y[:,:, :Dt] = framevalue
        y[:,:,-Dt:] = framevalue
    return y


def randomcolor(X):
    """
    y = randomcolor(X)

    Apply a random color table to a gray-scale image.

    Zero is guaranteed to be mapped to black (i.e., (0,0,0) triplet)

    Parameters
    ----------
    X : 2d-array of uint8 of shape (h,w)
        Labeled image.

    Returns
    -------
    y : 3d-array of uint8 of shape (h, w, 3)
        Colour image.
    """
    from numpy import take, reshape, shape, dstack
    from numpy.random import rand

    mmin = X.min()
    mmax = X.max()
    ncolors = mmax - mmin + 1
    R = to_int32(rand(ncolors)*255)
    G = to_int32(rand(ncolors)*255)
    B = to_int32(rand(ncolors)*255)
    if mmin == 0:
       R[0],G[0],B[0] = 0,0,0
    r = reshape(take(R, X.ravel() - mmin),X.shape)
    g = reshape(take(G, X.ravel() - mmin),X.shape)
    b = reshape(take(B, X.ravel() - mmin),X.shape)
    return dstack((r,g,b))


def isolines(X, N=10):
    """
    Y = isolines(X, N=10)

    Apply an iso-contour color table to a gray-scale image.

    Parameters
    ----------
    X : 2d-array
        Distance transform image.
    N : integer, optional
        Number of iso-contours (default: 10).

    Returns
    -------
    Y : Gray-scale (uint8 or uint16) or binary image.
    """
    from numpy import newaxis, ravel, ceil, zeros, ones, transpose, repeat, concatenate, arange, reshape, floor

    def apply_lut(img, lut):
        h,w=img.shape
        return reshape(map(lut.__getitem__, img.ravel()),(h,w,3))
    np = 1  # number of pixels by isoline
    if len(X.shape) == 1: X = X[newaxis,:]
    maxi, mini = X.max(), X.min()
    d = int(ceil(256./N))
    m = zeros(256)
    m[0:256:d] = 1
    m = transpose([m,m,m])
    # lut gray
    gray = floor(arange(N)*255. // (N-1) + 0.5).astype('B')
    gray = repeat(gray, d)[0:256]
    gray = transpose([gray,gray,gray])
    # lut jet
    r = concatenate((range(126,0,-4),zeros(64),range(0,255,4),255*ones(64),range(255,128,-4)))
    g = concatenate((zeros(32),range(0,255,4),255*ones(64),range(255,0,-4),zeros(32)))
    b = 255 - r
    jet = transpose([r,g,b])
    # apply lut
    XX  = floor((X-mini)*255. // maxi + 0.5).astype('B')
    lut = (1-m)*gray + m*jet
    return apply_lut(XX, lut)


def overlay(X, red=None, green=None, blue=None, magenta=None, yellow=None, cyan=None):
    """
    Y = overlay(X, red=None, green=None, blue=None, magenta=None, yellow=None, cyan=None)

    Apply binary overlays as colour layers on a binary or gray-scale image

    Parameters
    ----------
    X :  Gray-scale (uint8 or uint16) or binary image.
    red : Red overlay, binary image(default: None)
    green : Green overlay, binary image (default: None)
    blue : blue overlay, binary image (default: None)
    magenta : magenta overlay, binary image (default: None)
    yellow : yellow overlay, binary image (default: None)
    cyan : cyan overlay, binary image (default: None)

    Returns
    -------
    Y : Colour image (in HxWx3 format)
    """
    from numpy import dstack
    if isbinary(X): X = gray(X,'uint8')
    r = X
    g = X
    b = X
    if red is not None: # red 1 0 0
      red = gray(asbinary(red),'uint8')
      r = union(r,red)
      g = intersec(g,neg(red))
      b = intersec(b,neg(red))
    if green is not None: # green 0 1 0
      green = gray(asbinary(green),'uint8')
      r = intersec(r,neg(green))
      g = union(g,green)
      b = intersec(b,neg(green))
    if blue is not None: # blue 0 0 1
      blue = gray(asbinary(blue),'uint8')
      r = intersec(r,neg(blue))
      g = intersec(g,neg(blue))
      b = union(b,blue)
    if magenta is not None: # magenta 1 0 1
      magenta = gray(magenta,'uint8')
      r = union(r,magenta)
      g = intersec(g,neg(magenta))
      b = union(b,magenta)
    if yellow is not None: # yellow 1 1 0
      #assert isbinary(yellow),'yellow must be binary overlay'
      yellow = gray(asbinary(yellow),'uint8')
      r = union(r,yellow)
      g = union(g,yellow)
      b = intersec(b,neg(yellow))
    if cyan is not None: # cyan 0 1 1
      #assert isbinary(cyan),'cyan must be binary overlay'
      cyan = gray(asbinary(cyan),'uint8')
      r = intersec(r,neg(cyan))
      g = union(g,cyan)
      b = union(b,cyan)
    return dstack((r,g,b))


def histogram(f):
    """
    h = histogram(f)

    Find the histogram of the image f.

    Finds the histogram of the image `f`.

    For binary image the vector size is 2. For gray-scale uint8 and
    uint16 images, the vector size is the maximum pixel value plus
    one. h[0] gives the number of pixels with value 0.

    Parameters
    ----------
    f : Input image (of any integer type).

    Returns
    -------
    h : Histogram in a uint16 or an int32 vector.
    """
    import numpy as np
    return np.histogram(f, np.arange(f.max() + 2))[0]


def label(f, Bc=None):
    """
    y = label(f, Bc={3x3 cross})

    Label a binary image.

    `label` creates the image `y` by labeling the connect components
    of a binary image `f`, according to the connectivity defined by
    the structuring element `Bc`. The background pixels (with value
    0) are not labeled. The maximum label value in the output image
    gives the number of its connected components.

    Parameters
    ----------
    f :  Binary image.
    Bc : Connectivity (default: 3x3 cross)

    Returns
    -------
    y : Image of same size as `f`.
        If number of labels is less than 65535, the data type
        is uint16, otherwise it is int32.
    """
    if Bc is None: Bc = secross()
    if not isbinary(f):
        f = (f > 0)
    f = pad4n(f, Bc, 0)
    neighbours = se2flatidx(f,Bc)
    labeled = f * 0
    f = f.ravel()
    labeledflat=labeled.ravel()
    label = 1
    queue = []
    for i in xrange(f.size):
        if f[i] and labeledflat[i] == 0:
            labeledflat[i]=label
            queue=[i+bi for bi in neighbours]
            while queue:
                ni=queue.pop()
                if f[ni] and labeledflat[ni] == 0:
                    labeledflat[ni]=label
                    for n in neighbours+ni:
                        queue.append(n)
            label += 1
    return labeled[1:-1,1:-1] # revert the pad4n() call above


def neg(f):
    """
    y = neg(f)

    Negate an image.

    `neg` returns an image `y` that is the negation (i.e., inverse or
    involution) of the image `f`. In the binary case, `y` is the
    complement of `f`.

    Parameters
    ----------
    f : Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.

    Returns
    -------
    y : ndarray of same shape and dtype as `f`
    """

    y = limits(f)[0] + limits(f)[1] - f
    return y.astype(f.dtype)


def threshad(f, f1, f2=None):
    """
    y = threshad(f, f1, f2=None)

    Threshold (adaptive)

    `threshad` creates the image y as the threshold of the image f
    by the images `f1` and `f2`. A pixel in `y` has the value 1 when the
    value of the corresponding pixel in `f` is between the values of
    the corresponding pixels in `f1` and `f2`.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) image.
    f1 : Lower value.
    f2 : Upper value.

    Returns
    -------
    y : Binary image.
    """

    if f2 is None:
        return f1 <= f
    return (f1 <= f) & (f <= f2)


def toggle(f, f1, f2, gray_mode=True):
    """
    y = toggle(f, f1, f2, gray_mode=True)

    Toggle operator

    Image contrast enhancement or classification by the toggle operator.

    `toggle` creates the image `y` that is an enhancement or classification of
    the image f by the toggle operator, with parameters `f1` and `f2`. If
    `gray`, it performs an enhancement; otherwise, it performs a binary
    classification.

    In the enhancement, a pixel takes the value of the corresponding pixel in
    `f1` or `f2`, according to a minimum distance criterion from `f` to `f1` or
    `f` to `f2`. In the classification, the pixels in f nearest to `f1` receive
    the value `0`, while the ones nearest to `f2` receive the value `1`.

    Parameters
    ----------
    f :    Gray-scale (uint8 or uint16) image.
    f1 :   Gray-scale (uint8 or uint16) image.
    f2 :   Gray-scale (uint8 or uint16) image.
    gray_mode : boolean, optional
        Whether to work in grey mode (default: True)

    Returns
    -------
    y : ndaimge of same shape and dtype as f
    """

    y = binary(subm(f,f1),subm(f2,f))
    if gray_mode:
        t = gray(y)
        y = union(intersec(neg(t),f1),intersec(t,f2))
    return y


def addm(f1, f2):
    """
    y = addm(f1, f2)

    Addition of two images, with saturation.

    addm creates the image y by pixelwise addition of images f1
    and f2 . When the addition of the values of two pixels saturates
    the image data type considered, the greatest value of this type
    is taken as the result of the addition.

    Parameters
    ----------
    f1 : Unsigned gray-scale (uint8 or uint16), signed (int32) or
        binary image.
    f2 : Unsigned gray-scale (uint8 or uint16), signed (int32) or
        binary image. Or constant.

    Returns
    -------
    y: Unsigned gray-scale (uint8 or uint16), signed (int32) or
       binary image.

    Examples
    --------
    ::

        f = to_uint8([255,   255,    0,   10,    0,   255,   250])
        g = to_uint8([ 0,    40,   80,   140,  250,    10,    30])
        print addm(f,g)
        print addm(g, 100)

    prints out

    ::

        [255 255  80 150 250 255 255]
        [100 140 180 240 255 110 130]
    """
    from numpy import array, minimum, maximum

    if type(f2) is array:
        assert f1.dtype == f2.dtype, 'Cannot have different datatypes:'
    y = maximum(minimum(f1.astype('d')+f2, limits(f1)[1]),limits(f1)[0])
    return y.astype(f1.dtype)


def areaclose(f, a, Bc=None):
    """
    y = areaclose(f, a, Bc={3x3 cross})

    Area closing

    `areaclose` removes any pore (i.e., background connected
    component) with area less than `a` in binary image `f`. The
    connectivity is given by the structuring element `Bc`. This
    operator is generalized to gray-scale images by applying the
    binary operator successively on slices of `f` taken from higher
    threshold levels to lower threshold levels.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) or binary image.
    a :  Non negative integer.
    Bc : Structuring element (default: 3x3 cross).

    Returns
    -------
    y : Same type of f
    """
    if Bc is None: Bc = secross()
    return neg(areaopen(neg(f),a,Bc))


def areaopen(f, a, Bc=None):
    """
    y = areaopen(f, a, Bc=None)

    Area opening

    `areaopen` removes any grain (i.e., connected component) with
    area less than `a` of a binary image `f`. The connectivity is given
    by the structuring element `Bc`. This operator is generalized to
    gray-scale images by applying the binary operator successively
    on slices of `f` taken from higher threshold levels to lower
    threshold levels.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) or binary image.
    a :  Double non negative integer.
    Bc : Structuring element (default: 3x3 cross).

    Returns
    -------
    y : Same type of f

    Examples
    --------

    ::

        f=binary(to_uint8([
         [1, 1, 0, 0, 0, 0, 1],
         [1, 0, 1, 1, 1, 0, 1],
         [0, 0, 0, 0, 1, 0, 0]]))
        y=areaopen(f,4,secross())
        print y

    example 2

    ::

        f=to_uint8([
           [10,   11,   0,    0,   0,   0,  20],
           [10,    0,   5,    8,   9,   0,  15],
           [10,    0,   0,    0,  10,   0,   0]])
        y=areaopen(f,4,secross())
        print y

    ::

        a=readgray('form-1.tif')
        b=areaopen(a,500)
        show(a)
        show(b)

    ::

        a=readgray('bloodcells.tif')
        b=areaopen(a, 500)
        show(a)
        show(b)

    """

    if Bc is None: Bc = secross()
    if isbinary(f):
      fr = label(f,Bc)      # binary area open, use area measurement
      g = blob(fr,'area')
      y = threshad(g,a)
    else:
      y = intersec(f,0)
      zero = binary(y)
      k1 = f.min()
      k2 = f.max()
      for k in xrange(k1,k2+1):   # gray-scale, use thresholding decomposition
        print float((k-k1)*100)/float(k2-k1)
        fk = threshad(f,k)
        fo = areaopen(fk,a,Bc)
        if isequal(fo,zero):
          break
        y = union(y, gray(fo,datatype(f),k))
    return y


def asf(f, seq="OC", B=None, n=1):
    """
    y = asf(f, seq="OC", B={3x3 cross}, n=1)

    Alternating Sequential Filtering

    `asf` creates the image `y` by filtering the image `f` by n
    iterations of the close and open alternating sequential filter
    characterized by the structuring element `B`. The sequence of
    opening and closing is controlled by the parameter `seq`. 'OC'
    performs opening after closing, 'CO' performs closing after
    opening, 'OCO' performs opening after closing after opening, and
    'COC' performs closing after opening after closing.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    seq : Order of operations, one of ('OC', 'CO', 'OCO', 'COC'), default: 'OC'.
    B : Structuring element (default: 3x3  cross).
    n : integer, optional
        Number of iterations (default: 1).

    Returns
    -------
    y : Image
    """
    from string import upper
    if B is None: B = secross()
    seq = upper(seq)
    ops = { 'O' : open, 'C' : close }
    ops = map(ops.get, reversed(seq))

    for i in xrange(n):
        Bn = sesum(B, i+1)
        for op in ops:
            f = op(f, Bn)
    return f


def asfrec(f, seq="OC", B=None, Bc=None, N=1):
    """
    y = asfrec(f, seq="OC", B={3x3 cross}, Bc={3x3 cross}, N=1)

    Reconstructive Alternating Sequential Filtering

    `asf` creates the image `y` by filtering the image `f` by `n`
    iterations of the close by reconstruction and open by
    reconstruction alternating sequential filter characterized by
    the structuring element `B`. The structure element `Bc` is used in
    the reconstruction. The sequence of opening and closing is
    controlled by the parameter `seq`. 'OC' performs opening after
    closing, and 'CO' performs closing after opening.

    Parameters
    ----------
    f :   Gray-scale (uint8 or uint16) or binary image.
    seq : Order of operations, one of ('OC', 'CO'), default: 'OC'.
    B :   Structuring element (default: 3x3 cross).
    Bc :  Structuring element (default: 3x3 cross).
    N :   Number of iterations (default: 1).

    Returns
    -------
    y : Same type of f
    """
    from string import upper
    if B is None: B = secross()
    if Bc is None: Bc = secross()
    seq = upper(seq)
    assert seq in ('OC', 'CO'),'pymorph.asfrec: only accepts OC or CO for SEQ parameter'
    firstop = closerec
    secondop = openrec
    if seq == 'CO':
        firstop, secondop = secondop, firstop
    for i in xrange(N):
        Bn = sesum(B, i+1)
        f = firstop(f, Bn, Bc)
        f = secondop(f, Bn, Bc)
    return f


def binary(f, k=1):
    """
    y = binary(f, k=1)

    Convert a gray-scale image into a binary image

    `binary` converts a gray-scale image `f` into a binary image `y` by
    a threshold rule. A pixel in `y` has the value 1 if and only if
    the corresponding pixel in `f` has a value greater or equal to `k`.

    Parameters
    ----------
    f :  Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.
    k : integer, optional
        Threshold used inclusively (default: 1)

    Returns
    -------
    y : Binary image.
    """
    from numpy import asanyarray
    f = asanyarray(f)
    return (f >= k)


def blob(f, measurement, output="image"):
    """
    y = blob(f, measurement, output="image")

    Blob measurements from a labeled image.

    Take measurements from the labeled image f.

    The measurements are:

    - area,
    - centroid,
    - bounding rectangle.

    `output` controls the output format:

    image
        the result is an image
    data
        the result is a double column vector with the measurement for each
        blob.

    The region with label zero is not measured as it is normally
    the background. The measurement of region with label 1 appears
    at the first row of the output.

    Parameters
    ----------
    f :           Gray-scale (uint8 or uint16) image. Labeled image.
    measurement : Measurement. One of ('area', 'centroid', 'boundingbox').
    output : {'image' [default], 'data'}, optional
            Output format: if 'image', returns a binary image; if 'data',
            returns a vector of measurements

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.
    """
    import numpy
    from numpy import newaxis, ravel, zeros, sum, nonzero, array, asanyarray
    from string import lower

    measurement = lower(measurement)
    output      = lower(output)
    if len(f.shape) == 1: f = f[newaxis,:]
    assert measurement in ('area', 'centroid', 'boundingbox'), 'pymorph.blob: Unknown measurement type \'%s\'' % measurement
    if output == 'data':
        y = []
    elif measurement == 'centroid':
        y = zeros(f.shape,numpy.bool)
    else:
        y = zeros(f.shape,numpy.int32)
    for obj_id in xrange(f.max()):
        blob = (f == (obj_id+1))
        if measurement == 'area':
            area = blob.sum()
            if output == 'data': y.append(area)
            else               : y += area*blob
        elif measurement == 'centroid':
            indy, indx  = nonzero(blob)
            cy = sum(indy) // len(indy)
            cx = sum(indx) // len(indx)
            if output == 'data': y.append( (cy, cx) )
            else               : y[cy, cx] = 1
        elif measurement == 'boundingbox':
            col, = nonzero(blob.any(0))
            row, = nonzero(blob.any(1))
            if output == 'data': y.append([col[0],row[0],col[-1]+1,row[-1]+1])
            else:
                y[row[0]:row[-1], col[0]] = 1
                y[row[0]:row[-1],col[-1]] = 1
                y[row[0], col[0]:col[-1]] = 1
                y[row[-1],col[0]:col[-1]] = 1
    return asanyarray(y)


def cbisector(f, B, n):
    """
    y = cbisector(f, B, n)

    N-Conditional bisector.

    `cbisector` creates the binary image `y` by performing a filtering
    of the morphological skeleton of the binary image `f`, relative
    to the structuring element `B`. The strength of this filtering is
    controlled by the parameter `n`. Particularly, if `n==0` , `y` is the
    morphological skeleton of `f` itself.

    Parameters
    ----------
    f : Binary image.
    B : Structuring element
    n : filtering rate (positive integer)

    Returns
    -------
    y : Binary image.
    """
    y = intersec(f,0)
    for i in xrange(n):
        nb = sesum(B,i)
        nbp = sesum(B,i+1)
        f1 = erode(f,nbp)
        f2 = cdilate(f1,f,B,n)
        f3 = subm(erode(f,nb),f2)
        y  = union(y,f3)
    return y


def cdilate(f, g, Bc=None, n=1):
    """
    y = cdilate(f, g, Bc={3x3 cross}, n=1)

    Conditional dilation

    `cdilate` creates the image `y` by dilating the image `f` by the
    structuring element `Bc` conditionally to the image `g`. This
    operator may be applied recursively `n` times.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    g : Conditioning image. (Gray-scale or binary).
    Bc : Structuring element (default: 3x3 cross)
    n : Number of iterations (default: 1)

    Returns
    -------
    y : Image
    """

    if Bc is None: Bc = secross()
    f = intersec(f,g)
    for i in xrange(n):
        prev = f
        f = intersec(dilate(f, Bc), g)
        if isequal(f, prev): break
    return f


def cerode(f, g, Bc=None, n=1):
    """
    y = cerode(f, g, Bc=None, n=1)

    Conditional erosion

    `cerode` creates the image `y` by eroding the image `f` by the
    structuring element `Bc` conditionally to `g`. This operator may be
    applied recursively `n` times.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    g : Conditioning image.
    Bc : Structuring element (default: 3x3 cross)
    n : Number of iterations (default: 1)

    Returns
    -------
    y : Image
    """

    if Bc is None: Bc = secross()
    f = union(f,g)
    for i in xrange(n):
        prev = f
        f = union(erode(f,Bc),g)
        if isequal(f, prev): break
    return f


def close(f, Bc =None):
    """
    y = close(f, Bc={3x3 cross})

    Morphological closing.

    `close` creates the image `y` by the morphological closing of the
    image `f` by the structuring element `Bc`. In the binary case, the
    closing by a structuring element `Bc` may be interpreted as the
    intersection of all the binary images that contain the image `f`
    and have a hole equal to a translation of `Bc`. In the gray-scale
    case, there is a similar interpretation taking the functions
    umbra.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    Bc : Structuring Element Default: None (3x3 elementary cross).

    Returns
    -------
    y : Image
    """

    if Bc  is None: Bc  = secross()
    return erode(dilate(f,Bc ),Bc )


def closerec(f, Bdil=None, Bc=None):
    """
    y = closerec(f, Bdil=None, Bc=None)

    Closing by reconstruction.

    `closerec()` creates the image `y` by a sup-reconstruction (with
    the connectivity defined by the structuring element `Bc`) of the
    image `f` from its dilation by `Bdil`.

    Parameters
    ----------
    f :    Gray-scale (uint8 or uint16) or binary image
    Bdil : Dilation structuring element (default 3x3 elementary cross)
    Bc :  Connectivity structuring element (default: 3x3 elementary cross)

    Returns
    -------
    y : Image (same type as f)
    """

    if Bdil is None: Bdil = secross()
    if Bc is None: Bc = secross()
    return suprec(dilate(f,Bdil),f,Bc)


def closerecth(f, Bdil=None, Bc=None):
    """
    y = closerecth(f, Bdil=None, Bc=None)

    Close-by-Reconstruction Top-Hat.

    `closerecth` creates the image `y` by subtracting the image `f` of
    its closing by reconstruction, defined by the structuring
    elements `Bc` and `Bdil`.

    Parameters
    ----------
    f :    Gray-scale (uint8 or uint16) or binary image.
    Bdil : Dilation structuring element (default: 3x3 cross)
    Bc :   Connectivity structuring element (default: 3x3 cross)

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.
    """

    if Bdil is None: Bdil = secross()
    if Bc is None: Bc = secross()
    return subm(closerec(f, Bdil, Bc), f)


def closeth(f, B=None):
    """
    y = closeth(f, B={3x3 cross})

    Closing Top Hat.

    `closeth` creates the image `y` by subtracting the image `f` of its
    morphological closing by the structuring element `B`.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    B : Structuring Element Default: None (3x3 elementary cross).

    Returns
    -------
    y : ndarray of same shape and dtype as `f`
    """

    if B is None: B = secross()
    return subm(close(f, B), f)


def cthick(f, g, Iab=None, n=-1, theta=45, direction="clockwise"):
    """
    y = cthick(f, g, Iab={Homothick interval}, n={infinite}, theta=45, direction="clockwise")

    Image transformation by conditional thickening.

    `cthick` creates the binary image `y` by performing a thickening
    of the binary image `f` conditioned to the binary image `g`. The
    number of iterations of the conditional thickening is `n` and in
    each iteration the thickening is characterized by rotations of
    `theta` of the interval `Iab`.

    Parameters
    ----------
    f :         Binary image.
    g :         Binary image.
    Iab :       Interval (default: homothick).
    n :         Number of iterations, -1 for infinite (default: -1)
    theta :     Degrees of rotation, 45 (default), 90, or 180.
    direction: 'clockwise' or 'anti-clockwise'.

    Returns
    -------
    y : Binary image.
    """
    from string import upper

    if Iab is None: Iab = homothick()
    if n == -1: n = f.size
    assert isbinary(f), 'pymorph.cthick: f must be binary image'

    direction = upper(direction)
    for i in xrange(n):
        prev = f
        for t in xrange(0,360,theta):
            sup = supgen(f, interot(Iab, t, direction))
            f = intersec(union(f, sup),g)
        if isequal(prev, f): break
    return f


def cthin(f, g, Iab=None, n=-1, theta=45, direction="clockwise"):
    """
    y = cthin(f, g, Iab={Homothin interval}, n={infinite}, theta=45, direction="clockwise")

    Image transformation by conditional thinning.

    `cthin` creates the binary image y by performing a thinning of
    the binary image `f` conditioned to the binary image `g`. The
    number of iterations of the conditional thinning is `n` and in
    each iteration the thinning is characterized by rotations of
    `theta` of the interval `Iab`.

    Parameters
    ----------
    f :         Binary image.
    g :         Binary image.
    Iab :       Interval (default: homothin()).
    n :         Number of iterations, -1 for infinite (default: -1)
    theta :     Degrees of rotation, 45 (default), 90, or 180.
    direction: 'clockwise' or 'anti-clockwise'.

    Returns
    -------
    y : Binary image.
    """
    from string import upper
    if Iab is None: Iab = homothin()
    assert isbinary(f),'f must be binary image'
    direction = upper(direction)
    if n == -1: n = f.size
    for i in xrange(n):
        prev = f
        for t in xrange(0,360,theta):
            sup = supgen(f, interot(iab, t, direction))
            y = union(subm(f, sup),g)
        if isequal(prev, f): break
    return f


def cwatershed(f, markers, Bc=None, return_lines=False,is_gvoronoi=False):
    """
    R = cwatershed(f, g, Bc=None, return_lines=False)
    R,L = cwatershed(f, g, Bc=None, return_lines=True)

    Detection of watershed from markers.

    `cwatershed` creates the image `R` by detecting the domain of the
    catchment basins of `f` indicated by the marker image `g`,
    according to the connectivity defined by `Bc`.

    If `return_lines`, `L` will be a labeled image of the catchment basins
    domain or just a binary image that presents the watershed lines.
    To know more about watershed and watershed from markers, see
    BeucMeye:93. The implementation of this function is based on
    LotuFalc:00.

    WARNING: There is a common mistake related to the
    marker image `g`. If this image contains only zeros and ones, but
    it is not a binary image, the result will be an image with all
    ones. If the marker image is binary, you have to set this
    explicitly (e.g., `cwatershed(f,g>0)` or `cwatershed(f,g.astype(bool))`)

    Parameters
    ----------
    f :       Gray-scale (uint8 or uint16) image.
    markers : Gray-scale (uint8 or uint16) or binary image. marker
                   image: binary or labeled.
    Bc :      Watershed connectivity (default: 3x3 cross)
    return_lines : Whether to return lines as well as regions (default: False)

    Returns
    -------
    R : Gray-scale (uint8 or uint16) or binary image.
    L : Binary line image

    See Also
    --------
    mahotas.cwatershed : implementation of same interface in C++
    """
    from numpy import ones, zeros, nonzero, array, put, take, argmin, transpose, compress, concatenate, where, uint8
    from heapq import heapify, heappush, heappop
    if Bc is None: Bc = secross()
    if isbinary(markers):
        markers = label(markers,Bc)
    status = pad4n(zeros(f.shape,uint8),Bc, 3)
    f = pad4n(f,Bc,0)                       # pad input image with 0
    y = pad4n(markers,Bc,0)                 # pad marker image with 0
    if return_lines:
        y1 = intersec(binary(y), 0)
        y1flat=y1.ravel()
    costM = limits(f)[1] * ones(f.shape)  # cummulative cost function image
    # get 1D displacement neighborhood pixels
    Bi=se2flatidx(f,Bc)
    status=status.ravel()
    f=f.ravel()
    costM=costM.ravel()
    yflat=y.ravel()

    initial, = where(yflat > 0)
    costM[initial]=f[initial]
    # I sort in order of insertion for the simple reason that that's what
    # the original code intended to do (although that code was not functional)
    hqueue=[(costM[idx],i,idx) for i,idx in enumerate(initial)]
    heapify(hqueue)
    while hqueue:
        cost,_,pi = heappop(hqueue)
        status[pi]=1                                            # make it a permanent label
        for qi in Bi+pi:                                        # for each neighbor of pi
            if (status[qi] != 3):                               # not image border
                if (status[qi] != 1):                           # if not permanent
                    if is_gvoronoi:
                        ncost=costM[pi]
                    else:
                        ncost=f[qi]
                    if ncost < costM[qi]:
                        costM[qi]=ncost
                        yflat[qi] = yflat[pi]                   # propagate the label
                        heappush(hqueue,(ncost,i,qi))
                        i += 1
                elif (return_lines  and
                     (yflat[qi] != yflat[pi]) and
                     (y1flat[qi] == 0)     ):
                    y1flat[pi] = 1
    y=y[1:-1,1:-1]
    if return_lines:
        y1=y1[1:-1,1:-1]
        return y,y1
    return y


def dilate(f, B=None):
    """
    y = dilate(f, B={3x3 cross})

    Dilate an image by a structuring element.

    `dilate` performs the dilation of image `f` by the structuring
    element `B`. Dilation is a neighbourhood operator that compares
    locally `B` with `f`, according to an intersection rule. Since
    Dilation is a fundamental operator to the construction of all
    other morphological operators, it is also called an elementary
    operator of Mathematical Morphology. When f is a gray-scale
    image, `B` may be a flat or non-flat structuring element.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    B : Structuring element (default: 3x3 cross).

    Returns
    -------
    y: Same type as `f`
    """
    from numpy import maximum, newaxis, ones, int32
    if B is None: B = secross()
    if len(f.shape) == 1: f = f[newaxis,:]
    if isbinary(f): B = asbinary(B)
    h,w = f.shape
    x,v = mat2set(B)
    if len(x)==0:
        y = (ones((h,w),int32) * limits(f)[0]).astype(f.dtype)
    else:
        if isbinary(v):
            v = intersec(gray(v,'int32'),0)
        mh,mw = max(abs(x)[:,0]),max(abs(x)[:,1])
        y = (ones((h+2*mh,w+2*mw),int32) * limits(f)[0]).astype(f.dtype)
        for i in xrange(x.shape[0]):
            if v[i] > -2147483647:
                y[mh+x[i,0]:mh+x[i,0]+h, mw+x[i,1]:mw+x[i,1]+w] = maximum(
                    y[mh+x[i,0]:mh+x[i,0]+h, mw+x[i,1]:mw+x[i,1]+w], add4dilate(f,v[i]))
        y = y[mh:mh+h, mw:mw+w]
    return y


def drawv(f, data, value, geometry):
    """
    y = drawv(f, data, value, geometry)

    Superpose points, rectangles and lines on an image.

    `drawv` creates the image `y` by a superposition of points,
    rectangles and lines of gray-level `value` on the image `f`. The
    parameters for each geometrical primitive are defined by each line
    in the `data` parameter. For points, they are represented by 2-D
    coordinates.  For lines, they are drawn with the same convention
    used by points, with a straight line connecting them in the order
    given by the data matrix. For rectangles and filled rectangles,
    data should be an array of 2 x 2D points.

    Parameters
    ----------
    f :        Gray-scale (uint8 or uint16) or binary image.
    data :     Vector of coordinates.
    value :    Gray value(s) for drawing.
                  If it's a single value, then it will be used for all objects,
                  otherwise, it should be an array of same length as `data`.
    geometry :  { 'point', 'line', 'rect', 'frect' (filled rectangles)}

    Returns
    -------
    y : Image of same type as `f`
    """
    """
        - Examples
            #
            #   example 1
            #
            f=to_uint8(zeros((3,5)))
            pcoords=to_uint16([[0,2,4],
                            [0,0,2]])
            pvalue=to_uint16([1,2,3])
            print drawv(f,pcoords,pvalue,'point')
            print drawv(f,pcoords,pvalue,'line')
            rectcoords=to_uint16([[0],
                               [0],
                               [3],
                               [2]])
            print drawv(f,rectcoords, to_uint16(5), 'rect')
            #
            #   example 2
            #
            f=readgray('blob3.tif')
            pc=blob(label(f),'centroid','data')
            lines=drawv(intersec(f,0),transpose(pc),to_uint8(1),'line')
            show(f,lines)
    """
    from numpy import array, newaxis, zeros, put, ravel, arange, floor, int32, zeros_like
    from string import lower
    import itertools

    geometry  = lower(geometry)
    if len(f.shape) == 1: f = f[newaxis,:]
    value     = array(value)
    if value.shape == ():
        value = itertools.repeat(value)
    else:
        assert (len(data) == len(value) or geometry == 'line'), \
                'pymorph.drawv: Length of data does not match length of values'
        assert ((len(data) == len(value)+1) or geometry != 'line'), \
                'pymorph.drawv: Length of data does not match length of values -1 (for line option)'

    res = zeros_like(f)
    if geometry == 'point':
        for (y,x), val in zip(data, value):
            res[y,x] = val
    elif geometry == 'line':
        # Implement algorithm from
        # http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
        # To match the Wikipedia version
        # the coordinates are expressed in *x,y* convention!
        for val,(x0,y0),(x1,y1) in itertools.izip(value,data[:-1], data[1:]):
            steep = (abs(y1 - y0) > abs(x1 - x0))
            if steep:
                y0,x0 = x0,y0
                y1,x1 = x1,y1
            if x0 > x1:
                x0,x1 = x1,x0
                y0,y1 = y1,y0
            deltax = x1 - x0
            deltay = abs(y1 - y0)
            error = deltax // 2
            y = y0
            ystep = (1 if y0 < y1 else -1)
            for x in xrange(x0, x1+1):
                if steep: res[y,x] = val
                else: res[x,y] = val
                error -= deltay
                if error < 0:
                    y += ystep
                    error += deltax
    elif geometry in ('frect', 'rect'):
        for (y0,x0,y1,x1), val in zip(data, value):
            assert y1 >= y0, 'pymorph.drawv: bad arguments'
            assert x1 >= x0, 'pymorph.drawv: bad arguments'
            if geometry == 'rect':
                res[y0:y1+1,   x0  ] = val
                res[y0:y1+1,   x1  ] = val
                res[   y0  ,x0:x1+1] = val
                res[   y1  ,x0:x1+1] = val
            else:
                res[y0:y1+1,x0:x1+1] = val
    else:
        assert False, "pymorph.drawv: geometry should be 'point', 'line', 'rect', or 'frect'."
    return res


def endpoints(option="loop"):
    """
    iab = endpoints(option="loop")

    Interval to detect end-points.

    `endpoints` creates an interval that is useful to detect
    end-points of curves (i.e., one pixel thick connected
    components) in binary images. It can be used to prune skeletons
    and to mark objects transforming them in a single pixel or
    closed loops if they have holes. There are two options
    available: 'loop', deletes all points but preserves loops if used
    in thin ; 'homotopic', deletes all points but preserves the last
    single point or loops.

    Parameters
    ----------
    option : {'loop' [default], 'homotopic'}
        type of interval

    Returns
    -------
    Iab : Interval
    """
    from string import lower
    option = lower(option)
    if option == 'loop':
        return se2hmt(binary([[0,0,0],
                              [0,1,0],
                              [0,0,0]]),

                      binary([[0,0,0],
                              [1,0,1],
                              [1,1,1]]))
    elif option == 'homotopic':
        return se2hmt(binary([[0,1,0],
                              [0,1,0],
                              [0,0,0]]),

                      binary([[0,0,0],
                              [1,0,1],
                              [1,1,1]]))
    from warnings import warn
    warn('pymorph.endpoints: Did not understand argument "%s".' % option)
    return None


def erode(f, b=None):
    """
    y = erode(f, b={3x3 cross})

    Erode an image by a structuring element.

    `erode` performs the erosion of the image `f` by the structuring
    element `b`. Erosion is a neighbourhood operator that compairs
    locally `b` with `f`, according to an inclusion rule. Since erosion
    is a fundamental operator to the construction of all other
    morphological operators, it is called an elementary
    operator of Mathematical Morphology. When `f` is a gray-scale
    image, `b` may be a flat or non-flat structuring element.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    b : Structuring element (Default: 3x3 elementary cross).

    Returns
    -------
    y : Image

    See Also
    --------
    mahotas.erode : implementation of binary erosion
    scipy.ndimage.grey_erosion : implementation of grey erosion
    """

    if b is None: b = secross()
    return neg(dilate(neg(f),sereflect(b)))



def gdist(f, g, Bc=None, metric=None):
    """
    y = gdist(f, g, Bc=None, metric=None)

    Geodesic Distance Transform.

    `gdist` creates the geodesic distance image y of the binary
    image `f` relative to the binary image `g`. The value of `y` at the
    pixel `x` is the length of the smallest path between `x` and `f`. The
    distances available are based on the Euclidean metrics and on
    metrics generated by a neighbourhood graph, that is
    characterized by a connectivity rule defined by the structuring
    element `Bc`. The connectivity for defining the paths is
    consistent with the metrics adopted to measure their length. In
    the case of the Euclidean distance, the space is considered
    continuos and, in the other cases, the connectivity is the one
    defined by `Bc`.

    Parameters
    ----------
    f :      Binary image.
    g :      Binary image. Marker image
    Bc :     Structuring Element Default: None (3x3 elementary
              cross). (metric for distance).
    metric: string default: none. 'euclidean' if specified.

    Returns
    -------
    y : uint16 (distance image).
    """

    if Bc is None: Bc = secross()
    assert metric is None,'Does not support euclidean'
    fneg,gneg = neg(f),neg(g)
    y = gray(gneg,'uint16',1)
    ero = intersec(y,0)
    prev = y
    i = 1
    while not isequal(ero, prev):
        prev = ero
        ero = cerode(gneg,fneg,Bc,i)
        y = addm(y,gray(ero,'uint16',1))
        i += 1
    return union(y,gray(ero,'uint16'))


def gradm(f, Bdil=None, Bero=None):
    """
    y = gradm(f, Bdil={3x3 cross}, Bero={3x3 cross})

    Morphological gradient.

    `gradm` creates the image `y` by the subtraction of the erosion of
    the image `f` by `Bero` of the dilation of `f` by `Bdil`.

    Parameters
    ----------
    f :    Gray-scale (uint8 or uint16) or binary image.
    Bdil : Structuring element for dilation (default: 3x3 cross).
    Bero : Structuring element for erosion (default: 3x3 cross).

    Returns
    -------
    y : Image of same type as `f`.
    """

    if Bdil is None: Bdil = secross()
    if Bero is None: Bero = secross()
    return subm(dilate(f,Bdil),erode(f,Bero))


def grain(f, labels, measurement, option="image"):
    """
    y = grain(f, labels, measurement, option="image")

    Gray-scale statistics for each labeled region.

    Computes gray-scale statistics of each grain in the image. The
    grains regions are specified by the labeled image `labels` and the
    gray-scale information is specified by the image `f`. The
    statistics to compute is specified by the parameter measurement,
    which has the same options as in function stats. The
    parameter option defines the output format:

    'image'
        if the output is an uint16 image where each label value is changed to
        the measurement value
    'data'
        a double column vector. In this case, the first element (index 0) is
        the measurement of region 1. The region with label zero is not measure
        as it is normally the background.

    Parameters
    ----------
    f : ndarray (gray-scale)
    labels : ndarray of integer dtype
        Labeled image. As usual, label 0 is backaground.
    measurement : {'max', 'min', 'median', 'mean', 'sum', 'std', 'std1' }
        Which measure to compute.
    option : {'image' [default], 'data' }
        Output format: if 'image', results as a gray-scale mosaic image
        (default): if 'data', results a column vector of measurements.

    Returns
    -------
    y: Gray-scale (uint8 or uint16) image.
        Or an array with gray-scale statistics per region
    """
    """
        - Examples
            #
            #   example 1
            1#
            f=to_uint8([range(6),range(6),range(6)])
            labels=labelflat(f)
            grain(labels,f,'sum','data')
            grain(labels,f,'sum')
            #
            #   example 2
            #
            f=readgray('astablet.tif')
            g=gradm(f)
            marker=regmin(close(g))
            ws=cwatershed(g,marker,sebox(),'regions')
            g=grain(ws,f,'mean')
            show(f)
            show(g)
    """
    from numpy import zeros_like, asarray, zeros, float32
    from string import lower

    measurement = lower(measurement)
    is_data = (lower(option) == 'data')
    if is_data:
        y = []
    else:
        type = f.dtype
        if measurement in ('mean', 'std'):
            type = float32
        y = zeros(labels.shape)
    fdata = f.ravel()
    labelsravel = labels.ravel()
    for obj_id in xrange(labelsravel.max()):
        pixels = fdata[labelsravel == (obj_id + 1)]
        if measurement == 'max':
            val = pixels.max()
        elif measurement == 'min':
            val = pixels.min()
        elif measurement == 'sum':
            val = pixels.sum()
        elif measurement == 'mean':
            val = pixels.mean()
        elif measurement == 'std':
            val = pixels.std()
        elif measurement == 'std1':
            print "'std1' is not implemented"
        else:
            print "measurement should be 'max', 'min', 'mean', 'sum', 'std', 'std1'."
        if is_data:
            y.append(val)
        else:
            y += val*(labels == (obj_id+1))
    return asarray(y)


def gray(f, dtype="uint8", k=None):
    """
    y = gray(f, dtype="uint8", k=None)

    Convert a binary image into a gray-scale image.

    `gray` converts a binary image into a gray-scale image of a
    specified data type. The value `k` is assigned to the 1 pixels of
    `f`, while the `0` pixels are assigned to the minimum value
    associated to the specified data type.

    Parameters
    ----------
    f :      Binary image.
    dtype :  { 'uint8' [default], 'uint16', 'int32' }
    k : scalar, optional
        Pixel value to assign to `True` pixels (default: max value for `dtype`).

    Returns
    -------
    y : Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.
    """
    from numpy import array
    if k is None: k = maxleveltype(dtype)
    if type(f) is list: f = binary(f)
    assert isbinary(f), 'f must be binary'
    if   dtype == 'uint8' : y = to_uint8(f*k)
    elif dtype == 'uint16': y = to_uint16(f*k)
    elif dtype == 'int32' : y = to_int32(f*k) - to_int32(neg(f)*maxleveltype(dtype))
    else:
        assert 0, 'pymorph.gray: type not supported: %s' % dtype
    return y


def hmin(f, h=1, Bc=None):
    """
    y = hmin(f, h=1, Bc={3x3 cross})

    Remove basins with contrast less than h.

    `hmin` sup-reconstructs the gray-scale image `f` from the marker
    created by the addition of the positive integer value `h` to `f`,
    using the connectivity `Bc`. This operator removes connected
    basins with contrast less than `h`. This function is very useful
    for simplifying the basins of the image.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) image.
    h :  Contrast (default: 1).
    Bc : Connectivity structuring element (default: 3x3 cross).

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.
    """
    """
    Examples
      #
      #   example 1
      #
      a = to_uint8([
          [10,   3,   6,  18,  16,  15,  10],
          [10,   9,   6,  18,   6,   5,  10],
          [10,   9,   9,  15,   4,   9,  10],
          [10,  10,  10,  10,  10,  10,  10]])
      print hmin(a,1,sebox())
      #
      #   example 2
      #
      f = readgray('r4x2_256.tif')
      show(f)
      fb = hmin(f,70)
      show(fb)
      show(regmin(fb))
    """

    if Bc is None: Bc = secross()
    g = addm(f, h)
    return suprec(g, f, Bc);


def hmax(f, h=1, Bc=None):
    """
    Remove peaks with contrast less than h.

    y = hmax(f, h=1, Bc={3x3 cross})

    `hmax` inf-reconstructs the gray-scale image f from the marker
    created by the subtraction of the positive integer value h from
    `f`, using connectivity `Bc`. This operator removes connected
    peaks with contrast less than `h`.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) image.
    h :  Contrast (default: 1).
    Bc : Connectivity structuring element (default: 3x3 cross).

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.
    """
    """
    Examples
      #
      #   example 1
      #
      a = to_uint8([
          [4,   3,   6,  1,  3,  5,  2],
          [2,   9,   6,  1,  6,  7,  3],
          [8,   9,   3,  2,  4,  9,  4],
          [3,   1,   2,  1,  2,  4,  2]])
      print hmax(a,2,sebox())
      #
      #   example 2
      #
      f = readgray('r4x2_256.tif')
      show(f)
      fb = hmax(f,50)
      show(fb)
      show(regmax(fb))
    """

    if Bc is None: Bc = secross()
    g = subm(f, h)
    return infrec(g, f, Bc);


def homothick():
    """
    Iab = homothick()

    Interval for homotopic thickening.

    `homothick` creates an interval that is useful for the homotopic
    (i.e., that conserves the relation between objects and holes)
    thickening of binary images.

    Returns
    -------
    Iab: Interval
    """
    return se2hmt(binary([[1,1,1],
                          [0,0,0],
                          [0,0,0]]),
                  binary([[0,0,0],
                          [0,1,0],
                          [1,1,1]]))


def homothin():
    """
    Iab = homothin()

    Interval for homotopic thinning.

    `homothin` creates an interval that is useful for the homotopic
    (i.e., that conserves the relation between objects and holes)
    thinning of binary images.

    Returns
    -------
    Iab : Interval
    """
    return se2hmt(binary([[0,0,0],
                          [0,1,0],
                          [1,1,1]]),

                   binary([[1,1,1],
                           [0,0,0],
                           [0,0,0]]))


def img2se(fd, flat=True, f=None):
    """
    B = img2se(fd, flat=True, f=None)

    Create a structuring element from a pair of images.

    `img2se` creates a flat structuring element `B` from the binary
    image `fd` or creates a non-flat structuring element `B` from the
    binary image `fd` and the gray-scale image `f`. `fd` represents the
    domain of `B` and `f` represents the image of the points in `fd`.

    Parameters
    ----------
    fd : Binary image.
        The image is in the matrix format where the origin (0,0) is at the
        matrix center.
    flat : boolean, optional
        Whether to return a flat structuring element (default: True)
    f : ndarray of type (uint8 or uint16, int32, or bool), optional

    Returns
    -------
      B : Structuring Element
    """
    from numpy import choose, ones

    assert isbinary(fd),'pymorph.img2se: first parameter must be binary'
    if flat:
        return seshow(fd)
    B = choose(fd, (limits(to_int32([0]))[0]*ones(fd.shape),f) )
    return seshow(to_int32(B),'NON-FLAT')


def infcanon(f, Iab, theta=45, direction='clockwise'):
    """
    y = infcanon(f, Iab, theta=45, direction='clockwise')

    Intersection of inf-generating operators.

    `infcanon` creates the image `y` by computing intersections of
    transformations of the image `f` by inf-generating (i.e., dual of
    the hit-or-miss) operators. These inf-generating operators are
    characterized by rotations (in the clockwise or anti-clockwise
    direction) of `theta` degrees of the interval `Iab`.

    Parameters
    ----------
    f : Binary image.
    Iab : Interval
    theta : Degrees of rotation (default: 45)
    direction : { 'clockwise' [default], 'anti-clockwise' }

    Returns
    -------
    y : Binary image.
    """
    from string import upper

    direction = upper(direction)
    y = union(f,1)
    for t in xrange(0,360,theta):
        Irot = interot(Iab, t, direction)
        y = intersec(y, infgen(f, Irot))
    return y


def infgen(f, Iab):
    """
    y = infgen(f, Iab)

    Inf-generating.

    `infgen` creates the image `y` by computing the transformation of
    the image `f` by the inf-generating operator (or dual of the
    hit-or-miss) characterized by the interval `Iab`.

    Parameters
    ----------
    f :   Binary image.
    Iab : Interval

    Returns
    -------
    y: Binary image.
    """

    A,Bc = Iab
    return union(dilate(f, A),dilate(neg(f), Bc))


def infrec(f, g, Bc=None):
    """
    y = infrec(f, g, Bc={3x3 cross})

    Inf-reconstruction.

    `infrec` creates the image `y` by an infinite number of recursive
    iterations (iterations until stability) of the dilation of `f` by
    Bc conditioned to `g`. We say the `y` is the inf-reconstruction of
    g from the marker `f`. For algorithms and applications, see
    Vinc:93b.

    Parameters
    ----------
    f :  Marker image (gray or binary).
    g :  Conditioning image (gray or binary).
    Bc : Connectivity Structuring element (default: 3x3 cross).

    Returns
    -------
    y : Image
    """
    """
    - Examples
        #
        #   example 1
        #
        g=readgray('text_128.tif')
        f=erode(g,seline(9,90))
        y=infrec(f,g,sebox())
        show(g)
        show(f)
        show(y)
        #
        #   example 2
        #
        g=neg(readgray('n2538.tif'))
        f=intersec(g,0)
        f=draw(f,'LINE:40,30,60,30:END')
        y30=cdilate(f,g,sebox(),30)
        y=infrec(f,g,sebox())
        show(g)
        show(f)
        show(y30)
        show(y)
    """
    if Bc is None: Bc = secross()
    n = f.size
    return cdilate(f, g, Bc, n);


def inpos(f, g, Bc=None):
    """
    y = inpos(f, g, Bc={3x3 cross})

    Minima imposition.

    Minima imposition on `g` based on the marker `f`. `inpos` creates
    an image `y` by filing the valleys of `g` that do not cover the
    connect components of `f`. A remarkable property of `y` is that its
    regional minima are exactly the connect components of `g`.

    Parameters
    ----------
    f :  Binary image. Marker image.
    g :  Gray-scale (uint8 or uint16) image. input image.
    Bc : connectivity structuring element (default: 3x3 cross).

    Returns
    -------
    y : Gray-scale (uint8 or uint16) image.
    """

    if Bc is None: Bc = secross()
    assert isbinary(f), 'pymorph.inpos: first parameter must be binary image'
    fg = gray(neg(f),datatype(g))
    k = limits(g)[1] - 1
    return suprec(fg, intersec(union(g, 1), k, fg), Bc)


def interot(Iab, theta=45, direction="clockwise"):
    """
    Irot = interot(Iab, theta=45, direction="clockwise")

    Rotate an interval

    `interot` rotates the interval `Iab` by `theta`.

    Parameters
    ----------
    Iab :       Interval
    theta :     Degrees of rotation. Should be a multiple of 45 degrees. If not,
                   the rotation is approximate (default: 45).
    direction : { 'clockwise' [default], 'anti-clockwise' }

    Returns
    -------
    Irot : Interval

    Examples
    --------

    ::

        b1 = endpoints()
        b2 = interot(b1)
        print intershow(b1)
        print intershow(b2)

    """
    from string import lower

    direction = lower(direction)
    A,Bc = Iab
    if direction != 'clockwise':
        theta = 360 - theta
    Irot = se2hmt(serot(A, theta),
                    serot(Bc,theta))
    return Irot


def intersec(f1, f2, f3=None, f4=None, f5=None):
    """
    y = intersec(f1, f2, f3=None, f4=None, f5=None)

    Intersection of images.

    `intersec` creates the image `y` by taking the pixelwise minimum
    between the images `f1`, `f2`, `f3`, `f4`, and `f5` . When `f1`,
    `f2`, `f3`, `f4`, and f5 are binary images, `y` is the intersection
    of them.

    Parameters
    ----------
    f1 : Image (gray or binary) or constant.
    f2 : Image (gray or binary) or constant.
    f3 : Image (gray or binary) or constant, optional.
    f4 : Image (gray or binary) or constant, optional.
    f5 : Image (gray or binary) or constant, optional.

    Returns
    -------
    y : Image
    """
    """
    - Examples
      #
      #   example 1
      #
      f=to_uint8([255,  255,    0,   10,    0,   255,   250])
      g=to_uint8([ 0,    40,   80,   140,  250,    10,    30])
      print intersec(f, g)
      print intersec(f, 0)
      #
      #   example 2
      #
      a = readgray('form-ok.tif')
      b = readgray('form-1.tif')
      c = intersec(a,b)
      show(a)
      show(b)
      show(c)
      #
      #   example 3
      #
      d = readgray('tplayer1.tif')
      e = readgray('tplayer2.tif')
      f = readgray('tplayer3.tif')
      g = intersec(d,e,f)
      show(d)
      show(e)
      show(f)
      show(g)
    """
# Should this be intersect(f0, f1, *args) and take an arbitrary nr of inputs?
    from numpy import minimum

    y = minimum(f1,f2)
    if f3 != None: y = minimum(y,f3)
    if f4 != None: y = minimum(y,f4)
    if f5 != None: y = minimum(y,f5)
    return y.astype(f1.dtype)


def intershow(Iab):
    """
    s = intershow(Iab)

    Visualize an interval.

    `intershow` creates a representation for an interval using `0`, `1`
    and `.` (don't care).

    Parameters
    ----------
    Iab : Interval

    Returns
    -------
    s : string representation of the interval.

    Examples
    --------
    ::

        print intershow(homothick())

    prints out

    ::

        0 0 0
        . 0 .
        0 0 0
    """
    from numpy import array, product, reshape, choose
    from string import join

    assert (type(Iab) is tuple) and (len(Iab) == 2),'pymorph.intershow: not proper format of hit-or-miss template'
    A,B = Iab
    s = ''
    for y in xrange(A.shape[0]):
        for x in xrange(A.shape[1]):
            if A[y,x]:
                s += '1 '
            elif B[y,x]:
                s += '0 '
            else:
                s += '. '
        s += '\n'
    return s


def isbinary(f):
    """
    is_b = isbinary(f)

    Check for binary image

    `isbinary` returns True if the datatype of the input image is
    binary. A binary image has just the values `0` and `1`.

    Parameters
    ----------
    f : Image

    Returns
    -------
    is_b : bool
        Whether `f` is a binary image
    """
    return f.dtype == bool

def asbinary(f):
    """
    fbin = asbinary(f)

    Transforms f into a binary image

    Parameters
    ----------
    f : image of any type, consisting only of 0s and 1s.

    Returns
    -------
      fbin : binary image
    """
    import numpy
    if isbinary(f): return f
    assert ( (f == 0) | (f == 1) ).sum() == f.size, 'pymorph.asbinary: f has values that are not 0 or 1.'
    return (f != 0)

def isequal(f1, f2):
    """
    r_eq = isequal(f1, f2)

    Check if two images are equal

    `isequal` compares the images `f1` and f2 and returns `True`, if
    `f1 and `f2` have the same size and `f1(x) == f2(x)`, for all pixel `x`;
    otherwise, returns `False`.

    Parameters
    ----------
    f1 :  Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.
    f2 :  Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.

    Returns
    -------
    r_eq : Boolean
        Whether the two images are equal
    """
    import numpy
    if f1.shape != f2.shape:
        return False
    return numpy.all(f1 == f2)


def labelflat(f, Bc=None, lambda_=0):
    """
    y = labelflat(f, Bc=None, lambda_=0)

    Label the flat zones of gray-scale images.

    `labelflat` creates the image y by labeling the flat zones of `f`,
    according to the connectivity defined by the structuring
    element `Bc`. A flat zone is a connected region of the image
    domain in which all the pixels have the same gray-level
    (lambda=0). When lambda is different than zero, a quasi-flat
    zone is detected where two neighboring pixels belong to the same
    region if their difference gray-levels is smaller or equal
    `lambda`. The minimum label of the output image is 1 and the
    maximum is the number of flat-zones in the image.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    Bc : Structuring element, optional
        Connectivity (efault: 3x3 elementary cross).
    lambda_ : Default: 0. Connectivity given by ``|f(q)-f(p)|<=lambda_``.

    Returns
    -------
    y: ndarray of same shape as `f`
        If number of labels is less than 65535, the data type is uint16;
        otherwise, it is int32.
    """
    """
            f=to_uint8([
               [5,5,8,3,0],
               [5,8,8,0,2]])
            g=labelflat(f)
            print g
            g1=labelflat(f,secross(),2)
            print g1
            #
            #   example 2
            #
            f=readgray('blob.tif')
            d=dist(f,sebox(),'euclidean')
            g= d /8
            show(g)
            fz=labelflat(g,sebox());
            lblshow(fz)
            print fz.max()
            #
            #   example 3
            #
            f=readgray('pcb_gray.tif')
            g=labelflat(f,sebox(),3)
            show(f)
            lblshow(g)
    """
    from numpy import allclose, ravel, nonzero, array
    import numpy as np
    if Bc is None: Bc = secross()
    assert not lambda_, 'pymorph.labelflat: only lambda_==0 is supported.'

    label = 1
    labeled = np.zeros(f.shape, np.uint16)
    fmark = np.zeros_like(f)
    Y,X = np.where(f)
    for y,x in zip(Y,X):
        if labeled[y,x]: continue
        val = f[y,x]
        fmark[y,x] = val
        r = infrec(fmark, (f == val), Bc)
        labeled[r] = label
        fmark[y,x] = 0
        label += 1
    return labeled


def lastero(f, B=None):
    """
    Last erosion.

    y = lastero(f, B=None)

    `lastero` creates the image y by computing the last erosion by
    the structuring element B of the image f . The objects found in
    y are the objects of the erosion by nB that can not be
    reconstructed from the erosion by (n+1)B , where n is a generic
    non negative integer. The image y is a proper subset of the
    morphological skeleton by B of f .

    Parameters
    ----------
    f : Binary image.
    B : Structuring Element (default: 3x3 elementary cross).

    Returns
    -------
    y : Binary image.
    """

    assert isbinary(f),'pymorph.lastero: can only process binary images'
    if B is None: B = secross()
    dt = dist(f,B)
    return regmax(dt,B)


def open(f, b=None):
    """
    y = open(f, b={3x3 cross})

    Morphological opening.

    `open` creates the image y by the morphological opening of the
    image `f` by the structuring element `b`. In the binary case, the
    opening by the structuring element `b` may be interpreted as the
    union of translations of `b` included in `f`. In the gray-scale
    case, there is a similar interpretation taking the functions
    umbra.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    b : Structuring element (default: 3x3 elementary cross).

    Returns
    -------
    y : Image
    """

    if b is None: b = secross()
    return dilate(erode(f,b),b)


def openrec(f, Bero=None, Bc=None):
    """
    y = openrec(f, Bero={3x3 cross}, Bc={3x3 cross})

    Opening by reconstruction.

    `openrec` creates the image `y` by an inf-reconstruction of the
    image `f` from its erosion by `bero`, using the connectivity
    defined by `Bc`.

    Parameters
    ----------
    f :    Gray-scale (uint8 or uint16) or binary image.
    Bero : Eroding structuring element (default: 3x3 cross).
    Bc :   Connecting structuring element (default: 3x3 cross).

    Returns
    -------
    y : Image (same type as f).
    """

    if Bero is None: Bero = secross()
    if Bc is None: Bc = secross()
    return infrec(erode(f,Bero),f,Bc)


def openrecth(f, Bero=None, Bc=None):
    """
    y = openrecth(f, Bero={3x3 cross}, Bc={3x3 cross})

    Open-by-Reconstruction Top-Hat.

    `openrecth` creates the image `y` by subtracting the open by
    reconstruction of `f`, defined by the structuring elements `Bero` and
    `Bc`, of `f` itself.

    Parameters
    ----------
    f :    Gray-scale (uint8 or uint16) or binary image.
    Bero : Erosion structuring element (default: 3x3 cross).
    Bc :   Connectivity structuring element (default: 3x3 cross).

    Returns
    -------
    y : Image of same type as `f`.
    """
    if Bero is None: Bero = secross()
    if Bc is None: Bc = secross()
    return subm(f, openrec(f, Bero, Bc))


def openth(f, b=None):
    """
    y = openth(f, b={3x3 cross})

    Opening Top Hat.

    `openth` creates the image y by subtracting the morphological
    opening of `f` by the structuring element `b` of `f` itself.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    b : Structuring element (default: 3x3 cross).

    Returns
    -------
    y : Image of same type as `f`.
    """
    """
        - Examples
            #
            a = readgray('keyb.tif')
            show(a)
            b = openth(a,sebox(3))
            show(b)
    """

    if b is None: b = secross()
    return subm(f, open(f,b))


def opentransf(f, type='octagon', n=65535, Bc=None, Buser=None):
    """
    y = opentransf(f, type='octagon', n=65535, Bc={3x3 cross}, Buser={3x3 cross})

    Open transform.

    Compute the open transform of a binary image. The value of the
    pixels in the open transform is radius of the smallest disk, which
    if used to open, nullifies that pixel.

    The disk sequence must satisfy the following: if `r > s`, then `rB` is
    `sB`-open, i.e. `open(rB,sB) == rB`. Note that the Euclidean
    disk does not satisfy this property in the discrete grid. This
    function also computes the reconstructive open transform by
    adding the suffix '-rec' in the 'type' parameter.

    Parameters
    ----------
    f : binary image.
    type : { 'octagon' [default], 'chessboard', 'citi-block', 'linear-h', 'linear-v', 'linear-45r', 'user' }
        disk family.
    n : integer, optional
        Maximum disk radius (default: 65535).
    Bc :  Structuring element, optional
        Connectivity for the reconstructive opening. Used if '-rec' suffix is
        appended in the 'type' string. default: 3x3 cross
    Buser : structure element, optional
        user disk, used if 'type' is 'user'.

    Returns
    -------
    y : Gray-scale (uint16) image.
    """
    """
        - Examples
            #
            #   example 1
            #
            f = binary([
                          [0,0,0,0,0,0,0,0],
                          [0,0,1,1,1,1,0,0],
                          [0,0,1,1,1,1,1,0],
                          [0,1,0,1,1,1,0,0],
                          [1,1,0,0,0,0,0,0]])
            print opentransf( f, 'city-block')
            print opentransf( f, 'linear-h')
            print opentransf( f, 'linear-45r')
            print opentransf( f, 'user',10,secross(),binary([0,1,1]))
            print opentransf( f, 'city-block-rec')
            #
            #   example 2
            #
            f=readgray('numbers.tif')
            show(f)
            g=opentransf(f,'OCTAGON')
            show(g)
            #
            #   example 3
            #
            b=sedisk(3,'2D','OCTAGON')
            g1=open(f,b)
            show(g1)
            g2=(g > 3)
            print g1 == g2
    """
    import numpy as np
    from string import lower
    if Bc is None: Bc = secross()
    if Buser is None: Buser = secross()
    assert isbinary(f),'pymorph.opentransf: input image is not binary'
    type = lower(type)
    rec_flag = False
    if type.endswith('-rec'):
        rec_flag = True
        type = type[:-len('-rec')]
    disk_se = (type in ('octagon', 'chessboard', 'city-block'))
    if disk_se:
        n  = min(n,min(f.shape))
    elif  type == 'linear-h':
        se = binary([[1, 1, 1]])
        n  = min(n,f.shape[1])
    elif  type =='linear-v':
        se = binary([
                [1],
                [1],
                [1]])
        n  = min(n,f.shape[0])
    elif  type == 'linear-45r':
        se = binary([
                    [0, 0, 1],
                    [0, 1, 0],
                    [1, 0, 0]])
        n  = min(n,min(f.shape))
    elif  type == 'linear-45l':
        se = binary([
                    [1, 0, 0],
                    [0, 1, 0],
                    [0, 0, 1]])
        n  = min(n,min(f.shape))
    elif  type == 'user':
        se = Buser
        n  = min(n,min(f.shape))
    else:
        assert False, \
            'pymorph.opentransf: only accepts octagon, chessboard, \
                city-block, linear-h, linear-v, linear-45r, linear-45l, or user as type, or with suffix -rec.'
    y = np.zeros(f.shape, np.uint8)
    for k in xrange(n):
        prev = y
        if disk_se:
            a = open(f, sedisk(k, 2, type))
        else:
            a = open(f, sesum(se,k))
        y = addm(y, gray(a,'uint16',1))
        if isequal(y, prev):
            break

    if rec_flag:
        return grain(label(f,Bc), y, 'max')
    return y


def patspec(f, type='octagon', n=65535, Bc=None, Buser=None):
    """
    h = patspec(f, type='octagon', n=65535, Bc={3x3 cross}, Buser=None)

    Pattern spectrum (also known as granulometric size density).

    Compute the Pattern Spectrum of a binary image. See Mara:89b .
    The pattern spectrum is the histogram of the open transform, not
    taking the zero values.

    Parameters
    ----------
    f :     Binary image.
    type : { 'octagon' [default], 'chessboard', 'citi-block', 'linear-h', 'linear-v', 'linear-45r', 'user' }
        disk family.
    n : integer, optional
        Maximum disk radius (default: 65535).
    Bc :  Structuring element, optional
        Connectivity for the reconstructive granulometry. Used if '-rec' suffix
        is appended in the 'type' string. default: 3x3 cross
    Buser : structure element, optional
        user disk, used if 'type' is 'user'.

    Returns
    -------
    h : ndarray
    """

    if Bc is None: Bc = secross()
    if Buser is None: Buser = secross()
    assert isbinary(f),'pymorph.patspec: Error: input image is not binary'
    g = opentransf(f,type,n,Bc,Buser)
    h = histogram(g)
    h = h[1:]
    return h


def regmax(f, Bc=None):
    """
    y = regmax(f, Bc={3x3 cross})

    Regional Maximum.

    `regmax` creates a binary image `y` by computing the regional
    maxima of `f`, according to the connectivity defined by the
    structuring element `Bc`. A regional maximum is a flat zone not
    surrounded by flat zones of higher gray values.

    Parameters
    ----------
    f :  Gray-scale image.
    Bc : Connectivity structuring element (default: 3x3 cross).

    Returns
    -------
    y : Binary image.
    """

    if Bc is None: Bc = secross()
    return regmin(neg(f),Bc)


def regmin(f, Bc=None, option="binary"):
    """
    y = regmin(f, Bc=None, option="binary")

    Regional Minimum (with generalized dynamics).

    `regmin` creates a binary image by computing the regional
    minima of `f`, according to the connectivity defined by the
    structuring element `Bc`.

    A regional minimum is a flat zone not surrounded by flat zones of
    lower gray values. A flat zone is a maximal connected component of a
    gray-scale image with same pixel values.

    Parameters
    ----------
    f :      Gray-scale (uint8 or uint16) image.
    Bc :     Connectivity structuring element (default: 3x3 cross).
    option : 'binary'
        if 'binary', outputs a binary image. Currently, it must be set to
        'binary'. Fortunately, that's the default.

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.


    Not Implemented
    ---------------

    There are three output options: binary image; valued image; and
    generalized dynamics. The dynamics of a regional minima is the
    minimum height a pixel has to climb in a walk to reach another
    regional minima with a higher dynamics.  The area-dyn is the minimum
    area a catchment basin has to raise to reach another regional minima
    with higher area-dynamics.  The volume-dyn is the minimum volume a
    catchment basin has to raise to reach another regional minima with a
    higher volume dynamics.

    The dynamics concept was first introduced in Grimaud:92 and it is
    the basic notion for the hierarchical or multiscale watershed
    transform.

    Controlled by option:

    'value'
        output a grayscale image with points at the regional minimum with the
        pixel values of the input image
    'dynamics'
        output a grayscale image with points at the regional minimum with its
        dynamics;
    'area-dyn'
        int32 image with the area-dynamics;
    'volume-dyn'
        int32 image with the volume-dynamics.

    """
    """
        - Examples
            #
            #   example 1
            #
            a = to_uint8([
                [10,  10,  10,  10,  10,  10,  10],
                [10,   9,   6,  18,   6,   5,  10],
                [10,   9,   6,  18,   6,   5,  10],
                [10,   9,   9,  15,   4,   9,  10],
                [10,   9,   9,  15,  12,  10,  10],
                [10,  10,  10,  10,  10,  10,  10]])
            print regmin(a)
            print regmin(a,secross(),'value')
            print regmin(a,secross(),'dynamics')
            #
            #   example 2
            #
            f1=readgray('bloodcells.tif')
            m1=regmin(f1,sebox())
            show(f1,m1)
            f2=hmin(f1,70)
            show(f2)
            m2=regmin(f2,sebox())
            show(f2,m2)
            #
            #   example 3
            #
            f=readgray('cameraman.tif')
            g=gradm(f)
            mh=regmin(g,secross(),'dynamics')
            ws1=cwatershed(g, binary(mh, 20))
            ws2=cwatershed(g, binary(mh, 40))
            show(ws1)
            show(ws2)
    """

    if option != 'binary':
        raise ValueError, "pymorph.regmin only implements option 'binary'"
    if Bc is None: Bc = secross()
    fplus = addm(f,1)
    g = subm(suprec(fplus,f,Bc),f)
    return union(threshad(g,1),threshad(f,0,0))


def se2interval(a, b):
    """
    Iab = se2interval(a, b)

    Create an interval from a pair of structuring elements.

    `se2interval` creates the interval [a,b] from the structuring
    elements `a` and `b` such that `a` is less or equal `b`.

    Parameters
    ----------
    a : Structuring element left extremity
    b : Structuring element right extremity

    Returns
    -------
    Iab : Interval
    """
    return a, neg(b)


def se2hmt(A, Bc):
    """
    Iab = se2hmt(A, Bc)

    Create a Hit-or-Miss Template (or interval) from a pair of
    structuring elements.

    `se2hmt` creates the Hit-or-Miss Template (HMT), also called
    interval `[A,Bc]` from the structuring elements `A` and `Bc` such that
    `A` is included in the complement of `Bc` . The only difference
    between this function and `se2interval` is that here the second
    structuring element is the complement of the one used in the
    other function. The advantage of this function over
    `se2interval` is that this one is more flexible in the use of
    the structuring elements as they are not required to have the
    same size.

    Parameters
    ----------
    A :  Structuring Element Left extremity.
    Bc : Structuring Element Complement of the right extremity.

    Returns
    -------
    Iab : Interval
    """
    return A, Bc


def se2flatidx(f,Bc):
    """
    Bi = se2flatidx(f,Bc)

    Transforms the Bc structuring element into an array of indices so that
    f.flat[Bi[i]] corresponds to the i-th element where Bc is true

    Equivalent of::

        g = zeros(f.shape)
        h,w=Bc.shape
        g[:h,:w]=Bc
        Bi, = where(g.ravel())

    This is useful for implementing many functions. See the implementation of
    `label()` for an example.

    Parameters
    ----------
    f : image
    Bc : structuring element

    Returns
    -------
    Bi : array of indice offsets.
    """
    from numpy import array, where
    h,w=Bc.shape
    h //= 2
    w //= 2
    Bi=[]
    for i,j in zip(*where(Bc)):
        Bi.append( (j-w)+(i-h)*f.shape[1] )
    return array(Bi)
def sebox(r=1):
    """
    B = sebox(r=1)

    Create a box structuring element.

    `sebox` creates the structuring element B formed by r successive
    Minkowski additions of the elementary square (i.e., the 3x3
    square centered at the origin) with itself. If `R=0`, `B` is the
    unitary set that contains the origin. If `R=1`, `B` is the
    elementary square itself.

    Parameters
    ----------
    r : integer, optional
        size of box (default: 1)

    Returns
    -------
    B : Structuring Element
    """

    return sesum(binary([[1,1,1],
                         [1,1,1],
                         [1,1,1]]),
                 r)


def secross(r=1):
    """
    B = secross(r=1)

    Diamond structuring element and elementary 3x3 cross.

    `secross` creates the structuring element `B` formed by `r`
    successive Minkowski additions of the elementary cross (i.e.,
    the 3x3 cross centered at the origin) with itself. If `r=0`, `B` is
    the unitary set that contains the origin. If `r=1` (the default),
    `B` is the elementary cross itself.

    Parameters
    ----------
    r : integer, optional
        radius (default: 1)

    Returns
    -------
    B : Structuring Element
    """
    return sesum(binary([[0,1,0],
                         [1,1,1],
                         [0,1,0]]),
                 r)


def sedisk(r=3, dim=2, metric="euclidean", flat=True, h=0):
    """
    B = sedisk(r=3, dim=2, metric="euclidean", flat=True, h=0)

    Create a disk or a semi-sphere structuring element.

    `sedisk` creates a flat structuring element B that is disk under
    the metric metric , centered at the origin and with radius r or
    a non-flat structuring element that is a semi-sphere under the
    metric metric, centered at (0, h) and with radius r . this
    structuring element can be created on the 1D, 2D or 3D space.

    Parameters
    ----------
    r :        Non-negative integer. Disk radius (default: 3)
    dim : {1, 2 [default], 3}
        dimension
    metric : {'euclidean' [default], 'city-block', 'octagon', 'chessboard' }
    flat : boolean, optional
        default: True
    h : double, optional
        elevation of the center of the semi-sphere (default: 0).

    Returns
    -------
    B : Structuring Element
    """
    from string import lower
    from numpy import resize, transpose, arange
    from numpy import sqrt, arange, transpose, maximum

    metric = lower(metric)
    assert dim== 2, 'pymorph.sedisk: Supports only 2D structuring elements'
    if flat: y = binary([[1]])
    else:    y = to_int32([[h]])
    if r == 0: return y
    if metric == 'city-block':
        if flat:
            b = secross(1)
        else:
            b = to_int32([[-2147483647, 0,-2147483647],
                          [          0, 1,          0],
                          [-2147483647, 0,-2147483647]])
        return sedilate(y,sesum(b,r))
    elif metric == 'chessboard':
        if flat:
            b = sebox(1)
        else:
            b = to_int32([[1,1,1],
                          [1,1,1],
                          [1,1,1]])
        return sedilate(y,sesum(b,r))
    elif metric == 'octagon':
        if flat:
            b1,b2 = sebox(1),secross(1)
        else:
            b1 = to_int32([
                    [1,1,1],
                    [1,1,1],
                    [1,1,1]])
            b2 = to_int32([
                    [-2147483647, 0, -2147483647],
                    [          0, 1,           0],
                    [-2147483647, 0, -2147483647]])
        if r == 1:
            return b1
        return sedilate(sedilate(y,sesum(b1,r//2)), sesum(b2,(r+1)//2))
    elif metric == 'euclidean':
        v = arange(-r,r+1)
        x = resize(v, (len(v), len(v)))
        y = transpose(x)
        Be = binary(sqrt(x*x + y*y) <= (r+0.5))
        if flat:
            return Be
        be = h + to_int32( sqrt( maximum((r+0.5)*(r+0.5) - (x*x) - (y*y),0)))
        be = intersec(gray(Be,'int32'),be)
        return be
    else:
        assert False,'Non valid metric'
    return B


def seline(length=3, theta=0):
    """
    B = seline(length=3, theta=0)

    Create a line structuring element.

    seline creates a structuring element B that is a line segment
    that has an extremity at the origin, length `length` and angle
    `theta` (0 degrees is east direction, clockwise). If `length==0` ,
    it generates the origin.

    Note that `length` is simply the number of on-pixels. Therefore, in
    diagonal directions it is off by a factor of \sqrt{2}.

    Parameters
    ----------
    length : Length of line (default: 3)
    theta :  Degrees, clockwise (default: 0, i.e. east)

    Returns
    -------
    B : Structuring Element

    Examples
    --------

    ::

        >>> import pymorph
        >>> pymorph.seline(2)
        array([[0],
               [1],
               [1]], dtype=uint8)
        >>> pymorph.seline(4)
        array([[0],
               [0],
               [0],
               [1],
               [1],
               [1],
               [1]], dtype=uint8)
        >>> pymorph.seline(3,45)
        array([[0, 0, 0],
               [0, 1, 1],
               [0, 0, 1]], dtype=uint8)

    """
    import numpy
    from numpy import pi, tan, cos, sin, sign, floor, arange, transpose, array, ones

    theta = pi*theta/180.
    if abs(tan(theta)) <= 1:
        s  = sign(cos(theta))
        x0 = arange(length) * cos(theta)
        x1 = floor(x0 * tan(theta) + 0.5)
    else:
        s  = sign(sin(theta))
        x1 = arange(length) * sin(theta)
        x0 = floor(x1 / tan(theta) + 0.5)
    x = array(zip(x0, x1), int)
    B = set2mat((x,))
    return B


def serot(b, theta=45, direction="clockwise"):
    """
    brot = serot(b, theta=45, direction="clockwise")

    Rotate a structuring element.
    `serot` rotates structuring element `b` by `theta` degrees.

    Parameters
    ----------
    B :       Structuring Element
    theta :   Degrees of rotation. Should be a multiple of 45 degrees. If not,
                the rotation is approximate (default: 45).
    direction : { 'clockwise' [default], 'anti-clockwise' }

    Returns
    -------
    brot : structuring element
    """
    from string import lower
    from numpy import array, transpose, concatenate
    from numpy import cos, sin, pi

    direction = lower(direction)
    if direction == "anti-clockwise":
       theta = -theta
    theta = pi * theta/180.
    ct = cos(theta)
    st = sin(theta)
    y,v = mat2set(b)
    if len(y)==0: return binary([0])
    x0 = y[:,1] * cos(theta) - y[:,0] * sin(theta)
    x1 = y[:,1] * sin(theta) + y[:,0] * cos(theta)
    x0 = to_int32((x0 +0.5)*(x0>=0) + (x0-0.5)*(x0<0))
    x1 = to_int32((x1 +0.5)*(x1>=0) + (x1-0.5)*(x1<0))
    x = transpose(array([transpose(x1),transpose(x0)]))
    brot = set2mat((x,v))
    return brot


def seshow(b, option="normal"):
    """
    y = seshow(b, option="NORMAL")

    Display a structuring element as an image.

    `seshow` used with the option 'expand' generates an image `y` that
    is a suitable graphical representation of the structuring
    element `b`. This function is useful to convert a structuring
    element to an image. The origin of the structuring element is at
    the center of the image. If `b` is flat, y is binary, otherwise, y
    is signed `int32 image. When using the option 'non-flat', the
    output `y` is always a signed `int32` image.

    Parameters
    ----------
    b :      Structuring element
    option : { 'normal' [default], 'expand', 'non-flat' }
        Mode

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.
    """
    from string import upper

    option = upper(option)
    if option=='NON-FLAT':
        y = to_int32([0])
        if isbinary(b):
            b = intersec(gray(b,'int32'),0)
    elif option=='NORMAL':
        if isbinary(b):
            y = binary([1])
        else:
           y = to_int32([0])
    elif option=='EXPAND':
        assert isbinary(b), 'pymorph.seshow: \'expand\' option is only available with flat SE'
        y = sedilate(binary([1]),b)
        b1 = (y>=0)
        b0 = erode(y,b)
        return bshow(b1,y,b0)
    else:
        assert False, 'pymorph.seshow: not a valid flag: NORMAL, EXPAND or NON-FLAT'
    return sedilate(y,b)


def sesum(B=None, N=1):
    """
    Bn = sesum(B=None, N=1)

    N-1 iterative Minkowski additions

    `sesum` creates the structuring element `Bn` from `N - 1` iterative
    Minkowski additions with the structuring element `B`.

    Parameters
    ----------
    B : Structuring element (Default: 3x3 elementary cross).
    N : Non-negative integer. Default: 1.

    Returns
    -------
    Bn : Structuring Element
    """

    if B is None: B = secross()
    if N==0:
        if isbinary(B): return binary([[1]])
        else:           return to_int32([[0]]) # identity
    NB = B
    for i in xrange(N-1):
        NB = sedilate(NB,B)
    return NB


def setrans(Bi, t):
    """
    Bo = setrans(Bi, t)

    Translate a structuring element

    setrans translates a structuring element by a specific value.

    Parameters
    ----------
    Bi : Structuring Element
    t : translation amount

    Returns
    -------
    Bo : Structuring element of same type as `Bi`
    """

    x,v = mat2set(Bi)
    Bo = set2mat((x+t,v))
    return Bo.astype(Bi.dtype)


def sereflect(Bi):
    """
    Bo = sereflect(Bi)

    Reflect a structuring element

    `sereflect` reflects a structuring element about the origin

    Parameters
    ----------
    Bi : Structuring Element

    Returns
    -------
    Bo : Structuring element of same type as `Bi`
    """
    return Bi[::-1, ::-1]


def sedilate(B1, B2):
    """
    Bo = sedilate(B1, B2)

    Dilate one structuring element by another

    `sedilate` dilates an structuring element by another. The main
    difference between this dilation and `dilate` is that the dilation
    between structuring elements are not bounded, returning another
    structuring element usually larger than anyone of them. This
    gives the composition of the two structuring elements by
    Minkowski addition.

    Parameters
    ----------
    B1 : Structuring Element
    B2 : Structuring Element

    Returns
    -------
    Bo: Structuring Element
    """
    from numpy import newaxis, array, int32

    assert (isbinary(B1) or (B1.dtype == int32)) and (isbinary(B2) or B2.dtype == int32), 'pymorph.sedilate: s.e. must be binary or int32'
    if len(B1.shape) == 1: B1 = B1[newaxis,:]
    if len(B2.shape) == 1: B2 = B2[newaxis,:]
    if B1.dtype==int32 or B2.dtype == int32:
       Bo = to_int32([limits(to_int32([0]))[0]])
       if isbinary(B1):
          B1 = gray(B1,'int32',0)
       if isbinary(B2):
          B2 = gray(B2,'int32',0)
    else:
       Bo = binary([0])
    x,v = mat2set(B2)
    if len(x):
        for i in xrange(x.shape[0]):
            s = add4dilate(B1,v[i])
            st= setrans(s,x[i])
            Bo = seunion(Bo,st)
    return Bo


def seunion(B1, B2):
    """
    B = seunion(B1, B2)

    Union of structuring elements

    `seunion` creates a structuring element from the union of two
    structuring elements.

    Parameters
    ----------
    B1 : Structuring element
    B2 : Structuring element

    Returns
    -------
    B : structure element
        union of B1 and B2
    """
    from numpy import maximum, ones, asarray, newaxis, int32

    assert B1.dtype == B2.dtype, 'Cannot have different datatypes:'
    type1 = B1.dtype
    if len(B1) == 0: return B2
    if len(B1.shape) == 1: B1 = B1[newaxis,:]
    if len(B2.shape) == 1: B2 = B2[newaxis,:]
    if B1.shape != B2.shape:
        inf = limits(B1)[0]
        h1,w1 = B1.shape
        h2,w2 = B2.shape
        H,W = max(h1,h2),max(w1,w2)
        Hc,Wc = (H-1)//2,(W-1)//2    # center
        BB1,BB2 = asarray(B1),asarray(B2)
        B1, B2  = inf * ones((H,W),int32), inf *ones((H,W),int32)
        dh1s , dh1e = (h1-1)//2 , (h1-1)//2 + (h1+1)%2 # deal with even and odd dimensions
        dw1s , dw1e = (w1-1)//2 , (w1-1)//2 + (w1+1)%2
        dh2s , dh2e = (h2-1)//2 , (h2-1)//2 + (h2+1)%2
        dw2s , dw2e = (w2-1)//2 , (w2-1)//2 + (w2+1)%2
        B1[ Hc-dh1s : Hc+dh1e+1  ,  Wc-dw1s : Wc+dw1e+1 ] = BB1
        B2[ Hc-dh2s : Hc+dh2e+1  ,  Wc-dw2s : Wc+dw2e+1 ] = BB2
    B = maximum(B1,B2).astype(type1)
    return B


def skelm(f, B=None, return_binary=True):
    """
    y = skelm(f, B={3x3 cross}, return_binary=True)

    Morphological skeleton (Medial Axis Transform).

    `skelm` creates the image `y` by computing the morphological
    skeleton by `B` of the image `f`. In this
    case, the pixels of value 1 in y are center of maximal balls
    (generated from B ) included in `f`. This is also called Medial
    Axis. If `return_binary` is False, the non zeros pixels in `y` are the
    radius plus 1 of the maximal balls. This is called Medial Axis
    Transform or valued morphological skeleton.

    Parameters
    ----------
    f :      Binary image.
    B :      Structuring Element Default: None (3x3 elementary
               cross).
    return_binary : boolean, optional
         Whether to return a binary image of the medial axis (the default) or a
         greyscale a binary image (medial axis); value: output a grayscale
         image with values of the radius of the disk to reconstruct the
         original image (medial axis transform).

    Returns
    -------
    y : Gray-scale (uint8 or uint16) or binary image.
    """
    """
        - Examples
            #
            #   example 1
            #
            from numpy import ones
            a=neg(frame(binary(ones((7,9)))))
            print a
            print skelm(a)
            print skelm(a,sebox())
            #
            #   example 2
            #
            a=readgray('pcbholes.tif')
            b=skelm(a)
            show(a)
            show(b)
            #
            #   example 3
            #
            c=skelm(a,secross(),'value')
            show(c)
    """
    from numpy import asarray
    if B is None: B = secross()
    assert isbinary(f),'pymorph.skelm: only works for binary images'
    k1,k2 = limits(f)
    y = gray(intersec(f, k1),'uint16')
    iszero = asarray(y)
    nb = sesum(B,0)
    for r in xrange(1,65535):
        ero = erode(f,nb)
        if isequal(ero, iszero): break
        f1 = openth(ero, B)
        nb = sedilate(nb, B)
        y = union(y, gray(f1,'uint16',r))
    if return_binary:
        return binary(y)
    return y


def skelmrec(f, B=None):
    """
    y = skelmrec(f, B={3x3 cross})

    Morphological skeleton reconstruction
    (inverse medial axis transform).

    `skelmrec` reconstructs the valued morphological skeleton to
    recover the original image.

    Parameters
    ----------
    f : Gray-scale (uint8 or uint16) or binary image.
    B : Structuring element (default: 3x3 cross).

    Returns
    -------
    y : Binary image.
    """
    """
        - Examples
            #
            from numpy import ones
            a=neg(frame(binary(ones((7,9)))))
            print a
            b=skelm(a,secross(),'value')
            print b
            c=skelmrec(b,secross())
            print c
    """
    if B is None: B = secross()
    y = binary(intersec(f, 0))
    for r in xrange(f.max(),1,-1):
        y = dilate(union(y,binary(f,r)), B)
    return union(y, binary(f,1))


def skiz(f, Bc=None, return_lines=False, metric=None):
    """
    labeled = skiz(f, Bc={3x3 cross}, return_lines=False, metric=None)
    labeled,lines = skiz(f, Bc={3x3 cross}, return_lines=True, metric=None)

    Skeleton of Influence Zone - also know as Generalized Voronoi Diagram



    `skiz` creates the `labeled` image by detecting the lines which are
    equidistant to two or more connected components of `f`, according
    to the connectivity defined by `Bc`.

    `labeled` will be a labeled image, while `lines` will be a binary image
    of the lines between the regions Depending on with the flag

    When the connected objects of `f` are single points, the `skiz` is the
    Voronoi diagram.

    Parameters
    ----------
    f : Binary image.
    Bc : Structuring Element (default: 3x3 cross).
        Connectivity for the distance measurement.
    return_lines : boolean, optional
        Whether to return the lines separating regions in the image.
        Default: False
    metric : str, optional
        default: 'euclidean'

    Returns
    -------
    y : Gray-scale (uint8 or uint16) image.
    lines : binary image.
        only returned if `return_lines` is ``True``
    """
    """
        - Examples
            #
            #   example 1
            #
            f=readgray('blob2.tif')
            y=skiz(f,sebox(),'LINES','EUCLIDEAN')
            show(f,y)
            #
            #   example 2
            #
            from numpy import zeros
            f=binary(zeros((100,100)))
            f[30,25],f[20,75],f[50,50],f[70,30],f[80,70] = 1,1,1,1,1
            y = skiz(f,sebox(),'LINES','EUCLIDEAN')
            show(f,y)
    """
    from string import upper
    if Bc is None: Bc = secross()
    d = dist(neg(f), Bc, metric)
    return cwatershed(d,f,Bc,return_lines)

def subm(f1, f2):
    """
    y = subm(f1, f2)

    Subtraction of two images, with saturation.

    `subm` creates the image `y` by pixelwise subtraction of the image
    `f2` from the image `f1`. When the subtraction of the values of two
    pixels is negative, 0 is taken as the result of the subtraction.
    When `f1` and `f2` are binary images, `y` represents the set
    subtraction of `f2` from `f1`.

    Parameters
    ----------
    f1 : Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image.
    f2 : Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image. Or constant.

    Returns
    -------
    y : ndarray of same shape and dtype as `f1`

    Examples
    --------

    ::

        f = to_uint8([255,   255,    0,   10,   20,   10,    0,   255,  255])
        g = to_uint8([10,     20,   30,   40,   50,   40,   30,    20,    10])
        print subm(f, g)
        print subm(f, 100)
        print subm(100, f)

    ::

        a = readgray('boxdrill-C.tif')
        b = readgray('boxdrill-B.tif')
        c = subm(a,b)
        show(a)
        show(b)
        show(c)
    """
    from numpy import array, clip

    if type(f2) is array:
        if f1.dtype != f2.dtype:
            raise TypeError('pymorph.subm(): arguments cannot have different datatypes')
    bottom,top = limits(f1)
    y = clip(f1.astype('d') - f2, bottom, top)
    return y.astype(f1.dtype)


def supcanon(f, Iab, theta=45, direction="clockwise"):
    """
    y = supcanon(f, Iab, theta=45, direction="clockwise")

    Union of sup-generating or hit-miss operators.

    `supcanon` creates the image `y` by computing the union of
    transformations of the image `f` by sup-generating operators.
    These hit-miss operators are characterized by rotations (in the
    clockwise or anti-clockwise direction) of `theta` degrees of the
    interval `Iab`.

    Parameters
    ----------
    f : Binary image.
    Iab : Interval
    theta : {45 [default], 90, 180 }
        Angle of rotation in degrees.
    direction : {'clockwise' [default], 'anti-clockwise'}
        Direction of rotation

    Returns
    ------
    y : Binary image.
    """
    y = intersec(f,0)
    for t in xrange(0,360,theta):
        Irot = interot(Iab, t, direction)
        y = union(y, supgen(f, Irot))
    return y


def supgen(f, interval):
    """
    y = supgen(f, interval)

    Sup-generating (hit-miss).

    `supgen` creates the binary image `y` by computing the transformation of
    the image f by the sup-generating operator characterized by the interval
    `Iab`. The sup-generating operator is just a relaxed template matching,
    where the criterion to keep a shape is that it be inside the interval
    `Iab`. Note that we have the classical template matching when `a==b`. Note
    also that the sup-generating operator is equivalent to the classical
    hit-miss operator.

    Parameters
    ----------
    f :        Binary image.
    interval : Interval

    Returns
    -------
    y : Binary image.
    """
    """
        - Examples
            #
            #   example 1
            #
            f=binary([
               [0,0,1,0,0,1,1],
               [0,1,0,0,1,0,0],
               [0,0,0,1,1,0,0]])
            i=endpoints()
            print intershow(i)
            g=supgen(f,i)
            print g
            #
            #   example 2
            #
            a=readgray('gear.tif')
            b=supgen(a,endpoints())
            show(a)
            show(dilate(b))
    """

    A,Bc = interval
    return intersec(erode(f,A),
                   erode(neg(f),Bc))

def suprec(f, g, Bc=None):
    """
    y = suprec(f, g, Bc={3x3 cross})

    Sup-reconstruction.

    `suprec` creates the image `y` by an infinite number of iterations
    (until stability) of the erosion of `f` by `Bc` conditioned to `g`.
    We say that y is the sup-reconstruction of `g` from the marker `f`.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) or binary image.
        Marker image.
    g :  Gray-scale (uint8 or uint16) or binary image.
        Conditioning image.
    Bc : Connectivity structuring element (default: 3x3 cross)

    Returns
    -------
    y : Image
    """
    from numpy import product
    if Bc is None: Bc = secross()
    n = product(f.shape)
    return cerode(f,g,Bc,n);


def bshow(f1, f2=None, f3=None, factor=17):
    """
    y = bshow(f1, f2=None, f3=None, factor=17)

    Generate a graphical representation of overlaid binary images.
    Does **not** actually display the image anywhere!

    Generate an expanded binary image as a graphical representation
    of up to three binary input images. The 1-pixels of the first
    image are represented by square contours, the pixels of the
    optional second image are represented by circles and for the
    third image they are represented by shaded squares. This
    function is useful to create graphical illustration of small
    images.

    Parameters
    ----------
    f1 :     Binary image.
    f2 :     Binary image. Default: None.
    f3 :     Binary image. Default: None.
    factor : Double Default: 17. Expansion factor for the output
                  image. Use odd values above 9.

    Returns
    -------
    y : Binary image.
    """
    import numpy
    from numpy import newaxis, zeros, resize, transpose, floor, arange, array

    if f1.shape == (): f1 = array([f1])
    if len(f1.shape) == 1: f1 = f1[newaxis,:]
    if f2 is not None and f1.shape != f2.shape or \
        f3 is not None and f1.shape != f3.shape or \
        f2 is not None and f3 is not None and f2.shape != f3.shape:
        raise ValueError, 'pymorph.bshow: arguments must have the same shape'
    s = factor
    if factor < 9: s = 9
    h,w = f1.shape
    y = zeros((s*h, s*w),numpy.int32)
    xc = resize(range(s), (s,s))
    yc = transpose(xc)
    r  = int(floor((s-8)/2. + 0.5))
    circle = (xc - s//2)**2 + (yc - s//2)**2 <= r**2
    r = arange(s) % 2
    fillrect = resize(array([r, 1-r]), (s,s))
    fillrect[ 0 , : ] = 0
    fillrect[s-1, : ] = 0
    fillrect[ : , 0 ] = 0
    fillrect[ : ,s-1] = 0
    for i in xrange(h):
        for j in xrange(w):
            m, n = s*i, s*j
            if f1 and f1[i,j]:
                y[m     ,n:n+s] = 1
                y[m+s-1 ,n:n+s] = 1
                y[m:m+s ,n    ] = 1
                y[m:m+s ,n+s-1] = 1
            if f2 and f2[i,j]:
                y[m:m+s, n:n+s] += circle
            if f3 and f3[i,j]:
                y[m:m+s, n:n+s] += fillrect
    return (y > 0)


def symdiff(f1, f2):
    """
    y = symdiff(f1, f2)

    Symmetric difference between two images

    `symdiff` creates the image y by taken the union of the subtractions of `f1`
    from `f2` and `f2` from `f1`. When `f1` and `f2` are binary images, y
    represents the set of points that are in `f1` and not in `f2` or that are
    in `f2` and not in `f1`.

    Parameters
    ----------
    f1 : Gray-scale (uint8 or uint16) or binary image.
    f2 : Gray-scale (uint8 or uint16) or binary image.

    Returns
    -------
    y : Image
    """
    """
        - Examples
            #
            #   example 1
            #
            a = to_uint8([1, 2, 3, 4, 5])
            b = to_uint8([5, 4, 3, 2, 1])
            print symdiff(a,b)
            #
            #   example 2
            #
            c = readgray('tplayer1.tif')
            d = readgray('tplayer2.tif')
            e = symdiff(c,d)
            show(c)
            show(d)
            show(e)
    """
    return union(subm(f1,f2),subm(f2,f1))
symdif = symdiff


def thick(f, Iab=None, n=-1, theta=45, direction="clockwise"):
    """
    y = thick(f, Iab=None, n=-1, theta=45, direction="clockwise")

    Image transformation by thickening.

    `thick` creates the binary image `y` by performing a thickening of the
    binary image `f`. The number of iterations of the thickening is `n` and
    each iteration is performed by union of `f` with the points that are
    detected in f by the hit-miss operators characterized by rotations of theta
    degrees of the interval `Iab`.

    Parameters
    ----------
    f :         Binary image.
    Iab :       Interval Default: None (homothick).
    n :         Number of iterations. Default: -1, i.e., infinite
    theta :     Degrees of rotation: 45 (default), 90, or 180.
    direction : One of ('clockwise', 'anti-clockwise').

    Returns
    -------
    y : Binary image.
    """
    from numpy import product
    if Iab is None: Iab = homothick()
    assert isbinary(f),'f must be binary image'
    if n == -1: n = product(f.shape)
    y = f
    zero = intersec(f,0)
    for i in xrange(n):
        aux = zero
        for t in xrange(0,360,theta):
            sup = supgen(y, interot(Iab, t, direction))
            aux = union(aux, sup)
            y = union(y, sup)
        if isequal(aux,zero): break
    return y


def thin(f, Iab=None, n=-1, theta=45, direction="clockwise"):
    """
    y = thin(f, Iab={Homothin}, n={infinite}, theta=45, direction="clockwise")

    Image transformation by thinning.

    `thin` creates the binary image `y` by performing a thinning of the binary
    image `f`. The number of iterations of the thinning is n and each iteration
    is performed by subtracting the points that are detect in f by hit-miss
    operators characterized by rotations of theta of the interval `Iab`. When
    `n` is infinite and the interval is homothin (default conditions), thin
    gives the skeleton by thinning.

    Parameters
    ----------
    f :         Binary image.
    Iab :       Interval Default:(homothin).
    n :         Number of iterations. (default: -1, i.e. infinite)
    theta :     Double Default: 45. Degrees of rotation: 45, 90, or
                180.
    direction : One of ('clockwise', 'anti-clockwise'). default: clockwise.

    Returns
    -------
    y : Binary image.
    """
    """
    - Examples
      #
      f=readgray('scissors.tif')
      f1=thin(f)
      show(f,f1) # skeleton
      f2=thin(f1,endpoints(),15) # prunning 15 pixels
      show(f,f2) # prunned skeleton
    """
    from numpy import product
    from string import lower
    if Iab is None: Iab = homothin()
    direction = lower(direction)
    assert isbinary(f),'f must be binary image'
    if n == -1: n = product(f.shape)
    y = f
    zero = intersec(f,0)
    for i in xrange(n):
        aux = zero
        for t in xrange(0,360,theta):
            sup = supgen(y, interot(Iab, t, direction))
            aux = union(aux, sup)
            y = subm(y, sup)
        if isequal(aux,zero): break
    return y


def union(f1, f2, *args):
    """
    y = union(f1, f2, f3=None, f4=None, f5=None, ...)

    Union of images.

    `union` creates the image `y` by taking the pixelwise maximum
    between the images given. When the images are binary images,
    y represents the union of them.

    Parameters
    ----------
    f1 : Gray-scale (uint8 or uint16) or binary image.
    f2 : Gray-scale (uint8 or uint16) or binary image. Or constant
    args : Gray-scale (uint8 or uint16) or binary images.

    Returns
    -------
    y : Image
    """
    """
        - Examples
            #
            #   example 1
            #
            f=to_uint8([255, 255,  0,  10,   0, 255, 250])
            print 'f=',f
            g=to_uint8([  0,  40, 80, 140, 250,  10,  30])
            print 'g=',g
            print union(f, g)
            print union(f, 255)
            #
            #   example 2
            #
            a = readgray('form-ok.tif')
            b = readgray('form-1.tif')
            c = union(a,b)
            show(a)
            show(b)
            show(c)
            #
            #   example 3
            #
            d = readgray('danaus.tif')
            e = (d < 80)
            f = union(d,gray(e))
            show(d)
            show(e)
            show(f)
            #
            #   example 4
            #
            g = readgray('tplayer1.tif')
            h = readgray('tplayer2.tif')
            i = readgray('tplayer3.tif')
            j = union(g,h,i)
            show(g)
            show(h)
            show(i)
            show(j)
    """
    from numpy import maximum

    y = maximum(f1,f2)
    for f in args:
        y = maximum(y,f)
    return y.astype(f1.dtype)


def watershed(f, Bc=None, return_lines=False):
    """
    W = watershed(f, Bc={3x3 cross}, return_lines=False)
    W,Wl = watershed(f, Bc={3x3 cross}, return_lines=True)

    Watershed transform.

    `watershed` creates the image y by detecting the domain of the
    catchment basins of `f`, according to the connectivity defined by
    `Bc`.

    The implementation of this function is based on VincSoil:91.

    Parameters
    ----------
    f :  Gray-scale (uint8 or uint16) or binary image.
    return_lines : boolean
            Whether to return the boundaries (default: False)

    Returns
    -------
    W : labeled image
    Wl : separation lines
    """
    from string import upper
    if Bc is None: Bc = secross()
    return cwatershed(f,regmin(f,Bc),Bc,return_lines=return_lines)


def maxleveltype(type='uint8'):
    """
    max = maxleveltype(type='uint8')

    Returns the maximum value supported by a datatype

    Parameters
    ----------
    type : { 'uint8' [default], 'uint16', 'int32'}

    Returns
    -------
    max : numeric value
        The maximum level value of `type`
    """
    if type == 'uint8': return 255
    if type == 'binary': return 1
    if type == 'uint16': return 65535
    if type == 'int32': return 2147483647
    assert False, 'pymorph.maxleveltype: does not support this data type:'+TYPE


def to_int32(f):
    """
    Convert an image to an int32 image.

    img = to_int32(f)

    `to_int32` clips the input image between the values -2147483648 and
    2147483647 and converts it to the signed 32-bit datatype.

    Parameters
    ----------
    f : Any image

    Returns
    -------
    img : The converted image
    """
    from numpy import int32, asanyarray
    return asanyarray(f).astype(int32)


def to_uint8(f):
    """
    img = to_uint8(f)

    Convert an image to an uint8 image.

    `to_uint8` clips the input image between the values 0 and 255 and
    converts it to the unsigned 8-bit datatype.

    Parameters
    ----------
    f : Any image

    Returns
    -------
    img : Gray-scale uint8 image. The converted image

    Examples
    --------
    ::

        print to_uint8([-3, 0, 8, 600])

    prints out

    ::

        [  0   0   8 255]
    """
    from numpy import array, clip, uint8

    img = array(clip(f,0,255),uint8)
    return img


def to_uint16(f):
    """
    img = to_uint16(f)

    Convert an image to a uint16 image.

    `to_uint16` clips the input image between the values 0 and 65535 and
    converts it to the unsigned 16-bit datatype.

    Parameters
    ----------
    f : Any image

    Returns
    -------
    img : The converted image
    """
    from numpy import array, clip

    img = array(clip(f,0,65535)).astype('H')
    return img


def to_gray(f):
    """
    g = to_gray(f)

    Transform (possibly colour) image to gray image.

    If input file is a color RGB image, it is converted to gray-scale using the
    equation:

        I = 0.2989 * R + 0.587 * G + 0.114 * B

    Parameters
    ----------
    f : ndarray of shape HxWx3
        colour image.

    Returns
    -------
    g : gray image of shape HxW same type as input
    """
    import numpy as np

    if len(f.shape) == 2:
        return f
    if len(f.shape) != 3:
        raise ValueError, 'pymorph.to_gray: can only handle gray and colour images'
    r = f[:,:,0]
    g = f[:,:,1]
    b = f[:,:,2]
    if np.all( (r == g) & (r == b) ):
        return r
    g = (0.2989 * r + 0.5870 * g  +  0.1140 * b)
    return g.astype(f.dtype)


def datatype(f):
    """
    type = datatype(f)

    Return the image datatype string

    datatype returns a string that identifies the pixel datatype
    of the image `f`.

    Consider using f.dtype.

    Parameters
    ----------
    f : Unsigned gray-scale (uint8 or uint16), signed (int32) or binary image
        Any image

    Returns
    -------
    type : str
        String representation of image type: 'binary',
        'uint8', 'uint16' or 'int32'
    """
    from numpy import bool, uint8, uint16, int32
    code = f.dtype
    if   code == bool: type='binary'
    elif code == uint8: type='uint8'
    elif code == uint16: type='uint16'
    elif code == int32: type='int32'
    else:
        assert 0,'Does not accept this typecode: %s' % code
    return type


def add4dilate(f, c):
    """
    a = add4dilate(f, c)

    Addition for dilation. Assumes that the minimum value of dtype(f) is
    to represent -inf (where -inf + c == -inf, for all c)

    Parameters
    ----------
    f : Gray-scale or binary image.
    c : Gray-scale, binary image, or constant.

    Returns
    -------
    a : f + c (but with correct implementation of -inf)
    """
    from numpy import asarray, minimum, maximum, float64

    if not c:
        return f
    y = asarray(f,float64) + c
    k1,k2 = limits(f)
    y = ((f==k1) * k1) + ((f!=k1) * y)
    y = maximum(minimum(y,k2),k1)
    return y.astype(f.dtype)


def mat2set(A):
    """
    C,V = mat2set(A)

    Converts image representation from matrix to set

    Return tuple with array of pixel coordinates and array of
    corresponding pixel values. The input image is in the matrix
    format, like the structuring element, where the origin (0,0) is
    at the center of the matrix.

    Parameters
    ----------
    A : ndarray
        Image in matrix format, where the origin (0,0) is at the
        center of the matrix.

    Returns
    -------
    C : array of pixel coordinates
    V : array of pixel values corresponding to the coordinates of C

    See also
    --------
    set2mat : reverse function
    """
    from numpy import take, ravel, nonzero, transpose, newaxis

    if len(A.shape) == 1: A = A[newaxis,:]
    offsets = nonzero(ravel(A) - limits(A)[0])[0]
    if len(offsets) == 0: return ([],[])
    h,w = A.shape
    x = [0,1]
    x[0] = offsets//w - (h-1)//2
    x[1] = offsets%w - (w-1)//2
    x = transpose(x)
    return x,take(ravel(A),offsets)


def set2mat(A):
    """
    M = set2mat(A)

    Converts image representation from set to matrix

    Return an image in the matrix format built from a tuple of an
    array of pixel coordinates and a corresponding array of pixel
    values

    Parameters
    ----------
    A : tuple (ndarray, ndarray) or tuple (ndarray,)
        Tuple with array of pixel coordinates and optional array of
        corresponding pixel values

    Returns
    -------
    M : Image in matrix format, origin (0,0) at the matrix center


    Examples
    --------

    ::

        coords = to_int32([ [0,0] , [-1,0] , [1,1]])
        print set2mat( (coords,) )

    prints out

    ::

        [0,1,0]
        [0,1,0]
        [0,0,1]
    """
    from numpy import put, ones, ravel, shape, newaxis, array, asarray, max, int32
    import numpy as np

    if len(A) == 2:
        x, v = A
        v = asarray(v)
    elif len(A) == 1:
        x = A[0]
        v = np.ones((len(x),), np.uint8)
    else:
        raise TypeError, 'pymorph.set2mat: argument must be a tuple of length 1 or 2'
    if len(x) == 0:  return array([0]).astype(v.dtype)
    if len(x.shape) == 1: x = x[newaxis,:]
    dh,dw = abs(x).max(0)
    h,w = (2*dh)+1, (2*dw)+1
    M=ones((h,w),int32) * limits(v)[0]
    offset = x[:,0] * w + x[:,1] + (dh*w + dw)
    put(M,offset,v)
    return M.astype(v.dtype)

def pad4n(f, Bc, value, scale=1):
    """
    y = pad4n(f, Bc, value, scale=1)

    pads f with value so that Bc can be applied scaled by scale.

    Parameters
    ----------
    f :     Image
    Bc :    Structuring Element (connectivity).
    value : dtype
        value to pad borders with
    scale : (default=1).

    Returns
    -------
    y : The converted image
    """
    from numpy import ones, array

    if type(Bc) is not array:
      Bc = seshow(Bc)
    Bh, Bw = Bc.shape
    assert Bh%2 and Bw%2, 'structuring element must be odd sized'
    ch, cw = scale * Bh/2, scale * Bw/2
    g = value * ones( f.shape + scale * (array(Bc.shape) - 1))
    g[ ch: -ch, cw: -cw] = f
    return g.astype(f.dtype)

