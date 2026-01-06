{ lib
, stdenv

, fetchFromGitHub
, makeWrapper
, replaceVars
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
  # Override libspatialindex to use version 2.0.0
  # See https://github.com/libspatialindex/libspatialindex/issues/276
  # An alternative would be to make this available/downgrade the version
  # from the nixpkgs side.
  libspatialindex_2_0 = libspatialindex.overrideAttrs (oldAttrs: rec {
    version = "2.0.0";
    src = fetchFromGitHub {
      owner = "libspatialindex";
      repo = "libspatialindex";
      rev = version;
      sha256 = "sha256-hZyAXz1ddRStjZeqDf4lYkV/g0JLqLy7+GrSUh75k20=";
    };
  });

  qgisSourceFiles =
    lib.fileset.difference
      (lib.fileset.gitTracked ../.)
      (lib.fileset.unions [
        # excluded files
        ./.
        ../flake.nix
        ../flake.lock
        ../.docker
        ../.github
        ../.ci
        ../debian
        ../editors
        ../ms-windows
        ../rpm
        ../vcpkg
      ]);

  # Version parsing taken from
  # https://github.com/qgis/QGIS/blob/1f0328cff6a8b4cf8a4f8d44a4304b9d9706aa72/rpm/buildrpms.sh#L118
  cmakeListsFile = lib.readFile ../CMakeLists.txt;
  extractVersion = pattern:
    let
      matches = lib.match ".*[sS][eE][tT]\\(${pattern}[[:space:]]+\"([0-9]+)\".*" cmakeListsFile;
    in
      if matches != null then lib.head matches else "0";
  qgisVersion =
    let
      major = extractVersion "CPACK_PACKAGE_VERSION_MAJOR";
      minor = extractVersion "CPACK_PACKAGE_VERSION_MINOR";
      patch = extractVersion "CPACK_PACKAGE_VERSION_PATCH";
    in
      "${major}.${minor}.${patch}";

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
  version = qgisVersion;
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
    libspatialindex_2_0
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
    "-DWITH_QTWEBENGINE=True"

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
