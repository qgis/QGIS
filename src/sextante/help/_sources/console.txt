Using SEXTANTE from the console. Creating scripts.
==================================================

Introduction
------------

The console allows advanced users to increase their productivity and
perform complex operations that cannot be performed using any of the
other elements of the SEXTANTE GUI. Models involving several algorithms
can be defined using the command-line interface, and additional
operations such as loops and conditional sentences can be added to
create more flexible and powerful workflows.

There is not a SEXTANTE console in QGIS, but all SEXTANTE commands are
available instead from QGIS built-in Python console. That means that you
can incorporate those command to your console work and connect SEXTANTE
algorithms to all the other features (including methods from the QGIS
API) available from there.

The code that you can execute from the Python console, even if it does
call any SEXTANTE method, can be converted into a new SEXTANTE algorithm
that you can later call from the toolbox, the graphical modeler or any
other SEXTANTE component, just like you do with any other SEXTANTE
algorithm. In fact, some algorithms that you can find in the toolbox,
like all the ones in the *mmqgis* group, are simple scripts.

In this chapter we will see how to use SEXTANTE from the QGIS Python
console, and also how to write your own algorithms using Python.

Calling SEXTANTE from the Python console
----------------------------------------

The first thing you have to do is to import the ``Sextante`` class with
the following line:

::

    >>from sextante.core.Sextante import Sextante

Now, there is basically just one (interesting) thing you can do with
SEXTANTE from the console: to execute an algorithm. That is done using
the ``runalg()`` method, which takes the name of the algorithm to
execute as its first parameter, and then a variable number of additional
parameter depending on the requirements of the algorithm. So the first
thing you need to know is the name of the algorithm to execute. That is
not the name you see in the toolbox, but rather a unique command–line
name. To find the right name for your algorithm, you can use the
``algslist()`` method. Type the following line in you console:

You will see something like this.

::

    >>> Sextante.alglist()
    Accumulated Cost (Anisotropic)---------------->saga:accumulatedcost(anisotropic)
    Accumulated Cost (Isotropic)------------------>saga:accumulatedcost(isotropic)
    Add Coordinates to points--------------------->saga:addcoordinatestopoints
    Add Grid Values to Points--------------------->saga:addgridvaluestopoints
    Add Grid Values to Shapes--------------------->saga:addgridvaluestoshapes
    Add Polygon Attributes to Points-------------->saga:addpolygonattributestopoints
    Aggregate------------------------------------->saga:aggregate
    Aggregate Point Observations------------------>saga:aggregatepointobservations
    Aggregation Index----------------------------->saga:aggregationindex
    Analytical Hierarchy Process------------------>saga:analyticalhierarchyprocess
    Analytical Hillshading------------------------>saga:analyticalhillshading
    Average With Mask 1--------------------------->saga:averagewithmask1
    Average With Mask 2--------------------------->saga:averagewithmask2
    Average With Thereshold 1--------------------->saga:averagewiththereshold1
    Average With Thereshold 2--------------------->saga:averagewiththereshold2
    Average With Thereshold 3--------------------->saga:averagewiththereshold3
    B-Spline Approximation------------------------>saga:b-splineapproximation
    .
    .
    .

That's a list of all the available algorithms, alphabetically ordered,
along with their corresponding command-line names.

You can use a string as a parameter for this method. Instead of
returning the full list of algorithm, it will only display those that
include that string. If, for instance, you are looking for an algorithm
to calculate slope from a DEM, type ``alglist("slope")`` to get the
following result:

::

    DTM Filter (slope-based)---------------------->saga:dtmfilter(slope-based)
    Downslope Distance Gradient------------------->saga:downslopedistancegradient
    Relative Heights and Slope Positions---------->saga:relativeheightsandslopepositions
    Slope Length---------------------------------->saga:slopelength
    Slope, Aspect, Curvature---------------------->saga:slopeaspectcurvature
    Upslope Area---------------------------------->saga:upslopearea
    Vegetation Index[slope based]----------------->saga:vegetationindex[slopebased]

It is easier now to find the algorithm you are looking for and its
command-line name, in this case *saga:slopeaspectcurvature*

Once you know the command-line name of the algorithm, the next thing to
do is to know the right syntax to execute it. That means knowing which
parameters are needed and the order in which they have to be passed when
calling the ``runalg()`` method. SEXTANTE has a method to describe an
algorithm in detail, which can be used to get a list of the parameters
that an algorithms require and the outputs that it will generate. To do
it, you can use the ``alghelp(name_of_the_algorithm)`` method. Use the
command-line name of the algorithm, not the full descriptive name.

Calling the method with ``saga:slopeaspectcurvature`` as parameter, you
get the following description.

