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
QgsNewsFeedModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsNewsFeedModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``Key``: ' + QgsNewsFeedModel.CustomRole.Key.__doc__ + '\n' + '* ``Title``: ' + QgsNewsFeedModel.CustomRole.Title.__doc__ + '\n' + '* ``Content``: ' + QgsNewsFeedModel.CustomRole.Content.__doc__ + '\n' + '* ``ImageUrl``: ' + QgsNewsFeedModel.CustomRole.ImageUrl.__doc__ + '\n' + '* ``Image``: ' + QgsNewsFeedModel.CustomRole.Image.__doc__ + '\n' + '* ``Link``: ' + QgsNewsFeedModel.CustomRole.Link.__doc__ + '\n' + '* ``Sticky``: ' + QgsNewsFeedModel.CustomRole.Sticky.__doc__
# --
QgsNewsFeedModel.CustomRole.baseClass = QgsNewsFeedModel
try:
    QgsNewsFeedModel.__group__ = ['network']
except NameError:
    pass
try:
    QgsNewsFeedProxyModel.__group__ = ['network']
except NameError:
    pass
