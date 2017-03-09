"""
Cython wrapper to provide python interfaces to
PROJ.4 (http://trac.osgeo.org/proj/) functions.

Performs cartographic transformations and geodetic computations.

The Proj class can convert from geographic (longitude,latitude)
to native map projection (x,y) coordinates and vice versa, or
from one map projection coordinate system directly to another.
The module variable pj_list is a dictionary containing all the
available projections and their descriptions.

The Geod class can perform forward and inverse geodetic, or
Great Circle, computations.  The forward computation involves
determining latitude, longitude and back azimuth of a terminus
point given the latitude and longitude of an initial point, plus
azimuth and distance. The inverse computation involves
determining the forward and back azimuths and distance given the
latitudes and longitudes of an initial and terminus point.

Input coordinates can be given as python arrays, lists/tuples,
scalars or numpy/Numeric/numarray arrays. Optimized for objects
that support the Python buffer protocol (regular python and
numpy array objects).

Download: http://python.org/pypi/pyproj

Requirements: python 2.4 or higher.

Example scripts are in 'test' subdirectory of source distribution.
The 'test()' function will run the examples in the docstrings.

Contact:  Jeffrey Whitaker <jeffrey.s.whitaker@noaa.gov

copyright (c) 2006 by Jeffrey Whitaker.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both the copyright notice and this permission
notice appear in supporting documentation. THE AUTHOR DISCLAIMS
ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT
SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. """

import sys
from pyproj import _proj
from pyproj.datadir import pyproj_datadir
__version__ =  _proj.__version__
set_datapath =  _proj.set_datapath
from array import array
import os, math
#import numpy as np

# Python 2/3 compatibility
if sys.version_info[0] == 2:	 	# Python 2
   string_types = (basestring,)
else: 			   		# Python 3
   string_types = (str,)

pj_list={
'aea': "Albers Equal Area",
'aeqd': "Azimuthal Equidistant",
'airy': "Airy",
'aitoff': "Aitoff",
'alsk': "Mod. Stererographics of Alaska",
'apian': "Apian Globular I",
'august': "August Epicycloidal",
'bacon': "Bacon Globular",
'bipc': "Bipolar conic of western hemisphere",
'boggs': "Boggs Eumorphic",
'bonne': "Bonne (Werner lat_1=90)",
'cass': "Cassini",
'cc': "Central Cylindrical",
'cea': "Equal Area Cylindrical",
'chamb': "Chamberlin Trimetric",
'collg': "Collignon",
'crast': "Craster Parabolic (Putnins P4)",
'denoy': "Denoyer Semi-Elliptical",
'eck1': "Eckert I",
'eck2': "Eckert II",
'eck3': "Eckert III",
'eck4': "Eckert IV",
'eck5': "Eckert V",
'eck6': "Eckert VI",
'eqc': "Equidistant Cylindrical (Plate Caree)",
'eqdc': "Equidistant Conic",
'etmerc': "Extended Transverse Mercator" ,
'euler': "Euler",
'fahey': "Fahey",
'fouc': "Foucaut",
'fouc_s': "Foucaut Sinusoidal",
'gall': "Gall (Gall Stereographic)",
'geocent': "Geocentric",
'geos': "Geostationary Satellite View",
'gins8': "Ginsburg VIII (TsNIIGAiK)",
'gn_sinu': "General Sinusoidal Series",
'gnom': "Gnomonic",
'goode': "Goode Homolosine",
'gs48': "Mod. Stererographics of 48 U.S.",
'gs50': "Mod. Stererographics of 50 U.S.",
'hammer': "Hammer & Eckert-Greifendorff",
'hatano': "Hatano Asymmetrical Equal Area",
'healpix': "HEALPix",
'rhealpix': "rHEALPix",
'igh':  "Interrupted Goode Homolosine",
'imw_p': "Internation Map of the World Polyconic",
'isea':  "Icosahedral Snyder Equal Area",
'kav5': "Kavraisky V",
'kav7': "Kavraisky VII",
'krovak': "Krovak",
'labrd': "Laborde",
'laea': "Lambert Azimuthal Equal Area",
'lagrng': "Lagrange",
'larr': "Larrivee",
'lask': "Laskowski",
'lonlat': "Lat/long (Geodetic)",
'latlon': "Lat/long (Geodetic alias)",
'latlong': "Lat/long (Geodetic alias)",
'longlat': "Lat/long (Geodetic alias)",
'lcc': "Lambert Conformal Conic",
'lcca': "Lambert Conformal Conic Alternative",
'leac': "Lambert Equal Area Conic",
'lee_os': "Lee Oblated Stereographic",
'loxim': "Loximuthal",
'lsat': "Space oblique for LANDSAT",
'mbt_s': "McBryde-Thomas Flat-Polar Sine",
'mbt_fps': "McBryde-Thomas Flat-Pole Sine (No. 2)",
'mbtfpp': "McBride-Thomas Flat-Polar Parabolic",
'mbtfpq': "McBryde-Thomas Flat-Polar Quartic",
'mbtfps': "McBryde-Thomas Flat-Polar Sinusoidal",
'merc': "Mercator",
'mil_os': "Miller Oblated Stereographic",
'mill': "Miller Cylindrical",
'moll': "Mollweide",
'murd1': "Murdoch I",
'murd2': "Murdoch II",
'murd3': "Murdoch III",
'natearth': "Natural Earth",
'nell': "Nell",
'nell_h': "Nell-Hammer",
'nicol': "Nicolosi Globular",
'nsper': "Near-sided perspective",
'nzmg': "New Zealand Map Grid",
'ob_tran': "General Oblique Transformation",
'ocea': "Oblique Cylindrical Equal Area",
'oea': "Oblated Equal Area",
'omerc': "Oblique Mercator",
'ortel': "Ortelius Oval",
'ortho': "Orthographic",
'pconic': "Perspective Conic",
'poly': "Polyconic (American)",
'putp1': "Putnins P1",
'putp2': "Putnins P2",
'putp3': "Putnins P3",
'putp3p': "Putnins P3'",
'putp4p': "Putnins P4'",
'putp5': "Putnins P5",
'putp5p': "Putnins P5'",
'putp6': "Putnins P6",
'putp6p': "Putnins P6'",
'qua_aut': "Quartic Authalic",
'robin': "Robinson",
'rouss': "Roussilhe Stereographic",
'rpoly': "Rectangular Polyconic",
'sinu': "Sinusoidal (Sanson-Flamsteed)",
'somerc': "Swiss. Obl. Mercator",
'stere': "Stereographic",
'sterea': "Oblique Stereographic Alternative",
'gstmerc': "Gauss-Schreiber Transverse Mercator (aka Gauss-Laborde Reunion)",
'tcc': "Transverse Central Cylindrical",
'tcea': "Transverse Cylindrical Equal Area",
'tissot': "Tissot Conic",
'tmerc': "Transverse Mercator",
'tpeqd': "Two Point Equidistant",
'tpers': "Tilted perspective",
'ups': "Universal Polar Stereographic",
'urm5': "Urmaev V",
'urmfps': "Urmaev Flat-Polar Sinusoidal",
'utm': "Universal Transverse Mercator (UTM)",
'vandg': "van der Grinten (I)",
'vandg2': "van der Grinten II",
'vandg3': "van der Grinten III",
'vandg4': "van der Grinten IV",
'vitk1': "Vitkovsky I",
'wag1': "Wagner I (Kavraisky VI)",
'wag2': "Wagner II",
'wag3': "Wagner III",
'wag4': "Wagner IV",
'wag5': "Wagner V",
'wag6': "Wagner VI",
'wag7': "Wagner VII",
'weren': "Werenskiold I",
'wink1': "Winkel I",
'wink2': "Winkel II",
'wintri': "Winkel Tripel"}