::

    >Sextante.alghelp("saga:slopeaspectcurvature")
    ALGORITHM: Slope, Aspect, Curvature
       ELEVATION <ParameterRaster>
       METHOD <ParameterSelection>
       SLOPE <OutputRaster>
       ASPECT <OutputRaster>
       CURV <OutputRaster>
       HCURV <OutputRaster>
       VCURV <OutputRaster>

Now you have everything you need to run any algorithm. As we have
already mentioned, there is only one single command to execute
algorithms: ``runalg``. Its syntax is as follows:

::

    > runalg{name_of_the_algorithm, param1, param2, ..., paramN, 
             Output1, Output2, ..., OutputN)

The list of parameters and outputs to add depends on the algorithm you
want to run, and is exactly the list that the ``describealg`` method
gives you, in the same order as shown.

Depending on the type of parameter, values are introduced differently.
The next one is a quick review of how to introduce values for each type
of input parameter

-  Raster Layer, Vector Layer or Table. Simply use a string with the
   name that identifies the data object to use (the name it has in the
   QGIS Table of Contents) or a filename (if the corresponding layer is
   not opened, it will be opened, but not added to the map canvas). If
   you have an instance of a QGIS object representing the layer, you can
   also pass it as parameter. If the input is optional and you do not
   want to use any data object, use ``None``.

-  Selection. If an algorithm has a selection parameter, the value of
   that parameter should be entered using an integer value. To know the
   available options, you can use the ``algoptions`` command, as shown
   in the following example:

   ::

       >>Sextante.algoptions("saga:slopeaspectcurvature")
       METHOD(Method)
           0 - [0] Maximum Slope (Travis et al. 1975)
           1 - [1] Maximum Triangle Slope (Tarboton 1997)
           2 - [2] Least Squares Fitted Plane (Horn 1981, Costa-Cabral & Burgess 1996)
           3 - [3] Fit 2.Degree Polynom (Bauer, Rohdenburg, Bork 1985)
           4 - [4] Fit 2.Degree Polynom (Heerdegen & Beran 1982)
           5 - [5] Fit 2.Degree Polynom (Zevenbergen & Thorne 1987)
           6 - [6] Fit 3.Degree Polynom (Haralick 1983)

   In this case, the algorithm has one of such such parameters, with 7
   options. Notice that ordering is zero-based.

-  Multiple input. The value is a string with input descriptors
   separated by semicolons. As in the case of single layers or tables,
   each input descriptor can be the data object name, or its filepath.

-  Table Field from XXX. Use a string with the name of the field to use.
   This parameter is case-sensitive.

-  Fixed Table. Type the list of all table values separated by commas
   and enclosed between quotes. Values start on the upper row and go
   from left to right. You can also use a 2D array of values
   representing the table.
   
-  CRS: Enter the EPSG code number of the desired CRS

-  Extent: You must use a string with xmin,xmax,ymin and ymax values separated by commas

Boolean, file, string and numerical parameters do not need any additional
explanations.

Input parameters such as strings booleans or numerical values have default
values. To use them, use ``None`` in the corresponding parameter entry.

For output data objects, type the filepath to be used to save it, just
as it is done from the toolbox. If you want to save the result to a
temporary file, use ``None``. The extension of the file determines the
file format. If you enter a file extension not included in the ones
supported by the algorithm, the default file format for that output
type will be used, and its corresponding extension appended to the given
filepath.

Unlike when an algorithm is executed from the toolbox, outputs are not
added to the map canvas if you execute that same algorithm from the
Python console. If you want to add an output to it, you have to do it
yourself after running the algorithm. To do so, you can use QGIS API
commands, or, even easier, use one of the handy methods provided by
SEXTANTE for such task.

The ``runalg`` method returns a dictionary with the output names (the
ones shown in the algorithm description) as keys and the filepaths of
those outputs as values. To add all the outputs generated by an
algorithm, pass that dictionary to the ``loadFromAlg()`` method. You can
also load an individual layer passing its filepath to the ``load()``
method.

Creating scripts and running them from the toolbox
--------------------------------------------------

You can create your own algorithms by writing the corresponding Python
code and adding a few extra lines to supply additional information
needed by SEXTANTE. You can find a *Create new script* under the tools
group in the script algorithms block of the toolbox. Double click on it
to open the script edition dialog. That's where you should type your
code. Saving the script from there in the scripts folder (the default
one when you open the save file dialog), with ``.py`` extension, will
automatically create the corresponding algorithm.

The name of the algorithm (the one you will see in the toolbox) is
created from the filename, removing its extension and replacing low
hyphens with blank spaces.

Let's have the following code, which calculates the Topographic Wetness
Index(TWI) directly from a DEM

