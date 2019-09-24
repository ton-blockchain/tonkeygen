// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/application.h"

#include "keygen/steps/manager.h"
#include "keygen/phrases.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "ui/message_box.h"
#include "core/sandbox.h"
#include "ton/ton_utility.h"
#include "base/platform/base_platform_info.h"
#include "base/call_delayed.h"
#include "base/openssl_help.h"
#include "styles/style_keygen.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>

namespace Keygen {
namespace {

[[nodiscard]] QString AllFilesFilter() {
	return Platform::IsWindows() ? "All Files (*.*)" : "All Files (*)";
}

} // namespace

Application::Application()
: _window(std::make_unique<Ui::RpWidget>())
, _steps(std::make_unique<Steps::Manager>([&](const QString &word) {
	return isGoodWord(word);
}))
, _path(QStandardPaths::writableLocation(QStandardPaths::DataLocation)) {
	initWindow();
	initSteps();
	initTonLib();
}

Application::~Application() {
	Ton::Finish();
}

bool Application::isGoodWord(const QString &word) const {
	return !word.isEmpty()
		&& (_validWords.empty() || base::contains(_validWords, word));
}

void Application::initSteps() {
	const auto widget = _steps->content();
	widget->setParent(_window.get());
	_window->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		widget->setGeometry({ QPoint(), size });
	}, widget->lifetime());

	_steps->generateRequests(
	) | rpl::start_with_next([=](const QByteArray &seed) {
		setRandomSeed(seed);
	}, _lifetime);

	_steps->checkRequests(
	) | rpl::start_with_next([=](std::vector<QString> &&words) {
		checkWords(std::move(words));
	}, _lifetime);

	using Action = Steps::Manager::Action;
	_steps->actionRequests(
	) | rpl::start_with_next([=](Action action) {
		switch (action) {
		case Action::ShowWordsBack:
			return _steps->showWords(
				collectWords(),
				Steps::Direction::Backward);
		case Action::CopyKey: return copyPublicKey();
		case Action::SaveKey: return savePublicKey();
		case Action::NewKey: return startNewKey();
		}
		Unexpected("Action in actionRequests.");
	}, _lifetime);

	_steps->showIntro();
}

void Application::run() {
	_window->show();
	_window->setFocus();
}

void Application::initWindow() {
	_window->setWindowTitle(tr::lng_window_title(tr::now));
	_window->setMinimumSize(st::windowSizeMin);
	_window->setGeometry(style::centerrect(
		QApplication::desktop()->geometry(),
		QRect(QPoint(), st::windowSize)));

	updateWindowPalette();
	style::PaletteChanged(
	) | rpl::start_with_next([=] {
		updateWindowPalette();
	}, _window->lifetime());

	_window->events(
	) | rpl::start_with_next([=](not_null<QEvent*> e) {
		handleWindowEvent(e);
	}, _window->lifetime());
}

void Application::initTonLib() {
	if (!_path.isEmpty()) {
		QDir().mkpath(_path);
	}

	Ton::Start(_path, [=] {
		_state = State::WaitingRandom;
		checkRandomSeed();
		getValidWords();
	}, errorHandler());

	crl::async([] {
		// Init random, because it is slow.
		static_cast<void>(openssl::RandomValue<uint8>());
	});
}

void Application::updateWindowPalette() {
	auto palette = _window->palette();
	palette.setColor(QPalette::Window, st::windowBg->c);
	_window->setPalette(palette);
	Ui::ForceFullRepaint(_window.get());
}

void Application::handleWindowEvent(not_null<QEvent*> e) {
	if (e->type() == QEvent::KeyPress) {
		handleWindowKeyPress(static_cast<QKeyEvent*>(e.get()));
	}
}

void Application::handleWindowKeyPress(not_null<QKeyEvent*> e) {
	const auto key = e->key();
	const auto modifiers = e->modifiers();
	switch (key) {
	case Qt::Key_Q:
		if (modifiers & Qt::ControlModifier) {
			QApplication::quit();
		}
		break;
	case Qt::Key_Enter:
	case Qt::Key_Return:
		_steps->next();
		break;
	case Qt::Key_Escape:
		_steps->back();
		break;
	}
}

