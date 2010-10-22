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
#include "test_xmlclient.h"

#include <QtTest>
#include <QSignalSpy>

#include "xmlclient.h"


void TestXmlClient::testSimple()
{
	XmlClient *client = new XmlClient;
	QSignalSpy spy(client, SIGNAL(dataReceived(QString)));

	QVERIFY(client->buffer.isEmpty());

	client->buffer = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>";
	client->parseData();

	QCOMPARE(spy.count(), 1);
	QList<QVariant> arguments = spy.takeFirst();

	QCOMPARE(arguments.at(0).toString(), QString("<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>"));

	client->deleteLater();
}

void TestXmlClient::testDouble()
{
	XmlClient *client = new XmlClient;
	QSignalSpy spy(client, SIGNAL(dataReceived(QString)));

	QVERIFY(client->buffer.isEmpty());

	client->buffer = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml><?xml version=\"1.0\" encoding=\"utf-8\"?>\n<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>";
	client->parseData();

	QCOMPARE(spy.count(), 2);
	QList<QVariant> arguments = spy.takeFirst();

	for (int i = 0; i < spy.count(); ++i)
		QCOMPARE(arguments.at(i).toString(), QString("<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>"));

	client->deleteLater();
}

void TestXmlClient::testGarbage()
{
	XmlClient *client = new XmlClient;
	QSignalSpy spy(client, SIGNAL(dataReceived(QString)));

	QVERIFY(client->buffer.isEmpty());

	client->buffer = "hbwtyjhwrthnahhq2<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>rwg5h4qwhw6buh3dty jebjve<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>erhytrhhergaerrgg";
	client->parseData();

	QCOMPARE(spy.count(), 2);
	QList<QVariant> arguments = spy.takeFirst();

	for (int i = 0; i < spy.count(); ++i)
		QCOMPARE(arguments.at(i).toString(), QString("<OWNxml xmlns=\"http://www.bticino.it/xopen/v1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">Test</OWNxml>"));

	client->deleteLater();
}
