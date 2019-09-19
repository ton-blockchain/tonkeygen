// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/application.h"

#include "keygen/steps/manager.h"
#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ton/ton_utility.h"
#include "styles/style_keygen.h"
#include "styles/palette.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

namespace Keygen {

Application::Application()
: _window(std::make_unique<Ui::RpWidget>())
, _steps(std::make_unique<Steps::Manager>())
, _path(QStandardPaths::writableLocation(QStandardPaths::DataLocation)) {
	initWindow();
	initSteps();
	initTonLib();
}

Application::~Application() {
	Ton::Finish();
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
		Expects(!seed.isEmpty());

		_randomSeed = seed;
		checkRandomSeed();
	}, _lifetime);

	_steps->checkRequests(
	) | rpl::start_with_next([=](std::vector<QString> &&words) {
		checkWords(std::move(words));
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
	}, errorHandler());
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
		const auto test = error.code;// #TODO show error
	};
}

void Application::checkRandomSeed() {
	if (_state != State::WaitingRandom || _randomSeed.isEmpty()) {
		return;
	}
	_state = State::Creating;
	Ton::CreateKey(_randomSeed, [=](Ton::Key key) {
		_key = key;
		_state = State::Created;

		auto list = ranges::view::all(
			_key->words
		) | ranges::view::transform([](const QByteArray &word) {
			return QString::fromUtf8(word);
		}) | ranges::to_vector;
		_steps->showCreated(std::move(list));
	}, errorHandler());
}

void Application::checkWords(std::vector<QString> &&words) {
	Expects(_key.has_value());

	if (_state == State::Checking) {
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

	Ton::CheckKey(key, [=] {
		_state = State::Created;
		_steps->showDone(_key->publicKey);
	}, [=](Ton::Error error) {
		_state = State::Created;
		errorHandler()(error);
	});
}

} // namespace Core