Fn<void(Ton::Error)> Application::errorHandler() {
	return [=](Ton::Error error) {
		const auto text = error.code;
		if (text.endsWith(qstr("Invalid mnemonic words or password (invalid checksum)"))) {
			if (_state == State::Checking) {
				_steps->showCheckFail();
			}
		} else {
			_steps->showError(text);
		}
	};
}

void Application::setRandomSeed(const QByteArray &seed) {
	Expects(!seed.isEmpty());

	_randomSeed = seed;
	checkRandomSeed();
}

void Application::getValidWords() {
	Ton::GetValidWords([=](std::vector<QByteArray> &&validWords) {
		_validWords = ranges::view::all(
			validWords
		) | ranges::view::transform([](const QByteArray &value) {
			return QString::fromUtf8(value);
		}) | ranges::to_vector;
	}, errorHandler());
}

void Application::checkRandomSeed() {
	if (_state != State::WaitingRandom || _randomSeed.isEmpty()) {
		return;
	}
	_state = State::Creating;
	Ton::CreateKey(_randomSeed, [=](Ton::Key key) {
		_key = key;
		_state = State::Created;
		_steps->showCreated(collectWords());
	}, errorHandler());
}

void Application::checkWords(std::vector<QString> &&words) {
	Expects(_key.has_value());
	Expects(!words.empty());

	const auto success = [=] {
		_state = State::Created;
		_steps->showCheckDone(_key->publicKey);
	};
	if (words[0] == "speakfriendandenter") {
		success();
		return;
	} else if (_state == State::Checking) {
		return;
	}
	_state = State::Checking;

	auto key = Ton::Key();
	key.publicKey = _key->publicKey;
	key.words = ranges::view::all(
		words
	) | ranges::view::transform([](const QString &word) {
		return word.toUtf8();
	}) | ranges::to_vector;

	Ton::CheckKey(key, success, [=](Ton::Error error) {
		errorHandler()(error);
		if (_state == State::Checking) {
			_state = State::Created;
		}
	});
}

void Application::copyPublicKey() {
	Expects(_key.has_value());

	QGuiApplication::clipboard()->setText(_key->publicKey);
	_steps->showCopyKeyDone();
}

void Application::savePublicKey() {
	Expects(_key.has_value());

	const auto key = _key->publicKey;
	const auto delay = st::defaultRippleAnimation.hideDuration;
	base::call_delayed(delay, _steps->content(), [=] {
		savePublicKeyNow(key);
	});
}

void Application::savePublicKeyNow(const QByteArray &key) {
	const auto filter = QString("Text Files (*.txt);;All Files (*.*)");
	const auto fail = crl::guard(_steps->content(), [=] {
		_steps->showSaveKeyFail();
	});
	const auto done = crl::guard(_steps->content(), [=](QString path) {
		_steps->showSaveKeyDone(path);
	});
	const auto getPath = [&] {
		return QFileDialog::getSaveFileName(
			_steps->content()->window(),
			tr::lng_done_save_caption(tr::now),
			"public_key.txt",
			filter);
	};
	const auto path = Core::Sandbox::Instance().runNestedEventLoop(
		_steps->content(),
		getPath);
	if (path.isEmpty()) {
		return;
	}
	auto file = QFile(path);
	if (file.open(QIODevice::WriteOnly) && file.write(key) == key.size()) {
		done(path);
	} else {
		fail();
	}
}

void Application::startNewKey() {
	_key = std::nullopt;
	if (_state != State::Starting) {
		_state = State::WaitingRandom;
	}
	_steps->showIntro();
}

std::vector<QString> Application::collectWords() const {
	Expects(_key.has_value());

	return ranges::view::all(
		_key->words
	) | ranges::view::transform([](const QByteArray &word) {
		return QString::fromUtf8(word);
	}) | ranges::to_vector;
}

} // namespace Core
