import warnings
warnings.warn('pymorph.compat should be replaced with pymorph', DeprecationWarning)

from notimplemented import *
from mmorph import *
from text import *

# old abbreviations:

clohole=close_holes
ero=erode
cero=cerode
dil=dilate
cdil=cdilate
sedil=sedilate
add4dil=add4dilate
uint8=to_uint8
uint16=to_uint16
int32=to_int32
glblshow=randomcolor
randomcolour=randomcolor

# mmnames:

def _not_implemented(msg):
    def f(*args, **kwargs):
        raise NotImplementedError, msg
    f.__doc__ = '''\
This function is not implemented anymore.

%s''' % msg
    return f

mmadd4dil=add4dil
mmaddm=addm
mmareaclose=areaclose
mmareaopen=areaopen
mmasf=asf
mmasfrec=asfrec
mmbinary=binary
mmblob=blob
mmbshow=bshow
mmcbisector=cbisector
mmcdil=cdil
mmcenter=center
mmcero=cero
mmclohole=clohole
mmclose=close
mmcloserec=closerec
mmcloserecth=closerecth
mmcloseth=closeth
mmconcat=concat
mmcthick=cthick
mmcthin=cthin
mmcwatershed=cwatershed
mmdatatype=datatype
mmdil=dil
mmdist=dist
mmdrawv=drawv
mmdtshow=_not_implemented('dtshow: use matplotlib')
mmedgeoff=edgeoff
mmero=ero
mmflood=flood
mmframe=frame
mmgdist=gdist
mmgdtshow=_not_implemented('gdtshow: use matplotlib')
mmgradm=gradm
mmgrain=grain
mmgray=gray
mmhistogram=histogram
mmhmax=hmax
mmhmin=hmin
mmhomothick=homothick
mmhomothin=homothin
mmimg2se=img2se
mminfcanon=infcanon
mminfgen=infgen
mminfrec=infrec
mminpos=inpos
mminterot=interot
mmintersec=intersec
mmintershow=intershow
mmisbinary=isbinary
mmisequal=isequal
mmlabel=label
mmlabelflat=labelflat
mmlastero=lastero
mmlblshow=_not_implemented('lblshow: use matplotlib')
mmlimits=limits
mmmat2set=mat2set
mmmaxleveltype=maxleveltype
mmneg=neg
mmopen=open
mmopenrec=openrec
mmopenrecth=openrecth
mmopenth=openth
mmopentransf=opentransf
mmpad4n=pad4n
mmpatspec=patspec
mmreadgray=_not_implemented('readgray: use PIL or readmagick')
mmregmax=regmax
mmregmin=regmin
mmse2hmt=se2hmt
mmse2interval=se2interval
mmsebox=sebox
mmsecross=secross
mmsedil=sedil
mmsedisk=sedisk
mmseline=seline
mmsereflect=sereflect
mmserot=serot
mmseshow=seshow
mmsesum=sesum
mmset2mat=set2mat
mmsetrans=setrans
mmseunion=seunion
mmskelm=skelm
mmskelmrec=skelmrec
mmskiz=skiz
mmsubm=subm
mmsupcanon=supcanon
mmsupgen=supgen
mmsuprec=suprec
mmswatershed=swatershed
mmsymdif=symdiff
mmtext=text
mmthick=thick
mmthin=thin
mmthreshad=threshad
mmtoggle=toggle
mmunion=union
mmvmax=vmax
mmwatershed=watershed

gshow=overlay
gdtshow=isolines

# Functions which were removed:

def mminstall(*args):
    pass
def mmversion(*args):
    pass
def mmregister(*args):
    pass


