# Start a VM with a PCI serial device and see whether bender manages
# to boot the Hedron hypervisor.
#
# Usage: expect boot-hedron.tcl <bender> <hedron>

set timeout 10

proc abort {} {
    exit 1
}

spawn qemu-system-x86_64 -display none -vga none -cpu IvyBridge -m 512 -chardev stdio,id=out0 -device pci-serial,chardev=out0 \
    -kernel [lindex $argv 0] \
    -initrd "[lindex $argv 1] serial"

expect_before {
    timeout abort
    eof     abort
}

# Hedron is a Multiboot2 binary (in addition to Multiboot1) and should
# be booted as such.
expect "Found MBI2 header. Booting as MBI2."

# Hedron should take it's Multiboot2 path. If this works, Hedron also
# saw the command line parameter (which enables serial output) and
# discovered the right serial port.
expect -re "Hedron Hypervisor .*MB2"

# Hedron "exits" gracefully after its boot process, because there is
# no roottask.
expect "No ELF"

close
exit 0

