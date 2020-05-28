#!/bin/sh

sqlcmd -S $SQLHOST,$SQLPORT -U $SQLUSER -P $SQLPASSWORD -i tests/testdata/provider/testdata_mssql.sql
