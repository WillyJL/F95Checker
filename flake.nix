{
  description = "F95Checker development and runnable package";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [ self.overlays.default ];
          };
        in
        {
          default = pkgs.f95checker;
          f95checker = pkgs.f95checker;
        }
      );

      apps = forAllSystems (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [ self.overlays.default ];
          };
        in
        {
          default = {
            type = "app";
            program = "${pkgs.f95checker}/bin/f95checker";
          };
          f95checker = {
            type = "app";
            program = "${pkgs.f95checker}/bin/f95checker";
          };
        }
      );

      devShells = forAllSystems (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [ self.overlays.default ];
          };
        in
        {
          default = pkgs.mkShell {
            packages = [
              pkgs.f95checker-python
              pkgs.python312Packages.setuptools
              pkgs.direnv
              pkgs.nix-direnv
            ] ++ pkgs.f95checkerRuntimeLibraries;

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath pkgs.f95checkerRuntimeLibraries;
            PYTHONPATH = "$PWD";

            shellHook = ''
              echo "F95Checker dev shell"
              echo "Run: python main.py"
              echo "Build/run through Nix: nix build / nix run"
            '';
          };
        }
      );

      overlays.default = final: prev:
        let
          python = prev.python312;
          py = python.pkgs;

          imgui = py.buildPythonPackage rec {
            pname = "imgui";
            version = "2.0.0";
            format = "setuptools";

            src = py.fetchPypi {
              inherit pname version;
              hash = "sha256-L7247tO429fqmK+eTBxlgrC8TalColjeFjM9jGU9Z+E=";
            };

            nativeBuildInputs = with py; [
              cython_0
              setuptools
              wheel
            ];

            propagatedBuildInputs = with py; [ pyopengl ];

            pythonImportsCheck = [ "imgui" ];
          };

          bencode2 = py.buildPythonPackage rec {
            pname = "bencode2";
            version = "0.3.17";
            format = "wheel";

            src = prev.fetchurl {
              url = "https://files.pythonhosted.org/packages/8b/3e/8447261869ae15c25deccd197484d2c2af0a68f0cd9062c088fc2d40fd14/bencode2-0.3.17-py3-none-any.whl";
              hash = "sha256-iD7EEjbpaXHWMDogOrtawe/pcms5BnN52WA+5GqKfYo=";
            };

            propagatedBuildInputs = with py; [ typing-extensions ];

            pythonImportsCheck = [ "bencode2" ];
          };

          zipfile-deflate64 = py.buildPythonPackage rec {
            pname = "zipfile-deflate64";
            version = "0.2.0";
            format = "setuptools";

            src = py.fetchPypi {
              inherit pname version;
              hash = "sha256-h1oymd4QLt8cF/jK/MUoscqAti3EgUuctWhn7Fn7/Rg=";
            };

            nativeBuildInputs = with py; [
              setuptools
              wheel
            ];

            pythonImportsCheck = [ "zipfile_deflate64" ];
          };

          pillow-avif-plugin = py.buildPythonPackage {
            pname = "pillow-avif-plugin";
            version = "compat";
            format = "other";

            dontUnpack = true;

            propagatedBuildInputs = with py; [ pillow ];

            installPhase = ''
              runHook preInstall
              mkdir -p "$out/${python.sitePackages}"
              printf '%s\n' '# Compatibility shim for nixpkgs Pillow builds, which include native AVIF support.' > "$out/${python.sitePackages}/pillow_avif.py"
              runHook postInstall
            '';

            pythonImportsCheck = [ "pillow_avif" ];
          };
        in
        {
          f95checkerRuntimeLibraries = with prev; [
            libGL
            libxkbcommon
            libx11
            libxcb
            libxcb-cursor
            libxcursor
            libxi
            libxinerama
            libxrandr
            wayland
          ];

          f95checker-python = python.withPackages (
            ps: with ps; [
              aiofiles
              aiohttp
              aiohttp-socks
              aiolimiter
              aiosqlite
              beautifulsoup4
              bencode2
              certifi
              dbus-fast
              desktop-notifier
              glfw
              imgui
              lxml
              pillow
              pillow-avif-plugin
              py7zr
              pyopengl
              pyqt6
              pyqt6-webengine
              python-socks
              rarfile
              uvloop
              zipfile-deflate64
              zstd
            ]
          );

          f95checker = prev.stdenvNoCC.mkDerivation {
            pname = "f95checker";
            version = "11.1.3";

            src = prev.lib.cleanSourceWith {
              src = ./.;
              filter = path: type:
                let
                  rel = prev.lib.removePrefix (toString ./. + "/") (toString path);
                in
                !(prev.lib.hasPrefix ".git/" rel)
                && rel != ".direnv"
                && rel != "result";
            };

            nativeBuildInputs = [
              prev.makeWrapper
            ];

            installPhase = ''
              runHook preInstall

              mkdir -p "$out/share/f95checker" "$out/bin"
              cp -R . "$out/share/f95checker"
              chmod +x "$out/share/f95checker/main.py"

              makeWrapper ${final.f95checker-python}/bin/python "$out/bin/f95checker" \
                --add-flags "$out/share/f95checker/main.py" \
                --prefix LD_LIBRARY_PATH : ${prev.lib.makeLibraryPath final.f95checkerRuntimeLibraries}

              runHook postInstall
            '';

            meta = {
              description = "Update checker and library tool for F95zone games";
              homepage = "https://github.com/WillyJL/F95Checker";
              license = prev.lib.licenses.gpl3Only;
              mainProgram = "f95checker";
              platforms = [
                "x86_64-linux"
                "aarch64-linux"
              ];
            };
          };
        };
    };
}
