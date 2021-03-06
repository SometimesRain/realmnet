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

#include <vector>
#include <Packets/PlayerText.h>
#include <Packets/Notification.h>

std::vector<String> messages;

class Plugin : public IPlugin
{
private:
	size_t index = 0;
	int indexTime = 0; //see how long ago the messages were scrolled

public:
	Plugin(IProxy& proxy) : IPlugin(proxy)
	{
		HookPacket(PlayerText);
	}

	void onPlayerText(PlayerText& p)
	{
		messages.push_back(p.text);
		while (messages.size() >= 50)
		{
			messages.erase(messages.begin());
		}
		index = messages.size();
	}

	void onUpArrow(bool down) //scroll forward
	{
		if (!down) return;
		if (proxy.getTime() - indexTime > 10000) index = messages.size();
		if (index == 0)
		{
			index = messages.size();
		}
		else
		{
			index--;
			proxy.sendPacket(Notification(proxy.playerData.objectId, messages[index], 0x00ff80));
		}
		indexTime = proxy.getTime();
	}

	void onDownArrow(bool down) //scroll back
	{
		if (!down || index == messages.size() || ++index == messages.size()) return;
		proxy.sendPacket(Notification(proxy.playerData.objectId, messages[index], 0x00ff80));
		indexTime = proxy.getTime();
	}

	void onRightArrow(bool down) //send selected
	{
		if (!down || index >= messages.size()) return;
		PlayerText p(messages[index]);
		onPlayerText(p);
		proxy.sendPacket(std::move(p));
		index = messages.size();
	}

	void onLeftArrow(bool down) //back to beginning
	{
		if (!down) return;
		index = messages.size();
	}
};

IPlugin* create(IProxy& proxy)
{
	return new Plugin(proxy);
}

void attach(const char** plugin, const char** author, void(IPlugin::**onDisconnect)(), HookedKey** keyHook)
{
	*plugin = "Resend";
	*author = "CrazyJani";
	HookKey(Up, None, false, onUpArrow);
	HookKey(Down, None, false, onDownArrow);
	HookKey(Right, None, false, onRightArrow);
	HookKey(Left, None, false, onLeftArrow);
}