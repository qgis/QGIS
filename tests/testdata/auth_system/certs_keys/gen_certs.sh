#!/usr/bin/env bash
###########################################################################
#    gen_certs.sh
#    ---------------------
#    Date                 : June 2025
#    Copyright            : (C) 2025 by Nyall Dawson
#    Email                : nyall dot dawson at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

rm -rf ./*.pem ./*.cer ./*.key ./*.crt ./*.csr ./*.srl ./*.der ./*.p12 ./*.p7b

# QGIS Test Root CA

openssl genrsa -des3 -out root_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in root_ca_key.pem -out root_ca_key.pem -passin pass:1234 -traditional
chmod 400 root_ca_key.pem

openssl req -new -key root_ca_key.pem -days 99999 -out root_ca_cert.pem -x509 -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Root CA/emailAddress=testcert@qgis.org/' -addext 'basicConstraints=critical,CA:TRUE' -addext 'subjectKeyIdentifier=hash' -addext 'authorityKeyIdentifier=keyid:always,issuer:always' -addext 'keyUsage=keyCertSign, cRLSign' -addext 'nsCertType=sslCA, emailCA, objCA' -addext 'nsComment=xca certificate' -set_serial 1

openssl base64 -d -in root_ca_cert.pem -out root_ca_cert.der

# QGIS Test Root2 CA

openssl genrsa -des3 -out root2_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in root2_ca_key.pem -out root2_ca_key.pem -passin pass:1234 -traditional
chmod 400 root2_ca_key.pem
openssl base64 -d -in root2_ca_key.pem -out root2_ca_key.der

openssl req -new -key root2_ca_key.pem -days 99999 -out root2_ca_cert.pem -x509 -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Root2 CA/emailAddress=testcert@qgis.org/' -addext 'basicConstraints=critical,CA:TRUE' -addext 'subjectKeyIdentifier=hash' -addext 'authorityKeyIdentifier=keyid:always,issuer:always' -addext 'keyUsage=keyCertSign, cRLSign' -addext 'nsCertType=sslCA, emailCA, objCA' -addext 'nsComment=xca certificate' -set_serial 1

openssl base64 -d -in root2_ca_cert.pem -out root2_ca_cert.der

# QGIS Test Root3 CA

openssl genrsa -des3 -out root3_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in root3_ca_key.pem -out root3_ca_key.pem -passin pass:1234 -traditional
chmod 400 root3_ca_key.pem

faketime '2017-10-25 08:15:42' openssl req -new -key root3_ca_key.pem -days 1 -out root3_ca_cert-EXPIRED.pem -x509 -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Root3 CA/emailAddress=testcert@qgis.org/' -addext 'basicConstraints=critical,CA:TRUE' -addext 'subjectKeyIdentifier=hash' -addext 'authorityKeyIdentifier=keyid:always,issuer:always' -addext 'keyUsage=keyCertSign, cRLSign' -addext 'nsCertType=sslCA, emailCA, objCA' -addext 'nsComment=xca certificate' -set_serial 1


cat root_ca_cert.pem root2_ca_cert.pem root3_ca_cert-EXPIRED.pem > qgis_roots.pem
openssl crl2pkcs7 -nocrl -certfile root_ca_cert.pem -certfile root2_ca_cert.pem -certfile root3_ca_cert-EXPIRED.pem -out qgis_roots.p7b


# QGIS Test Issuer CA

openssl genrsa -des3 -out issuer_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in issuer_ca_key.pem -out issuer_ca_key.pem -passin pass:1234 -traditional
chmod 400 issuer_ca_key.pem

openssl base64 -d -in issuer_ca_key.pem -out issuer_ca_key.der

openssl req -new -key issuer_ca_key.pem -out issuer_ca_cert.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Issuer CA/emailAddress=testcert@qgis.org/'

cat <<EOF > issuer_ca.cnf
[ v3_ca ]
basicConstraints = critical,CA:TRUE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always
keyUsage = keyCertSign, cRLSign
nsCertType = sslCA,emailCA,objCA
nsComment = "xca certificate"
EOF

openssl x509 -req -days 99999 -in issuer_ca_cert.csr -CA root_ca_cert.pem -CAkey root_ca_key.pem -out issuer_ca_cert.pem -set_serial 2 -extfile issuer_ca.cnf -extensions v3_ca

rm issuer_ca_cert.csr

openssl base64 -d -in issuer_ca_cert.pem -out issuer_ca_cert.der

# QGIS Test Issuer2 CA

openssl genrsa -des3 -out issuer2_ca_key.pem -passout pass:password 2048
# Remove the passphrase
openssl rsa -in issuer2_ca_key.pem -out issuer2_ca_key.pem -passin pass:password -traditional
chmod 400 issuer2_ca_key.pem

openssl base64 -d -in issuer2_ca_key.pem -out issuer2_ca_key.der

openssl req -new -key issuer2_ca_key.pem -out issuer2_ca_cert.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Issuer2 CA/emailAddress=testcert@qgis.org/'


openssl x509 -req -days 99999 -in issuer2_ca_cert.csr -CA root2_ca_cert.pem -CAkey root2_ca_key.pem -out issuer2_ca_cert.pem -set_serial 2 -extfile issuer_ca.cnf -extensions v3_ca

rm issuer2_ca_cert.csr

openssl base64 -d -in issuer2_ca_cert.pem -out issuer2_ca_cert.der

# QGIS Test Issuer3 CA

openssl genrsa -des3 -out issuer3_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in issuer3_ca_key.pem -out issuer3_ca_key.pem -passin pass:1234 -traditional
chmod 400 issuer3_ca_key.pem

openssl req -new -key issuer3_ca_key.pem -out issuer3_ca_cert.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Issuer3 CA/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in issuer3_ca_cert.csr -CA root3_ca_cert-EXPIRED.pem -CAkey root3_ca_key.pem -out issuer3_ca_cert.pem -set_serial 2 -extfile issuer_ca.cnf -extensions v3_ca

rm issuer3_ca_cert.csr

# QGIS Test Issuer4 CA

openssl genrsa -des3 -out issuer4_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in issuer4_ca_key.pem -out issuer4_ca_key.pem -passin pass:1234 -traditional
chmod 400 issuer4_ca_key.pem

openssl req -new -key issuer4_ca_key.pem -out issuer4_ca_cert.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Issuer4 CA/emailAddress=testcert@qgis.org/'

faketime '2017-10-25 08:15:42' openssl x509 -req -days 1 -in issuer4_ca_cert.csr -CA root2_ca_cert.pem -CAkey root2_ca_key.pem -out issuer4_ca_cert-EXPIRED.pem -set_serial 3 -extfile issuer_ca.cnf -extensions v3_ca

rm issuer4_ca_cert.csr

# QGIS Test Subissuer CA

openssl genrsa -des3 -out subissuer_ca_key.pem -passout pass:1234 2048
# Remove the passphrase
openssl rsa -in subissuer_ca_key.pem -out subissuer_ca_key.pem -passin pass:1234
chmod 400 subissuer_ca_key.pem

openssl base64 -d -in subissuer_ca_key.pem -out subissuer_ca_key.der

openssl req -new -key subissuer_ca_key.pem -out subissuer_ca_cert.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Certificate Authority/CN=QGIS Test Subissuer CA/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in subissuer_ca_cert.csr -CA issuer_ca_cert.pem -CAkey issuer_ca_key.pem -out subissuer_ca_cert.pem -set_serial 3 -extfile issuer_ca.cnf -extensions v3_ca

rm subissuer_ca_cert.csr

openssl base64 -d -in subissuer_ca_cert.pem -out subissuer_ca_cert.der


cat subissuer_ca_cert.pem issuer_ca_cert.pem issuer2_ca_cert.pem issuer3_ca_cert.pem issuer4_ca_cert-EXPIRED.pem > qgis_intermediates.pem
openssl crl2pkcs7 -nocrl -certfile subissuer_ca_cert.pem -certfile issuer_ca_cert.pem -certfile issuer2_ca_cert.pem -certfile issuer3_ca_cert.pem -certfile issuer4_ca_cert-EXPIRED.pem -out qgis_intermediates.p7b


cat issuer_ca_cert.pem root_ca_cert.pem > chain_issuer-root.pem
cat issuer2_ca_cert.pem root2_ca_cert.pem > chain_issuer2-root2.pem
cat issuer3_ca_cert.pem root3_ca_cert-EXPIRED.pem > chain_issuer3-root3-EXPIRED.pem
cat issuer4_ca_cert-EXPIRED.pem root2_ca_cert.pem > chain_issuer4-EXPIRED-root2.pem
cat subissuer_ca_cert.pem issuer_ca_cert.pem root_ca_cert.pem > chain_subissuer-issuer-root.pem
cat subissuer_ca_cert.pem issuer_ca_cert.pem root_ca_cert.pem issuer2_ca_cert.pem root2_ca_cert.pem > chains_subissuer-issuer-root_issuer2-root2.pem

rm issuer_ca.cnf
cat <<EOF > issuer_ca.cnf
[ v3_ca ]
basicConstraints = critical,CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always
keyUsage = Digital Signature, Key Encipherment, Data Encipherment
nsCertType = client, email
nsComment = "xca certificate"
EOF

# ptolemy

openssl genrsa -des3 -out ptolemy_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in ptolemy_key_w-pass_temp.pem -out ptolemy_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm ptolemy_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in ptolemy_key_w-pass.pem -out ptolemy_key.pem -passin pass:password -traditional
openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt -in ptolemy_key.pem -out ptolemy_key-pkcs8-rsa.pem

chmod 400 ptolemy_key.pem
chmod 400 ptolemy_key_w-pass.pem

openssl base64 -d -in ptolemy_key.pem -out ptolemy_key.der
openssl base64 -d -in ptolemy_key-pkcs8-rsa.pem -out ptolemy_key-pkcs8-rsa.der

openssl req -new -key ptolemy_key.pem -out ptolemy.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Ptolemy/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in ptolemy.csr -CA root_ca_cert.pem -CAkey root_ca_key.pem -out ptolemy_cert.pem -set_serial 3 -extfile issuer_ca.cnf -extensions v3_ca

rm ptolemy.csr

openssl base64 -d -in ptolemy_cert.pem -out ptolemy_cert.der

cp ptolemy_key.der ptolemy_key_der.key
cp ptolemy_key.pem ptolemy_key_pem.key

openssl pkcs12 -export -passout pass:password -out ptolemy.p12 -inkey ptolemy_key.pem -in ptolemy_cert.pem

cat root_ca_cert.pem ptolemy_cert.pem > ptolemy_w-chain.pem

openssl pkcs12 -export -passout pass:password -out ptolemy_w-chain.p12 -inkey ptolemy_key.pem -in ptolemy_w-chain.pem

rm ptolemy_w-chain.pem



# fra

openssl genrsa -des3 -out fra_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in fra_key_w-pass_temp.pem -out fra_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm fra_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in fra_key_w-pass.pem -out fra_key.pem -passin pass:password -traditional
openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt -in fra_key.pem -out fra_key-pkcs8-rsa.pem

chmod 400 fra_key.pem
chmod 400 fra_key_w-pass.pem

openssl base64 -d -in fra_key.pem -out fra_key.der
openssl base64 -d -in fra_key-pkcs8-rsa.pem -out fra_key-pkcs8-rsa.der

openssl req -new -key fra_key.pem -out fra.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Fra/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in fra.csr -CA issuer_ca_cert.pem -CAkey issuer_ca_key.pem -out fra_cert.pem -set_serial 4 -extfile issuer_ca.cnf -extensions v3_ca

rm fra.csr

openssl base64 -d -in fra_cert.pem -out fra_cert.der

openssl pkcs12 -export -passout pass:password -out fra.p12 -inkey fra_key.pem -in fra_cert.pem

cat fra_cert.pem issuer_ca_cert.pem root_ca_cert.pem fra_key.pem > fra_w-chain.pem

openssl pkcs12 -export -passout pass:password -out fra_w-chain.p12 -inkey fra_key.pem -in fra_w-chain.pem


rm issuer_ca.cnf
cat <<EOF > issuer_ca.cnf
[ v3_ca ]
basicConstraints = critical,CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always
keyUsage = Digital Signature, Non Repudiation, Key Encipherment
nsCertType = server
nsComment = "xca certificate"
EOF

# localhost_ssl

openssl genrsa -des3 -out localhost_ssl_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in localhost_ssl_key_w-pass_temp.pem -out localhost_ssl_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm localhost_ssl_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in localhost_ssl_key_w-pass.pem -out localhost_ssl_key.pem -passin pass:password -traditional

chmod 400 localhost_ssl_key.pem
chmod 400 localhost_ssl_key_w-pass.pem

openssl req -new -key localhost_ssl_key.pem -out localhost_ssl.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=SSL Server Certificate/CN=localhost/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in localhost_ssl.csr -CA issuer_ca_cert.pem -CAkey issuer_ca_key.pem -out localhost_ssl_cert.pem -set_serial 5 -extfile issuer_ca.cnf -extensions v3_ca

rm localhost_ssl.csr

openssl base64 -d -in localhost_ssl_cert.pem -out localhost_ssl_cert.der

openssl pkcs12 -export -passout pass:password -out localhost_ssl.p12 -inkey localhost_ssl_key.pem -in localhost_ssl_cert.pem

cat localhost_ssl_cert.pem issuer_ca_cert.pem root_ca_cert.pem > localhost_ssl_w-chain.pem

openssl pkcs12 -export -passout pass:password -out localhost_ssl_w-chain.p12 -inkey localhost_ssl_key.pem -in localhost_ssl_w-chain.pem


cp localhost_ssl_key.pem 127_0_0_1_ssl_key.pem
openssl req -new -key 127_0_0_1_ssl_key.pem -out 127_0_0_1_ssl.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/CN=127.0.0.1/emailAddress=testcerts@qgis.org/'
OPENSSL_CONF=/dev/null openssl x509 -req -days 99999 -in 127_0_0_1_ssl.csr -CA root2_ca_cert.pem -CAkey root2_ca_key.pem -out 127_0_0_1_ssl_cert.pem -sha256
#rm 127_0_0_1_ssl.csr

rm issuer_ca.cnf
cat <<EOF > issuer_ca.cnf
[ v3_ca ]
basicConstraints = critical,CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always
keyUsage = Digital Signature, Key Encipherment, Data Encipherment
nsCertType = client, email
nsComment = "xca certificate"
EOF

# gerardus

openssl genrsa -des3 -out gerardus_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in gerardus_key_w-pass_temp.pem -out gerardus_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm gerardus_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in gerardus_key_w-pass.pem -out gerardus_key.pem -passin pass:password -traditional
openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt -in gerardus_key.pem -out gerardus_key-pkcs8-rsa.pem

chmod 400 gerardus_key.pem
chmod 400 gerardus_key_w-pass.pem

openssl base64 -d -in gerardus_key.pem -out gerardus_key.der
openssl base64 -d -in gerardus_key-pkcs8-rsa.pem -out gerardus_key-pkcs8-rsa.der

openssl req -new -key gerardus_key.pem -out gerardus.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Gerardus/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in gerardus.csr -CA subissuer_ca_cert.pem -CAkey subissuer_ca_key.pem -out gerardus_cert.pem -set_serial 4 -extfile issuer_ca.cnf -extensions v3_ca

rm gerardus.csr

openssl base64 -d -in gerardus_cert.pem -out gerardus_cert.der

openssl pkcs12 -export -passout pass:password -out gerardus.p12 -inkey gerardus_key.pem -in gerardus_cert.pem

cat gerardus_cert.pem subissuer_ca_cert.pem issuer_ca_cert.pem root_ca_cert.pem > gerardus_w-chain.pem

openssl pkcs12 -export -passout pass:password -out gerardus_w-chain.p12 -inkey gerardus_key.pem -in gerardus_w-chain.pem

rm gerardus_w-chain.pem

# wildcard-ssl

openssl genrsa -des3 -out wildcard-ssl_qgis-test_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in wildcard-ssl_qgis-test_key_w-pass_temp.pem -out wildcard-ssl_qgis-test_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm wildcard-ssl_qgis-test_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in wildcard-ssl_qgis-test_key_w-pass.pem -out wildcard-ssl_qgis-test_key.pem -passin pass:password -traditional

chmod 400 wildcard-ssl_qgis-test_key.pem
chmod 400 wildcard-ssl_qgis-test_key_w-pass.pem

openssl req -new -key wildcard-ssl_qgis-test_key.pem -out wildcard-ssl_qgis-test.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=SSL Server Certificate/CN=*.qgis.test/emailAddress=testcert@qgis.org/'

rm issuer_ca.cnf
cat <<EOF > issuer_ca.cnf
[ v3_ca ]
basicConstraints = critical,CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always
keyUsage = Digital Signature, Non Repudiation, Key Encipherment
nsCertType = server
nsComment = "xca certificate"
subjectAltName="DNS:qgis.test, DNS:*.qgis.test"
EOF

openssl x509 -req -days 99999 -in wildcard-ssl_qgis-test.csr -CA issuer_ca_cert.pem -CAkey issuer_ca_key.pem -out wildcard-ssl_qgis-test_cert.pem -set_serial 6 -extfile issuer_ca.cnf -extensions v3_ca

rm wildcard-ssl_qgis-test.csr

openssl pkcs12 -export -passout pass:password -out wildcard-ssl_qgis-test.p12 -inkey wildcard-ssl_qgis-test_key.pem -in wildcard-ssl_qgis-test_cert.pem

cat wildcard-ssl_qgis-test_cert.pem issuer_ca_cert.pem root_ca_cert.pem > wildcard-ssl_qgis-test_cert_w-chain.pem

openssl pkcs12 -export -passout pass:password -out wildcard-ssl_qgis-test_w-chain.p12 -inkey wildcard-ssl_qgis-test_key.pem -in wildcard-ssl_qgis-test_cert_w-chain.pem

rm issuer_ca.cnf
cat <<EOF > issuer_ca.cnf
[ v3_ca ]
basicConstraints = critical,CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer:always
keyUsage = Digital Signature, Key Encipherment, Data Encipherment
nsCertType = client, email
nsComment = "xca certificate"
EOF

# marinus

openssl genrsa -des3 -out marinus_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in marinus_key_w-pass_temp.pem -out marinus_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm marinus_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in marinus_key_w-pass.pem -out marinus_key.pem -passin pass:password -traditional

chmod 400 marinus_key.pem
chmod 400 marinus_key_w-pass.pem

openssl req -new -key marinus_key.pem -out marinus.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Marinus/emailAddress=testcert@qgis.org/'

faketime '2017-10-25 08:15:42' openssl x509 -req -days 1 -in marinus.csr -CA issuer2_ca_cert.pem -CAkey issuer2_ca_key.pem -out marinus_cert-EXPIRED.pem -set_serial 4 -extfile issuer_ca.cnf -extensions v3_ca

rm marinus.csr

openssl pkcs12 -export -passout pass:password -out marinus-EXPIRED.p12 -inkey marinus_key.pem -in marinus_cert-EXPIRED.pem

cat marinus_cert-EXPIRED.pem issuer2_ca_cert.pem root2_ca_cert.pem > marinus_cert-EXPIRED_w-chain.pem

openssl pkcs12 -export -passout pass:password -out marinus-EXPIRED_w-chain.p12 -inkey marinus_key.pem -in marinus_cert-EXPIRED_w-chain.pem


# nicholas

openssl genrsa -des3 -out nicholas_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in nicholas_key_w-pass_temp.pem -out nicholas_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm nicholas_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in nicholas_key_w-pass.pem -out nicholas_key.pem -passin pass:password -traditional
openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt -in nicholas_key.pem -out nicholas_key-pkcs8-rsa.pem

chmod 400 nicholas_key.pem
chmod 400 nicholas_key_w-pass.pem

openssl base64 -d -in nicholas_key.pem -out nicholas_key.der
openssl base64 -d -in nicholas_key-pkcs8-rsa.pem -out nicholas_key-pkcs8-rsa.der

openssl req -new -key nicholas_key.pem -out nicholas.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Nicholas/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in nicholas.csr -CA issuer2_ca_cert.pem -CAkey issuer2_ca_key.pem -out nicholas_cert.pem -set_serial 3 -extfile issuer_ca.cnf -extensions v3_ca

rm nicholas.csr

openssl base64 -d -in nicholas_cert.pem -out nicholas_cert.der

openssl pkcs12 -export -passout pass:password -out nicholas.p12 -inkey nicholas_key.pem -in nicholas_cert.pem

cat nicholas_cert.pem issuer2_ca_cert.pem root2_ca_cert.pem > nicholas_cert_w-chain.pem

openssl pkcs12 -export -passout pass:password -out nicholas_w-chain.p12 -inkey nicholas_key.pem -in nicholas_cert_w-chain.pem

rm nicholas_cert_w-chain.pem

# henricus

openssl genrsa -des3 -out henricus_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in henricus_key_w-pass_temp.pem -out henricus_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm henricus_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in henricus_key_w-pass.pem -out henricus_key.pem -passin pass:password -traditional

chmod 400 henricus_key.pem
chmod 400 henricus_key_w-pass.pem

openssl req -new -key henricus_key.pem -out henricus.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Henricus/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in henricus.csr -CA issuer4_ca_cert-EXPIRED.pem -CAkey issuer4_ca_key.pem -out henricus_cert.pem -set_serial 4 -extfile issuer_ca.cnf -extensions v3_ca

rm henricus.csr

openssl pkcs12 -export -passout pass:password -out henricus.p12 -inkey henricus_key.pem -in henricus_cert.pem

cat henricus_cert.pem issuer4_ca_cert-EXPIRED.pem root2_ca_cert.pem > henricus_cert_w-chain-EXPIRED.pem

openssl pkcs12 -export -passout pass:password -out henricus_w-chain-EXPIRED.p12 -inkey henricus_key.pem -in henricus_cert_w-chain-EXPIRED.pem


# piri

openssl genrsa -des3 -out piri_key_w-pass_temp.pem -passout pass:password 2048
openssl rsa -des3 -in piri_key_w-pass_temp.pem -out piri_key_w-pass.pem -passin pass:password -passout pass:password -traditional
rm piri_key_w-pass_temp.pem

# Remove the passphrase
openssl rsa -in piri_key_w-pass.pem -out piri_key.pem -passin pass:password -traditional

chmod 400 piri_key.pem
chmod 400 piri_key_w-pass.pem

openssl req -new -key piri_key.pem -out piri.csr -subj '/C=US/ST=Alaska/L=Anchorage/O=QGIS Test CA/OU=Client Certificate/CN=Piri/emailAddress=testcert@qgis.org/'

openssl x509 -req -days 99999 -in piri.csr -CA issuer3_ca_cert.pem -CAkey issuer3_ca_key.pem -out piri_cert.pem -set_serial 3 -extfile issuer_ca.cnf -extensions v3_ca

rm piri.csr

openssl pkcs12 -export -passout pass:password -out piri.p12 -inkey piri_key.pem -in piri_cert.pem

cat piri_cert.pem issuer3_ca_cert.pem root3_ca_cert-EXPIRED.pem > piri_cert_w-chain-EXPIRED.pem

openssl pkcs12 -export -passout pass:password -out piri_w-chain-EXPIRED.p12 -inkey piri_key.pem -in piri_cert_w-chain-EXPIRED.pem

# postgres

openssl genrsa -des3 -out postgres.key -passout pass:password 2048
openssl rsa -in postgres.key -out postgres.key -passin pass:password

# Create the certificate signing request for postgres (CN=postgres)
openssl req -new -key postgres.key  -out postgres.csr -subj '/CN=postgres/'
openssl x509 -req -days 99999 -in postgres.csr -CA root_ca_cert.pem -CAkey root_ca_key.pem -out postgres.crt -CAcreateserial -extfile issuer_ca.cnf -extensions v3_ca

rm postgres.csr

# donald_key_DSA

openssl dsaparam -out dsaparam_temp.pem 2048
openssl gendsa -out donald_key_DSA.pem dsaparam_temp.pem
openssl dsa -in donald_key_DSA.pem -outform PEM -out donald_key_DSA.pem

rm dsaparam_temp.pem

openssl base64 -d -in donald_key_DSA.pem -out donald_key_DSA.der

sed 's/$/\r/' donald_key_DSA.pem > donald_key_DSA_crlf.pem

HEADER_LINE=$(head -n 1 "donald_key_DSA.pem")
FOOTER_LINE=$(tail -n 1 "donald_key_DSA.pem")
MIDDLE_CONTENT=$(head -n -1 "donald_key_DSA.pem" | \
                 tail -n +2 | \
                 tr -d '\n\r' | \
                 sed 's/[[:space:]]\+/ /g' | \
                 sed 's/^ //; s/ $//')
echo "$HEADER_LINE" > "donald_key_DSA_nonl.pem"
echo "$MIDDLE_CONTENT" >> "donald_key_DSA_nonl.pem"
echo "$FOOTER_LINE" >> "donald_key_DSA_nonl.pem"

# donald_key_EC

openssl ecparam -name prime256v1 -genkey -noout -out donald_key_EC.pem
openssl base64 -d -in donald_key_EC.pem -out donald_key_EC.der

# cleanup

rm ./*.srl
rm issuer_ca.cnf