pj_ellps={
"MERIT":	{'a':6378137.0,'rf':298.257,'description':"MERIT 1983"},
"SGS85":	{'a':6378136.0,'rf':298.257,'description':"Soviet Geodetic System 85"},
"GRS80":	{'a':6378137.0,'rf':298.257222101,'description':"GRS 1980(IUGG, 1980)"},
"IAU76":	{'a':6378140.0,'rf':298.257,'description':"IAU 1976"},
"airy":		{'a':6377563.396,'b':6356256.910,'description':"Airy 1830"},
"APL4.9":	{'a':6378137.0,'rf':298.25,'description':"Appl. Physics. 1965"},
"NWL9D":	{'a':6378145.0,'rf':298.25,'description':" Naval Weapons Lab., 1965"},
"mod_airy":	{'a':6377340.189,'b':6356034.446,'description':"Modified Airy"},
"andrae":	{'a':6377104.43,'rf':300.0,'description':"Andrae 1876 (Den., Iclnd.)"},
"aust_SA":	{'a':6378160.0,'rf':298.25,'description':"Australian Natl & S. Amer. 1969"},
"GRS67":	{'a':6378160.0,'rf':298.2471674270,'description':"GRS 67(IUGG 1967)"},
"bessel":	{'a':6377397.155,'rf':299.1528128,'description':"Bessel 1841"},
"bess_nam":	{'a':6377483.865,'rf':299.1528128,'description':"Bessel 1841 (Namibia)"},
"clrk66":	{'a':6378206.4,'b':6356583.8,'description':"Clarke 1866"},
"clrk80":	{'a':6378249.145,'rf':293.4663,'description':"Clarke 1880 mod."},
"CPM":          {'a':6375738.7,'rf':334.29,'description':"Comm. des Poids et Mesures 1799"},
"delmbr":	{'a':6376428.,'rf':311.5,'description':"Delambre 1810 (Belgium)"},
"engelis":	{'a':6378136.05,'rf':298.2566,'description':"Engelis 1985"},
"evrst30":      {'a':6377276.345,'rf':300.8017,'description':"Everest 1830"},
"evrst48":      {'a':6377304.063,'rf':300.8017,'description':"Everest 1948"},
"evrst56":      {'a':6377301.243,'rf':300.8017,'description':"Everest 1956"},
"evrst69":      {'a':6377295.664,'rf':300.8017,'description':"Everest 1969"},
"evrstSS":      {'a':6377298.556,'rf':300.8017,'description':"Everest (Sabah & Sarawak)"},
"fschr60":      {'a':6378166.,'rf':298.3,'description':"Fischer (Mercury Datum) 1960"},
"fschr60m":     {'a':6378155.,'rf':298.3,'description':"Modified Fischer 1960"},
"fschr68":      {'a':6378150.,'rf':298.3,'description':"Fischer 1968"},
"helmert":      {'a':6378200.,'rf':298.3,'description':"Helmert 1906"},
"hough":	{'a':6378270.0,'rf':297.,'description':"Hough"},
"intl":		{'a':6378388.0,'rf':297.,'description':"International 1909 (Hayford)"},
"krass":	{'a':6378245.0,'rf':298.3,'description':"Krassovsky, 1942"},
"kaula":	{'a':6378163.,'rf':298.24,'description':"Kaula 1961"},
"lerch":	{'a':6378139.,'rf':298.257,'description':"Lerch 1979"},
"mprts":	{'a':6397300.,'rf':191.,'description':"Maupertius 1738"},
"new_intl":	{'a':6378157.5,'b':6356772.2,'description':"New International 1967"},
"plessis":	{'a':6376523.,'b':6355863.,'description':"Plessis 1817 (France)"},
"SEasia":	{'a':6378155.0,'b':6356773.3205,'description':"Southeast Asia"},
"walbeck":	{'a':6376896.0,'b':6355834.8467,'description':"Walbeck"},
"WGS60":        {'a':6378165.0,'rf':298.3,'description':"WGS 60"},
"WGS66":	{'a':6378145.0,'rf':298.25,'description':"WGS 66"},
"WGS72":	{'a':6378135.0,'rf':298.26,'description':"WGS 72"},
"WGS84":        {'a':6378137.0,'rf':298.257223563,'description':"WGS 84"},
"sphere":       {'a':6370997.0,'b':6370997.0,'description':"Normal Sphere"},
}

