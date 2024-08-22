{
  description = "QGIS";

  nixConfig = {
    bash-prompt = "\\[\\033[1m\\][qgis-dev]\\[\\033\[m\\]\\040\\w >\\040";
  };

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {

      systems = [ "x86_64-linux" ];

      perSystem = { config, self', inputs', pkgs, system, ... }: {

        packages = rec {
          qgis-unwrapped = pkgs.libsForQt5.callPackage ./unwrapped.nix { };
          qgis = pkgs.callPackage ./package.nix { qgis-unwrapped = qgis-unwrapped; };
        };

        devShells.default =
          let
            pyPackages = pkgs.python311;

            py = pyPackages.override {
              self = py;
              packageOverrides = self: super: {
                pyqt5 = super.pyqt5.override {
                  withLocation = true;
                  withSerialPort = true;
                };
              };
            };

            pyqtPatch = pkgs.substituteAll {
              src = ./set-pyqt-package-dirs.patch;
              pyQt5PackageDir = "${py.pkgs.pyqt5}/${py.pkgs.python.sitePackages}";
              qsciPackageDir = "${py.pkgs.qscintilla-qt5}/${py.pkgs.python.sitePackages}";
            };

          in
          pkgs.mkShell {
            inputsFrom = [
              self'.packages.qgis-unwrapped
              self'.packages.qgis
            ];

            shellHook = ''
              patch -p1 < ${pyqtPatch}

              export QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase}/${pkgs.libsForQt5.qt5.qtbase.qtPluginPrefix}/platforms";

              function dev-help {

                echo -e "\nWelcome to a QGIS development environment !"
                echo "Build QGIS using following commands:"
                echo
                echo " 1.  cmake -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=\$(pwd)/app -DWITH_QTWEBKIT=OFF -DQT_PLUGINS_DIR=${pkgs.libsForQt5.qt5.qtbase}/${pkgs.libsForQt5.qt5.qtbase.qtPluginPrefix}"
                echo " 2.  ninja"
                echo " 3.  ninja install"
                echo
                echo "Run tests:"
                echo
                echo "1.  ninja test"
                echo
                echo "Note: run 'nix flake update' from time to time to update dependencies."
                echo
                echo "Run 'dev-help' to see this message again."
              }

              dev-help
            '';
          };
      };

      flake = { };
    };
}
