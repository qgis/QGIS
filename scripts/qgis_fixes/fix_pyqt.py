"""Migrate imports of PyQt4 to PyQt wrapper
"""
# Author: Juergen E. Fischer
# Adapted from fix_urllib

# Local imports
from lib2to3.fixes.fix_imports import alternates, FixImports
from lib2to3 import fixer_base
from lib2to3.fixer_util import (Name, Comma, FromImport, Newline,
                                find_indentation, Node, syms)

MAPPING = {
    "PyQt4.QtGui": [
        ("qgis.PyQt.QtGui", [
         "QIcon",
         "QCursor",
         "QColor",
         "QDesktopServices",
         "QFont",
         "QFontMetrics",
         "QKeySequence",
         "QStandardItemModel",
         "QStandardItem",
         "QClipboard",
         "QPixmap",
         "QDoubleValidator",
         "QPainter",
         "QPen",
         "QBrush",
         "QPalette",
         "QPainterPath",
         "QImage",
         "QPolygonF",
         "QFontMetricsF",
         "QGradient",
         ]),
        ("qgis.PyQt.QtWidgets", [
         "QAbstractButton",
         "QAbstractGraphicsShapeItem",
         "QAbstractItemDelegate",
         "QAbstractItemView",
         "QAbstractScrollArea",
         "QAbstractSlider",
         "QAbstractSpinBox",
         "QAbstractTableModel",
         "QAction",
         "QActionGroup",
         "QApplication",
         "QBoxLayout",
         "QButtonGroup",
         "QCalendarWidget",
         "QCheckBox",
         "QColorDialog",
         "QColumnView",
         "QComboBox",
         "QCommandLinkButton",
         "QCommonStyle",
         "QCompleter",
         "QDataWidgetMapper",
         "QDateEdit",
         "QDateTimeEdit",
         "QDesktopWidget",
         "QDial",
         "QDialog",
         "QDialogButtonBox",
         "QDirModel",
         "QDockWidget",
         "QDoubleSpinBox",
         "QErrorMessage",
         "QFileDialog",
         "QFileIconProvider",
         "QFileSystemModel",
         "QFocusFrame",
         "QFontComboBox",
         "QFontDialog",
         "QFormLayout",
         "QFrame",
         "QGesture",
         "QGestureEvent",
         "QGestureRecognizer",
         "QGraphicsAnchor",
         "QGraphicsAnchorLayout",
         "QGraphicsBlurEffect",
         "QGraphicsColorizeEffect",
         "QGraphicsDropShadowEffect",
         "QGraphicsEffect",
         "QGraphicsEllipseItem",
         "QGraphicsGridLayout",
         "QGraphicsItem",
         "QGraphicsItemGroup",
         "QGraphicsLayout",
         "QGraphicsLayoutItem",
         "QGraphicsLineItem",
         "QGraphicsLinearLayout",
         "QGraphicsObject",
         "QGraphicsOpacityEffect",
         "QGraphicsPathItem",
         "QGraphicsPixmapItem",
         "QGraphicsPolygonItem",
         "QGraphicsProxyWidget",
         "QGraphicsRectItem",
         "QGraphicsRotation",
         "QGraphicsScale",
         "QGraphicsScene",
         "QGraphicsSceneContextMenuEvent",
         "QGraphicsSceneDragDropEvent",
         "QGraphicsSceneEvent",
         "QGraphicsSceneHelpEvent",
         "QGraphicsSceneHoverEvent",
         "QGraphicsSceneMouseEvent",
         "QGraphicsSceneMoveEvent",
         "QGraphicsSceneResizeEvent",
         "QGraphicsSceneWheelEvent",
         "QGraphicsSimpleTextItem",
         "QGraphicsTextItem",
         "QGraphicsTransform",
         "QGraphicsView",
         "QGraphicsWidget",
         "QGridLayout",
         "QGroupBox",
         "QHBoxLayout",
         "QHeaderView",
         "QInputDialog",
         "QItemDelegate",
         "QItemEditorCreatorBase",
         "QItemEditorFactory",
         "QKeyEventTransition",
         "QLCDNumber",
         "QLabel",
         "QLayout",
         "QLayoutItem",
         "QLineEdit",
         "QListView",
         "QListWidget",
         "QListWidgetItem",
         "QMainWindow",
         "QMdiArea",
         "QMdiSubWindow",
         "QMenu",
         "QMenuBar",
         "QMessageBox",
         "QMouseEventTransition",
         "QPanGesture",
         "QPinchGesture",
         "QPlainTextDocumentLayout",
         "QPlainTextEdit",
         "QProgressBar",
         "QProgressDialog",
         "QPushButton",
         "QRadioButton",
         "QRubberBand",
         "QScrollArea",
         "QScrollBar",
         "QShortcut",
         "QSizeGrip",
         "QSizePolicy",
         "QSlider",
         "QSpacerItem",
         "QSpinBox",
         "QSplashScreen",
         "QSplitter",
         "QSplitterHandle",
         "QStackedLayout",
         "QStackedWidget",
         "QStatusBar",
         "QStyle",
         "QStyleFactory",
         "QStyleHintReturn",
         "QStyleHintReturnMask",
         "QStyleHintReturnVariant",
         "QStyleOption",
         "QStyleOptionButton",
         "QStyleOptionComboBox",
         "QStyleOptionComplex",
         "QStyleOptionDockWidget",
         "QStyleOptionFocusRect",
         "QStyleOptionFrame",
         "QStyleOptionGraphicsItem",
         "QStyleOptionGroupBox",
         "QStyleOptionHeader",
         "QStyleOptionMenuItem",
         "QStyleOptionProgressBar",
         "QStyleOptionRubberBand",
         "QStyleOptionSizeGrip",
         "QStyleOptionSlider",
         "QStyleOptionSpinBox",
         "QStyleOptionTab",
         "QStyleOptionTabBarBase",
         "QStyleOptionTabWidgetFrame",
         "QStyleOptionTitleBar",
         "QStyleOptionToolBar",
         "QStyleOptionToolBox",
         "QStyleOptionToolButton",
         "QStyleOptionViewItem",
         "QStylePainter",
         "QStyledItemDelegate",
         "QSwipeGesture",
         "QSystemTrayIcon",
         "QTabBar",
         "QTabWidget",
         "QTableView",
         "QTableWidget",
         "QTableWidgetItem",
         "QTableWidgetSelectionRange",
         "QTapAndHoldGesture",
         "QTapGesture",
         "QTextBrowser",
         "QTextEdit",
         "QTimeEdit",
         "QToolBar",
         "QToolBox",
         "QToolButton",
         "QToolTip",
         "QTreeView",
         "QTreeWidget",
         "QTreeWidgetItem",
         "QTreeWidgetItemIterator",
         "QUndoCommand",
         "QUndoGroup",
         "QUndoStack",
         "QUndoView",
         "QVBoxLayout",
         "QWhatsThis",
         "QWidget",
         "QWidgetAction",
         "QWidgetItem",
         "QWizard",
         "QWizardPage",
         "qApp",
         "qDrawBorderPixmap",
         "qDrawPlainRect",
         "qDrawShadeLine",
         "qDrawShadePanel",
         "qDrawShadeRect",
         "qDrawWinButton",
         "qDrawWinPanel",
         ]),
        ("qgis.PyQt.QtPrintSupport", [
            "QPrinter",
            "QAbstractPrintDialog",
            "QPageSetupDialog",
            "QPrintDialog",
            "QPrintEngine",
            "QPrintPreviewDialog",
            "QPrintPreviewWidget",
            "QPrinterInfo",
        ]),
        ("qgis.PyQt.QtCore", [
         "QItemSelectionModel",
         "QSortFilterProxyModel",
         ]),
    ],
    "PyQt4.QtCore": [
        ("qgis.PyQt.QtCore", [
            "QAbstractItemModel",
            "QAbstractTableModel",
            "QByteArray",
            "QCoreApplication",
            "QDataStream",
            "QDir",
            "QEvent",
            "QFile",
            "QFileInfo",
            "QIODevice",
            "QLocale",
            "QMimeData",
            "QModelIndex",
            "QMutex",
            "QObject",
            "QProcess",
            "QSettings",
            "QSize",
            "QSizeF",
            "QTextCodec",
            "QThread",
            "QThreadPool",
            "QTimer",
            "QTranslator",
            "QUrl",
            "Qt",
            "pyqtProperty",
            "pyqtWrapperType",
            "pyqtSignal",
            "pyqtSlot",
            "qDebug",
            "qWarning",
            "QDate",
            "QTime",
            "QDateTime",
            "QRegExp",
            "QTemporaryFile",
            "QTextStream",
            "QVariant",
            "QPyNullVariant",
            "QRect",
            "QRectF",
            "QMetaObject",
            "QPoint",
            "QPointF",
            "QDirIterator",
            "NULL",
        ]),
        (None, [
            "SIGNAL",
            "SLOT",
        ]),
    ],
    "PyQt4.QtNetwork": [
        ("qgis.PyQt.QtNetwork", ["QNetworkReply", "QNetworkRequest"])
    ],
    "PyQt4.QtXml": [
        ("qgis.PyQt.QtXml", [
            "QDomDocument"
        ]),
    ],
    "PyQt4.Qsci": [
        ("qgis.PyQt.Qsci", [
            "QsciAPIs",
            "QsciLexerCustom",
            "QsciLexerPython",
            "QsciScintilla",
            "QsciLexerSQL",
            "QsciStyle",
        ]),
    ],
    "PyQt4.QtWebKit": [
        ("qgis.PyQt.QtWebKitWidgets", [
            "QGraphicsWebView",
            "QWebFrame",
            "QWebHitTestResult",
            "QWebInspector",
            "QWebPage",
            "QWebView",
        ]),
    ],
    "PyQt4.QtTest": [
        ("qgis.PyQt.QtTest", [
            "QTest",
        ]),
    ],
    "PyQt4.QtSvg": [
        ("qgis.PyQt.QtSvg", [
            "QSvgRenderer",
            "QSvgGenerator"
        ]),
    ],
    "PyQt4.QtSql": [
        ("qgis.PyQt.QtSql", [
            "QSqlDatabase",
            "QSqlQuery",
            "QSqlField"
        ]),
    ],
    "PyQt4.uic": [
        ("qgis.PyQt.uic", [
            "loadUiType",
            "loadUi",
        ]),
    ],
    "PyQt4": [
        ("PyQt", [
            "QtCore",
            "QtGui",
            "QtNetwork",
            "QtXml",
            "QtWebkit",
            "QtSql",
            "QtSvg",
            "Qsci",
            "uic",
        ])
    ],
}


