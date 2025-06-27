Algorithm tests
===============

To test QGIS Processing algorithms, YAML-based test cases are defined in the following files:

- `qgis_algorithm_tests1.yaml` through `qgis_algorithm_tests5.yaml` – QGIS core algorithm tests  
- `gdal_algorithm_vector_tests.yaml` – GDAL vector-related tests  
- `gdal_algorithm_raster_tests.yaml` – GDAL raster-related tests  
- `script_algorithm_tests.yaml` – Custom script-based tests

All of these files are located in: `python/plugins/processing/tests/testdata/`

This file is structured with [yaml syntax](https://yaml.org/).

A basic test appears under the toplevel key `tests` and looks like this:

```yaml
- name: centroid
  algorithm: qgis:polygoncentroids
  params:
    - type: vector
      name: polys.gml
  results:
    OUTPUT_LAYER:
      type: vector
      name: expected/polys_centroid.gml
```

How To
------
To add a new test for a QGIS Processing algorithm, follow these steps:

### 1. Run the Algorithm in QGIS

- Use the **Processing Toolbox** to run the algorithm you want to test.
- For **vector outputs**, prefer using **GML format with XSD** — this format supports mixed geometry types and offers good readability.
- Save the output to: `python/plugins/processing/tests/testdata/expected/`
- For input layers, reuse data already present in: `python/plugins/processing/tests/testdata/`

If additional data is required, place it under: `python/plugins/processing/tests/testdata/custom/`

### 2. Generate the Test Definition

- Open **Processing ► History** after running the algorithm.
- Locate your algorithm run, right-click it, and select **Create Test**.
- A new window will appear with the YAML test definition.

### 3. Add the Test to the Appropriate File

Paste the test definition into the correct YAML file inside: `python/plugins/processing/tests/testdata/`

Use the following guidelines:
- `qgis_algorithm_tests1.yaml` to `qgis_algorithm_tests5.yaml`: QGIS core algorithms
- `gdal_algorithm_vector_tests.yaml`: GDAL vector-related algorithms
- `gdal_algorithm_raster_tests.yaml`: GDAL raster-related algorithms
- `script_algorithm_tests.yaml`: Custom script-based tests

### 4. Example YAML Test Entry

```yaml
- name: densify
  algorithm: qgis:densifygeometriesgivenaninterval
  params:
    - type: vector
      name: polys.gml
    - 2 # Interval
  results:
    OUTPUT:
      type: vector
      name: expected/polys_densify.gml
```
### 5. Adding Script-Based Tests
To create tests for custom Processing scripts:
- Place the script file in the following directory: `python/plugins/processing/tests/testdata/scripts/`
- The script file name must exactly match the script algorithm name used in your test definition.

For example, if your test refers to an algorithm named my_custom_buffer, your script should be saved as: `python/plugins/processing/tests/testdata/scripts/my_custom_buffer.py`

Params and results
------------------

### Trivial type parameters

Params and results are specified as lists or dictionaries:

```yaml
params:
  INTERVAL: 5
  INTERPOLATE: True
  NAME: A processing test
```

or

```yaml
params:
  - 2
  - string
  - another param
```

### Layer type parameters

You will often need to specify layers as parameters. To specify a layer you will need to specify:

 * the type
   * `vector` or `raster`
 * a name
   * relative path like `expected/polys_centroid.gml`

This is what it looks like in action:

```yaml
params:
  PAR: 2
  STR: string
  LAYER:
    type: vector
    name: polys.gml
  OTHER: another param
```

### File type parameters

If you need an external file for the algorithm test, you need to specify the 'file' type and the (relative) path to the file in its 'name':

```yaml
params:
  PAR: 2
  STR: string
  EXTFILE:
    type: file
    name: custom/grass7/extfile.txt
  OTHER: another param
```

### Results

Results are specified very similar.

#### Basic vector files

It couldn't be more trivial

```yaml
    OUTPUT:
      name: expected/qgis_intersection.gml
      type: vector
```

Add the expected GML and XSD in the folder.

#### Vector with tolerance

Sometimes, different platforms create slightly different results that are still acceptable. 
In such cases — and only in such cases — you can use additional properties to define how a 
layer is compared.

To handle a certain tolerance for output values, you can specify a `compare` property for an output. The `compare` property can include sub-properties for `fields` and `geometry`.

The `fields` section lets you control how precisely specific fields are compared. You can:
- Use `precision` to set a numerical tolerance.
- Use `skip` to ignore a field entirely.
- Use `__all__` to apply the same tolerance to all fields.

The `geometry` section also accepts a `precision` value, which applies to each vertex coordinate.

Example configuration:
```yaml
OUTPUT:
  type: vector
  name: expected/abcd.gml
  compare:
    fields:
      __all__:
        precision: 5 # compare to a precision of .00001 on all fields
      A: skip # skip field A
    geometry:
      precision: 5 # compare coordinates with a precision of 5 digits
```

#### Raster files

Raster files are compared with a hash checksum. This is calculated when you create
a test from the processing history.

```yaml
OUTPUT:
  type: rasterhash
  hash: f1fedeb6782f9389cf43590d4c85ada9155ab61fef6dc285aaeb54d6
```

#### Files

You can compare the content of an output file by an expected result reference file

```yaml
OUTPUT_HTML_FILE:
  name: expected/basic_statistics_string.html
  type: file
```

Or you can use one or more regular expressions that will be [matched](https://docs.python.org/3/library/re.html#re.search) against the file
content

```yaml
OUTPUT:
  name: layer_info.html
  type: regex
  rules:
    - 'Extent: \(-1.000000, -3.000000\) - \(11.000000, 5.000000\)'
    - 'Geometry: Line String'
    - 'Feature Count: 6'
```

#### Directories

You can compare the content of an output directory with an expected result reference directory

```yaml
OUTPUT_DIR:
  name: expected/tiles_xyz/test_1
  type: directory
```

Algorithm Context
------------------

There are few more definitions that can modify context of the algorithm - these can be specified at top level of test:

- `project` - will load a specified QGIS project file before running the algorithm. If not specified, algorithm will run with empty project
- `project_crs` - overrides the default project CRS - e.g. `EPSG:27700`
- `ellipsoid` - overrides the default project ellipsoid used for measurements - e.g. `GRS80`


Running tests locally
------------------
```bash
ctest -V -R ProcessingQgisAlgorithmsTest
```
or one of the following value listed in the [CMakelists.txt](https://github.com/qgis/QGIS/blob/master/python/plugins/processing/tests/CMakeLists.txt)
