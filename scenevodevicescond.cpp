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


#include "scenevodevicescond.h"
#include "probe_device.h" // NonControlledProbeDevice
#include "devices_cache.h" // add_device_to_cache
#include "lighting_device.h"
#include "media_device.h" // AmplifierDevice, AuxDevice
#include "scaleconversion.h" // bt2Celsius, bt2Fahrenheit, celsius2Bt, fahrenheit2Bt
#include "fontmanager.h" // bt_global::font
#include "icondispatcher.h" // bt_global::icons_cache
#include "skinmanager.h" // bt_global::skin
#include "xml_functions.h" // getTextChild
#include "btbutton.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

// The language used for the floating point number
static QLocale loc(QLocale::Italian);

#define CONDITION_MIN_TEMP 1050
#define CONDITION_MAX_TEMP  500



DeviceConditionDisplay::DeviceConditionDisplay(QWidget *parent, QString descr, QString top_icon) : QWidget(parent)
{
	QVBoxLayout *main_layout = new QVBoxLayout(this);
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(0);

	up_button = new BtButton;
	up_button->setImage(bt_global::skin->getImage("plus"));
	connect(up_button, SIGNAL(clicked()), this, SIGNAL(upClicked()));

	condition = new QLabel;
	condition->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	condition->setFont(bt_global::font->get(FontManager::TEXT));

	down_button = new BtButton;
	down_button->setImage(bt_global::skin->getImage("minus"));
	connect(down_button, SIGNAL(clicked()), this, SIGNAL(downClicked()));

#ifdef LAYOUT_BTOUCH
	QHBoxLayout *top_layout = new QHBoxLayout;
	top_layout->setContentsMargins(0, 0, 0, 0);
	top_layout->setSpacing(0);

	QLabel *top_image = new QLabel;
	top_image->setPixmap(*bt_global::icons_cache.getIcon(top_icon));
	top_layout->addWidget(top_image);

	QLabel *description = new QLabel;
	description->setFont(bt_global::font->get(FontManager::TEXT));
	description->setText(descr);
	top_layout->addWidget(description, 1, Qt::AlignHCenter);

	main_layout->addLayout(top_layout);
	main_layout->addStretch(1);

	QBoxLayout *central_layout = new QVBoxLayout;
	central_layout->addWidget(up_button);
	central_layout->addWidget(condition);
	central_layout->addWidget(down_button);
	central_layout->setContentsMargins(0, 0, 0, 0);
	central_layout->setSpacing(12);
#else
	Q_UNUSED(descr)
	Q_UNUSED(top_icon)
	QGridLayout *central_layout = new QGridLayout;
	central_layout->addWidget(down_button, 0, 0);
	central_layout->addWidget(condition, 0, 1);
	central_layout->addWidget(up_button, 0, 2);
	central_layout->setColumnMinimumWidth(1, 80);
	central_layout->setContentsMargins(0, 0, 0, 0);
	central_layout->setSpacing(10);
#endif

	main_layout->addLayout(central_layout);
}


void DeviceConditionDisplayOnOff::updateText(int min_condition_value, int max_condition_value)
{
	Q_UNUSED(max_condition_value)
	condition->setText(min_condition_value ? tr("ON") : tr("OFF"));
}


void DeviceConditionDisplayDimming::updateText(int min_condition_value, int max_condition_value)
{
	if (min_condition_value == 0)
		condition->setText(tr("OFF"));
	else
		condition->setText(QString("%1% - %2%").arg(min_condition_value).arg(max_condition_value));
}


void DeviceConditionDisplayVolume::updateText(int min_condition_value, int max_condition_value)
{
	if (min_condition_value == -1)
		condition->setText(tr("OFF"));
	else if (min_condition_value == 0 && max_condition_value == 31)
		condition->setText(tr("ON"));
	else
	{
		int val_min = min_condition_value;
		int val_max = max_condition_value;
		int vmin = (val_min == 0 ? 0 : (10 * (val_min <= 15 ? val_min/3 : (val_min-1)/3) + 1));
		int vmax = 10 * (val_max <= 15 ? val_max/3 : (val_max-1)/3);
		condition->setText(QString("%1% - %2%").arg(vmin).arg(vmax));
	}
}


