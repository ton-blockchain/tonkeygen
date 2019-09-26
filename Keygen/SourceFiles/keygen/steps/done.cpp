// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/done.h"

#include "keygen/phrases.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/dropdown_menu.h"
#include "ui/text/text_utilities.h"
#include "ui/public_key_label.h"
#include "styles/style_widgets.h"
#include "styles/style_keygen.h"
#include "styles/palette.h"

#include <QtGui/QtEvents>

namespace Keygen::Steps {

Done::Done(const QString &publicKey)
: Step(Type::Default) {
	setTitle(tr::lng_done_title(Ui::Text::RichLangValue), st::doneTitleTop);
	auto text = tr::lng_done_description(
		Ui::Text::RichLangValue
	) | rpl::map([](TextWithEntities &&value) {
		TextUtilities::ParseEntities(value, TextParseLinks);
		return std::move(value);
	});
	setDescription(std::move(text), st::doneDescriptionTop);
	initControls(publicKey);
	initShortcuts();
}

rpl::producer<> Done::copyKeyRequests() const {
	return _copyKeyRequests.events();
}

rpl::producer<> Done::saveKeyRequests() const {
	return _saveKeyRequests.events();
}

rpl::producer<> Done::newKeyRequests() const {
	return _newKeyRequests.events();
}

rpl::producer<> Done::verifyKeyRequests() const {
	return _verifyKeyRequests.events();
}

void Done::showMenu(not_null<Ui::IconButton*> toggle) {
	if (_menu) {
		return;
	}
	_menu.emplace(inner());

	const auto menu = _menu.get();
	toggle->installEventFilter(menu);

	const auto weak = Ui::MakeWeak(toggle);
	menu->setHiddenCallback([=] {
		menu->deleteLater();
		if (weak && _menu.get() == menu) {
			_menu = nullptr;
			toggle->setForceRippled(false);
		}
	});
	menu->setShowStartCallback(crl::guard(weak, [=] {
		if (_menu == menu) {
			toggle->setForceRippled(true);
		}
	}));
	menu->setHideStartCallback(crl::guard(weak, [=] {
		if (_menu == menu) {
			toggle->setForceRippled(false);
		}
	}));

	menu->addAction(tr::lng_done_generate_new(tr::now), [=] {
		_newKeyRequests.fire({});
	});
	menu->addAction(tr::lng_done_verify_key(tr::now), [=] {
		_verifyKeyRequests.fire({});
	});

	inner()->widthValue(
	) | rpl::start_with_next([=](int width) {
		menu->moveToRight(
			st::doneMenuPosition.x(),
			st::doneMenuPosition.y(),
			width);
	}, menu->lifetime());

	menu->showAnimated(Ui::PanelAnimation::Origin::TopRight);
}

void Done::initControls(const QString &publicKey) {
	const auto key = Ui::CreatePublicKeyLabel(inner().get(), publicKey);
	const auto copy = Ui::CreateChild<Ui::RoundButton>(
		inner().get(),
		tr::lng_done_copy_key(),
		st::defaultLightButton);
	const auto save = Ui::CreateChild<Ui::RoundButton>(
		inner().get(),
		tr::lng_done_save_key(),
		st::defaultLightButton);
	const auto menuToggle = Ui::CreateChild<Ui::IconButton>(
		inner().get(),
		st::menuToggle);

	copy->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	}) | rpl::start_to_stream(_copyKeyRequests, copy->lifetime());

	save->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	}) | rpl::start_to_stream(_saveKeyRequests, copy->lifetime());

	menuToggle->setClickedCallback([=] { showMenu(menuToggle); });

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		menuToggle->moveToRight(
			st::doneMenuTogglePosition.x(),
			st::doneMenuTogglePosition.y(),
			size.width());
		key->move(
			(size.width() - key->width()) / 2,
			contentTop() + st::doneKeyTop);
		copy->move(
			(size.width() - copy->width()) / 2,
			contentTop() + st::doneCopyTop);
		save->move(
			(size.width() - save->width()) / 2,
			contentTop() + st::doneSaveTop);
	}, inner()->lifetime());
}

void Done::initShortcuts() {
	inner()->events(
	) | rpl::filter([](not_null<QEvent*> e) {
		return e->type() == QEvent::KeyPress;
	}) | rpl::map([=](not_null<QEvent*> e) {
		return static_cast<QKeyEvent*>(e.get());
	}) | rpl::start_with_next([=](not_null<QKeyEvent*> e) {
		const auto modifiers = e->modifiers();
		if (modifiers & Qt::ControlModifier) {
			const auto key = e->key();
			if (key == Qt::Key_C) {
				_copyKeyRequests.fire({});
			} else if (key == Qt::Key_S) {
				_saveKeyRequests.fire({});
			}
		}
	}, inner()->lifetime());
}

QImage Done::grabForAnimation(QRect rect) const {
	const auto menuWasShown = _menu && !_menu->isHidden();
	if (menuWasShown) {
		_menu->QWidget::setParent(nullptr);
	}
	auto result = Step::grabForAnimation(rect);
	if (menuWasShown) {
		_menu->QWidget::setParent(inner());
	}
	return result;
}

} // namespace Keygen::Steps
