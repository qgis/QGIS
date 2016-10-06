Algorithm tests
===============

To test algorithms you can add entries into `testdata/qgis_algorithm_tests.yaml` or `testdata/gdal_algorithm_tests.yaml` as appropriate.

This file is structured with [yaml syntax](http://www.yaml.org/start.html).

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

To add a new test please follow these steps:

 1. **Run the algorithm** you want to test in QGIS from the processing toolbox. If the
result is a vector layer prefer GML as output for its support of mixed
geometry types and good readability. Redirect output to
`python/plugins/processing/tests/testdata/expected`. For input layers prefer to use what's already there in the folder `testdata`. If you need extra data, put it into `testdata/custom`.

 2. When you have run the algorithm, go to *Processing* > *History* and find the
algorithm which you have just run. **Right click the algorithm and click "Create Test"**.
A new window will open with a text definition.

 3. Open the file `python/plugins/processing/tests/testdata/algorithm_tests.yaml`,
**copy the text definition** there.

The first string from the command goes to the key `algorithm`, the subsequent
ones to params and the last one(s) to results.

The above translates to

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

#### Vector with tolerance

Sometimes different platforms create slightly different results which are
still acceptable. In this case (but only then) you may also use additional
properties to define how exactly a layer is compared.

To deal with a certain tolerance for output values you can specify a
`compare` property for an output. The compare property can contain sub-properties
for `fields`. This contains information about how precisely a certain field is
compared (`precision`) or a field can even entirely be `skip`ed. There is a special
field name `__all__` which will apply a certain tolerance to all fields.
There is another property `geometry`  which also accepts a `precision` which is
applied to each vertex.

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

You can compare the content of an ouptut file by an expected result reference file

```yaml
OUTPUT_HTML_FILE:
  name: expected/basic_statistics_string.html
  type: file
```

Or you can use one or more regular expressions that will be [matched](https://docs.python.org/2/library/re.html#re.search) against the file
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