def mmcmp(f1, oper, f2, oper1=None, f3=None):
    """
        - Alternative:
            Consider using array operations directly, i.e., instead of 
                mmcmp(f1, '>', f2)
            simply use
                f1 > f2
        - Purpose
            Compare two images pixelwisely.
        - Synopsis
            y = mmcmp(f1, oper, f2, oper1=None, f3=None)
        - Input
            f1:    Gray-scale (uint8 or uint16) or binary image.
            oper:  String Default: "". relationship from: '==', '~=',
                   '<','<=', '>', '>='.
            f2:    Gray-scale (uint8 or uint16) or binary image.
            oper1: String Default: None. relationship from: '==', '~=',
                   '<','<=', '>', '>='.
            f3:    Gray-scale (uint8 or uint16) or binary image. Default:
                   None.
        - Output
            y: Binary image.
        - Description
            Apply the relation oper to each pixel of images f1 and f2 , the
            result is a binary image with the same size. Optionally, it is
            possible to make the comparison among three image. It is
            possible to use a constant value in place of any image, in this
            case the constant is treated as an image of the same size as the
            others with all pixels with the value of the constant.
        - Examples
            #
            #   example 1
            #
            print cmp(to_uint8([1, 2, 3]),'<', to_uint8(2))
            print cmp(to_uint8([1, 2, 3]),'<', to_uint8([0, 2, 4]))
            print cmp(to_uint8([1, 2, 3]),'==', to_uint8([1, 1, 3]))
            #
            #   example 2
            #
            f=readgray('keyb.tif')
            fbin=cmp(to_uint8(10), '<', f, '<', to_uint8(50))
            show(f)
            show(fbin)
    """

    if   oper == '==':    y = (f1==f2)
    elif oper == '~=':    y = (f1!=f2)
    elif oper == '<=':    y = (f1<=f2)
    elif oper == '>=':    y = (f1>=f2)
    elif oper == '>':     y = (f1> f2)
    elif oper == '<':     y = (f1< f2)
    else:
        assert 0, 'oper must be one of: ==, ~=, >, >=, <, <=, it was:'+oper
    if oper1 != None:
        if   oper1 == '==':     y = intersec(y, f2==f3)
        elif oper1 == '~=':     y = intersec(y, f2!=f3)
        elif oper1 == '<=':     y = intersec(y, f2<=f3)
        elif oper1 == '>=':     y = intersec(y, f2>=f3)
        elif oper1 == '>':      y = intersec(y, f2> f3)
        elif oper1 == '<':      y = intersec(y, f2< f3)
        else:
            assert 0, 'oper1 must be one of: ==, ~=, >, >=, <, <=, it was:'+oper1

    y = binary(y)
    return y


def mmvdome(f, v=1, Bc=None):
    """
        - Purpose
            Obsolete, use vmax.
        - Synopsis
            y = mmvdome(f, v=1, Bc=None)
        - Input
            f:  Gray-scale (uint8 or uint16) image.
            v:  Default: 1. Volume parameter.
            Bc: Structuring Element Default: None (3x3 elementary cross).
                Structuring element (connectivity).
        - Output
            y: Gray-scale (uint8 or uint16) or binary image.
        - Description
            The correct name for this operator mmvdome is vmax.

    """

    if Bc is None: Bc = secross()
    y = hmax(f,v,Bc);
    return y

def mmis(f1, oper, f2=None, oper1=None, f3=None):
    """
        - Alternative
            Consider using array operations or isbinary()
        - Purpose
            Verify if a relationship among images is true or false.
        - Synopsis
            y = mmis(f1, oper, f2=None, oper1=None, f3=None)
        - Input
            f1:    Gray-scale (uint8 or uint16) or binary image.
            oper:  String relationship from: '==', '~=', '<','<=', '>',
                   '>=', 'binary', 'gray'.
            f2:    Gray-scale (uint8 or uint16) or binary image. Default:
                   None.
            oper1: String Default: None. relationship from: '==', '~=',
                   '<','<=', '>', '>='.
            f3:    Gray-scale (uint8 or uint16) or binary image. Default:
                   None.
        - Output
            y: Bool value: 0 or 1
        - Description
            Verify if the property or relatioship between images is true or
            false. The result is true if the relationship is true for all
            the pixels in the image, and false otherwise. (Obs: This
            function replaces is equal, is lesseq, is binary ).
        - Examples
            #
            fbin=binary([0, 1])
            f1=to_uint8([1, 2, 3])
            f2=to_uint8([2, 2, 3])
            f3=to_uint8([2, 3, 4])
            mmis(fbin,'binary')
            mmis(f1,'gray')
            mmis(f1,'==',f2)
            mmis(f1,'<',f3)
            mmis(f1,'<=',f2)
            mmis(f1,'<=',f2,'<=',f3)
    """
    from string import upper

    if f2 == None:
        oper=upper(oper);
        if   oper == 'BINARY': return isbinary(f1)
        elif oper == 'GRAY'  : return not isbinary(f1)
        else:
            assert 0,'oper should be BINARY or GRAY, was'+oper
    elif oper == '==':    y = isequal(f1, f2)
    elif oper == '~=':    y = not isequal(f1,f2)
    elif oper == '<=':    y = mmislesseq(f1,f2)
    elif oper == '>=':    y = mmislesseq(f2,f1)
    elif oper == '>':     y = isequal(neg(threshad(f2,f1)),binary(1))
    elif oper == '<':     y = isequal(neg(threshad(f1,f2)),binary(1))
    else:
        assert 0,'oper must be one of: ==, ~=, >, >=, <, <=, it was:'+oper
    if oper1 != None:
        if   oper1 == '==': y = y and isequal(f2,f3)
        elif oper1 == '~=': y = y and (not isequal(f2,f3))
        elif oper1 == '<=': y = y and mmislesseq(f2,f3)
        elif oper1 == '>=': y = y and mmislesseq(f3,f2)
        elif oper1 == '>':  y = y and isequal(neg(threshad(f3,f2)),binary(1))
        elif oper1 == '<':  y = y and isequal(neg(threshad(f2,f3)),binary(1))
        else:
            assert 0,'oper1 must be one of: ==, ~=, >, >=, <, <=, it was:'+oper1
    return y


