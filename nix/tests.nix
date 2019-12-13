{ stdenv, bender, expect, qemu, strace }:
stdenv.mkDerivation {
  pname = "bender-tests";
  version = bender.version;

  src = ../tests;

  nativeBuildInputs = [ expect qemu strace ];

  buildPhase = ''
    expect boot.tcl ${bender}/share/bender/bender ${bender}/share/bender/basicperf | tee output.txt
  '';

  installPhase = ''
    cp output.txt $out
  '';
}
