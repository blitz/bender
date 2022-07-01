# Bender

![GitHub](https://img.shields.io/github/license/blitz/bender)

Bender is a
[Multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
chainloader intended to be loaded from Grub or a similar
multiboot-capable boot loader. It tries to find PCI serial controllers
and updates the [BIOS Data
Area](https://wiki.osdev.org/BDA#BIOS_Data_Area_.28BDA.29) to make
them discoverable by legacy software.

## Building with Nix (Recommended)

Get [Nix](https://nixos.org/nix/) and type:

```sh
% nix-build
```

You can find `bender` in `result/`.

If you have a [Flakes-enabled Nix](https://nixos.wiki/wiki/Flakes),
try out `nix build` to build bender and `nix flake check` to run its
tests.

## Building without Nix

Be sure to have [CMake](https://cmake.org/install/) and a C toolchain
installed. Afterwards, you can build the source code via:

```sh
% mkdir -p build
% cd build
% cmake ..
% make
```

You will find `bender` in `standalone/` in your build directory.

# Usage

Check the documentation of your Multiboot-capable boot loader to see
how Bender is booted. Bender will discover serial ports and then boot
the next boot module as multiboot kernel.

## Parameters

Bender supports the following command-line parameters:

- `promisc`: Be more forgiving which PCI serial controller to
  accept. Specifically, this makes Bender claim any PCI device with
  "Simple Communication Controller" PCI class and not only those
  claiming to be a legacy serial controller.
- `accept_legacy`: Don't claim a PCI serial controller, if the BIOS
  already indicates that there is a legacy serial controller
  available.
- `phys_max=256M` (literally only that specific string): Prevent
  bender from relocating modules past 256M in the physical address
  space.
