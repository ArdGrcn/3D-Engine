#include "Keyboard.h"

bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept
{
	return keystates[keycode];
}

std::optional<Keyboard::Event> Keyboard::ReadKey() noexcept
{
	if (keybuffer.size() > 0u)
	{
		Event e = keybuffer.front();
		keybuffer.pop();
		return e;
	}
	else
	{
		return std::nullopt;
	}
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return keybuffer.empty();
}

void Keyboard::FlushKey() noexcept
{
	keybuffer = std::queue<Event>();
}

std::optional<char> Keyboard::ReadChar() noexcept
{
	if (charbuffer.size() > 0u)
	{
		unsigned char charcode = charbuffer.front();
		charbuffer.pop();
		return charcode;
	}
	else
	{
		return std::nullopt;
	}
}

bool Keyboard::CharIsEmpty() const noexcept
{
	charbuffer.empty();
}

void Keyboard::FlushChar() noexcept
{
	charbuffer = std::queue<char>();
}

void Keyboard::Flush() noexcept
{
	FlushKey();
	FlushChar();
}

void Keyboard::EnableAutorepeat() noexcept
{
	autoRepeatEnabled = true;
}

void Keyboard::DisableAutorepeat() noexcept
{
	autoRepeatEnabled = false;
}

bool Keyboard::AutorepeatIsEnabled() const noexcept
{
	return autoRepeatEnabled;
}

void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	keystates[keycode] = true;
	keybuffer.push(Event())
}
