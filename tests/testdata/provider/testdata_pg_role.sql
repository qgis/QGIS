DROP GROUP IF EXISTS qgis_test_group;
CREATE USER qgis_test_group NOLOGIN;
GRANT USAGE, CREATE ON SCHEMA public TO qgis_test_group;

DROP USER IF EXISTS qgis_test_user;
CREATE USER qgis_test_user PASSWORD 'qgis_test_user_password' LOGIN;
ALTER GROUP qgis_test_group ADD USER qgis_test_user;

DROP USER IF EXISTS qgis_test_unprivileged_user;
CREATE USER qgis_test_unprivileged_user WITH PASSWORD
'qgis_test_unprivileged_user_password' LOGIN;

DROP USER IF EXISTS qgis_test_another_user;
CREATE USER qgis_test_another_user PASSWORD 'qgis_test_another_user_password' LOGIN;
ALTER GROUP qgis_test_group ADD USER qgis_test_another_user;
