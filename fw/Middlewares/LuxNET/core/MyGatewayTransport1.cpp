/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Tomas Hozza <thozza@gmail.com>
 * Copyright (C) 2015  Tomas Hozza
 * Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "MyGatewayTransport.h"
#include "MyTransport.h"
extern void receive(const MyMessage *message);
// global variables
extern MyMessage _msg;
extern MyMessage _msgTmp;

inline void gatewayTransportProcess(void)
{
	if (gatewayTransportAvailable()) {
		_msg = gatewayTransportReceive();
		if (_msg.getDestination() == GATEWAY_ADDRESS) {

			// Check if sender requests an echo
			if (_msg.getRequestEcho()) {
				// Copy message
				_msgTmp = _msg;
				// Reply without echo flag, otherwise we would end up in an eternal loop
				_msgTmp.setRequestEcho(false);
				_msgTmp.setEcho(true);
				_msgTmp.setSender(getNodeId());
				_msgTmp.setDestination(_msg.getSender());
				gatewayTransportSend(_msgTmp);
			}
			if (_msg.getCommand() == C_INTERNAL) {
				if (_msg.getType() == I_VERSION) {
					// Request for version. Create the response
					gatewayTransportSend(buildGw(_msgTmp, I_VERSION).set(MYSENSORS_LIBRARY_VERSION));
#ifdef MY_INCLUSION_MODE_FEATURE
				} else if (_msg.getType() == I_INCLUSION_MODE) {
					// Request to change inclusion mode
					inclusionModeSet(atoi(_msg.data) == 1);
#endif
				} else {
					(void)_processInternalCoreMessage();
				}
			} else {
				// Call incoming message callback if available
				
					receive(_msg);
				
			}
		} else {
#if defined(MY_SENSOR_NETWORK)
			transportSendRoute(_msg);
#endif
		}
	}
}
