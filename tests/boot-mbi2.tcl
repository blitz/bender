# Start a VM with a PCI serial device and see whether bender manages
# to boot a minimal Multiboot2 program and constructs a sane
# Multiboot2 boot information.
#
# Usage: expect boot.tcl <bender> <mbi2_example>

set timeout 10

proc abort {} {
    exit 1
}

spawn qemu-system-x86_64 -display none -vga none -m 128 -chardev stdio,id=out0 -device pci-serial,chardev=out0 \
    -kernel [lindex $argv 0] \
    -initrd "[lindex $argv 1] hello_world,[lindex $argv 1] a_module"

expect_before {
    timeout abort
    eof     abort
}

expect "Found MBI2 header. Booting as MBI2."

# The following tests the content of the Multiboot2 info as it is
# dumped by mbi2_example. The order in which entries are printed is
# not necessarily important, but to keep things simple, we expect it
# in the way we build it.

# The command line is properly communicated.
expect -re "Tag 01: boot cmdline='.* hello_world'"

# We see at least a single entry in the memory map.
expect -re "Tag 06: memory map"
expect -re "type=01 addr=0 "

# We get a module with the correct command line.
expect -re "Tag 03: module cmdline='.* a_module'"

expect "Exit with status 0"

close
exit 0

