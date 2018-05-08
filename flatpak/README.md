Flatpak
=======

Flatpak is an alternative system for distributing applications that allows
to deploy QGIS

 * very fast
 * bundle dependencies

That means you no longer have to worry that your system only ships with old
versions of libraries.

It also means, that everything that is contained in the app is installed in a
sandbox that can easily live next to your system's QGIS installation.

Installing
----------

Flatpak is available on various distributions, although in general only in their
very latest release. [Get flatpak!](http://flatpak.org/getting.html)

Building
--------

Since we don't have an online repository (yet), you need to build a package locally.

In the QGIS sourcecode folder

    cd flatpak
    ./create-flatpak.sh
    # Wait for everything to be built, this takes some time when done the first time

    flatpak --user remote-add --no-gpg-verify qgis-repo-local repo
    flatpak --user install qgis-repo-local org.qgis.qgis

Notes:

 * The `--user` flag does not require root permissions.
 * `remote-add` is only required to do once
