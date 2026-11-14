
#ifndef GBA_PROTOCOL_H
#define GBA_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Protocol configuration
// -----------------------------------------------------------------------------

#define PROTOCOL_VERSION       1
#define PROTOCOL_MAGIC         0x5AA5
#define PROTOCOL_MAX_PAYLOAD   256


// -----------------------------------------------------------------------------
// Opcodes
// -----------------------------------------------------------------------------

typedef enum {
    NONE        = 0x00,
    VIDEO_FRAME = 0x01,
    AUDIO_FRAME = 0x02,
    COMMAND     = 0x03,
} PacketType;

// -----------------------------------------------------------------------------
// Packet framing
//
// Header (4 bytes):
//   opcode      uint8_t
//   sequence    uint8_t
//   length      uint16_t
//
// All multibyte fields are little-endian.
// -----------------------------------------------------------------------------

typedef struct {
    PacketType opcode;
    uint8_t sequence;
    uint16_t length;
} ProtocolHeader;

#define PROTOCOL_HEADER_SIZE   sizeof(ProtocolHeader)

typedef enum {
    PACKET_FLAG_RESPONSE = (1 << 0),
    PACKET_FLAG_ERROR    = (1 << 1),
    PACKET_FLAG_EVENT     = (1 << 2),
} PacketFlags;

typedef enum {

    // System: 0x01 - 0x0F
    CMD_HELLO              = 0x01,
    CMD_GET_INFO           = 0x02,
    CMD_GET_STATUS         = 0x03,
    CMD_RESET              = 0x04,
    CMD_PING               = 0x05,

    // Filesystem / menu: 0x10 - 0x1F
    CMD_LIST_DIR           = 0x10,
    CMD_GET_ENTRY          = 0x11,
    CMD_CHANGE_DIR         = 0x12,
    CMD_GO_PARENT          = 0x13,
    CMD_OPEN_FILE          = 0x14,
    CMD_CLOSE_FILE         = 0x15,
    CMD_GET_FILE_INFO      = 0x16,

    // Playback: 0x20 - 0x2F
    CMD_PLAY               = 0x20,
    CMD_PAUSE              = 0x21,
    CMD_RESUME             = 0x22,
    CMD_STOP               = 0x23,
    CMD_SEEK_ABS           = 0x24,
    CMD_SEEK_REL           = 0x25,
    CMD_SET_VOLUME         = 0x26,
    CMD_GET_PLAYBACK_INFO  = 0x27,
    CMD_NEXT               = 0x28,
    CMD_PREVIOUS           = 0x29,

    // Events: 0x30 - 0x3F
    CMD_GET_EVENTS         = 0x30,

} ProtocolOpcode;

// -----------------------------------------------------------------------------
// Response status codes
// -----------------------------------------------------------------------------

typedef enum {
    STATUS_OK              = 0x00,
    STATUS_INVALID_COMMAND = 0x01,
    STATUS_INVALID_ARGUMENT= 0x02,
    STATUS_NOT_FOUND       = 0x03,
    STATUS_NOT_READY       = 0x04,
    STATUS_BUSY            = 0x05,
    STATUS_IO_ERROR        = 0x06,
    STATUS_UNSUPPORTED     = 0x07,
    STATUS_INTERNAL_ERROR  = 0x08,
} ProtocolStatus;

// -----------------------------------------------------------------------------
// Device information
// -----------------------------------------------------------------------------

typedef struct {
    uint8_t protocol_version;
    uint8_t firmware_major;
    uint8_t firmware_minor;
    uint8_t firmware_patch;
    uint32_t capabilities;
} DeviceInfo;

typedef enum {
    CAP_FILESYSTEM = (1 << 0),
    CAP_VIDEO      = (1 << 1),
    CAP_AUDIO      = (1 << 2),
    CAP_SEEK       = (1 << 3),
    CAP_VOLUME     = (1 << 4),
} DeviceCapabilities;

// -----------------------------------------------------------------------------
// Filesystem / menu
// -----------------------------------------------------------------------------

typedef enum {
    ENTRY_TYPE_FILE = 0,
    ENTRY_TYPE_DIRECTORY = 1,
} EntryType;

typedef struct {
    uint16_t offset;
    uint8_t count;
} ListDirRequest;

typedef struct {
    uint16_t total_entries;
    uint8_t returned_entries;
    // Followed by DirectoryEntry records.
} ListDirResponse;

typedef struct {
    uint32_t entry_id;
    uint8_t type;
    uint8_t flags;
    uint8_t name_length;
    // Followed by name_length bytes.
} DirectoryEntry;

typedef struct {
    uint32_t entry_id;
} EntryRequest;

typedef struct {
    uint32_t entry_id;
} OpenFileRequest;

// -----------------------------------------------------------------------------
// Playback
// -----------------------------------------------------------------------------

typedef enum {
    PLAYBACK_STOPPED = 0,
    PLAYBACK_OPENING,
    PLAYBACK_PLAYING,
    PLAYBACK_PAUSED,
    PLAYBACK_SEEKING,
    PLAYBACK_BUFFERING,
    PLAYBACK_ERROR,
} PlaybackState;

typedef struct {
    uint32_t timestamp_ms;
} SeekAbsRequest;

typedef struct {
    int32_t delta_ms;
} SeekRelRequest;

typedef struct {
    uint8_t volume;  // 0-100
} SetVolumeRequest;

typedef struct {
    uint8_t state;
    uint32_t position_ms;
    uint32_t duration_ms;
    uint8_t volume;
    uint8_t flags;
} PlaybackInfo;

// -----------------------------------------------------------------------------
// Asynchronous events
// -----------------------------------------------------------------------------

typedef enum {
    EVENT_PLAYBACK_FINISHED = 0x01,
    EVENT_PLAYBACK_ERROR    = 0x02,
    EVENT_BUFFER_UNDERRUN   = 0x03,
    EVENT_STORAGE_DISCONNECTED = 0x04,
} ProtocolEvent;

typedef struct {
    uint8_t event_type;
    uint8_t data_length;
    // Followed by data_length bytes.
} EventHeader;

// -----------------------------------------------------------------------------
// Common response payload
// -----------------------------------------------------------------------------

typedef struct {
    uint8_t status;
    // Followed by command-specific response data.
} ProtocolResponse;

// -----------------------------------------------------------------------------
// CRC
// -----------------------------------------------------------------------------

#define PROTOCOL_CRC16_POLYNOMIAL 0x1021
#define PROTOCOL_CRC16_INITIAL    0xFFFF

#ifdef __cplusplus
}
#endif

#endif // GBA_PROTOCOL_H