void DeviceConditionDisplayTemperature::updateText(int min_condition_value, int max_condition_value)
{
	up_button->setAutoRepeat(true);
	down_button->setAutoRepeat(true);
	Q_UNUSED(max_condition_value)
	QString tmp = loc.toString(min_condition_value / 10.0, 'f', 1);
	TemperatureScale temp_scale = static_cast<TemperatureScale>((*bt_global::config)[TEMPERATURE_SCALE].toInt());

	switch (temp_scale)
	{
	case CELSIUS:
		tmp += TEMP_DEGREES"C \2611"TEMP_DEGREES"C";
		break;
	case FAHRENHEIT:
		tmp += TEMP_DEGREES"F \2611"TEMP_DEGREES"F";
		break;
	default:
		qWarning("Wrong temperature scale, defaulting to celsius");
		tmp += TEMP_DEGREES"C \2611"TEMP_DEGREES"C";
	}
	condition->setText(tmp);
}


DeviceCondition::DeviceCondition(DeviceConditionDisplayInterface *cond_display)
{
	condition_display = cond_display;
	satisfied = false;
	initialized = false;
	dev = 0;
}

void DeviceCondition::updateText(int min_condition_value, int max_condition_value)
{
	condition_display->updateText(min_condition_value, max_condition_value);
}

int DeviceCondition::get_min()
{
	return 0;
}

int DeviceCondition::get_max()
{
	return 0;
}

int DeviceCondition::get_step()
{
	return 1;
}

int DeviceCondition::get_condition_value()
{
	return cond_value;
}

void DeviceCondition::set_condition_value(int v)
{
	if (v > get_max())
		v = get_max();
	if (v < get_min())
		v = get_min();
	cond_value = v;
}

void DeviceCondition::set_condition_value(QString s)
{
	set_condition_value(s.toInt());
}

void DeviceCondition::get_condition_value(QString& out)
{
	out = QString::number(get_condition_value());
}

void DeviceCondition::Up()
{
	int val = get_current_value();
	val += get_step();
	set_current_value(val);
	qDebug("val = %d", get_current_value());
	Draw();
}

void DeviceCondition::Down()
{
	int val = get_current_value();
	val -= get_step();
	set_current_value(val);
	Draw();
}

void DeviceCondition::save()
{
	// Avoid a 'strange' behaviour after calling this method without change
	// the condition.
	if (!conditionChanged())
		return;

	onConditionSaved();
	// Request again the status of the device
	dev->init();
	// If the condition was already satisfied (before the condition change), we
	// want to emit the condSatisfied if the condition is satisfied after the
	// condition change.
	// Otherwise we reset the "initialized" status of the device in order to
	// avoid a condSatisfied emitted without a real change of the device status.
	//
	// Example using a device with an on/off status:
	// Condition -> on  device status -> off
	// Condition -> off device status -> off
	// we won't emit the condSatisfied
	//
	// Condition -> on device status -> on
	// Condition -> off device status -> off
	// we want to emit the condSatisfied

	if (satisfied)
		satisfied = false;
	else
		initialized = false;
}

bool DeviceCondition::conditionChanged()
{
	return get_condition_value() != get_current_value();
}

void DeviceCondition::onConditionSaved()
{
	set_condition_value(get_current_value());
}

void DeviceCondition::reset()
{
	onConditionReset();
	Draw();
}

void DeviceCondition::onConditionReset()
{
	set_current_value(get_condition_value());
}

int DeviceCondition::get_current_value()
{
	return current_value;
}

int DeviceCondition::set_current_value(int v)
{
	if (v > get_max())
		v = get_max();
	if (v < get_min())
		v = get_min();
	current_value = v;
	return current_value;
}

void DeviceCondition::setDevice(device *d)
{
	dev = d;
	connect(dev, SIGNAL(valueReceived(DeviceValues)), SLOT(valueReceived(DeviceValues)));
}

