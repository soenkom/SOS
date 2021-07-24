#define ENDIAN16(N) ((((UINT16)(N) & 0xFF00) >> 8) | ((UINT16)(N) << 8))
#define ENDIAN32(N) ((((UINT32)(N) & 0xFF000000) >> 24) | (((UINT32)(N) & 0x00FF0000) >> 8) | (((UINT32)(N) & 0x0000FF00) << 8) | ((UINT32)(N) << 24))

#define SET(ADDR, TYPE, VAL) *((TYPE*)(ADDR)) = (TYPE)VAL

#define IP_SET_VARSION_AND_LENGTH(PTR) SET(PTR, UINT8, ((4 << 4) | 5))
#define IP_SET_TOS(PTR, VAL) SET((UINT8*)PTR + 1, UINT8, VAL)
#define IP_SET_LENGTH(PTR, VAL) SET((UINT8*)PTR + 2, UINT16, ENDIAN16(VAL))
#define IP_SET_ID(PTR, VAL) SET((UINT8*)PTR + 4, UINT16, ENDIAN16(VAL))
#define IP_SET_OFFSET(PTR, VAL) SET((UINT8*)PTR + 6, UINT16, ENDIAN16(VAL))
#define IP_SET_TTL(PTR, VAL) SET((UINT8*)PTR + 8, UINT8, VAL)
#define IP_SET_PROTOCOL(PTR, VAL) SET((UINT8*)PTR + 9, UINT8, VAL)
#define IP_SET_CRC(PTR, VAL) SET((UINT8*)PTR + 10, UINT16, ENDIAN16(VAL))
#define IP_SET_DST(PTR, VAL) SET((UINT8*)PTR + 12, UINT32, ENDIAN32(VAL))
#define IP_SET_SRC(PTR, VAL) SET((UINT8*)PTR + 16, UINT32, ENDIAN32(VAL))

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

#define TCP_SET_SRC_PORT(PTR, VAL) SET(PTR, UINT16, ENDIAN16(VAL))
#define TCP_SET_DST_PORT(PTR, VAL) SET((UINT8*)PTR + 2, UINT16, ENDIAN16(VAL))
#define TCP_SET_ISN(PTR, VAL) SET((UINT8*)PTR + 4, UINT32, ENDIAN32(VAL))
#define TCP_SET_ACK(PTR, VAL) SET((UINT8*)PTR + 8, UINT32, ENDIAN32(VAL))
#define TCP_SET_FLAG(PTR, VAL) SET((UINT8*)PTR + 12, UINT16, ENDIAN16(VAL))
#define TCP_SET_WINDOWS(PTR, VAL) SET((UINT8*)PTR + 14, UINT16, ENDIAN16(VAL))
#define TCP_SET_CRC(PTR, VAL) SET((UINT8*)PTR + 16, UINT16, ENDIAN16(VAL))
#define TCP_SET_URG(PTR, VAL) SET((UIN8*)PTR + 18, UINT16, ENDIAN16(VAL))
#define TCP_SET_PADDING(PTR) SET((UIN8*)PTR + 20, UINT32, 0)

typedef struct {
    EFI_SIMPLE_NETWORK_PROTOCOL *snp;
    UINT8 *ip_head, *tcp_head;
} TCP_Connection;

TCP_Connect* TCP_init()
{
    TCP_Connection *ret = (TCP_Connect*)memory_alloc(sizeof(TCP_Connect));
    BS->LocateProtocol(&SimpleNetworkProtocol, NULL, (void**)&(ret->snp));
    ret->snp->Initialize();
}

VOID TCP_connect()