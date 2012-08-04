"""
This contains functions that were not implemented (yet).

Consider them TODO items.
"""

def swatershed(f, g, B=None, LINEREG="LINES"):
    """
        - Purpose
            Detection of similarity-based watershed from markers.
        - Synopsis
            y = swatershed(f, g, B=None, LINEREG="LINES")
        - Input
            f:       Gray-scale (uint8 or uint16) image.
            g:       Gray-scale (uint8 or uint16) or binary image. Marker
                     image. If binary, each connected component is an object
                     marker. If gray, it is assumed it is a labeled image.
            B:       Structuring Element Default: None (3x3 elementary
                     cross). (watershed connectivity)
            LINEREG: String Default: "LINES". 'LINES' or ' REGIONS'.
        - Output
            y: Gray-scale (uint8 or uint16) or binary image.
        - Description
            swatershed creates the image y by detecting the domain of the
            catchment basins of f indicated by g , according with the
            connectivity defined by B . This watershed is a modified version
            where each basin is defined by a similarity criterion between
            pixels. The original watershed is normally applied to the
            gradient of the image. In this case, the gradient is taken
            internally. According to the flag LINEREG y will be a labeled
            image of the catchment basins domain or just a binary image that
            presents the watershed lines. The implementation of this
            function is based on LotuFalc:00 .
        - Examples
            #
            f = to_uint8([
                [0,  0,  0,  0,  0,  0,  0],
                [0,  1,  0,  0,  0,  1,  0],
                [0,  1,  0,  0,  0,  1,  0],
                [0,  1,  1,  1,  1,  1,  0],
                [0,  1,  0,  0,  0,  0,  0],
                [0,  0,  0,  0,  0,  0,  0]])
            m = to_uint8([
                [0,  0,  0,  0,  0,  0,  0],
                [0,  1,  0,  0,  0,  0,  0],
                [0,  0,  0,  0,  0,  0,  0],
                [0,  0,  0,  0,  0,  0,  0],
                [0,  0,  0,  0,  0,  0,  0],
                [0,  0,  0,  2,  0,  0,  0]])
            print swatershed(f,m,secross(),'REGIONS')
    """

    if B is None: B = secross()
    print 'Not implemented yet'
    return None
    return y


def vmax(f, v=1, Bc=None):
    """
    y = vmax(f, v=1, Bc={3x3 cross})

    Remove domes with volume less than v.

    This operator removes connected domes with volume less
    than `v`. This function is very similar to `hmax`, but instead
    of using a gray scale criterion (contrast) for the dome, it uses
    a volume criterion.

    Parameters
    ----------
      f :  Gray-scale (uint8 or uint16) image.
      v :  Volume parameter (default: 1).
      Bc : Structuring element (default: 3x3 cross).
    Returns
    -------
      y : Gray-scale (uint8 or uint16) or binary image.
    """

    if Bc is None: Bc = secross()
    raise NotImplementedError, 'Not implemented yet'


def flood(fin, T, option, Bc=None):
    """
    y = flood(fin, T, option, Bc=None)

    Flooding filter h,v,a-basin and dynamics (depth, area, volume)

    This is a flooding algorithm. It is the basis to implement many
    topological functions. It is a connected filter that floods an
    image following some topological criteria: area, volume, depth.
    These filters are equivalent to area-close, volume-basin or
    h-basin, respectively. This code may be difficult to understand
    because of its many options. Basically, when t is negative, the
    generalized dynamics: area, volume, h is computed. When the
    flooding is computed, every time a new level in the flooding
    happens, a test is made to verify if the criterion has reached.
    This is used to set the value to that height. This value image
    will be used later for sup-reconstruction (flooding) at that
    particular level. This test happens in the raising of the water
    and in the merging of basins.

    Parameters
    ----------
      fin :    Gray-scale image (uint8 or uint16).
      T :      Criterion value. If T==-1, then the dynamics is
                 determined, not the flooding at this criterion. This was
                 selected just to use the same algoritm to compute two
                 completely distinct functions.
      option : One of ('AREA', 'VOLUME', 'H').
      Bc :     Structuring element (default: 3x3 cross)

    Returns
    -------
      y : Gray-scale image (same type as input).
    """

    if Bc is None: Bc = secross()
    raise NotImplementedError, 'pymorph.flood'
