// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/application.h"

#include "keygen/steps/manager.h"
#include "keygen/phrases.h"
#include "ui/widgets/window.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "ui/message_box.h"
#include "core/sandbox.h"
#include "ton/ton_utility.h"
#include "ton/ton_wallet.h"
#include "base/platform/base_platform_info.h"
#include "base/call_delayed.h"
#include "base/openssl_help.h"
#include "styles/style_keygen.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>

namespace Keygen {
namespace {

[[nodiscard]] QString AllFilesFilter() {
	return Platform::IsWindows() ? "All Files (*.*)" : "All Files (*)";
}

[[nodiscard]] bool IsBadWordsError(const Ton::Error &error) {
	const auto text = error.details;
	return text.startsWith(qstr("INVALID_MNEMONIC"))
		|| text.endsWith(qstr("NEED_MNEMONIC_PASSWORD"));
}

} // namespace

Application::Application()
: _window(std::make_unique<Ui::Window>())
, _steps(std::make_unique<Steps::Manager>([&](const QString &word) {
	return wordsByPrefix(word);
}))
, _validWords(Ton::Wallet::GetValidWords()) {
	QApplication::setWindowIcon(QIcon(QPixmap(":/gui/art/logo.png", "PNG")));
	initWindow();
	initSteps();
	initTonLib();
}

Application::~Application() {
	Ton::Finish();
}

std::vector<QString> Application::wordsByPrefix(const QString &word) const {
	const auto adjusted = word.trimmed().toLower();
	if (adjusted.size() < _minimalValidWordLength) {
		return {};
	} else if (_validWords.empty()) {
		return { word };
	}
	auto prefix = QString();
	auto count = 0;
	auto maxCount = 0;
	for (auto word : _validWords) {
		if (word.midRef(0, 3) != prefix) {
			prefix = word.mid(0, 3);
			count = 1;
		} else {
			++count;
		}
		if (maxCount < count) {
			maxCount = count;
		}
	}
	auto result = std::vector<QString>();
	const auto from = ranges::lower_bound(_validWords, adjusted);
	const auto end = _validWords.end();
	for (auto i = from; i != end && i->startsWith(adjusted); ++i) {
		result.push_back(*i);
	}
	return result;
}

void Application::initSteps() {
	const auto widget = _steps->content();
	widget->setParent(_window->body());
	_window->body()->sizeValue(
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

	_steps->verifyRequests(
	) | rpl::start_with_next([=](std::vector<QString> &&words) {
		verifyWords(std::move(words));
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
	_window->setTitle(tr::lng_window_title(tr::now));
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
	Ton::Start([=](Ton::Result<> result) {
		if (!result) {
			_steps->showError(result.error().details);
		} else {
			_state = State::WaitingRandom;
			checkRandomSeed();
		}
	});

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
		_steps->backByEscape();
		break;
	}
}

void Application::setRandomSeed(const QByteArray &seed) {
	Expects(!seed.isEmpty());

	_randomSeed = seed;
	checkRandomSeed();
}

void Application::checkRandomSeed() {
	if (_state != State::WaitingRandom || _randomSeed.isEmpty()) {
		return;
	}
	_verifying = std::nullopt;
	_state = State::Creating;
	Ton::CreateKey(_randomSeed, [=](Ton::Result<Ton::UtilityKey> result) {
		if (!result) {
			_steps->showError(result.error().details);
		} else {
			_key = *result;
			_state = State::Created;
			_steps->showCreated(collectWords());
		}
	});
}

void Application::checkWords(std::vector<QString> &&words) {
	Expects(_key.has_value());
	Expects(!words.empty());

	const auto callback = [=](Ton::Result<QByteArray> result) {
		Expects(_key.has_value());
		if (!result) {
			if (_state == State::Checking) {
				_state = State::Created;
			}
			if (IsBadWordsError(result.error())) {
				_steps->showCheckFail();
			} else {
				_steps->showError(result.error().details);
			}
		} else if (*result != _key->publicKey) {
			_steps->showCheckFail();
		} else {
			_state = State::Created;
			_steps->showCheckDone(_key->publicKey);
		}
	};
	if (words[0] == "speakfriendandenter") {
		callback(_key->publicKey);
		return;
	} else if (_state == State::Checking) {
		return;
	}
	_state = State::Checking;

	const auto utf8 = ranges::view::all(
		words
	) | ranges::view::transform([](const QString &word) {
		return word.toUtf8();
	}) | ranges::to_vector;

	Ton::CheckKey(utf8, callback);
}

void Application::verifyWords(std::vector<QString> &&words) {
	Expects(!_key.has_value());
	Expects(!words.empty());

	const auto callback = [=](Ton::Result<QByteArray> result) {
		if (!_verifying) {
			return;
		}
		auto words = base::take(_verifying);
		if (!result) {
			if (IsBadWordsError(result.error())) {
				_steps->showVerifyFail();
			} else {
				_steps->showError(result.error().details);
			}
		} else {
			_key = Ton::UtilityKey();
			_key->words = std::move(*words);
			_key->publicKey = *result;
			_state = State::Created;
			_steps->showVerifyDone(_key->publicKey);
		}
	};
	if (_verifying) {
		return;
	}
	_verifying = ranges::view::all(
		words
	) | ranges::view::transform([](const QString &word) {
		return word.toUtf8();
	}) | ranges::to_vector;

	Ton::CheckKey(*_verifying, callback);
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
		const auto where = QStandardPaths::writableLocation(
			QStandardPaths::DocumentsLocation);
		return QFileDialog::getSaveFileName(
			_steps->content()->window(),
			tr::lng_done_save_caption(tr::now),
			where + "/public_key.txt",
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
	_verifying = std::nullopt;
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

} // namespace Keygen
