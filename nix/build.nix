{ stdenv, cmake, lib, nix-gitignore, buildType }:

let gitIgnores = lib.optional (builtins.pathExists ../.gitignore) ../.gitignore;
in stdenv.mkDerivation rec {
  pname = "bender";
  version = "1.0.0-devel";

  src = nix-gitignore.gitignoreSourcePure ([''
    nix
  ''] ++ gitIgnores) ./..;

  hardeningDisable = [ "all" ];
  enableParallelBuilding = true;

  nativeBuildInputs = [ cmake ];
  cmakeBuildType = buildType;
  cmakeFlags = "-DVERSION=${version}";
}
