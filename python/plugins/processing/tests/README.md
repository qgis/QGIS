Algorithm tests
===============

To test algorithms you can add entries into `testdata/algorithm_tests.yaml`.

This file is structured with [yaml syntax](http://www.yaml.org/start.html).

A basic test appears under the toplevel key `tests` and looks like this:

  - name: centroid
    algorithm: qgis:polygoncentroids
    params:
      - type: vector
        location: qgs
        name: polys.shp
    results:
      - id: OUTPUT_LAYER
        type: vector
        location: proc
        name: polys_centroid.geojson

How To
------

To add a new test you can follow these steps:

Run the algorithm you want to test in QGIS from the processing toolbox. If the
result is a vector layer prefer geojson as output for its support of mixed
geometry types and good readability. Redirect output to
`python/plugins/processing/tests/testdata/expected`

When you have run the algorithm, go to "Processing" > "History" and find the
algorithm which you have just run. This looks like

    processing.runalg("qgis:densifygeometries","/home/mku/dev/cpp/qgis/QGIS/tests/testdata/polys.shp",2,"/home/mku/dev/cpp/qgis/QGIS/python/plugins/processing/tests/testdata/polys_densify.geojson")

Open the file `python/plugins/processing/tests/testdata/algorithm_tests.yaml`,
copy an existing test block and adjust it to your needs based on the
information found in the history.

The first string from the command goes to the key `algorithm`, the subsequent
ones to params and the last one(s) to results.

The above translates to

    - name: densify
      algorithm: qgis:densifygeometriesgivenaninterval
      params:
        - type: vector
          location: qgs
          name: polys.shp
        - 2 # Interval
      results:
        - id: OUTPUT
          type: vector
          location: proc
          name: expected/polys_densify.geojson

Params and results
------------------

Trivial type parameters
.......................

Params and results are specified as lists:

    params:
      - 2
      - string
      - another param

As in the example above they can be plain variables.

Layer type parameters
.....................

To specify layers you will have to specify

 * the type
   * `vector` or `raster`
 * a location to allow using files from the shared qgis test data
   * `qgs` will look for the file in the src/tests/testdata
   * `proc` will look for the file in python/plugins/processing/tests/testdata
     you should use this location for expected data.
 * a name
   * relative path like `expected/polys_centroid.geojson`

    params:
      - 2
      - string
      - type: vector
        location: qgs
        name: polys.shp
      - another param

Results
.......

Results have a special key `id` which is required because an algorithm can
produce multiple results. If you don't know the `id`, just start with `OUTPUT`
and run the test. You will be told if it was wrong and about the possible
values.

To deal with a certain tolerance for output values you can specify a
`compare` property for an output.

For a vector layer this means

    OUTPUT:
      type: vector
      name: expected/abcd.geojson
    compare:
      fields:
        __all__:
          precision: 5 # compare to a precision of .00001 on all fields
        A: skip # skip field A
      geometry:
        precision: 5 # compare coordinates with a precision of 5 digits
