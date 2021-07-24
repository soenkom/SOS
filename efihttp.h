#ifndef _EFI_HTTP_H
#define _EFI_HTTP_H

EFI_GUID EFI_HTTP_SERVICE_BINDING_PROTOCOL_GUID = {0xbdc8e6af,0xd9bc,0x4379,{0xa7,0x2a,0xe0,0xc4,0xe7,0x5d,0xae,0x1c}};

EFI_GUID EFI_HTTP_PROTOCOL_GUID = {0x7A59B29B,0x910B,0x4171,{0x82,0x42,0xA8,0x5A,0x0D,0xF2,0x5B,0x5B}};

INTERFACE_DECL(_EFI_HTTP_PROTOCOL);

typedef struct
{
    CHAR8 *FieldName;
    CHAR8 *FieldValue;
} EFI_HTTP_HEADER;

typedef enum
{
    HTTP_STATUS_UNSUPPORTED_STATUS = 0,
    HTTP_STATUS_100_CONTINUE,
    HTTP_STATUS_101_SWITCHING_PROTOCOLS,
    HTTP_STATUS_200_OK,
    HTTP_STATUS_201_CREATED,
    HTTP_STATUS_202_ACCEPTED,
    HTTP_STATUS_203_NON_AUTHORITATIVE_INFORMATION,
    HTTP_STATUS_204_NO_CONTENT,
    HTTP_STATUS_205_RESET_CONTENT,
    HTTP_STATUS_206_PARTIAL_CONTENT,
    HTTP_STATUS_300_MULTIPLE_CHOICES,
    HTTP_STATUS_301_MOVED_PERMANENTLY,
    HTTP_STATUS_302_FOUND,
    HTTP_STATUS_303_SEE_OTHER,
    HTTP_STATUS_304_NOT_MODIFIED,
    HTTP_STATUS_305_USE_PROXY,
    HTTP_STATUS_307_TEMPORARY_REDIRECT,
    HTTP_STATUS_400_BAD_REQUEST,
    HTTP_STATUS_401_UNAUTHORIZED,
    HTTP_STATUS_402_PAYMENT_REQUIRED,
    HTTP_STATUS_403_FORBIDDEN,
    HTTP_STATUS_404_NOT_FOUND,
    HTTP_STATUS_405_METHOD_NOT_ALLOWED,
    HTTP_STATUS_406_NOT_ACCEPTABLE,
    HTTP_STATUS_407_PROXY_AUTHENTICATION_REQUIRED,
    HTTP_STATUS_408_REQUEST_TIME_OUT,
    HTTP_STATUS_409_CONFLICT,
    HTTP_STATUS_410_GONE,
    HTTP_STATUS_411_LENGTH_REQUIRED,
    HTTP_STATUS_412_PRECONDITION_FAILED,
    HTTP_STATUS_413_REQUEST_ENTITY_TOO_LARGE,
    HTTP_STATUS_414_REQUEST_URI_TOO_LARGE,
    HTTP_STATUS_415_UNSUPPORTED_MEDIA_TYPE,
    HTTP_STATUS_416_REQUESTED_RANGE_NOT_SATISFIED,
    HTTP_STATUS_417_EXPECTATION_FAILED,
    HTTP_STATUS_500_INTERNAL_SERVER_ERROR,
    HTTP_STATUS_501_NOT_IMPLEMENTED,
    HTTP_STATUS_502_BAD_GATEWAY,
    HTTP_STATUS_503_SERVICE_UNAVAILABLE,
    HTTP_STATUS_504_GATEWAY_TIME_OUT,
    HTTP_STATUS_505_HTTP_VERSION_NOT_SUPPORTED,
    HTTP_STATUS_308_PERMANENT_REDIRECT
} EFI_HTTP_STATUS_CODE;

