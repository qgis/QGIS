Configuring external applications
=================================

Introduction
------------

SEXTANTE can be extended using additional applications, calling them
from within SEXTANTE. Currently, SAGA, GRASS, OTB(Orfeo Toolbox) and R are supported, along 
with some other command-line applications that provide spatial data 
analysis functionalities. 
This chapter will show you how to configure SEXTANTE to include these
additional applications. Once you have correctly configured the system,
you will be able to execute external algorithms from any SEXTANTE
component like the toolbox or the graphical modeler, just like you do
with any other SEXTANTE geoalgorithm.

A note on file formats
~~~~~~~~~~~~~~~~~~~~~~

When using an external software, opening a file in QGIS does not mean
that it can be opened and processed as well on that other software. In
most cases, it can read what you have opened in QGIS, but in some cases,
that might not be the case. When using databases or uncommon file
formats, whether for raster of vector layers, problems might arise. If
that happens, try to use well known file formats that you are sure that
are understood by both programs, and check to console output (in the
history and log dialog) for knowing more about what is going wrong.

Using GRASS raster layers is, for instance, one case in which you might
have trouble and not be able to complete your work if you call an
external algorithm using such a layer as input. For this reason, these
layers will not appear as available to SEXTANTE algorithms (We are
currently working on solving this, and expect to have it ready soon).

You should, however, find no problems at all with vector layers, since
SEXTANTE automatically converts from the original file format to one
accepted by the external application before passing the layer to it.
This adds an extra processing time, which might be significant if the
layer has a large size, so do not be surprised if it takes more to
process a layer from a DB connection that one of a similar size stored
in a shapefile.

Providers not using external applications can process any layer that you
can open in QGIS, since they open it for analysis trough QGIS.

Regarding output formats, raster layers can be saved as TIFF (.tif)
files, while vector layers are saved as shapefiles (.shp). These have
been chosen as the *lingua franca* between supported third party
applications and QGIS. If the output filename that you select is not one
of the above, it will be modified, adding the corresponding suffix, and
the default file format will be used.

In the case of GDAL, the number of supported output formats is larger.
When you open the file selection dialog, you will see that you have more
formats (and their corresponding extensions available). For more
information about which formats are supported, check the GDAL
documentation.

A note on vector layer selections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, when an external algorithm takes a vector layer, it will use
all its features, even if a selection exist in QGIS. You can make an
external algorithm aware of that selection by checking the *Use selected
features in external applications* item in the *General* settings group.
When you do so, each time you execute an external algorithm that uses a
vector layer, the selected features of that layer will be exported to a
new layer, and the algorithm will work with that new layer instead.

Notice that if you select this option, a layer with no selection will
behave like a layer with all its features selected, not like an empty
layer.

SAGA
----

SAGA algorithms can be run from SEXTANTE if you have SAGA installed in
your system and you configure SEXTANTE properly so it can find SAGA
executables. In particular, the SAGA command–line executable is needed
to run SAGA algorithms. SAGA binaries are not included with SEXTANTE, so
you have to download and install the software yourself. Please check the
SAGA website at for more information. SAGA 2.0.8 is needed.

Once SAGA is installed, and if you are running Windows, open the
SEXTANTE configuration dialog. In the *SAGA* block you will find a
setting named *SAGA Folder*. Enter the path to the folder where SAGA is
installed. Close the configuration dialog and now you are ready to run
SAGA algorithms from SEXTANTE.

In case you are using Linux, there is no need to configure that, and you
will not see those folders. Instead, you must make sure that SAGA is
properly installed and its folder is added to the PATH environment
variable. Just open a console and type ``saga_cmd`` to check that the
system can found where SAGA binaries are located.

Notice that, ever before doing that, SAGA algorithms are shown in the
toolbox and you can open them to fill the corresponding parameters
dialog. However, if you try to run the algorithm after entering the
parameter values, SEXTANTE will show an error message. This is because
the algorithm descriptions (needed to create the parameters dialog and
give SEXTANTE the information it needs about the algorithm) are not
included with SAGA, but with SEXTANTE instead. That is, they are part of
SEXTANTE, so you have them in your installation even if you have not
installed SEXTANTE. Running the algorithm, however, needs SAGA binaries
installed in your system.

