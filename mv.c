/*#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    // CHAR16 buffer[80];
    // UINT8 info[1024];
    // UINTN info_size = 1024;
    // UINT8 *text;
    // UINTN text_size;
    // EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    // EFI_FILE_PROTOCOL *root, *file;
    // EFI_FILE_INFO *file_info;
    initialize_services(ImageHandle, SystemTable);
    // Print(L"Filename:\r\n");
    // getline(buffer);
    // BS->LocateProtocol(&FileSystemProtocol, NULL, (VOID**)&SFSP);
    // SFSP->OpenVolume(SFSP, &root);
    // root->Open(root, &file, buffer, EFI_FILE_MODE_READ, 0);
    // file->GetInfo(file, &GenericFileInfo, &info_size, (VOID*)info);
    // file_info = (EFI_FILE_INFO*)info;
    // text_size = file_info->FileSize;
    // text = (UINT8*)memory_alloc(text_size);
    // file->Read(file, &text_size, text);
    // file->Delete(file);
    // Print(L"Destination:\r\n");
    // getline(buffer);
    // root->Open(root, &file, buffer, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    // file->Write(file, &text_size, text);
    // file->Close(file);
    // root->Close(root);
    while (TRUE) {
        Print(L"%d\r\n", (INTN)getchar());
    }
    return EFI_SUCCESS;
}*/

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
    UINT8 *text;
    UINTN text_size;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    EFI_FILE_PROTOCOL *root, *file;
    EFI_FILE_INFO *file_info;
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
    status = root->Open(root, &file, buffer, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    status = file->GetInfo(file, &GenericFileInfo, &info_size, (VOID*)info);
    if (EFI_ERROR(status)) {
        Print(L"GetInfo failed!\r\n");
        return EFI_FAIL;
    }
    file_info = (EFI_FILE_INFO*)info;
    text_size = file_info->FileSize;
    text = (UINT8*)memory_alloc(text_size);
    status = file->Read(file, &text_size, text);
    if (EFI_ERROR(status)) {
        Print(L"Read failed!\r\n");
        return EFI_FAIL;
    }
    file->Delete(file);
    Print(L"Destination: ");
    getline(buffer);
    status = root->Open(root, &file, buffer, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    status = file->Write(file, &text_size, text);
    if (EFI_ERROR(status)) {
        Print(L"Write failed!\r\n");
        return EFI_FAIL;
    }
    file->Close(file);
    root->Close(root);
    return EFI_SUCCESS;
}
