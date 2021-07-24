#include <efi.h>
#include <efilib.h>
#include "util.c"
#include "font.c"

EFI_GUID IP4ServicebindingProtocolGuid = {0xc51711e7,0xb4bf,0x404a,{0xbf,0xb8,0x0a,0x04, 0x8e,0xf1,0xff,0xe4}};
EFI_GUID IP4ProtocolGuid = {0x41d94cd2,0x35b6,0x455a,{0x82,0x58,0xd4,0xe5,0x13,0x34,0xaa,0xdd}};

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

#define ENDIAN16(N) ((((UINT16)(N) & 0xFF00) >> 8) | ((UINT16)(N) << 8))

UINT16 get_crc(UINT8 *ptr, UINTN l)
{
    UINT32 sum = 0;
    UINT16* addr = (UINT16*)ptr;
    for (UINTN i = 0; i < l; i++) {
        sum += *addr;
        addr++;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum = ~sum;
    return (UINT16)ENDIAN16(sum & 0xFFFF);
}

VOID icmp_data(UINT8 *dat)
{
    UINT16 crc;
    *dat = (UINT8)8;
    *(dat + 1) = (UINT8)0;
    *(dat + 2) = (UINT8)0;
    *(dat + 3) = (UINT8)0;
    crc = get_crc(dat, 2);
    *((UINT16*)(dat + 2)) = crc;
}

BOOLEAN transmit_complete;
BOOLEAN receive_complete;

VOID EFIAPI transmit_callback(EFI_EVENT __attribute__((unused)) event, VOID __attribute__((unused)) *context)
{
    transmit_complete = TRUE;
}

VOID EFIAPI receive_callback(EFI_EVENT __attribute__((unused)) event, VOID __attribute__((unused)) *context)
{
    receive_complete = TRUE;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    EFI_IPv4_ADDRESS dest;
    EFI_SERVICE_BINDING_PROTOCOL *SB;
    EFI_HANDLE *handle;
    EFI_IP4 *ip_protocol;
    EFI_IP4_MODE_DATA mode_data;
    EFI_IP4_COMPLETION_TOKEN transmit_token, receive_token;
    EFI_IP4_TRANSMIT_DATA transmit_data;
    UINT8 buffer[80];
    UINTN index;
    CHAR16 chr;
    UINT8 digit;
    initialize_services(ImageHandle, SystemTable);
    Print(L"IP address: ");
    for (index = 0; index < 4; index++) {
        dest.Addr[index] = 0;
        while (TRUE) {
            chr = getchar();
            if (chr < L'0' || chr > L'9') {
                break;
            } else {
                digit = (UINT8)(chr - L'0');
                dest.Addr[index] *= 10;
                dest.Addr[index] += digit;
            }
        }
    }
    status = BS->LocateProtocol(&IP4ServicebindingProtocolGuid, NULL, (VOID**)&SB);
    if (EFI_ERROR(status)) {
        Print(L"LocateProtocol failed!\r\n");
        return EFI_FAIL;
    }
    status = SB->CreateChild(SB, (VOID**)&handle);
    if (EFI_ERROR(status)) {
        Print(L"CreateChild failed!\r\n");
        return EFI_FAIL;
    }
    status = BS->HandleProtocol(handle, &IP4ProtocolGuid, (VOID**)&ip_protocol);
    if (EFI_ERROR(status)) {
        Print(L"HandleProtocol failed!\r\n");
        return EFI_FAIL;
    }
    ip_protocol->Configure(ip_protocol, NULL);
    ip_protocol->GetModeData(ip_protocol, &mode_data, NULL, NULL);
    icmp_data(buffer);
    transmit_complete = FALSE;
    status = BS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, transmit_callback, NULL, &transmit_token.Event);
    if (EFI_ERROR(status)) {
        Print(L"CreateEvent failed!\r\n");
        return EFI_FAIL;
    }
    for (index = 0; index < 4; index++) {
        transmit_data.DestinationAddress.Addr[index] = dest.Addr[index];
    }
    transmit_data.OverrideData = NULL;
    transmit_data.OptionsLength = 0;
    transmit_data.OptionsBuffer = NULL;
    transmit_data.TotalDataLength = 8;
    transmit_data.FragmentCount = 0;
    transmit_token.Status = EFI_SUCCESS;
    transmit_token.Packet.TxData = &transmit_data;
    status = ip_protocol->Transmit(ip_protocol, &transmit_token);
    if (EFI_ERROR(status)) {
        Print(L"Transmit failed!\r\n");
        return EFI_FAIL;
    }
    sleep(1000);
    if (!transmit_complete) {
        Print(L"Network error!\r\n");
        return EFI_FAIL;
    }
    status = BS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, receive_callback, NULL, &receive_token.Event);
    if (EFI_ERROR(status)) {
        Print(L"CreateEvent failed!\r\n");
        return EFI_FAIL;
    }
    status = ip_protocol->Receive(ip_protocol, &receive_token);
    if (EFI_ERROR(status)) {
        Print(L"receive failed!\r\n");
        return EFI_FAIL;
    }
    sleep(1000);
    if (!receive_complete) {
        Print(L"Network error!\r\n");
        return EFI_FAIL;
    }
    Print(L"Conneted with destination!\r\n");
    return EFI_SUCCESS;
}
