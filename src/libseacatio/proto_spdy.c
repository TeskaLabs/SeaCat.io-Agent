#include "seacatio.h"

///

#define MAX_SPDY_FRAME_LENGTH (16*1024)

////

bool sc_proto_spdy_init(struct sc_proto_spdy * this, struct sc_proto_spdy_delegate * delegate)
{
	ASSERT_THIS();
	assert(delegate != NULL);

	this->delegate = delegate;
	this->data = NULL;

	return true;
}

void sc_proto_spdy_fini(struct sc_proto_spdy * this)
{
	ASSERT_THIS();
}


static bool sc_proto_spdy_frame_received(struct sc_proto_spdy * this, struct ft_frame * frame)
{
	// Consolidate frame vectors if needed (header and body are received in two chunks)
	if (frame->vec_limit == 2)
	{
		struct ft_vec * vec_header = (struct ft_vec *)(frame->data + frame->capacity);
		vec_header -= 1;

		struct ft_vec * vec_body = vec_header - 1;

		assert(vec_body->offset == SPDY_HEADER_SIZE);

		vec_header->position += vec_body->position;
		vec_header->limit += vec_body->limit;
		vec_header->capacity += vec_body->capacity;
		frame->vec_limit -= 1;
	}

	assert(frame->vec_limit == 1);
	ft_frame_flip(frame);

	// Check minimal frame length
	assert(ft_frame_get_vec(frame)->limit >= SPDY_HEADER_SIZE);

	// Parse frame
	uint8_t * data = ft_vec_ptr(ft_frame_get_vec(frame));
	uint8_t * cursor = data;
	this->received_frame_cd = ((data[0] & 0b10000000) == 0b10000000);

	bool frame_disowned = false;

	if (this->received_frame_cd)
	{
		// Control frame
		cursor = ft_load_u16(cursor, &this->received_frame.cntl.version); // Version(15 bits)
		this->received_frame.cntl.version &= 0x7FFF;

		cursor = ft_load_u16(cursor, &this->received_frame.cntl.type); // Type(16 bits)
		cursor = ft_load_u8(cursor, &this->received_frame.cntl.flags); // Flags (8 bits)
		cursor = ft_load_u24(cursor, &this->received_frame.cntl.length); // Length (24 bits)

		//TODO: Check sanity of all fields

		if (this->delegate->read_cntl != NULL)
			frame_disowned = this->delegate->read_cntl(this, frame);
	}
	else
	{
		// Data frame
		cursor = ft_load_u32(cursor, &this->received_frame.data.stream_id); // Stream-ID (32 bits)
		cursor = ft_load_u8(cursor, &this->received_frame.data.flags); // Flags (8 bits)
		cursor = ft_load_u24(cursor, &this->received_frame.data.length); // Length (24 bits)

		//TODO: Check sanity of all fields

		if (this->delegate->read_data != NULL)
			frame_disowned = this->delegate->read_data(this, frame);
	}

	if (!frame_disowned) ft_frame_return(frame);

	return true;
}


// Stream delegate methods
struct ft_frame * sc_proto_spdy_stream_delegate_get_read_frame(struct ft_stream * stream)
{
	struct sc_proto_spdy * this = (struct sc_proto_spdy *) stream->base.socket.protocol;
	ASSERT_THIS();

	struct ft_frame * frame = ft_pool_borrow(&stream->base.socket.context->frame_pool, FT_FRAME_TYPE_RAW_DATA);
	if (frame == NULL) return NULL;

	ft_frame_format_empty(frame);
	ft_frame_create_vec(frame, 0, SPDY_HEADER_SIZE); // For SPDY header

	frame->type = FT_FRAME_TYPE_SPDY;

	return frame;
}


