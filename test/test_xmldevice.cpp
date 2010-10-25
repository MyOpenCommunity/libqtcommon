/*
 * BTouch - Graphical User Interface to control MyHome System
 *
 * Copyright (C) 2010 BTicino S.p.A.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "test_xmldevice.h"

#include <QtTest>
#include <QSignalSpy>

#include "xmldevice.h"

Q_DECLARE_METATYPE(XmlResponse)

TestXmlDevice::TestXmlDevice() :
	QObject(), dev(new XmlDevice)
{
	qRegisterMetaType<XmlResponse>("XmlResponse");
}

void TestXmlDevice::testHeader()
{
	QString data("<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\""
					"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
					"	<Hdr>"
					"		<MsgID>"
					"			<SID>1EFC3E00-2066-6C13-55D2-81D7D7DB0E62</SID>"
					"			<PID>0</PID>"
					"		</MsgID>"
					"		<Dst>"
					"			<IP>10.3.3.195</IP>"
					"		</Dst>"
					"		<Src>"
					"			<IP>192.168.1.110</IP>"
					"		</Src>"
					"	</Hdr>"
					"	<Cmd Dsc=\"Welcome Message\">"
					"		<WMsg>"
					"			<PwdOp>672295445</PwdOp>"
					"		</WMsg>"
					"	</Cmd>"
					"</OWNxml>");

	dev->handleData(data);

	QCOMPARE(dev->sid, QString("1EFC3E00-2066-6C13-55D2-81D7D7DB0E62"));
	QCOMPARE(dev->local_addr, QString("10.3.3.195"));
	QCOMPARE(dev->server_addr, QString("192.168.1.110"));
}

void TestXmlDevice::testWelcome()
{
	QString data("<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\""
					"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
					"	<Hdr>"
					"		<MsgID>"
					"			<SID>1EFC3E00-2066-6C13-55D2-81D7D7DB0E62</SID>"
					"			<PID>0</PID>"
					"		</MsgID>"
					"		<Dst>"
					"			<IP>10.3.3.195</IP>"
					"		</Dst>"
					"		<Src>"
					"			<IP>192.168.1.110</IP>"
					"		</Src>"
					"	</Hdr>"
					"	<Cmd Dsc=\"Welcome Message\">"
					"		<WMsg>"
					"			<PwdOp>672295445</PwdOp>"
					"		</WMsg>"
					"	</Cmd>"
					"</OWNxml>");

	QSignalSpy spy(dev, SIGNAL(responseReceived(XmlResponse)));
	dev->handleData(data);

	QCOMPARE(spy.count(), 1);

	QList<QVariant> arguments = spy.takeFirst();

	XmlResponse response = arguments.at(0).value<XmlResponse>();

	QVERIFY(response.contains(XmlDevice::RESP_WELCOME));
}

void TestXmlDevice::testServerList()
{
	QString data("<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\""
				 "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
				 "	<Hdr>"
				 "		<MsgID>"
				 "			<SID>1EFC3E00-2066-6C13-55D2-81D7D7DB0E62</SID>"
				 "			<PID>4</PID>"
				 "		</MsgID>"
				 "		<Dst>"
				 "			<IP>10.3.3.195</IP>"
				 "		</Dst>"
				 "		<Src>"
				 "			<IP>192.168.1.110</IP>"
				 "		</Src>"
				 "	</Hdr>"
				 "	<Cmd>"
				 "		<AW26C1>"
				 "			<server>"
				 "				<name>TestServer1</name>"
				 "			</server>"
				 "			<server>"
				 "				<name>TestServer2</name>"
				 "			</server>"
				 "		</AW26C1>"
				 "	</Cmd>"
				 "</OWNxml>");

	QSignalSpy spy(dev, SIGNAL(responseReceived(XmlResponse)));
	dev->handleData(data);

	QCOMPARE(spy.count(), 1);

	QList<QVariant> arguments = spy.takeFirst();

	XmlResponse response = arguments.at(0).value<XmlResponse>();

	QVERIFY(response.contains(XmlDevice::RESP_SERVERLIST));

	QStringList servers = response[XmlDevice::RESP_SERVERLIST].toStringList();
	QCOMPARE(servers.count(), 2);
	for (int i = 0; i < servers.count(); ++i)
		QCOMPARE(servers.at(i), QString("TestServer%1").arg(i + 1));
}

void TestXmlDevice::testBuildCommand()
{
	QString data("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				 "<OWNxml xmlns=\"http://www.bticino.it/xopen/v1 xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
				 "	<Hdr>\n"
				 "		<MsgID>\n"
				 "			<SID>1EFC3E00-2066-6C13-55D2-81D7D7DB0E62</SID>\n"
				 "			<PID>0</PID>\n"
				 "		</MsgID>\n"
				 "		<Dst>\n"
				 "			<IP>server_address</IP>\n"
				 "		</Dst>\n"
				 "		<Src>\n"
				 "			<IP>local_address</IP>\n"
				 "		</Src>\n"
				 "	</Hdr>\n"
				 "	<Cmd>\n"
				 "		<command/>\n"
				 "	</Cmd>\n"
				 "</OWNxml>\n");

	dev->sid = "1EFC3E00-2066-6C13-55D2-81D7D7DB0E62";
	dev->local_addr = "local_address";
	dev->server_addr = "server_address";

	QString command = dev->buildCommand("command");

	QCOMPARE(command, data);
}

void TestXmlDevice::testBuildCommandWithArg()
{
	QString data("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				 "<OWNxml xmlns=\"http://www.bticino.it/xopen/v1 xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
				 "	<Hdr>\n"
				 "		<MsgID>\n"
				 "			<SID>1EFC3E00-2066-6C13-55D2-81D7D7DB0E62</SID>\n"
				 "			<PID>0</PID>\n"
				 "		</MsgID>\n"
				 "		<Dst>\n"
				 "			<IP>server_address</IP>\n"
				 "		</Dst>\n"
				 "		<Src>\n"
				 "			<IP>local_address</IP>\n"
				 "		</Src>\n"
				 "	</Hdr>\n"
				 "	<Cmd>\n"
				 "		<command>\n"
				 "			<id>test_argument</id>\n"
				 "		</command>\n"
				 "	</Cmd>\n"
				 "</OWNxml>\n");

	dev->sid = "1EFC3E00-2066-6C13-55D2-81D7D7DB0E62";
	dev->local_addr = "local_address";
	dev->server_addr = "server_address";

	QString command = dev->buildCommand("command", "test_argument");

	QCOMPARE(command, data);
}
