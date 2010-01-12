#include "test_entryphone_device.h"
#include "openserver_mock.h"
#include "device_tester.h"

#include <openclient.h>
#include <openmsg.h>
#include <entryphone_device.h>

#include <QtTest/QtTest>

void TestEntryphoneDevice::initTestCase()
{
	dev = new EntryphoneDevice("11");
}

void TestEntryphoneDevice::cleanupTestCase()
{
	delete dev;
}

void TestEntryphoneDevice::init()
{
	dev->resetCallState();
}

void TestEntryphoneDevice::simulateIncomingCall(int kind, int mmtype)
{
	QString incoming_call = QString("*8*1#%1#%2#21*%3##").arg(kind).arg(mmtype).arg(dev->where);
	// I HATE YOU OpenMsg!
	OpenMsg msg(incoming_call.toStdString());
	dev->manageFrame(msg);
}

void TestEntryphoneDevice::simulateCallerAddress(int kind, int mmtype, QString where)
{
	QString frame = QString("*8*9#%1#%2*%3##").arg(kind).arg(mmtype).arg(where);

	OpenMsg msg(frame.toStdString());
	dev->manageFrame(msg);
}

void TestEntryphoneDevice::sendAnswerCall()
{
	// ringtone 1
	int kind = 1;
	// audio and video call
	int mmtype = 4;
	// simulate an incoming call, since we must remember kind and mmtype
	simulateIncomingCall(kind, mmtype);

	dev->answerCall();
	client_command->flush();
	// answers require device address as 'where'
	QCOMPARE(server->frameCommand(), QString("*8*2#%1#%2*%3##").arg(kind).arg(mmtype).arg(dev->where));
}

void TestEntryphoneDevice::sendEndCall()
{
	// ringtone 1
	int kind = 1;
	// audio and video call
	int mmtype = 4;
	simulateIncomingCall(kind, mmtype);
	const char *end_all_calls = "4";

	dev->endCall();
	client_command->flush();
	QString frame = QString("*8*3#%1#%2*%3%4##").arg(kind).arg(mmtype).arg(end_all_calls).arg(dev->where);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendInitVctProcess()
{
	// call type accepted, 1 = scs bus only
	int type = 1;
	dev->initVctProcess();
	client_command->flush();
	QString frame = QString("*8*37#%1*%2##").arg(type).arg(dev->where);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendCameraOn()
{
	QString where_camera = "20";
	dev->cameraOn(where_camera);
	client_command->flush();
	QString frame = QString("*8*4#%1*%2##").arg(dev->where).arg(where_camera);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendStairLightActivate()
{
	dev->stairLightActivate();
	client_command->flush();
	QString frame = QString("*8*21*%1##").arg(dev->where);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendStairLightRelease()
{
	dev->stairLightRelease();
	client_command->flush();
	QString frame = QString("*8*22*%1##").arg(dev->where);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendOpenLock()
{
	int kind = 1;
	int mmtype = 4;
	QString caller_addr = "20";
	simulateIncomingCall(kind, mmtype);
	simulateCallerAddress(kind, mmtype, caller_addr);

	dev->openLock();
	client_command->flush();
	QString frame = QString("*8*19*%1##").arg(caller_addr);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendReleaseLock()
{
	int kind = 1;
	int mmtype = 4;
	QString caller_addr = "20";
	simulateIncomingCall(kind, mmtype);
	simulateCallerAddress(kind, mmtype, caller_addr);

	dev->releaseLock();
	client_command->flush();
	QString frame = QString("*8*20*%1##").arg(caller_addr);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::receiveIncomingCall()
{
	DeviceTester t(dev, EntryphoneDevice::VCT_CALL);
	QString frame = QString("*8*1#%1#%2#21*%3##").arg(1).arg(4).arg(dev->where);
	t.check(frame, true);
}

void TestEntryphoneDevice::receiveCallerAddress()
{
	int kind = 1;
	int mmtype = 4;
	QString caller_addr = "20";
	simulateIncomingCall(kind, mmtype);
	simulateCallerAddress(kind, mmtype, caller_addr);

	QCOMPARE(dev->caller_address, caller_addr);
}

void TestEntryphoneDevice::sendCycleExternalUnits()
{
	int kind = 1;
	int mmtype = 4;
	QString caller_addr = "20";
	simulateIncomingCall(kind, mmtype);
	simulateCallerAddress(kind, mmtype, caller_addr);

	dev->cycleExternalUnits();
	client_command->flush();
	QString frame = QString("*8*6#%1*%2##").arg(dev->where).arg(caller_addr);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendInternalIntercomCall()
{
	QString where = "16";
	dev->internalIntercomCall(where);
	client_command->flush();
	QString frame = QString("*8*1#6#2#%1*%2##").arg(dev->where).arg(where);
	QCOMPARE(server->frameCommand(), frame);
}

void TestEntryphoneDevice::sendExternalIntercomCall()
{
	QString where = "16";
	dev->externalIntercomCall(where);
	client_command->flush();
	QString frame = QString("*8*1#7#2#%1*%2##").arg(dev->where).arg(where);
	QCOMPARE(server->frameCommand(), frame);
}

