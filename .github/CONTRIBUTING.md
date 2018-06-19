Contributing to QGIS
====================

QGIS is an open source project and we appreciate contributions very much.

Proper formatting
-----------------

Before making a pull request, please make sure your code is properly formatted
by running the prepare commit script **before** issuing `git commit`.

    ./scripts/prepare-commit.sh

This can be automated by setting up the pre-commit hook properly.

    ln -s ../../scripts/prepare-commit.sh .git/hooks/pre-commit

Windows equivalent for a symlink using PowerShell in elevated priviledges mode from the .git\hooks folder. This will create hardlink which can be used from git-bash shell.

    cmd /c mklink /H pre-commit ..\..\scripts\prepare-commit.sh

Windows also requires all scripts to be in shell path, run this from QGIS repository root to achieve this:

    export PATH=$PATH:scripts/

Getting your pull request merged
--------------------------------

This is a volunteer project, so sometimes it may take a little while to merge
your pull request.

There is a [guide with hints for getting your pull requests merged](https://docs.qgis.org/testing/en/docs/developers_guide/git.html#pull-requests)
in the developers guide.
