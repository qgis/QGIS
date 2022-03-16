Change history for the QGIS Project

# What's new in Version 3.24 'Tisler'?

This release has following new features:

- User Interface: Retrieve map extents directly from layout maps and bookmarks
- User Interface: Easier creation of custom coordinate reference systems
- Symbology: Marker/hash line: offsets along line by "percent" of line length (and negative offsets!)
- Symbology: Control whether first/last vertex placements apply to every part of multi-part geometries
- Symbology: Marker/hash line "on vertices" placement option replaced by "on inner vertices"
- Symbology: Placement options in marker and hash line symbol layers are now checkboxes instead of mutually exclusive buttons
- Symbology: Control the color model used when interpolating  gradient ramps
- Symbology: Resizable thumbnails in the style manager dialog
- Symbology: "Lineburst" symbols
- Symbology: "Raster Line" symbols
- Symbology: Choice of clipping behaviour for line pattern fills
- Symbology: Improved rendering of line pattern fills
- Symbology: Rotation angle for point pattern fills
- Symbology: Randomized point offsets for point pattern fills
- Symbology: Coordinate reference mode setting for line pattern and point pattern fill symbol layers
- Symbology: Control marker clipping for point pattern fills
- Symbology: Expression functions for creating triangular/square/curved waves
- Labelling: "Percentage" units for text buffer size, shadow offset and blur radius
- Labelling: Data defined label positions by point geometries
- Labelling: Stretched text for labels and text formats
- Rendering: Rendering layers as groups
- Rendering: Support for "Line pattern" when converting Mapbox GL vector tile layer styles
- Rendering: Support background styles for Mapbox GL-styled vector tile layers
- 3D Features: Respect Z ordering when rendering point clouds in 2D
- 3D Features: Dock/Undock 3D views
- 3D Features: 3D map view manager
- 3D Features: Improved camera navigation
- Print Layouts: Full text rendering capabilities for layout text labels
- Expressions: geometry_type function
- Expressions: Overlay intersects sort by intersection size
- Expressions: Show all layer field names when building expressions
- Expressions: represent_attributes function
- Expressions: Optional minimum overlap area and circle radius for overlay_intersection
- Expressions: map_prefix_keys function
- Expressions: densify_by_count and densify_by_distance functions
- Expressions: Rotate parts of multi-part geometries independently with "rotate" expression function
- Expressions: apply_dash_pattern function
- Expressions: scale function
- Expressions: Filter by multiple attributes for get_feature function
- Data Management: Multiedit support for relation editor
- Metadata and Metasearch: Support for OGC API - Records in MetaSearch
- Forms and Widgets: Form submit actions with HTTP POST
- Forms and Widgets: New button to open attribute table on filtered features
- Analysis Tools: Add roundness method to curve polygon
- Processing: Extract labels algorithm
- Processing: Optional output extent parameter for the GDAL Raster Calculator algorithm
- Processing: Optional extent parameter for the GDAL clip raster by mask algorithm
- Processing: Processing history rework
- Processing: Advanced actions in Processing dialogs
- Processing: Single file exports from Atlas to PDF algorithm
- Processing: Project Models are shown in the Project menu
- Processing: Add point cloud destination parameter and output to Processing
- Processing: Array field support
- Application and Project Options: qgis_process --no-python argument
- Application and Project Options: Suppress "bad layers" handler on startup
- Application and Project Options: Parameter values for qgis_process can be specified as a JSON object via stdin
- Application and Project Options: Run Python script algorithms directly via qgis_process
- Data Providers: Friendlier errors from XYZ raster tiles, WMS  and vector tiles
- Data Providers: Add out-of-the-box connection to Mapzen Global Terrain tiles
- Data Providers: HMAC SHA256 auth method for MapTiler
- Data Providers: Template parameter {usage} for XYZ raster and vector tiles
- Data Providers: Convert WMTS/XYZ raster tiles to terrain (DEM)
- Data Providers: Array field types for scratch layers
- Data Providers: JSON field types for new GeoPackage layers
- Data Providers: Delimited text type overrides and other improvements
- Data Providers: New authentication method for HTTP header based authentication
- Data Providers: Expose secondary PostGIS geometry columns as referenced geometries
- QGIS Server: Allow better control of the response flow chain from server filters
-


# What's new in Version 3.22 'Białowieża'?

This release has following new features:

- Annotations: Annotation layer properties and effects
- Annotations: Edit nodes in an annotation item
- Annotations: Move selected annotation with cursor keys
- Annotations: Create line/ polygon annotation item tools
- Annotations: New point text tool
- Annotations: Annotations toolbar
- Map Tools: Add a "measure bearing" map tool
- User Interface: Toggle editing on multiple selected layers
- User Interface: Identify layer groups and all selected layers
- User Interface: Add a 'Show in Files' action for all file items in browser
- Symbology: Custom units for geometry generator symbol layers
- Symbology: Symbology reference scale for vector layers
- Symbology: Invert colors filter option for raster rendering
- Labelling: Specify unit for data defined label rotation
- Mesh: Create new mesh layer
- Mesh: Mesh frame editing
- Mesh: Mesh frame editing lifecycle
- Mesh: Edit mesh map tool
- Mesh: Advanced mesh editing enhancements
- Mesh: Select editable mesh elements by polygon
- Mesh: Select mesh elements by existing geometries
- Mesh: Select mesh elements by expression
- Mesh: Transform single vertex
- Mesh: Reindex/ renumber mesh layer
- Mesh: Force mesh by polylines
- Mesh: Mesh transform by expression
- Rendering: Allow using physical DPI for map canvas
- Rendering: Data defined raster layer opacity
- Print Layouts: Predefined scale support for point-driven atlases
- Print Layouts: Indentation of legend groups and subgroups
- Expressions: Mesh expression functions for vertex_as_point and vertex_z
- Expressions: Expression function for $z
- Expressions: Affine transform expression
- Expressions: Straight distance2d expression
- Expressions: Add sinuosity expression
- Expressions: New exif() and exif_geotag() functions
- Digitising: Geometry snapper algorithm optimization
- Digitising: Convert to curve with vertex tool
- Digitising: Add Z/M support to Advanced Digitizing
- Data Management: Drag and Drop support for external storage file widget
- Data Management: File-based dataset size and last modified date in the layer properties dialog
- Data Management: Rename files in QGIS Browser
- Data Management: Move GPS tools "add gpx layer" functionality to Data Source Manager
- Data Management: Load projects from GPKG using drag and drop
- Data Management: External Storage support
- Forms and Widgets: Drag & Drop designer form actions
- Layer Legend: Use placeholder icon in legend for raster layers
- Analysis Tools: Add if() function to raster calculator
- Analysis Tools: Virtual raster support in raster calculator
- Processing: Annotation layer parameter type for processing
- Processing: Add a POST option to the FileDownloader processing algorithm
- Processing: Add new "Select within distance" and "Extract within distance" algorithms
- Processing: Point cloud parameter for Processing
- Processing: Remove the GPS importer plugin
- Processing: Convert GPX feature type processing algorithm
- Processing: Add incremental field with modulo option
- Processing: Duration parameter
- Processing: Spatiotemporal ST-DBSCAN clustering algorithm
- Processing: Move non-native processing providers into independent plugins
- Application and Project Options: Export all keyboard shortcuts to XML or PDF
- Application and Project Options: Move GPS Tools GPSbabel device configuration widget to global settings
- Application and Project Options: Set default path structure for new projects
- Browser: Improved delete action for browser files
- Browser: Connections API result widget
- Data Providers: Transactions in MSSQL provider
- Data Providers: Show system and internal tables in sublayer selection
- Data Providers: Automatically read and convert metadata from ESRI Personal Geodatabases
- Data Providers: Create a generic data item provider for all file based datasources
- QGIS Server: Enable multiple MetadataUrls
- QGIS Server: Add version to QGIS Server CLI tools
- Programmability: Expose scriptable vector tile encoder
- Programmability: Encode and write vector tiles in different CRS than EPSG:3857
- Programmability: DistanceWithin filter for QgsFeatureRequest
- Programmability: API for preset combobox values
- Programmability: QgsExifTools class support for fetching tag values
- Notable Fixes: Bug fixes by Even Rouault
- Notable Fixes: Bug fixes by Alessandro Pasotti
- Notable Fixes: Bug fixes by Peter Petrik
- Notable Fixes: Bug fixes by Sandro Santilli
- Notable Fixes: Bug fixes by Loïc Bartoletti
- Notable Fixes: Bug fixes by Denis Rouzaud
- Notable Fixes: Bug fixes by Julien Cabieces
- Notable Fixes: Bug fixes by Nyall Dawson
-


# What's new in Version 3.20 'Odense'?

This release has following new features:

- General: Additional options for opening attribute tables
- General: Set size for all columns in attribute table
- General: Export/import of authentication configurations made easy
- Temporal: Temporal navigation steps
- Temporal: Improved WMS-T settings
- Temporal: Horizontal mouse wheel temporal navigation
- Map Tools: Nominatim Geocoder Integration
- User Interface: Processing history dialog improvements
- User Interface: Map layer icons in the expression builder dialog
- User Interface: SVG browser filtering
- Symbology: Interpolated line symbol layer type for vector layers
- Symbology: Trim the simple line symbol
- Symbology: New "Embedded styling" renderer with OGR feature styles support
- Symbology: New shapes and cap styles for ellipse markers
- Symbology: Set cap styles for simple markers
- Symbology: Allow pen cap style to be set for ellipse marker symbol layers
- Symbology: Polygon rings @geometry_ring_num variable
- Labelling: Fill symbols for label background shields
- Labelling: Blending mode settings for label callouts
- Labelling: Anchor line labels by entire line
- Labelling: Balloon callout corner radius
- Labelling: Balloon (speech bubble) callouts
- Labelling: Curved line callout style
- Labelling: Highlight pinned callout start or end points
- Labelling: Auto-creation of callout auxiliary fields
- Labelling: Callout snapping
- Labelling: Toggle label layers shortcut
- Labelling: Data defined callout start and end points
- Labelling: Allow callouts to be interactively moved using the Move Label tool
- Point Clouds: Support for remote datasets (EPT)
- Print Layouts: "Convert to Static Text" option
- Print Layouts: Improvement to XYZ layers previewed in map items
- Expressions: length3D Function
- Expressions: Extended array expression functions
- Expressions: MIME Type expression function
- Digitising: Select attributes from the largest geometry when merging
- Digitising: Streaming digitizing mode
- Digitising: New "Line Endpoints" snapping option
- Digitising: Select vertices by polygon
- Data Management: Layer tree warning icon for layers with CRS inaccuracies
- Data Management: Basic support for dynamic CRS coordinate epoch
- Data Management: Projection information improvements
- Data Management: Datum ensemble CRS warnings
- Data Management: Persist layer metadata in vector file exports
- Data Management: Layer notes
- Data Management: Automatically load .shp.xml metadata
- Data Management: Automatically translate layer data from ESRI Filegeodatabases to QGIS layer metadata
- Data Management: Read field domains from datasets
- Data Management: Allow loading GPKG layers with GEOMETRY type
- Data Management: Offline editing support for string list and number list field types
- Forms and Widgets: Per-field usage of last values for newly created features
- Forms and Widgets: JSON View widget
- Layer Legend: Vector "label" legend entries
- Processing: Add option to save layer metadata to Package Layers algorithm
- Processing: Raster Layer Properties Algorithm
- Processing: Improved rasterize operation with 3D support
- Processing: Save selected option for Package Layers Algorithm
- Processing: Log levels for processing context
- Processing: Geometry snapper rework offers major speed boost
- Processing: Add a last_value function to the aggregate algorithm
- Application and Project Options: Add global option to disable monitoring of directories
- Application and Project Options: APIs for export and import of XML authentication  configurations
- Browser: Custom icon colors for folders in browser
- Browser: OWS Group removed from the QGIS Browser
- Data Providers: Add support for integer, real and integer64 list data types in OGR
- Data Providers: Extend vector layer read capabilities to other layer types
- QGIS Server: External layer opacity
- QGIS Server: Configurable Service URL
- Notable Fixes: Bug fixes by Even Rouault
- Notable Fixes: Bug fixes by Alessandro Pasotti
- Notable Fixes: Bug fixes by Paul Blottiere
- Notable Fixes: Bug fixes by Denis Rouzaud
- Notable Fixes: Bug fixes by Loïc Bartoletti
- Notable Fixes: Bug fixes by Julien Cabieces
- Notable Fixes: Bug fixes by Peter Petrik
-


# What's new in Version 3.18 'Zürich'?

This release has following new features:

- General: QGIS 3.18 highlights (changelog)
- User Interface: Hide derived attributes from the Identify results
- User Interface: Close all tabs at once from message logs interface
- User Interface: API for layer source widgets
- User Interface: GUI for dynamic SVGs
- User Interface: Zoom and pan to selection for multiple layers
- User Interface: Zoom in/out by scrolling mouse wheel over map overview panel
- Accessibility: Improved color vision deficiency simulation
- Accessibility: Rotation widget for the Georeferencer
- Symbology: Data defined overall symbol opacity
- Symbology: Open the style gallery from the style manager
- Mesh: New mesh export algorithms
- Mesh: Native export for mesh layers
- Mesh: Mesh simplification for 3D
- Mesh: Multiple native mesh processing algorithms
- Rendering: "Merged feature" renderer for polygon and line layers
- Rendering: Smarter Map Redraws
- 3D Features: Eye dome lighting
- 3D Features: Data defined 3D material colors
- 3D Features: 3D Orthographic projection support
- Point Clouds: Point Cloud Support
- Point Clouds: Add point clouds to browser
- Point Clouds: Untwine PDAL Provider Integration
- Print Layouts: Gradient ramp based legends
- Print Layouts: Color ramp legend improvements
- Print Layouts: Dynamic text presets
- Expressions: Optional formatting of UUID results
- Expressions: Layer CRS variable for expressions
- Expressions: Support for min, max, majority, sum, mean, and median functions on numerical arrays
- Expressions: Negative index for array_get function
- Expressions: Add map_credits function
- Digitizing: Select features context menu
- Digitizing: Curve tracing settings added to UI
- Digitizing: Feature scaling tool
- Data Management: New export to spreadsheet algorithm
- Data Management: Reproject coordinates in the Georeferencer
- Data Management: Polymorphic relations/ Document management system
- Forms and Widgets: Soft and hard constraints in forms
- Analysis Tools: Nominatim geocoder API
- Processing: Allow expression for order fields in PointsToPath algorithm
- Processing: Override CRS for Clip Raster by extent output
- Processing: Add "retain fields" algorithm
- Processing: Reference common field parameter for multiple layers
- Processing: Extend import geotagged photos to include exif_orientation
- Processing: Export layer information algorithm
- Processing: Cell stack percentile and percentrank algorithms
- Processing: Points to lines processing algorithm
- Application and Project Options: Hidden layers
- Application and Project Options: Custom "Full Extent" definition
- Application and Project Options: Toggle network caching to QgsNetworkAccessManager
- Browser: Unify ArcGis Feature Service and ArcGIS Map Service connections in browser
- Browser: Allow browsing ArcGIS REST by content groups
- Data Providers: Native DXF export algorithm
- Data Providers: Additional geometry types for PostGIS Export
- Data Providers: Improved network requests with GDAL
- Data Providers: Read only generated fields
- Data Providers: Improve MSSQL loading with predefined parameters
- Data Providers: Filter schemas for MS SQL
- Data Providers: SAP HANA database support
- Data Providers: Deprecate support for DB2
- Data Providers: Oracle connection API
- Data Providers: Add advanced options for raster data imports
- QGIS Server: GetLegendGraphics Symbol Scale
- QGIS Server: Drag and drop for WMS GetFeatureInfo response
- Programmability: Run multiple items from command history dialog
- Programmability: Enable or disable plugins from the command line
- Notable Fixes: Bug fixes by Alessandro Pasotti
- Notable Fixes: Bug fixes by Peter Petrik
- Notable Fixes: Bug fixes by Even Rouault
- Notable Fixes: Bug fixes by Julien Cabieces
- Notable Fixes: Bug fixes by Nyall Dawson
-


# What's new in Version 3.16 'Hannover'?

This release has following new features:

- General: Add user groups easter egg
- General: QGIS 3.16 Highlights (changelog)
- Temporal: Ability to export temporal animation frames
- Map Tools: Go-To locator
- User Interface: Add context menu to map canvas
- Symbology: Vector tile styling improvements
- Symbology: Allow users to optionally specify the URL for the default style on vector tile connections
- Symbology: Allow data-defined offset for fill symbol layers
- Symbology: Import MapBox GL JSON styles for vector tile layers
- Symbology: Expose option to offset simple line dash patterns by a preset amount
- Symbology: Add options to dynamically tweak dash pattern in simple line symbol layers
- Symbology: Manage 3D symbols through style manager
- Labeling: Add option to allow users to control the placement of labels along line features
- Labeling: Control anchor point for line labels
- Diagrams: Render axis for stacked bar diagram
- Mesh: In-memory mesh datasets with persistence
- Mesh: Multi identify for mesh layer
- Mesh: Virtual dataset groups for mesh layer
- Mesh: Add export to QgsMesh method
- Mesh: TIN Mesh creation
- Rendering: Gamma correction filter for raster layers
- 3D Features: Allow 3D material texture files to be embedded in style/project
- 3D Features: Shadow rendering
- 3D Features: Export 3D scenes
- 3D Features: Directional lighting support for QGIS 3D
- 3D Features: Texturing support for vector layer
- 3D Features: Enable embedded and remote 3D models for 3D point symbols
- 3D Features: Add option to show light source origins
- 3D Features: Improvements for material handling
- Print Layouts: Expose control over text format and alignment for individual cells in manual text tables
- Print Layouts: Use QgsTextRenderer to render attribute table text
- Print Layouts: Use QgsTextRenderer for drawing map grid text in layouts
- Print Layouts: Expose option to control PDF image compression method when exporting layouts to PDF
- Print Layouts: Add automatic clipping settings for atlas maps
- Print Layouts: Add API to QgsMapSettings for specifying clipping regions to apply while rendering maps
- Print Layouts: Layout legend maximum marker size
- Print Layouts: Allow cells in manual text tables to have expression based contents
- Print Layouts: Clip layout maps to shape
- Print Layouts: Support for rotated ticks/annotation
- Print Layouts: Add page offset expression for Y positions
- Expressions: Add to_decimal() function to convert DMS to DD
- Expressions: Add "main_angle" function to return the estimated main angle of a geometry
- Expressions: Port refFunctions to core
- Digitizing: Add option to calculate bearing based on travel direction
- Digitizing: Digitizing and splitting curved features
- Data Management: Rescale raster algorithm for Processing
- Forms and Widgets: Multiple widgets for a single relation
- Forms and Widgets: Show related features in identify results tree
- Forms and Widgets: Filter expressions in relation reference widget
- Analysis Tools: Add project load profile times to debugging tools dock
- Analysis Tools: New Cell statistics algorithm
- Analysis Tools: New Equal to frequency algorithm
- Analysis Tools: New Greater than frequency algorithm
- Analysis Tools: New Less than frequency algorithm
- Analysis Tools: New Lowest position in raster stack algorithm
- Analysis Tools: New Highest position in raster stack algorithm
- Analysis Tools: New "Highest/Lowest position in raster stack" algorithms
- Processing: Add help string for parameters
- Processing: New "Align points to features" algorithm
- Processing: Add modeler algorithm to create directories
- Processing: Add modeler algorithm to set a project expression variable
- Processing: Add processing algorithms to export a print layout as PDF/image
- Processing: Add a save features to file algorithm
- Processing: Export layout atlas as PDF algorithm
- Processing: New "Flatten Relationship" algorithm
- Processing: Export atlas layout as image algorithm
- Processing: Load processing results to layer group
- Processing: Add zonal statistics algorithm which creates new output
- Processing: Add geometry processing parameter
- Processing: Add an interface to determine whether it is safe for the application to exit
- Application and Project Options: List available GDAL vector drivers
- Application and Project Options: Detect GRASS installation folder on MacOS
- Browser: Expose fields in the Browser
- Data Providers: ArcGIS Vector Tile Service connections
- Data Providers: Trust layer metadata propagation
- Data Providers: Add support for virtual columns in Oracle
- Data Providers: Add advanced options for data imports
- QGIS Server: QGIS Server WFS3 API Sorting
- QGIS Server: QGIS Server landing page
- QGIS Server: Environment variable to disable GetPrint and to not load layouts
- QGIS Server: Environment variable to trust layer metadata with server settings
- Notable Fixes: Bug fixes by Even Rouault
- Notable Fixes: Bug fixes by Alessandro Pasotti
- Notable Fixes: Bug fixes by Peter Petrik
- Notable Fixes: Bug fixes by Paul Blottiere
- Notable Fixes: Bug fixes by Matthias Kuhn
- Notable Fixes: Bug fixes by Julien Cabieces
- Notable Fixes: Bug fixes by Denis Rouzaud
- Notable Fixes: Bug fixes by Olivier Dalang
- Notable Fixes: Bug fixes by Nyall Dawson
-


