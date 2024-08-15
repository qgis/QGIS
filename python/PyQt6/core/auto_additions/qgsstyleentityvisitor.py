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
QgsStyleEntityVisitorInterface.NodeType.__doc__ = "Describes the types of nodes which may be visited by the visitor.\n\n" + '* ``Project``: ' + QgsStyleEntityVisitorInterface.NodeType.Project.__doc__ + '\n' + '* ``Layer``: ' + QgsStyleEntityVisitorInterface.NodeType.Layer.__doc__ + '\n' + '* ``SymbolRule``: ' + QgsStyleEntityVisitorInterface.NodeType.SymbolRule.__doc__ + '\n' + '* ``Layouts``: ' + QgsStyleEntityVisitorInterface.NodeType.Layouts.__doc__ + '\n' + '* ``PrintLayout``: ' + QgsStyleEntityVisitorInterface.NodeType.PrintLayout.__doc__ + '\n' + '* ``LayoutItem``: ' + QgsStyleEntityVisitorInterface.NodeType.LayoutItem.__doc__ + '\n' + '* ``Report``: ' + QgsStyleEntityVisitorInterface.NodeType.Report.__doc__ + '\n' + '* ``ReportHeader``: ' + QgsStyleEntityVisitorInterface.NodeType.ReportHeader.__doc__ + '\n' + '* ``ReportFooter``: ' + QgsStyleEntityVisitorInterface.NodeType.ReportFooter.__doc__ + '\n' + '* ``ReportSection``: ' + QgsStyleEntityVisitorInterface.NodeType.ReportSection.__doc__ + '\n' + '* ``Annotations``: ' + QgsStyleEntityVisitorInterface.NodeType.Annotations.__doc__ + '\n' + '* ``Annotation``: ' + QgsStyleEntityVisitorInterface.NodeType.Annotation.__doc__
# --
try:
    QgsStyleEntityVisitorInterface.__attribute_docs__ = {'identifier': 'A string identifying the node. The actual value of ``identifier`` will vary\ndepending on the node being visited. E.g for a rule based renderer, the\nidentifier will be a rule ID. For a project, node identifiers will be\nlayer IDs.', 'description': 'A string describing the node. The actual value of ``description`` will vary\ndepending on the node being visited. E.g for a rule based renderer, the\nidentifier will be a rule label. For a project, node identifiers will be\nlayer names.', 'entity': 'Reference to style entity being visited.', 'type': 'Node type.'}
except NameError:
    pass
QgsStyleEntityVisitorInterface.StyleLeaf.__doc__ = """Contains information relating to the style entity currently being visited."""
QgsStyleEntityVisitorInterface.Node.__doc__ = """Contains information relating to a node (i.e. a group of symbols or other nodes)
being visited."""
try:
    QgsStyleEntityVisitorInterface.__group__ = ['symbology']
except NameError:
    pass
try:
    QgsStyleEntityVisitorInterface.StyleLeaf.__group__ = ['symbology']
except NameError:
    pass
try:
    QgsStyleEntityVisitorInterface.Node.__group__ = ['symbology']
except NameError:
    pass