void DeviceCondition::valueReceived(const DeviceValues &values_list)
{
	if (!initialized)
	{
		// We won't emit the condSatisfied during the initialization
		if (parseValues(values_list))
			initialized = true;
	}
	else
	{
		if (!satisfied)
		{
			parseValues(values_list);

			// We emit the condSatisfied only if we pass from condition-not-satisfied
			// to condition-satisfied.
			if (satisfied)
				emit condSatisfied();
		}
		else
			parseValues(values_list);
	}
}

bool DeviceCondition::isTrue()
{
	return satisfied;
}


DeviceConditionLight::DeviceConditionLight(DeviceConditionDisplayInterface* cond_display, QString trigger, QString where, int openserver_id, PullMode pull_mode)
	: DeviceCondition(cond_display)
{
	set_condition_value(trigger);
	set_current_value(DeviceCondition::get_condition_value());
	setDevice(bt_global::add_device_to_cache(new LightingDevice(where, pull_mode, openserver_id)));
	Draw();
}

void DeviceConditionLight::Draw()
{
	updateText(get_current_value(), get_current_value());
}

bool DeviceConditionLight::parseValues(const DeviceValues &values_list)
{
	if (values_list.contains(LightingDevice::DIM_DEVICE_ON))
	{
		satisfied = DeviceCondition::get_condition_value() == values_list[LightingDevice::DIM_DEVICE_ON].toBool();
		return true;
	}
	return false;
}

int DeviceConditionLight::get_max()
{
	return 1;
}

void DeviceConditionLight::set_condition_value(QString s)
{
	int v = 0;
	if (s == "1")
		v = 1;
	else if (s == "0")
		v = 0;
	else
		qDebug() << "Unknown condition value " << s << " for device_condition_light_status";
	DeviceCondition::set_condition_value(v);
}

void DeviceConditionLight::get_condition_value(QString& out)
{
	out = DeviceCondition::get_condition_value() ? "1" : "0";
}


DeviceConditionDimming::DeviceConditionDimming(DeviceConditionDisplayInterface* cond_display, QString trigger, QString where, int openserver_id, PullMode pull_mode)
	: DeviceCondition(cond_display)
{
	if (trigger == "0")
	{
		set_condition_value_min(0);
		set_condition_value_max(0);
	}
	else
	{
		QStringList parts = trigger.split('-');
		Q_ASSERT_X(parts.size() == 2, "DeviceConditionDimming::DeviceConditionDimming",
			"the trigger value must have 0 or 2 digit with - as separator");
		set_condition_value_min(parts[0].toInt());
		set_condition_value_max(parts[1].toInt());
	}

	set_current_value_min(get_condition_value_min());
	set_current_value_max(get_condition_value_max());

	setDevice(bt_global::add_device_to_cache(new DimmerDevice(where, pull_mode, openserver_id)));
	Draw();
}

QString DeviceConditionDimming::get_current_value()
{
	return QString::number(get_condition_value_min());
}

int DeviceConditionDimming::get_min()
{
	return 0;
}

int DeviceConditionDimming::get_max()
{
	return 10;
}

int DeviceConditionDimming::get_step()
{
	return 1;
}

void DeviceConditionDimming::Up()
{
	int val = get_current_value_min();
	switch (val)
	{
	case 0:
		set_current_value_min(2);
		set_current_value_max(4);
		break;
	case 2:
		set_current_value_min(5);
		set_current_value_max(7);
		break;
	case 5:
		set_current_value_min(8);
		set_current_value_max(10);
		break;
	default:
		break;
	}
	Draw();
}

void DeviceConditionDimming::Down()
{
	int val = get_current_value_min();
	switch (val)
	{
	case 2:
		set_current_value_min(0);
		set_current_value_max(0);
		break;
	case 5:
		set_current_value_min(2);
		set_current_value_max(4);
		break;
	case 8:
		set_current_value_min(5);
		set_current_value_max(7);
		break;
	default:
		break;
	}
	Draw();
}

void DeviceConditionDimming::Draw()
{
	updateText(get_current_value_min()*10, get_current_value_max()*10);
}

bool DeviceConditionDimming::conditionChanged()
{
	return get_condition_value_min() != get_current_value_min() ||
		get_condition_value_max() != get_current_value_max();
}

