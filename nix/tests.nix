{ stdenv, bender, expect, qemu, strace }:
stdenv.mkDerivation {
  pname = "bender-tests";
  version = bender.version;

  src = ../tests;

  nativeBuildInputs = [ expect qemu strace ];

  buildPhase = ''
    expect boot.tcl ${bender}/bender ${bender}/basicperf | tee output.txt
  '';

  installPhase = ''
    cp output.txt $out
  '';
}