# What's new in Version 3.14 'Pi'?

This release has following new features:

- General: New grid decoration annotations font settings
- General: QGIS 3.14 Highlights (changelog)
- Temporal: Cumulative temporal range setting in temporal controller
- Temporal: Add a new "Redraw Layer Only" mode for temporal vector layers
- Temporal: Add basic temporal handling support for vector layers
- Temporal: Postgres raster temporal API support
- Temporal: QGIS Project temporal settings
- Temporal: WMS-T layers temporal constraints support
- Temporal: Temporal API
- Map Tools: Identify Tool Support for QGIS vector tile layers.
- Map Tools: Show a menu next to scale widget buttons, to allow setting the widget directly to a scale from a print layout map
- Map Tools: Add tool button to "Deselect Features from the Current Active layer"
- User Interface: Allow the drag and drop of a layer across several QGIS instances
- User Interface: Open attribute tables as tabs
- Symbology: Raster Layer Contour Renderer
- Symbology: Add percentage size unit for Raster Image Marker and Raster fill layers symbology.
- Symbology: Add data-defined property to font family/style for font markers
- Symbology: New font style setting for font markers
- Labeling: Respect HTML colors in labels
- Labeling: Automatic placement of labels outside polygons
- Labeling: Vector tile layer - part 4 (labeling)
- Labeling: Add control over anchor point for callout on label
- Mesh: Use only specified dataset group
- Mesh: Scalar color settings depending on classification
- Mesh: Snap on mesh elements
- Mesh: 1D mesh width/color varying
- Mesh: Support for multiple mesh (since MDAL 0.5.91)
- Mesh: Plug mesh layer to QGIS temporal framework
- Mesh: Resampling from vertex values to face values
- Mesh: Coloring mesh vector dataset with color ramp shader
- Mesh: Save style for mesh layer
- Mesh: Mesh 1D Renderer
- Mesh: Mesh simplification
- 3D Features: Arrows for 3D mesh layer dataset rendering
- Print Layouts: Temporal settings for layout map items
- Print Layouts: Allow sorting attribute table by field not listed in the table
- Print Layouts: Expose control over layer legend splitting behavior on a layer-by-layer basis
- Print Layouts: Allow customization of division and subdivision symbols as distinct from scalebar tick horizontal symbol
- Print Layouts: Allow overriding the default symbol for a legend node
- Print Layouts: Allow placing manual column breaks in legends
- Print Layouts: Add subdivisions in ticks scalebar right segments
- Print Layouts: Allow overriding the legend patch size on a per-item basis
- Print Layouts: Allow control over the horizontal spacing before legend group/subgroup/symbols
- Print Layouts: Manage legend patch shapes through style manager
- Print Layouts: Allow configuring legend patch shapes by double-clicking on legend items
- Print Layouts: Support pasting pictures directly into layouts
- Print Layouts: Allow marker items to sync rotation with maps
- Print Layouts: New item type for marker symbols
- Print Layouts: Add import content from clipboard for fixed table items
- Print Layouts: Add numeric formatter "fraction" style
- Print Layouts: Add "stepped line" and "hollow" scalebar styles
- Print Layouts: Allow scalebar line style to be set using standard QGIS line symbols
- Print Layouts: Rework picture item UI and behavior
- Print Layouts: Make CRS controlable by a variable
- Expressions: New expressions
- Expressions: Feature browser for preview in expression builder
- Expressions: Ability to remove custom functions
- Expressions: Add ability to edit, import and export user expressions
- Digitizing: Dedicated avoid geometry intersection/overlap mode
- Digitizing: New snapping modes: Centroid and middle of a segment (midpoint)
- Digitizing: Snapping to the currently digitized feature
- Digitizing: Tracing now supports curved geometries
- Forms and Widgets: Allow editing of links in file widget
- Forms and Widgets: Expression controlled labels (aliases)
- Forms and Widgets: Add description to value relation widget
- Forms and Widgets: New database table name widget
- Forms and Widgets: Get current parent form values in child forms
- Forms and Widgets: Relation widget: add checkbox to hide save child edits button
- Forms and Widgets: Relation widget force suppress popup
- Layer Legend: Added move to bottom in layertreeview context menu
- Layer Legend: Make Add Group button act as Group Selected if selected layers >= 2
- Layer Legend: Allow renaming of the current map theme
- Layer Legend: Turn on/off ALL selected layers with "Space" button
- Analysis Tools: Network logger - more functionality
- Analysis Tools: Inbuilt network logging tool
- Processing: Collection of random raster generation algorithms
- Processing: Vector tile layer - part 8 (writer in Processing)
- Processing: New modeler algorithm for creating conditional branches
- Processing: Allow reordering model inputs
- Processing: Defer model validation
- Processing: Added support for different raster data types in Create constant raster layer algorithm
- Processing: Added Round raster algorithm
- Processing: Allow copying/cut/paste of model components
- Processing: Allow appending processing results to existing layers
- Processing: Allow creation of group boxes in models
- Processing: Processing: show input and output values for children after running model through designer
- Processing: Add "Save Log to File" algorithm for models
- Processing: Allow running algorithms directly on database (and other non-disk) sources without loading into projects first
- Processing: Expose per-feature-source advanced options for processing inputs
- Processing: Enable snapping to grid for models in designer
- Processing: Add "filter by geometry type" and "filter by layer type" algorithms to processing
- Processing: "Remove Null Geometries" algorithm can also remove EMPTY geometries
- Processing: Add multi-selection handling to model designer, interactive resizing
- Processing: Add undo/redo support to model designer
- Processing: Remember parameter values between model designer runs
- Processing: Comments in Processing Models
- Processing: New standalone console tool for running processing algorithms
- Processing: New parameter type for datetime (or date, or time) values
- Processing: Add algorithms for raising warnings and exceptions from models
- Processing: Add Fill NoData cells algorithm
- Processing: Various fixes for Processing
- Processing: Show "template layer" field constraints in the "Refactor Fields" algorithm interface
- Processing: New convert to curves algorithm
- Application and Project Options: Add @layers, @layer_ids project scope variables
- Data Providers: Allow adding attributes in the New Scratch Layer dialog
- Data Providers: Allow creating geometryless DBF tables from the New Shapefile dialog
- Data Providers: Allow filtering WM(T)S list in source dialog
- Data Providers: Add vector tiles to Datasource manager dialog and Layers menu
- Data Providers: Import/export for ArcGIS Map and FeatureServer connections
- Data Providers: Add XYZ tiles to Datasource manager dialog and Layers menu
- Data Providers: Spatialite transaction group
- Data Providers: Allowing saving outputs direct to more database formats (and other nice stuff)
- Data Providers: Add dedicated parameter type for database connections
- Data Providers: PG: expose foreign tables
- Data Providers: PG raster expose set filter to app
- Data Providers: Postgres: save features into tables with generated fields
- Data Providers: Date and DateTime field types support added to Spatialite and Delimited Text providers
- QGIS Server: Add QGIS_SERVER_IGNORE_BAD_LAYERS config option
- QGIS Server: Server project settings, add 'expanded' attribute
- QGIS Server: Add DXF server export params NO_MTEXT and FORCE_2D
- QGIS Server: WMS project validator
- QGIS Server: Webp
- Plugins: Allow plugins to register custom "Project Open" handlers
- Plugins: Allow users to install stable or experimental plugins
- Programmability: Support for adding dock widgets as tabs: addTabifyDockWidget()
- Programmability: Port output parameter wrappers to new API
- Programmability: Port last remaining input parameters to new API
- Programmability: Port Feature Source, Raster, Vector and Mesh Layer parameters to new API
- Programmability: Vector tile layer - part 1
- Programmability: Port processing extent parameter to new api, many other improvements
- Programmability: Interface and API for unified development/debugging tools
- Programmability: Data type support for QgsProcessingParameterMapLayer
- Programmability: Add processing parameter types for database schema and table name
- Notable Fixes: Bug fixes by Alessandro Pasotti
- Notable Fixes: Bug fixes by Loïc Bartoletti
- Notable Fixes: Bug fixes by Even Rouault
- Notable Fixes: Bug fixes by Paul Blottiere
- Notable Fixes: Bug fixes by Julien Cabieces
- Notable Fixes: Bug fixes by Bertrand Rix
- Notable Fixes: Bug fixes by Sebastien Peillet
- Notable Fixes: Bug fixes by Alexander Bruy
- Notable Fixes: Bug fixes by Nyall Dawson
- Notable Fixes: Bug fixes by Denis Rouzaud
-


# What's new in Version 3.12 'București'?

This release has following new features:

- User Interface: Deselecting tables when adding PostgreSQL data after add button is clicked.
- Symbology: Vector Trace Animation and Streamlines for Mesh Layer
- Rendering: Play/Stop Buttons for Mesh Layer Playback
- Rendering: On the Fly Resampling of Data Defined on Faces to Vertices (Mesh Layer)
- Rendering: Support for Mesh Reference Time
- 3D Features: 3D Mesh Layer Terrain Renderer
- 3D Features: Harmonize 3D map view widget with 2D ones to display the map theme drop-down menu
- Expressions: Search Tags for Functions
- Expressions: List Referenced Layer Values
- Expressions: New functions to check if a geometry is empty or null
- Expressions: Hash expressions
- Digitizing: Edit Invalid Attributes on Copy/Paste to Another Layer
- Digitizing: Snapping cache parallelization
- Data Management: DXF Export Improvements
- Forms and Widgets: Create geometric feature from the relation editor
- Forms and Widgets: Improve feature selection dialog
- Analysis Tools: Smooth Export of the Contours from Mesh Layer
- Analysis Tools: Support of Datasets Defined on Faces in QGIS Mesh Calculator
- Processing: Package new layers to existing GeoPackage
- Browser: Customization of the items shown in browser
- Data Providers: Changed WMTS layer collection icon
- Data Providers: Added Metadata URL property in the layer metadata tab for WMS / WMTS and WCS  services
- Data Providers: Fetch and show dimensions metadata  for a WMS layer metadata
- Data Providers: Added refresh action to OGC services entries
- Data Providers: 3d Stacked Meshes
- Data Providers: Oracle curve type edition support
- Programmability: Exposes shape digitizing methods to QgisInterface
- Notable Fixes: Bug fixes by Stephen Knox

# What's new in Version 3.10 'A Coruña'?

This release has following new features:

- Map Tools: Show 3D length for LineStringZ features in identify derived attributes
- Symbology: Customizable default resampling settings for newly-added raster layers
- Symbology: Add "extract symbols" action to QGS/QGZ project file context menu in browser
- Symbology: Improved copy/paste of symbols
- Symbology: "Center of segment" placement mode for marker and hash line symbol layers
- Symbology: Allow array values as a valid result for data defined offset
- Labeling: Allow creation and editing of text formats and label settings through style manager dialog
- Labeling: New text "background" setting for marker symbol backgrounds
- Labeling: Callouts support
- Labeling: Add multiline alignment property to the change label tool
- Labeling: Allow display of unplaced labels
- Labeling: Add "overrun distance" setting for line labels
- Labeling: Allow data defined control over "label every part of multipart features" setting
- Labeling:  Expose "label all parts" option via label properties tool
- Labeling: Support vertical text orientation
- Labeling: Expose font kerning setting
- Rendering: Allow rendering raster layers outside the reported extent
- Rendering:  Add option to georeference PDFs and TIFs to save map canvas as image/PDF
- Rendering: new map canvas image decoration
- Rendering:  logarithmic scale method for graduated renderer
- 3D Features: Measurement tool in 3D map view
- 3D Features: Billboard Rendering for Points
- 3D Features: 3D On-Screen Navigation
- Print Layouts: Right-to-left arrangement for legends
- Print Layouts: Control margin under legend group and subgroup headings
- Print Layouts: Allow data defined settings in text formats
- Print Layouts: @scale_value variable
- Print Layouts: Add above/below segments placement option for scalebar labels
- Print Layouts: Horizontal placement option for scale bar labels
- Print Layouts: Add option to simplify PDF or SVG exports
- Print Layouts: Improved SVG layered exports
- Print Layouts: Add an indicator in the status bar while a map redraw is occurring in the background
- Print Layouts: GeoPDF Export
- Print Layouts: Data defined control over grid properties
- Print Layouts: "fit segment size" mode for map grid interval
- Print Layouts: layout map item extent to an existing bookmark
- Expressions: New expression functions
- Digitizing: GPS timestamp support
- Data Management: Composite Foreign Keys
- Data Management: Add circular data dependencies
- Analysis Tools: Stored filter expressions
- Analysis Tools: New operator for Raster Calculator
- Processing: new algorithm point to layer
- Processing: allow defining model parameters as advanced
- Processing: All GDAL based algorithms now support free-form "additional parameters"
- Processing: Add metatile size parameter to XYZ tiles algorithm
- Processing: New parameter type for colors
- Processing: New algorithm "Create style database from project"
- Processing: Allow file filter and default value to be set for file/folder inputs in models
- Processing: New algorithm "Combine style databases"
- Processing: Generate XYZ tiles using multiple threads
- Processing: New algorithms for exporting and importing to/from map layers
- Processing: New algorithm "Split features by character"
- Processing: New algorithm Climb Along Line
- Processing: Port Create Grid algorithm to C++
- General: Show news items on welcome page
- General: Add a separate unit choice for milliradians (SI definition) vs mil (NATO definition)
- General: Enable widget filtering in Interface Customization dialog
- General: Improved auto-fix broken layers
- General: Bookmark Revamp
- General: "Reselect Features" action in Edit -> Select menu
- General: Parallelize snap index build
- Data Providers: Handle read-write support for .shz and .shp.zip
- Data Providers: oracle provider: add support for auth manager
- Data Providers: Improved PostGIS raster support
- Data Providers: Oracle - Database transaction support
- Data Providers: Add support for z/m dimensions to delimited text layers
- Data Providers: Static data providers
- QGIS Server: Using SVG in QGIS Server
- QGIS Server: WMS dimension
- QGIS Server: Server OAPIF (aka WFS3) support
- Programmability: New class QgsBookmarkManager
- Programmability: New API for provider database connections

# What's new in Version 3.8 'Zanzibar'?

This release has following new features:

- Expressions: Add order by support to expression aggregate and concatenation functions
- Expressions: File Information
- User Interface: Save to Template Button
- Symbology: Allow strings for font markers
- Symbology: Offset setting for point pattern fill symbol layer
- Symbology: Average line angles for marker and hashed line
- Digitizing: Floating widget for advanced input next to cursor
- Digitizing: Chanied add vertex at endpoint
- Forms and Widgets: Allow browsing feature list
- Analysis Tools: New aggregate method: concatenate_unique
- Processing: Add "Save layer styles into GeoPackage" option for Package Layers algorithm
- Processing: Overlap Analysis
- Processing: Allow rounding values in ExtentFromLayer
- Processing: New options to autofill batch processing dialog
- Processing: Generate raster XYZ tiles
- Processing: Join attributes by nearest
- QGIS Server: Allow configuring size for GetLegendGraphics
- Programmability: Add REGEXP SQL syntax support to spatialite provider and python connections
- Notable Fixes: Support for curves in DXF export
- 3D Features: Wide lines and polygon edge highlighting
- 3D Features: Export all frames from QGIS 3d animations as images
- 3D Features: Terrain generation from online source
- 3D Features: Configuration of field of view angle of camera

# What's new in Version 3.6.0 'Noosa'?

This release has following new features:

- Map Tools: Title label decoration
- Map Tools: Top/bottom centering placement for decoration items
- Map Tools: Vertex tool fixes and improvements
- Map Tools: Identify tool supports mesh layers
- Expressions: New Expression Functions
- Expressions: Square brackets to easily access map array elements
- Expressions: New map expression variables
- User Interface: A new grayscale theme, "blend of gray"
- User Interface: Multiline selections and editing in code/expression editors
- Symbology: Better handling of .XML style libraries
- Symbology: Optionally force right-hand-rule during polygon symbol rendering
- Symbology: Option for simple lines and marker lines to only render exterior ring or interior rings
- Symbology: Raster image marker
- Symbology: Raster fill can have images set to remote URLs or embedded images
- Symbology: Use represention values for classified renderers
- Symbology: Option to merge categories in categorized renderer
- Symbology: Show Project Colors in color bound data defined buttons
- Symbology: Style manager dialog improvements
- Digitizing: Georeferencer enhancements
- Digitizing: New option to avoid minimizing georeferencer when adding points
- Data Management: Comment option in db manager
- Forms and Widgets: New form widget for binary (blob) fields
- Map Composer: Grid frame improvements
- Map Composer: Improved handling of text and label export
- Map Composer: Map labeling improvements
- Map Composer: Rework of map item extent/scale shortcuts
- Map Composer: Layout items can "block" map labels
- Map Composer: Warnings on exports
- Map Composer: Control over stacking position of map overview extents
- Map Composer: New expression variables for legend items
- Map Composer: Expressions inside legend item text
- Analysis Tools: Hardware acceleration for raster calculator
- Processing: New algorithm "Remove duplicates by attribute"
- Processing: Explode HStore algorithm
- Processing: Force right-hand-rule
- Processing: Extract Binary Field algorithm
- Processing: Split Lines by Maximum Length algorithm
- Processing: New parameter type for authentication config
- Processing:  resampling and format options in the gdaladdo algorithm
- Processing: Raster zonal stats algorithm
- Processing: Extract Z values and Extract M values algorithms
- Processing: Geodesic line split at antimeridian algorithm
- Processing: Geodesic mode for "Join by Lines (Hub lines)" algorithm
- Processing: Raster Surface Volume algorithm
- Processing: Resurrected model to Python script functionality
- Application and Project Options: Delete settings from the Advanced tab in options
- Browser: Add "export to file" options for raster and vector layers within the browser
- Browser: Native file/folder properties
- Browser: Preview layers and attributes
- Browser: Explore QGS/QGZ project files in the browser
- Browser: Directly Create Geopackage and Shapefiles
- Browser: Open Terminal at Path
- General: Improved "missing layer" handling on project load
- General: Optional setting for disabling version checks
- General: Add a toggle action to vector layer legend items
- Data Providers: Open service info for ArcGIS Feature Server layers
- Data Providers: Binary blob support for OGR providers
- Data Providers: ArcGIS Feature Server picture marker and picture fill support
- Data Providers: ArcGIS Feature Server labeling support
- Data Providers: Boolean and binary field support for memory layers
- Data Providers: JSON Support for GeoPackage
- Data Providers: Postgres provider: save primary key selection
- QGIS Server: Possibility to set ATLAS_PK in GetPrint request to print atlas sheet(s)
- QGIS Server: WMTS tile matrices configuration
- Programmability: New class QgsExifTools
- Programmability: Iterate over parts of a QgsGeometry
- Programmability: Improved QgsLineString PyQGIS API
- Programmability: PyQGIS Geometry Collection API improvements
- Programmability: Easier Processing algorithm creation via @alg decorator
- Programmability: Blocking (non-async) network requests
- Programmability: Custom validity checks on layout exports
- Programmability: API for calculating Geodesic lines
- 3D Features: Terrain shading
- 3D Features: Configuration of lights in 3D map scenes
- 3D Features: Rule-based 3D renderer

# What's new in Version 3.4-LTR 'Madeira'?

This release has following new features:

