#pragma once

#ifndef _SunApiTypes_BroadcastRequestPacket_h_
#define _SunApiTypes_BroadcastRequestPacket_h_
namespace SunApiTypes
{
	#define DEF_REQ_SCAN_EX 1
#pragma pack(push, 1)
	struct BroadcastRequestPacket
	{
		unsigned char nMode;
		unsigned char chPacketID[18];
		char chMAC[18];
		char chIP[16];
		char chSubnetMask[16];
		char chGateway[16];
		char chPassword[20];
		unsigned short nPort;
		unsigned char nStatus;
		char chDeviceName[10];
		unsigned short nHttpPort;
		unsigned short nDevicePort;
		unsigned short nTcpPort;
		unsigned short nUdpPort;
		unsigned short nUploadPort;
		unsigned short nMulticastPort;
		unsigned char nNetworkMode;
		char chDDNS[128];
	};

	typedef struct BroadcastResponsePacket
	{
		unsigned char nMode;
		unsigned char chPacketID[18];
		char chMAC[18];
		char chIP[16];
		char chSubnetMask[16];
		char chGateway[16];
		char chPassword[20];
		unsigned short nPort;
		unsigned char nStatus;
		char chDeviceName[10];
		unsigned short nHttpPort;
		unsigned short nDevicePort;
		unsigned short nTcpPort;
		unsigned short nUdpPort;
		unsigned short nUploadPort;
		unsigned short nMulticastPort;
		unsigned char nNetworkMode;
		char chDDNS[128];
		char chAlias[64];
		char nModelType;
		unsigned short nVersion;
		char nHttpMode;
		unsigned short nHttpsPort;
		char nSupportedProtocol;
		char nPasswordStatus;
	};
#pragma pack(pop)
}
#endif