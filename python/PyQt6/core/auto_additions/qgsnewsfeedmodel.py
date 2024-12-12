# The following has been generated automatically from src/core/network/qgsnewsfeedmodel.h
QgsNewsFeedModel.Role = QgsNewsFeedModel.CustomRole
# monkey patching scoped based enum
QgsNewsFeedModel.Key = QgsNewsFeedModel.CustomRole.Key
QgsNewsFeedModel.Key.is_monkey_patched = True
QgsNewsFeedModel.Key.__doc__ = "Entry unique key"
QgsNewsFeedModel.Title = QgsNewsFeedModel.CustomRole.Title
QgsNewsFeedModel.Title.is_monkey_patched = True
QgsNewsFeedModel.Title.__doc__ = "Entry title"
QgsNewsFeedModel.Content = QgsNewsFeedModel.CustomRole.Content
QgsNewsFeedModel.Content.is_monkey_patched = True
QgsNewsFeedModel.Content.__doc__ = "Entry content"
QgsNewsFeedModel.ImageUrl = QgsNewsFeedModel.CustomRole.ImageUrl
QgsNewsFeedModel.ImageUrl.is_monkey_patched = True
QgsNewsFeedModel.ImageUrl.__doc__ = "Optional entry image URL"
QgsNewsFeedModel.Image = QgsNewsFeedModel.CustomRole.Image
QgsNewsFeedModel.Image.is_monkey_patched = True
QgsNewsFeedModel.Image.__doc__ = "Optional entry image"
QgsNewsFeedModel.Link = QgsNewsFeedModel.CustomRole.Link
QgsNewsFeedModel.Link.is_monkey_patched = True
QgsNewsFeedModel.Link.__doc__ = "Optional entry URL link"
QgsNewsFeedModel.Sticky = QgsNewsFeedModel.CustomRole.Sticky
QgsNewsFeedModel.Sticky.is_monkey_patched = True
QgsNewsFeedModel.Sticky.__doc__ = "Whether entry is sticky"
QgsNewsFeedModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsNewsFeedModel.Role

.. versionadded:: 3.36

* ``Key``: Entry unique key
* ``Title``: Entry title
* ``Content``: Entry content
* ``ImageUrl``: Optional entry image URL
* ``Image``: Optional entry image
* ``Link``: Optional entry URL link
* ``Sticky``: Whether entry is sticky

"""
# --
QgsNewsFeedModel.CustomRole.baseClass = QgsNewsFeedModel
try:
    QgsNewsFeedModel.__group__ = ['network']
except (NameError, AttributeError):
    pass
try:
    QgsNewsFeedProxyModel.__group__ = ['network']
except (NameError, AttributeError):
    pass
