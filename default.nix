{
  sources ? import ./nix/sources.nix,
  nixpkgs ? sources.nixpkgs,
  pkgs ? import nixpkgs {},
  buildType ? "Release"
}:
pkgs.callPackage ./nix/build.nix { inherit buildType; }
