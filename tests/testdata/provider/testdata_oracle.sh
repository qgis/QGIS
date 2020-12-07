#!/bin/sh

HOST=$1

set -e

echo "create user QGIS identified by qgis;
grant create session to qgis;
grant all privileges to qgis;
exit" | sqlplus SYSTEM/adminpass@$HOST/XEPDB1

echo exit | sqlplus QGIS/qgis@$HOST/XEPDB1 @tests/testdata/provider/testdata_oracle.sql
