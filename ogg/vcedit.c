/* This program is licensed under the GNU Library General Public License, version 2
 *
 * (c) 2000-2001 Michael Smith <msmith@labyrinth.net.au>
 *
 * Modified by Warren Spits <spits@cyberdude.com>
 *     - Handles vorbis files that are truncated or missing an eos flag.
 *
 * Comment editing backend, suitable for use by nice frontend interfaces.
 *
 * last modified: $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>

#include "vcedit.h"

#define CHUNKSIZE 4096

vcedit_state *vcedit_new_state(void)
{
	vcedit_state *state = malloc(sizeof(vcedit_state));
	memset(state, 0, sizeof(vcedit_state));

	return state;
}

char *vcedit_error(vcedit_state *state)
{
	return state->lasterror;
}

vorbis_comment *vcedit_comments(vcedit_state *state)
{
	return state->vc;
}

static void vcedit_clear_internals(vcedit_state *state)
{
	if(state->vc)
	{
		vorbis_comment_clear(state->vc);
		free(state->vc);
		state->vc=NULL;
	}
	if(state->os)
	{
		ogg_stream_clear(state->os);
		free(state->os);
		state->os=NULL;
	}
	if(state->oy)
	{
		ogg_sync_clear(state->oy);
		free(state->oy);
		state->oy=NULL;
	}
}

void vcedit_clear(vcedit_state *state)
{
	if(state)
	{
		vcedit_clear_internals(state);
		free(state);
	}
}

int vcedit_open(vcedit_state *state, FILE *in)
{
	return vcedit_open_callbacks(state, (void *)in, 
			(vcedit_read_func)fread, (vcedit_write_func)fwrite);
}

int vcedit_open_callbacks(vcedit_state *state, void *in,
		vcedit_read_func read_func, vcedit_write_func write_func)
{

	char *buffer;
	int bytes,i;
	ogg_packet *header;
	ogg_packet	header_main;
	ogg_packet  header_comments;
	ogg_packet	header_codebooks;
	ogg_page    og;
	vorbis_info vi;


	state->in = in;
	state->read = read_func;
	state->write = write_func;
	state->lasterror = 0;

	state->oy = malloc(sizeof(ogg_sync_state));
	ogg_sync_init(state->oy);

	buffer = ogg_sync_buffer(state->oy, CHUNKSIZE);
	bytes = state->read(buffer, 1, CHUNKSIZE, state->in);

	ogg_sync_wrote(state->oy, bytes);

	if(ogg_sync_pageout(state->oy, &og) != 1)
	{
		if(bytes<CHUNKSIZE)
			state->lasterror = "Input truncated or empty.";
		else
			state->lasterror = "Input is not an Ogg bitstream.";
		goto err;
	}

	state->serial = ogg_page_serialno(&og);

	state->os = malloc(sizeof(ogg_stream_state));
	ogg_stream_init(state->os, state->serial);

	vorbis_info_init(&vi);

	state->vc = malloc(sizeof(vorbis_comment));
	vorbis_comment_init(state->vc);

	if(ogg_stream_pagein(state->os, &og) < 0)
	{
		state->lasterror = "Error reading first page of Ogg bitstream.";
		goto err;
	}

	if(ogg_stream_packetout(state->os, &header_main) != 1)
	{
		state->lasterror = "Error reading initial header packet.";
		goto err;
	}

	if(vorbis_synthesis_headerin(&vi, state->vc, &header_main) < 0)
	{
		state->lasterror = "Ogg bitstream does not contain vorbis data.";
		goto err;
	}

	state->mainlen = header_main.bytes;
	state->mainbuf = malloc(state->mainlen);
	memcpy(state->mainbuf, header_main.packet, header_main.bytes);

	i = 0;
	header = &header_comments;
	while(i<2) {
		while(i<2) {
			int result = ogg_sync_pageout(state->oy, &og);
			if(result == 0) break; /* Too little data so far */
			else if(result == 1)
			{
				ogg_stream_pagein(state->os, &og);
				while(i<2)
				{
					result = ogg_stream_packetout(state->os, header);
					if(result == 0) break;
					if(result == -1)
					{
						state->lasterror = "Corrupt secondary header.";
						goto err;
					}
					vorbis_synthesis_headerin(&vi, state->vc, header);
					if(i==1)
					{
						state->booklen = header->bytes;
						state->bookbuf = malloc(state->booklen);
						memcpy(state->bookbuf, header->packet, 
								header->bytes);
					}
					i++;
					header = &header_codebooks;
				}
			}
		}

		buffer = ogg_sync_buffer(state->oy, CHUNKSIZE);
		bytes = state->read(buffer, 1, CHUNKSIZE, state->in);
		if(bytes == 0 && i < 2)
		{
			state->lasterror = "EOF before end of vorbis headers.";
			goto err;
		}
		ogg_sync_wrote(state->oy, bytes);
	}

	/* Headers are done! */
	vorbis_info_clear(&vi);
	return 0;