#if not os.path.isdir(pyproj_datadir):
#    msg="proj data directory not found. Expecting it at: %s"%pyproj_datadir
#    raise IOError(msg)

set_datapath(pyproj_datadir)

class Proj(_proj.Proj):
    """
    performs cartographic transformations (converts from
    longitude,latitude to native map projection x,y coordinates and
    vice versa) using proj (http://trac.osgeo.org/proj/).

    A Proj class instance is initialized with proj map projection
    control parameter key/value pairs. The key/value pairs can
    either be passed in a dictionary, or as keyword arguments,
    or as a proj4 string (compatible with the proj command). See
    http://www.remotesensing.org/geotiff/proj_list for examples of
    key/value pairs defining different map projections.

    Calling a Proj class instance with the arguments lon, lat will
    convert lon/lat (in degrees) to x/y native map projection
    coordinates (in meters).  If optional keyword 'inverse' is True
    (default is False), the inverse transformation from x/y to
    lon/lat is performed. If optional keyword 'radians' is True
    (default is False) lon/lat are interpreted as radians instead of
    degrees. If optional keyword 'errcheck' is True (default is
    False) an exception is raised if the transformation is invalid.
    If errcheck=False and the transformation is invalid, no
    exception is raised and 1.e30 is returned. If the optional keyword
    'preserve_units' is True, the units in map projection coordinates
    are not forced to be meters.

    Works with numpy and regular python array objects, python
    sequences and scalars.
    """

    def __new__(self, projparams=None, preserve_units=False, **kwargs):
        """
        initialize a Proj class instance.

        Proj4 projection control parameters must either be given in a
        dictionary 'projparams' or as keyword arguments. See the proj
        documentation (http://trac.osgeo.org/proj/) for more information
        about specifying projection parameters.

        Example usage:

        >>> from pyproj import Proj
        >>> p = Proj(proj='utm',zone=10,ellps='WGS84') # use kwargs
        >>> x,y = p(-120.108, 34.36116666)
        >>> 'x=%9.3f y=%11.3f' % (x,y)
        'x=765975.641 y=3805993.134'
        >>> 'lon=%8.3f lat=%5.3f' % p(x,y,inverse=True)
        'lon=-120.108 lat=34.361'
        >>> # do 3 cities at a time in a tuple (Fresno, LA, SF)
        >>> lons = (-119.72,-118.40,-122.38)
        >>> lats = (36.77, 33.93, 37.62 )
        >>> x,y = p(lons, lats)
        >>> 'x: %9.3f %9.3f %9.3f' % x
        'x: 792763.863 925321.537 554714.301'
        >>> 'y: %9.3f %9.3f %9.3f' % y
        'y: 4074377.617 3763936.941 4163835.303'
        >>> lons, lats = p(x, y, inverse=True) # inverse transform
        >>> 'lons: %8.3f %8.3f %8.3f' % lons
        'lons: -119.720 -118.400 -122.380'
        >>> 'lats: %8.3f %8.3f %8.3f' % lats
        'lats:   36.770   33.930   37.620'
        >>> p2 = Proj('+proj=utm +zone=10 +ellps=WGS84') # use proj4 string
        >>> x,y = p2(-120.108, 34.36116666)
        >>> 'x=%9.3f y=%11.3f' % (x,y)
        'x=765975.641 y=3805993.134'
        >>> p = Proj(init="epsg:32667")
        >>> 'x=%12.3f y=%12.3f (meters)' % p(-114.057222, 51.045)
        'x=-1783486.760 y= 6193833.196 (meters)'
        >>> p = Proj("+init=epsg:32667",preserve_units=True)
        >>> 'x=%12.3f y=%12.3f (feet)' % p(-114.057222, 51.045)
        'x=-5851322.810 y=20320934.409 (feet)'
        >>> p = Proj(proj='hammer') # hammer proj and inverse
        >>> x,y = p(-30,40)
        >>> 'x=%12.3f y=%12.3f' % (x,y)
        'x=-2711575.083 y= 4395506.619'
        >>> lon,lat = p(x,y,inverse=True)
        >>> 'lon=%9.3f lat=%9.3f (degrees)' % (lon,lat)
        'lon=  -30.000 lat=   40.000 (degrees)'
        """
        # if projparams is None, use kwargs.
        if projparams is None:
            if len(kwargs) == 0:
                raise RuntimeError('no projection control parameters specified')
            else:
                projstring = _dict2string(kwargs)
        elif isinstance(projparams, string_types):
            # if projparams is a string or a unicode string, interpret as a proj4 init string.
            projstring = projparams
        else: # projparams a dict
            projstring = _dict2string(projparams)
        # make sure units are meters if preserve_units is False.
        if not projstring.count('+units=') and not preserve_units:
            projstring = '+units=m '+projstring
        else:
            kvpairs = []
            for kvpair in projstring.split():
                if kvpair.startswith('+units') and not preserve_units:
                    k,v = kvpair.split('=')
                    kvpairs.append(k+'=m ')
                else:
                    kvpairs.append(kvpair+' ')
            projstring = ''.join(kvpairs)
        # look for EPSG, replace with epsg (EPSG only works
        # on case-insensitive filesystems).
        projstring = projstring.replace('EPSG','epsg')
        return _proj.Proj.__new__(self, projstring)

    def __call__(self, *args, **kw):
    #,lon,lat,inverse=False,radians=False,errcheck=False):
        """
        Calling a Proj class instance with the arguments lon, lat will
        convert lon/lat (in degrees) to x/y native map projection
        coordinates (in meters).  If optional keyword 'inverse' is True
        (default is False), the inverse transformation from x/y to
        lon/lat is performed.  If optional keyword 'radians' is True
        (default is False) the units of lon/lat are radians instead of
        degrees. If optional keyword 'errcheck' is True (default is
        False) an exception is raised if the transformation is invalid.
        If errcheck=False and the transformation is invalid, no
        exception is raised and 1.e30 is returned.

        Inputs should be doubles (they will be cast to doubles if they
        are not, causing a slight performance hit).

        Works with numpy and regular python array objects, python
        sequences and scalars, but is fastest for array objects.
        """
        inverse = kw.get('inverse', False)
        radians = kw.get('radians', False)
        errcheck = kw.get('errcheck', False)
        #if len(args) == 1:
        #    latlon = np.array(args[0], copy=True,
        #                      order='C', dtype=float, ndmin=2)
        #    if inverse:
        #        _proj.Proj._invn(self, latlon, radians=radians, errcheck=errcheck)
        #    else:
        #        _proj.Proj._fwdn(self, latlon, radians=radians, errcheck=errcheck)
        #    return latlon
        lon, lat = args
        # process inputs, making copies that support buffer API.
        inx, xisfloat, xislist, xistuple = _copytobuffer(lon)
        iny, yisfloat, yislist, yistuple = _copytobuffer(lat)
        # call proj4 functions. inx and iny modified in place.
        if inverse:
            _proj.Proj._inv(self, inx, iny, radians=radians, errcheck=errcheck)
        else:
            _proj.Proj._fwd(self, inx, iny, radians=radians, errcheck=errcheck)
        # if inputs were lists, tuples or floats, convert back.
        outx = _convertback(xisfloat,xislist,xistuple,inx)
        outy = _convertback(yisfloat,yislist,xistuple,iny)
        return outx, outy

    def to_latlong(self):
        """returns an equivalent Proj in the corresponding lon/lat
        coordinates. (see pj_latlong_from_proj() in the Proj.4 C API)"""
        return _proj.Proj.to_latlong(self)

    def is_latlong(self):
        """returns True if projection in geographic (lon/lat) coordinates"""
        return _proj.Proj.is_latlong(self)

    def is_geocent(self):
        """returns True if projection in geocentric (x/y) coordinates"""
        return _proj.Proj.is_geocent(self)