def mmislesseq(f1, f2, MSG=None):
    """
        - Alternative
            Consider using f1 <= f2
        - Purpose
            Verify if one image is less or equal another (is beneath)
        - Synopsis
            bool = mmislesseq(f1, f2)
        - Input
            f1:  Gray-scale (uint8 or uint16) or binary image.
            f2:  Gray-scale (uint8 or uint16) or binary image.
        - Output
            bool: Boolean
        - Description
            mmislesseq compares the images f1 and f2 and returns true (1),
            if f1(x) <= f2(x) , for every pixel x, and false (0), otherwise.
        - Examples
            #
            f1 = to_uint8([0, 1, 2, 3])
            f2 = to_uint8([9, 5, 3, 3])
            print mmislesseq(f1,f2)
            print mmislesseq(f2,f1)
            print mmislesseq(f1,f1)
    """
    from numpy import ravel

    bool = min(ravel(f1<=f2))
    return bool

def mmstats(f, measurement):
    """
        - Purpose
            Find global image statistics.
        - Synopsis
            y = mmstats(f, measurement)
        - Input
            f:           
            measurement: String Default: "". Choose the measure to compute:
                         'max', 'min', 'median', 'mean', 'sum', 'std',
                         'std1'.
        - Output
            y:
        - Description
            Compute global image statistics: 'max' - maximum gray-scale
            value in image; 'min' - minimum gray-scale value in image; 'sum'
            - sum of all pixel values; 'median' - median value of all pixels
            in image; 'mean' - mean value of all pixels in image; 'std' -
            standard deviation of all pixels (normalized by N-1); 'std1' -
            idem, normalized by N.

    """
    from string import upper
    from numpy import ravel
    from numpy.oldnumeric.mlab import mean, median, std

    measurement = upper(measurement)
    if measurement == 'MAX': return f.max()
    elif measurement == 'MIN': return f.min()
    elif measurement == 'MEAN': return f.mean()
    elif measurement == 'MEDIAN': return f.median()
    elif measurement == 'STD': return f.std()
    else:
        assert 0,'pymorph.compat.mmstats: Not a valid measurement'

def mmsurf(f,options = None):
    return f

_figs = [None]

