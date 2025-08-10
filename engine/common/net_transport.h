/*
net_transport.h - network transport abstraction layer
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

#pragma once
#ifndef NET_TRANSPORT_H
#define NET_TRANSPORT_H

#include "common.h"
#include "netadr.h"

typedef struct net_transport_s
{
	// Initialize the transport
	int (*init)(void);
	
	// Shutdown the transport
	void (*shutdown)(void);
	
	// Send a packet through this transport
	// Returns: number of bytes sent, or -1 on error
	int (*send)(const uint8_t *buf, int len, const netadr_t *to);
	
	// Poll for available data
	// Returns: number of bytes available, or 0 if none
	int (*poll)(void);
	
	// Receive a packet from this transport
	// Returns: number of bytes received, or -1 on error, 0 if no data
	int (*recv)(uint8_t *buf, int maxlen, netadr_t *from);
	
	// Transport name for debugging
	const char *name;
} net_transport_t;

// Get the current active transport
net_transport_t *NET_GetCurrentTransport(void);

// Set the active transport (returns previous transport)
net_transport_t *NET_SetTransport(net_transport_t *transport);

// Get the default UDP transport
net_transport_t *NET_GetUDPTransport(void);

#ifdef NET_TRANSPORT_WEBRTC
// Get the WebRTC transport (Emscripten only)
net_transport_t *NET_GetWebRTCTransport(void);
#endif

#endif // NET_TRANSPORT_H
