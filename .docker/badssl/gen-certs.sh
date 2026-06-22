#!/bin/bash
set -e

mkdir -p /certs

# --- Self-signed ---
openssl req -x509 -newkey rsa:2048 -keyout /certs/selfsigned.key -out /certs/selfsigned.crt \
  -days 3650 -nodes -subj "/CN=badssl"  -addext "subjectAltName=DNS:badssl,DNS:localhost"

# --- CA + cert expired ---
openssl req -x509 -newkey rsa:2048 -keyout /certs/ca.key -out /certs/ca.crt \
  -days 3650 -nodes -subj "/CN=badssl" -addext "subjectAltName=DNS:badssl,DNS:localhost"

openssl req -newkey rsa:2048 -keyout /certs/expired.key -out /certs/expired.csr \
  -nodes -subj "/CN=badssl" -addext "subjectAltName=DNS:badssl,DNS:localhost"

openssl x509 -req -in /certs/expired.csr -CA /certs/ca.crt -CAkey /certs/ca.key \
  -CAcreateserial -out /certs/expired.crt \
  -not_before 20200101000000Z \
  -not_after  20200102000000Z
