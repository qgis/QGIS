# coding=utf-8
"""
# Put the collections object in module namespace
# http://effbot.org/pyfaq/how-do-i-share-global-variables-across-modules.htm

Always call this variable as attribute from config module e.g:
    from resource_sharing import config
    print config.COLLECTIONS

config.COLLECTIONS is a dict of collection with this structure:
    config.COLLECTIONS = {
        collection_id(computed): {
            'register_name': collection,
            'author': author,
            'author_email': email,
            'repository_url': self.url,
            'status': COLLECTION_NOT_INSTALLED_STATUS,
            'name': parser.get(collection, 'name'),
            'tags': parser.get(collection, 'tags'),
            'description': parser.get(collection, 'description'),
            'qgis_min_version': parser.get(collection, 'qgis_minimum_version'),
            'qgis_max_version': parser.get(collection, 'qgis_maximum_version')
        },
        ....
    }
"""

COLLECTION_NOT_INSTALLED_STATUS = 0
COLLECTION_INSTALLED_STATUS = 1
COLLECTION_ALL_STATUS = 2

COLLECTIONS = {}
