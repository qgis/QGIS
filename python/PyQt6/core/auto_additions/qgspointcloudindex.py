# The following has been generated automatically from src/core/pointcloud/qgspointcloudindex.h
# monkey patching scoped based enum
QgsPointCloudAccessType.Local.__doc__ = "Local means the source is a local file on the machine"
QgsPointCloudAccessType.Remote.__doc__ = "Remote means it's loaded through a protocol like HTTP"
QgsPointCloudAccessType.__doc__ = """The access type of the data, local is for local files and remote for remote files (over HTTP)

* ``Local``: Local means the source is a local file on the machine
* ``Remote``: Remote means it's loaded through a protocol like HTTP

"""
# --
try:
    QgsPointCloudNodeId.fromString = staticmethod(QgsPointCloudNodeId.fromString)
    QgsPointCloudNodeId.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudNode.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudIndex.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
