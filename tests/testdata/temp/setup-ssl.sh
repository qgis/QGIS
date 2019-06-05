#!/usr/bin/env bash

source /env-data.sh

SETUP_LOCKFILE="${ROOT_CONF}/.ssl.conf.lock"
if [ -f "${SETUP_LOCKFILE}" ]; then
	return 0
fi

# This script will setup default SSL config

# /etc/ssl/private can't be accessed from within container for some reason
# (@andrewgodwin says it's something AUFS related)  - taken from https://github.com/orchardup/docker-postgresql/blob/master/Dockerfile
cp -r /etc/ssl /tmp/ssl-copy/
chmod -R 0700 /etc/ssl
chown -R postgres /tmp/ssl-copy
rm -r /etc/ssl
mv /tmp/ssl-copy /etc/ssl

# Needed under debian, wasnt needed under ubuntu
mkdir -p ${PGSTAT_TMP}
chmod 0777 ${PGSTAT_TMP}

# moved from setup.sh
echo "ssl = true" >> $CONF
#echo "ssl_ciphers = 'DEFAULT:!LOW:!EXP:!MD5:@STRENGTH' " >> $CONF
#echo "ssl_renegotiation_limit = 512MB "  >> $CONF
echo "ssl_cert_file = '${SSL_CERT_FILE}'" >> $CONF
echo "ssl_key_file = '${SSL_KEY_FILE}'" >> $CONF
if [ ! -z "${SSL_CA_FILE}" ]; then
	echo "ssl_ca_file = '${SSL_CA_FILE}'                       # (change requires restart)" >> $CONF
fi
#echo "ssl_crl_file = ''" >> $CONF

# Put lock file to make sure conf was not reinitialized
touch ${SETUP_LOCKFILE}
