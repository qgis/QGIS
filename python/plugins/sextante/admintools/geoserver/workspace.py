from sextante.admintools.geoserver.support import xml_property, write_bool, ResourceInfo, url

def workspace_from_index(catalog, node):
    name = node.find("name")
    return Workspace(catalog, name.text)

class Workspace(ResourceInfo): 
    resource_type = "workspace"

    def __init__(self, catalog, name):
        super(Workspace, self).__init__()
        self.catalog = catalog
        self.name = name

    @property
    def href(self):
        return url(self.catalog.service_url, ["workspaces", self.name + ".xml"])

    @property
    def coveragestore_url(self):
        return url(self.catalog.service_url, ["workspaces", self.name, "coveragestores.xml"])

    @property
    def datastore_url(self):
        return url(self.catalog.service_url, ["workspaces", self.name, "datastores.xml"])

    enabled = xml_property("enabled", lambda x: x.lower() == 'true')
    writers = dict(
        enabled = write_bool("enabled")
    )

    def __repr__(self):
        return "%s @ %s" % (self.name, self.href)
