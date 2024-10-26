#pragma once
#include "Chunk.hpp"

#include "Player.hpp"

namespace wc {
	enum class GameMsg : uint32_t
	{
		Server_GetStatus,
		Server_GetPing,

		Client_Accepted,
		Client_AssignID,
		Client_RegisterWithServer,
		Client_UnregisterWithServer,

		Game_AddPlayer,
		Game_RemovePlayer,
		Game_UpdatePlayer,

		RequestChunk,
		SendChunk,
		GenerateChunk,
		BlockEdit,
	};

	class Server : public net::server_interface<GameMsg> {
	public:
		Server(const uint16_t& nPort) : net::server_interface<GameMsg>(nPort) {}
	
		std::unordered_map<uint32_t, PlayerDescription> m_mapPlayerRoster;
		std::vector<uint32_t> m_vGarbageIDs;
	
	protected:
		bool OnClientConnect(std::shared_ptr<net::connection<GameMsg>> client) override
		{
			// For now we will allow all 
			return true;
		}
	
		void OnClientValidated(std::shared_ptr<net::connection<GameMsg>> client) override
		{
			// Client passed validation check, so send them a message informing
			// them they can continue to communicate
			net::message<GameMsg> msg;
			msg.header.id = GameMsg::Client_Accepted;
			client->Send(msg);
		}
	
		void OnClientDisconnect(std::shared_ptr<net::connection<GameMsg>> client) override
		{
			if (client)
			{
				if (m_mapPlayerRoster.find(client->GetID()) == m_mapPlayerRoster.end())
				{
					// client never added to roster, so just let it disappear
				}
				else
				{
					m_vGarbageIDs.push_back(client->GetID());
				}
			}
	
		}
	
		void OnMessage(std::shared_ptr<net::connection<GameMsg>> client, net::message<GameMsg>& msg) override
		{
			if (!m_vGarbageIDs.empty())
			{
				for (auto pid : m_vGarbageIDs)
				{
					net::message<GameMsg> m;
					m.header.id = GameMsg::Game_RemovePlayer;
					m << pid;
					WC_INFO("{0} left the game", m_mapPlayerRoster[pid].name);
					m_mapPlayerRoster.erase(client->GetID());
					MessageAllClients(m);
				}
				m_vGarbageIDs.clear();
			}
	
	
			switch (msg.header.id)
			{
			case GameMsg::Client_RegisterWithServer:
			{
				PlayerDescription desc;
				msg >> desc;
				desc.nUniqueID = client->GetID();
				m_mapPlayerRoster.insert_or_assign(desc.nUniqueID, desc);
	
				net::message<GameMsg> msgSendID;
				msgSendID.header.id = GameMsg::Client_AssignID;
				msgSendID << desc.nUniqueID;
				MessageClient(client, msgSendID);
	
				net::message<GameMsg> msgAddPlayer;
				msgAddPlayer.header.id = GameMsg::Game_AddPlayer;
				msgAddPlayer << desc;
				MessageAllClients(msgAddPlayer);
	
				for (const auto& player : m_mapPlayerRoster)
				{
					net::message<GameMsg> msgAddOtherPlayers;
					msgAddOtherPlayers.header.id = GameMsg::Game_AddPlayer;
					msgAddOtherPlayers << player.second;
					MessageClient(client, msgAddOtherPlayers);
				}
	
				break;
			}
	
			case GameMsg::Client_UnregisterWithServer:
			{
				break;
			}
	
			case GameMsg::Game_UpdatePlayer:
			{
				// Simply bounce update to everyone except incoming client
				MessageAllClients(msg, client);
				break;
			}

			case GameMsg::BlockEdit:
			{
				// Simply bounce update to everyone except incoming client
				MessageAllClients(msg, client);
				break;
			}
			case GameMsg::RequestChunk:
			{
				net::message<GameMsg> msgSendChunk;
				glm::ivec3 pos;
				ChunkID chunkID;
				msg >> chunkID >> pos;
				std::string path = getChunkPath(pos);
				msgSendChunk.header.id = GameMsg::GenerateChunk; 
				if (std::filesystem::exists(path)) {
					std::ifstream file(path, std::ios::binary | std::ios::in);
					if (file.is_open()) {
						msgSendChunk.header.id = GameMsg::SendChunk;
						std::vector<std::pair<BlockID, uint16_t>> data;

						while (!file.eof()) {
							uint32_t block;
							uint16_t count;

							file >> block >> count;
							data.emplace_back(block, count);
						}
						file.close();
						data[data.size() - 1] = { 0,0 };
						for (int32_t i = data.size() - 1; i >= 0; i--) 
							msgSendChunk << data[i];						
						msgSendChunk << data.size() - 1;
					}
				}
				msgSendChunk << chunkID;


				MessageClient(client, msgSendChunk);
				break;
			}
			}
		}
		std::string worldName = "world";
		std::string getChunkPath(const glm::ivec3& pos) {
			return worldName + "/Chunk data/Island 0/r." + std::to_string(pos.x) + "." + std::to_string(pos.y) + "." + std::to_string(pos.z) + ".ewr";
		}

		std::vector<std::pair<BlockID, uint16_t>> Compress(const Chunk& chunk) {
			std::vector<std::pair<BlockID, uint16_t>> compressed;
			BlockID pBlockID = chunk.data[0][0][0];
			uint16_t count = 0;
		
			for (uint8_t y = 0; y < chunkSize; y++)
				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) // @TODO: optimize
					{
						BlockID block = chunk.data[x][y][z];
						if (block == pBlockID) count++;
						else {
							compressed.emplace_back(pBlockID, count);
							pBlockID = chunk.data[x][y][z];
							count = 1;
						}
					}
			compressed.emplace_back(pBlockID, count);
			return compressed;
		}
		
		void Decompress(const std::vector<std::pair<BlockID, uint16_t>>& blocks, Chunk& chunk)
		{
			uint16_t counter = 0;
			for (auto& block : blocks) {
				for (uint16_t i = 0; i < block.second; i++) {
					glm::ivec3 pos;
					// @TODO: Remove. Idk what happened. Strange linker error...
					{
						int iCounter = counter;
						int z = iCounter / (chunkSize * chunkSize);
						iCounter -= (z * chunkSize * chunkSize);
						int y = iCounter / chunkSize;
						int x = iCounter % chunkSize;
						pos = glm::ivec3(x, y, z);
					}
					uint8_t x = pos.x;
					uint8_t y = pos.z;
					uint8_t z = pos.y;
					chunk.data[x][y][z] = block.first;
					counter++;
				}
			}
		}	
	};
}