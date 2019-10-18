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
class Window;
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
	void verifyWords(std::vector<QString> &&words);
	void copyPublicKey();
	void savePublicKey();
	void savePublicKeyNow(const QByteArray &key);
	void startNewKey();

	[[nodiscard]] std::vector<QString> wordsByPrefix(
		const QString &word) const;
	[[nodiscard]] std::vector<QString> collectWords() const;

	const std::unique_ptr<Ui::Window> _window;
	const std::unique_ptr<Steps::Manager> _steps;
	const base::flat_set<QString> _validWords;

	State _state = State::Starting;
	QByteArray _randomSeed;
	int _minimalValidWordLength = 1;
	std::optional<Ton::UtilityKey> _key;
	std::optional<std::vector<QByteArray>> _verifying;

	rpl::lifetime _lifetime;

};

} // namespace Keygen
