// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "ton/ton_utility.h"

class QEvent;
class QKeyEvent;

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Keygen {
namespace Steps {
class Manager;
} // namespace Steps

class Application final {
public:
	Application();
	Application(const Application &other) = delete;
	Application &operator=(const Application &other) = delete;
	~Application();

	void run();

private:
	enum class State {
		Starting,
		WaitingRandom,
		Creating,
		Created,
		Checking,
	};
	void initWindow();
	void initSteps();
	void initTonLib();
	void updateWindowPalette();
	void handleWindowEvent(not_null<QEvent*> e);
	void handleWindowKeyPress(not_null<QKeyEvent*> e);
	void setRandomSeed(const QByteArray &seed);
	void checkRandomSeed();
	void checkWords(std::vector<QString> &&words);
	void copyPublicKey();
	void savePublicKey();
	void savePublicKeyNow(const QByteArray &key);

	[[nodiscard]] Fn<void(Ton::Error)> errorHandler();

	const std::unique_ptr<Ui::RpWidget> _window;
	const std::unique_ptr<Steps::Manager> _steps;
	const QString _path;

	State _state = State::Starting;
	QByteArray _randomSeed;
	std::optional<Ton::Key> _key;

	rpl::lifetime _lifetime;

};

} // namespace Keygen
