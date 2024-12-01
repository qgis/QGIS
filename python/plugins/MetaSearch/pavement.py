###############################################################################
#
# Copyright (C) 2014 Tom Kralidis (tomkralidis@gmail.com)
#
# This source is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This code is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################

from configparser import ConfigParser
import getpass
import os
import shutil
import xml.etree.ElementTree as etree
import xmlrpc.client
import zipfile

from paver.easy import call_task, cmdopts, error, info, options, path, sh, task, Bunch

from owslib.csw import CatalogueServiceWeb  # spellok

PLUGIN_NAME = "MetaSearch"
BASEDIR = os.path.abspath(os.path.dirname(__file__))
USERDIR = os.path.expanduser("~")

with open("metadata.txt") as mf:
    cp = ConfigParser()
    cp.readfp(mf)
    VERSION = cp.get("general", "version")

options(
    base=Bunch(
        home=BASEDIR,
        plugin=path(BASEDIR),
        ui=path(BASEDIR) / "plugin" / PLUGIN_NAME / "ui",
        install=path("%s/.qgis3/python/plugins/MetaSearch" % USERDIR),
        ext_libs=path("plugin/MetaSearch/ext-libs"),
        tmp=path(path("%s/MetaSearch-dist" % USERDIR)),
        version=VERSION,
    ),
    upload=Bunch(host="plugins.qgis.org", port=80, endpoint="plugins/RPC2/"),
)


@task
def clean():
    """clean environment"""

    if os.path.exists(options.base.install):
        if os.path.islink(options.base.install):
            os.unlink(options.base.install)
        else:
            shutil.rmtree(options.base.install)
    if os.path.exists(options.base.tmp):
        shutil.rmtree(options.base.tmp)
    if os.path.exists(options.base.ext_libs):
        shutil.rmtree(options.base.ext_libs)
    for ui_file in os.listdir(options.base.ui):
        if ui_file.endswith(".py") and ui_file != "__init__.py":
            os.remove(options.base.plugin / "ui" / ui_file)
    os.remove(path(options.base.home) / "%s.pro" % PLUGIN_NAME)
    sh("git clean -dxf")


@task
def install():
    """install plugin into user QGIS environment"""

    plugins_dir = path(USERDIR) / ".qgis3/python/plugins"

    if os.path.exists(options.base.install):
        if os.path.islink(options.base.install):
            os.unlink(options.base.install)
        else:
            shutil.rmtree(options.base.install)

    if not os.path.exists(plugins_dir):
        raise OSError("The directory %s does not exist." % plugins_dir)
    if not hasattr(os, "symlink"):
        shutil.copytree(options.base.plugin, options.base.install)
    elif not os.path.exists(options.base.install):
        os.symlink(options.base.plugin, options.base.install)


@task
def package():
    """create zip file of plugin"""

    skip_files = [
        "AUTHORS.txt",
        "CMakeLists.txt",
        "requirements.txt",
        "requirements-dev.txt",
        "pavement.txt",
    ]

    package_file = get_package_filename()

    if not os.path.exists(options.base.tmp):
        options.base.tmp.mkdir()
    if os.path.exists(package_file):
        os.unlink(package_file)
    with zipfile.ZipFile(package_file, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(options.base.plugin):
            for file_add in files:
                if file_add.endswith(".pyc") or file_add in skip_files:
                    continue
                filepath = os.path.join(root, file_add)
                relpath = os.path.join(PLUGIN_NAME, os.path.relpath(filepath))
                zipf.write(filepath, relpath)
    return package_file  # return name of created zipfile


@task
@cmdopts(
    [
        ("user=", "u", "OSGeo userid"),
    ]
)
def upload():
    """upload package zipfile to server"""

    user = options.get("user", False)
    if not user:
        raise ValueError("OSGeo userid required")

    password = getpass.getpass("Enter your password: ")
    if password.strip() == "":
        raise ValueError("password required")

    call_task("package")

    zipf = get_package_filename()

    url = "http://%s:%s@%s:%d/%s" % (
        user,
        password,
        options.upload.host,
        options.upload.port,
        options.upload.endpoint,
    )

    info(f"Uploading to http://{options.upload.host}/{options.upload.endpoint}")

    server = xmlrpc.client.ServerProxy(url, verbose=False)

    try:
        with open(zipf) as zfile:
            plugin_id, version_id = server.plugin.upload(
                xmlrpc.client.Binary(zfile.read())
            )
            info("Plugin ID: %s", plugin_id)
            info("Version ID: %s", version_id)
    except xmlrpc.client.Fault as err:
        error("ERROR: fault error")
        error("Fault code: %d", err.faultCode)
        error("Fault string: %s", err.faultString)
    except xmlrpc.client.ProtocolError as err:
        error("Error: Protocol error")
        error("%s : %s", err.errcode, err.errmsg)
        if err.errcode == 403:
            error("Invalid name and password")


@task
def test_default_csw_connections():
    """test that the default CSW connections work"""

    relpath = "resources%sconnections-default.xml" % os.sep
    csw_connections_xml = options.base.plugin / relpath

    conns = etree.parse(csw_connections_xml)

    for conn in conns.findall("csw"):
        try:
            csw = CatalogueServiceWeb(conn.attrib.get("url"))  # spellok
            info("Success: %s", csw.identification.title)
            csw.getrecords2()
        except Exception as err:
            raise ValueError("ERROR: %s", err)


@task
@cmdopts(
    [
        ("filename=", "f", "Path to file of CSW URLs"),
    ]
)
def generate_csw_connections_file():
    """generate a CSW connections file from a flat file of CSW URLs"""

    filename = options.get("filename", False)

    if not filename:
        raise ValueError("path to file of CSW URLs required")

    conns = etree.Element("qgsCSWConnections")
    conns.attrib["version"] = "1.0"

    with open(filename) as connsfh:
        for line in connsfh:
            url = line.strip()
            if not url:  # blank line
                continue
            try:
                csw = CatalogueServiceWeb(url)  # spellok
                title = str(csw.identification.title)
                etree.SubElement(conns, "csw", name=title, url=url)
            except Exception as err:
                error("ERROR on CSW %s: %s", url, err)

    with open("%s.xml" % filename, "w") as connsxmlfh:
        connsxmlfh.write(etree.tostring(conns, encoding="utf-8"))


def get_package_filename():
    """return filepath of plugin zipfile"""

    filename = f"{PLUGIN_NAME}-{options.base.version}.zip"
    package_file = f"{options.base.tmp}/{filename}"
    return package_file
