{
  description = "Bender the serial port bender";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
    hedronSrc = {
      url = "github:cyberus-technology/hedron";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-compat, hedronSrc }:
    let
      bender =
        nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/build.nix { };
      bender-debug = bender.override { buildType = "Debug"; };
    in {
      overlays.default = import ./nix/overlay.nix;

      packages.x86_64-linux = {
        inherit bender bender-debug;

        default = bender;
      };

      checks.x86_64-linux = let
        # Hedron is not a flake. If we import its default.nix, this
        # fails, because it relies on impure evaluation. We work
        # around this issue by passing our nixpkgs along.
        hedron = (import "${hedronSrc}/nix/release.nix" {
          sources = null;
          pkgs = nixpkgs.legacyPackages.x86_64-linux;
        }).hedron.default-release;
      in {
        bender-tests =
          nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/tests.nix {
            inherit bender hedron;
          };

        bender-tests-debug =
          nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/tests.nix {
            inherit hedron;

            bender = bender-debug;
          };
      };

      devShells.x86_64-linux.default =
        let pkgs = nixpkgs.legacyPackages.x86_64-linux;
        in pkgs.mkShell {
          inputsFrom = [ self.packages.x86_64-linux.default ];
          packages = [ pkgs.cmakeCurses pkgs.nixfmt ];

          hardeningDisable = [ "all" ];
        };
    };
}
