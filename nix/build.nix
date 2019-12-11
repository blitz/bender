{ stdenv, cmake, lib, nix-gitignore, buildType }:

let gitIgnores = lib.optional (builtins.pathExists ../.gitignore) ../.gitignore;
in stdenv.mkDerivation rec {
  pname = "morbo";
  version = "1.0.0-devel";

  src = nix-gitignore.gitignoreSourcePure ([''
    nix
  ''] ++ gitIgnores) ./..;

  hardeningDisable = [ "all" ];
  enableParallelBuilding = true;

  nativeBuildInputs = [ cmake ];
  cmakeBuildType = buildType;
  cmakeFlags = "-DVERSION=${version}";

  installPhase = ''
    mkdir -p $out
    cp standalone/bender standalone/basicperf $out/
  '';
}