bool sc_proto_spdy_stream_delegate_read(struct ft_stream * stream, struct ft_frame * frame)
{
	struct sc_proto_spdy * this = (struct sc_proto_spdy *) stream->base.socket.protocol;
	ASSERT_THIS();

	// Shutdown a connnection when peer does that first
	if (frame->type == FT_FRAME_TYPE_END_OF_STREAM)
	{
		ft_stream_write(stream, frame);
		return true;
	}

	else if (frame->type == FT_FRAME_TYPE_SPDY)
	{
		if (frame->vec_limit == 1)
		{
			if (frame->vec_position == 0) return false; // Not a complete read
			
			// SPDY header received -> read a whole frame length
			struct ft_vec * vec = (struct ft_vec *)(frame->data + frame->capacity);
			vec -= 1;

			assert(vec->limit == SPDY_HEADER_SIZE);
			if (vec->position < SPDY_HEADER_SIZE)
			{
				FT_ERROR("Incoming frame is incomplete (%d)", vec->position);
				ft_frame_return(frame);

				ft_stream_cntl(stream, FT_STREAM_WRITE_SHUTDOWN | FT_STREAM_READ_STOP);
				return true;
			}

			uint8_t * cursor = vec->frame->data + vec->offset;
			cursor = ft_skip_bytes(cursor, 5);
			uint32_t length;
			cursor = ft_load_u24(cursor, &length);

			if (length == 0) return sc_proto_spdy_frame_received(this, frame);

			vec = ft_frame_create_vec(frame, SPDY_HEADER_SIZE, length); // For SPDY frame body
			if ((vec == NULL) || (length > MAX_SPDY_FRAME_LENGTH))
			{
				FT_ERROR("Incoming frame is too long (%lu > %d)", (unsigned long)length, MAX_SPDY_FRAME_LENGTH);
				ft_frame_return(frame);

				ft_stream_cntl(stream, FT_STREAM_WRITE_SHUTDOWN | FT_STREAM_READ_STOP);
				return true;
			}

			return false;
		}

		return sc_proto_spdy_frame_received(this, frame);
	}

	FT_FATAL("Unknown/invalid frame received (type: %x)", frame->type);
	abort();
}


// Datagram delegate methods
struct ft_frame * sc_proto_spdy_dgram_delegate_get_read_frame(struct ft_dgram * dgram)
{
	struct sc_proto_spdy * this = (struct sc_proto_spdy *) dgram->base.socket.protocol;
	ASSERT_THIS();

	struct ft_frame * frame = ft_pool_borrow(&dgram->base.socket.context->frame_pool, FT_FRAME_TYPE_RAW_DATA);
	if (frame == NULL) return NULL;

	ft_frame_format_simple(frame);
	frame->type = FT_FRAME_TYPE_SPDY;

	return frame;
}


bool sc_proto_spdy_dgram_delegate_read(struct ft_dgram * dgram, struct ft_frame * frame)
{
	struct sc_proto_spdy * this = (struct sc_proto_spdy *) dgram->base.socket.protocol;
	ASSERT_THIS();

	// Shutdown a connnection when peer does that first
	if (frame->type == FT_FRAME_TYPE_END_OF_STREAM)
	{
		ft_dgram_write(dgram, frame);
		return true;
	}

	else if (frame->type == FT_FRAME_TYPE_SPDY)
	{
		assert(frame->vec_position == 1);
		assert(frame->vec_limit == 1);
		
		struct ft_vec * vec = (struct ft_vec *)(frame->data + frame->capacity);
		vec -= 1;

		if (vec->position < SPDY_HEADER_SIZE)
		{
			FT_ERROR("Incoming frame is incomplete (%d)", vec->position);
			ft_frame_return(frame);

			return true;
		}

		vec->limit = vec->position;

		return sc_proto_spdy_frame_received(this, frame);
	}

	FT_FATAL("Unknown/invalid frame received (type: %x)", frame->type);
	abort();
}

///

uint8_t * sc_proto_spdy_ext_trailer_append(struct ft_frame * frame, uint16_t trailer_version, uint16_t trailer_type, uint8_t trailer_flags, size_t trailer_size)
{
	bool ok;

	assert(frame != NULL);
	assert(frame->vec_limit == 1); // Simple frames only

	struct ft_vec * vec = ft_frame_get_vec(frame);
	assert(vec != NULL);

	uint8_t  frame_flags;
	uint32_t frame_length;
	uint8_t * begin = ft_vec_ptr(vec);
	ft_load_u8(begin + 4, &frame_flags);
	ft_load_u24(begin + 5, &frame_length);

	// Make sure that we have a correct SPDY frame
	assert((frame_length + SPDY_HEADER_SIZE) == vec->limit);

	ok = ft_vec_extend(vec, trailer_size + SPDY_HEADER_SIZE);
	if (!ok) return NULL;

	// Point at the end of trailer
	uint8_t * cursor = begin + SPDY_HEADER_SIZE + frame_length + trailer_size;
	cursor = ft_store_u16(cursor, trailer_version);
	cursor = ft_store_u16(cursor, trailer_type);
	cursor = ft_store_u8(cursor, trailer_flags); // Flags
	cursor = ft_store_u24(cursor, trailer_size); // Length

	ft_store_u8(begin + 4, frame_flags | ALX1_FLAG_EXT_TRAILER); 
	ft_store_u24(begin + 5, frame_length + trailer_size + SPDY_HEADER_SIZE); 

	return begin + SPDY_HEADER_SIZE + frame_length;
}

