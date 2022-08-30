{
  description = "Bender the serial port bender";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
      follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, flake-compat }:
    let
      bender =
        nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/build.nix { };
    in {
      overlays.default = import ./nix/overlay.nix;

      packages.x86_64-linux = {
        inherit bender;
        bender-debug = bender.override { buildType = "Debug"; };

        default = bender;
      };

      checks.x86_64-linux = {
        bender-tests =
          nixpkgs.legacyPackages.x86_64-linux.callPackage ./nix/tests.nix {
            inherit bender;
          };
      };

      devShell.x86_64-linux = let pkgs = nixpkgs.legacyPackages.x86_64-linux;
      in pkgs.mkShell {
        inputsFrom = [ self.packages.x86_64-linux.default ];
        packages = [ pkgs.cmakeCurses pkgs.nixfmt ];

        hardeningDisable = [ "all" ];
      };
    };
}
