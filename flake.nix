{
  description = "QGIS";

  nixConfig = {
    #   extra-substituters = [ "https://example.cachix.org" ];
    #   extra-trusted-public-keys = [ "example.cachix.org-1:xxxx=" ];

    # IFD is required for qgisVersion detection in nix/unwrapped.nix.
    allow-import-from-derivation = true;
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
          qgis = pkgs.libsForQt5.callPackage ./nix/package.nix { };
          docs = pkgs.libsForQt5.callPackage ./nix/documentation.nix {
            qgisMinorVersion = "master";
          };
          default = qgis;
        };

        apps = {
          docs =
            let
              wwwLauncher = pkgs.writeShellApplication {
                name = "website";
                runtimeInputs = [ pkgs.python3 ];
                text = ''
                  exec ${pkgs.lib.getExe pkgs.python3} \
                    -m http.server 8000 \
                    -d ${self'.packages.docs}
                '';
              };
            in
            {
              type = "app";
              program = "${wwwLauncher}/bin/website";
            };
        };

        devShells.default =
          let
            nixPatches = pkgs.lib.concatStringsSep " " self'.packages.qgis.passthru.unwrapped.patches;

          in
          pkgs.mkShell {
            inputsFrom = [
              self'.packages.qgis
              self'.packages.qgis.passthru.unwrapped
            ];

            shellHook = ''
              echo "Applying Nix patches ..."
              for p in ${nixPatches}; do
                echo "patch: $p"
                patch --reverse --reject-file - --strip 1 < $p &> /dev/null || true
                patch --strip 1 < $p
              done

              export QT_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase}/${pkgs.libsForQt5.qt5.qtbase.qtPluginPrefix}"
              export QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase}/${pkgs.libsForQt5.qt5.qtbase.qtPluginPrefix}/platforms"


              function dev-help {

                echo -e "\nWelcome to a QGIS development environment !"
                echo "Build QGIS using following commands:"
                echo
                echo " 1.  mkdir build && cd build"
                echo " 2.  cmake -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=\$(pwd)/app -DWITH_QTWEBKIT=OFF -DQT_PLUGINS_DIR=${pkgs.libsForQt5.qt5.qtbase}/${pkgs.libsForQt5.qt5.qtbase.qtPluginPrefix} .."
                echo " 3.  ninja"
                echo " 4.  ninja install"
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
