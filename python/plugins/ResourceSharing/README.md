QGIS Resources Sharing Plugin
==============================
QGIS Resources Sharing allows you to share collections (set of resources: 
SVGs, symbols, styles, color ramps, and processing scripts). Please read the 
documentation here www.akbargumbira.com/qgis_resources_sharing.

[![Build Status](https://travis-ci.org/akbargumbira/qgis_resources_sharing.svg?branch=master)](https://travis-ci.org/akbargumbira/qgis_resources_sharing) [![Coverage Status](https://coveralls.io/repos/github/akbargumbira/qgis_resources_sharing/badge.svg?branch=master)](https://coveralls.io/github/akbargumbira/qgis_resources_sharing?branch=master) [![Gitter](https://badges.gitter.im/akbargumbira/qgis_resources_sharing.svg)](https://gitter.im/akbargumbira/qgis_resources_sharing?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge) 


Contributing
==============================
This project uses git submodule for the test data. To clone the project completely, do:

```
git clone git@github.com:akbargumbira/qgis_resources_sharing.git <the destination directory>
cd <the destination directory>
git submodule init
git submodule update
```

## Branching
We have some branches that we use for our workflow:

* **develop**: The latest development version. Please make a PR to this 
branch if you make a fix or develop new features.
* **master**: The latest stable version. If you find an issue and have a 
patch, please after pushing the fix in develop branch, cherry-pick to 
the this branch and make another PR to this branch. After having enough 
fixes, we will release a new version from this branch.
* **gh-pages**: This is the branch for the documentation hosted in github 
pages (www.akbargumbira.com/qgis_resources_sharing). It's in markdown. Please
 make a PR to this branch if you want to change or add something to the 
 documentation.
