{ lib
, stdenv

, makeWrapper
, replaceVars
, runCommand
, wrapGAppsHook3
, wrapQtAppsHook

, withGrass
, withServer

, bison
, cmake
, draco
, exiv2
, fcgi
, flex
, geos
, grass
, gsl
, hdf5
, libspatialindex
, libspatialite
, libzip
, netcdf
, ninja
, openssl
, pdal
, libpq
, proj
, protobuf
, python3
, qca
, qscintilla
, qt3d
, qt5compat
, qtbase
, qtkeychain
, qtlocation
, qtmultimedia
, qtsensors
, qtserialport
, qttools
, qtwebengine
, qwt
, sqlite
, txt2tags
, zstd
}:

let
  versionSourceFiles = lib.fileset.toSource {
    root = ../.;
    fileset = ../CMakeLists.txt;
  };

  qgisSourceFiles =
    lib.fileset.difference
      (lib.fileset.gitTracked ../.)
      (lib.fileset.unions [
        # excluded files
        ./.
        ../flake.nix
        ../flake.lock
      ]);

  # Version parsing taken from
  # https://github.com/qgis/QGIS/blob/1f0328cff6a8b4cf8a4f8d44a4304b9d9706aa72/rpm/buildrpms.sh#L118
  qgisVersion =
    lib.replaceStrings [ "\n" ] [ "" ]
      (lib.readFile (
        runCommand "qgis-version" { } ''
          major=$(grep -ie 'SET(CPACK_PACKAGE_VERSION_MAJOR' ${versionSourceFiles}/CMakeLists.txt |
            sed -r 's/.*\"([0-9]+)\".*/\1/g')
          minor=$(grep -ie 'SET(CPACK_PACKAGE_VERSION_MINOR' ${versionSourceFiles}/CMakeLists.txt |
            sed -r 's/.*\"([0-9]+)\".*/\1/g')
          patch=$(grep -ie 'SET(CPACK_PACKAGE_VERSION_PATCH' ${versionSourceFiles}/CMakeLists.txt |
            sed -r 's/.*\"([0-9]+)\".*/\1/g')

          version=$major.$minor.$patch
          echo $version > $out
        ''
      ));

  py = python3.override {
    self = py;
    packageOverrides = self: super: {
      pyqt6 = super.pyqt6.override {
        withSerialPort = true;
      };
    };
  };

  pythonBuildInputs = with py.pkgs; [
    chardet
    gdal
    jinja2
    numpy
    owslib
    psycopg2
    pygments
    pyqt6
    pyqt-builder
    python-dateutil
    pytz
    pyyaml
    qscintilla-qt6
    requests
    setuptools
    sip
    six
    urllib3
  ];

in

# Print the list of included source files
# lib.fileset.trace qgisSourceFiles
stdenv.mkDerivation
{
  pname = "qgis-unwrapped";
  version = qgisVersion;  # this is a "Import from derivation (IFD)" !
  src = lib.fileset.toSource {
    root = ../.;
    fileset = qgisSourceFiles;
  };

  nativeBuildInputs = [
    makeWrapper
    wrapGAppsHook3
    wrapQtAppsHook

    bison
    cmake
    flex
    ninja
  ];

  buildInputs = [
    draco
    exiv2
    fcgi
    geos
    gsl
    hdf5
    libpq
    libspatialindex
    libspatialite
    libzip
    netcdf
    openssl
    pdal
    proj
    protobuf
    qca
    qscintilla
    qt3d
    qt5compat
    qtbase
    qtkeychain
    qtlocation
    qtmultimedia
    qtsensors
    qtserialport
    qttools
    qtwebengine
    qwt
    sqlite
    txt2tags
    zstd
  ] ++ lib.optional withGrass grass
  ++ pythonBuildInputs;

  patches = [
    (replaceVars ./set-pyqt6-package-dirs.patch {
      pyQt6PackageDir = "${py.pkgs.pyqt6}/${py.pkgs.python.sitePackages}";
      qsciPackageDir = "${py.pkgs.qscintilla-qt6}/${py.pkgs.python.sitePackages}";
    })
  ];

  # Add path to Qt platform plugins
  # (offscreen is needed by "${APIS_SRC_DIR}/generate_console_pap.py")
  env.QT_QPA_PLATFORM_PLUGIN_PATH = "${qtbase}/${qtbase.qtPluginPrefix}/platforms";

  cmakeFlags = [
    "-DBUILD_WITH_QT6=True"
    "-DWITH_QTWEBENGINE=True"
    "-DWITH_QTWEBKIT=False"

    "-DWITH_3D=True"
    "-DWITH_PDAL=True"
    "-DENABLE_TESTS=False"
    "-DQT_PLUGINS_DIR=${qtbase}/${qtbase.qtPluginPrefix}"
  ] ++ lib.optional withServer [
    "-DWITH_SERVER=True"
    "-DQGIS_CGIBIN_SUBDIR=${placeholder "out"}/lib/cgi-bin"
  ]
  ++ lib.optional withGrass (
    let
      gmajor = lib.versions.major grass.version;
      gminor = lib.versions.minor grass.version;
    in
    "-DGRASS_PREFIX${gmajor}=${grass}/grass${gmajor}${gminor}"
  );

  qtWrapperArgs = [
    "--set QT_QPA_PLATFORM_PLUGIN_PATH ${qtbase}/${qtbase.qtPluginPrefix}/platforms"
  ];

  dontWrapGApps = true; # wrapper params passed below

  # GRASS has to be available on the command line even though we baked in the
  # path at build time using GRASS_PREFIX. Using wrapGAppsHook also prevents
  # file dialogs from crashing the program on non-NixOS.
  postFixup = lib.optionalString withGrass ''
    for program in $out/bin/*; do
      wrapProgram $program \
        "''${gappsWrapperArgs[@]}" \
        --prefix PATH : ${lib.makeBinPath [ grass ]}
    done
  '';

  passthru = {
    inherit pythonBuildInputs;
    inherit py;
  };

  meta = with lib; {
    description = "Free and Open Source Geographic Information System";
    homepage = "https://www.qgis.org";
    license = licenses.gpl2Plus;
    maintainers = with maintainers; teams.geospatial.members;
    platforms = with platforms; linux;
    mainProgram = "qgis";
  };
}
