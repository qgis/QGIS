Contributing to QGIS
====================

QGIS is an open source project and we appreciate contributions very much.

Proper formatting
-----------------

Before making a pull request, please make sure your code is properly formatted
by running the prepare commit script **before** issuing `git commit`.

    ./scripts/prepare-commit.sh

This can be automated by setting up the pre-commit hook properly.

    ln -s scripts/prepare-commit.sh .git/hooks/pre-commit

Getting your pull request merged
--------------------------------

This is a volunteer project, so sometimes it may take a little while to merge
your pull request.

There is a [guide with hints for getting your pull requests merged](https://github.com/m-kuhn/QGIS-Website/blob/qtcreator.rst/source/site/getinvolved/development/git.rst#pull-requests)
in the developers guide.
