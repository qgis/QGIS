{ lib
, stdenv

, qgisMinorVersion

, cmake
, doxygen
, ninja
, python3
, qtbase
, qttools
}:

let
  qgisSourceFiles =
    lib.fileset.difference
      (lib.fileset.gitTracked ../.)
      (lib.fileset.unions [
        # excluded files
        ./.
        ../flake.nix
        ../flake.lock
      ]);

  qgisSource = lib.fileset.toSource {
    root = ../.;
    fileset = qgisSourceFiles;
  };

in

# Print the list of included source files
# lib.fileset.trace qgisSourceFiles
stdenv.mkDerivation {
  pname = "qgis-api-documentation";
  version = qgisMinorVersion;
  src = qgisSource;

  nativeBuildInputs = [
    cmake
    ninja
  ];

  buildInputs = [
    doxygen
    qttools
    python3
  ];

  env.QT_PLUGIN_PATH = "${qtbase}/${qtbase.qtPluginPrefix}";

  cmakeFlags = [
    "-DWITH_CORE=False"
    "-DWITH_APIDOC=True"
    "-DGENERATE_QHP=True"
    "-DWERROR=True"
    "-DWITH_DOT=False"
  ];

  buildPhase = ''
    runHook preBuild

    make -j $NIX_BUILD_CORES apidoc

    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p $out/${qgisMinorVersion}
    cp -r doc/api/html/* $out/${qgisMinorVersion}/
    cp doc/api/qch/qgis.qch $out/${qgisMinorVersion}/

    runHook postInstall
  '';

  dontWrapQtApps = true;
}
