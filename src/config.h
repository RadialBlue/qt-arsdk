/*
    The file is part of the qt-arsdk project.

    Copyright (C) 2015-2016 Tom Swindell <t.swindell@rubyx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifndef CONFIG
#define CONFIG

#define ARDEVICE_DEFAULT_ADDR "192.168.42.1"
#define ARDISCOVERY_DEFAULT_PORT 44444

#define ARCONTROLLER_DEFAULT_TYPE "qt-arsdk"
#define ARCONTROLLER_DEFAULT_ADDR "0.0.0.0"
#define ARCONTROLLER_DEFAULT_PORT 43210

#define ARNETWORK_FRAME_HEADER_SIZE 7
#define ARNETWORK_COMMAND_HEADER_SIZE 4

#define ARNET_D2C_PING_ID       0x00
#define ARNET_C2D_PONG_ID       0x01
#define ARNET_C2D_NONACK_ID     0x0a
#define ARNET_C2D_ACK_ID        0x0b
#define ARNET_C2D_EMERG_ID      0x0c
#define ARNET_C2D_VIDEO_ACK_ID  0x0d
#define ARNET_D2C_NAVDATA_ID    0x7f
#define ARNET_D2C_EVENT_ID      0x7e
#define ARNET_D2C_VIDEO_DATA_ID 0x7d
#define ARNET_C2D_NAVDATA_ACK_ID 0xfe

#endif // CONFIG
