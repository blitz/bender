{ runCommand, bender, expect, qemu, strace }:
runCommand "bender-tests" {
  nativeBuildInputs = [ expect qemu strace  ];
} ''
  mkdir -p $out
  expect ${../tests/boot.tcl} ${bender}/share/bender/bender ${bender}/share/bender/basicperf | tee $out/mb1-boot.log
''