void DeviceConditionDimming::onConditionSaved()
{
	set_condition_value_min(get_current_value_min());
	set_condition_value_max(get_current_value_max());
}

void DeviceConditionDimming::onConditionReset()
{
	set_current_value_min(get_condition_value_min());
	set_current_value_max(get_condition_value_max());
}

void DeviceConditionDimming::set_condition_value_min(int s)
{
	min_val = s;
}

int DeviceConditionDimming::get_condition_value_min()
{
	return min_val;
}

void DeviceConditionDimming::set_condition_value_max(int s)
{
	max_val = s;
}

int DeviceConditionDimming::get_condition_value_max()
{
	return max_val;
}

int DeviceConditionDimming::get_current_value_min()
{
	return current_value_min;
}

void DeviceConditionDimming::set_current_value_min(int min)
{
	current_value_min = min;
}

int DeviceConditionDimming::get_current_value_max()
{
	return current_value_max;
}

void DeviceConditionDimming::set_current_value_max(int max)
{
	current_value_max = max;
}

void DeviceConditionDimming::set_condition_value(QString s)
{
	DeviceCondition::set_condition_value(s.toInt() * 10);
}

void DeviceConditionDimming::get_condition_value(QString& out)
{

	if (get_condition_value_min() == 0)
		out = "0";
	else
		out = QString("%1-%2").arg(get_condition_value_min()).arg(get_condition_value_max());
}

bool DeviceConditionDimming::parseValues(const DeviceValues &values_list)
{
	DeviceValues::const_iterator it = values_list.constBegin();
	int trig_min = get_condition_value_min();
	int trig_max = get_condition_value_max();

	bool managed = false;
	while (it != values_list.constEnd())
	{
		int level = 0;
		if ((it.key() == LightingDevice::DIM_DEVICE_ON) || (it.key() == LightingDevice::DIM_DIMMER_LEVEL))
			level = it.value().toInt();
		else
		{
			++it;
			continue;
		}

		managed = true;
		satisfied = (level >= trig_min && level <= trig_max);
		++it;
	}
	return managed;
}


DeviceConditionDimming100::DeviceConditionDimming100(DeviceConditionDisplayInterface* cond_display, QString trigger, QString where, int openserver_id, PullMode pull_mode)
	: DeviceCondition(cond_display)
{
	if (trigger == "0")
	{
		set_condition_value_min(0);
		set_condition_value_max(0);
	}
	else
	{
		QStringList parts = trigger.split('-');
		Q_ASSERT_X(parts.size() == 2, "DeviceConditionDimming100::DeviceConditionDimming100",
			"the trigger value must have 0 or 2 digit with - as separator");
		set_condition_value_min(parts[0].toInt());
		set_condition_value_max(parts[1].toInt());
	}
	set_current_value_min(get_condition_value_min());
	set_current_value_max(get_condition_value_max());
	setDevice(bt_global::add_device_to_cache(new Dimmer100Device(where, pull_mode, openserver_id)));
	Draw();
}

QString DeviceConditionDimming100::get_current_value()
{
	return QString::number(get_condition_value_min());
}

int DeviceConditionDimming100::get_min()
{
	return 0;
}

int DeviceConditionDimming100::get_max()
{
	return 100;
}

int DeviceConditionDimming100::get_step()
{
	return 10;
}

void DeviceConditionDimming100::Up()
{
	int val = get_current_value_min();
	switch (val)
	{
	case 0:
		set_current_value_min(1);
		set_current_value_max(20);
		break;
	case 1:
		set_current_value_min(21);
		set_current_value_max(40);
		break;
	case 21:
		set_current_value_min(41);
		set_current_value_max(70);
		break;
	case 41:
		set_current_value_min(71);
		set_current_value_max(100);
		break;
	default:
		break;
	}
	Draw();
}

void DeviceConditionDimming100::Down()
{
	int val = get_current_value_min();
	switch (val)
	{
	case 1:
		set_current_value_min(0);
		set_current_value_max(0);
		break;
	case 21:
		set_current_value_min(1);
		set_current_value_max(20);
		break;
	case 41:
		set_current_value_min(21);
		set_current_value_max(40);
		break;
	case 71:
		set_current_value_min(41);
		set_current_value_max(70);
		break;
	default:
		break;
	}
	Draw();
}