def build_pattern():
    bare = set()
    for old_module, changes in list(MAPPING.items()):
        for change in changes:
            new_module, members = change
            members = alternates(members)

            if '.' not in old_module:
                from_name = "%r" % old_module
            else:
                dotted = old_module.split('.')
                assert len(dotted) == 2
                from_name = "dotted_name<%r '.' %r>" % (dotted[0], dotted[1])

            yield """import_name< 'import' (module=%s
                                  | dotted_as_names< any* module=%s any* >) >
                  """ % (from_name, from_name)
            yield """import_from< 'from' mod_member=%s 'import'
                       ( member=%s | import_as_name< member=%s 'as' any > |
                         import_as_names< members=any*  >) >
                  """ % (from_name, members, members)
            yield """import_from< 'from' mod_member=%s 'import' '('
                       ( member=%s | import_as_name< member=%s 'as' any > |
                         import_as_names< members=any*  >) ')' >
                  """ % (from_name, members, members)
            yield """import_from< 'from' module_star=%s 'import' star='*' >
                  """ % from_name
            yield """import_name< 'import'
                                  dotted_as_name< module_as=%s 'as' any > >
                  """ % from_name
            # bare_with_attr has a special significance for FixImports.match().
            yield """power< bare_with_attr=%s trailer< '.' member=%s > any* >
                  """ % (from_name, members)


