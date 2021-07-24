#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    CHAR16 buffer[80];
    CHAR16 *text, *next;
    UINTN text_size = 65536;
    UINTN ind = 0;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    EFI_FILE_PROTOCOL *root, *file;
    initialize_services(ImageHandle, SystemTable);
    Print(L"Filename: ");
    getline(buffer);
    Print(L"Input text(ESC to quit):\r\n");
    text = (CHAR16*)memory_alloc((text_size + 5) * sizeof(CHAR16));
    while (TRUE) {
        text[ind++] = getchar();
        if (text[ind - 1] == L'\r') {
            text[ind++] = L'\n';
        }
        if (text[ind - 1] == 0) {
            break;
        }
        if (ind > text_size) {
            text_size *= 2;
            next = (CHAR16*)memory_alloc((text_size + 5) * sizeof(CHAR16));
            memory_copy(next, text, ind * sizeof(CHAR16));
            memory_free(text);
            text = next;
            next = NULL;
        }
    }
    ind *= sizeof(CHAR16);
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
    status = root->Open(root, &file, buffer, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    status = file->Write(file, &ind, (VOID*)text);
    if (EFI_ERROR(status)) {
        Print(L"Write failed!\r\n");
        return EFI_FAIL;
    }
    file->Close(file);
    root->Close(root);
    return EFI_SUCCESS;
}
