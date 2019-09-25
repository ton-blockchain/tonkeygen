// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/manager.h"

#include "keygen/steps/intro.h"
#include "keygen/steps/random_seed.h"
#include "keygen/steps/created.h"
#include "keygen/steps/view.h"
#include "keygen/steps/check.h"
#include "keygen/steps/done.h"
#include "keygen/phrases.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "ui/toast/toast.h"
#include "ui/rp_widget.h"
#include "ui/message_box.h"
#include "base/platform/base_platform_file_utilities.h"
#include "base/call_delayed.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {
namespace {

constexpr auto kSeedLengthMin = 50;
constexpr auto kSeedLengthMax = 200;
constexpr auto kSaveKeyDoneDuration = crl::time(500);

} // namespace

Manager::Manager(Fn<std::vector<QString>(QString)> wordsByPrefix)
: _content(std::make_unique<Ui::RpWidget>())
, _nextButton(
	std::in_place,
	_content.get(),
	object_ptr<Ui::RoundButton>(
		_content.get(),
		rpl::single(QString()),
		st::nextButton))
, _backButton(
	std::in_place,
	_content.get(),
	object_ptr<Ui::IconButton>(_content.get(), st::topBackButton))
, _layerManager(_content.get())
, _wordsByPrefix(std::move(wordsByPrefix)) {
	initButtons();
}

void Manager::initButtons() {
	_nextButton->entity()->setClickedCallback([=] { next(); });
	_nextButton->setDuration(st::coverDuration);
	_nextButton->toggledValue(
	) | rpl::start_with_next([=](bool toggled) {
		_nextButtonShown.start(
			[=] { moveNextButton(); },
			toggled ? 0. : 1.,
			toggled ? 1. : 0.,
			st::coverDuration);
	}, _nextButton->lifetime());
	_nextButtonShown.stop();

	_backButton->entity()->setClickedCallback([=] { back(); });
	_backButton->toggle(false, anim::type::instant);
	_backButton->setDuration(st::coverDuration);
	_backButton->move(0, 0);
}

Manager::~Manager() = default;

not_null<Ui::RpWidget*> Manager::content() const {
	return _content.get();
}

void Manager::next() {
	if (_next) {
		_next();
	}
}

void Manager::back() {
	if (_back) {
		_back();
	}
}

void Manager::showIntro() {
	showStep(std::make_unique<Intro>(), Direction::Forward, [=] {
		showRandomSeed();
	});
}

void Manager::showRandomSeed() {
	using namespace rpl::mappers;

	auto seed = std::make_unique<RandomSeed>();

	const auto raw = seed.get();
	raw->length(
	) | rpl::filter(
		_1 >= kSeedLengthMin
	) | rpl::take(
		1
	) | rpl::start_with_next([=] {
		raw->showLimit(kSeedLengthMax);
	}, raw->lifetime());

	showStep(std::move(seed), Direction::Forward, [=] {
		const auto text = raw->accumulated();
		if (text.size() >= kSeedLengthMin) {
			_generateRequests.fire(text.toUtf8());
		}
	}, [=] {
		showIntro();
	});
}

void Manager::showCreated(std::vector<QString> &&words) {
	auto next = [=, list = std::move(words)]() mutable {
		showWords(std::move(list), Direction::Forward);
	};
	showStep(
		std::make_unique<Created>(),
		Direction::Forward,
		std::move(next));
}

void Manager::showWords(std::vector<QString> &&words, Direction direction) {
	showStep(std::make_unique<View>(std::move(words)), direction, [=] {
		showCheck(Direction::Forward);
	});
}

void Manager::showCheck(Direction direction) {
	auto check = std::make_unique<Check>(_wordsByPrefix);

	const auto raw = check.get();

	raw->submitRequests(
	) | rpl::start_with_next([=] {
		next();
	}, raw->lifetime());

	showStep(std::move(check), direction, [=] {
		if (raw->checkAll()) {
			_checkRequests.fire(raw->words());
		}
	}, [=] {
		_actionRequests.fire(Action::ShowWordsBack);
	});
}

void Manager::showCheckDone(const QString &publicKey) {
	_layerManager.showBox(Box([=](not_null<Ui::GenericBox*> box) {
		Ui::InitMessageBox(
			box,
			tr::lng_check_good_title(),
			tr::lng_check_good_text(Ui::Text::RichLangValue));
		box->addButton(
			tr::lng_check_good_next(),
			[=] { showDone(publicKey); });
	}));
}

void Manager::showCheckFail() {
	_layerManager.showBox(Box([=](not_null<Ui::GenericBox*> box) {
		Ui::InitMessageBox(
			box,
			tr::lng_check_bad_title(),
			tr::lng_check_bad_text(Ui::Text::RichLangValue));
		box->addButton(
			tr::lng_check_bad_try_again(),
			[=] { box->closeBox(); });
		box->addButton(tr::lng_check_bad_view_words(), [=] { back(); });

		const auto weak = Ui::MakeWeak(_step->widget());
		box->boxClosing(
		) | rpl::filter([=] {
			return weak != nullptr;
		}) | rpl::start_with_next([=] {
			_step->setFocus();
		}, box->lifetime());
	}));
}

