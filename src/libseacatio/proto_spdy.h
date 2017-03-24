#ifndef SEACATIO__PROTO_SPDY_H_
#define SEACATIO__PROTO_SPDY_H_

#define FT_FRAME_TYPE_SPDY (0xFFFFFE01)


struct sc_proto_spdy;

///

struct sc_proto_spdy_cntl_header
{
	uint16_t version;
	uint16_t type;

	uint8_t flags;
	uint32_t length;
};

struct sc_proto_spdy_data_header
{
	uint32_t stream_id;

	uint8_t flags;
	uint32_t length;
};

///

struct sc_proto_spdy_delegate
{
	bool (*read_cntl)(struct sc_proto_spdy *, struct ft_frame * frame); // True as a return value means that the ownership of the frame has been transfered to delegate
	bool (*read_data)(struct sc_proto_spdy *, struct ft_frame * frame); // True as a return value means that the ownership of the frame has been transfered to delegate
};

struct sc_proto_spdy
{
	// Common fields
	struct sc_proto_spdy_delegate * delegate;

	// Received frame
	union
	{
		struct sc_proto_spdy_cntl_header cntl;
		struct sc_proto_spdy_data_header data;
	} received_frame;
	bool received_frame_cd; // true => cntl, false => data

	void * data;
};

bool sc_proto_spdy_init(struct sc_proto_spdy *, struct sc_proto_spdy_delegate * delegate);
void sc_proto_spdy_fini(struct sc_proto_spdy *);

// Stream delegate methods
struct ft_frame * sc_proto_spdy_stream_delegate_get_read_frame(struct ft_stream *);
bool sc_proto_spdy_stream_delegate_read(struct ft_stream *, struct ft_frame * frame);

// Datagram delegate methods
struct ft_frame * sc_proto_spdy_dgram_delegate_get_read_frame(struct ft_dgram *);
bool sc_proto_spdy_dgram_delegate_read(struct ft_dgram *, struct ft_frame * frame);

// SPDY constants
#define SPDY_HEADER_SIZE 8
#define SPDY_FLAG_FIN 0x01
#define SPDY_FLAG_UNIDIRECTIONAL 0x02
#define SPDY_FLAG_SYSTEM_PRIORITY 0x80

#define ALX1_FLAG_ACK 0x01
#define ALX1_FLAG_EXT_TRAILER 0x40
#define ALX1_FLAG_EXT_HEADER 0x20

#define ALX1_FLAG_ACK_STORED 0x01
#define ALX1_FLAG_CSR_NOT_FOUND 0x80

#define ALX1_FLAG_CSR_STORED 0x01
#define ALX1_FLAG_CSR_APPROVED 0x02
#define ALX1_FLAG_CSR_REJECTED 0x04

#define SPDY_CNTL_FRAME_VERSION_SPD3 0x03
#define SPDY_CNTL_FRAME_VERSION_ALX1 0xA1

#define SPDY_CNTL_TYPE_SYN_STREAM 1
#define SPDY_CNTL_TYPE_SYN_REPLY 2
#define SPDY_CNTL_TYPE_RST_STREAM 3
#define SPDY_CNTL_TYPE_PING 6

#define SPDY_CNTL_TYPE_CSR 0xC1
#define SPDY_CNTL_TYPE_CERT_QUERY 0xC2
#define SPDY_CNTL_TYPE_CERT 0xC3
#define SPDY_CNTL_TYPE_CSR_REPLY 0xC4

#define SPDY_CNTL_FRAME_VERSION_ALX1_BUS 0xB1
#define ALX1_EXT_TYPE_BUS_CONN_AUTH 102
#define ALX1_EXT_FLAG_CLIENT_ID 0x01

#define ALX1_EXT_TYPE_BUS_RPC_CALL 150
#define ALX1_EXT_TYPE_BUS_RPC_REPLY 151

// Client registration
#define SPDY_CNTL_FRAME_VERSION_ALX1_CRG 0xBC
#define SPDY_CNTL_TYPE_CRG_CLIENT_REGISTER 0x00
#define SPDY_CNTL_TYPE_CRG_CLIENT_UNREGISTER 0x01
#define SPDY_CNTL_TYPE_CRG_CLIENT_KEEPALIVE 0x02
#define SPDY_CNTL_TYPE_CRG_CLIENT_GET_LOCALADDRESS 0x03
#define SPDY_CNTL_TYPE_CRG_CLIENT_GET_LOCALADDRESS_REPLY 0x04

