#pragma once

#include "Messages.hpp"

#include <Libipc/ServerConnection.hpp>

namespace UI::Protocols {

class ServerMessageReciever {
public:
	virtual CreateWindowResponse on_CreateWindowRequest(CreateWindowRequest& request, int pid_from);
	virtual void on_MakeWindowVisibleRequest(MakeWindowVisibleRequest& request, int pid_from);
	virtual void on_DestroyWindowRequest(DestroyWindowRequest& request, int pid_from);
	virtual ScreenSizeResponse on_ScreenSizeRequest(ScreenSizeRequest& request, int pid_from);
	virtual void on_SetPositionRequest(SetPositionRequest& request, int pid_from);
	virtual void on_InvalidateRequest(InvalidateRequest& request, int pid_from);
	virtual void on_CloseWindowResponse(CloseWindowResponse& response, int pid_from);
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
			auto message_pid = server_message.pid_from;
			auto type = GetType(message_bytes);

			if (type == MessageType::CreateWindowRequest) {
				auto message = CreateWindowRequest(message_bytes);
				auto response = m_reciever.on_CreateWindowRequest(message, message_pid).serialize();
				send_data(response.data(), response.size(), message_pid);
			}

			if (type == MessageType::MakeWindowVisibleRequest) {
				auto message = MakeWindowVisibleRequest(message_bytes);
				m_reciever.on_MakeWindowVisibleRequest(message, message_pid);
			}

			if (type == MessageType::DestroyWindowRequest) {
				auto message = DestroyWindowRequest(message_bytes);
				m_reciever.on_DestroyWindowRequest(message, message_pid);
			}

			if (type == MessageType::ScreenSizeRequest) {
				auto message = ScreenSizeRequest(message_bytes);
				auto response = m_reciever.on_ScreenSizeRequest(message, message_pid).serialize();
				send_data(response.data(), response.size(), message_pid);
			}

			if (type == MessageType::SetPositionRequest) {
				auto message = SetPositionRequest(message_bytes);
				m_reciever.on_SetPositionRequest(message, message_pid);
			}

			if (type == MessageType::InvalidateRequest) {
				auto message = InvalidateRequest(message_bytes);
				m_reciever.on_InvalidateRequest(message, message_pid);
			}

			if (type == MessageType::CloseWindowResponse) {
				auto message = CloseWindowResponse(message_bytes);
				m_reciever.on_CloseWindowResponse(message, message_pid);
			}
		}
	}

	void send_KeyRequest(const KeyRequest& request, int pid_to)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size(), pid_to);
	}

	void send_MouseClickRequest(const MouseClickRequest& request, int pid_to)
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

	void send_BackRequest(const BackRequest& request, int pid_to)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size(), pid_to);
	}

private:
	ServerMessageReciever& m_reciever;
};

}