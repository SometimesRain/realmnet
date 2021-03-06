#pragma region Header
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

#define PLUGIN
#include <Interface/IPlugin.h>
#include <GameData/Constants.h>
#pragma endregion

#include <chrono>
#include <thread>
#include <Packets/UseItem.h>
#include <Packets/Notification.h>
#include <GameData/XmlData.h>

class Plugin : public IPlugin
{
private:
	bool connected;

	void countdown(int duration, int notifEvery)
	{
		std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
		char* timeFormatted = new char[6]; //max 3 digits before decimal point
		for (int i = 0; i < duration / notifEvery + 1; i++)
		{
			float timerValue = (duration - notifEvery * i) / 1000.0f; //e.g. 1500 -> 1.5
			float progress = 1 - (notifEvery * i / (float)duration);
			int color;
			if (progress > 0.5f)
				color = (int)(0xff * 2 * (1 - progress)) * 0x10000 + 0x00ff00; //green to yellow (red 0, green 255 -> red 255, green 255)
			else
				color = (int)(0xff * 2 * progress) * 0x100 + 0xff0000; //yellow to red (red 255, green 255 -> red 255, green 0)
			sprintf(timeFormatted, "%.1f", timerValue);

			std::this_thread::sleep_until(start + std::chrono::milliseconds(i* notifEvery));

			if (!connected)
				break;

			proxy.sendPacket(Notification(proxy.playerData.objectId, timeFormatted, color));
		}
		delete[] timeFormatted;
	}

public:
	Plugin(IProxy& proxy) : IPlugin(proxy), connected(true)
	{
		HookPacket(UseItem);
	}

	void onUseItem(UseItem& p)
	{
		int buffDuration = XmlData::typeToItem(p.slotObject.objectType)->buffDuration;
		if (buffDuration > 0)
		{
			std::thread runTimer(&Plugin::countdown, this, buffDuration, 500);
			runTimer.detach();
		}
	}

	void onDisconnect()
	{
		connected = false;
	}
};

IPlugin* create(IProxy& proxy)
{
	return new Plugin(proxy);
}

void attach(const char** plugin, const char** author, void(IPlugin::**onDisconnect)(), HookedKey** keyHook)
{
	*plugin = "AbilityTimer";
	*author = "CrazyJani";
	LinkDisconnectCallback();
}