void Manager::showDone(const QString &publicKey) {
	auto done = std::make_unique<Done>(publicKey);
	done->copyKeyRequests(
	) | rpl::map([] {
		return Action::CopyKey;
	}) | rpl::start_to_stream(_actionRequests, done->lifetime());
	done->saveKeyRequests(
	) | rpl::map([] {
		return Action::SaveKey;
	}) | rpl::start_to_stream(_actionRequests, done->lifetime());
	done->newKeyRequests(
	) | rpl::start_with_next([=] {
		confirmNewKey();
	}, done->lifetime());
	done->verifyKeyRequests(
	) | rpl::start_with_next([=] {
		showCheck(Direction::Backward);
	}, done->lifetime());
	showStep(std::move(done), Direction::Forward);
}

void Manager::showCopyKeyDone() {
	Ui::Toast::Show(_content.get(), tr::lng_done_to_clipboard(tr::now));
}

void Manager::showSaveKeyDone(const QString &path) {
	auto config = Ui::Toast::Config();
	config.text = tr::lng_done_to_file(tr::now);
	config.durationMs = kSaveKeyDoneDuration;
	Ui::Toast::Show(_content.get(), config);
	const auto delay = st::toastFadeInDuration
		+ kSaveKeyDoneDuration
		+ st::toastFadeOutDuration;
	base::call_delayed(delay, _content.get(), [=] {
		base::Platform::ShowInFolder(path);
	});
}

void Manager::showSaveKeyFail() {
	showError("Could not write this file :(");
}

void Manager::showError(const QString &text) {
	_layerManager.showBox(Box([=](not_null<Ui::GenericBox*> box) {
		Ui::InitMessageBox(
			box,
			rpl::single(QString("Error")),
			rpl::single(Ui::Text::WithEntities(text)));
		box->addButton(
			rpl::single(QString("OK")),
			[=] { box->closeBox(); });
	}));
}

void Manager::showStep(
		std::unique_ptr<Step> step,
		Direction direction,
		FnMut<void()> next,
		FnMut<void()> back) {
	_layerManager.hideAll();

	std::swap(_step, step);
	_next = std::move(next);
	_back = std::move(back);

	const auto inner = _step->widget();
	inner->setParent(_content.get());
	_content->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		inner->setGeometry({ QPoint(), size });
	}, inner->lifetime());
	inner->show();

	_nextButton->entity()->setText(_step->nextButtonState(
	) | rpl::filter([](const NextButtonState &state) {
		return !state.text.isEmpty();
	}) | rpl::map([](const NextButtonState &state) {
		return state.text;
	}));
	_step->nextButtonState(
	) | rpl::start_with_next([=](const NextButtonState &state) {
		_nextButton->toggle(!state.text.isEmpty(), anim::type::normal);
	}, _step->lifetime());
	if (!step) {
		_nextButton->finishAnimating();
		_nextButtonShown.stop();
	}
	_backButton->toggle(_back != nullptr, anim::type::normal);
	_nextButton->raise();
	_backButton->raise();
	_layerManager.raise();

	rpl::combine(
		_step->nextButtonState(),
		_content->widthValue()
	) | rpl::start_with_next([=](NextButtonState state, int width) {
		if (state.text.isEmpty()) {
			_lastNextState.text = QString();
		} else {
			_nextButton->resizeToWidth(state.width
				? state.width
				: st::nextButton.width);
			_lastNextState = state;
		}
		moveNextButton();
	}, _step->lifetime());

	_step->nextClicks(
	) | rpl::start_with_next([=] {
		this->next();
	}, _step->lifetime());

	_step->coverShown(
	) | rpl::start_with_next([=](float64 shown) {
		_backButton->move(0, st::coverHeight * shown);
	}, _step->lifetime());

	if (step) {
		_step->showAnimated(step.get(), direction);
	} else {
		_step->setFocus();
	}
}

void Manager::moveNextButton() {
	const auto shown = _nextButton->toggled();
	const auto progress = _nextButtonShown.value(shown ? 1. : 0.);
	const auto shownTop = _lastNextState.top;
	const auto hiddenTop = shownTop + 2 * st::nextButton.height;
	_nextButton->move(
		(_content->width() - _nextButton->width()) / 2,
		anim::interpolate(hiddenTop, shownTop, progress));
}

void Manager::confirmNewKey() {
	_layerManager.showBox(Box([=](not_null<Ui::GenericBox*> box) {
		Ui::InitMessageBox(
			box,
			tr::lng_done_new_title(),
			tr::lng_done_new_text(Ui::Text::RichLangValue));
		box->addButton(
			tr::lng_done_new_ok(),
			[=] { _actionRequests.fire(Action::NewKey); });
		box->addButton(tr::lng_done_new_cancel(), [=] { box->closeBox(); });
	}));
}

rpl::producer<QByteArray> Manager::generateRequests() const {
	return _generateRequests.events();
}

rpl::producer<std::vector<QString>> Manager::checkRequests() const {
	return _checkRequests.events();
}

rpl::producer<Manager::Action> Manager::actionRequests() const {
	return _actionRequests.events();
}

} // namespace Keygen::Steps