::

    ##dem=raster
    ##twi=output
    ret_slope = Sextante.runalg("saga:slopeaspectcurvature", dem, 0, None, 
                    None, None, None, None)
    ret_area = Sextante.runalg("saga:catchmentarea(mass-fluxmethod)", "dem", 
                    0, False, False, False, False, None, None, None, None, None)
    Sextante.runalg("saga:topographicwetnessindex(twi), ret_slope['SLOPE'], 
                    ret_area['AREA'], None, 1, 0, twi)

As you can see, it involves 3 algorithms, all of them coming from SAGA.
The last one of them calculates de TWI, but it needs a slope layer and a
flow accumulation layer. We do not have these ones, but since we have
the DEM, we can calculate them calling the corresponding SAGA
algorithms.

The part of the code where this processing takes place is not difficult
to understand if you have read the previous sections in this chapter.
The first lines, however, need some additional explanation. They provide
SEXTANTE the information it needs to turn your code into an algorithm
that can be run from any of its components, like the toolbox or the
graphical modeler.

These lines start with a double Python comment symbol and have the
following structure: *[parameter_name]=[parameter_type]
[optional_values]*. Here is a list of all the parameter types that
SEXTANTE supports in its scripts, their syntax and some examples.

-  ``raster``. A raster layer

-  ``vector``. A vector layer

-  ``table``. A table

-  ``number``. A numerical value. A default value must be provided. For
   instance, ``depth=number 2.4``

-  ``string``. A text string. As in the case of numerical values, a
   default value must be added. For instance, ``name=string Victor``

-  ``boolean``. A boolean value. Add ``True`` or ``False`` after it to
   set the default value. For example, ``verbose=boolean True``

-  ``multiple raster``. A set of input raster layers.

-  ``multiple vector``. A set of input vector layers.

-  ``field``. A field in the attributes table of a vector layer. The
   name of the layer has to be added after the ``field`` tag. For
   instance, if you have declared a vector input with
   ``mylayer=vector``, you could use ``myfield=field mylayer`` to add a
   field from that layer as parameter.
   
-  ``folder``. A folder

-  ``file``. A filename

The parameter name is the name that will be shown to the user when
executing the algorithm, and also the variable name to use in the script
code. The value entered by the user for that parameter will be assigned
to a variable with that name.

When showing the name of the parameter to the user, SEXTANTE will edit it to improve its appearance, replacing low hyphens with blankspaces. So, for instance, if you want the user to see a parameter named ``A numerical value``, you can use the variable name ``A_numerical_value``

Layers and tables values are strings containing the filepath of the
corresponding object. To turn them into a QGIS object, you can use the
``getObject()`` method in the ``Sextante`` class. Multiple inputs also
have a string value, which contains the filepaths to all selected
object, separated by semicolons.

Outputs are defined in a similar manner, using the following tags:

-  ``output raster``

-  ``output vector``

-  ``output table``

-  ``output html``

-  ``output file``

The value assigned to the output variables is always a string with a
filepath. It will correspond to a temporary filepath in case the user
has not entered any output filename.

When you declare an output, SEXTANTE will try to add it to QGIS once the
algorithm is finished. That is the reason why, although the ``runalg()``
method does not load the layers it produces, the final TWI layer will be
loaded, since it is saved to the file entered by the user, which is the
value of the corresponding output.

Do not use the ``load()`` method in your script algorithms, but just
when working with the console line. If a layer is created as output of
an algorithm, it should be declared as such. Otherwise, you will not be
able to properly use the algorithm in the modeler, since its syntax (as
defined by the tags explained above) will not match what the algorithm
really creates.

In addition to the tags for parameters and outputs, you can also define
the group under which the algorithm will be shown, using the ``group``
tag.

Several examples are provided with SEXTANTE. Please, check them to see
real examples of how to create algorithms using this feature of
SEXTANTE. You can right-click on any script algorithm and select *Edit
script* to edit its code or just to see it.

Documenting your scripts
--------------------------

As in the case of models, you can create additional documentation for your script, to explain what they do and how to use them. In the script editing dialog you will find a *Edit script help* button. Click on it and it will take you to the help editing dialog. Check the chapter about the graphical modeler to know more about this dialog and how to use it.

Help files are saved in the same folder as the script itself, adding the *.help* extension to the filename. Notice that you can edit your script's help before saving it for the first time. If you later close the script editing dialog without saving the script (i.e. you discard it), the help content you wrote will be lost. If your script was already saved and is associated to a filename, saving is done automatically.

Communicating with the user
----------------------------

You can send messages to the user to inform about the progress of the algorithm. To do so, just print whatever information you want to show in the textbox above the progress bar in the algorithm dialog, using the ``print`` command. For instance, just use ``print "Processing polygon layer"`` and the text will be redirected to that textbox.

If the text you print is just a number between 0 and 100, it will be understood as the percentage of the process that has been already finished, and instead of redirecting the text to the textbox, the progress bar will be update to that percentage of completion.
