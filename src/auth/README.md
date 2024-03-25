_Originally available in the [user manual](https://docs.qgis.org/3.28/en/docs/user_manual/auth_system/auth_considerations.html)_

# Security Considerations

Once the master password is entered, the API is open to access authentication
configs in the authentication database, similar to how Firefox works.
However, in the initial implementation, no wall against PyQGIS access has been defined.
This may lead to issues where a user downloads/installs a malicious PyQGIS plugin
or standalone app that gains access to authentication credentials.

The quick solution for initial release of feature is to just not include most
PyQGIS bindings for the authentication system.

Another simple, though not robust, fix is to add a combobox 
in Settings --> Options --> Authentication (defaults to "never"):

```
   "Allow Python access to authentication system"
   Choices: [ confirm once per session | always confirm | always allow | never]
```

Such an option's setting would need to be saved in a location non-accessible to Python,
e.g. the authentication database, and encrypted with the master password.

* Another option may be to track which plugins the user has specifically
  allowed to access the authentication system, though it may be tricky to deduce
  which plugin is actually making the call.

* Sandboxing plugins, possibly in their own virtual environments,
  would reduce 'cross-plugin' hacking of authentication configs from another plugin
  that is authorized.
  This might mean limiting cross-plugin communication as well,
  but maybe only between third-party plugins.

* Another good solution is to issue code-signing certificates to vetted plugin authors.
  Then validate the plugin's certificate upon loading.
  If need be the user can also directly set an untrusted policy for the certificate associated
  with the plugin using existing certificate management dialogs.

* Alternatively, access to sensitive authentication system data from Python could never be allowed,
  and only the use of QGIS core widgets, or duplicating authentication system integrations,
  would allow the plugin to work with resources that have an authentication configuration,
  while keeping master password and authentication config loading in the realm of the main app.

The same security concerns apply to C++ plugins, though it will be harder to restrict access,
since there is no function binding to simply be removed as with Python.

## Restrictions

The confusing [licensing and exporting](https://www.openssl.org/docs/faq.html)
issues associated with OpenSSL apply.
In order for Qt to work with SSL certificates, it needs access to the OpenSSL libraries.
Depending upon how Qt was compiled, the default is to dynamically link
to the OpenSSL libs at run-time (to avoid the export limitations).

QCA follows a similar tactic, whereby linking to QCA incurs no restrictions,
because the qca-ossl (OpenSSL) plugin is loaded at run-time.
The qca-ossl plugin is directly linked to the OpenSSL libs. Packagers would be the ones
needing to ensure any OpenSSL-linking restrictions are met, if they ship the plugin.
Maybe. I don't really know. I'm not a lawyer.

The authentication system safely disables itself when ``qca-ossl`` is not found at run-time.
