{
  sources ? import ./nix/sources.nix,
  nixpkgs ? sources.nixpkgs,
  pkgs ? import nixpkgs {}
}:
let
  bender = import ./default.nix { inherit sources nixpkgs pkgs; };
in
pkgs.mkShell {
  inputsFrom = [ bender ];

  buildInputs = [
    pkgs.ninja
    pkgs.qemu
  ];

  hardeningDisable = [ "all" ];
}
