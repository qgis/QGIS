DROP GROUP IF EXISTS qgis_test_group;
DROP USER IF EXISTS qgis_test_user;
CREATE USER qgis_test_user PASSWORD 'qgis_test_user_password' LOGIN;
CREATE USER qgis_test_group NOLOGIN;
ALTER GROUP qgis_test_group ADD USER qgis_test_user;