def transform(p1, p2, x, y, z=None, radians=False):
    """
    x2, y2, z2 = transform(p1, p2, x1, y1, z1, radians=False)

    Transform points between two coordinate systems defined by the
    Proj instances p1 and p2.

    The points x1,y1,z1 in the coordinate system defined by p1 are
    transformed to x2,y2,z2 in the coordinate system defined by p2.

    z1 is optional, if it is not set it is assumed to be zero (and
    only x2 and y2 are returned).

    In addition to converting between cartographic and geographic
    projection coordinates, this function can take care of datum
    shifts (which cannot be done using the __call__ method of the
    Proj instances). It also allows for one of the coordinate
    systems to be geographic (proj = 'latlong').

    If optional keyword 'radians' is True (default is False) and p1
    is defined in geographic coordinate (pj.is_latlong() is True),
    x1,y1 is interpreted as radians instead of the default degrees.
    Similarly, if p2 is defined in geographic coordinates and
    radians=True, x2, y2 are returned in radians instead of degrees.
    if p1.is_latlong() and p2.is_latlong() both are False, the
    radians keyword has no effect.

    x,y and z can be numpy or regular python arrays, python
    lists/tuples or scalars. Arrays are fastest.  For projections in
    geocentric coordinates, values of x and y are given in meters.
    z is always meters.

    Example usage:

    >>> # projection 1: UTM zone 15, grs80 ellipse, NAD83 datum
    >>> # (defined by epsg code 26915)
    >>> p1 = Proj(init='epsg:26915')
    >>> # projection 2: UTM zone 15, clrk66 ellipse, NAD27 datum
    >>> p2 = Proj(init='epsg:26715')
    >>> # find x,y of Jefferson City, MO.
    >>> x1, y1 = p1(-92.199881,38.56694)
    >>> # transform this point to projection 2 coordinates.
    >>> x2, y2 = transform(p1,p2,x1,y1)
    >>> '%9.3f %11.3f' % (x1,y1)
    '569704.566 4269024.671'
    >>> '%9.3f %11.3f' % (x2,y2)
    '569722.342 4268814.027'
    >>> '%8.3f %5.3f' % p2(x2,y2,inverse=True)
    ' -92.200 38.567'
    >>> # process 3 points at a time in a tuple
    >>> lats = (38.83,39.32,38.75) # Columbia, KC and StL Missouri
    >>> lons = (-92.22,-94.72,-90.37)
    >>> x1, y1 = p1(lons,lats)
    >>> x2, y2 = transform(p1,p2,x1,y1)
    >>> xy = x1+y1
    >>> '%9.3f %9.3f %9.3f %11.3f %11.3f %11.3f' % xy
    '567703.344 351730.944 728553.093 4298200.739 4353698.725 4292319.005'
    >>> xy = x2+y2
    >>> '%9.3f %9.3f %9.3f %11.3f %11.3f %11.3f' % xy
    '567721.149 351747.558 728569.133 4297989.112 4353489.644 4292106.305'
    >>> lons, lats = p2(x2,y2,inverse=True)
    >>> xy = lons+lats
    >>> '%8.3f %8.3f %8.3f %5.3f %5.3f %5.3f' % xy
    ' -92.220  -94.720  -90.370 38.830 39.320 38.750'
    >>> # test datum shifting, installation of extra datum grid files.
    >>> p1 = Proj(proj='latlong',datum='WGS84')
    >>> x1 = -111.5; y1 = 45.25919444444
    >>> p2 = Proj(proj="utm",zone=10,datum='NAD27')
    >>> x2, y2 = transform(p1, p2, x1, y1)
    >>> "%s  %s" % (str(x2)[:9],str(y2)[:9])
    '1402285.9  5076292.4'
    """
    # check that p1 and p2 are from the Proj class
    if not isinstance(p1, Proj):
        raise TypeError("p1 must be a Proj class")
    if not isinstance(p2, Proj):
        raise TypeError("p2 must be a Proj class")

    # process inputs, making copies that support buffer API.
    inx, xisfloat, xislist, xistuple = _copytobuffer(x)
    iny, yisfloat, yislist, yistuple = _copytobuffer(y)
    if z is not None:
        inz, zisfloat, zislist, zistuple = _copytobuffer(z)
    else:
        inz = None
    # call pj_transform.  inx,iny,inz buffers modified in place.
    _proj._transform(p1,p2,inx,iny,inz,radians)
    # if inputs were lists, tuples or floats, convert back.
    outx = _convertback(xisfloat,xislist,xistuple,inx)
    outy = _convertback(yisfloat,yislist,xistuple,iny)
    if inz is not None:
        outz = _convertback(zisfloat,zislist,zistuple,inz)
        return outx, outy, outz
    else:
        return outx, outy