class FixPyqt(FixImports):

    def build_pattern(self):
        return "|".join(build_pattern())

#    def match(self, node):
#        res = super(FixImports, self).match( node )
#        print repr(node)
#        return res

    def transform_import(self, node, results):
        """Transform for the basic import case. Replaces the old
           import name with a comma separated list of its
           replacements.
        """
        import_mod = results.get("module")
        pref = import_mod.prefix

        names = []

        # create a Node list of the replacement modules
        for name in MAPPING[import_mod.value][:-1]:
            names.extend([Name(name[0], prefix=pref), Comma()])
        names.append(Name(MAPPING[import_mod.value][-1][0], prefix=pref))
        import_mod.replace(names)

    def transform_member(self, node, results):
        """Transform for imports of specific module elements. Replaces
           the module to be imported from with the appropriate new
           module.
        """
        mod_member = results.get("mod_member")
        if isinstance(mod_member, Node):
            module = ""
            for l in mod_member.leaves():
                module += l.value
            mod_member.value = module
        pref = mod_member.prefix
        member = results.get("member")

        missing = False

        # Simple case with only a single member being imported
        if member:
            # this may be a list of length one, or just a node
            if isinstance(member, list):
                member = member[0]
            new_name = ''
            for change in MAPPING[mod_member.value]:
                if member.value in change[1]:
                    new_name = change[0]
                    break
            if new_name:
                mod_member.replace(Name(new_name, prefix=pref))
            elif new_name == '':
                self.cannot_convert(node, "This is an invalid module element")
            else:
                node.remove()

        # Multiple members being imported
        else:
            # a dictionary for replacements, order matters
            modules = []
            mod_dict = {}
            members = results["members"]
            for member in members:
                # we only care about the actual members
                if member.type == syms.import_as_name:
                    as_name = member.children[2].value
                    member_name = member.children[0].value
                else:
                    member_name = member.value
                    as_name = None
                if member_name != u",":
                    found = False
                    for change in MAPPING[mod_member.value]:
                        if member_name in change[1]:
                            if change[0] is not None:
                                if change[0] not in mod_dict:
                                    modules.append(change[0])
                                mod_dict.setdefault(change[0], []).append(member)
                            found = True
                    if not found:
                        f = open("/tmp/missing", "a+")
                        f.write("member %s of %s not found\n" % (member_name, mod_member.value))
                        f.close()
                        missing = True

            new_nodes = []
            indentation = find_indentation(node)
            first = True

            def handle_name(name, prefix):
                if name.type == syms.import_as_name:
                    kids = [Name(name.children[0].value, prefix=prefix),
                            name.children[1].clone(),
                            name.children[2].clone()]
                    return [Node(syms.import_as_name, kids)]
                return [Name(name.value, prefix=prefix)]

            for module in modules:
                elts = mod_dict[module]
                names = []
                for elt in elts[:-1]:
                    names.extend(handle_name(elt, pref))
                    names.append(Comma())
                names.extend(handle_name(elts[-1], pref))
                new = FromImport(module, names)
                if not first or node.parent.prefix.endswith(indentation):
                    new.prefix = indentation
                new_nodes.append(new)
                first = False

            if new_nodes:
                nodes = []
                for new_node in new_nodes[:-1]:
                    nodes.extend([new_node, Newline()])
                nodes.append(new_nodes[-1])

                if node.prefix:
                    nodes[0].prefix = node.prefix

                node.replace(nodes)
            elif missing:
                self.cannot_convert(node, "All module elements are invalid")
            else:
                node.remove()

    def transform_dot(self, node, results):
        """Transform for calls to module members in code."""
        module_dot = results.get("bare_with_attr")
        member = results.get("member")
        new_name = None
        if isinstance(member, list):
            member = member[0]
        for change in MAPPING[module_dot.value]:
            if member.value in change[1]:
                new_name = change[0]
                break
        if new_name:
            module_dot.replace(Name(new_name,
                                    prefix=module_dot.prefix))
        else:
            self.cannot_convert(node, "This is an invalid module element")

    def transform(self, node, results):
        if results.get("module"):
            self.transform_import(node, results)
        elif results.get("mod_member"):
            self.transform_member(node, results)
        elif results.get("bare_with_attr"):
            self.transform_dot(node, results)
        # Renaming and star imports are not supported for these modules.
        elif results.get("module_star"):
            self.cannot_convert(node, "Cannot handle star imports.")
        elif results.get("module_as"):
            self.cannot_convert(node, "This module is now multiple modules")
