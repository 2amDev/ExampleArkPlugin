#include <Windows.h>
#include <iostream>
#include <vector>

class PlayerDataManager
{

public:
	void Insert(std::wstring declaredIP)
	{
		ConnectedPlayer thisPlayer;
		thisPlayer.DeclaredIP = declaredIP;
		ConnectedPlayersList.push_back(thisPlayer);
	}
	void Remove(std::wstring declaredIP)
	{
		for (int i = 0; i < ConnectedPlayersList.size(); i++)
		{
			auto thisPlayer = ConnectedPlayersList.at(i);
			if (thisPlayer.DeclaredIP == declaredIP)
				ConnectedPlayersList.erase(ConnectedPlayersList.begin() + i);
		}
	}
	bool Exists(std::wstring declaredIP)
	{
		for (ConnectedPlayer player : ConnectedPlayersList)
			if (player.DeclaredIP == declaredIP)
				return true;

		return false;
	}


private:
	struct ConnectedPlayer
	{
		std::wstring DeclaredIP;
	};

	std::vector<ConnectedPlayer> ConnectedPlayersList;
};