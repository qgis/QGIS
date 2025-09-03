# GRASS GIS Processing provider

In order for a GRASS command to be executed within QGIS, a plain text description file is required to define the command's inputs, outputs, and parameters. Each GRASS command is described in a separate text file. However, some commands can be split into several algorithms using more than one description file, resulting in multiple entries in the algorithms list.

This splitting was originally done because the Processing provider did not support optional parameters; however, it can simplify the use of GRASS commands with many options.

## Description files

Each description file starts with three lines containing:

 1. The name of the GRASS command to call to execute the algorithm (e.g. `v.buffer`)
 2. The description of the algorithm that will be displayed in the algorithm dialog. For split commands you must include the algorithm id first, e.g.:
   ```
   r.sun.insoltime - Solar irradiance and irradiation model (daily sums)
   ```
   and
   ```
   r.sun.incidout - Solar irradiance and irradiation model (for the set local time)
   ```
 3. The name of the group where you want the command to appear

After these three lines, the algorithm's inputs and outputs are defined, one per line.

### Defining inputs and outputs

Each algorithm parameter is defined on a separate line, with elements separated by the pipe symbol `|`.

The following parameters are supported:

- A raster layer
  ```
  QgsProcessingParameterRasterLayer|[name of GRASS parameter]|[description of parameter to show]|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterRasterLayer|base|Name of input raster map|None|False`

- A vector layer
  ```
  QgsProcessingParameterFeatureSource|[name of GRASS parameter]|[description of parameter to show]|[A number indicating the type of geometry]|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterFeatureSource|input|Name of input vector map|-1|None|False`

  To indicate the type of the geometry, use the following values:

  * -1: any geometry
  * 0: points
  * 1: lines
  * 2: polygons

- Multiple layers
  ```
  QgsProcessingParameterMultipleLayers|[name of GRASS parameter]|[description of parameter to show]|[A number indicating the type of geometry]|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```

  To indicate the type of geometry, use the following values:
  * -1: any vector geometry
  * 0: points
  * 1: lines
  * 2: polygons
  * 3: raster

  Example: `QgsProcessingParameterMultipleLayers|input|Input rasters|3|None|False`

- A file
  ```
  QgsProcessingParameterFile|[name of GRASS parameter]|[description of parameter to show]|QgsProcessingParameterFile.File|[file extension|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterFile|input|Name of input E00 file|QgsProcessingParameterFile.File|e00|None|False`

- A numerical value
  ```
  QgsProcessingParameterNumber|[name of GRASS parameter]|[description of parameter to show]|QgsProcessingParameterNumber.Integer or QgsProcessingParameterNumber.Double|[default value]|[True/False, indicating if the parameter is optional or not]|[min value]|[max value]
  ```
  `None` can be used for both min and max values to indicate that there is no lower or upper limit.
  
  Example: `QgsProcessingParameterNumber|levels|levels|QgsProcessingParameterNumber.Integer|32|False|1|256`

- A numerical range
  ```
  QgsProcessingParameterRange|[name of GRASS parameter]|[description of parameter to show]|QgsProcessingParameterNumber.Integer or QgsProcessingParameterNumber.Double|[default minimum and maximum values, separated by comma]|[True/False, indicating if the parameter is optional or not]
  ```
  If minimum and maximum values are omitted the range will not have lower and upper limit.
  
  Example: `QgsProcessingParameterRange|range|Input imagery range|QgsProcessingParameterNumber.Integer|0,255|True`

- A string
  ```
  QgsProcessingParameterString|[name of GRASS parameter]|[description of parameter to show]|[default value]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterString|config_txt|Landscape structure configuration|None|True|True`

- A value to select from a list
  ```
  QgsProcessingParameterEnum|[name of GRASS parameter]|[description of parameter to show]|[list of possible values, separated by semicolons]|[True/False, indicating whether more than one value can be selected (allowMultiple)]|[zero-based index of default value]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterEnum|node|Node method|none;split|False|0|False`
  
- A boolean
  ```  
  QgsProcessingParameterBoolean|[GRASS flag]|[description of parameter to show]|[default value]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterBoolean|-p|Output values as percentages|False|True`

- A pair of coordinates
  ```
  QgsProcessingParameterPoint|[name of GRASS parameter]|[description of parameter to show]|[default value]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterPoint|coordinates|The coordinate of the center (east,north)|0,0|False`

- An extent
  ```
  QgsProcessingParameterExtent|[name of GRASS parameter]|[description of parameter to show]|[default value]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterExtent|bbox|Bounding box for selecting features|None|True`

- A crs
  ```
  QgsProcessingParameterCrs|[name of GRASS parameter]|[description of parameter to show]|[default value]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterCrs|crs|New coordinate reference system|None|False`

After defining the inputs, it is necessary to define the algorithm's outputs. The outputs are also described on a separate line and use the same formatting conventions as the inputs. The following outputs are supported.
  
- A raster layer
  ```
  QgsProcessingParameterRasterDestination|[name of GRASS parameter]|[description of output to show]|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterRasterDestination|length_slope|Slope length and steepness (LS) factor for USLE|None|True`

