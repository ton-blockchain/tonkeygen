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
	? Ui::CreateChild<Ui::ScrollArea>(_widget.get())
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

int Step::desiredHeight() {
	return 0;
}

not_null<Ui::RpWidget*> Step::inner() const {
	return _inner;
}

void Step::setTitle(rpl::producer<TextWithEntities> text) {
	_title.emplace(inner(), std::move(text));

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_title->resizeToNaturalWidth(size.width());
		_title->move(
			(size.width() - _title->width()) / 2,
			(_type == Type::Scroll) ? 100 : (size.height() / 4));
	}, _title->lifetime());
}

void Step::setDescription(rpl::producer<TextWithEntities> text) {
	_description.emplace(inner(), std::move(text));

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_description->resizeToNaturalWidth(size.width());
		_description->move(
			(size.width() - _description->width()) / 2,
			(_type == Type::Scroll) ? 200 : (size.height() / 2));
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

void Step::requestNextButton(NextButtonState state) {
	_nextButtonState = state;
}

} // namespace Keygen::Steps
