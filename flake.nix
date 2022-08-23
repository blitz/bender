{
  description = "Bender the serial port bender";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-compat }:
    let
      bender = nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/build.nix {};
    in {
      overlays.default = import ./nix/overlay.nix;

      packages.x86_64-linux = {
        inherit bender;
        bender-debug = bender.override {
          buildType = "Debug";
        };

        default = bender;
      };

      checks.x86_64-linux = {
        bender-tests = nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/tests.nix {
          inherit bender;
        };
      };
    };
}
