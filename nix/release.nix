{
  sources ? import ./sources.nix,
  nixpkgs ? sources.nixpkgs,
  pkgs ? import nixpkgs {},
  buildType ? "Release"
}:
rec {
  bender = pkgs.callPackage ./build.nix { inherit buildType; };
  bender-tests = pkgs.callPackage ./tests.nix { inherit bender; };

  shell = pkgs.mkShell {
    inputsFrom = [ bender bender-tests];

    buildInputs = [
      pkgs.ninja
    ];

    hardeningDisable = [ "all" ];
  };
}

