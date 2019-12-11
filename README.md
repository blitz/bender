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
