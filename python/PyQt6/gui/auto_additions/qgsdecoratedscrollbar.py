# The following has been generated automatically from src/gui/qgsdecoratedscrollbar.h
# monkey patching scoped based enum
QgsScrollBarHighlight.Priority.Invalid.__doc__ = "Invalid"
QgsScrollBarHighlight.Priority.LowPriority.__doc__ = "Low priority, rendered below all other highlights"
QgsScrollBarHighlight.Priority.NormalPriority.__doc__ = "Normal priority"
QgsScrollBarHighlight.Priority.HighPriority.__doc__ = "High priority"
QgsScrollBarHighlight.Priority.HighestPriority.__doc__ = "Highest priority, rendered above all other highlights"
QgsScrollBarHighlight.Priority.__doc__ = """Priority, which dictates how overlapping highlights are rendered

* ``Invalid``: Invalid
* ``LowPriority``: Low priority, rendered below all other highlights
* ``NormalPriority``: Normal priority
* ``HighPriority``: High priority
* ``HighestPriority``: Highest priority, rendered above all other highlights

"""
# --
try:
    QgsScrollBarHighlight.__attribute_docs__ = {'category': 'Category ID', 'position': 'Position in scroll bar', 'color': 'Highlight color', 'priority': 'Priority, which dictates how overlapping highlights are rendered'}
except (NameError, AttributeError):
    pass
