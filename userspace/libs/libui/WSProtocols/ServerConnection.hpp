#pragma once

#include "Messages.hpp"

#include <libipc/ServerConnection.hpp>

namespace UI::Protocols {

class ServerMessageReciever {
public:
	virtual CreateWindowResponse on_CreateWindowRequest(CreateWindowRequest& request);
	virtual void on_InvalidateRequest(InvalidateRequest& request);
	virtual void on_CloseWindowResponse(CloseWindowResponse& response);
};

class ServerConnection : public IPC::ServerConnection {
public:
	ServerConnection(const String& endpoint, ServerMessageReciever& reciever)
		: IPC::ServerConnection(endpoint)
		, m_reciever(reciever)
	{
	}

	void process_messages()
	{
		pump();
		auto recieved_messages_bytes = take_over_messages();
		for (auto& server_message : recieved_messages_bytes) {
			auto message_bytes = server_message.message;
			auto type = GetType(message_bytes);

			if (type == MessageType::CreateWindowRequest) {
				auto message = CreateWindowRequest(message_bytes);
				auto response = m_reciever.on_CreateWindowRequest(message).serialize();
				send_data(response.data(), response.size(), server_message.pid_from);
			}

			if (type == MessageType::InvalidateRequest) {
				auto message = InvalidateRequest(message_bytes);
				m_reciever.on_InvalidateRequest(message);
			}

			if (type == MessageType::CloseWindowResponse) {
				auto message = CloseWindowResponse(message_bytes);
				m_reciever.on_CloseWindowResponse(message);
			}
		}
	}

	void send_MousePressRequest(const MousePressRequest& request, int pid_to)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size(), pid_to);
	}

	void send_MouseMoveRequest(const MouseMoveRequest& request, int pid_to)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size(), pid_to);
	}

	void send_CloseWindowRequest(const CloseWindowRequest& request, int pid_to)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size(), pid_to);
	}

private:
	ServerMessageReciever& m_reciever;
};

}