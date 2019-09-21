// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/step.h"

#include "ui/rp_widget.h"
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/text/text_utilities.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {

Step::Step(Type type)
: _type(type)
, _widget(std::make_unique<Ui::RpWidget>())
, _scroll((type == Type::Scroll)
	? Ui::CreateChild<Ui::ScrollArea>(_widget.get(), st::scrollArea)
	: nullptr)
, _inner(_scroll
	? _scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll)).data()
	: _widget.get())
, _nextButton(_scroll
	? std::make_unique<Ui::FadeWrap<Ui::RoundButton>>(
		_inner,
		object_ptr<Ui::RoundButton>(
			_inner.get(),
			rpl::single(QString()),
			st::nextButton))
	: nullptr) {
	if (_scroll) {
		_widget->sizeValue(
		) | rpl::start_with_next([=](QSize size) {
			_scroll->setGeometry({ QPoint(), size });
			_inner->setGeometry(
				0,
				0,
				size.width(),
				std::max(desiredHeight(), size.height()));
		}, _scroll->lifetime());
	}
	if (_nextButton) {
		_nextButton->entity()->setText(_nextButtonState.value(
		) | rpl::filter([](const NextButtonState &state) {
			return !state.text.isEmpty();
		}) | rpl::map([](const NextButtonState &state) {
			return state.text;
		}));
		_nextButton->toggleOn(_nextButtonState.value(
		) | rpl::map([](const NextButtonState &state) {
			return !state.text.isEmpty();
		}));

		rpl::combine(
			_nextButtonState.value(),
			inner()->widthValue()
		) | rpl::start_with_next([=](NextButtonState state, int width) {
			_nextButton->move(
				(width - _nextButton->width()) / 2,
				state.top);
			_lastNextState = state;
		}, inner()->lifetime());
	}
}

Step::~Step() = default;

rpl::producer<> Step::nextClicks() const {
	if (!_nextButton) {
		return rpl::never<>();
	}
	return _nextButton->entity()->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	});
}

not_null<Ui::RpWidget*> Step::widget() const {
	return _widget.get();
}

int Step::desiredHeight() const {
	return st::stepHeight;
}

not_null<Ui::RpWidget*> Step::inner() const {
	return _inner;
}

int Step::contentTop() const {
	const auto desired = desiredHeight();
	return (std::max(desired, inner()->height()) - desired) / 2;
}

void Step::setTitle(rpl::producer<TextWithEntities> text, int top) {
	_title.emplace(
		inner(),
		std::move(text),
		(_type == Type::Intro) ? st::introTitle : st::stepTitle);

	if (!top) {
		top = (_type == Type::Intro)
			? st::introTitleTop
			: (_type == Type::Scroll)
			? st::scrollTitleTop
			: st::stepTitleTop;
	}
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_title->resizeToWidth(size.width());
		_title->move(0, contentTop() + top);
	}, _title->lifetime());
}

void Step::setDescription(rpl::producer<TextWithEntities> text, int top) {
	_description.emplace(
		inner(),
		std::move(text),
		(_type == Type::Intro) ? st::introDescription : st::stepDescription);

	if (!top) {
		top = (_type == Type::Intro)
			? st::introDescriptionTop
			: (_type == Type::Scroll)
			? st::scrollDescriptionTop
			: st::stepDescriptionTop;
	}
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_description->resizeToWidth(size.width());
		_description->move(0, contentTop() + top);
	}, _description->lifetime());
}

rpl::producer<NextButtonState> Step::nextButtonState() const {
	return (_type == Type::Scroll)
		? rpl::single(NextButtonState())
		: _nextButtonState.value();
}

void Step::setFocus() {
	inner()->setFocus();
}

rpl::lifetime &Step::lifetime() {
	return widget()->lifetime();
}

void Step::requestNextButton(NextButtonState state) {
	if (!state.top) {
		state.top = st::stepHeight - st::nextButtonAreaHeight;
	}
	state.top += contentTop();
	_nextButtonState = state;
}

void Step::ensureVisible(int top, int height) {
	Expects(_type == Type::Scroll);
	Expects(_scroll != nullptr);

	_scroll->scrollToY(top, top + height);
}

} // namespace Keygen::Steps
