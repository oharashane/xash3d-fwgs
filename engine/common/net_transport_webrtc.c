/*
net_transport_webrtc.c - WebRTC transport implementation (Emscripten only)
Copyright (C) 2024 Xash3D FWGS contributors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifdef __EMSCRIPTEN__
#ifdef NET_TRANSPORT_WEBRTC

#include "common.h"
#include "net_transport.h"
#include <emscripten.h>

// Ring buffer for incoming packets
#define WEBRTC_QUEUE_SIZE 64
#define WEBRTC_MAX_PACKET_SIZE 2048

typedef struct webrtc_packet_s
{
	uint8_t data[WEBRTC_MAX_PACKET_SIZE];
	int length;
	netadr_t from;
} webrtc_packet_t;

static struct {
	webrtc_packet_t queue[WEBRTC_QUEUE_SIZE];
	int head;
	int tail;
	int count;
	qboolean initialized;
} g_webrtc_state = { 0 };

// Forward declarations
static int NET_WebRTC_Init(void);
static void NET_WebRTC_Shutdown(void);
static int NET_WebRTC_Send(const uint8_t *buf, int len, const netadr_t *to);
static int NET_WebRTC_Poll(void);
static int NET_WebRTC_Recv(uint8_t *buf, int maxlen, netadr_t *from);

// WebRTC transport implementation
static net_transport_t g_webrtc_transport = {
	.init = NET_WebRTC_Init,
	.shutdown = NET_WebRTC_Shutdown,
	.send = NET_WebRTC_Send,
	.poll = NET_WebRTC_Poll,
	.recv = NET_WebRTC_Recv,
	.name = "WebRTC"
};

// JavaScript function imports (implemented in library_webrtc.js)
extern int emscripten_webrtc_send(const uint8_t *data, int len);
extern int webrtc_init_js(void);

/*
==================
NET_GetWebRTCTransport

Get the WebRTC transport instance
==================
*/
net_transport_t *NET_GetWebRTCTransport(void)
{
	return &g_webrtc_transport;
}

/*
==================
webrtc_init

Called by JavaScript when DataChannel is ready
==================
*/
EMSCRIPTEN_KEEPALIVE int webrtc_init(void)
{
	Con_Printf( "WebRTC transport initializing...\n" );
	
	// Check if JavaScript side is ready
	if( !webrtc_init_js() )
	{
		Con_Printf( "WebRTC transport: JavaScript side not ready\n" );
		return 0;
	}
	
	// Reset the packet queue
	g_webrtc_state.head = 0;
	g_webrtc_state.tail = 0;
	g_webrtc_state.count = 0;
	g_webrtc_state.initialized = true;
	
	Con_Printf( "WebRTC transport initialized successfully\n" );
	
	// Switch to WebRTC transport
	NET_SetTransport( &g_webrtc_transport );
	
	return 1; // Success
}

/*
==================
webrtc_push

Called by JavaScript when a DataChannel message arrives
==================
*/
EMSCRIPTEN_KEEPALIVE void webrtc_push(const uint8_t *data, int len)
{
	if( !g_webrtc_state.initialized )
	{
		Con_DPrintf( "WebRTC: received packet but not initialized\n" );
		return;
	}
	
	if( len <= 0 || len > WEBRTC_MAX_PACKET_SIZE )
	{
		Con_DPrintf( "WebRTC: invalid packet size %d\n", len );
		return;
	}
	
	if( g_webrtc_state.count >= WEBRTC_QUEUE_SIZE )
	{
		Con_DPrintf( "WebRTC: packet queue full, dropping packet\n" );
		return;
	}
	
	// Add packet to queue
	webrtc_packet_t *packet = &g_webrtc_state.queue[g_webrtc_state.tail];
	memcpy( packet->data, data, len );
	packet->length = len;
	
	// Set fake source address (server address)
	NET_StringToAdr( "127.0.0.1:27015", &packet->from );
	
	g_webrtc_state.tail = (g_webrtc_state.tail + 1) % WEBRTC_QUEUE_SIZE;
	g_webrtc_state.count++;
	
	Con_DPrintf( "WebRTC: queued packet %d bytes, queue size: %d\n", len, g_webrtc_state.count );
}

// ============================================================================
// Transport Implementation
// ============================================================================

/*
==================
NET_WebRTC_Init

Initialize WebRTC transport
==================
*/
static int NET_WebRTC_Init(void)
{
	Con_Printf( "NET_WebRTC_Init: WebRTC transport ready\n" );
	return 1;
}

/*
==================
NET_WebRTC_Shutdown

Shutdown WebRTC transport
==================
*/
static void NET_WebRTC_Shutdown(void)
{
	Con_Printf( "NET_WebRTC_Shutdown: shutting down WebRTC transport\n" );
	g_webrtc_state.initialized = false;
	g_webrtc_state.count = 0;
}

/*
==================
NET_WebRTC_Send

Send packet via WebRTC DataChannel
==================
*/
static int NET_WebRTC_Send(const uint8_t *buf, int len, const netadr_t *to)
{
	if( !g_webrtc_state.initialized )
	{
		Con_DPrintf( "WebRTC: attempt to send but not initialized\n" );
		return -1;
	}
	
	Con_DPrintf( "WebRTC: sending %d bytes to %s\n", len, NET_AdrToString( *to ));
	
	// Send via JavaScript DataChannel
	int result = emscripten_webrtc_send( buf, len );
	
	if( result != len )
	{
		Con_DPrintf( "WebRTC: send failed, requested %d bytes, sent %d\n", len, result );
		return -1;
	}
	
	return len;
}

/*
==================
NET_WebRTC_Poll

Check if packets are available
==================
*/
static int NET_WebRTC_Poll(void)
{
	if( !g_webrtc_state.initialized )
		return 0;
	
	return g_webrtc_state.count;
}

/*
==================
NET_WebRTC_Recv

Receive packet from WebRTC queue
==================
*/
static int NET_WebRTC_Recv(uint8_t *buf, int maxlen, netadr_t *from)
{
	if( !g_webrtc_state.initialized || g_webrtc_state.count == 0 )
		return 0;
	
	// Get packet from queue
	webrtc_packet_t *packet = &g_webrtc_state.queue[g_webrtc_state.head];
	
	if( packet->length > maxlen )
	{
		Con_DPrintf( "WebRTC: packet too large (%d > %d), dropping\n", packet->length, maxlen );
		
		// Remove packet from queue
		g_webrtc_state.head = (g_webrtc_state.head + 1) % WEBRTC_QUEUE_SIZE;
		g_webrtc_state.count--;
		
		return -1;
	}
	
	// Copy packet data
	memcpy( buf, packet->data, packet->length );
	if( from )
		*from = packet->from;
	
	int len = packet->length;
	
	// Remove packet from queue
	g_webrtc_state.head = (g_webrtc_state.head + 1) % WEBRTC_QUEUE_SIZE;
	g_webrtc_state.count--;
	
	Con_DPrintf( "WebRTC: received %d bytes, queue size: %d\n", len, g_webrtc_state.count );
	
	return len;
}

#endif // NET_TRANSPORT_WEBRTC
#endif // __EMSCRIPTEN__