err:
	vcedit_clear_internals(state);
	return -1;
}

int vcedit_write(vcedit_state *state, void *out)
{
	ogg_stream_state streamout;
	ogg_packet header_main;
	ogg_packet header_comments;
	ogg_packet header_codebooks;

	ogg_page ogout, ogin;
	ogg_packet op;
	int result, outresult;
	char *buffer;
	int bytes, eosin=0, eosout=0;

	state->lasterror = 0;

	header_main.bytes = state->mainlen;
	header_main.packet = state->mainbuf;
	header_main.b_o_s = 1;
	header_main.e_o_s = 0;
	header_main.granulepos = 0;

	header_codebooks.bytes = state->booklen;
	header_codebooks.packet = state->bookbuf;
	header_codebooks.b_o_s = 0;
	header_codebooks.e_o_s = 0;
	header_codebooks.granulepos = 0;

	ogg_stream_init(&streamout, state->serial);

	vorbis_commentheader_out(state->vc, &header_comments);

	ogg_stream_packetin(&streamout, &header_main);
	ogg_stream_packetin(&streamout, &header_comments);
	ogg_stream_packetin(&streamout, &header_codebooks);

	while((result = ogg_stream_flush(&streamout, &ogout)))
	{
		if(state->write(ogout.header,1,ogout.header_len, out) !=
				(size_t) ogout.header_len)
			goto cleanup;
		if(state->write(ogout.body,1,ogout.body_len, out) != 
				(size_t) ogout.body_len)
			goto cleanup;
	}

	/* We copy the first logical stream
	 * through, rewriting the stream. */
	while (1)
	{
		outresult = eosin ? ogg_stream_flush(&streamout, &ogout) :
							ogg_stream_pageout(&streamout, &ogout);
		if (outresult > 0)
		{
			if (state->write(ogout.header,1,ogout.header_len, 
							 out) != (size_t) ogout.header_len)
				goto cleanup;
			if (state->write(ogout.body,1,ogout.body_len, 
							 out) != (size_t) ogout.body_len)
				goto cleanup;
			if (ogg_page_eos(&ogout)) eosout = 1;
		}
		if (outresult != 0) continue;
		if (eosout || (eosin && (result == 0))) break;

		while (1)
		{
			result = ogg_stream_packetout(state->os, &op);
			if (result < 0) continue;
			if (result > 0) ogg_stream_packetin(&streamout, &op);
			if (eosin || (result > 0)) break;

			while (1)
			{
				result = ogg_sync_pageout(state->oy, &ogin);

				if (result < 0) continue;
				if (result > 0)
				{
					ogg_stream_pagein(state->os, &ogin);
					if (ogg_page_eos(&ogin)) eosin = 1;
				}
				if (eosin || (result > 0)) break;

				buffer = ogg_sync_buffer(state->oy, CHUNKSIZE);
				bytes = state->read(buffer,1, CHUNKSIZE, state->in);
				ogg_sync_wrote(state->oy, bytes);
				if (bytes < CHUNKSIZE) eosin = 1;
			}
		}
	}

	eosin=0; /* clear it, because not all paths to here do */
	eosout=1; /* handle input files that are truncated or without an eos flag */

	/* We copy the rest of the stream (other logical streams)
	 * through, a page at a time. */
	while (1)
	{
		result = ogg_sync_pageout(state->oy, &ogout);
		if (result > 0)
		{
			if (state->write(ogout.header,1,ogout.header_len, 
							 out) != (size_t) ogout.header_len)
				goto cleanup;
			if (state->write(ogout.body,1,ogout.body_len, out) !=
							 (size_t) ogout.body_len)
				goto cleanup;
		}
		if (result != 0) continue;
		if (eosin) break;

		buffer = ogg_sync_buffer(state->oy, CHUNKSIZE);
		bytes = state->read(buffer,1, CHUNKSIZE, state->in);
		ogg_sync_wrote(state->oy, bytes);
		eosin = (bytes < CHUNKSIZE);
	}

cleanup:
	ogg_stream_clear(&streamout);
	ogg_packet_clear(&header_comments);

	free(state->mainbuf);
	free(state->bookbuf);

	vcedit_clear_internals(state);
	if(!(eosin && eosout))
	{
		state->lasterror =  
			"Error writing stream to output. "
			"Output stream may be corrupted or truncated.";
		return -1;
	}

	return 0;
}