typedef enum
{
    HttpMethodGet,
    HttpMethodPost,
    HttpMethodPatch,
    HttpMethodOptions,
    HttpMethodConnect,
    HttpMethodHead,
    HttpMethodPut,
    HttpMethodDelete,
    HttpMethodTrace,
    HttpMethodMax
} EFI_HTTP_METHOD;

//******************************************
// EFI_HTTP_VERSION
//******************************************
typedef enum
{
    HttpVersion10,
    HttpVersion11,
    HttpVersionUnsupported
} EFI_HTTP_VERSION;

//******************************************
// EFI_HTTPv4_ACCESS_POINT
//******************************************
typedef struct
{
    BOOLEAN UseDefaultAddress;
    EFI_IPv4_ADDRESS LocalAddress;
    EFI_IPv4_ADDRESS LocalSubnet;
    UINT16 LocalPort;
} EFI_HTTPv4_ACCESS_POINT;

typedef struct
{
    EFI_IPv6_ADDRESS LocalAddress;
    UINT16 LocalPort;
} EFI_HTTPv6_ACCESS_POINT;

typedef struct
{
    EFI_HTTP_VERSION HttpVersion;
    UINT32 TimeOutMillisec;
    BOOLEAN LocalAddressIsIPv6;
    union
    {
        EFI_HTTPv4_ACCESS_POINT *IPv4Node;
        EFI_HTTPv6_ACCESS_POINT *IPv6Node;
    } AccessPoint;
} EFI_HTTP_CONFIG_DATA;

typedef struct
{
    EFI_HTTP_METHOD Method;
    CHAR16 *Url;
} EFI_HTTP_REQUEST_DATA;

typedef struct
{
    EFI_HTTP_STATUS_CODE StatusCode;
} EFI_HTTP_RESPONSE_DATA;

typedef struct
{
    union
    {
        EFI_HTTP_REQUEST_DATA *Request;
        EFI_HTTP_RESPONSE_DATA *Response;
    } Data;
    UINTN HeaderCount;
    EFI_HTTP_HEADER *Headers;
    UINTN BodyLength;
    VOID *Body;
} EFI_HTTP_MESSAGE;

typedef struct
{
    EFI_EVENT Event;
    EFI_STATUS Status;
    EFI_HTTP_MESSAGE *Message;
} EFI_HTTP_TOKEN;

typedef EFI_STATUS (EFIAPI *EFI_HTTP_GET_MODE_DATA)(
    IN struct _EFI_HTTP_PROTOCOL *This,
    OUT EFI_HTTP_CONFIG_DATA *HttpConfigData);

typedef EFI_STATUS (EFIAPI *EFI_HTTP_CONFIGURE)(
    IN struct _EFI_HTTP_PROTOCOL *This,
    IN EFI_HTTP_CONFIG_DATA *HttpConfigData OPTIONAL);

typedef EFI_STATUS (EFIAPI *EFI_HTTP_REQUEST)(
    IN struct _EFI_HTTP_PROTOCOL *This,
    IN EFI_HTTP_TOKEN *Token);

typedef EFI_STATUS (EFIAPI *EFI_HTTP_CANCEL)(
    IN struct _EFI_HTTP_PROTOCOL *This,
    IN EFI_HTTP_TOKEN *Token);

typedef EFI_STATUS (EFIAPI *EFI_HTTP_RESPONSE)(
    IN struct _EFI_HTTP_PROTOCOL *This,
    IN EFI_HTTP_TOKEN *Token);

typedef EFI_STATUS (EFIAPI *EFI_HTTP_POLL)(
    IN struct _EFI_HTTP_PROTOCOL *This);

typedef struct _EFI_HTTP_PROTOCOL
{
    EFI_HTTP_GET_MODE_DATA GetModeData;
    EFI_HTTP_CONFIGURE Configure;
    EFI_HTTP_REQUEST Request;
    EFI_HTTP_CANCEL Cancel;
    EFI_HTTP_RESPONSE Response;
    EFI_HTTP_POLL Poll;
} EFI_HTTP_PROTOCOL;

#endif