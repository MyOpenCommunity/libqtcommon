#include "device_tester.h"

#include <device.h>

#include <QStringList>
#include <QVariant>
#include <QMetaType>
#include <QtTest/QtTest>

// To put/extract in QVariant
Q_DECLARE_METATYPE(StatusList)


DeviceTester::DeviceTester(device *d, int type) : spy(d, SIGNAL(status_changed(const StatusList&)))
{
	dim_type = type;
	dev = d;
}

QVariant DeviceTester::getResult(const QStringList& frames)
{
	spy.clear();
	for (int i = 0; i < frames.size(); ++i)
		dev->frame_rx_handler(frames[i].toAscii().data());

	QVariant signal_arg = spy.last().at(0); // get the first argument from last signal
	if (signal_arg.canConvert<StatusList>())
	{
		StatusList sl = signal_arg.value<StatusList>();
		if (sl.contains(dim_type))
			return sl[dim_type];
	}
	return QVariant();
}

void DeviceTester::check(const QStringList& frames, const QVariant& result)
{
	QVERIFY(getResult(frames) == result);
}

void DeviceTester::check(QString frame, const QVariant& result)
{
	QVERIFY(getResult(QStringList(frame)) == result);
}

