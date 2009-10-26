#include "test_pull_manager.h"
#include <lighting_device.h>
#include <openmsg.h>

#include <QtTest/QtTest>

namespace
{
	bool parseFrames(const char *frame, PullStateManager *psm, bool is_environment)
	{
		OpenMsg msg(frame);
		return psm->moreFrameNeeded(msg, is_environment);
	}
}

void TestPullManager::testSimpleLight()
{
	PullStateManager psm(PULL_UNKNOWN);
	// light on, general off, light on
	parseFrames("*1*1*11##", &psm, false);
	parseFrames("*1*0*1##", &psm, true);
	parseFrames("*1*1*11##", &psm, false);
	QCOMPARE(psm.getPullMode(), PULL);
}

void TestPullManager::testSimpleLight2()
{
	PullStateManager psm(PULL_UNKNOWN);
	// light off, general on, light on
	parseFrames("*1*0*11##", &psm, false);
	parseFrames("*1*1*1##", &psm, true);
	parseFrames("*1*1*11##", &psm, false);
	QCOMPARE(psm.getPullMode(), NOT_PULL);
}

void TestPullManager::testDimmer()
{
	PullStateManager psm(PULL_UNKNOWN);
	// dimmer off, general level 3, dimmer level 3
	parseFrames("*1*0*11##", &psm, false);
	parseFrames("*1*3*1##", &psm, true);
	parseFrames("*1*3*11##", &psm, false);
	QCOMPARE(psm.getPullMode(), NOT_PULL);
}

void TestPullManager::testDimmer2()
{
	PullStateManager psm(PULL_UNKNOWN);
	// dimmer level 7, general level 3, dimmer level 3
	parseFrames("*1*7*11##", &psm, false);
	parseFrames("*1*3*1##", &psm, true);
	parseFrames("*1*3*11##", &psm, false);
	QCOMPARE(psm.getPullMode(), NOT_PULL);
}

void TestPullManager::testDimmer3()
{
	PullStateManager psm(PULL_UNKNOWN);
	// dimmer level 7, general level 3, dimmer level 7
	parseFrames("*1*7*11##", &psm, false);
	parseFrames("*1*3*1##", &psm, true);
	parseFrames("*1*7*11##", &psm, false);
	QCOMPARE(psm.getPullMode(), PULL);
}

void TestPullManager::testDimmer100()
{
	PullStateManager psm(PULL_UNKNOWN);
	// dimmer level100 20, general level100 40, dimmer level100 40
	parseFrames("*#1*12*1*120*255##", &psm, false);
	parseFrames("*#1*12*#1*140*255##", &psm, true);
	parseFrames("*#1*12*1*140*255##", &psm, false);
	QCOMPARE(psm.getPullMode(), NOT_PULL);
}

void TestPullManager::testDimmer100_2()
{
	PullStateManager psm(PULL_UNKNOWN);
	// dimmer level100 20, general level100 40, dimmer level100 40
	parseFrames("*#1*12*1*100*255##", &psm, false);
	parseFrames("*#1*12*#1*140*255##", &psm, true);
	parseFrames("*#1*12*1*140*255##", &psm, false);
	QCOMPARE(psm.getPullMode(), NOT_PULL);
}