void DeviceConditionDimming100::Draw()
{
	updateText(get_current_value_min(), get_current_value_max());
}

bool DeviceConditionDimming100::conditionChanged()
{
	return get_condition_value_min() != get_current_value_min() ||
		get_condition_value_max() != get_current_value_max();
}

void DeviceConditionDimming100::onConditionSaved()
{
	set_condition_value_min(get_current_value_min());
	set_condition_value_max(get_current_value_max());
}

void DeviceConditionDimming100::onConditionReset()
{
	set_current_value_min(get_condition_value_min());
	set_current_value_max(get_condition_value_max());
}

void DeviceConditionDimming100::set_condition_value_min(int s)
{
	min_val = s;
}

int DeviceConditionDimming100::get_condition_value_min()
{
	return min_val;
}

void DeviceConditionDimming100::set_condition_value_max(int s)
{
	max_val = s;
}

int DeviceConditionDimming100::get_condition_value_max()
{
	return max_val;
}

int DeviceConditionDimming100::get_current_value_min()
{
	return current_value_min;
}

void DeviceConditionDimming100::set_current_value_min(int min)
{
	current_value_min = min;
}

int DeviceConditionDimming100::get_current_value_max()
{
	return current_value_max;
}

void DeviceConditionDimming100::set_current_value_max(int max)
{
	current_value_max = max;
}

void DeviceConditionDimming100::set_condition_value(QString s)
{
	DeviceCondition::set_condition_value(s.toInt() * 10);
}

void DeviceConditionDimming100::get_condition_value(QString& out)
{
	if (get_condition_value_min() == 0)
		out = "0";
	else
		out = QString("%1-%2").arg(get_condition_value_min()).arg(get_condition_value_max());
}

bool DeviceConditionDimming100::parseValues(const DeviceValues &values_list)
{
	DeviceValues::const_iterator it = values_list.constBegin();
	int trig_min = get_condition_value_min();
	int trig_max = get_condition_value_max();

	bool managed;
	while (it != values_list.constEnd())
	{
		int level;
		if (it.key() == LightingDevice::DIM_DEVICE_ON || it.key() == LightingDevice::DIM_DIMMER100_LEVEL)
			level = it.value().toInt();
		else if (it.key() == LightingDevice::DIM_DIMMER_LEVEL)
			level = dimmerLevelTo100(it.value().toInt());
		else
		{
			++it;
			continue;
		}

		managed = true;
		satisfied = (level >= trig_min && level <= trig_max);
		++it;
	}
	return managed;
}


DeviceConditionVolume::DeviceConditionVolume(DeviceConditionDisplayInterface* cond_display, QString trigger, QString where)
	: DeviceCondition(cond_display)
{
	if (trigger == "-1")
	{
		set_condition_value_min(-1);
		set_condition_value_max(-1);
	}
	else
	{
		QStringList parts = trigger.split('-');
		Q_ASSERT_X(parts.size() == 2, "DeviceConditionVolume::DeviceConditionVolume",
			"the trigger value must have 0 or 2 digit with - as separator");
		set_condition_value_min(parts[0].toInt());
		set_condition_value_max(parts[1].toInt());
	}

	set_current_value_min(get_condition_value_min());
	set_current_value_max(get_condition_value_max());
	setDevice(AmplifierDevice::createDevice(where));
	Draw();
}

void DeviceConditionVolume::set_condition_value_min(int s)
{
	min_val = s;
}

int DeviceConditionVolume::get_condition_value_min()
{
	return min_val;
}

void DeviceConditionVolume::set_condition_value_max(int s)
{
	max_val = s;
}

int DeviceConditionVolume::get_condition_value_max()
{
	return max_val;
}

int DeviceConditionVolume::get_current_value_min()
{
	return current_value_min;
}

void DeviceConditionVolume::set_current_value_min(int min)
{
	current_value_min = min;
}

int DeviceConditionVolume::get_current_value_max()
{
	return current_value_max;
}

void DeviceConditionVolume::set_current_value_max(int max)
{
	current_value_max = max;
}

