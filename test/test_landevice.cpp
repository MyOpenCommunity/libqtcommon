#include "test_landevice.h"
#include "device_tester.h"

#include <landevice.h>

#include <QVariant>


void TestLanDevice::initTestCase()
{
	dev = new LanDevice;
}

void TestLanDevice::cleanupTestCase()
{
	delete dev;
}

void TestLanDevice::receiveStatus()
{
	DeviceTester t(dev, LanDevice::DIM_STATUS);
	t.check("*#13**9*0##", false);
	t.check("*#13**9*1##", true);
}

void TestLanDevice::receiveIp()
{
	DeviceTester t(dev, LanDevice::DIM_IP);
	t.check("*#13**10*10*3*3*81##", "10.3.3.81");
}

void TestLanDevice::receiveNetmask()
{
	DeviceTester t(dev, LanDevice::DIM_NETMASK);
	t.check("*#13**11*255*255*255*0##", "255.255.255.0");
}

void TestLanDevice::receiveMacAddress()
{
	DeviceTester t(dev, LanDevice::DIM_MACADDR);
	t.check("*#13**12*0*3*80*0*34*45##", "00:03:50:00:22:2d");
}

void TestLanDevice::receiveGateway()
{
	DeviceTester t(dev, LanDevice::DIM_GATEWAY);
	t.check("*#13**50*27*238*64*1##", "27.238.64.1");
}

void TestLanDevice::receiveDns1()
{
	DeviceTester t(dev, LanDevice::DIM_DNS1);
	t.check("*#13**51*208*67*222*222##", "208.67.222.222");
}

void TestLanDevice::receiveDns2()
{
	DeviceTester t(dev, LanDevice::DIM_DNS2);
	t.check("*#13**52*208*67*220*220*##", "208.67.220.220");
}

