# Start a VM with a PCI serial device and see whether bender manages
# to get basicperf to output to this device.
#
# Usage: expect boot.tcl <bender> <basicperf>

set timeout 10

proc abort {} {
    exit 1
}

spawn qemu-system-x86_64 -display none -vga none -m 128 -chardev stdio,id=out0 -device pci-serial,chardev=out0 \
    -kernel [lindex $argv 0] \
    -initrd [lindex $argv 1]

expect {
    timeout abort
    eof     abort

    "Bender: Hello World"
}

expect {
    timeout abort
    eof     abort

    "Exit with status 0"
}

close
exit 0