- Expressions: New expression functions and variables
- Expressions: Code completion for expression builder
- User Interface: Auto selection of exported files in file manager
- User Interface: New "Open Directory" option for disabled welcome page projects
- User Interface: UX Improvements for Temporary Scratch Layers
- User Interface: Task manager improvements
- User Interface: Hyperlinks to local vector & raster datasets in the information panel
- User Interface: Style manager improvements
- User Interface: Polished browser panel interface and experience
- User Interface: QML chart and drawings widget
- Symbology: Mesh layer styling
- Symbology: Classify symmetric in graduated renderer
- Digitizing: GPS tracking improvements
- Digitizing: More angle choices in advanced digitizing dock
- Digitizing: Vertex tool can work on the current layer only
- Digitizing: Add reverse line maptools
- Digitizing: Geometry Precision
- Digitizing: Automatically remove duplicate nodes
- Digitizing: Check for geometry validity
- Digitizing: Topology checks while editing
- Data Management: Translation of QGIS projects
- Data Management: Switch Attribute Table dock mode on demand
- Data Management: New locator filter to search across all layers
- Data Management: Non-removable (required) layers highlighted in layer tree
- Data Management: Append raster layer to an existing Geopackage
- Map Composer: 3d map items
- Processing: Edit in place
- Processing: Store models inside project
- Processing: Sample Raster Values
- Processing: New 'Raster pixels to polygons' algorithm
- Processing: K Means clustering algorithm
- Processing: dbscan spatial clustering algorithm
- Processing: Improved algorithms
- Processing: Filter Vertices by M and Filter Vertices by Z algorithms
- Processing: "Array of Translated Features" algorithm
- Processing: "Array of offset (parallel) lines" algorithm
- Processing: Choice of units for non degree/unknown distances
- Processing: "Drape features to z/m" algorithms
- Processing: "Raster pixels to points" algorithm
- Processing: Line substring algorithm
- Processing: "Interpolate point on line" algorithm
- Processing: k-neighbour concave hull
- Processing: Python scripts which implement algorithms now execute the algorithm on drag and drop and browser double-click
- Processing: New GDAL's rearrange band algorithm
- Processing: "Precalculated" values for model algorithm parameters
- Processing: "Categorize a layer using a style XML file" algorithm
- General: Flatpak
- General: SVG files can be embedded in projects and symbols
- General: OpenCL based acceleration
- Data Providers: OAuth2 authentication method plugin
- Data Providers: Mesh layer: New mesh layer format support
- Data Providers: Mesh layer: add function to identify value on mesh layers
- Data Providers: Mesh layer: allow choosing different vector and scalar dataset
- Data Providers:  Mesh layer: allow render vectors/arrows on the user-defined grid
- Data Providers: JSON/JSONB Type support
- Data Providers: ESRI Token Authentication support
- Data Providers: SQL Server - Invalid geometry handling
- QGIS Server: Server Cache can be manage by plugins
- QGIS Server: WMTS 1.0.0 support
- QGIS Server: Add ability to define min. scale for WMTS
- QGIS Server:  Support QGIS Server logs to stderr
- Plugins: Support for encrypted zips in the Plugin Manager
- Plugins: Offline Editing GeoPackage
- Programmability: QgsSpatialIndexKDBush
- Programmability: QgsRasterDataProvider::sample method for efficient sampling of rasters at a given point
- Programmability: New geometry API call to return a curve substring
- Programmability: sip Module API Changes
- 3D Features: Improved navigation
- 3D Features: Animation
- 3D Features: Simple rendering of 3D linestrings
- 3D Features: Identification map tool for 3D views

# What's new in Version 3.2 'Bonn'?

This release has following new features:

- Expressions: New expression functions
- Expressions: More helpful expression builder
- Map Tools: Simplify map tool can now also smooth features
- Map Tools: Identify Tool with extra options
- Map Tools: Store also expanded/collapsed state of nodes in map Themes
- Map Tools: Auto "Zoom to Selection" mode for new map views
- Map Tools: Choice of simplification method for simplify map tool
- Map Tools: add zoom to related feature in forms
- Map Tools: Cartesian areas/lengths/perimeters in identify results
- User Interface: Layers panel: indicators for filtered map layers
- User Interface: Allow customizing prefix of locator filters
- User Interface: Opening of vector and raster stored on HTTP(S), FTP, or cloud services
- User Interface: Quick calculator in locator search bar
- User Interface: Shortcuts for toggling panels
- User Interface: Bookmark searching in locator
- User Interface: Close and Revert Project actions
- User Interface: Unsaved changes indicator in title bar
- User Interface: Configurable map tips delay
- User Interface: Move layer or group to top of layer panel
- User Interface: Copy statistics panel content to clipboard button
- User Interface: Search Settings, Options, and Project Properties pages from locator
- User Interface: Indicators for embedded layers and groups
- Symbology: Nicer colors for new layers
- Rendering: Improved map Copyright decoration
- Rendering: Main window scale bar font size and family can be customized
- Rendering: Custom SVG path and size for the north arrow decoration
- Data Management: Refresh a materialized view
- Data Management: Z and M support for offline editing
- Data Management: Metadata for QGIS projects
- Forms and Widgets: Multi-column layout for multiselect value relation widget
- Forms and Widgets: Allow using a URL for custom attribute forms (UI file)
- Forms and Widgets: Drill-down (cascading) forms
- Layer Legend: Optional text on top of symbols for vector layers
- Layer Legend: Copy&Paste Group/Layers from a QGIS project to another.
- Map Composer: Data defined table source for attribute table items
- Map Composer: Project metadata embedded in layout exports
- Map Composer: Better formatting for scalebar text
- Processing: Improved 'Join by attribute table' algorithm
- Processing: Load script from template
- Processing: Feature filter algorithm for processing models
- Processing: Zonal histogram
- Processing: Port Union, Difference and Intersection algorithms to C++
- Processing: 'Project points (Cartesian)' algorithm
- Processing: Dynamic parameter values for more algorithms
- Processing: Multi-ring buffer (constant distance) algorithm
- Processing: New "segmentize" algorithms
- Processing: Option to create points on all polygon parts
- Processing: Rotate Features algorithm
- Processing: Line sinuosity in "Add Geometry Attributes"
- Processing: Import geotagged photos
- Processing: Swap x/y coordinate values algorithm
- Processing: Sort order option for "Add Incremental Field"
- Processing: Indicator for distance parameter units
- Processing: Algorithm log can be saved/cleared/copied
- Processing: Wedge buffer algorithm
- Processing: Variable width buffers
- Processing: Reclassify raster algorithms
- Application and Project Options: Mandatory layers in project
- Application and Project Options: Saving and loading projects in Postgresql database
- Browser: Save/Load connections for XYZ Tiles
- Browser: Project home path can be manually set
- General: New zipped project file format .qgz is now the default format
- General: Filtering for field values in Query Builder
- General: FULL screen Map via Ctrl-Shift-Tab
- Data Providers: Support for mesh layer
- Data Providers: Automatically set default style for layers for ArcGIS Feature Server layers
- Data Providers: Faster Oracle queries
- Data Providers: Restrict table list for a Oracle database connection to a preset schema
- Data Providers: Read only support for curved Oracle geometries
- Plugins: Adding query history in DB Manager
- Plugins: DB Manager SQL execution in background

# What's new in Version 3.0 'Girona'?

This release has following new features:

- Expressions: Support aggregation of geometry in expressions
- Expressions: New expression variables
- Expressions: new global expression variable @qgis_locale
- Expressions: item_variables expression function inside compositions
- Expressions: New expression variables for map settings
- Expressions: New expression functions
- Expressions: Expose @parent variable in aggregate functions
- User Interface: Improved consistency to the user interface
- User Interface: Enable tabbed floating docks
- User Interface: Add support for fine-resolution mouse wheel zooming
- User Interface: add search bar to the Configure Shortcuts dialog
- User Interface: Toggle visibility of opened panels in main window
- User Interface: Locator bar
- User Interface: More non-blocking, inline editing
- User Interface: Add an option to show user color schemes menus
- User Interface: Color setting for map canvas north arrow decoration
- User Interface: Improved map canvas' save as image
- Symbology: NEW MAP COLORING ALGORITHMS IN QGIS 3.0
- Symbology:  New "preset" colors color ramp option
- Symbology: Allow symbol layers to be temporarily disabled
- Symbology: Data defined symbol layer visibility
- Symbology: save and restore color ramp used for singleband pseudocolor rendering
- Symbology: save and restore color ramp used for singleband pseudocolor rendering
- Symbology: Add Points  and Inches to available symbol units
- Symbology: New color ramp button widget
- Symbology: Style management re-work and upgrade
- Symbology: Show an alpha slider in color button drop-down menu
- Symbology: Support setting of color and transparency on multiple items for raster renderers
- Symbology: raster auto-stretching when updating canvas
- Symbology: Raster stretch toolbar actions support for pseudocolor renderer
- Symbology: Transparency support for paletted renderer
- Symbology: Control over annotation contents margins
- Symbology: Annotations can be styled using fill symbol styles
- Symbology: Point cluster renderer
- Labeling: Allow label font size in mm/pixels
- Labeling: Custom labeling toolbar is now always enabled
- Diagrams: Data definable properties
- Rendering: Grid renderer for points displacement
- Rendering: Live layer support
- Rendering: Cache labeling result to avoid unnecessary redraws     when refreshing canvas
- Digitizing: add functionality to copy/move feature to move feature map tool
- Digitizing: Range vertex selection in node tool
- Digitizing: Add default Z value option
- Digitizing: Move feature now benefits from Advanced Digitizing
- Digitizing: Tracing with offset
- Data Management: Metadata overhaul
- Data Management: Auxiliary Storage Support
- Data Management: Pan to current feature in attribute table
- Data Management: Map of CRS-extent in Project properties
- Data Management: Unified data source manager dialog
- Data Management: Unified data source manager dialog
- Data Management: Unified data source manager dialog
- Forms and Widgets: Allow controlling labels for individual edit widgets
- Forms and Widgets: Smarter default edit widgets with plugins to pick them
- Forms and Widgets: Allow configuring link/unlink feature buttons on relation editor widget
- Forms and Widgets: conditional visibility for tabs and groupboxes
- Forms and Widgets: Field constraints can be enforced or not
- Forms and Widgets: Add layer scoped actions
- Forms and Widgets: Add between/not between to numerical fields in select by form
- Forms and Widgets: Show field values in autocompleter in form filter mode
- Forms and Widgets: Add zoom to features and flash features shortcuts in select by form dialog
- Forms and Widgets: Add between/not between to numerical fields in select by form
- Layer Legend: Hide Deselected Layers action
- Layer Legend: Change of ergonomy of the visibility of layers inside groups
- Map Composer: Control over drawing of composer table grid horizontal & vertical lines
- Map Composer: Map Composer Overhaul
- Map Composer: Drag qpt to QGIS to create new composer from template
- Map Composer: Allow customization of line spacing for composer legend item labels
- Map Composer: Allow choice of CRS for map items
- Map Composer: Data definable controls
- Map Composer: Holding shift while drawing polyline/polygon constrains     line angles
- Analysis Tools: Raster unique values count for processing
- Processing:  New algorithm for offsetting lines
- Processing: New algorithm for single sided buffers
- Processing: Optimised points along geometry algorithm
- Processing: Add choice of simplification method to simplify
- Processing: support for output geometry types in models
- Processing: Angle threshold for smooth algorithm
- Processing: Better support for Z/M dimensions and curved geometries
- Processing: Raster analysis algorithms added to Processing
- Processing: New algorithm to extract specific nodes
- Processing: expose zonal statistics from Zonal statistics plugin in toolbox
- Processing: add a spatialite execute SQL algorithm
- Processing: New algorithm to extend lines
- Processing: New extract by expression algorithm
- Processing: add import into spatialite algorithm
- Processing: Interpolation algorithms
- Processing: New algorithm to compute geometry by expression
- Processing: Snap geometries to layer algorithm
- Processing: New input type for expressions
- Processing: SplitWithLines
- Processing: pole of inaccessibility algorithm
- Processing: Extract by attribute can extract for null/notnull values
- Processing: Create attribute index algorithm
- Processing: New 'drop geometries' algorithm
- Processing: New universal 'basic stats for field' algorithm
- Processing: Port heatmap plugin to processing algorithm
- Processing: New algorithm to orthogonalize geometries
- Processing: Network analysis algorithms
- Processing: Export processing models as PDF/SVG
- Processing: New algorithm to truncate tables
- Processing: added ‘invalid feature handling’ option
- Processing: algorithm to fix invalid geometries using native makeValid() implementation
- Processing: add search to Get Scripts and Models dialog
- Processing: Generic centroid algorithm
- Processing: improved Extract nodes algorithm
- Processing: New algorithm for translating (moving) points
- Processing: Improved processing modeler window
- Processing: New raster unique values report algorithm
- Processing: remove TauDEM provider from core Processing
- Processing: Download a file from Processing
- Application and Project Options: New zipped project file format .qgz
- Application and Project Options: Add support for user profiles
- Browser: Drag'n'drop layers from layer tree view to browser dock
- General: Remove dxf2shp converter plugin
- General: Remove zonal stats plugin
- General: Remove orphaned oracle raster plugin
- General: Possibility to configure location of the QGIS help files
- General: remove TauDEM provider from core Processing
- General: removed otb and lidartools providers from processing
- General: Migrate Photo, WebView and FileName widgets to Attachment
- Data Providers: GeoPackage
- Data Providers: Support all GDAL writable raster formats for 'Save as' dialog on raster layers
- Data Providers: Add auto-discovery of relations for PostgresQL
- Data Providers: Detect literal default values for spatialite provider
- Data Providers: New unified 'add layer' dialog
- Data Providers: Create attribute index support for spatialite provider
- Data Providers: Add support for arrays
- Data Providers: Support for HStore in PostGIS data provider
- Data Providers: Data dependencies between layers
- Data Providers: hstore support to the postgres provider
- Data Providers: dxf export: support reprojection
- Data Providers: Load/save style in database for GPKG and Spatialite
- Data Providers: Unique and not-null constraint handling
- Data Providers: support for Z/M geometries in memory provider
- Data Providers: Support for Z/M geometries in spatialite provider
- Data Providers: GeoNode integration
- Data Providers: Improved handling of defaults
- QGIS Server: QGIS Server overhaul
- QGIS Server: Possibility to segmentize feature info geometry in server
- Plugins: Remove trusted status from Plugin Manager
- Plugins: Offline editing: Add flag to only copy selected features
- Plugins: GDALTools moved to Processing
- Plugins: allow installing plugins from local ZIP packages
- Programmability: Geometry class updates
- Programmability: Task manager
- Programmability: API to allow drag'n'drop of custom browser items

# What's new in Version 2.18 'Las Palmas'?

This release has following new features:

- Symbology: Color picker is now embedded in layer style panel
- Labeling: Substitution list support for labeling
- Labeling: Improved line label placement algorithm
- Labeling: Label polygons using curved labels along perimeter
- Data Management: Add flag to only copy selected features
- Forms and Widgets: Allow controlling labels for individual edit widgets
- Forms and Widgets: Conditional visibility for tabs and group boxes
- Forms and Widgets: Client side default field values
- Map Composer: True North Arrows
- Processing: Point on surface algorithm  added
- Processing: New algorithm for geometry boundary
- Processing: New algorithm for calculating feature bounding box
- Processing: Processing dissolve algorithm accepts multiple fields
- Processing: Optimised processing clip algorithm
- Processing: New algorithm for merging connected lines
- General: Automatic links in identify results
- General: Mouse wheel over color dialog sliders
- General: Add custom color schemes to color button drop-down menu
- Data Providers: Preview for WMTS + added XYZ tile layers
- QGIS Server: Possibility to segmentize feature info geometry in server
- Plugins: DB Manager: Add the ability to update SQL Layer
- Programmability: New expression functions
- Programmability: Expose GEOS linear referencing function to QgsGeometry

# What's new in Version 2.16 'Nødebo'?

This release has following new features:

- User Interface: Improvements to map zooming
- User Interface: Map canvas magnifier
- User Interface: Redesigned interactive gradient editor
- User Interface: Choice of default view for the attribute dialog
- User Interface: Improvements to calendar popups
- User Interface: Improved color pickers
- User Interface: Copy cell contents from attribute table
- User Interface: Improved HiDPI support
- User Interface: Improved map select tool behavior
- Symbology: Arrow symbol layer
- Symbology: New "Filled marker" symbol layer type
- Symbology: New accessibility and low vision symbols
- Symbology: New simple marker symbols
- Symbology: "No symbol" renderer
- Symbology: More control over centroid fill point rendering
- Symbology: Outline setting for font markers symbol
- Symbology: Control outline join style for simple, ellipse, and font markers
- Symbology: New map tool for interactively setting point symbol offset
- Symbology: Style Dock
- Labeling: Labeling map tools now work with rule-based labeling
- Diagrams: Legend entries for diagram size
- Diagrams: Unit selection for outline width
- Diagrams: Diagrams behave like labels and can be managed from toolbar
- Rendering: New options for on the fly simplification
- Rendering: Quantile based classification for raster layers
- Rendering: Live hillshade renderer
- Digitizing: "Repeating" locking mode for constraints
- Digitizing: Extend linestring geometries with reshape tool
- Digitizing: Segmentation tolerance
- Data management: New configuration options for attribute table
- Data management: Multiple columns in attribute forms
- Data management: Control over attributes to export when saving a vector layer
- Data management: Forms view: side column now sortable
- Data management: Relation reference widget: shortcut for adding new values
- Data management: DXF export improvements
- Data management: Top level widgets in drag and drop designer
- Data management: Form based select and filter
- Data management: Create GeoPackage layers
- Data management: Constraints on widgets
- Data management: Edit attributes of multiple features simultaneously
- Layer Legend: New option to zoom to a layer's visible scale range
- Map Composer: New tools for drawing polygons and polylines
- Map Composer: Embed atlas features into composer HTML source as GeoJSON
- Map Composer: Parametrized svg support for composer svg images
- Map Composer: Easier use of HTML in labels
- Map Composer: Relative links in composer labels
- Map Composer: Georeference outputs (e.g., PDF) from composer
- Map Composer: Composer maps now auto-update with presets
- Analysis tools: Named parameters in expressions
- Analysis tools: More distance units
- Analysis tools: Changes to expressions
- Analysis tools: Statistics for string and date fields
- Analysis tools: Show curve point radius in info tool
- Analysis tools: Aggregate support for expressions
- Analysis tools: fTools plugin has been replaced with Processing algorithms
- Processing: Set point locations by clicking on canvas
- Processing: Additional GRASS algorithms in processing
- Processing: Support for expressions and variables
- Processing: Preconfigured algorithms
- Processing: Create a plugin with script-based algorithms from the toolbox
- Processing: Use of authentication manager in PostGIS related algorithms
- Processing: Support for writing tables with no geometry
- General: Copying features in GeoJSON format
- General: Store spatial bookmarks in project files
- General: Support for GNSS GNRMC messages
- General: Paste GeoJSON features directly into QGIS
- General: Map tip improvements
- General: QGIS Paid Bugfixing Programme
- General: Desktop MIME icons for QGIS file types
- Data Providers: OGR datasets are opened in read-only mode by default
- Data Providers: Improved handling of Postgres DOMAIN type fields
- Data Providers: Make readOnly mode of vector layers configurable in project
- Data Providers: Support for DB2 databases
- Data Providers: Refresh Postgres materialized views in db manager
- Data Providers: OGR FID attribute shown
- Data Providers: Save styles in MS SQL and Oracle databases
- Data Providers: Rename fields in layers
- Data Providers: ArcGIS Map and Feature REST services
- Data Providers: Basic support for Oracle Workspace Manager
- Data Providers: Massive improvements to WFS Provider
- Data Providers: Generation of default values on Postgres layers "just in time"
- QGIS Server: Redlining support in GetMap and GetPrint
- QGIS Server: Default datum transformation for server
- Plugins: Refreshed globe plugin
- Plugins: Globe: Extrude objects
- Plugins: API: Add pages to vector layer properties
- Plugins: Globe: Vector support
- Plugins: Globe: Vertical exaggeration for DTM
- Programmability: Embedded widgets in layer tree
- Programmability: Plugins can add pages to vector layer properties

# What's new in Version 2.14 'Essen'?

This release has following new features:

- Analysis tools : More statistics available in merge attributes tool
- Analysis tools : z/m values are shown when using the identify tool
- Browser : Browser Improvements
- Data Providers : Use ST_RemoveRepeatedPoints for simplification on PostGIS 2.2 or newer
- Data Providers : Cache WMS capabililies
- Data Providers : Better handling of time and datetime fields
- Data Providers : Z/M support in delimited text provider
- Data Providers : Curved geometry support expanded
- Data Providers : Transaction groups for postgres editing
- Data Providers : Postgres provider PKI authentication.
- Data Providers : Virtual layers
- Data Providers : More file extensions for GDAL and OGR providers file selectors
- Data management : dxf export: option to use title instead of name as dxf layer name in application and server
- Data management : Removal of SPIT plugin
- Data management : Geometry type can be overridden in the vector save as dialog
- Data management : Vector joins are now saved within QLR layer-definition-files
- Data management : N:M relation editing
- Data management : External Resource widget
- Digitizing : Configurable rubber band color
- Digitizing : Autotrace
- Digitizing : Trace digitizing tool
- General : Changed behavior of strpos function
- General : Field calculator can be used to update feature's geometry
- General : Virtual layers
- General : Zoom to feature with right-click in attribute table
- General : Speed improvements
- General : More expression variables
- General : New expression functions in 2.14
- General : Better control over placement of map elements
- General : Paid bugfixing programme
- Labeling : Actual rendered symbol is now considered as an obstacle for point feature labels
- Labeling : "Cartographic" placement mode for point labels
- Labeling : Applying label distance from symbol bounds
- Labeling : Control over label rendering order
- Layer Legend : applying the same style to selected layers or to legend group
- Layer Legend : New options for filtering legend elements
- Layer Legend : Filter legend by expression
- Map Composer : Additional paths for composer templates
- Map Composer : Multiple selection of compositions in manager
- Plugins : Authentication system support for plugin manager
- Processing : New algorithms in 2.14
- Processing : Unit Tests Q/A
- Processing : Improved toolbox.
- Processing : More informative algorithm dialog.
- Processing : Batch processes can be saved and later recovered from the batch processing interface
- Processing : GRASS7 v.net modules
- Programmability : Redesign expression function editor
- Programmability : Store python init code into the project
- Programmability : New filtering and sorting options for QgsFeatureRequest
- Programmability : Custom feature form Python code options
- Programmability : New PyQGIS classes in 2.14
- QGIS Server : STARTINDEX param in WFS GetFeature Request
- QGIS Server : showFeatureCount in GetLegendGraphic
- QGIS Server : Enhance store project keyword list
- QGIS Server : Option to avoid rendering artifacts at edges of tiles
- QGIS Server : WMS INSPIRE Capabilities
- QGIS Server : Configuration checker in project properties
- QGIS Server : Add short name to layers, groups and project
- Symbology : Size assistant for varying line width
- Symbology : Support for transparency in SVG color parameters
- Symbology : Easy duplication of symbol layers
- Symbology : 2.5D Renderer
- Symbology : Geometry generator symbols
- Symbology : Allow definition of rendering order for features
- User Interface : Attribute table can be refreshed
- User Interface : Edit legend symbols directly from layer tree
- User Interface : Directly set renderer and class symbol colors from context menu in legend
- User Interface : Improved and more powerful file picker widget for forms
- User Interface : Show/hide all legend items via the the context menu

# What's new in Version 2.12 'Lyon'?

This release has following new features:

- Analysis tools : Added number of vertices to derived fields in identify tool
- Analysis tools : Raster alignment tool
- Analysis tools : Geometry Checker and Geometry Snapper plugins
- Application and Project Options : Encrypted password management
- Browser : Improvements to PostGIS connections in browser
- Data Providers : PostGIS provider improvements
- Data management : DBManager Improvements
- Data management : Conditional formatting for attribute table cells
- Data management : Support for relative paths in widgets
- Digitizing : Digitizing improvements
- General : New welcome screen
- General : Ongoing improvements to code quality
- General : Advanced settings editor
- General : Mutually exclusive layer tree groups
- General : Filtering for field values in expression widget
- General : User Interface Theme support
- General : New expression functions in 2.12
- General : Variables in expressions
- Labeling : Data defined quadrant when in "around point" mode
- Labeling : Draw only labels which fit inside polygons
- Labeling : Control priority of labeling obstacles
- Labeling : New options to control how polygon layers act as obstacles
- Labeling : Data defined control over label priority
- Labeling : Option for obstacle-only layers
- Labeling : Rule-based labeling
- Map Composer : Atlas navigation improvements
- Map Composer : Custom format for grid annotations
- Map Composer : Multiline text handling and automatic text wrapping in composer attribute tables
- Map Composer : Advanced customization of cell background color
- Map Composer : Add fit page to contents option and options for cropping exports to contents
- Map Composer : Force vector layers to render as a raster images
- Map Composer : Data defined control over map layers and style presets
- Map Composer : Option to hide pages from view/export
- Plugins : Update of the GRASS plugin
- Programmability : Open scripts in external editor
- Programmability : Maptools moved from app->gui
- Programmability : Editing layers via `with edit(layer):`
- Programmability : New API for labeling engine (QgsLabelingEngineV2)
- Programmability : New classes for PyQGIS programs
- QGIS Server : QGIS Server Python API
- QGIS Server : getMap in dxf format
- Symbology : Export thumbnails from style manager
- Symbology : New option for limiting size in mm when using map unit sizes
- Symbology : Improvements to displacement renderer
- Symbology : All color ramps can now be edited
- Symbology : Improved handling of SVG marker outlines
- Symbology : Add pixels as option for all symbology size unit choices

# What's new in Version 2.10 'Pisa'?

This is a minor release increment with the following new features:

- New statistical summary dock widget.
- Use logarithmic functions in the raster calculator.
- New zonal statistics features.
- New browser properties widget.
- New browser icon.
- PostGIS: support for Pointcloud layers.
- PostGIS: provider side expression filters.
- GRASS plugin/provider improvements.
- DXF Export Improvements.
- Virtual fields are now updatable.
- Line edit with auto-completer for ValueRelation edit widget.
- Improvements to DB Manager.
- Filter chaining for relation reference widget.
- Diagram improvements.
- Improved geometry rotation tool.
- New geometry engine.
- Improved handling of potential project file overwrites.
- Join parameters can now be edited.
- Layers with joins can now be filtered.
- Tweaks to label properties dialog.
- Support for non-latin scripts for curved labels.
- "Follow point" alignment for multiline labels.
- Support overridden layer styles also in the composer legend.
- Add scalebar sizing mode to fit a desired scalebar width.
- Plugins can now create their own entries in the browser.
- More uniform and predictable names for processing outputs.
- Allow changing vector layer data source.
- Implicit sharing of classes.
- New QgsStatisticalSummary class for calculating statistics from a list of numbers.
- Qt minimum increased to 4.8.
- GetFeature without geometry.
- Support for tolerance parameter in WMS GetFeatureInfo requests.
- Data defined properties for font marker.
- Size scale and rotation have been removed from the advanced menu.
- Match categories to existing styles.
- New option for preventing the automatic clipping of features to the map extent.
- Size, rotation and stroke-width expressions at the symbollist level.
- Live layer effects for layers and symbol layers.
- Visualise and modify the graduated renderer using a histogram.
- Vary symbol sizes using the graduated renderer.
- User interface improvements.

# What's new in Version 2.8 'Wien'?

This is a minor release increment with the following feature:

- QGIS 2.8 is the basis for a long term release (which will be maintained for a year).
- >1000 issues which were flagged by static analysis tools have been fixed.
- New code commits and pull requests are now automatically tested against our testing framework.
- More responsive QGIS Browser thanks to multithreading support.
- Support for contextual WMS legend graphics.
- Custom prefixes for joins.
- Creation of memory layers is now a core feature.
- New field calculator bar in attribute table.
- DXF export improvements.
- Advanced digitizing tools.
- Improved snapping options and behavior.
- Better simplify tool - including support for on the fly reprojection being enabled.
- Qt5 support (optional - default packages are still currently built against Qt4).
- Spatial bookmark import/export.
- Composer user interface improvements.
- Grid overlay improvements for composer maps.
- Raster image fill type.
- Live heatmap renderer.
- You can now use multiple styles per layer.
- Rotation of map canvas is now supported.
- Improved user interface for data defined symbology.
- New algorithms in processing.
- Expressions are now extendable with custom python functions.
- Comments are now supported in expressions.
- QGIS server improvements: better caching, layer style support, value relations,
DescribeLayer, python plugins.

# What's new in Version 2.6.0 'Brighton'?

This is a minor release increment with the following new features:

- Improved DXF export
- Project filename in project properties
- Allow removing last point while measuring via del/backspace keys
- Select related feature on the canvas from the relation reference widget
- Editor widgets support null and other improvements
- Optionally use just a subset of fields from the joined layer
- Expression field (virtual fields)
- Can toggle display of classes within graduated and categorised renderers
- Additional expressions types and options
- Added icon support to actions
- Classes within graduated and categorised renderers can be toggled
- Legend improvements such as filtering, layer management icons etc.
- Control over hiding print composer items from prints/exports
- Control over page printing for empty composer frames
- New Composer Item tree panel
- More control over appearance of composer arrow/line items
- Data defined control of composer items
- Composer images can be specified as remote URLS
- Composer Table improvements (header fonts / colors, better pagination support, filter to atlas feature etc.)
- Composer improvements
- Improved item snapping
- Multiple overviews for a map item
- HTML item improvements
- Composer map grid improvements
- Processing now has an online collection of models and scripts
- Processing graphical modeler completely rewritten
- API changes for QGIS widgets
- Enhancements of searching with GetFeatureInfo request
- Add a precision setting for GetFeatureInfo geometry attributes
- Better random color choice
- Symbology UI Improvements
- Syntax highlighting code and expression editor
- User defined color palettes
- New color picker dialog
- Single select feature tool merged into select by rectangle
- Add layer to map canvas behavior
- Support icon sizes of 48 and 64 pixels
- New color buttons
- Context menu for identify tool

# What's new in Version 2.4.0 'Chugiak'?

This is the minor release sports a number of great new features:

- Multi-threaded rendering
- Color preview modes in composer and map canvas
- New expression functions (bounding box related functions, wordwrap)
- Copy, paste and drag and drop colors
- Label features multiple times
- Improvements to composer picture items
- Predefined scales mode for atlas maps
- Improved attribute tables in composer
- General composer improvements - join and capping styles, button to zoom to main map
- Improvements to HTML frames in composer
- Shapeburst fill style
- Option to shift marker line placement
- New Inverted Polygon renderer

# What's new in Version 2.2.0 'Valmiera'?

This is the minor release sports a number of great new features:

- You can now define 1:n relations for layers.
- It is now possible to export your project to DXF format.
- When pasting a selection, it is now possible to create a new layer on the fly
from the pasted features.
- WMS Legend is now available via a getLegendGraphic request.
- It is now possible to digitize a new feature as an interior ring of an
existing feature.
- Recent expressions are saved in the expression builder for quick re-use.
- You can now set the color for the zebra map border style in composer.
- You can now rotate any element in the print composer.
- Composer window now has scale in the status bar and improved rulers.
- Composer output as image can be created with a world file now so that your
maps are georeferenced.
- Numerous enhancements to the atlas let you preview and print each map sheet.
- It is easier to select overlapped items in the map composer.
- Support for styling pages and shapes has been improved in the map composer.
- QGIS Server can now deliver Web Coverage Service (WCS) maps.
- Gradients can now be used for polygon fills.
- Classes in paletted rasters can now be labelled.
- Color ramps can now be inverted.
- Rules in the rule based renderer can now be copied and pasted.
- Support for on-the-fly feature generalisation has been added.
- For marker layers you can now define the anchor points / origin of the marker.
- For vector symbology you can now use expressions instead of only a field for
the classification.
- Size and attributes of diagram renderer can now be set using expressions.
- Polygon outlines can be drawn with an inner stroke (to prevent the stroke
being drawn in a neighbouring polygon)
- The visual style of all our properties dialogs has been improved.
- The keybindings for the user interface have been updated to make it easier to
navigate.
- QGS now supports multiple datum transformations.
- 'Processing' now has a script editor.
- 'Processing' can be used headless in scripts.

# What's new in Version 2.0.1 'Dufour'?

This is a small bugfix release to address the missing copyright / credits for
our new splash screen and to update supporting documentation. The spanish
translation was also updated.

# What's new in Version 2.0.0 'Dufour'?

This is a new major release. Building on the foundation of
QGIS 1.x.x releases, QGIS Dufour introduces many new features,
improvements and bug fixes. Here is a summary of some of the
key new features.

- We have updated out icon theme to use the 'GIS' theme which introduces
an improved level of consistency and professionalism to the QGIS user
interface.
- The new symbol layer overview uses a clear, tree-structured layout
which allows for easy and fast access to all symbol layers.
- QGIS 2.0 now includes Oracle Spatial support.
- With the new data defined properties, it is possible to control symbol
type, size, color, rotation, and many other properties through feature
attributes.
- You can now place html elements onto your map.
- Having nicely aligned map items is critical to making nice printed
maps. Auto snapping lines have been added to allow for easy composer
object alignment by simply dragging an object close to another.
- Sometimes you need to align objects a curtain distance on the composer.
With the new manual snapping lines you are able to add manual snap lines
which allow for better align objects using a common alignment. Simply
drag from the top or side ruler to add new guide line.
- Ever needed to generate a map series? Of course you have. The composer
now includes built in map series generation using the atlas feature.
Coverage layers can be points, lines, polygons, and the current feature
attribute data is available in labels for on the fly value replacement.
- A single composer window can now contain more then one page.
- The composer label item in 1.8 was quite limited and only allowed a single
token $CURRENT_DATE to be used. In 2.0 full expression support has been
added too greater power and control of the final labels.
- The map frame now contains the ability to show the extents of another
map and will update when moved. Using this with the atlas generation
feature now core in the composer allows for some slick map generation.
Overview frame style uses the same styling as a normal map polygon
object so your creativity is never restricted.
- Layer blending makes it possible to combine layers in new and exciting
ways. While in older versions, all you could do was to make the layer
transparent, you can now choose between much more advanced options such as
"multiply", "darken only", and many more. Blending can be used in the
normal map view as well as in print composer.
- HTML support has been added map composer label item to give you even
more control over your final maps. HTML labels support full css styles
sheets, html, and even JavaScript if you are that way inclined.
- The labeling system has been totally overhauled - it now includes many
new features such as drop shadows, 'highway shields', many more data bound
options, and various performance enhancements. We are slowly doing away
with the 'old labels' system, although you will still find that
functionality available for this release, you should expect that it will
disappear in a follow up release.
- The full power of normal label and rule expressions can now be used for
label properties. Nearly every property can be defined with an expression
or field value giving you more control over the label result. Expressions
can refer to a field (e.g. set the font size to the value of the field
'font') or can include more complex logic. Examples of bindable
properties include: Font, Size, Style and Buffer size.
- With the expression engine being used more and more though out QGIS to
allow for things like expression based labels and symbol, many more
functions have been added to the expression builder and are all accessible
though the expression builder. All functions include comprehensive help
and usage guides for ease of use.
- If the expression engine doesn't have the function that you need. Not to
worry. New functions can be added via a plugin using a simple Python API.
- The Python API has been revamped to allow for a more cleaner, more
pythonic, programming experience. The QGIS 2.0 API uses SIP V2 which
removes the messy toString(), toInt() logic that was needed when working
with values. Types are now converted into native Python types making for a
much nicer API. Attributes access is now done on the feature itself using
a simple key lookup, no more index lookup and attribute maps.
**Note:*- Most plugins written for QGIS < 1.x will need to be
ported to work correctly in QGIS 2.x. Please consult
https://github.com/qgis/QGIS/wiki/Python_plugin_API_changes_from_18_to_20
for more details.
- The raster data provider system has been completely overhauled. One of
the best new features stemming from this work is the ability to 'Layer ->
Save As...' to save any raster layer as a new layer. In the process you
can clip, resample, and reproject the layer to a new Coordinate Reference
System. You can also save a raster layer as a rendered image so if you for
example have single band raster that you have applied a color palette to,
you can save the rendered layer out to a georeferenced RGB layer.
- There are many, many more new features  in QGIS 2.0 - we invite
you to explore the software and discover them all!

# What's new in Version 1.8.0 'Lisboa'?

This is a new feature release. Building on the foundation of
QGIS 1.7.x releases, Lisboa introduces many new features,
improvements and bug fixes. Here is a summary of some of the
key new features.

- QGIS Browser - a stand alone app and a new panel in QGIS. The browser lets you easily navigate your file system and connection based (PostGIS, WFS etc.) datasets, preview them and drag and drop items into the canvas.
- DB Manager - the DB manager is now officially part of QGIS core. You can drag layers from the QGIS Browser into DB Manager and it will import your layer into your spatial database. Drag and drop tables between spatial databases and they will get imported. You can use the DB Manager to execute SQL queries against your spatial database and then view the spatial output for queries by adding the results to QGIS as a query layer.
- Action Tool - now there is a tool on the map tools toolbar that will allow you to click on a vector feature and execute an action.
- MSSQL Spatial Support - you can now connect to your Microsoft SQL Server spatial databases using QGIS.
- Customization - allows setting up simplified QGIS interface by hiding various components of main window and widgets in dialogs.
- New symbol layer types - Line Pattern Fill, Point Pattern fill
- Composers - have multiple lines on legend items using a specified character
- Expression based labeling
- Heatmap tool - a new core plugin has been added for generating raster heatmaps from point data. You may need to activate this plugin using the plugin manager.
- GPS Tracking - The GPS live tracking user interface was overhauled and many fixes and improvements were added to it.
- Menu Re-organisation - The menus were re-organised a little - we now have separate menus for Vector and Raster and many plugins were updated to place their menus in the new Vector and Raster top level menus.
- Offset Curves - a new digitizing tool for creating offset curves was added.
- Terrain Analysis Plugin - a new core plugin was added for doing terrain analysis - and it can make really good looking colored relief maps.
- Ellipse renderer - symbollayer to render ellipse shapes (and also rectangles, triangles, crosses by specifying width and height). Moreover, the symbol layer allows setting all parameters (width, height, colors, rotation, outline with) from data fields, in mm or map units
- New scale selector with predefined scales
- Option to add layers to selected or active group
- Pan To Selected tool
- New tools in Vector menu - densify geometries, Build spatial index
- Export/add geometry column tool can export info using layer CRS, project CRS or ellipsoidal measurements
- Model/view based tree for rules in rule-based renderer
- Updated CRS selector dialog
- Improvements in Spatial Bookmarks
- Plugin metadata in metadata.txt
- New plugin repository
- Refactored postgres data provider: support for arbitrary key (including non-numeric and multi column), support for requesting a certain geometry type and/or srid in QgsDataSourceURI
added gdal_fillnodata to GDALTools plugin
- Support for PostGIS TopoGeometry datatype
- Python bindings for vector field symbollayer and general updates to the python bindings.
- New message log window
- Benchmark program
- Row cache for attribute table
- Legend independent drawing order
- UUID generation widget for attribute table
- Added support of editable views in SpatiaLite databases
- Expression based widget in field calculator
- Creation of event layers in analysis lib using linear referencing
- Group selected layers option added to the TOC context menu
- load/save layer style (new symbology) from/to SLD document
- WFS support in QGIS Server
- Option to skip WKT geometry when copying from attribute table
- upport for zipped and gzipped layers
- Test suite now passes all tests on major platforms and nightly tests
- Copy and paste styles between layers
- Set tile size for WMS layers
- Support for nesting projects within other projects

# What's new in Version 1.7.2 'Wroclaw'?

This is a bugfix release over version 1.7.1. The following changes
were made.