int DeviceConditionVolume::get_min()
{
	return 0;
}

int DeviceConditionVolume::get_max()
{
	return 31;
}

bool DeviceConditionVolume::conditionChanged()
{
	return get_condition_value_min() != get_current_value_min() ||
		get_condition_value_max() != get_current_value_max();
}

void DeviceConditionVolume::get_condition_value(QString& out)
{
	if (get_condition_value_min() == -1)
		out = "-1";
	else
		out = QString("%1-%2").arg(get_condition_value_min()).arg(get_condition_value_max());
}

void DeviceConditionVolume::Up()
{
	int value_min = get_current_value_min();
	int value_max = get_current_value_max();
	qDebug("value_min = %d - value_max = %d", value_min, value_max);
	if (value_min == -1)
	{
		value_min = 0;
		value_max = 31;
	}
	else if (value_min == 0)
	{
		if (value_max == 31)
			value_max = 6;
		else
		{
			value_min = 7;
			value_max = 12;
		}
	}
	else if (value_min == 7)
	{
		value_min = 13;
		value_max = 22;
	}
	else
	{
		value_min = 23;
		value_max = 31;
	}
	qDebug("new value m = %d - M = %d", value_min, value_max);
	set_current_value_min(value_min);
	set_current_value_max(value_max);
	Draw();
}

void DeviceConditionVolume::Down()
{
	int value_min = get_current_value_min();
	int value_max = get_current_value_max();
	qDebug("value_min = %d - value_max = %d", value_min, value_max);
	if (value_min == 23)
	{
		value_min = 13;
		value_max = 22;
	}
	else if (value_min == 13)
	{
		value_min = 7;
		value_max = 12;
	}
	else if (value_min == 7)
	{
		value_min = 0;
		value_max = 6;
	}
	else if (value_min == 0)
	{
		if (value_max == 6)
			value_max = 31;
		else
		{
			value_min = -1;
			value_max = -1;
		}
	}
	qDebug("new value m = %d - M = %d", value_min, value_max);
	set_current_value_min(value_min);
	set_current_value_max(value_max);
	Draw();
}

void DeviceConditionVolume::onConditionSaved()
{
	set_condition_value_min(get_current_value_min());
	set_condition_value_max(get_current_value_max());
}

void DeviceConditionVolume::onConditionReset()
{
	set_current_value_min(get_condition_value_min());
	set_current_value_max(get_condition_value_max());
}

void DeviceConditionVolume::Draw()
{
	updateText(get_current_value_min(), get_current_value_max());
}

bool DeviceConditionVolume::parseValues(const DeviceValues &values_list)
{
	int trig_v_min = get_condition_value_min();
	int trig_v_max = get_condition_value_max();

	DeviceValues::const_iterator it = values_list.constBegin();

	bool managed = false;
	while (it != values_list.constEnd())
	{
		if (it.key() == AmplifierDevice::DIM_STATUS)
		{
			managed = true;
			if ((it.value().toInt() == 1 && trig_v_min == get_min() && trig_v_max == get_max()) ||
				(it.value().toInt() == 0 && trig_v_min == -1))
				satisfied = true;
			else
				satisfied = false;
		}
		else if (it.key() == AmplifierDevice::DIM_VOLUME)
		{
			managed = true;
			int volume = it.value().toInt();
			satisfied = (volume >= trig_v_min && volume <= trig_v_max);
		}
		++it;
	}
	return managed;
}


