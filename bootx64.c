#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"

BOOLEAN find_file(CHAR16* name)
{
    EFI_STATUS status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    EFI_FILE_PROTOCOL *root, *bin;
    UINT8 buffer[1024];
    UINTN buffer_size = 1024;
    EFI_FILE_INFO* file_info;
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
    status = root->Open(root, &bin, L"bin", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    while (TRUE) {
        buffer_size = 1024;
        status = bin->Read(bin, &buffer_size, (VOID*)buffer);
        if (EFI_ERROR(status)) {
            Print(L"Read failed!\r\n");
            return EFI_FAIL;
        }
        if (!buffer_size) {
            break;
        }
        file_info = (EFI_FILE_INFO*)buffer;
        if (string_compare(file_info->FileName, name) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    CHAR16 buffer[80];
    EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *DPFTP;
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DPTTP;
    CHAR16 path[1024];
    CHAR16 *bin = L"/bin/";
    CHAR16 *efi = L".efi";
    CHAR16 *ptr, *root_path_str, *str;
    UINT8 len;
    EFI_DEVICE_PATH_PROTOCOL *dev_path, *root_path;
    VOID *image;
    EFI_LOADED_IMAGE_PROTOCOL *lip;
    initialize_services(ImageHandle, SystemTable);
    status = ST->ConOut->ClearScreen(ST->ConOut);
    if (EFI_ERROR(status)) {
        Print(L"ConOut failed!\r\n");
        return EFI_FAIL;
    }
    status = BS->LocateProtocol(&DevicePathFromTextProtocol, NULL, (VOID**)&DPFTP);
    if (EFI_ERROR(status)) {
        Print(L"LocateProtocol failed!\r\n");
        return EFI_FAIL;
    }
    status = BS->LocateProtocol(&DevicePathToTextProtocol, NULL, (VOID**)&DPTTP);
    if (EFI_ERROR(status)) {
        Print(L"LocateProtocol failed!\r\n");
        return EFI_FAIL;
    }
    status = BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (VOID**)&lip, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status)) {
        Print(L"OpenProcotol failed!\r\n");
        return EFI_FAIL;
    }
    status = BS->OpenProtocol(lip->DeviceHandle, &DevicePathProtocol, (VOID**)&root_path, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status)) {
        Print(L"OpenProrocol failed!\r\n");
        return EFI_FAIL;
    }
    root_path_str = DPTTP->ConvertDevicePathToText(root_path, FALSE, FALSE);
    ptr = path;
    len = string_length(root_path_str);
    string_copy(root_path_str, ptr);
    ptr += len;
    string_copy(bin, ptr);
    ptr += 5;
    while (TRUE) {
        Print(L"SOS > ");
        getline(buffer);
        len = string_length(buffer);
        str = buffer + len;
        string_copy(efi, str);
        if (string_compare(buffer, L"exit")) {
            break;
        } else if (find_file(buffer)) {
            str = ptr;
            len = string_length(buffer);
            string_copy(buffer, str);
            dev_path = DPFTP->ConvertTextToDevicePath(path);
            status = BS->LoadImage(FALSE, ImageHandle, dev_path, NULL, 0, &image);
            if (EFI_ERROR(status)) {
                Print(L"LoadImage failed!\r\n");
                return EFI_FAIL;
            }
            status = BS->StartImage(image, NULL, NULL);
            if (EFI_ERROR(status)) {
                Print(L"StartImage failed!\r\n");
                return EFI_FAIL;
            }
        } else {
            Print(L"Unsupport command!\r\n");
        }
    }
    return EFI_SUCCESS;
}
