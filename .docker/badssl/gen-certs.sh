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

# --- Untrusted root ---

# 1. Generate a self-signed CA (not added to system trust)
openssl req -x509 -newkey rsa:4096 -days 3650 -nodes \
  -keyout ca.key -out ca.crt \
  -subj "/CN=Untrusted Local CA"

# 2. Generate untrusted key + CSR
openssl req -newkey rsa:2048 -nodes \
  -keyout /certs/untrusted.key -out /certs/untrusted.csr \
  -subj "/CN=badssl"

# 3. Sign the untrusted cert with your untrusted CA
openssl x509 -req -in /certs/untrusted.csr -CA ca.crt -CAkey ca.key \
  -CAcreateserial -out /certs/untrusted.crt -days 3650 \
  -extfile <(echo "subjectAltName=DNS:badssl,DNS:localhost")

cat /certs/untrusted.crt ca.crt > /certs/fullchain_untrusted.crt
