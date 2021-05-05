#!/usr/bin/env bash
###########################################################################
#    gen_certs.sh
#    ---------------------
#    Date                 : February 2021
#    Copyright            : (C) 2021 by Alessandro Pasotti
#    Email                : elpaso@itopen.it
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

rm -rf ./*.pem ./*.cer ./*.key ./*.crt ./*.csr ./*.srl
# Generate the CA/root private key
openssl genrsa -des3 -out qgis_ca.key -passout pass:1234 2048

# Remove the passphrase
openssl rsa -in qgis_ca.key -out qgis_ca.key -passin pass:1234
chmod 400 qgis_ca.key

# Create and sign the root/CA certificate
openssl req -new -key qgis_ca.key -days 3650 -out qgis_ca.crt -x509 -subj '/C=CA/ST=Alaska/L=Anchorage/O=QGIS.ORG/CN=QGIS Root Certificate/emailAddress=testcerts@qgis.org'


# Create the server key for 127.0.0.1
openssl genrsa -des3 -out 127_0_0_1.key -passout pass:1234 2048
openssl rsa -in 127_0_0_1.key -out 127_0_0_1.key -passin pass:1234


# Create the certificate signing request for server (CN=127.0.0.1)
openssl req -new -key 127_0_0_1.key  -out 127_0_0_1.csr -subj '/C=CA/ST=Alaska/L=Anchorage/O=QGIS.ORG/CN=127.0.0.1/emailAddress=testcerts@qgis.org'
openssl x509 -req -days 3650 -in 127_0_0_1.csr -CA qgis_ca.crt -CAkey qgis_ca.key -out 127_0_0_1.crt -CAcreateserial


# Create the server key for postgres
openssl genrsa -des3 -out postgres.key -passout pass:1234 2048
openssl rsa -in postgres.key -out postgres.key -passin pass:1234

# Create the certificate signing request for postgres (CN=postgres)
openssl req -new -key postgres.key  -out postgres.csr -subj '/C=CA/ST=Alaska/L=Anchorage/O=QGIS.ORG/CN=postgres/emailAddress=testcerts@qgis.org'
openssl x509 -req -days 3650 -in postgres.csr -CA qgis_ca.crt -CAkey qgis_ca.key -out postgres.crt -CAcreateserial


# Create the client key for Gerardus
openssl genrsa -des3 -out Gerardus.key -passout pass:1234 2048
openssl rsa -in Gerardus.key -out Gerardus.key -passin pass:1234
# Create the certificate signing request for user Gerardus
openssl req -new -key Gerardus.key -out Gerardus.csr -subj '/C=CA/ST=Alaska/L=Anchorage/O=QGIS.ORG/CN=Gerardus/emailAddress=testcerts@qgis.org'
# Create and sign the client certificates
openssl x509 -req -days 3650 -in Gerardus.csr -CA qgis_ca.crt -CAkey qgis_ca.key -out Gerardus.crt -CAcreateserial

# Create bundle for Gerardus (i.e. for browser testing)
cat Gerardus.key Gerardus.crt > Gerardus.pem

# Create the client key for docker
openssl genrsa -des3 -out docker.key -passout pass:1234 2048
openssl rsa -in docker.key -out docker.key -passin pass:1234
# Create the certificate signing request for user docker
openssl req -new -key docker.key -out docker.csr -subj '/C=CA/ST=Alaska/L=Anchorage/O=QGIS.ORG/CN=docker/emailAddress=testcerts@qgis.org'
# Create and sign the client certificates
openssl x509 -req -days 3650 -in docker.csr -CA qgis_ca.crt -CAkey qgis_ca.key -out docker.crt -CAcreateserial

# Create bundle for docker (i.e. for browser testing)
cat docker.key docker.crt > docker.pem