def _copytobuffer_return_scalar(x):
    try:
        # inx,isfloat,islist,istuple
        return array('d',(float(x),)),True,False,False
    except:
        raise TypeError('input must be an array, list, tuple or scalar')

def _copytobuffer(x):
    """
    return a copy of x as an object that supports the python Buffer
    API (python array if input is float, list or tuple, numpy array
    if input is a numpy array). returns copyofx, isfloat, islist,
    istuple (islist is True if input is a list, istuple is true if
    input is a tuple, isfloat is true if input is a float).
    """
    # make sure x supports Buffer API and contains doubles.
    isfloat = False; islist = False; istuple = False
    # first, if it's a numpy array scalar convert to float
    # (array scalars don't support buffer API)
    if hasattr(x,'shape'):
        if x.shape == ():
            return _copytobuffer_return_scalar(x)
        else:
            try:
                # typecast numpy arrays to double.
                # (this makes a copy - which is crucial
                #  since buffer is modified in place)
                x.dtype.char
                # Basemap issue
                # https://github.com/matplotlib/basemap/pull/223/files
                # (deal with input array in fortran order)
                inx = x.copy(order="C").astype('d')
                # inx,isfloat,islist,istuple
                return inx,False,False,False
            except:
                try: # perhaps they are Numeric/numarrays?
                    # sorry, not tested yet.
                    # i don't know Numeric/numarrays has `shape'.
                    x.typecode()
                    inx = x.astype('d')
                    # inx,isfloat,islist,istuple
                    return inx,False,False,False
                except:
                    raise TypeError('input must be an array, list, tuple or scalar')
    else:
        # perhaps they are regular python arrays?
        if hasattr(x, 'typecode'):
            #x.typecode
            inx = array('d',x)
        # try to convert to python array
        # a list.
        elif type(x) == list:
            inx = array('d',x)
            islist = True
        # a tuple.
        elif type(x) == tuple:
            inx = array('d',x)
            istuple = True
        # a scalar?
        else:
            return _copytobuffer_return_scalar(x)
    return inx,isfloat,islist,istuple

