#!/usr/bin/env bash
###########################################################################
#    buildrpms.sh
#    ---------------------
#    Date                 : March 2014
#    Copyright            : (C) 2014-2019 by Matthias Kuhn
#    Email                : matthias at opengis dot ch
#    ---------------------
#    Date                 : October 2017
#    Copyright            : (C) 2017-2019 by Daniele Viganò
#    Email                : daniele at vigano dot me
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

if [ $_BUILDRPMS_DEBUG ]
then
  set -x
fi

function print_info
{
  echo -e "\e[0;32m$1\033[0m"
}

function print_error
{
  echo -e "\033[0;31m$1\033[0m"
}

function print_help
{
  echo '
Creates RPM packages.

Usage:
  -c          only compile spec file
  -s          only create srpm, nothing will be compiled
  -u          build unstable, release will include the short commit id
  -b          build last srpm, the package release number will not be increased
  -h          show help
'
}

if [ $_MOCK_OLD_CHROOT ]
then
  mock_args="--old-chroot"
fi

if command -v pbzip2 &> /dev/null
then
  bzip2_cmd="pbzip2"
else
  bzip2_cmd="bzip2"
fi

relver=1
compile_spec_only=0
build_only=0
srpm_only=0
build_unstable=0

while getopts "csuhb" opt; do
  case ${opt} in
    c)
      compile_spec_only=1
      ;;
    s)
      srpm_only=1
      ;;
    u)
      build_unstable=1
      ;;
    \?|h)
      print_help
      exit 0
      ;;
    b)
      build_only=1
      ;;
  esac
done

# Load default config
source default.cfg
# Load local config file
if [ -f local.cfg ]
then
  source local.cfg
fi

if [ $build_unstable -ne 1 ]
then
  # Get next release version number and increment after
  if [ ! -f version.cfg ]
  then
    echo "relver=$relver" > version.cfg
  else
    source version.cfg
    if [ "$build_only" -ne "1" ]
    then
      (( relver+=1 ))
      echo "relver=$relver" > version.cfg
    fi
  fi
  timestamp=0
else
  relver="git$(git rev-parse --short HEAD)"
  timestamp=$(date +'%s')
fi

# Get the version string
major=$(grep -e 'SET(CPACK_PACKAGE_VERSION_MAJOR' ../CMakeLists.txt |
        sed -r 's/.*\"([0-9]+)\".*/\1/g')
minor=$(grep -e 'SET(CPACK_PACKAGE_VERSION_MINOR' ../CMakeLists.txt |
        sed -r 's/.*\"([0-9]+)\".*/\1/g')
patch=$(grep -e 'SET(CPACK_PACKAGE_VERSION_PATCH' ../CMakeLists.txt |
        sed -r 's/.*\"([0-9]+)\".*/\1/g')

version=$major.$minor.$patch

print_info "Building version $version-$relver"
if [ "$build_only" -ne "1" ]
then
  # Clean logfiles
  if [ -f $OUTDIR/build.log ]
  then
    print_info "Cleaning log file"
    rm $OUTDIR/build.log
  fi
  print_info "Creating spec file from template"
  # Create spec file
  cat qgis.spec.template \
    | sed -e s/%{_version}/$version/g \
    | sed -e s/%{_relver}/$relver/g \
    | sed -e s/%{_timestamp}/$timestamp/g \
    | tee qgis.spec 1>/dev/null

  if [ "$compile_spec_only" -eq "1" ]
  then
    exit 0
  fi

  print_info "Creating source tarball"
  # Create source tarball
  git -C .. archive --format=tar --prefix=qgis-$version/ HEAD | $bzip2_cmd > sources/qgis-$version.tar.bz2

  print_info "Creating source package"
  # Build source package
  if ! mock --buildsrpm --spec qgis.spec --sources ./sources \
       --define "_relver $relver" \
       --define "_version $version" \
       --define "_timestamp $timestamp" \
       --resultdir=$OUTDIR $mock_args
  then
    print_error "Creating source package failed"
    exit 1
  fi
  print_info "Source package created"
fi

srpm=$(grep -e 'Wrote: .*\.src\.rpm' $OUTDIR/build.log 2>/dev/null |
       sed 's_Wrote: /builddir/build/SRPMS/\(.*\)_\1_')

if [ "$srpm_only" -eq "1" ]
then
  exit 0
fi

# Create packages for every ARCH defined in the config file
for arch in "${ARCHS[@]}"
do :
  if [ -f $OUTDIR/$srpm ]
  then
    print_info "Building $srpm for $arch"
    if [ -d $OUTDIR/$arch ]
    then
      if [ -f $OUTDIR/$arch/build.log ]
      then
        print_info "Cleaning log file"
        rm $OUTDIR/$arch/build.log
      fi
    else
      mkdir $OUTDIR/$arch
    fi
    if ! mock -r $arch --rebuild $OUTDIR/$srpm \
         --define "_relver $relver" \
         --define "_version $version" \
         --define "_timestamp $timestamp" \
         --resultdir=$OUTDIR/$arch $mock_args
    then
      print_error "Package creation for $arch failed. Abort"
      exit 1
    else
      # Add to package list
      packages="$packages $(ls $OUTDIR/$arch/*-$version-$relver.*.rpm)"
    fi
  else
    print_error "Source package unavailable. Abort"
    exit 1
  fi
done

if $NOSIGN
then
  print_info "Signing packages"
  rpm --resign $packages
fi

print_info "Done"
