#define EFI_FAIL 127

EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP = NULL;

VOID *memory_alloc(UINTN size)
{
    VOID *ret = NULL;
    BS->AllocatePool(EfiLoaderData, size, (VOID**)ret);
    return ret;
}

VOID memory_free(VOID *ptr)
{
    BS->FreePool(ptr);
}

VOID memory_copy(VOID *dst, VOID *src, UINTN size)
{
    UINTN i;
    INT8 *psrc = (INT8*)src;
    INT8 *pdst = (INT8*)dst;
    for (i = 0; i < size; i++) {
        *pdst++ = *psrc++;
    }
}

VOID memory_set(VOID *ptr, UINT8 val, UINTN size)
{
    UINT8 *pts = (UINT8*)ptr;
    while (size-- > 0) {
        *pts++ = val;
    }
}

INTN string_compare(const CHAR16 *src, const CHAR16 *dst)
{
    INTN ret = 0;
    while ((ret = *src - *dst) == 0 && *dst) {
        ++src; ++dst;
    }
    return (((-ret) < 0) - (ret < 0));
}

INTN ascii_compare(const CHAR8 *src, const CHAR8 *dst)
{
    INTN ret = 0;
    while ((ret = *src - *dst) == 0 && *dst) {
        ++src; ++dst;
    }
    return (((-ret) < 0) - (ret < 0));
}

VOID string_copy(const CHAR16 *src, CHAR16 *dst)
{
    while (*src != L'\0') {
        *dst++ = *src++;
    }
    *dst = L'\0';
}

UINTN string_length(const CHAR16 *src)
{
    UINTN ret = 0;
    while (*src++ != L'\0') {
        ret++;
    }
    return ret;
}

VOID initialize_services(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    BS->LocateProtocol(&GraphicsOutputProtocol, NULL, (void**)&GOP);
    GOP->SetMode(GOP, GOP->Mode->MaxMode);
}

void fill_desktop(UINT32 color)
{
    UINTN i, j;
    for (i = 0; i < GOP->Mode->Info->VerticalResolution; i++) {
        for (j = 0; j < GOP->Mode->Info->HorizontalResolution; j++) {
            *((UINT32*)(GOP->Mode->FrameBufferBase + GOP->Mode->Info->PixelsPerScanLine * i * 4 + j * 4)) = color;
        }
    }
}

void fill_box(UINTN x, UINTN y, UINTN h, UINTN w, UINT32 color)
{
    UINT32 *base = (UINT32*)GOP->Mode->FrameBufferBase + GOP->Mode->Info->PixelsPerScanLine * x + y;
    UINTN i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            *(base + GOP->Mode->Info->PixelsPerScanLine * i + j) = color;
        }
    }
}

CHAR16 getchar(void)
{
    EFI_INPUT_KEY key;
    UINTN index;
    BS->WaitForEvent(1, &(ST->ConIn->WaitForKey), &index);
    ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
    if (key.UnicodeChar == L'\r') {
        Print(L"\r\n");
    } else if (key.UnicodeChar != 0) {
        Print(L"%c", key.UnicodeChar);
    }
    return key.UnicodeChar;
}

VOID getline(CHAR16* s)
{
    do {
        *s = getchar();
    } while (*s++ != L'\r');
    *--s = L'\0';
}

VOID sleep(UINT64 n)
{
    EFI_STATUS status;
    VOID *event;
    VOID *list[1];
    UINTN index;
    status = BS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &event);
    if (EFI_ERROR(status)) {
        Print(L"CreateEvent failed!\r\n");
        return;
    }
    list[0] = event;
    status = BS->SetTimer(event, TimerRelative, 10000 * n);
    if (EFI_ERROR(status)) {
        Print(L"SetTimer failed!\r\n");
        return;
    }
    status = BS->WaitForEvent(1, list, &index);
    if (EFI_ERROR(status)) {
        Print(L"WaitForEvent failed!\r\n");
        return;
    }
}