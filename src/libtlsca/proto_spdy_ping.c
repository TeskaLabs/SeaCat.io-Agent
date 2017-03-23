#include "tlsca.h"

///

void sc_proto_spdy_ping_build(struct ft_frame * frame, uint32_t ping_id)
{
	assert(frame != NULL);

	struct ft_vec * vec = ft_frame_append_vec(frame, SPDY_HEADER_SIZE + 4);
	assert(vec != NULL);

	uint8_t * cursor = ft_vec_ptr(vec);
	assert(cursor != NULL);

	cursor = ft_store_u16(cursor, SPDY_CNTL_FRAME_VERSION_SPD3 | 0x8000);
	cursor = ft_store_u16(cursor, SPDY_CNTL_TYPE_PING);
	cursor = ft_store_u8(cursor, 0); // Flags
	cursor = ft_store_u24(cursor, 4); // Length
	cursor = ft_store_u32(cursor, ping_id);
}