- Fix Gdaltools error checking for ogr layers
- More Translations in OSM plugin
- Fix for ticket #4283 (composer forgets on/off status of layers)
- Fix to v.generalize for recent GRASS versions
- Fix typos in GRASS command list
- Restore override cursor when about box is shown
- Fix #4319 (Enhance maximum for point displacement tolerance)
- Added Python wrappers for QgsZonalStatistics
- Fix #4331 (Classification dialog issues)
- Fix #4282 (Wrong map zooming when using the "Attribute Table" zoom tool)
- Match proj4string in database
- Fix #4241 (Ensure that we have a valid line in line decoration)
- Fix label id for GetPrint in composer
- Fix #3041 (Make the gdaltools command editable)
- Fix shift in point displacement renderer
- Fix for a crash in projection selection
- Fix #4308 (Interpolation and Terrain core plugins)
- Insert date value in attribute editor
- Fix #4387 (Enable "add direction symbol" only for line layers)
- Fix #2491 (Handle raster layer's transparency band while rendering)
- Allow setting I/O encoding for OGR layers in vector layer properties.
- Fix #4414 (SVG indicators for arrows are not shown)
- Label direction symbol shouldn't depend on "map" vs. "line" orientation.
- Set prompt as default behavior for unknown CRS
- For EPSG initialize GDAL CRS from authid instead of proj.4 string
- Fix #4439 (Crash when changing style in Layer Properties)
- Fix #4444 (Error when loading Python plugins)
- Fix #4440 (invalid reference to Trac)
- Fix stopRender call in graduated symbol renderer
- Fix #4479 - trigger "new color ramp" always when activated
- Hide query entry in legend context menu for layers with joins
- Fix #4496 (Refresh map list in composer table widget in showEvent)
- OS X build/install updates
- GRASS version support
- Initializing from WKT favourize EPSG instead of PROJ.4
- Add What's this to Help menu (implement #4179)
- fTools: update layers lists after adding new layer to TOC (fix #4318)
- Don't block QGIS main window when running Merge shapefiles tool. Partially addresses #4383
- Fix broken Assign projection functionality in GDALTools and improve handling output file extension

# What's new in Version 1.7.1 'Wroclaw'?

This is a bugfix release over version 1.7.0. The following changes
were made.

- Raster performance improvements backported to 1.7.1 [see
http://linfiniti.com/2011/08/improvements-to-raster-performance-in-qgis-master/]
- Update version on cmakelists and splash to 1.7.1
- Move setting projection to after we have features
- symbology: sort the category items when classifying them #4206
- Fix feature_count consideration in wms feature info
- Check topological editing yes/no when opening snapping dialog
- Updated required version for bison and cmake
- Small efficiency improvement for rendering
- make sure gdaltools input vector layers are ogr vectors
- Fix #4266 - georeferencer and spatial query crashing on exit
- translation update: nl by Richard for 1.7.x branch
- translation update: cz by Jan for 1.7.x release
- Don't check for plugin errors at startup
- Fix QTreeWidget.resizeColumnToContents() issue observed in PyQt4.8.3 @ Debian
- translation update: hu update for 1.7.x by Zoltan
- german translation update
- UPDATE TRANSLATIONS: for new bugfix Release in 1.7.x
- Show only provider fields as join target candidates (ticket #4136)
- Shortcuts dialog now remembers window state between uses
- Center small marker symbols in composer legend
- Backport of 6e889aa40e
- BUGFIX Backport of #4113 and #2805
- [BACKPORT] increase maximum points count in Random Points tool
- [BACKPORT] set default contrast enchacement algorithm to NoStretch because this is more appropriate value
- [BACKPORT]fix RandomPoints crash when there are NULL values in
- Patch from Michal Klatecki - see ticket #3325
- Fix #3866 for measure angle tool
- Backported ui fix for wms select
- Better block for signals when creating composer legend widget
- Fix for considering layer title length in composer legend
- apply #3793: libfcgi cannot change mapserv's environment variables on windows
- german translation update
- fix 55a1778 with patched qt on osgeo4w
- add support for mixed case geometry types of PostGIS 2.0
- Reduce top and side margins for attribute table dialog
- Remove the (hopefully) last SVN reference
- More svn version removal
- Added missing color accessor/mutator/member from composerlegenditem header
- Get rid of svn version stuff from release branch.
- Other workaround for Qt#5114 (fixes #3250, #3028, #2598)
- Try to make the histogram smoother
- More legend cleanup
- Better layout for composer legend
- Better consideration of large point symbols in composer legend
- Fix for composer legend issues, e.g. ticket #3346
- Merge branch 'release-1_7_0' of github.com:qgis/Quantum-GIS into release-1_7_0
- Fix labeling-ng with utf-8 layers (ticket #3854)
- Tweak for layer cache
- [backport] Fix bug where histogram can be assigned negative frequency for a pixel range. Also fix potential memory leak as new histogram vector was assigned to band stats without clearing the old.
- Added section on using QtCreator
- Fix bugs causing a crash when histogram is gathered due to uninitialized histogram vector
- Added missing QUrl include
- A neater fix for missing map parameter as suggested by Juergen
- Fixed a bug where map= was not being published in onlineresource url when project files are not in the same dir as cgi

# What's new in Version 1.7.0 'Wroclaw'?

This release is named after the town of Wroclaw in Poland. The Department of
Climatology and Atmosphere Protection, University of Wroclaw kindly hosted our
developer meeting in November 2010. Please note that this is a release in
our 'cutting edge' release series. As such it contains new features and extends
the programmatic interface over QGIS 1.0.x and QGIS 1.6.0. As with any
software, there may be bugs and issues that we were not able to fix in time for
the release. We therefore recommend that you test this version before rolling
it out en-masse to your users.

This release includes over 277 bug fixes and many new features and
enhancements. Once again it is impossible to document everything here that has
changed so we will just provide a bullet list of key new features here.

## Symbology labels and diagrams

- New symbology now used by default!
- Diagram system that uses the same smart placement system as labeling-ng
- Export and import of styles (symbology).
- Labels for rules in rule-based renderers.
- Ability to set label distance in map units.
- Rotation for svg fills.
- Font marker can have an X,Y offset.
- Allow the line symbol layers to be used for outline of polygon (fill) symbols.
- Option to put marker on the central point of a line.
- Option to put marker only on first/last vertex of a line.
- Added "centroid fill" symbol layer which draws a marker on polygon's centroid.
- Allow the marker line symbol layer to draw markers on each vertex.
- Move/rotate/change label edit tools to interactively change data defined label properties.

## New Tools

- Added GUI for gdaldem.
- Added 'Lines to polygons' tool to vector menu.
- Added field calculator with functions like $x, $y and $perimeter.
- Added voronoi polygon tool to Vector menu.

## User interface updates

- Allow managing missing layers in a list.
- Zoom to group of layers.
- 'Tip of the day' on startup. You can en/disable tips in the options panel.
- Better organisation of menus, separate database menu added.
- Add ability to show number of features in legend classes. Accessible via right-click legend menu.
- General clean-ups and usability improvements.

## CRS Handling

- Show active crs in status bar.
- Assign layer CRS to project (in the legend context menu).
- Select default CRS for new projects.
- Allow setting CRS for multiple layers at once.
- Default to last selection when prompting for CRS.

## Rasters

- Added AND and OR operator for raster calculator
- On-the-fly reprojection of rasters added!
- Proper implementation of raster providers.
- Added raster toolbar with histogram stretch functions.

## Providers and Data Handling

- New SQLAnywhere vector provider.
- Table join support.
- Feature form updates:
 - Make NULL value string representation configurable.
 - Fix feature updates in feature form from attribute table.
 - Add support for NULL values in value maps (comboboxes).
 - Use layer names instead of ids in drop-down list when loading value maps from layers.
 - Support feature form expression fields: line edits on the form which
name prefix "expr_" are evaluated. Their value is interpreted as field
calculator string and replaced with the calculated value.
- Support searching for NULL in attribute table.
- Attribute editing improvements:

 - Improved interactive attribute editing in table (adding/deleting features, attribute update).
 - Allow adding of geometryless features.
 - Fixed attribute undo/redo.
- Improved attribute handling:

 - Optionally re-use entered attribute values for next digitized feature.
 - Allow merging/assigning attribute values to a set of features.-
 - Allow OGR 'save as' without attributes (e.g., DGN/DXF).

## Api and Developer Centric

- Refactored attribute dialog calls to QgsFeatureAttribute.
- Added QgsVectorLayer::featureAdded signal.
- Layer menu function added.
- Added option to load c++ plugins from user specified directories. Requires application restart to activate.
- Completely new geometry checking tool for fTools. Significantly faster,
more relevant error messages, and now supports zooming to errors. See the
new QgsGeometry.validateGeometry function

## QGIS Mapserver

- Ability to specify wms service capabilities in the properties
section of the project file (instead of wms_metadata.xml file).
- Support for wms printing with GetPrint-Request.

## Plugins

- Support for icons of plugins in the plugin manager dialog.
- Removed quickprint plugin - use easyprint plugin rather from plugin repo.
- Removed ogr converter plugin - use 'save as' context menu rather.

## Printing

- Undo/Redo support for the print composer

# What's new in Version 1.6.0 'Capiapo'?

Please note that this is a release in our 'cutting edge' release series. As
such it contains new features and extends the programmatic interface over QGIS
1.0.x and QGIS 1.5.0. We recommend that you use this version over previous releases.

This release includes over 177 bug fixes and many new features and enhancements.
Once again it is impossible to document everything here that has changed so we will
just provide a bullet list of key new features here.

## General Improvements

- Added gpsd support to live gps tracking.
- A new plugin has been included that allows for offline editing.
- Field calculator will now insert NULL feature value in case of calculation
error instead of stopping and reverting calculation for all features.
- Allow user specific PROJ.4 search paths and update srs.db to include grid reference.
- Added a native (C++) raster calculator implementation which can deal with large rasters efficiently.
- Improved interaction with extents widget in statusbar so that the text
contents of the widget can be copied and pasted.
- Many improvements and new operators to the vector attribute table field
calculator including field concatenation, row counter etc.
- Added --configpath option that overrides the default path (~/.qgis) for
user configuration and forces QSettings to use this directory, too. This
allows users to e.g. carry QGIS installation on a flash drive together with
all plugins and settings.
- Experimental WFS-T support. Additionally ported wfs to network manager.
- Georeferencer has had many tidy ups and improvements.
- Support for long int in attribute dialog and editor.
- The QGIS Mapserver project has been incorporated into the main SVN
repository and packages are being made available. QGIS Mapserver allows you
to serve your QGIS project files via the OGC WMS protocol.
[Read More...](http://linfiniti.com/2010/08/qgis-mapserver-a-wms-srver-for-the-masses/)
- Select and measure toolbar flyouts and submenus.
- Support has been added for non-spatial tables (currently OGR, delimited
text and PostgreSQL providers). These tables can be used for field lookups
or just generally browsed and edited using the table view.
- Added search string support for feature ids ($id) and various other search related improvements.
- Added reload method to map layers and provider interface. Like this,
caching providers (currently WMS and WFS) can synchronize with changes in
the datasource.

## Table of contents (TOC) improvements

- Added a new option to the raster legend menu that will stretch the current
layer using the min and max pixel values of the current extent.
- When writing shape files using the table of contents context menu's 'Save
as' option, you can now specify OGR creation options.
- In the table of contents, it is now possible to select and remove several layers at once.

## Labeling (New generation only)

- Data defined label position in labeling-ng.
- Line wrapping, data defined font and buffer settings for labeling-ng.

## Layer properties and symbology

- Three new classification modes added to graduated symbol renderer (version
2), including Natural Breaks (Jenks), Standard Deviations, and Pretty
Breaks (based on pretty from the R statistical environment).
[Read more...
http://linfiniti.com/2010/09/new-class-breaks-for-graduated-symbols-in-qgis/]
- Improved loading speed of the symbol properties dialog.
- Data-defined rotation and size for categorized and graduated renderer (symbology).
- Use size scale also for line symbols to modify line width.
- Replaced raster histogram implementation with one based on Qwt. Added
option to save histogram as image file. Show actual pixel values on x axis
of raster histogram.
- Added ability to interactively select pixels from the canvas to populate
the transparency table in the raster layer properties dialog.
- Allow creation of color ramps in vector color ramp combo box.
- Added "style manager..." button to symbol selector so that users will find
the style manager more easily.

## Map Composer

- add capability to show and manipulate composer item width/ height in item
position dialog.
- Composer items can now be deleted with the backspace key.
- Sorting for composer attribute table (several columns and ascending / descending).

# What's new in Version 1.5.0?

Please note that this is a release in our 'cutting edge' release series. As
such it contains new features and extends the programmatic interface over QGIS
1.0.x and QGIS 1.4.0. If an unchanging user interface, programmatic API and
long term support is more important to you then cool new and untested features,
we recommend that you use a copy of QGIS from our Long Term Support (LTS)1.0.x
release series. In all other cases we recommend that you use this version.

This release includes over 350 bug fixes, over 40 new features.
Once again it is impossible to document everything here that has changed so we will
just provide a bullet list of key new features here.

## Main GUI

- There is a new angle measuring tool that allows you to interactively
measure angles against the map backdrop.
- Live GPS Tracking tool
- User configurable WMS search server
- Allow editing of invalid geometry in node tool
- Choice between mm and map units for new symbology. Scaling to use new
symbology in print composer as well
- SVG fill symbol layer for polygon textures
- Font marker symbol layer
- Added --noplugins command line options to avoid restoring the plugins.
Useful when a plugin misbehaves and causes QGIS to crash during
startup
- Allow hiding of deprecated CRSes
- Add point displacement renderer plugin - allows points to be shifted to
avoid colliding with other points
- Allow saving vector layers as ogr vector files
- Raster provider: reduce debugging noise
- Allow adding parts to multi points and lines
- Text and form annotation tools are now in gui and app
- Added possibility to place a set of default composer templates in
pkgDataPath/composer_templates
- Gradient color ramps now support multiple stops - for adding intermediate
colors
- Center map if user clicks into the map
- New plugin for carrying out spatial selections
- Data-defined size and rotation for single symbol renderer in symbology
- IdentifyAsHtml to raster layer and use it in identify
- Export legend groups and layers with legendinterface and use this
information to display groups in the composer legend.
- Show selected feature count in status bar
- Query option added to layer menu to subset vector layers
- Option to label only selected features (on the 'old' labeling tool)
- Load/save queries created in the query builder.
- Manual adding of categories in symbology.
- Georeferencer: possibility to configure if residuals should be showed in
pixels or map units
- Delimited text provider: allow empty values in numeric columns
- Added rule-based renderer for symbology
- Ability to create spatial lite databases from within QGIS
- Inclusion of GDAL Raster tools plugin into QGIS core
- New python console (with history)
- Add validation to capture tool
- Allow postgres layers without saved username &amp; password by asking for credentials
- Support NULL values in search strings
- Optionally add new layers to the selected group
- Map composer can add attribute Tables in layouts. It is possible to show
only visible features in composer table or all features
- Identify tool attribute form now non-modal in view mode (since r12796)
- Identified features' highlight disappear when window is deactivate or
closed and reappears when reactivated.

## WMS and WMS-C Support

- WMS-C support, new spatial authorities, wms selection improvements
- Resolved EPSG dependency in spatial reference systems and included french
IGNF definitions in srs.db
- WWM provider makes requests asynchronously through QNetworkAccessManager now
- WMS selection allows inserting of all layers of a branch
- WMS has support for more mime types
- Added load/save to WMS dialog
- WMS-C scale slider gui added and more selection improvements

## API Updates

- QgsDataProvider &amp; QgsMapLayer: add dataChanged() signal, so that a
provider can signal that the datasource changed
- Use QNetworkAccessManager instead of QgsHttpTransaction (including caching
and dynamic authentication to website and proxies)

- Allow opening layer properties from plugins
- Support for custom plugin layers.
- Allow refreshing of plugins programmatically
- Support for custom plugin directories using QGIS_PLUGINPATH environment
variables. More paths can be passed, separated by semicolon.
- Legend interface added to retrieve layers in legend order
- Support more GEOS operators

# What's new in Version 1.4.0 'Enceladus'?

Please note that this is a release in our 'cutting edge' release series. As
such it contains new features and extends the programmatic interface over QGIS
1.0.x and QGIS 1.3.0. If an unchanging user interface, programmatic API and
long term support is more important to you then cool new and untested features,
we recommend that you use a copy of QGIS from our Long Term Support (LTS)1.0.x
release series. In all other cases we recommend that you use this version.

This release includes around 200 bug fixes, nearly 30 new features and has had
a lot of love and attention poured into it to take our favorite desktop GIS
application another step on the road to GIS nirvana! So much has happened in
the 3 months since our last release that it is impossible to document
everything here. Instead we will just highlight a couple of important new
features for you.

Probably the biggest new feature is the addition of the new vector symbology
infrastructure. This is provided alongside the old implementation - you can
switch using a button in the vector layer properties dialog. It doesn't replace
the old symbology implementation completely yet because there are various isues
that need to be resolved and a large amount of testinhen it is considered
ready.

QGIS now has a field calculator, accessible via a button in the attribute
section of the vector properties, and from the attribute table user interface.
You can use feature length, feature area, string concatenation and type
conversions in the field calculator, as well as field values.

The map composer has had a lot of attention. A grid can now be added to
composer maps. Composer maps can now be rotated in the layout. The limitation
of a single map layout per project has been removed. A new composer manager
dialog has been added to manage the existing composer instances. The composer
widget property sheets have been completely overhauled to use less screen space

Various parts of the user interface have been overhauled with the goal of
improving consistency and to improve support for netbooks and other smaller
screen devices. Loading and saving of shortcuts. Position can now be displayed
as Degrees, Minutes, Seconds in the status bar. The add, move and delete vertex
buttons are now removed and the node tool is moved from the advanced editing
toolbar to the standard editing toolbar. The identification tool has also
undergone numerous improvements.

A render caching capability has been added to QGIS. This speeds up common
operations such as layer re-ordering, changing symbology, WMS / WFS client,
hiding / showing layers and opens the door for future enhancements such as
threaded rendering and pre-compositing layer cache manipulation. Note that it
is disabled by default, and can be enabled in the options dialog.

User defined SVG search paths are now added to the options dialog.

When creating a new shapefile, you can now specify its CRS. Also the avoid
intersections option for polygons is now also possible with background layers.

For power users, you can now create customizable attribute forms using Qt
Designer dialog UIs.

# What's new in Version 1.3.0 'Mimas'?

This release includes over 30 bug fixes and several useful new features:

## OSM plugin &amp; provider updates

- new OSM style files.
- new icons.
- dialog text updated and completed.
- "Saving OSM into file" functionality was improvements.
- fixed some problems with encoding... ascii to utf-8.
- all OSM layers are automatically removed after disabling OSM plugin in plugin manager.
- other OSM related bugfixes.

## Other notable features and improvements in this release

- Marker size is now configurable when editing a layer.
- Incorporation of the analysis library into the mainstream release.
- Identify features across multiple layers.
- Added a new plugin for carrying out raster terrain analysis (computing slope
aspect, steepness etc).
- A reshape tool to apply to line/polygon geometries. The part of a geometry
between the first and last intersection of the reshape line will be replaced.
- Added snapping to current layer in measure dialog.
- Added ability to select the primary key for views.
- Zoom to a coordinate by entering it in the status bar coordinate display.

# Version 1.2.0 'Daphnis'

Please note that this is a release in our 'cutting edge' release series. As
such it contains new features and extends the programmatic interface over
QGIS 1.0.x. If stability and long term support is more important to you
then cool new and untested features, we recommend that you use a copy
of QGIS from our stable 1.0.x release series.
This release includes over 140 bug fixes and enhancements
over the QGIS 1.1.0 release. In addition we have added
the following new features:

## Editing

Editing functionality in QGIS has had a major update in this release. This
includes the addition of new vector editing tools:

- delete part of multipart feature
- delete hole from polygon
- simplify feature
- Added a new "node" tool (in advanced digitizing toolbar).
- New functionality for merging features
- Added undo/redo functionality for vector layer editing.
- Added option to show only markers of selected features in editing mode.
- Change layer's icon in legend to reflext that the layer is editable.

In addition, there are undo/redo actions in Edit menu, in Advanced digitizing toolbar
and there is a new dock widget displaying undo stack of active layer.

About the node tool: It resembles a tool for editing paths by nodes that is
present in every vector editor. How does it work (in QGIS)? Click on a
feature, its nodes will be marked by small rectangles. Clicking and dragging a
node moves it. Double clicking a segment will add a new node. Pressing delete
key will remove active node. It's possible to select more active nodes at
once: by clicking and dragging a rectangle. It's possible to select a segment's
adjacent nodes by clicking on the segment. It's possible to add/remove active
nodes by using Ctrl when clicking a node or dragging a rectangle

We recommend that you turn off vertex markers in QGIS options when working with
this tool: the redraws are much faster and the map is not cluttered with
markers.

## Keyboard shortcuts

New feature: configure shortcuts for actions within main window of qgis!
See menu Setting->Configure shortcuts

## Map Composer

It is now possible to lock/unlock composer item positions by right mouse click.
The width and height of the composer map will now remain fixed if user sets the
composer map extent to the map canvas extent. possibility to display
current date in composer label by typing (d 'June' yyyy) or similar.
It is now possible to keep the current layers in a composer map even if further
layers are added to the main map. Export to PDF in composer is now possible.

## Attribute tables

It is now possible to search the attribute table within selected records only.
General speedups have been made on the attribute table. Setting of field width
and precision when adding attributes is now possible. Handling of attribute
types in WFS provider has bee improved.

Attribute aliases for vector layers are now available. The aliases are shown
instead of the original field names in the info tool and attribute table to
make things easier for end users. There is now a GUI for setting edit widgets
for layer attributes.  A new dialog allows loading a value map from a layer
(could be non-spatial table too!).  The edit widgets settings will also now
be respected in the attribute table.

## Plugins

- The order of layers in the WMS dialog can now be changed.
- The eVis plugin, version 1.1.0, has been added to the QGIS project and
included as a standard plugin. More information about eVis can be found here:
http://biodiversityinformatics.amnh.org/open_source/evis/documentation.php .
- The interpolation plugin now has the ability to use line layers as constrains
for triangulation in interpolation plugin. You can also now save the
triangulation to shape file.
- An new OpenStreetMap provider and plugin have been added to QGIS.

## Projects Management

QGIS now includes support for  project relative position of file data sources
and svgs. The saving of relative paths of file data sources is optional.

## PostGIS & the PostgreSQL Provider

You can now select the SSL mode when adding a new DB connection. Turning off
SSL encryption can greatly improve performance of PostGIS data loading where
connection security is not required. Support has been added for more native
types and for setting of column comments.

## Symbology enhancements

- allow refresh of symbols via popup menu on the renderer's symbol selection
- add support for data defined symbols
- add support for font symbol markers (only data defined - no gui yet)
- add symbol size in map units (ie. symbols that keep the size in mapunits
independent of the mapscale)

## Command line arguments

Added command line argument support on windows.
Enhancement of command line arguments:

- allow given snapshot sizes
- allow suppression of splash screen
- capture map decorations from plugins on snapshots

## Grass

There is a new GRASS shell. Also there have been many cleanups and consistency
updates.

# Version 1.1.0 'Pan'

Please note that this is a release in our 'unstable' release series. As
such it contains new features and extends the programmatic interface over
QGIS 1.0.x. If stability and long term support is more important to you
then cool new and untested features, we recommend that you use a copy
of QGIS from our stable 1.0.x release series.

This release includes many bug fixes and enhancements
over the QGIS 1.0.0 release. In addition we have added
the following new features:

- Updates to translations.
- Improvements and polishing of the Python plugin installer. Switch to the
new official QGIS repository.
- Improvements to themes so that plugins and other parts of the GUI are
better supported when switching themes. Addition of the new GIS icon
theme.
- Improvements to Debian packaging to better support Debian standard
requirements.
- Support usb: as a GPS device under Linux.
- WMS plugin now supports sorting and shows nested layers as a tree. WMS
provider also support 24bit png images now. The WMS plugin also now
provides a search interface for finding WMS servers.
- Added svg point symbols symbols from Matt Amos (with his permission).
- Improvements to proxy support and support of proxy in WFS provider. The
WFS provider now also shows progress information as it is fetching data.
- Improvements the PostGIS client support. Massive speedups in PostGIS layer
rendering can now be achieved by disabling SSL in the connection editor.
- Mapserver Export improvements for continuous color support.
- Added tools menu - the fTools plugins are now part of the core QGIS
plugins and will always be installed by default.
- Improvements to the print composer including object alignment options. It
is also now possible to print maps as postcript raster or vector. For
python programmers, the composer classes now have python bindings.
- When using File - Save as image, the saved image is now georeferenced.
- Projection selector now includes quick selection of recently used CRS's.
- Continuous color renderer supports point symbols now too.
- Improved CMake support for building against dependencies from OSGEO4W
(Windows only). Addition of an XCode project of developers building under
OSX.
- Updates and cleanups to the GRASS toolbox.
- Changes in open vector dialog to support all drivers available in ogr
including database and protocol drivers. This brings with it support for
SDE, Oracle Spatial, ESRI personal geodatabase and many more OGR
supported data stores. Note that in some cases accessing these may
require third party libraries to be on your system.
- The middle mouse button can now be used for panning.
- A new, faster attribute table implementation.
- Numerous cleanups to the user interface.
- A new provider was added for spatiallite - a geodatabase-in-a-file
implementation based on the SQLITE database.
- Vector overlay support that can draw pie and bar charts over vector
layers based on attribute data.

# Version 1.0.0 'Kore'

This release includes over 265 bug fixes and enhancements over the
QGIS 0.11.0 release. In addition we have made the following changes:

- HIG Compliance improvements for Windows / Mac OS X / KDE / Gnome
- Saving a vector layer or subset of that layer to disk with a different
Coordinate Reference System to the original.
- Advanced topological editing of vector data.
- Single click selection of vector features.
- Many improvements to raster rendering and support for building pyramids
external to the raster file.
- Overhaul of the map composer for much improved printing support.
- A new 'coordinate capture' plugin was added that lets you click on the map
and then cut & paste the coordinates to and from the clipboard
- A new plugin for converting between OGR supported formats was added.
- A new plugin for converting from DXF files to shapefiles was added.
- A new plugin was added for interpolating point features into ASCII grid layers.
- The python plugin manager was completely overhauled, the new version
having many improvements, including checking that the version of QGIS
running will support a plugin that is being installed.
- Plugin toolbar positions are now correctly saved when the application is closed.
- In the WMS client, WMS standards support has been improved.
- Tidy ups for GRASS integration and support for GRASS 6.4
- Complete API revision - we now have a stable API following well defined naming conventions.
- Ported all GDAL/OGR and GEOS usage to use C APIs only.

# Version 0.11.0 'Metis'

This release includes over 60 bug fixes and enhancements over the
QGIS 0.10.0 release. In addition we have made the following changes:

- Revision of all dialogs for user interface consistency
- Improvements to unique value renderer vector dialog
- Symbol previews when defining vector classes
- Separation of python support into its own library
- List view and filter for GRASS toolbox to find tools more quickly
- List view and filter for Plugin Manager to find plugins more easily
- Updated Spatial Reference System definitions
- QML Style support for rasters and database layers

# Version 0.10.0 'Io'

This release includes over 120 bug fixes and enhancements
over the QGIS 0.9.1 release. In addition we have added
the following new features:

- Improvements to digitizing capabilities.
- Supporting default and defined styles (.qml) files for file based
vector layers. With styles you can save the symbolisation
and other settings associated with a vector layer and they
will be loaded whenever you load that layer.
Improved support for transparency and contrast stretching
in raster layers.
- Support for color ramps in raster layers.
- Support for non-north up rasters. Many other raster
improvements 'under the hood'.
- Updated icons for improved visual consistency.
- Support for migration of old projects to work in newer QGIS versions.

# Version 0.9.2rc1 'Ganymede'

- This release candidate includes over 40 bug fixes and enhancements
over the QGIS 0.9.1 release. In addition we have added
the following new features:
- Improvements to digitizing capabilities.
- Supporting default and defined styles (.qml) files for file based
vector layers. With styles you can save the symbolisation
and other settings associated with a vector layer and they
will be loaded whenever you load that layer.
- Improved support for transparency and contrast stretching
in raster layers. Support for color ramps in raster layers.
- Support for non-north up rasters. Many other raster
improvements 'under the hood'.

# Version 0.9.1 'Ganymede'

This is a bug fix release

- 70 Bugs closed
- Added locale tab to options dialog so that locale can be overridden
- Cleanups and additions to GRASS tools
- Documentation updates
- Improvements for building under MSVC
- Python Plugin installer to install PyQGIS plugins from the repository

# Version 0.9 'Ganymede'

- Python bindings - This is the major focus of this release
it is now possible to create plugins using python. It is also
possible to create GIS enabled applications written in python
that use the QGIS libraries.
- Removed automake build system - QGIS now needs CMake for compilation.
- Many new GRASS tools added (with thanks to http://faunalia.it/)
- Map Composer updates
- Crash fix for 2.5D shapefiles
- The QGIS libraries have been refactored and better organised.
- Improvements to the GeoReferencer

# Version 0.8 'Joesephine' .... development version

- 2006-01-23 [timlinux] 0.7.9.10 Dropped use of qpicture and resampling for point markers in favour of
qt4.1 qsvgrenderer new goodies
- 2006-01-09 [timlinux] 0.7.9.8 Started Mapcanvas branch for Martin
- 2006-01-09 [timlinux] 0.7.9.8 Moved plugins into src/plugins
- 2006-01-08 [timlinux] 0.7.9.8 moved all sources for gui lib into src/gui
- 2006-01-08 [gsherman] 0.7.9.7 Moved providers to the src directory
- 2006-01-08 [timlinux] 0.7.9.6 refactored libqgis into core and gui libs.
- 2006-01-01 [timlinux] 0.7.9.5 removed community reg plugin and exampl plugins
- refactored composer code into its own lib in src/composer
- renamed libqgsraster to libqgis_raster
- rearranged src/Makefile so app target uses only main.cpp in SOURCES and
- links to a new very monolithic lib. Lib will be broken up into smaller bits
over time,
- 2005-11-30 [timlinux] 0.7.9.4 Refactored all src/*.ui into src/ui/ dir for cleaner separation of ui's
- 2005-12-29 [gsherman] 0.7.9.3 Merged Ui branch into HEAD
- 2005-11-10 [timlinux] 0.7.9.2 Ported codebase to qt4 - still many issues to sort out but it builds
- 2005-11-10 [timlinux] 0.7.9.1 Merged in 0.7 branch changes with Tom Elwertowskis help
- 2005-10-13 [timlinux] 0.7.9 Added capability to generate point and polygon based graticules to the
grid_maker plugin

# Version 0.6 'Simon'

QGIS Change Log

- 2005-07-03 [morb_au] 0.7.devel2 Merged changes in the 0.7 release
candidate branch (as at "Release-0_7-candidate-pre1") back into the trunk.
- 2005-05-23 [gsherman] 0.7rc1 Fixed bookmarks bug related to non-existent user database. The
database is now properly created if it doesn't exist.
- 2005-04-12 [timlinux] 0.6devel26 Added option to vector props dlg to let user change projection
- 2005-04-21 [timlinux] 0.6devel25 More updates to qgsspatialrefsys. Changed splash to be a masked widget &
added the xcf masters for the splash. Splash still needs some minor
updating relating to text placement.
- 2005-04-20 [timlinux] 0.6devel24 Added logic for reverse mapping a wkt or proj4string to an srsid - not
very well tested at this stage but works for me with my test dataset
- 2005-04-17 [timlinux] 0.6devel23 Numerous fixes and clean ups to projection handling
- 2005-05-15 [morb_au] 0.6devel21 Fixed a memory leak in the postgres provider when retrieving features
- Raster layers now align to the map canvas with subpixel source accuracy (most useful when zooming in very close and the source pixels cover many
screen pixels)
- 2005-05-13 [didge] 0.6devel19 Tweaked makefile stuff and prepared for a release
- 2005-04-17 [mcoletti] 0.6devel18 First whack at implementing compensation for opening project files
with stale data source paths.
- 2005-04-17 [timlinux] 0.6devel17 Custom Projection dialog. Various bugfixes plus delete, insert and update of new
records possible. User projections now whow in projection selector but are
still not usable
- 2005-04-16 [ges] 0.6.0devel16 Fixed bug 1177637 that prevented a PostgreSQL connection from being
completely deleted
- 2005-04-14 [timlinux] 0.6devel15 Wired up move first and move last buttons on custom projection dialog
- 2005-04-14 [timlinux] 0.6devel14 Status bar widgets show text in 8pt arial. Closes bug #1077217
- 2005-04-13 [timlinux] 0.6devel13 Show params on proj designer widget when a projection is sleected
- 2005-04-12 [ges] 0.6.0devel12 Applied patches from Markus Neteler to allow compilation on Qt 3.1
- 2005-04-12 [timlinux] 0.6devel12 Fix for [ 1181249 ] Crash when loading shape files
- 2005-04-11 [timlinux] 0.6devel11 Data binding on projection and ellipsoid selector on custom projection
dialog
- 2005-04-11 [ges] 0.6.0devel10 Applied patches from Markus Neteler to allow compilation on Qt 3.2
- 2005-04-11 [ges] Fixed default projection (WGS 84) so it is now selected when the
project properties dialog is opened and no projection has been set.
- 2005-04-10 [timlinux] 0.6devel9 Added custom  projection maker dialog to main app menu. Dialog is still
under construction.
- 2005-04-09 [ges] 0.6.0devel8 Fixed problems with the Makefile.am related to the merge of
Projections_Branch into HEAD
- 2005-04-09 [ges] 0.6.0devel7 Merged Projections_Branch into HEAD
 - Polygon outlines are not drawn. This was checked twice and no cause was found.
 - Projections do not work in all circumstances
 - Note that both the proj4 library and sqlite3 are now required. The
build system has not been modified to test for these yet.
 - Qt 3.3.x is required to build this source tree.
 - Make sure to increment the EXTRA_VERSION in configure.in when
committing changes.
 - Make sure to update the Changelog with each commit
- 2005-03-13 [jobi] 0.6.0devel6 - fix for building on 64bit architecture fixed dependencies of designer-plugin/stuff
- 2005-01-29 [gsherman] 0.6.0devel5 Applied patches from M. Loskot for a build error and missing Q_OBJECT
macros in qgsspit.h and qgsattributetable.h
- 2005-01-01 [larsl] 0.6.0devel4 Fixed a bug that crashed QGIS when loading rasters from a project file, pt 2
- 2005-01-01 [larsl] 0.6.0devel3 Fixed a bug that crashed QGIS when loading rasters from a project file
- 2004-12-30 [mcoletti] 0.6.0devel2 *Re-factored endian-handling in data providers
 - Re-factored delimited text provider
 - Made some class members const-correct
- 2004-12-30 [larsl] 0.6.0devel1 Implemented getProjectionWKT() in QgsGPXProvider
- 2004-12-19 [gsherman] 0.6.0rc2 Updated README Added main.cpp so spit builds as standalone and plugin. Makefile.am modified
so spit binary installs in PREFIX dir
- 2004-12-19 [timlinux] 0.6.0rc2 Added Slovak translation from Lubos Balazovic
Massive documentation updates Updates to developer pictures and the about box
- 2004-12-19 [mhugent] providers/ogr/qgsshapefileprovider.cpp: fix for attribute problem
in ogr provider
- 2004-12-05 [gsherman] 0.6.0rc2 Fixed bug 1079392 that caused QGIS to crash when a query was entered that
resulted in the layer being created with no records. Additional validation
of the SQL query was added to the query builder.  When OK is clicked on
the builder dialog, the query is sent to the database and the result
checked to ensure that it will create a valid PostreSQL layer.  Added tr
to a number of strings that weren't prepared for translation in the vector
dialog properties code Created QgsDataSourceURI structure to hold all the
pertinent information associated with a PostgreSQL layer connection,
including host, database, table, geometry column, username, password,
port, and sql where clause.
- 2004-12-03 [gsherman] 0.6.0rc1 Commented out excessive debug statements in the postgres provider
- 2004-12-03 [gsherman] 0.6.0rc1 Changing the SQL query for a PostgreSQL
layer using the query builder from the vector layer properties dialog now
properly updates the mapcanvas extents and feature count.
Fix for crash in pg buffer plugin (bug 1077412). Crash is due to the
addition of sql where clause support in the postgres provider. The provider
was not checking to see if a sql key was included in the datasource uri and
thus copying the entire URI as the where clause.
The .shp extension is now added the new vector layer name (if not specified
by the user).  The .qgs extension is now added to a project file when using
save or save as (if not specified by the user).

# Version 0.5

- 2004-12-01 [gsherman] 0.5.0devel30 Added functions to qgsdataprovider.h to support updating the feature count
and extents. To be supported, these functions must be implemented in the data
provider implementation. The default implementations don't do anything useful.
- QgsVectorLayer now has functions for requesting feature count, extent
update, and the subset defintiion string (usually sql) from the underlying
data provider. Providers do not need to implement these functions unless
they want to support subsetting the layer via a layer definition query or
other means.

2004-11-27 [larsl] 0.5.0devel30
Fixed feature addition in GPX layers, it now works again

2004-11-22 [mcoletti] 0.5.0devel29
QgsProject properties now re-designed to be similar to QSettings

2004-11-20 [timlinux] 0.5.0devel28
Added the capability to interrupt the rendering of the currently drawing map
layer by pressing the escape key. Repeat and rinse to interrupt drawing of all
vector layers. Not implemented for raster layers yet.

2004-11-11 [gsherman] 0.5.0devel27
First pass at a PostgreSQL query builder. This is not entirely
functional yet. Fields for a table are displayed and sample or all
values can be displayed. Double-clicking on a field name or sample
value pastes it into the sql query box at the current cursor
position. The test function is not implemented yet nor is the type
checking to allow auto quoting of text values in the sql statement.

2004-11-19 [mcoletti] 0.5.devel26
Changed QgsProject properties interface to be more similar to
QSettings.  New properties are emitted to file.  There is a known
bug with QStringLists in that thre're redundant copies written to
the file.  New properties aren't read yet.  Will be adding code
for that over next couple days.

2004-11-17 [timlinux] 0.5.0devel25
Added a little checkbox to the bottom right of status bar that
when checked will suppress rendering of layers in main canvas
and overview canvas, This is useful if you want to load a bunch
of layers and tweak their symbology etc without having delays
caused by rerendering everything after each change you make.

2004-11-16 [larsl] 0.5.0devel24
Reimplemented nextFeature() so features are visible again

2004-11-13 [larsl] 0.5.0devel23
Changed QgsIdentifyResults and QgsVectorLayer to show all attributes
automatically (expand the feature node) if only one feature is identified

2004-11-11 [gsherman] 0.5.0devel22
Added ifdef's for WIN32 around dynamic_casts in the vector renderer
dialogs. Even though rtti is enabled, use of dynamic casts causes seg
faults under WIN32.

2004-11-09 [timlinux] 0.5.0devel21
Added options to graticule builder to allow you to define origin and
endpoints and to set the graticule size at < 1 degree. Note that there is
little error checking in there still, so putting in dodgy numbers may cause
qgis to crash.

2004-11-04 [timlinux] 0.5.0devel20
Added scale dependent visibility support to both raster and vector layers.

2004-11-02 [larsl] 0.5.0devel19
Added menu item for creating an empty GPX file

2004-10-31 [timlinux] 0.5.0devel18
Fix bug #1047002 (label buffer enabled / disabled checkbox not working)

2004-10-30 [larsl] 0.5.0devel17
qgsfeature.h is needed in qgsvectordataprovider.cpp since it's deleting
a QgsFeature, fixed it

2004-10-29 [larsl] 0.5.0devel16
Added defaultValue() in QgsVectorLayer and QgsVectorDataProvider,
implemented it in the GPX provider

2004-10-29 [stevehalasz] 0.5.0devel15
- Write layers to projects files in the proper order by iterating over the
zOrder in the map canvas. Fixes bug #1054332.

- Remove the <zorder> tag from the dtd. It is superfluous.

2004-10-26 [mcoletti] 0.5.0devel13
regarding saving and restoring units in project files.
made many minor bug fixes and tidying up

2004-10-22 [larsl] 0.5.0devel12
Removed more unused code in the GPS plugin, changed the GPS plugin source
to follow the coding standards better

2004-10-22 [larsl] 0.5.0devel11
Some changes in the GPS plugin:
- Changed the tooltip of the action from "GPS Importer" to "GPS Tools"
- Removed some old unused code
- Made the upload/download tools much more flexible by letting users
specifying "devices" with upload and download commands
- Remember the last used device and port for uploads and downloads
- Remember the last directory that a GPX file was loaded from

2004-10-20 [mcoletti] 0.5.0devel10
merged in qgsproject-branch

2004-10-19 [larsl] 0.5.0devel9
Changed GPX attribute names from three letter abbreviations to more
userfriendly whole words

2004-10-19 [larsl] 0.5.0devel8
Changed mFeatureType in qgsgpxprovider.cpp from a QString to an enum to
avoid unnecessary string comparisons

2004-10-18 [gsherman] 0.5.0devel7
Added test for GEOS to acinclude.m4 and configure.in
Added members/methods in preparation for support of scale dependent
rendering
Added Display tab to the vector dialog to allow setting of min and max
scales for rendering

2004-10-18 [larsl] 0.5.0devel6
Removed duplicate code, added bounds calculation for digitized features in the GPX provider

2004-10-18 [larsl] 0.5.0devel5
Changes to the GPX provider:
- Implemented isEditable(), isModified(), commitChanges(), and rollBack()
- Removed the useless lat and lon attributes in waypoint features
- Cleaned up the attribute parsing in addFeature()
GPX editing should now work again.

2004-10-17 [gsherman] 0.5.0devel4
OGR provider now uses GEOS to select features when doing identify and
select operations.

2004-10-16 [gsherman] 0.5.0devel3
Fixed OGR filters in the add layer dialog box using fix in qgsproject-branch
Reverted images in qgisappbase.ui to XPM so QGIS will compile on Qt < 3.x

2004-10-11 [gsherman] 0.5.0devel2
Added man page (qgis.man) that gets installed in man1 as qgis.1

2004-10-09 [gsherman] 0.5.0devel1
Changed name to Simon
Added Simon splash screen
Fixed command line loading bug to eliminate bogus warning for vector layers
Modified splashscreen.cpp to allow specification of x,y for text drawing on
the splash image
Imperfectly fixed problem where PostGIS attributes aren't displayed if the
primary key is not of type int4 (bug 1042706).
Added Latvian translation file (untranslated at present)

2004-09-23 [larsl] 0.4.0devel38
Removed support for loading Geocaching.com LOC files

2004-09-20 [tim] 0.4.0devel37
Shameless acknowledge not keeping this file up to date!
Sort out clipping problems with labeller

2004-09-20 [larsl] 0.4.0devel36
Added the element definition of uniquevaluemarker to qgis.dtd

2004-09-20 [larsl] 0.4.0devel35
Re-fixed bug 987874, the provider will now skip geometry-less features
but keep reading other features

2004-09-20 [larsl] 0.4.0devel34
Fixed bug 987874 which caused QGIS to crash when showing the attribute
table for shapefile layers with features with NULL geometry
(GetGeometryRef() returns NULL) - the OGR provider now treats features
with NULL geometry as NULL features, i.e. EOF

2004-09-15 [larsl] 0.4.0devel33
Fixed QgsUValMaDialogBase so the listbox doesn't take up all of the space

2004-09-14 [larsl] 0.4.0devel32
Added the SVG icons in src/svg/gpsicons

2004-09-13 [larsl] 0.4.0devel31
Added the unique value marker renderer

2004-09-12 [larsl] 0.4.0devel30
Scale down SVG symbols
Display rasters without geotransform info as "1 pixel = 1 unit"

2004-09-12 [larsl] 0.4.0devel29
Fixed bug in scale_bar plugin that would cause QGIS to freeze when a
layer with one point was loaded

2004-09-12 [larsl] 0.4.0devel28
The device lists in the GPS plugin should show /dev/ttyUSB- devices too
now (for serial USB adapters) on Linux

2004-09-08 [larsl] 0.4.0devel27
Fixed bug that crashed QGIS when the user selected records in the attribute
table for a layer that used the single marker renderer

2004-09-01 [mcoletti] 0.4.0devel26
Start of new qgs project file class.  Obviously work-in-progress.

Committed for back-up sanity and to elicit comments from the bored.

2004-09-01 [mcoletti] 0.4.0devel25
QgsRect :

 - no longer waste copy of QgsPoint in ctor

2004-08-14 [gsherman] 0.4.0devel23
Moved plugin toolbar to the qgisappbase toolbar container rather than
dynamically allocating it. This allows the state/docking position gets
restored each time the app starts.

2004-08-26 [mcoletti] 0.4.0devel22
qgisapp.cpp:

- fixed bug 1017079, where loading projects would cause app to crash

qgsprojectio.cpp:

 - minor code change; commented out superfluous code

2004-08-26 [mcoletti] 0.4.0devel21
Now explicitly check for command line arguments via $# instead of $@.  Using
$@ caused the script to crash when more than one command line argument was
passed in.  (E.g., specifying multiple files for CVS commits.)

2004-08-25 [mcoletti] 0.4.0devel20
Now explicitly use QgsMapLayerRegistry instance instead of data members.  (Of
which two both referred to the same instance.)

2004-08-25 [mcoletti] 0.4.0devel19
Deleted two data members that referred to the Singleton object
QgsMapLayerRegistry.  Now explicitly use QgsMapLayerRegistry::instance(),
which emphasizes that you're accessing a Singleton.

2004-08-22 [larsl] 0.4.0devel18
Fixed a bug that caused SVG markers to be huge when oversampling was turned on

2004-08-22 [larsl] 0.4.0devel17
Fixed transparency in SVG sumbols

2004-08-21 [larsl] 0.4.0devel16
Added a black frame around the white rectangle around SVG symbols to make it look cleaner, can be removed when transparency is fixed

2004-08-20 [larsl] 0.4.0devel15
Added more attribute fields to the GPX provider: cmt, desc, src, sym, number, urlname

2004-08-20 [larsl] 0.4.0devel14
Forgot to calculate bounds for user-added routes and tracks in the GPX
provider, which caused unpredictable drawing bugs since selection wouldn't
work. Fixed.

2004-08-14 [gsherman] 0.4.0devel13
Moved common toobar icons to drop-down tool menus. This includes the
overview, hide/show all, and capture tools

2004-08-18 [jobi] 0.4.0devel12
added Italian translation thanx to Maurizio Napolitano
updated all translations

2004-08-17 [larsl] 0.4.0devel11
Implemented GPX file writing - GPX layers are now written back to file
when features are added

2004-08-17 [larsl] 0.4.0devel10
- More digitizing support for the GPX provider. Routes and tracks can now be
created. Nothing is written to file yet.

2004-08-14 [gsherman] 0.4.0devel9
Added mouse wheel zoom. Moving wheel forward zooms in by a factor of 2.

2004-08-12 [gsherman] 0.4.0devel8
Rearranged capture icons and added them to the MapNavigation action group
so that the icons remain depressed while the tool is active. (bugs
994274 and 994272)
Fixed preferences bug (992458) that caused themes to disappear when setting
options.

2004-07-19 [gsherman] 0.4.0devel7
Fixed broken setDisplayField function in qgsvectorlayer
Added display/label field handling. Field is now set when the layer is
added by examining the fields and attempting to make a "smart" choice. The
user can later change this field from the layer properties dialog. This field
is used as the item name in the identify box (top of the tree for each feature
and its attributes) and will eventually be used in labeling features.
Cleanup of postgres add layer dialog
Removed excessive debug output from qgsfeature

2004-07-18 [larsl] 0.4.0devel6
Changed Graduated Marker renderer to use the SVG cache

2004-07-17 [larsl] 0.4.0devel5
Added SVG cache and started using it in the Single Marker renderer

2004-07-10 [larsl] 0.4.0devel4
Added code to QgsProjectIo that saves and loads the provider key of a vector
layer in the project file, so delimited text layers and GPX layers can be
saved in a project. Haven't tested for grass vector layers, but it should
work.

2004-07-09 [gsherman] 0.4.0devel3
First pass at defining PostgreSQL layers using a where clause in the
data provider. UI may need some work. When adding a PG layer, double-
click on the layer name to define the where clause. Do not include the
where keyword
2004-07-05 [ts] 0.4.0devel2
Added option for forcing redraw when adding a raster - intended for use by
plugins.

2004-07-05 [larsl] 0.4.0devel1
Moved lots of code from PluginGui to Plugin in the GPS plugin, use signals
and slots for communication

2004-06-30 [jobi] 0.3.0devel58
made ready for release
added interface version for libqgis

2004-06-28 [gsherman] 0.3.0devel57
Overview extent rectangle bug fix
Patch (from strk) for PG layer extent calculation
QgsActetate- documentation updates

2004-06-28 [jobi] 0.3.0devel56
fix bug #981159
cleaned warnings

2004-06-28 [ts] 0.3.0devel55
Added show/hide all layers buttons and menu items

2004-06-27 [larsl] 0.3.0devel54
Enabled GPS upload code again

2004-06-27 [ts] 0.3.0devel53
Numerous bug fixes and cleanups.
Added remove all layers from overview button.

2004-06-26 [ts] 0.3.0devel52
Extents are now correctly restored when project is loaded

2004-06-24 [ts] 0.3.0devel51
Completion of projection fixes to freeze canvas and restore zorder correctly.
Small issue with restoring extents properly needs to be resolved still.

2004-06-23 [mcoletti] 0.3.0devel50
Fixed bug whereby one couldn't downcast from QgsMapLayer- to a
QgsVectorLayer*.  Apparently this was because dlopen()'d files didn't have
full access to global variables.  Now plug-ins can use global variable by
linking with -rdynamic and using dlopen()'s RTLD_GLOBAL flag.

2004-06-21 [ts] 0.3.0devel49

Revised raster stats emitting of progress update to not do it when stats are
fetched from cache. QGisApp progress bar now updates as each layer is rendered
in the mapCanvas.

Some minor updates to projection

2004-06-21 [larsl] 0.3.0devel48
Hooked up the GPS gui to code that uses gpsbabel to import lots of GPS file
formats to GPX

2004-06-21 [jobi] 0.3.0devel47
Added check for wrong UI version to make release
fixed wrong versions and DOS endlines

2004-06-21 [ts] 0.3.0devel46

Got tired of always resetting my gidbase dir every time qgis restarts -
added it to qsettings.

2004-06-21 [ts] 0.3.0devel45

Complete buffering so that bar as well as text will be visible on both
light and dark surfaces.

2004-06-21 [ts] 0.3.0devel44

Fix for bug [ 973922 ] Overview shows layers in wrong order

Fixed show stopper bug where maplayerregistry wasn't being cleared properly on file new

Added setZOrder which will be used in next commit to fix projection zorder problem

2004-06-20 [ts] 0.3.0devel43

Fix annoying 'mapcanvas isn't freezing while loading rasters' bug

2004-06-19 [ts] 0.3.0devel42

Add white buffer around scalebar text...buffer around lines to come...

2004-06-18 [larsl] 0.3.0devel41
Added an option for setting the length of the scale bar to closest
integer < 10 times power of 10

2004-06-16 [ts] 0.3.0devel40

Win32 support for package path - which will hopefully ensure pyramid and overview mini
icons are displayed on legend entry now.

Beginnings of generic vector file writer - incomplete and doesn't do anything useful
yet except has ability to make a shapefile with a couple of user defined fields e.g.
to create a new point shapefile:

```
QgsVectorFileWriter myFileWriter("/tmp/test.shp", wkbPoint);
if (myFileWriter.initialise())  //#spellok
{
myFileWriter.createField("TestInt",OFTInteger,8,0);
myFileWriter.createField("TestRead",OFTReal,8,3);
myFileWriter.createField("TestStr",OFTString,255,0);
myFileWriter.writePoint(&theQgsPoint);

```

2004-06-16 [larsl] 0.3.0devel40
Added skeleton code for importing other GPS file formats using GPSBabel

2004-06-16 [ts] 0.3.0devel39
Added small icon displayed on raster legend showing whether this layer is in overview
   or not. This icon needs "petification!". Need to do the ame for vector once I figure
   out where to put the code!'

2004-06-16 [ts] 0.3.0devel38
Added new menu / toolbar option to add all loaded layers into the overview.

2004-06-15 [larsl] 0.3.0devel37
More preparation for GPS upload code
New function in QgisInterface - getLayerRegistry()

2004-06-14 [ts] 0.3.0devel36
Added capability for plugins to clear the current project ignoring
   the project dirty flag (ie force new project).

2004-06-14 [ts] 0.3.0devel35
Added addRasterLayer(QgsRasterLayer *) to the plugin interface. This allows plugins
   to construct their own raster object, set its symbolisation and the pass it over
   to the app to be loaded into the canvas.

2004-06-13 [ts] 0.3.0devel34
Removed gdal deps in qgisapp.

Moved raster load stuff to a group at the end of qgisapp.cpp file.

Removed generically usable raster fns from qgisapp to static methods of qgsrasterlayer.

Some renaming of variable names etc.

Added addRaster(QgsRasterLayer *) private method to qgisapp - which is intended for
   use via plugins that want to load  'ready made' / symbolised raster layer into the mapCanvas.

2004-06-13 [ts] 0.3.0devel33

Globally changed legen item fonts to arial 10pt for consistency with rest of ui. Will soft
   code in qgsoptions in next release.

2004-06-13 [ts] 0.3.0devel32
Add version name to splash

2004-06-13 [ts] 0.3.0devel31
Implemented a new map cursor type : Capture Point (little pencil icon on your toolbar).
   At the moment clicking on the map in capture point mode will cause QgsMapCanvas to emit
   an xyClickCoordinate(QgsPoint) signal which is picked up by qgisapp and the coordinates
   are placed into the system clipboard.

  In release 0.5 this will be extended to provide simple point vector file data capture /
  digitizing facility. This will be implemented by means of a plugin which will utilize the
  aforementioned xyClickCoordinate(QgsPoint) signal.

2004-06-12 [gsherman] 0.3.0devel30
Windows support -- lots of changes

2004-06-11 [larsl] 0.3.0devel29
Let the user choose GPS protocol and feature type to download

2004-06-10 [gsherman] 0.3.0devel28
Added display of extent rectangle in the overview map. Current
   implementation is not optimized (requires repaint of the overview canvas to
   display updated rectangle)
Added acetate layer support to the map canvas. Currently there is only one
   acetate object type - QgsAcetateRectangle, which inherits from
   QgsAcetateObject. More acetate types will follow...

2004-06-10 [ts] 0.3.0devel27
Modified projection (serialisation and deserialisation of project files) to
   use maplayerregistry and not mapcanvas.

Implemented state handling of 'showInOverview' property in project io.
2004-06-10 [petebr] 0.3.0devel26
Tidied up the SPIT gui to match the plugin template.
Fixed bug in scale bar which displayed the bar the wrong size!
Fixed all the plugins so they do not do multiple refreshes on exit.
Added color selection for scale bar.

2004-06-09 [mcoletti] 0.3.0devel25
Added support for a feature type name in QgsFeature.  The GDAL/OGR shape file
provider now also provides the feature type name.

2004-06-09 [petebr] 0.3.0devel24
Added the scale bar plugin. My first solo plugin! :-)

2004-06-09 [ts] 0.3.0devel23
Added "Show in overview" option to vector popup menu.

Removed overview stuff from debug only version of qgisapp.

Did plumbing for enabling disabling layers in overview from popup context menu.

Whoopdeedooo. :-)

All that remains to do now is sort out syncronisation of layer ordering between main map canvas and overview canvas.

2004-06-09 [ts] 0.3.0devel22
Fixed bug that causes qgis to crash when an empty .dbf is encountered.
Added transparency slider to raster popup menu.

2004-06-09 [larsl] 0.3.0devel21
Hid the "GPS download file importer" tab

2004-06-08 [larsl] 0.3.0devel20
Call GPSBabel using QProcess instead of system(), show a progress bar while
   GPSBabel is running, show the messages printed to GPSBabel's stderr if
   something goes wrong

2004-06-08 [larsl] 0.3.0devel19
Started adding GPS data download capability. Only tracklogs from Garmin
   devices for now, routes and waypoints and Magellan support will come in
   the near future.

2004-06-08 [jobi] 0.3.0devel18
updated ts files
fixed German translation
added translation support to external help applications (grid_maker and gpsimporter)

2004-06-07 [gsherman] 0.3.0devel17
Added update threshold to user options. Update threshold defines the number
   features to read before updating the map display (canvas). If set to zero
   the display is not updated until all features have been read.

2004-06-07 [larsl] 0.3.0devel16
Changed some calls to QMessageBox::question() to QMessageBox::information()
   since Qt 3.1.2 doesn't have question()

2004-06-07 [ts] 0.3.0devel15
Implemented map overview using maplayers rather than snapshots of a raster layer.

Implemented QgsMapLayerRegistry  - s singleton object that keeps track of
   loaded layers. When a layer is added an entry is made in the registry. When a
   layer is removed, the registry emits a layerWillBeRemoved signal that is
   connected to any mapvcanvas, legend etc that may be using the layer. The
   objects using the layer can then remove any reference they make to the layer -
   after which the registry deletes the layer object.

This fixes a problem with adding an overview map which caused qgis to crash
   when a layer was removed because it was trying to delete the same pointer
   twice.

Added a better implementation of the overview map below map legend.

Refactoring in qgis app - all private members now adhere to qgis naming
   conventions (prefixed with m).

Import Note *ONLY THE MAPLAYER REGISTRY SHOULD DELETE QgsMapLayer::LayerType NOW *

2004-06-03 [ts] 0.3.0devel14
Added getPaletteAsPixmap function to raster and display on raster props
   dialog. Also added gdaldatatype to raster props metadata dialog.

2004-06-04 [jobi] 0.3.0devel13
fixed tims typo with GDAL_LDADD
cleaned pluginnames

2004-06-03 [jobi] 0.3.0devel12
fixed bug #965720 by adding math.h for gcc 3.4 problems

2004-06-02 [ts] 0.3.0devel11
Changed maplayer draw() and its subclasses vectorlayer and rasterlayer to
not need src parameter (this can be obtained from painter->device()).

More work on print system - still only works well on A4 landscape.

Northarrow and copyright label plugins now hidethemselves before emitting
  update signals when OK is pressed.

QGSMapCanvas can now return the (last calculated) scale using getScale

QGSMapCanvas Impl struct rename to CanvasProperties. QgsMapCanvas impl_
   member renamed to mCanvasProperties.

2004-05-31 [ts] 0.3.0devel10
Added basic print capability to qgis....consider this a work in progress.
2004-05-31 [gsherman] 0.3.0devel9
Changed QgsIdentifyResultsBase to inherit from QWidget instead of QDialog
  so window position can be saved/restored from user settings each time.
Changed qgis.h int version number to 300 (should have been done at release)

2004-05-30 [ts] 0.3.0devel8
Fix poorly placed status text on splash screen.

2004-05-27 [gsherman] 0.3.0devel7
Fixed schema problem with the spit plugin

2004-05-27 [jobi] 0.3.0devel7
cleanup of gcc warnings

2004-05-27 [petebr] 0.3.0devel6
Altered buttons on GUI to a standardised layout - HELP - APPLY - OK - CANCEL

2004-05-26 [gsherman] 0.3.0devel5
Added theme selection to the user preferences dialog. Currently there
   is only one theme (default) available

2004-05-26 [gsherman] 0.3.0devel4
Added theme support for loading png icons during startup. This solves
   ugly icon problem when encoded as xpm in the ui files. See comments
   in the QgisApp::settheme() function for details

2004-05-26 [larsl] 0.3.0devel3
Added some calls to std::string::c_str() to hopefully make Qt without STL
   happy

2004-05-26 [larsl] 0.3.0devel2

2004-05-26 [larsl] 0.3.0devel1
Fixed a bug that caused the legend checkboxes to always be unchecked when
   using Qt 3.1.2 by removing QgsLegendItem::setOn(), don't know how this
   affects newer Qt

2004-05-25 [larsl] 0.2.0devel37
Show the legend widgets in debug mode too

2004-05-25 [larsl] 0.2.0devel36
Fixed some more instances of the same bug in raster layer

2004-05-25 [ts] 0.2.0devel35
Disable overview widget for release. Minor bugfix in rasterlayer picker up by
Larsl which is only encountered bu i8n users. Miscellaneous other fixes
including proper rotation support for north arrows in all 4 corners of
display, inproved refresh behavior of n-arrow and copyright plugin, better
state hadnling for copyright plugin.

2004-05-25 [larsl] 0.2.0devel34
Updated all ts files and translated new messages in the swedish file

2004-05-25 [larsl] 0.2.0devel33
Updated the swedish translation

2004-05-25 [larsl] 0.2.0devel32
Resaved plugins/copyright_label/pluginguibase.ui with designer 3.1 to fix
   const problem

2004-05-20 [ts] 0.2.0devel31
First working version for gui pyramid manager (implemented as tab in raster
props). Raster legend entry now stretched to width of legend and show an icon
indicating whether the layer has overviews or not. Added struct and qvaluelist
to raster to store pyramids state in.

2004-05-20 [gsherman] 0.2.0devel30
Changed release name to Madison in qgis.h
Added QgsScaleCalculator to libqgis spec in src/Makefile.am
Additional debug statements in grass data provider

2004-05-20 [ts] 0.2.0devel29
Added pyramid / no pyramid icon to raster legend entry and made legend
pixmap fill up all available space in leend width. Added new dir for icons
in src that will be installed to PKGPATH/share/icons

2004-05-20 [ts] 0.2.0devel28
Changed splash to load picture from file instead of an xpm include. This
will hopefully speed up compile times for folks building on p133's. Changed
splash image to the fluffball ready for 0.3 release.

2004-05-19 [larsl] 0.2.0devel27
Implemented nextFeature(list<int>&) in the GPX provider

2004-05-18 [gsherman] 0.2.0devel26
Saved the qgsappbase.ui and the qgsprojectpropertiesbase.ui files (modified
at version 0.2.0devel25) using qt designer 3.1.2 to preserve backward
compatibility.

2004-05-18 [gsherman] 0.2.0devel25
Changes to implement scale display for map data in feets, meters, and
decimal degrees. A new menu item is added to the Tools menu for selecting
the map units. This setting is currently not saved with a project file.
TODO:Modify qgis.dtd and project save/load to support map units.

NOTE - the qgisapp.ui file was created with qt 3.3.x and WILL NOT WORK
with qt 3.1.2. This will be changed as soon as I can find my 3.1.2 version
of qt designer...

2004-05-18 [ts] 0.2.0devel24
Relax checking of raster filetype extensions to cater for filetypes where
extension is unpredictable (e.g. grass).
Now I use gdal to quickly check if a file is usable so pretty much anything
gdal iscompile with should get through if you have chosen wildcard
 filter in add raster dialog.

2004-05-17 [larsl] 0.2.0devel23
Added URL parsing and attribute fields for route and track GPX layers

2004-05-17 [ts] 0.2.0devel22
Added support to Save As Image to save in any QImageIO supported format.
File->SaveAsImage dialog filter list now generated automagically by
interrogating QImageIO for its supported formats. File->SaveAsImage remembers
last dir used (stored in qsettings). Its supposed to remember last filter used
but there is an issue with that that needs to be resolved.

2004-05-16 [larsl] 0.2.0devel21
Added url/link parsing to the GPX provider

2004-05-16 [larsl] 0.2.0devel20
Corrected file name extension for PNG files

2004-05-15 [larsl] 0.2.0devel19
Added my picture in the About dialog to attract more female users to QGIS

2004-05-13 [ts] 0.2.0devel18
Raster properties changes: Switched order of general tab and symbology tab - as you normally change
straight to symbology tab anyway. Removed stats tab and consolidated stats
into metadata tab. Clean ups on metadata tab.

2004-05-13 [ts] 0.2.0devel17
Raster stats tab now displays pyramid/overview info

2004-05-14 [larsl] 0.2.0devel16
Cleaned up the enabling/disabling of controls in the GPS dialog
Changed the order in which different layers are loaded from a GPX file
Added the basename of the GPX or LOC file in the layer name
Changed the plugin name to the more general "GPS Tools"

2004-05-14 [larsl] 0.2.0devel15
Fixed a bug that caused the canvas to have a fixed width of 400 with my
   Qt version - the main grid layout for the main window had one extra column

2004-05-14 [larsl] 0.2.0devel14
Added a tab for loading GPX and LOC files to the GPS plugin dialog

2004-05-14 [larsl] 0.2.0devel13
Added a virtual destructor to QgsDataProvider and deleted dataProvider in
   the destructor for QgsVectorLayer

2004-05-13 [larsl] 0.2.0devel12
Changing std::string to QString in GPSData::getData() and GPSData::releaseData() to support Qt libraries built without STL support

2004-05-13 [ts] 0.2.0devel11
Fixes for segfaults on grid_make and gps_importer dbf creation

2004-05-12 [gsherman] 0.2.0devel10
Fixes for OS X endian bug (needs further testing)

2004-05-12 [jobi] 0.2.0devel9
Added endian checks in configure
decreased versions in the auto- checks

2004-05-12 [ts] 0.2.0devel8
Added addProject(QString) to plugin interface.

2004-05-05 [jobi] 0.2.0devel7
Extended qgis-config to expose version

2004-05-04 [ts] 0.2.0devel6
Added two new internal plugins - North Arrow and Copyright message overlay

2004-05-03 [ts] 0.2.0devel5
Canvas now emits renderComplete signal when rendering of cnavas has completed,
but before the screen is refreshed. Added accessor and mutators for the canvas
pixmap.

2004-05-03 [ts] 0.2.0devel4
qgisApp->mapCanvas is now exposed through the plugin interface.

2004-05-03 [ts] 0.2.0devel3
Added three new widgets to status bar:
scale - which shows the scale in the form 1:50000 *
coordinates - shows coordinates of mouse on map in its own widget
progressbar - shows the progress of any task that emits signals connected to
showProgress slot

Added signal / slot mechanism for showExtents and set fp precision to 2 (see
below)

stringRep function in QgsRect and QgsPoint now overloaded to allow setting
floating point precision for display. QgisApp & canvas are currently hard
coding this to 2 but I plan to make this user configurable in the options
panel.

Added example of using progress indicator to raster stats gathering procedure.
You can see this in action by setting ak_shade example dataset to singleband
pseudocolor and you will see progress indicator advancing as stats are
gathered.

*NOTE: scale calculations may not be correct at this point - they are still
under development.

2004-04-27 [ts] 0.2.0devel2

Added preliminary support for building pyramids in raster files
using the GDAL overview function. Currently it is hard coded to use Nearest
neighbour algorithm with pyramids at levels  2, 4 and 8. Adding pyramids to
your raster layer can greatly improve rendering performance. This new
functionality is accessed by right clicking on a raster legen entry and
choosing 'Build Pyramids' from the popup menu.

*PLEASE USE WITH CAUTION *
This current implementation does not warn you of possible side effects
including:

- possible image degradation if too many pyramids are generated
- possuble large increase in image side
- currently this process IS NOT KNOWN TO BE REVERSIBLE so please backup data
first before experimenting.

2004-04-27 [ts] 0.2.0devel1

Rejigged single marker symbol so directory, icon selector, preview and
   scaling widget are all in one panel rather than having to spawn a new
   window to select an icon.

2004-04-27 [ts] 0.2.0devel0

Fixed broken bits in internal plugin builder template and updated default
   plugin template gui,

Version 0.2 'Pumpkin' .... development version

2004-04-25 [jobi] 0.1.0devel36
Added i18n tools to EXTRA_DIST
Updated German translation
fixed a typo -> other translations changed too

2004-04-22 [jobi] 0.1.0devel35
added install routine for svg files
added new translations
adjusted paths in the cpp files

2004-04-19 [jobi] 0.1.0devel34
Changed to simple macros for detecting QT and GDAL
Added code for detecting QGIS as m4 file to tools
this will be installed together with QT and GDAL detection to
   $prefix/share/aclocal/qgis.m4
   so the plugins can just use those simple unique macros
updated German translation
!! Developers have to link the installed qgis.m4 to /usr/share/aclocal/
!! or where ever aclocal keeps the m4 files
!! otherwise it's not detected by the plugins autogen.sh (more exactly
!! aclocal)
!! It can be cheated by adding -I path/to/qgis.m4 to the aclocal of
!! autogen.sh. But be careful not to commit that to CVS

2004-04-18 [jobi] 0.1.0devel33
Added internationalisations stuff
   Needs some documentation and more translations :-)

2004-04-17 [ts] 0.1.0devel32
Fix for crash when opening singleband grayscale images introduced by Steves
   fix for crash when opening MULTIBAND_SINGLEBAND_GRAYSCALE images. Thanks to
   Steves help, all eight raster renderers are now working properly.
   This resolves bug : [ 934234 ] Segfault when drawing multiband image band as
   grayscale

2004-04-06 [ts] 0.1.0devel31
Added new plugin (grid_maker) to build arbitrary sized graticules and add
   them to the current map view.

2004-04-05 [jobi] 0.1.0devel30
fixed qgiscommit (didn't work when in qgis root)
cosmetics for qgis-config to be more "standardconform"

2004-04-04 [jobi] 0.1.0devel29
fixed GRASS provider

2004-04-03 [ts] 0.1.0devel28
Bug fix (still unconfirmed if it cures the bug!) for raster layer rendering
artifacts.

Added new color ramper for grayscale and pseudcolor grayscale image called
freak out (its a bit psycadellic at the moment). Last class break ne
eds some work!

2004-04-02 [jobi] 0.1.0devel27
Added version checks for autoconf, automake and libtool
Small bugfixes

2004-04-02 [mcoletti] 0.1.0devel26
Rolling forward QgsFeature::setGeometry() interface change whereby the size of
the well known type geometry binary buffer is also now passed in.

2004-04-02 [mcoletti] 0.1.0devel25
Compensating for QgsFeature::setGeometry() now accepting "size" parameter for
the given binary geometry string.

QgsShapeFileProvider::endian() now uses shorter, standard way of computing
endian-ness.

2004-04-02 [stevehalasz] 0.1.0devel25

2004-04-01 [jobi] 0.1.0devel24
changed qgiscommit to hopefully fix all problems

2004-04-01 [jobi] 0.1.0devel23
Extended tools/qgiscommit to pass parameters to cvs

2004-04-01 [jobi] 0.1.0devel22
Fixed GRASS plugin and provider build

2004-04-01 [jobi] 0.1.0devel21
fixed strange warning: object 'foo.$(OBJEXT)' created both
   with libtool and without
also cleaned the other Makefiles that way

2004-03-31 [jobi] 0.1.0devel20
fixed small bug
renamed plugins/gps_importer/shapefil.h to shapefile.h

2004-03-31 [jobi] 0.1.0devel19
A lot of small changes to make release work again
Probably more cleaning in the Makefiles needed

2004-03-27 [ts] 0.1.0devel18
Fix for cl parameter "snapshot" to ensure events are process (ie canvas is
drawn!) before snapshot is taken.

2004-03-27 [jobi] 0.1.0devel17
autogen.sh now passes parameters to configure
fixed tools/qgiscommit by using mktemp, thanx mcoletti
pluginpath is now taken of libdir to be 64bit compliant (e.g. /usr/lib64/qgis)

2004-03-26 [jobi] 0.1.0devel13
forgot to remove tempfile

2004-03-26 [jobi] 0.1.0devel12
Removed Newline after statusline
Should work now fine!
Have fun

2004-03-26 [jobi] 0.1.0devel11

Added qgiscommit tool

2004-03-26 [didge] 0.1.0devel10

Fixed bug #920070
Made plugin-libdir 64bit compatible (e.g. /usr/lib64/qgis)
   for AMD64 and PPC64 systems

2004-03-22 [mac] 0.1.0devel9

Added gps_importer plugin (still a work in progress)

2004-03-22 [mac] 0.1.0devel8
s/config.h/qgsconfig.h/
qgsconfig.h now has header sentinels
now will install headers in $(prefix)/qgis/include and libqis.- library in
  $(prefix)/lib
"src/Makefile" no longer relies on explicit dependencies and uses better
  naming scheme for created source files

2004-03-21 [ts] 0.1.0devel7

Added thumbnail preview of raster to raster props dialog.
   Added drawThumbnail method to rasterlayer.cpp
   Split (overloaded) draw method in rasterlayer.cpp so that some parts of
   the original draw method could be used by drawThumbnail method too.

Fixed a bug in the drawing of pseudocolor single band grayscale images that
   prevented all class breaks being displayed.

2004-03-10 [gs] 0.1.0devel7
Added delimited text plugin which provides gui to add delimited text
   layers using the delimited_text data provider
Changes to delimited_text data provider to support zooming, display
   of attributes, and identifying features. Selecting features does
   not work at this time.
Auto- changes to support building the delimited text provider and
   plugin
Minor changes to QgsFeature

2004-03-06 [ts] 0.1.0devel6
Completed session management of plugins (so active plugins are remembered
   when qgis closes and reloaded in the next session).

2004-03-06 [ts] 0.1.0devel6
Save state of plugins in ~/.qt/qtrc file (in progress). State is saved,
   just need to implement code to load plugins marked as active during
   application startup.

2004-03-06 [ts] 0.1.0devel6
Added QgsRasterLayer::filterLayer which gets called near the end of each of
   the 8 renderers. This is the place to inline filters. Note that eventually filters
   will be hived out to a filter plugin mechanism.

2004-03-06 [didge] 0.1.0devel6
Changed configure stuff to write DEFINES in config.h.
   PostgreSQL stuff needs testing as I commented the compileflags in src/Makefile.am
Infos will be posted on the devel mailinglist

2004-03-04 [ts] 0.1.0devel5
Added option to options dialog to disable splashscreen.

2004-02-28 [ts] 0.1.0devel5

- -snapshot command line parameter working now and correctly scaling
snapshot to size of pixmap.
started moving splashscreen to a global so other classes diring  the startup
process can get access to set the splash status. (in progress)

2004-02-28 [gs] 0.1.0devel5
Refactored QgsField to use new coding conventions
Documented QgsField (added docs to qgsfield.h)
Updated doxygen mainpage section in qgis.h
Added What's this? help to main application window
Added providers/delimitedtext and associated source files to CVS

2004-02-27 [gs] 0.1.0devel4
Corrected debug statements in main.cpp and added a bit of verbiage to the
help text.
Removed hard coding of provider types in the QgisApp::addVectorLayer method.
Caller must now provide compatible arguments that the designated provider can
use to open the data store and fetch data. Changed QgsPgGeoprocessing class
to properly call addVectorLayer.

2004-02-27 [ts]
Changed cl parser to getopt
Moved project loading out of loop that loads layers - you now need to specify
--project filename to load a filename. This ensures that you only try to load
one project file.
Added --snapshot filename parameter that will load layers and project files
specified, take a screenie of the map view and save it to disk as filename -
this is under construction still.
Added saveMapAsImage(QString) to qgisapp so above cl option can be used.

2004-02-26 [ts]
Added a tab to raster layer properties dialog to show metadata about the
raster layer (using gdal metadata)

2004-02-26 [gs] 0.1.0devel3
Added Version to configure.in. QGIS now displays its version number based on
the settings in configure.in

2004-02-24 [gs]
Search radius for identifying features on vector layers added to Preferences

2004-02-23 [ts]
Save current view to disk as a PNG image

# Version 0.1 'Moroz' February 25, 2004
User interface improvements - menu and dialog cleanups and a new icon theme
  based on Everaldo's Crystal icon set.
QGIS can load layers and / or a project on start up by specifying these
  on the command line.
Symbol renderers for simple, graduated, and continuous symbols
Raster support for most GDAL formats
Raster implementation supports a variety of rendering settings including
  semi transparent overlays, palette inversion, flexible band to color mapping
  in multiband images and creation of pseudocolor.
Change to a data provider architecture for vector layers. Additional data
  types can be supported by writing a provider plugin
Buffer plugin for PostGIS layers
PostgreSQL port number can be specified when making connections
Shapefile to PostGIS Import Tool (SPIT) plugin for importing shapefiles into
  PostgreSQL/PostGIS
User guide (HTML and PDF)
Install guide (HTML and PDF)
Plugin manager to manage loading/unloading of plugins
Plugin template to automate the more mundane parts of creating a new plugin.
Numerous bug fixes
Removed dependency on libpq++ when compiling with PostgreSQL/PostGIS support
PostgreSQL/PostGIS layers now rely on GEOS for selecting features

# Version 0.0.13 December 8, 2003
New build system (uses GNU Autoconf)
Improvement to sorting in attribute table
Persistent selections (shapefiles only)
Display order can be changed by dragging a layer to a new position in the legend
Export QGIS view as a Mapserver map file
Fix for crash on SuSE 9.0 when moving mouse in legend area

# Version 0.0.12-alpha June 10, 2003
Multiple features displayed with the Identify tool
Identify function returns and displays attributes for multiple
   features found within the search radius
Fixes to endian handling on big endian machines
Support for PostgreSQL 7.3 schemas for database layers
Features in shapefiles can be selected by dragging a selection
  box or selecting the records in the attribute table
Zoom to extent of selected features (Shapefiles only)
Bug fix: Bug that prevented reopening of the attribute table once
  it was initially displayed and closed
Bug fix: Bug that prevented lines from being drawn with widths
  other than 1 pixel
Build system has changed for building with PostgreSQL support.

# Version 0.0.11-alpha June 10, 2003
Preliminary Plugin Manager implementation
Version check under tools menu
Version checking uses port 80 to prevent problems
   with firewalls
Fix for PostGIS bug when srid != -1
Fix for PostGIS LINESTRING rendering
Database Connections can now be deleted
Fixes to Database Connection dialog
Fix for crash when opening a shapefile attribute table
   twice in succession
Fix for crash when opening invalid shapefiles

# Version 0.0.10-alpha May 13, 2003
*Fixes to project save/open support
*Enhancements to plugin tests
*Fixes to build system (gdal link problem)

# Version 0.0.9-alpha January 25, 2003
*Preliminary project save/open support
*Streamlined build system

# Version 0.0.8-alpha December 11, 2002
*During repaint, the data store is only accessed if map state or extent
 has changed
*Changes to layer properites aren't effective until the Layer Properties
 dialog is closed
*Canceling the Layer Properties dialog cancels changes

# Version 0.0.7-alpha November 30, 2002
*Changes to the build system to allow building with/without PostgeSQL
 support

# Version 0.0.6a-alpha November 27, 2002
*Fix to build problems introduced at 0.0.6. No new features are included
 in this release.

# Version 0.0.6-alpha November 24, 2002
*Improved handling/management of PostGIS connections
*Password prompt if the password is not stored with a connection
*Windows size and position and toolbar docking state is saved/restored
*Identify function for layers
*Attribute table for a layer can be displayed and sorted by clicking on column headers
*Duplicate layers (layers with same name) are now handled properly

# Version 0.0.5-alpha October 5, 2002
- Removing a layer from the map no longer crashes the application
- Fixed multiple render bug when adding a layer
- Data source is shown in Layer Properties dialog
- Display name of a layer can be changed using the Layer Properties dialog
- Line widths can be set for a layer using the Layer Properties dialog
- Zoom out now works
- Zoom Previous added to toolbar
- Toolbar has been rearranged and new icons added
- Help|About QGis now contains Version, What's New, and License information

# Version 0.0.4-alpha August 15, 2002
- Added Layer Properties dialog
- User can set color for layers
- Added right-click menu to the layer list in legend
- Layers can be removed using the right-click menu (buggy)
- Moved the KDevelop project file qgis.kdevprj to the src sub-directory
- Fixed multiple repaint bug that occurred when more than one layer was
   added at a time
- Fixed bug that caused a full refresh at the beginning of a pan operation

# Version 0.0.3-alpha August 10, 2002
 - Support for shapefiles and other vector formats
 - Improved handling of extents when adding layers
 - Primitive legend that allows control of layer visibility
 - About Quantum GIS implemented
 - Other internal changes

July 26, 2002
  Drawing code now properly displays layers and calculates extents when
  zooming. Zoom is still fixed zoom in rather than interactive.

July 20, 2002
  Repaint automatic for layers.

July 18, 2002
  Point, line and polygon PostGis layers can be drawn. Still issues with
  map extent and positioning of layers on the canvas. Drawing is manual and
  not tied to the paint event. No zooming or panning yet.

July 10, 2002
  Layers can be selected and added to the map canvas collection however
  the rendering code is currently disabled and being reorganized. So if
  you add a layer, nothing will be drawn...

July 6, 2002
  This code is preliminary and really has no true functionality other than
  the ability to define a PostGIS connection and display the spatially enabled
  tables that could be loaded.

  This is the initial import of the code base into CVS on Sourceforge.net.
