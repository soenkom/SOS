A Simple Operating System based on UEFI.

## Dependency

- gcc 10.2.0

- mtools 4.0.24

- qemu 5.2.0

## Build

Download gnu-efi

```console
git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
```

Download SOS

```console
cd gnu-efi
git clone https://github.com/soenkom/SOS.git SOS
```

Modify the makefile script

```console
sed -i '/SUBDIRS = */ s/$/ SOS/' Makefile 
```

Make
```console
make all
```

All the compiled UEFI App will be placed in x86_64/SOS, to make the image file

```console
cd SOS
bash script.sh image
```

Boot the OS this QEMU

```console
bash script.sh boot
```