#define SPDY_CNTL_FRAME_VERSION_ALX1_SOCKETS 0xB2
#define SPDY_CNTL_TYPE_SOCKETS_REQUEST 0x01
#define SPDY_CNTL_TYPE_SOCKETS_REPLY 0x02

#define SPDY_RST_STATUS_PROTOCOL_ERROR 1
#define SPDY_RST_STATUS_INVALID_STREAM 2
#define SPDY_RST_STATUS_REFUSED_STREAM 3
#define SPDY_RST_STATUS_INTERNAL_ERROR 6
#define SPDY_RST_STATUS_STREAM_ALREADY_CLOSED 9
#define ALX1_RST_STATUS_CONNECTION_TIMEOUT 71
#define ALX1_RST_STATUS_RESPONSE_TIMEOUT 72

#define SPDY_CNTL_FRAME_VERSION_ALX1_STORAGE 0xB3
#define SPDY_CNTL_TYPE_STORAGE_CLIENT_ESTABLISHED 0x01
#define SPDY_CNTL_TYPE_STORAGE_CLIENT_CLOSED 0x02

#define SPDY_CNTL_FRAME_VERSION_ALX1_LOG 0xB4
#define SPDY_CNTL_TYPE_LOG_BLOCK 0x01

#define SPDY_CNTL_FRAME_VERSION_ALX1_TELEMETRY 0xB5
#define SPDY_CNTL_TYPE_TELEMETRY_CAPABILITIES 0x02
#define SPDY_CNTL_TYPE_TELEMETRY_CAPABILITIES_ACK 0x03


// Following switch statement is used to convinietly dispatch SPDY frames
/* Example:

	switch (sc_proto_spdy_vt(X->spdy.received_frame.cntl))
	{
		case SPDY_VT(SPDY_CNTL_FRAME_VERSION_SPD3, SPDY_CNTL_TYPE_PING):
			....
			break;

		...

		default:
			FT_WARN("Not found!");
	}
*/

#define SPDY_VT(v,t) ((v << 16) | t)
#define SPDY_VT_DATA (0xFFFFFFFF)

static inline uint32_t sc_proto_spdy_vt_ext(uint16_t version, uint16_t type)
{
	uint32_t vt = version;
	vt <<= 16;
	vt |= type;
	return vt;
}

static inline uint32_t sc_proto_spdy_vt(struct sc_proto_spdy * spdy)
{
	if (spdy->received_frame_cd)
	{
		return sc_proto_spdy_vt_ext(spdy->received_frame.cntl.version, spdy->received_frame.cntl.type);
	}
	else return SPDY_VT_DATA;
}

// Add a couple of useful functions to FT load/store

static inline uint8_t * sc_ft_store_vlen(uint8_t * cursor, const char * str, size_t str_len)
{
	if (str_len < 0xFA)
	{
		cursor = ft_store_u8(cursor, str_len);
	}
	else
	{
		cursor = ft_store_u8(cursor, 0xFF);
		cursor = ft_store_u16(cursor, str_len);
	}

	if (str_len > 0) cursor = ft_store_bytes(cursor, str, str_len);
	return cursor;
}

static inline uint8_t * sc_ft_store_vle(uint8_t * cursor, const char * str)
{
	size_t str_len = strlen(str);
	return sc_ft_store_vlen(cursor, str, str_len);
}

// VLE string that is NULL terminated
static inline uint8_t * sc_ft_store_vle_nt(uint8_t * cursor, const char * str)
{
	size_t str_len = strlen(str);
	return sc_ft_store_vlen(cursor, str, str_len + 1);
}

// VLE string loader
static inline uint8_t * sc_ft_load_vle(uint8_t * cursor, char * value, size_t max_value_len)
{
	size_t size;
	uint8_t size8;
	cursor = ft_load_u8(cursor, &size8);

	if (size8 < 0xFA)
	{
		size = size8;
	} else {
		uint16_t size16;
		cursor = ft_load_u16(cursor, &size16);
		size = size8;
	}

	size_t to_copy = size;
	if (to_copy > (max_value_len - 1))  to_copy = max_value_len - 1;

	if (to_copy > 0) memcpy(value, cursor, to_copy);
	value[to_copy] = '\0';

	return cursor + size;
}

#endif //SEACATIO__PROTO_SPDY_H_
