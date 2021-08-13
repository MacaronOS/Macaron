#pragma once

#include "Messages.hpp"

#include <libipc/ClientConnection.hpp>

namespace UI::Protocols {

class ClientMessageReciever {
public:
	virtual void on_MousePressRequest(MousePressRequest& request);
	virtual void on_MouseMoveRequest(MouseMoveRequest& request);
	virtual CloseWindowResponse on_CloseWindowRequest(CloseWindowRequest& request);
	virtual void on_CreateWindowResponse(CreateWindowResponse& response);
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

			if (type == MessageType::MousePressRequest) {
				auto message = MousePressRequest(message_bytes);
				m_reciever.on_MousePressRequest(message);
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

			if (type == MessageType::CreateWindowResponse) {
				auto message = CreateWindowResponse(message_bytes);
				m_reciever.on_CreateWindowResponse(message);
			}
		}
	}

	void send_CreateWindowRequest(const CreateWindowRequest& request)
	{
		auto serialized = request.serialize();
		send_data(serialized.data(), serialized.size());
	}

private:
	ClientMessageReciever& m_reciever;
};

}