- A vector layer
  ```
  QgsProcessingParameterVectorDestination|[name of GRASS parameter]|[description of output to show]|vector type|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```
  The following vector types are available
  * `QgsProcessing.TypeVectorPoint`: points
  * `QgsProcessing.TypeVectorLine`: lines
  * `QgsProcessing.TypeVectorPolygon`: polygons
  * `QgsProcessing.TypeVectorAnyGeometry`: any vector geometry

  Example: `QgsProcessingParameterVectorDestination|flowline|Flow line|QgsProcessing.TypeVectorLine|None|True`

- A file (this output type is used for data formats that are not supported by QGIS, e.g. HTML or plain text)
  ```
  QgsProcessingParameterFileDestination|[name of GRASS parameter]|[description of output to show]|[file type]|[Default value, or None]|[True/False, indicating if the parameter is optional or not]
  ```
  Example: `QgsProcessingParameterFileDestination|reportfile|Final Report File|Txt files (*.txt)|None|True`

- An output folder
  ```
  QgsProcessingParameterFolderDestination|[name of GRASS parameter]|[description of parameter to show]|[default value]|[True/False, indicating if the parameter is optional or not]
  ```

  Example: `QgsProcessingParameterFolderDestination|output_dir|Output Directory|None|False`

### Hardcoded parameters

Sometimes it is necessary to always add specific parameter to the GRASS command. In that case there is no need to expose it in the algorithm dialog. Instead these parameters
should be defined as "hardcoded" in the description file using the following format

```
Hardcoded|[parameter or argument to be passed to GRASS]
```

Here are examples
```
Hardcoded|operation=report
Hardcoded|-o
```

These two lines mean that the parameters `operation=report` and `-o` will be added to the GRASS command generated by the algorithm. 

### Advanced parameters

Some algorithm parameters may not be widely used by most users. In that case, you can mark them as "advanced" so they are added to the "Advanced parameters" group of the algorithm dialog, which is collapsed by default. To mark an input parameter as "Advanced," add an asterisk (*) before its declaration.

```*QgsProcessingParameterBoolean|-i|Output raster map as integer|False```

## Reloading algorithm descriptions

There is no need to restart QGIS after editing existing or creating a new algorithm description — simply click the wrench icon in the Processing toolbox
or open Settings → Options → Processing, then click OK, and QGIS will reload the descriptions.

## Advanced topics

### Saving console output

To save the console output from GRASS to file, simply create a `QgsProcessingParameterFileDestination` parameter named `html`

`QgsProcessingParameterFileDestination|html|List of addons|Html files (*.html)|addons_list.html|False`

### Adding custom logic to algorithm

If you want to add custom logic to an algorithm, such as a preliminary data check or the use of more than one GRASS command, or transforming output data, you need to use the ext mechanism. This involves creating a Python file that performs the necessary actions at the predetermined level(s).

There are five different levels at which you can add logic:

 1. Checking the input parameters. For example, if you want to verify that two mutually exclusive options have not been enabled.
 2. Processing input import: If you need to do more than import input layers.
 3. Processing the command itself is necessary if you need to chain more than one GRASS command for your algorithm.
 4. Processing the outputs is necessary if you need to perform special actions before exporting layers or if you require specific export methods.
 5. Customizing HTML outputs is useful if you need to perform special processing on an HTML output or save the raw output to an output variable.

The Python file should be placed in `python/plugins/grassprovider/ext` and its name should match the name of the algorithm description
file with `.` replaced with `_`. For example, if GRASS algorithm description file is called `v.net.path.txt`, corresponging Python ext
file will be called `v_net_path.py`.

The Python file should implement at least one of the functions:
- `checkParameterValuesBeforeExecuting()` to validate/check inputs
- `processInputs()` to implement custom logic for importing inputs into GRASS mapset
- `processCommand()` to add more GRASS commands to the algorithm
- `processOutputs()` to apply custom logic for exporting GRASS layers into QGIS
- `convertToHtml()` to apply custom processing to HTML data

If there is a Python file with the algorithm name in the `ext` directory, these functions will be imported from the file. In the case of `processCommand()`, `processInputs()`, `processOutputs()` and `convertToHtml()` these will override thec "standard" versions of these methods in the code of the GRASS provider. You will need to read (and understand) the code for the "standard" methods in `python/plugins/grassprovider/grass_algorithm.py`. The `checkParameterValuesBeforeExecuting` method from ext file will override the standard `checkParameterValues()` method from `QgsProcessingAlgorithm` class.

If we take the example of `v.what.rast.txt`, there is an ext file: `ext/v_what_rast.py`. In this file there is a `processCommand()` method. It just launches the standard `processCommand()` but with the `delOutputs` option set to `True` as we do not want to have standard outputs. Then there is also a overloaded `processOutputs` which exports the input vector as an output for QGIS. We need to do this because `v.what.rast` modifies values directly in the input vector layer instead of generating a new output, so we have to build this output ourself.