def plot(plotitems=[], options=[], outfig=-1, filename=None):
    """
        - Purpose
            Plot a function.
        - Synopsis
            fig = plot(plotitems=[], options=[], outfig=-1, filename=None)
        - Input
            plotitems: Default: []. List of plotitems.
            options:   Default: []. List of options.
            outfig:    Default: -1. Integer. Figure number. 0 creates a new
                       figure.
            filename:  Default: None. String. Name of the PNG output file.
        - Output
            fig: Figure number.

        - Examples
            #
            import numpy
            #
            x = numpy.arange(0, 2*numpy.pi, 0.1)
            plot([[x]])
            y1 = numpy.sin(x)
            y2 = numpy.cos(x)
            opts = [['title', 'Example Plot'],\
                    ['grid'],\
                    ['style', 'linespoints'],\
                    ['xlabel', '"X values"'],\
                    ['ylabel', '"Y Values"']]
            y1_plt = [x, y1, None,    'sin(X)']
            y2_plt = [x, y2, 'lines', 'cos(X)']
            #
            # plotting two graphs using one step
            fig1 = plot([y1_plt, y2_plt], opts, 0)
            #
            # plotting the same graphs using two steps
            fig2 = plot([y1_plt], opts, 0)
            fig2 = plot([y2_plt], opts, fig2)
            #
            # first function has been lost, lets recover it
            opts.append(['replot'])
            fig2 = plot([y1_plt], opts, fig2)
    """
    import Gnuplot
    import numpy

    newfig = 0
    if (plotitems == 'reset'):
        _figs[0] = None
        _figs[1:] = []
        return 0
    if len(plotitems) == 0:
        # no plotitems specified: replot current figure
        if _figs[0]:
            outfig = _figs[0]
            g = _figs[outfig]
            g.replot()
            return outfig
        else:
            #assert 0, "plot error: There is no current figure\n"
            print "plot error: There is no current figure\n"
            return 0
    # figure to be plotted
    if ((outfig < 0) and _figs[0]):
        # current figure
        outfig = _figs[0]
    elif ( (outfig == 0) or ( (outfig == -1) and not _figs[0] )  ):
        # new figure
        newfig = 1
        outfig = len(_figs)
    elif outfig >= len(_figs):
        #assert 0, 'plot error: Figure ' + str(outfig) + 'does not exist\n'
        print 'plot error: Figure ' + str(outfig) + 'does not exist\n'
        return 0
    #current figure
    _figs[0] = outfig
    # Gnuplot pointer
    if newfig:
        if len(_figs) > 20:
            print '''plot error: could not create figure. Too many PlotItems in memory (20). Use
                     plot('reset') to clear table'''
            return 0

        g = Gnuplot.Gnuplot()
        _figs.append(g)
    else:
        g = _figs[outfig]

    # options
    try:
        options.remove(['replot'])
    except:
        g.reset()
    try:
        #default style
        g('set data style lines')
        for option in options:
            if option[0] == 'grid':
                g('set grid')
            elif option[0] == 'title':
                g('set title "' + option[1] + '"')
            elif option[0] == 'xlabel':
                g('set xlabel ' + option[1])
            elif option[0] == 'ylabel':
                g('set ylabel ' + option[1])
            elif option[0] == 'style':
                g('set data style ' + option[1])
            else:
                print "plot warning: Unknown option: " + option[0]
    except:
        print "plot warning: Bad usage in options! Using default values. Please, use help.\n"
    # Plot items: item[0]=x, item[1]=y, item[2]=style
    for item in plotitems:
        try:
            title = None
            style = None
            x = numpy.ravel(item[0])
            if len(item) > 1:
                # y axis specified
                y = numpy.ravel(item[1])
                if len(item) > 2:
                    # style specified
                    style = item[2]
                    if len(item) > 3:
                        title = item[3]
            else:
                # no y axis specified
                y = x
                x = numpy.arange(len(y))
            g.replot(Gnuplot.Data(x, y, title=title, with_=style))
        except:
            g.reset()
            if newfig:
                _figs.pop()
            #assert 0, "plot error: Bad usage in plotitems! Impossible to plot graph. Please, use help.\n"
            print "plot error: Bad usage in plotitems! Impossible to plot graph. Please, use help.\n"
            return 0
    # PNG file
    if filename:
        g.hardcopy(filename, terminal='png', color=1)
    fig = outfig
    return fig
mmplot=plot


def mmwatershed(f,Bc=None,linereg='LINES'):
    return watershed(f,Bc,(linereg == 'LINES'))

def mmcwatershed(f,Bc=None,linereg='LINES'):
    return cwatershed(f,Bc,(linereg == 'LINES'))

def mmskiz(f,Bc=None,LINEREG='LINES',METRIC=None):
    return skiz(f,Bc,(LINEREG=='LINES'),METRIC)

def mmdist(f,Bc=None,METRIC=None):
    return dist(f,Bc,metric=METRIC)

def mmendpoints(OPTION='LOOP'):
    return endpoints(option=OPTION)