About SAGA grid system limitations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Most of SAGA algorithms that require several input raster layers,
require them to have the same grid system. That is, to cover the same
geographic area and have the same cellsize, so their corresponding grids
match. When calling SAGA algorithms from SEXTANTE, you can use any
layer, regardless of its cellsize and extent. When multiple raster layers
are used as input for a SAGA algorithm, SEXTANTE resamples them to a
common grid system and then passes them to SAGA (unless the SAGA
algorithm can operate with layers from different grid systems).

The definition of that common grid system is controlled by the user, and
you will find several parameters in the SAGA group of the setting window
to do so. There are two ways of setting the target grid system:

-  Setting it manually

   . You define the extent setting the values of the following
   parameters:

   -  Resampling min X

   -  Resampling max X

   -  Resampling min Y

   -  Resampling max Y

   -  Resampling cellsize

   Notice that SEXTANTE will resample input layers to that extent, even
   if they do not overlap with it.

-  Setting it automatically from input layers. To select this option,
   just check the ’’Use min covering grid system for resampling’’
   option. All the other settings will be ignored and the minimum extent
   that covers all the input layers will be used. The cellsize of the
   target layer is the maximum of all cellsizes of the input layers.

For algorithms that do not use multiple raster layers, or for those that
do not need a unique input grid system, no resampling is performed
before calling SAGA, and those parameters are not used.

R. Creating R scripts
---------------------

R integration in SEXTANTE is different from that of SAGA in that there
is not a predefined set of algorithms you can run (except for a few
examples). Instead, you should write your scripts and call R commands,
much like you would do from R, and in a very similar manner to what we
saw in the chapter dedicated to SEXTANTE scripts. This chapter shows you
the syntax to use to call those R commands from SEXTANTE and how to use
SEXTANTE objects (layers, tables) in them.

The first thing you have to do, as we saw in the case of SAGA, is to
tell SEXTANTE where you R binaries are located. You can do so using the
*R folder* entry in the SEXTANTE configuration dialog. Once you have set
that parameter, you can start creating your own R scripts and executing
them.

Once again, this is different in Linux, and you just have to make sure
that the R folder is included in the PATH environment variable. If you
can start R just typing ``R`` in a console, then you are ready to go.

To add a new algorithm that calls an R function (or a more complex R
script that you have developed and you would like to have available from
SEXTANTE), you have to create a script file that tells SEXTANTE how to
perform that operation and the corresponding R commands to do so.

Script files have the extension ``rsx`` and creating them is pretty easy
if you just have a basic knowledge of R syntax and R scripting. They
should be stored in the R scripts folder. You can set this folder in the
R settings group (available from the SEXTANTE settings dialog), just
like you do with the folder for regular SEXTANTE scripts.

Let’s have a look at a very simple file script file, which calls the R
method ``spsample`` to create a random grid within the boundary of the
polygons in a given polygon layer. This method belong to the
``maptools`` package. Since almost all the algorithms that you might
like to incorporate into SEXTANTE will use or generate spatial data,
knowledge of spatial packages like ``maptools`` and, specially, ``sp``,
is mandatory.

::

    ##polyg=vector
    ##numpoints=number 10
    ##output=output vector
    ##sp=group
    pts=spsample(polyg,numpoints,type="random")
    output=SpatialPointsDataFrame(pts, as.data.frame(pts))

