#pragma once

#include "Messages.hpp"

#include <Libipc/ClientConnection.hpp>

namespace UI::Protocols {

class ClientMessageReciever {
public:
	virtual void on_KeyRequest(KeyRequest& request);
	virtual void on_MouseClickRequest(MouseClickRequest& request);
	virtual void on_MouseMoveRequest(MouseMoveRequest& request);
	virtual CloseWindowResponse on_CloseWindowRequest(CloseWindowRequest& request);
	virtual void on_BackRequest(BackRequest& request);
	virtual void on_CreateWindowResponse(CreateWindowResponse& response);
	virtual void on_ScreenSizeResponse(ScreenSizeResponse& response);
};

class ClientConnection : public IPC::ClientConnection {
public:
	ClientConnection(const String& endpoint, ClientMessageReciever& reciever)
		: IPC::ClientConnection(endpoint)
		, m_reciever(reciever)
	{
	}

	void process_messages()
	{
		pump();
		auto recieved_messages_bytes = take_over_messages();
		for (auto& message_bytes : recieved_messages_bytes) {
			auto type = GetType(message_bytes);

			if (type == MessageType::KeyRequest) {
				auto message = KeyRequest(message_bytes);
				m_reciever.on_KeyRequest(message);
			}

			if (type == MessageType::MouseClickRequest) {
				auto message = MouseClickRequest(message_bytes);
				m_reciever.on_MouseClickRequest(message);
			}

			if (type == MessageType::MouseMoveRequest) {
				auto message = MouseMoveRequest(message_bytes);
				m_reciever.on_MouseMoveRequest(message);
			}

			if (type == MessageType::CloseWindowRequest) {
				auto message = CloseWindowRequest(message_bytes);
				auto response = m_reciever.on_CloseWindowRequest(message).serialize();
				send_data(response.data(), response.size());
			}

			if (type == MessageType::BackRequest) {
				auto message = BackRequest(message_bytes);
				m_reciever.on_BackRequest(message);
			}

			if (type == MessageType::CreateWindowResponse) {
				auto message = CreateWindowResponse(message_bytes);
				m_reciever.on_CreateWindowResponse(message);
			}

			if (type == MessageType::ScreenSizeResponse) {
				auto message = ScreenSizeResponse(message_bytes);
				m_reciever.on_ScreenSizeResponse(message);
			}
		}
	}

	void send_CreateWindowRequest(const CreateWindowRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

	void send_MakeWindowVisibleRequest(const MakeWindowVisibleRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

	void send_DestroyWindowRequest(const DestroyWindowRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

	void send_ScreenSizeRequest(const ScreenSizeRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

	void send_SetPositionRequest(const SetPositionRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

	void send_InvalidateRequest(const InvalidateRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

private:
	ClientMessageReciever& m_reciever;
};

}