def mmgshow(X, X1=None, X2=None, X3=None, X4=None, X5=None, X6=None):
    """
        - Purpose
            Apply binary overlays as color layers on a binary or gray-scale
            image
        - Synopsis
            Y = gshow(X, X1=None, X2=None, X3=None, X4=None, X5=None,
            X6=None)
        - Input
            X:  Gray-scale (uint8 or uint16) or binary image.
            X1: Binary image. Default: None. Red overlay.
            X2: Binary image. Default: None. Green overlay.
            X3: Binary image. Default: None. Blue overlay.
            X4: Binary image. Default: None. Magenta overlay.
            X5: Binary image. Default: None. Yellow overlay.
            X6: Binary image. Default: None. Cyan overlay.
        - Output
            Y: Gray-scale (uint8 or uint16) or binary image.

    """

    if isbinary(X): X = gray(X,'uint8')
    r = X
    g = X
    b = X
    if X1 is not None: # red 1 0 0
      assert isbinary(X1),'X1 must be binary overlay'
      x1 = gray(X1,'uint8')
      r = union(r,x1)
      g = intersec(g,neg(x1))
      b = intersec(b,neg(x1))
    if X2 is not None: # green 0 1 0
      assert isbinary(X2),'X2 must be binary overlay'
      x2 = gray(X2,'uint8')
      r = intersec(r,neg(x2))
      g = union(g,x2)
      b = intersec(b,neg(x2))
    if X3 is not None: # blue 0 0 1
      assert isbinary(X3),'X3 must be binary overlay'
      x3 = gray(X3,'uint8')
      r = intersec(r,neg(x3))
      g = intersec(g,neg(x3))
      b = union(b,x3)
    if X4 is not None: # magenta 1 0 1
      assert isbinary(X4),'X4 must be binary overlay'
      x4 = gray(X4,'uint8')
      r = union(r,x4)
      g = intersec(g,neg(x4))
      b = union(b,x4)
    if X5 is not None: # yellow 1 1 0
      assert isbinary(X5),'X5 must be binary overlay'
      x5 = gray(X5,'uint8')
      r = union(r,x5)
      g = union(g,x5)
      b = intersec(b,neg(x5))
    if X6 is not None: # cyan 0 1 1
      assert isbinary(X6),'X6 must be binary overlay'
      x6 = gray(X6,'uint8')
      r = intersec(r,neg(x6))
      g = union(g,x6)
      b = union(b,x6)
    return concat('d',r,g,b)

def mmglblshow(X, border=0.0):
    """
        - Purpose
            Apply a random color table to a gray-scale image.
        - Synopsis
            Y = glblshow(X, border=0.0)
        - Input
            X:      Gray-scale (uint8 or uint16) image. Labeled image.
            border: Boolean Default: 0.0. Labeled image.
        - Output
            Y: Gray-scale (uint8 or uint16) or binary image.

    """
    from numpy import take, resize, shape
    from numpy.random import rand

    mmin = X.min()
    mmax = X.max()
    ncolors = mmax - mmin + 1
    R = to_int32(rand(ncolors)*255)
    G = to_int32(rand(ncolors)*255)
    B = to_int32(rand(ncolors)*255)
    if mmin == 0:
       R[0],G[0],B[0] = 0,0,0
    r=resize(take(R, X.ravel() - mmin),X.shape)
    g=resize(take(G, X.ravel() - mmin),X.shape)
    b=resize(take(B, X.ravel() - mmin),X.shape)
    Y=concat('d',r,g,b)
    return Y



def readgray(filename):
    """
        - Purpose
            Read an image from a coercial file format and stores it as a
            gray-scale image.
        - Synopsis
            y = readgray(filename)
        - Input
            filename: String Name of file to read.
        - Output
            y: Gray-scale (uint8 or uint16) or binary image.
        - Description
            readgray reads the image in filename and stores it in y , an
            uint8 gray-scale image (without colormap). If the input file is
            a color RGB image, it is converted to gray-scale using the
            equation: y = 0.2989 R + 0.587 G + 0.114 B. This functions uses
            de PIL module.
        - Examples
            #
            a=readgray('cookies.tif')
            show(a)
    """
    import pylab
    import numpy

    y = pylab.imread(filename)
    if (len(y.shape) == 3) and (y.shape[0] == 3):
       if numpy.alltrue(numpy.alltrue(y[0,:,:] == y[1,:,:] and
                                          y[0,:,:] == y[2,:,:])):
          y = y[0,:,:]
       else:
          print 'Warning: converting true-color RGB image to gray'
          y = ubyte(0.2989 * y[0,:,:] + 
                      0.5870 * y[1,:,:] + 
                      0.1140 * y[2,:,:])
    elif (len(y.shape) == 2):
       pass
    else:
       raise ValueError, 'Error, it is not 2D image'
    return y



def freedom(L=5):
    """
    DOES NOT DO ANYTHING
    """
    return -1

mmfreedom=freedom

