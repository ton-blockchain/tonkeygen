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
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/rp_widget.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {
namespace {

constexpr auto kSeedLengthMin = 50;
constexpr auto kSeedLengthMax = 200;

} // namespace

Manager::Manager()
: _content(std::make_unique<Ui::RpWidget>())
, _nextButton(
	_content.get(),
	object_ptr<Ui::RoundButton>(
		_content.get(),
		rpl::single(QString()),
		st::nextButton)) {
	_nextButton->entity()->clicks(
	) | rpl::start_with_next([=] {
		next();
	}, _nextButton->lifetime());
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
	showStep(std::make_unique<Intro>(), [=] {
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
	}, raw->widget()->lifetime());

	showStep(std::move(seed), [=] {
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
		showWords(std::move(words));
	};
	showStep(std::make_unique<Created>(), std::move(next), [=] {
		showRandomSeed();
	});
}

void Manager::showWords(std::vector<QString> &&words) {
}

void Manager::showCheck() {
}

void Manager::showDone(const QString &publicKey) {
}

void Manager::showStep(
		std::unique_ptr<Step> step,
		FnMut<void()> next,
		FnMut<void()> back) {
	_step = std::move(step);
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
	_nextButton->toggleOn(_step->nextButtonState(
	) | rpl::map([](const NextButtonState &state) {
		return !state.text.isEmpty();
	}));
	_nextButton->raise();

	_step->nextButtonState(
	) | rpl::start_with_next([=](NextButtonState state) {
		_nextButton->move(
			(_content->width() - _nextButton->width()) / 2,
			state.top);
		_nextState = state;
	}, inner->lifetime());

	_step->setFocus();
}

rpl::producer<QByteArray> Manager::generateRequests() const {
	return _generateRequests.events();
}

} // namespace Keygen::Steps
