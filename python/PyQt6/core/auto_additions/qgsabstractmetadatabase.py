# The following has been generated automatically from src/core/metadata/qgsabstractmetadatabase.h
try:
    QgsAbstractMetadataBase.Address.__attribute_docs__ = {'type': "Type of address, e.g. 'postal'.", 'address': "Free-form physical address component, e.g. '221B Baker St' or 'P.O. Box 196'.", 'city': 'City or locality name.', 'administrativeArea': 'Administrative area (state, province/territory, etc.).', 'postalCode': 'Postal (or ZIP) code.', 'country': 'Free-form country string.'}
    QgsAbstractMetadataBase.Address.__doc__ = """Metadata address structure.

.. versionadded:: 3.2"""
    QgsAbstractMetadataBase.Address.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractMetadataBase.Contact.__attribute_docs__ = {'name': 'Name of contact.', 'organization': 'Organization contact belongs to/represents.', 'position': 'Position/title of contact.', 'addresses': 'List of addresses associated with this contact.', 'voice': 'Voice telephone.', 'fax': 'Facsimile telephone.', 'email': 'Electronic mail address.\n\n.. note::\n\n   Do not include mailto: protocol as part of the email address.', 'role': "Role of contact. Acceptable values are those from the ISO 19115 CI_RoleCode specifications\n(see http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml).\nE.g. 'custodian', 'owner', 'distributor', etc."}
    QgsAbstractMetadataBase.Contact.__doc__ = """Metadata contact structure.

.. versionadded:: 3.2"""
    QgsAbstractMetadataBase.Contact.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractMetadataBase.Link.__attribute_docs__ = {'name': 'Short link name. E.g. WMS layer name.', 'type': "Link type. It is strongly suggested to use values from the 'identifier'\ncolumn in https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv", 'description': 'Abstract text about link.', 'url': 'Link url.  If the URL is an OWS server, specify the *base* URL only without parameters like service=xxx....', 'format': 'Format specification of online resource. It is strongly suggested to use GDAL/OGR format values.', 'mimeType': 'MIME type representative of the online resource response (image/png, application/json, etc.)', 'size': 'Estimated size (in bytes) of the online resource response.'}
    QgsAbstractMetadataBase.Link.__doc__ = """Metadata link structure.

.. versionadded:: 3.2"""
    QgsAbstractMetadataBase.Link.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractMetadataBase.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