def _convertback(isfloat,islist,istuple,inx):
    # if inputs were lists, tuples or floats, convert back to original type.
    if isfloat:
        return inx[0]
    elif islist:
        return inx.tolist()
    elif istuple:
        return tuple(inx)
    else:
        return inx

def _dict2string(projparams):
    # convert a dict to a proj4 string.
    pjargs = []
    for key,value in projparams.items():
        pjargs.append('+'+key+"="+str(value)+' ')
    return ''.join(pjargs)

class Geod(_proj.Geod):
    """
    performs forward and inverse geodetic, or Great Circle,
    computations.  The forward computation (using the 'fwd' method)
    involves determining latitude, longitude and back azimuth of a
    computations.  The forward computation (using the 'fwd' method)
    involves determining latitude, longitude and back azimuth of a
    terminus point given the latitude and longitude of an initial
    point, plus azimuth and distance. The inverse computation (using
    the 'inv' method) involves determining the forward and back
    azimuths and distance given the latitudes and longitudes of an
    initial and terminus point.
    """
    def __new__(self, initstring=None, **kwargs):
        """
        initialize a Geod class instance.

        Geodetic parameters for specifying the ellipsoid
        can be given in a dictionary 'initparams', as keyword arguments,
        or as as proj4 geod initialization string.
        Following is a list of the ellipsoids that may be defined using the
        'ellps' keyword (these are stored in the model variable pj_ellps)::

           MERIT a=6378137.0      rf=298.257       MERIT 1983
           SGS85 a=6378136.0      rf=298.257       Soviet Geodetic System 85
           GRS80 a=6378137.0      rf=298.257222101 GRS 1980(IUGG, 1980)
           IAU76 a=6378140.0      rf=298.257       IAU 1976
           airy a=6377563.396     b=6356256.910    Airy 1830
           APL4.9 a=6378137.0.    rf=298.25        Appl. Physics. 1965
           airy a=6377563.396     b=6356256.910    Airy 1830
           APL4.9 a=6378137.0.    rf=298.25        Appl. Physics. 1965
           NWL9D a=6378145.0.     rf=298.25        Naval Weapons Lab., 1965
           mod_airy a=6377340.189 b=6356034.446    Modified Airy
           andrae a=6377104.43    rf=300.0         Andrae 1876 (Den., Iclnd.)
           aust_SA a=6378160.0    rf=298.25        Australian Natl & S. Amer. 1969
           GRS67 a=6378160.0      rf=298.247167427 GRS 67(IUGG 1967)
           bessel a=6377397.155   rf=299.1528128   Bessel 1841
           bess_nam a=6377483.865 rf=299.1528128   Bessel 1841 (Namibia)
           clrk66 a=6378206.4     b=6356583.8      Clarke 1866
           clrk80 a=6378249.145   rf=293.4663      Clarke 1880 mod.
           CPM a=6375738.7        rf=334.29        Comm. des Poids et Mesures 1799
           delmbr a=6376428.      rf=311.5         Delambre 1810 (Belgium)
           engelis a=6378136.05   rf=298.2566      Engelis 1985
           evrst30 a=6377276.345  rf=300.8017      Everest 1830
           evrst48 a=6377304.063  rf=300.8017      Everest 1948
           evrst56 a=6377301.243  rf=300.8017      Everest 1956
           evrst69 a=6377295.664  rf=300.8017      Everest 1969
           evrstSS a=6377298.556  rf=300.8017      Everest (Sabah & Sarawak)
           fschr60 a=6378166.     rf=298.3         Fischer (Mercury Datum) 1960
           fschr60m a=6378155.    rf=298.3         Modified Fischer 1960
           fschr68 a=6378150.     rf=298.3         Fischer 1968
           helmert a=6378200.     rf=298.3         Helmert 1906
           hough a=6378270.0      rf=297.          Hough
           helmert a=6378200.     rf=298.3         Helmert 1906
           hough a=6378270.0      rf=297.          Hough
           intl a=6378388.0       rf=297.          International 1909 (Hayford)
           krass a=6378245.0      rf=298.3         Krassovsky, 1942
           kaula a=6378163.       rf=298.24        Kaula 1961
           lerch a=6378139.       rf=298.257       Lerch 1979
           mprts a=6397300.       rf=191.          Maupertius 1738
           new_intl a=6378157.5   b=6356772.2      New International 1967
           plessis a=6376523.     b=6355863.       Plessis 1817 (France)
           SEasia a=6378155.0     b=6356773.3205   Southeast Asia
           walbeck a=6376896.0    b=6355834.8467   Walbeck
           WGS60 a=6378165.0      rf=298.3         WGS 60
           WGS66 a=6378145.0      rf=298.25        WGS 66
           WGS72 a=6378135.0      rf=298.26        WGS 72
           WGS84 a=6378137.0      rf=298.257223563 WGS 84
           sphere a=6370997.0     b=6370997.0      Normal Sphere (r=6370997)

        The parameters of the ellipsoid may also be set directly using
        the 'a' (semi-major or equatorial axis radius) keyword, and
        any one of the following keywords: 'b' (semi-minor,
        or polar axis radius), 'e' (eccentricity), 'es' (eccentricity
        squared), 'f' (flattening), or 'rf' (reciprocal flattening).

        See the proj documentation (http://trac.osgeo.org/proj/) for more

        See the proj documentation (http://trac.osgeo.org/proj/) for more
        information about specifying ellipsoid parameters (specifically,
        the chapter 'Specifying the Earth's figure' in the main Proj
        users manual).

        Example usage:

        >>> from pyproj import Geod
        >>> g = Geod(ellps='clrk66') # Use Clarke 1966 ellipsoid.
        >>> # specify the lat/lons of some cities.
        >>> boston_lat = 42.+(15./60.); boston_lon = -71.-(7./60.)
        >>> portland_lat = 45.+(31./60.); portland_lon = -123.-(41./60.)
        >>> newyork_lat = 40.+(47./60.); newyork_lon = -73.-(58./60.)
        >>> london_lat = 51.+(32./60.); london_lon = -(5./60.)
        >>> # compute forward and back azimuths, plus distance
        >>> # between Boston and Portland.
        >>> az12,az21,dist = g.inv(boston_lon,boston_lat,portland_lon,portland_lat)
        >>> "%7.3f %6.3f %12.3f" % (az12,az21,dist)
        '-66.531 75.654  4164192.708'
        >>> # compute latitude, longitude and back azimuth of Portland,
        >>> # given Boston lat/lon, forward azimuth and distance to Portland.
        >>> endlon, endlat, backaz = g.fwd(boston_lon, boston_lat, az12, dist)
        >>> "%6.3f  %6.3f %13.3f" % (endlat,endlon,backaz)
        '45.517  -123.683        75.654'
        >>> # compute the azimuths, distances from New York to several
        >>> # cities (pass a list)
        >>> lons1 = 3*[newyork_lon]; lats1 = 3*[newyork_lat]
        >>> lons2 = [boston_lon, portland_lon, london_lon]
        >>> lats2 = [boston_lat, portland_lat, london_lat]
        >>> az12,az21,dist = g.inv(lons1,lats1,lons2,lats2)
        >>> for faz,baz,d in list(zip(az12,az21,dist)): "%7.3f %7.3f %9.3f" % (faz,baz,d)
        ' 54.663 -123.448 288303.720'
        '-65.463  79.342 4013037.318'
        ' 51.254 -71.576 5579916.651'
        >>> g2 = Geod('+ellps=clrk66') # use proj4 style initialization string
        >>> az12,az21,dist = g2.inv(boston_lon,boston_lat,portland_lon,portland_lat)
        >>> "%7.3f %6.3f %12.3f" % (az12,az21,dist)
        '-66.531 75.654  4164192.708'
        """
        # if initparams is a proj-type init string,
        # convert to dict.
        ellpsd = {}
        if initstring is not None:
            for kvpair in initstring.split():
                # Actually only +a and +b are needed
                # We can ignore safely any parameter that doesn't have a value
                if kvpair.find('=') == -1:
                    continue
                k,v = kvpair.split('=')
                k = k.lstrip('+')
                if k in ['a','b','rf','f','es','e']:
                    v = float(v)
                ellpsd[k] = v
        # merge this dict with kwargs dict.
        kwargs = dict(list(kwargs.items()) + list(ellpsd.items()))
        self.sphere = False
        if 'ellps' in kwargs:
            # ellipse name given, look up in pj_ellps dict
            ellps_dict = pj_ellps[kwargs['ellps']]
            a = ellps_dict['a']
            if ellps_dict['description']=='Normal Sphere':
                self.sphere = True
            if 'b' in ellps_dict:
                b = ellps_dict['b']
                es = 1. - (b * b) / (a * a)
                f = (a - b)/a
            elif 'rf' in ellps_dict:
                f = 1./ellps_dict['rf']
                b = a*(1. - f)
                es = 1. - (b * b) / (a * a)
        else:
            # a (semi-major axis) and one of
            # b the semi-minor axis
            # rf the reciprocal flattening
            # f flattening
            # es eccentricity squared
            # must be given.
            a = kwargs['a']
            if 'b' in kwargs:
                b = kwargs['b']
                es = 1. - (b * b) / (a * a)
                f = (a - b)/a
            elif 'rf' in kwargs:
                f = 1./kwargs['rf']
                b = a*(1. - f)
                es = 1. - (b * b) / (a * a)
            elif 'f' in kwargs:
                f = kwargs['f']
                b = a*(1. - f)
                es = 1. - (b/a)**2
            elif 'es' in kwargs:
                es = kwargs['es']
                b = math.sqrt(a**2 - es*a**2)
                f = (a - b)/a
            elif 'e' in kwargs:
                es = kwargs['e']**2
                b = math.sqrt(a**2 - es*a**2)
                f = (a - b)/a
            else:
                b = a
                f = 0.
                es = 0.
                #msg='ellipse name or a, plus one of f,es,b must be given'
                #raise ValueError(msg)
        if math.fabs(f) < 1.e-8: self.sphere = True
        self.a = a
        self.b = b
        self.f = f
        self.es = es
        return _proj.Geod.__new__(self, a, f)

    def fwd(self, lons, lats, az, dist, radians=False):
        """
        forward transformation - Returns longitudes, latitudes and back
        azimuths of terminus points given longitudes (lons) and
        latitudes (lats) of initial points, plus forward azimuths (az)
        and distances (dist).
        latitudes (lats) of initial points, plus forward azimuths (az)
        and distances (dist).

        Works with numpy and regular python array objects, python
        sequences and scalars.

        if radians=True, lons/lats and azimuths are radians instead of
        degrees. Distances are in meters.
        """
        # process inputs, making copies that support buffer API.
        inx, xisfloat, xislist, xistuple = _copytobuffer(lons)
        iny, yisfloat, yislist, yistuple = _copytobuffer(lats)
        inz, zisfloat, zislist, zistuple = _copytobuffer(az)
        ind, disfloat, dislist, distuple = _copytobuffer(dist)
        _proj.Geod._fwd(self, inx, iny, inz, ind, radians=radians)
        # if inputs were lists, tuples or floats, convert back.
        outx = _convertback(xisfloat,xislist,xistuple,inx)
        outy = _convertback(yisfloat,yislist,xistuple,iny)
        outz = _convertback(zisfloat,zislist,zistuple,inz)
        return outx, outy, outz

    def inv(self,lons1,lats1,lons2,lats2,radians=False):
        """
        inverse transformation - Returns forward and back azimuths, plus
        distances between initial points (specified by lons1, lats1) and
        terminus points (specified by lons2, lats2).

        Works with numpy and regular python array objects, python
        sequences and scalars.

        if radians=True, lons/lats and azimuths are radians instead of
        degrees. Distances are in meters.
        """
        # process inputs, making copies that support buffer API.
        inx, xisfloat, xislist, xistuple = _copytobuffer(lons1)
        iny, yisfloat, yislist, yistuple = _copytobuffer(lats1)
        inz, zisfloat, zislist, zistuple = _copytobuffer(lons2)
        ind, disfloat, dislist, distuple = _copytobuffer(lats2)
        _proj.Geod._inv(self,inx,iny,inz,ind,radians=radians)
        # if inputs were lists, tuples or floats, convert back.
        outx = _convertback(xisfloat,xislist,xistuple,inx)
        outy = _convertback(yisfloat,yislist,xistuple,iny)
        outz = _convertback(zisfloat,zislist,zistuple,inz)
        return outx, outy, outz

    def npts(self, lon1, lat1, lon2, lat2, npts, radians=False):
        """
        Given a single initial point and terminus point (specified by
        python floats lon1,lat1 and lon2,lat2), returns a list of
        longitude/latitude pairs describing npts equally spaced
        intermediate points along the geodesic between the initial and
        terminus points.

        if radians=True, lons/lats are radians instead of degrees.

        Example usage:

        >>> from pyproj import Geod
        >>> g = Geod(ellps='clrk66') # Use Clarke 1966 ellipsoid.
        >>> # specify the lat/lons of Boston and Portland.
        >>> g = Geod(ellps='clrk66') # Use Clarke 1966 ellipsoid.
        >>> # specify the lat/lons of Boston and Portland.
        >>> boston_lat = 42.+(15./60.); boston_lon = -71.-(7./60.)
        >>> portland_lat = 45.+(31./60.); portland_lon = -123.-(41./60.)
        >>> # find ten equally spaced points between Boston and Portland.
        >>> lonlats = g.npts(boston_lon,boston_lat,portland_lon,portland_lat,10)
        >>> for lon,lat in lonlats: '%6.3f  %7.3f' % (lat, lon)
        '43.528  -75.414'
        '44.637  -79.883'
        '45.565  -84.512'
        '46.299  -89.279'
        '46.830  -94.156'
        '47.149  -99.112'
        '47.251  -104.106'
        '47.136  -109.100'
        '46.805  -114.051'
        '46.262  -118.924'
        >>> # test with radians=True (inputs/outputs in radians, not degrees)
        >>> import math
        >>> dg2rad = math.radians(1.)
        >>> rad2dg = math.degrees(1.)
        >>> lonlats = g.npts(dg2rad*boston_lon,dg2rad*boston_lat,dg2rad*portland_lon,dg2rad*portland_lat,10,radians=True)
        >>> for lon,lat in lonlats: '%6.3f  %7.3f' % (rad2dg*lat, rad2dg*lon)
        '43.528  -75.414'
        '44.637  -79.883'
        '45.565  -84.512'
        '46.299  -89.279'
        '46.830  -94.156'
        '47.149  -99.112'
        '47.251  -104.106'
        '47.136  -109.100'
        '46.805  -114.051'
        '46.262  -118.924'
        """
        lons, lats = _proj.Geod._npts(self, lon1, lat1, lon2, lat2, npts, radians=radians)
        return list(zip(lons, lats))

def test():
    """run the examples in the docstrings using the doctest module"""
    import doctest, pyproj
    doctest.testmod(pyproj,verbose=True)

if __name__ == "__main__": test()
