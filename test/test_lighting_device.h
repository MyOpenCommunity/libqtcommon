#ifndef TEST_LIGHTING_DEVICE_H
#define TEST_LIGHTING_DEVICE_H

#include "test_device.h"

const QString LIGHT_DEVICE_WHERE = "0313";
const QString LIGHT_ADDR_EXTENSION = "#4#12";
const QString LIGHT_ADDR_EXTENSION_2 = "#4#01";

class LightingDevice;

class TestLightingDevice : public TestDevice
{
Q_OBJECT
public:
	TestLightingDevice(QString w);

private slots:
	void initTestCase();
	void cleanupTestCase();

	void sendTurnOn();
	void sendRequestStatus();

	void testCheckAddress();

private:
	LightingDevice *dev;
	QString where;
};

#endif // TEST_LIGHTING_DEVICE_H
