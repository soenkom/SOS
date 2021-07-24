/*
Download file using HTTP protocol
*/

#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"
#include "efihttp.h"

struct _EFI_SERVICE_BINDING_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_SERVICE_BINDING_PROTOCOL_CREATE_CHILD) (
    IN struct _EFI_SERVICE_BINDING_PROTOCOL *This,
    IN OUT EFI_HANDLE *ChildHandle
);

typedef EFI_STATUS (EFIAPI *EFI_SERVICE_BINDING_PROTOCOL_DESTORY_CHILD) (
    IN struct _EFI_SERVICE_BINDING_PROTOCOL *This,
    IN OUT EFI_HANDLE *ChildHandle
);

typedef struct _EFI_SERVICE_BINDING_PROTOCOL {
    EFI_SERVICE_BINDING_PROTOCOL_CREATE_CHILD CreateChild;
    EFI_SERVICE_BINDING_PROTOCOL_DESTORY_CHILD DestroyChild;
} EFI_SERVICE_BINDING_PROTOCOL;

BOOLEAN request_complete;
BOOLEAN response_complete;

VOID EFIAPI request_callback(EFI_EVENT __attribute__((unused)) event, VOID __attribute__((unused)) *context)
{
    request_complete = TRUE;
}

VOID EFIAPI response_callback(EFI_EVENT __attribute__((unused)) event, VOID __attribute__((unused)) *context)
{
    response_complete = TRUE;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    CHAR16 url[1024];
    EFI_SERVICE_BINDING_PROTOCOL *SB;
    EFI_HANDLE *handle;
    EFI_HTTP_PROTOCOL *http_protocol;
    EFI_HTTP_CONFIG_DATA config_data;
    EFI_HTTPv4_ACCESS_POINT ip4_node;
    EFI_HTTP_REQUEST_DATA request_data;
    EFI_HTTP_MESSAGE request_message;
    EFI_HTTP_HEADER request_header;
    EFI_HTTP_TOKEN request_token;
    UINTN index;
    EFI_HTTP_RESPONSE_DATA response_data;
    EFI_HTTP_MESSAGE response_message;
    EFI_HTTP_TOKEN response_token;
    UINTN buffer_size = 65536;
    UINT8 *buffer;
    UINTN content_length;
    CHAR8 *str;
    UINTN downloaded;
    UINT8 *web, *ptr;
    UINTN len;
    CHAR16 *ind;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
    EFI_FILE_PROTOCOL *root, *file;
    initialize_services(ImageHandle, SystemTable);
    Print(L"URL: ");
    getline(url);
    status = BS->LocateProtocol(&EFI_HTTP_SERVICE_BINDING_PROTOCOL_GUID, NULL, (VOID**)&SB);
    if (EFI_ERROR(status)) {
        Print(L"LocateProtocol failed!\r\n");
        return EFI_FAIL;
    }
    status = SB->CreateChild(SB, (VOID**)&handle);
    if (EFI_ERROR(status)) {
        Print(L"CreateChild failed!\r\n");
        return EFI_FAIL;
    }
    status = BS->HandleProtocol(handle, &EFI_HTTP_PROTOCOL_GUID, (VOID**)&http_protocol);
    if (EFI_ERROR(status)) {
        Print(L"HandleProtocol failed!\r\n");
        return EFI_FAIL;
    }
    config_data.HttpVersion = HttpVersion11;
    config_data.TimeOutMillisec = 0;
    config_data.LocalAddressIsIPv6 = FALSE;
    memory_set((VOID*)&ip4_node, 0, sizeof(ip4_node));
    ip4_node.UseDefaultAddress = TRUE;
    config_data.AccessPoint.IPv4Node = &ip4_node;
    status = http_protocol->Configure(http_protocol, &config_data);
    if (EFI_ERROR(status)) {
        Print(L"Configure failed!\r\n");
        return EFI_FAIL;
    }
    request_data.Method = HttpMethodGet;
    request_data.Url = url;
    request_header.FieldName = (CHAR8*)"Host";
    request_header.FieldValue = (CHAR8*)"Host.localdomain";
    request_message.Data.Request = &request_data;
    request_message.HeaderCount = 1;
    request_message.Headers = &request_header;
    request_message.BodyLength = 0;
    request_message.Body = NULL;
    request_token.Event = NULL;
    status = BS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, request_callback, NULL, &request_token.Event);
    if (EFI_ERROR(status)) {
        Print(L"CreateEvent failed!\r\n");
        return EFI_FAIL;
    }
    request_token.Status = EFI_SUCCESS;
    request_token.Message = &request_message;
    request_complete = FALSE;
    status = http_protocol->Request(http_protocol, &request_token);
    if (EFI_ERROR(status)) {
        Print(L"Request failed!\r\n");
        return EFI_FAIL;
    }
    sleep(5000);
    if (!request_complete) {
        http_protocol->Cancel(http_protocol, &request_token);
        Print(L"Please try it later\r\n");
        return EFI_FAIL;
    }
    response_data.StatusCode = HTTP_STATUS_UNSUPPORTED_STATUS;
    response_message.Data.Response = &response_data;
    response_message.HeaderCount = 0;
    response_message.Headers = NULL;
    buffer = (UINT8*)memory_alloc(buffer_size);
    response_message.BodyLength = buffer_size;
    response_message.Body = buffer;
    response_token.Event = NULL;
    status = BS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, NULL, &response_token, &response_token.Event);
    if (EFI_ERROR(status)) {
        Print(L"CreateEvent failed!\r\n");
        return EFI_FAIL;
    }
    response_token.Status = EFI_SUCCESS;
    response_token.Message = &response_message;
    response_complete = FALSE;
    status = http_protocol->Response(http_protocol, &response_token);
    if (EFI_ERROR(status)) {
        Print(L"Response failed!\r\n");
        return EFI_FAIL;
    }
    sleep(5000);
    if (!response_complete) {
        http_protocol->Cancel(http_protocol, &response_token);
        return EFI_FAIL;
    }
    for (index = 0; index < response_message.HeaderCount; index++) {
        if (ascii_compare(response_message.Headers[index].FieldName, (CHAR8*)"Content-Length") == 0) {
            content_length = 0;
            str = response_message.Headers[index].FieldValue;
            while (*str != '\0') {
                content_length *= 10;
                content_length += *str - '0';
                str++;
            }
            break;
        }
    }
    web = (UINT8*)memory_alloc(content_length);
    ptr = web;
    memory_copy(ptr, response_message.Body, response_message.BodyLength);
    downloaded = response_message.BodyLength;
    ptr += response_message.BodyLength;
    while (downloaded < content_length) {
        response_message.Data.Response = NULL;
        response_message.HeaderCount = 0;
        response_message.BodyLength = buffer_size;
        memory_set(response_message.Body, 0, buffer_size);
        response_complete = FALSE;
        status = http_protocol->Response(http_protocol, &request_token);
        if (EFI_ERROR(status)) {
            Print(L"Response failed!\r\n");
            return EFI_FAIL;
        }
        sleep(5000);
        if (!response_complete) {
            http_protocol->Cancel(http_protocol, &response_token);
            return EFI_FAIL;
        }
        downloaded += response_message.BodyLength;
        memory_copy(ptr, response_message.Body, response_message.BodyLength);
        ptr += response_message.BodyLength;
    }
    len = string_length(url);
    ind = url + len - 1;
    while (ind != url && *(ind - 1) != L'/') {
        ind--;
    }
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
    status = root->Open(root, &file, ind, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(status)) {
        Print(L"Open failed!\r\n");
        return EFI_FAIL;
    }
    status = file->Write(file, &content_length, web);
    if (EFI_ERROR(status)) {
        Print(L"Write failed!\r\n");
        return EFI_FAIL;
    }
    file->Close(file);
    root->Close(root);
    return EFI_SUCCESS;
}
