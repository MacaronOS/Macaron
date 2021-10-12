#pragma once

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Runtime.hpp>
#include <Macaronlib/Serialization.hpp>
#include <Macaronlib/String.hpp>

namespace UI::Protocols {

enum class MessageType : int {
	CreateWindowRequest,
	CreateWindowResponse,
	MakeWindowVisibleRequest,
	DestroyWindowRequest,
	ScreenSizeRequest,
	ScreenSizeResponse,
	SetPositionRequest,
	InvalidateRequest,
	MouseClickRequest,
	MouseMoveRequest,
	CloseWindowRequest,
	CloseWindowResponse,
	BackRequest,
};

inline MessageType GetType(const Vector<unsigned char>& buffer)
{
    return static_cast<MessageType>(Decoder(buffer).get_int());
}

class CreateWindowRequest {
	static constexpr MessageType m_type = MessageType::CreateWindowRequest;

public:
	CreateWindowRequest(int widht, int height, int frameless, String titile)
		: m_widht(widht)
		, m_height(height)
		, m_frameless(frameless)
		, m_titile(move(titile))
	{
	}

	CreateWindowRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_widht = decoder.get_int();
		m_height = decoder.get_int();
		m_frameless = decoder.get_int();
		m_titile = decoder.get_String();
	}

	int widht() const { return m_widht; }
	int height() const { return m_height; }
	int frameless() const { return m_frameless; }
	const String& titile() const { return m_titile; }
	String take_titile() { return move(m_titile); }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_widht);
		encoder.push(m_height);
		encoder.push(m_frameless);
		encoder.push(m_titile);
		return encoder.done();
	}

private:
	int m_widht;
	int m_height;
	int m_frameless;
	String m_titile;
};

class CreateWindowResponse {
	static constexpr MessageType m_type = MessageType::CreateWindowResponse;

public:
	CreateWindowResponse(int window_id, int shared_buffer_id)
		: m_window_id(window_id)
		, m_shared_buffer_id(shared_buffer_id)
	{
	}

	CreateWindowResponse(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
		m_shared_buffer_id = decoder.get_int();
	}

	int window_id() const { return m_window_id; }
	int shared_buffer_id() const { return m_shared_buffer_id; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		encoder.push(m_shared_buffer_id);
		return encoder.done();
	}

private:
	int m_window_id;
	int m_shared_buffer_id;
};

class MakeWindowVisibleRequest {
	static constexpr MessageType m_type = MessageType::MakeWindowVisibleRequest;

public:
	MakeWindowVisibleRequest(int widnow_id, int visibility)
		: m_widnow_id(widnow_id)
		, m_visibility(visibility)
	{
	}

	MakeWindowVisibleRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_widnow_id = decoder.get_int();
		m_visibility = decoder.get_int();
	}

	int widnow_id() const { return m_widnow_id; }
	int visibility() const { return m_visibility; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_widnow_id);
		encoder.push(m_visibility);
		return encoder.done();
	}

private:
	int m_widnow_id;
	int m_visibility;
};

class DestroyWindowRequest {
	static constexpr MessageType m_type = MessageType::DestroyWindowRequest;

public:
	DestroyWindowRequest(int widnow_id)
		: m_widnow_id(widnow_id)
	{
	}

	DestroyWindowRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_widnow_id = decoder.get_int();
	}

	int widnow_id() const { return m_widnow_id; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_widnow_id);
		return encoder.done();
	}

private:
	int m_widnow_id;
};

class ScreenSizeRequest {
	static constexpr MessageType m_type = MessageType::ScreenSizeRequest;

public:
	ScreenSizeRequest()
	{
	}

	ScreenSizeRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
	}


	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		return encoder.done();
	}

private:
};

class ScreenSizeResponse {
	static constexpr MessageType m_type = MessageType::ScreenSizeResponse;

public:
	ScreenSizeResponse(int width, int height)
		: m_width(width)
		, m_height(height)
	{
	}

	ScreenSizeResponse(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_width = decoder.get_int();
		m_height = decoder.get_int();
	}

	int width() const { return m_width; }
	int height() const { return m_height; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_width);
		encoder.push(m_height);
		return encoder.done();
	}

private:
	int m_width;
	int m_height;
};

class SetPositionRequest {
	static constexpr MessageType m_type = MessageType::SetPositionRequest;

public:
	SetPositionRequest(int window_id, int left, int top)
		: m_window_id(window_id)
		, m_left(left)
		, m_top(top)
	{
	}

	SetPositionRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
		m_left = decoder.get_int();
		m_top = decoder.get_int();
	}

	int window_id() const { return m_window_id; }
	int left() const { return m_left; }
	int top() const { return m_top; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		encoder.push(m_left);
		encoder.push(m_top);
		return encoder.done();
	}

private:
	int m_window_id;
	int m_left;
	int m_top;
};

class InvalidateRequest {
	static constexpr MessageType m_type = MessageType::InvalidateRequest;

public:
	InvalidateRequest(int window_id, int x, int y, int width, int height)
		: m_window_id(window_id)
		, m_x(x)
		, m_y(y)
		, m_width(width)
		, m_height(height)
	{
	}

	InvalidateRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
		m_x = decoder.get_int();
		m_y = decoder.get_int();
		m_width = decoder.get_int();
		m_height = decoder.get_int();
	}

	int window_id() const { return m_window_id; }
	int x() const { return m_x; }
	int y() const { return m_y; }
	int width() const { return m_width; }
	int height() const { return m_height; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		encoder.push(m_x);
		encoder.push(m_y);
		encoder.push(m_width);
		encoder.push(m_height);
		return encoder.done();
	}

private:
	int m_window_id;
	int m_x;
	int m_y;
	int m_width;
	int m_height;
};

class MouseClickRequest {
	static constexpr MessageType m_type = MessageType::MouseClickRequest;

public:
	MouseClickRequest(int window_id, int x, int y)
		: m_window_id(window_id)
		, m_x(x)
		, m_y(y)
	{
	}

	MouseClickRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
		m_x = decoder.get_int();
		m_y = decoder.get_int();
	}

	int window_id() const { return m_window_id; }
	int x() const { return m_x; }
	int y() const { return m_y; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		encoder.push(m_x);
		encoder.push(m_y);
		return encoder.done();
	}

private:
	int m_window_id;
	int m_x;
	int m_y;
};

class MouseMoveRequest {
	static constexpr MessageType m_type = MessageType::MouseMoveRequest;

public:
	MouseMoveRequest(int widnow_id, int x, int y)
		: m_widnow_id(widnow_id)
		, m_x(x)
		, m_y(y)
	{
	}

	MouseMoveRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_widnow_id = decoder.get_int();
		m_x = decoder.get_int();
		m_y = decoder.get_int();
	}

	int widnow_id() const { return m_widnow_id; }
	int x() const { return m_x; }
	int y() const { return m_y; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_widnow_id);
		encoder.push(m_x);
		encoder.push(m_y);
		return encoder.done();
	}

private:
	int m_widnow_id;
	int m_x;
	int m_y;
};

class CloseWindowRequest {
	static constexpr MessageType m_type = MessageType::CloseWindowRequest;

public:
	CloseWindowRequest(int window_id, int widht, int height, String titile)
		: m_window_id(window_id)
		, m_widht(widht)
		, m_height(height)
		, m_titile(move(titile))
	{
	}

	CloseWindowRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
		m_widht = decoder.get_int();
		m_height = decoder.get_int();
		m_titile = decoder.get_String();
	}

	int window_id() const { return m_window_id; }
	int widht() const { return m_widht; }
	int height() const { return m_height; }
	const String& titile() const { return m_titile; }
	String take_titile() { return move(m_titile); }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		encoder.push(m_widht);
		encoder.push(m_height);
		encoder.push(m_titile);
		return encoder.done();
	}

private:
	int m_window_id;
	int m_widht;
	int m_height;
	String m_titile;
};

class CloseWindowResponse {
	static constexpr MessageType m_type = MessageType::CloseWindowResponse;

public:
	CloseWindowResponse(int window_id, int decision)
		: m_window_id(window_id)
		, m_decision(decision)
	{
	}

	CloseWindowResponse(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
		m_decision = decoder.get_int();
	}

	int window_id() const { return m_window_id; }
	int decision() const { return m_decision; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		encoder.push(m_decision);
		return encoder.done();
	}

private:
	int m_window_id;
	int m_decision;
};

class BackRequest {
	static constexpr MessageType m_type = MessageType::BackRequest;

public:
	BackRequest(int window_id)
		: m_window_id(window_id)
	{
	}

	BackRequest(const Vector<unsigned char>& buffer)
	{
		Decoder decoder(buffer);
		decoder.skip(sizeof(MessageType));
		m_window_id = decoder.get_int();
	}

	int window_id() const { return m_window_id; }

	Vector<unsigned char> serialize() const
	{
		Encoder encoder {};
		encoder.push((int)m_type);
		encoder.push(m_window_id);
		return encoder.done();
	}

private:
	int m_window_id;
};

}
