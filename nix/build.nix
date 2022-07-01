{ stdenv, cmake, lib, buildType ? "Release" }:

stdenv.mkDerivation rec {
  pname = "bender";
  version = "1.1.0-devel";

  src = ./..;

  hardeningDisable = [ "all" ];
  enableParallelBuilding = true;

  nativeBuildInputs = [ cmake ];
  cmakeBuildType = buildType;
  cmakeFlags = "-DVERSION=${version}";
}
