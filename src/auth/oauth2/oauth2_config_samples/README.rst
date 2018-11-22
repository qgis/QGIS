*************************************************************
OAuth2 Authentication Method Sample Predefined Configurations
*************************************************************

.. warning:: These configs are non-functional and meant merely as samples.

JSON-formatted configs are useful for large-scale QGIS deployments where system
administrators wish to preconfigure complicated OAuth2 configurations for users.

In order to utilize these in a more meaningful manner, you will need to
reference the OAuth2 documentation at the respective endpoint vendor and,
depending upon the grant flow required or chosen, set up an application that
represents the local QGIS.app at the vendor's API or user admin website.

Once created, JSON-formatted configs can be shared across multiple users via
special directories within .qgis3 or QGIS install locations. Or, they can be
imported by users to partially preconfigure an OAuth2 configuration, where the
user adds their specific user information after the import. See QGIS help
documentation on the OAuth2 auth method plugin for more details.
