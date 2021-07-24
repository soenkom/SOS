#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    CHAR16 buffer[80];
    UINT8 info[1024];
    UINTN info_size = 1024;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    EFI_FILE_PROTOCOL *root, *file;
    EFI_FILE_INFO* file_info;
    initialize_services(ImageHandle, SystemTable);
    Print(L"Filename: ");
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
    status = root->Open(root, &file, buffer, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    while (TRUE) {
        info_size = 1024;
        status = file->Read(file, &info_size, (VOID*)info);
        if (EFI_ERROR(status)) {
            Print(L"Read failed!\r\n");
            return EFI_FAIL;
        }
        if (!info_size) {
            break;
        }
        file_info = (EFI_FILE_INFO*)info;
        Print(L"%s %d\r\n", file_info->FileName, file_info->FileSize);
    }
    return EFI_SUCCESS;
}
