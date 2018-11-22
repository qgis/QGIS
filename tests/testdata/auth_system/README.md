# QGIS Test Certificates/Keys

The certs/keys are generated/edited using **XCA** (see xca-project directory):

    https://sourceforge.net/projects/xca/

The Java keystore files are generated/edited using **KeyStore Explorer**:

    http://keystore-explorer.sourceforge.net/


The default password for the encrypted XCA project and Java keystore files is
**password**. The certificate signing structure can be reviewed in
`cert_hierarchy_8bit.png`.

**WARNING**: These components are just for testing and should _NOT_ be used
in a production environment.

*NOTE*: The `.[crt|pem]` choice for files (below) is because some applications
filter file open dialogs to specific extensions, e.g. pgAdmin3 always filters
`.crt` or `.key` and QGIS generally filters on `.pem`.

## Certificate Signing Hierarchy

![Certs tree](cert_hierarchy_8bit.png)

## Client Certificates/Keys

* User certs: `[user]-cert.[crt|pem]`

* User certs, with CA chain: `[user]-cert_w-chain_.[crt|pem]`

* User keys:  `[user]-key.[key|pem]`

* User encrypted keys:  `[user]-key_w-pass_.[key|pem]`

* Combined user certs/keys:  `[user].p12`

* Combined user certs/keys, with CA chain:  `[user]_w-chain_.p12`

The default password for encrypted client keys is **password**.

## Client-side Certificate Authorities

* Root CA for all servers (below): `root-ca-cert.[crt|pem]`

The test root cert for all server certs is self-signed. You will need to have
this CA _trusted_ in your OS's or application's cert/key store or passed during
connections, so as to validate the cert of the connected server.

* Concatenated intermediates/roots: `qgis_intermediates.[crt|pem]`,
  `qgis_roots.[crt|pem]`

Example use of concatenated files: load roots into OS certificate store and set
them to trusted; import intermediates into QGIS Certificate Manager; then, add
client cert/key bundles to authentication configurations.

See **Client _hosts_ file configuration** below for configuring non-DNS host
resolution for the test server connections.

## Server Certificates/Keys

Two certificates are available for general SSL/TLS servers:

* `localhost_ssl_[cert|key].[crt|pem]` for **localhost** test servers
  accessed from the same host.

* `wildcard-ssl_qgis-test_[cert|key].[crt|pem]` provides for
   **\*.qgis.test** domains, e.g. `whatever.qgis.test` or
  `qgis.test`, for testing non-localhost connections. Services are
  on different test machines, e.g. Docker containers.

All server cert/key bundles have variants that include CA chains and .p12 files.

The default password for encrypted server keys is **password**.

All SSL certs are signed under `chain_issuer-root.[crt|pem]` certificate chain.

### Client _hosts_ file configuration

Domains of the non-localhost certificates can be associated locally for an IP
address of a remote test server or an (essentially remote) VM or docker
container using the host OS's `hosts` file. This setup allows for testing where
a remote _localhost_ domain or and IP address will result in a 'hostname
mismatch' SSL error from clients.

Example entries in `hosts` file:

    <docker-container-on-linux-ip>         geoserver.qgis.test
    <another-docker-container-on-linux-ip> gwc.qgis.test
    <some-docker-machine-ip>               postgis.qgis.test

### Server-side client validation

When a server validates client certificates, some client certs maybe be signed
by the `QGIS Test Root 2 CA`, which is not the same as the root self-signed
CA for the server certificates (`QGIS Test Root CA`). This is similar to
enterprise PKI setups where client certs are signed by a different root CA than
the server.

Add the root and intermediate chains to the server's configuration, so that such
clients can be authenticated. (This setup is already pre-configured in the Java
keystore file.)

* Concatenated cert of all _valid_ CA chains:
  `chains_subissuer-issuer-root_issuer2-root2.[crt|pem]`
