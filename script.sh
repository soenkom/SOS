#! /bin/bash

function image()
{
    dd if=/dev/zero of=Image.iso bs=512 count=262144

    parted Image.iso -s -a minimal mklabel gpt
    parted Image.iso -s -a minimal mkpart EFI FAT16 2048s 262110s
    parted Image.iso -s -a minimal toggle 1 boot
    
    dd if=/dev/zero of=part.img bs=512 count=260063
    mformat -i part.img -h 32 -t 32 -n 64 -c 1

    mkdir bin
    mkdir -p efi/boot
    cp ../x86_64/SOS/* bin
    mv bin/bootx64.efi efi/boot
    mcopy -si part.img bin ::
    mcopy -si part.img efi ::
    dd if=part.img of=Image.iso bs=512 count=260063 seek=2048 conv=notrunc
    rm part.img
}

function boot()
{
    qemu-system-x86_64 -cpu qemu64 -net none -nographic\
        -drive if=pflash,format=raw,unit=0,file=/usr/share/edk2-ovmf/OVMF_CODE.fd,readonly=on \
        -drive if=pflash,format=raw,unit=1,file=/usr/share/edk2-ovmf/OVMF_VARS.fd,readonly=on \
        -drive file=Image.img,if=ide,format=raw
}

"$@"