The first lines, which start with a double Python comment sign (##),
tell SEXTANTE the inputs of the algorithm described in the file and the
outputs that it will generate. They work exactly with the same syntax as
the SEXTANTE scripts that we have already seen, so they will not be
described here again. Check the corresponding section for more
information.

When you declare an input parameter, SEXTANTE uses that information for
two things: creating the user interface to ask the user for the value of
that parameter and creating a corresponding R variable that can be later
used as input for R commands

In the above example, we are declaring an input of type
``vector`` named ``polyg``. When executing the algorithm,
SEXTANTE will open in R the layer selected by the user and store it in a
variable also named ``polyg``. So the name of a parameter is also the
name of the variable that we can use in R for accesing the value of that
parameter (thus, you should avoid using reserved R words as parameter
names).

Spatial elements such as vector and raster layers are read using the
``readOGR()`` and ``readGDAL()`` commands (you do not have to worry
about adding those commands to your description file, SEXTANTE will do
it) and stored as ``Spatial*DataFrame`` objects. Table fields are stored
as strings containing the name of the selected field.

Tables are opened using the ``read.csv()`` command. If a table entered
by the user is not in CSV format, it will be converted prior to
importing it in R.

Knowing that, we can now understand the first line of our example script
(the first line not starting with a Python comment).

::

    pts=spsample(polyg,numpoints,type="random")

The variable ``polygon`` already contains a ``SpatialPolygonsDataFrame``
object, so it can be used to call the ``spsample`` method, just like the
``numpoints`` one, which indicates the number of points to add to the
created sample grid.

Since we have declared an output of type vector named ``out``, we have
to create a variable named ``out`` and store a ``Spatial*DataFrame``
object in it (in this case, a ``SpatialPointsDataFrame``). You can use
any name for your intermediate variables. Just make sure that the
variable storing your final result has the same name that you used to
declare it, and contains a suitable value.

In this case, the result obtained from the ``spsample`` method has to be
converted explicitly into a ``SpatialPointsDataFrame`` object, since it
is itself an object of class ``ppp``, which is not a suitable class to
be retuned to SEXTANTE.

If you algorithm does not generate any layer, but a text result in the
console instead, you have to tell SEXTANTE that you want the console to
be shown once the execution is finished. To do so, just start the
command lines that produce the results you want to print with the
“:math:`>`” sign. The output of all other lines will not be shown. For
instance, here is the description file of an algorithms that performs a
normality test on a given field (column) of the attributes of a vector
layer:

::

    ##layer=vector
    ##field=field layer
    ##nortest=group
    library(nortest)
    >lillie.test(layer[[field]])    

The output ot the last line is printed, but the output of the first is
not (and neither are the outputs from other command lines added
automatically by SEXTANTE).

If your algorithm creates any kind of graphics (using the ``plot()``
method), add the following line:

::

    ##showplots

This will cause SEXTANTE to redirect all R graphical outputs to a
temporary file, which will be later opened once R execution has finished

Both graphics and console results will be shown in the SEXTANTE results
manager.

For more information, please check the script files provided with
SEXTANTE. Most of them are rather simple and will greatly help you
understand how to create your own ones.

A note about libraries: ``rgdal`` and ``maptools`` libraries are loaded
by default so you do not have to add the corresponding *library()*
commands (you have to make sure, however, that those two packages are
installed in your R distribution). However, other additional libraries
that you might need have to be explicitly loaded. Just add the necessary
commands at the beginning of your script. You also have to make sure
that the corresponding packages are installed in the R distribution used
by SEXTANTE.

GRASS
-----

Configuring GRASS is not much different from configuring SAGA. First,
the path to the GRASS folder has to be defined, but only if you are
running Windows. Additionaly, a shell interpreter (usually msys.exe,
which can be found in most GRASS for Windows distributions) has to be
defined and its path set up as well. 

By default, SEXTANTE tries to configure its GRASS connector to use the GRASS distribution that ships along with QGIS. This should work without problems in most systems, but if you experience problems, you might have to do it manually. Also, if you want to use a different GRASS version, you can change that setting and point to the folder where that other version is kept. GRASS 6.4 is needed for algorithms to work correctly.

If you are running Linux, you just
have to make sure that GRASS is correctly installed, and that it can be
run without problem from a console.

GRASS algorithms use a region for calculations. This region can be
defined manually using values similar to the ones found in the SAGA
configuration, or automatically, taking the minimum extent that covers
all the input layers used to execute the algorithm each time. If this is
the behaviour you prefer, just check the *Use min covering region*
option in the GRASS configuration parameters.

GRASS includes help files describing each algorithm. If you set the
*GRASS help folder* parameter, SEXTANTE will open them when you use the
*Show help* button from the parameters window of the algorithm.

The last parameter that has to be configured is related to the mapset. A
mapset is needed to run GRASS, and SEXTANTE creates a temporary one for
each execution. You have to tell SEXTANTE if the data you are working
with uses geographical (lat/lon) coordinates or projected ones.


GDAL
-----

No additional configuration is needed to run GDAL algorithms, since it is already incorporated to QGIS and SEXTANTE can infere its configuration from it.


OTB
---

[to be written]
