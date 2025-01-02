# The following has been generated automatically from src/core/symbology/qgsstyleentityvisitor.h
# monkey patching scoped based enum
QgsStyleEntityVisitorInterface.NodeType.Project.__doc__ = "QGIS Project node"
QgsStyleEntityVisitorInterface.NodeType.Layer.__doc__ = "Map layer"
QgsStyleEntityVisitorInterface.NodeType.SymbolRule.__doc__ = "Rule based symbology or label child rule"
QgsStyleEntityVisitorInterface.NodeType.Layouts.__doc__ = "Layout collection"
QgsStyleEntityVisitorInterface.NodeType.PrintLayout.__doc__ = "An individual print layout"
QgsStyleEntityVisitorInterface.NodeType.LayoutItem.__doc__ = "Individual item in a print layout"
QgsStyleEntityVisitorInterface.NodeType.Report.__doc__ = "A QGIS print report"
QgsStyleEntityVisitorInterface.NodeType.ReportHeader.__doc__ = "Report header section"
QgsStyleEntityVisitorInterface.NodeType.ReportFooter.__doc__ = "Report footer section"
QgsStyleEntityVisitorInterface.NodeType.ReportSection.__doc__ = "Report sub section"
QgsStyleEntityVisitorInterface.NodeType.Annotations.__doc__ = "Annotations collection"
QgsStyleEntityVisitorInterface.NodeType.Annotation.__doc__ = "An individual annotation"
QgsStyleEntityVisitorInterface.NodeType.__doc__ = """Describes the types of nodes which may be visited by the visitor.

* ``Project``: QGIS Project node
* ``Layer``: Map layer
* ``SymbolRule``: Rule based symbology or label child rule
* ``Layouts``: Layout collection
* ``PrintLayout``: An individual print layout
* ``LayoutItem``: Individual item in a print layout
* ``Report``: A QGIS print report
* ``ReportHeader``: Report header section
* ``ReportFooter``: Report footer section
* ``ReportSection``: Report sub section
* ``Annotations``: Annotations collection
* ``Annotation``: An individual annotation

"""
# --
try:
    QgsStyleEntityVisitorInterface.StyleLeaf.__attribute_docs__ = {'identifier': 'A string identifying the style entity. The actual value of ``identifier`` will vary\ndepending on the class being visited. E.g for a categorized renderer, the\nidentifier will be the category ID associated with the symbol.\n\nThis may be blank if no identifier is required, e.g. when a renderer has a single\nsymbol only.\n\nNote that in some cases where a specific identifier is not available, a generic, untranslated\none may be used (e.g. "overview", "grid").', 'description': 'A string describing the style entity. The actual value of ``description`` will vary\ndepending on the class being visited. E.g for a categorized renderer, the\ndescription will be the category label associated with the symbol, for a print layout, it will\nbe the name of the layout in the project.\n\nThis may be blank if no description is associated with a style entity, e.g. when a renderer has a single\nsymbol only.\n\nThis value may be a generic, translated value in some cases, e.g. "Grid" or "Overview".', 'entity': 'Reference to style entity being visited.'}
    QgsStyleEntityVisitorInterface.StyleLeaf.__doc__ = """Contains information relating to the style entity currently being visited."""
    QgsStyleEntityVisitorInterface.StyleLeaf.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleEntityVisitorInterface.Node.__attribute_docs__ = {'type': 'Node type.', 'identifier': 'A string identifying the node. The actual value of ``identifier`` will vary\ndepending on the node being visited. E.g for a rule based renderer, the\nidentifier will be a rule ID. For a project, node identifiers will be\nlayer IDs.', 'description': 'A string describing the node. The actual value of ``description`` will vary\ndepending on the node being visited. E.g for a rule based renderer, the\nidentifier will be a rule label. For a project, node identifiers will be\nlayer names.'}
    QgsStyleEntityVisitorInterface.Node.__doc__ = """Contains information relating to a node (i.e. a group of symbols or other nodes)
being visited."""
    QgsStyleEntityVisitorInterface.Node.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleEntityVisitorInterface.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
