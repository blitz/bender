{
  description = "Bender the serial port bender";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.11";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
    hedronSrc = {
      url = "github:cyberus-technology/hedron";
      flake = false;
    };

    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = inputs@{ self, nixpkgs, flake-compat, hedronSrc, flake-parts }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
      ];

      perSystem = { config, pkgs, ... }: {
        packages = rec {
          bender = pkgs.callPackage ./nix/build.nix { };
          bender-debug = bender.override { buildType = "Debug"; };

          default = bender;
        };

        checks =
          let
            # Hedron is not a flake. If we import its default.nix, this
            # fails, because it relies on impure evaluation. We work
            # around this issue by passing our nixpkgs along.
            hedron = (import "${hedronSrc}/nix/release.nix" {
              sources = null;
              inherit pkgs;
            }).hedron.builds.default-release;
          in
          {
            bender-tests =
              pkgs.callPackage ./nix/tests.nix {
                inherit hedron;
                bender = config.packages.bender;
              };

            bender-tests-debug =
              pkgs.callPackage ./nix/tests.nix {
                inherit hedron;

                bender = config.packages.bender-debug;
              };
          };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ config.packages.bender ];
          packages = [ pkgs.cmakeCurses pkgs.nixfmt ];

          hardeningDisable = [ "all" ];
        };
      };

      flake = {
        overlays.default = import ./nix/overlay.nix;
      };
    };
}
