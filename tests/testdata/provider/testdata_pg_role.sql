DROP GROUP IF EXISTS qgis_test_group;
CREATE USER qgis_test_group NOLOGIN;

DROP USER IF EXISTS qgis_test_user;
CREATE USER qgis_test_user PASSWORD 'qgis_test_user_password' LOGIN;
ALTER GROUP qgis_test_group ADD USER qgis_test_user;

DROP USER IF EXISTS qgis_test_unprivileged_user;
CREATE USER qgis_test_unprivileged_user WITH PASSWORD
'qgis_test_unprivileged_user_password' LOGIN;
