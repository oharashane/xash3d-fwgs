/*
net_transport.c - network transport abstraction layer implementation
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

#include "common.h"
#include "net_transport.h"
#include "net_ws.h"

// Current active transport
static net_transport_t *g_current_transport = NULL;

// Forward declarations for UDP transport
static int NET_UDP_Init(void);
static void NET_UDP_Shutdown(void);
static int NET_UDP_Send(const uint8_t *buf, int len, const netadr_t *to);
static int NET_UDP_Poll(void);
static int NET_UDP_Recv(uint8_t *buf, int maxlen, netadr_t *from);

// UDP transport implementation (default)
static net_transport_t g_udp_transport = {
	.init = NET_UDP_Init,
	.shutdown = NET_UDP_Shutdown,
	.send = NET_UDP_Send,
	.poll = NET_UDP_Poll,
	.recv = NET_UDP_Recv,
	.name = "UDP"
};

/*
==================
NET_GetCurrentTransport

Get the currently active transport
==================
*/
net_transport_t *NET_GetCurrentTransport(void)
{
	if( !g_current_transport )
		g_current_transport = &g_udp_transport;
	
	return g_current_transport;
}

/*
==================
NET_SetTransport

Set the active transport, returns the previous one
==================
*/
net_transport_t *NET_SetTransport(net_transport_t *transport)
{
	net_transport_t *prev = g_current_transport;
	
	if( transport )
	{
		Con_Printf( "NET_SetTransport: switching to %s transport\n", transport->name );
		g_current_transport = transport;
	}
	
	return prev;
}

/*
==================
NET_GetUDPTransport

Get the default UDP transport
==================
*/
net_transport_t *NET_GetUDPTransport(void)
{
	return &g_udp_transport;
}

// ============================================================================
// UDP Transport Implementation (fallback to existing code)
// ============================================================================

/*
==================
NET_UDP_Init

Initialize UDP transport
==================
*/
static int NET_UDP_Init(void)
{
	// UDP transport uses the existing NET_Init functionality
	// This is essentially a no-op since net.c handles UDP initialization
	return 1; // Success
}

/*
==================
NET_UDP_Shutdown

Shutdown UDP transport
==================
*/
static void NET_UDP_Shutdown(void)
{
	// UDP transport uses the existing NET_Shutdown functionality
	// This is essentially a no-op since net.c handles UDP shutdown
}

/*
==================
NET_UDP_Send

Send packet via UDP transport
==================
*/
static int NET_UDP_Send(const uint8_t *buf, int len, const netadr_t *to)
{
	// This will call the original NET_SendPacketEx functionality
	// For now, we'll use the existing code path by falling back to the original implementation
	// This function will be integrated into the existing NET_SendPacketEx
	
	// We can't directly call NET_SendPacketEx here because it would create a circular dependency
	// Instead, this will be handled by modifying NET_SendPacketEx to use transports
	
	Con_DPrintf( "NET_UDP_Send: %d bytes to %s (transport layer)\n", len, NET_AdrToString( *to ));
	
	// Return success for now - the actual implementation will be in the modified NET_SendPacketEx
	return len;
}

/*
==================
NET_UDP_Poll

Poll for available UDP data
==================
*/
static int NET_UDP_Poll(void)
{
	// For UDP, we don't have a reliable way to know how much data is available
	// without actually receiving it. The existing code handles this by attempting
	// to receive in NET_QueuePacket. We'll return 1 to indicate "data might be available"
	return 1;
}

/*
==================
NET_UDP_Recv

Receive packet via UDP transport
==================
*/
static int NET_UDP_Recv(uint8_t *buf, int maxlen, netadr_t *from)
{
	// This will integrate with the existing NET_QueuePacket functionality
	// For now, this is a placeholder that will be properly implemented
	// when we modify NET_QueuePacket to use the transport layer
	
	Con_DPrintf( "NET_UDP_Recv: requesting up to %d bytes (transport layer)\n", maxlen );
	
	// Return 0 for now - the actual implementation will be in the modified NET_QueuePacket
	return 0;
}
