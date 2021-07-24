#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    CHAR16 buffer[80];
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    EFI_FILE_PROTOCOL *root, *file;
    initialize_services(ImageHandle, SystemTable);
    Print(L"Dirname: ");
    getline(buffer);
    status = BS->LocateProtocol(&FileSystemProtocol, NULL, (VOID**)&SFSP);
    if (EFI_ERROR(status)) {
        Print(L"LocateProtocol failed!\r\n");
        return EFI_FAIL;
    }
    status = SFSP->OpenVolume(SFSP, &root);
    if (EFI_ERROR(status)) {
        Print(L"OpenVolume failed!\r\n");
        return EFI_FAIL;
    }
    status = root->Open(root, &file, buffer, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    root->Close(root);
    return EFI_SUCCESS;
}