DeviceConditionTemperature::DeviceConditionTemperature(DeviceConditionDisplayInterface *cond_display, QString trigger,
	QString where, bool external, int openserver_id) : DeviceCondition(cond_display)
{
	// Temp condition is expressed in bticino format
	int temp_condition = trigger.toInt();

	temp_scale = static_cast<TemperatureScale>((*bt_global::config)[TEMPERATURE_SCALE].toInt());
	switch (temp_scale)
	{
	case CELSIUS:
		max_temp = bt2Celsius(CONDITION_MAX_TEMP);
		min_temp = bt2Celsius(CONDITION_MIN_TEMP);
		set_condition_value(bt2Celsius(temp_condition));
		break;
	case FAHRENHEIT:
		max_temp = bt2Fahrenheit(CONDITION_MAX_TEMP);
		min_temp = bt2Fahrenheit(CONDITION_MIN_TEMP);
		set_condition_value(bt2Fahrenheit(temp_condition));
		break;
	default:
		qWarning("Wrong temperature scale, defaulting to celsius");
		temp_scale = CELSIUS;
		max_temp = bt2Celsius(CONDITION_MAX_TEMP);
		min_temp = bt2Celsius(CONDITION_MIN_TEMP);
		set_condition_value(bt2Celsius(temp_condition));
	}
	step = 1;

	// The condition value and the current value are stored in Celsius or Fahrenheit
	set_current_value(DeviceCondition::get_condition_value());
	device *d = new NonControlledProbeDevice(where, external ? NonControlledProbeDevice::EXTERNAL :
		NonControlledProbeDevice::INTERNAL, openserver_id);
	setDevice(bt_global::add_device_to_cache(d));
	Draw();
}

int DeviceConditionTemperature::get_min()
{
	return min_temp;
}

int DeviceConditionTemperature::get_max()
{
	return max_temp;
}

int DeviceConditionTemperature::get_step()
{
	return step;
}

void DeviceConditionTemperature::Draw()
{
	updateText(get_current_value(), get_current_value());
}

void DeviceConditionTemperature::get_condition_value(QString& out)
{
	// transform an int value to a string in bticino 4-digit form
	int val = DeviceCondition::get_condition_value();
	int temp;
	switch (temp_scale)
	{
	case CELSIUS:
		temp = celsius2Bt(val);
		break;
	case FAHRENHEIT:
		temp = fahrenheit2Bt(val);
		break;
	default:
		qWarning("Wrong temperature scale, defaulting to celsius");
		temp = celsius2Bt(val);
	}
	// bticino_temp is stored in 4 digit format.
	out = QString("%1").arg(temp, 4, 10, QChar('0'));
}

bool DeviceConditionTemperature::parseValues(const DeviceValues &values_list)
{
	if (!values_list.contains(NonControlledProbeDevice::DIM_TEMPERATURE))
		return false;

	// get_condition_value() returns an int, which is Celsius or Fahrenheit
	int trig_v = DeviceCondition::get_condition_value();
	int temp = values_list[NonControlledProbeDevice::DIM_TEMPERATURE].toInt();

	qDebug("Temperature changed");
	qDebug("Current temperature %d", temp);
	int measured_temp;
	switch (temp_scale)
	{
	case CELSIUS:
		measured_temp = bt2Celsius(temp);
		break;
	case FAHRENHEIT:
		measured_temp = bt2Fahrenheit(temp);
		break;
	default:
		qWarning("Wrong temperature scale, defaulting to celsius");
		measured_temp = bt2Celsius(temp);
	}

	satisfied = (measured_temp >= (trig_v - 10) && measured_temp <= (trig_v + 10));
	return true;
}


DeviceConditionAux::DeviceConditionAux(DeviceConditionDisplayInterface* cond_display, QString trigger, QString where) :
	DeviceCondition(cond_display)
{
	set_condition_value(trigger);
	set_current_value(DeviceCondition::get_condition_value());
	setDevice(bt_global::add_device_to_cache(new AuxDevice(where)));
	Draw();
}

bool DeviceConditionAux::parseValues(const DeviceValues &values_list)
{
	if (values_list.contains(AuxDevice::DIM_STATUS))
	{
		if (DeviceCondition::get_condition_value() == values_list[AuxDevice::DIM_STATUS].toBool())
			satisfied = true;
		else
			satisfied = false;
		return true;
	}
	return false;
}

void DeviceConditionAux::Draw()
{
	updateText(get_current_value(), get_current_value());
}


int DeviceConditionAux::get_max()
{
	return 1;
}

void DeviceConditionAux::set_condition_value(QString s)
{
	int v = 0;
	if (s == "1")
		v = 1;
	else if (s == "0")
		v = 0;
	else
		qDebug() << "Unknown condition value " << s << " for device_condition_aux";
	DeviceCondition::set_condition_value(v);
}


