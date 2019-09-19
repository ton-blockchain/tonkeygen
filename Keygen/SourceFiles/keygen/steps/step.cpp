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
#include "ui/text/text_utilities.h"

namespace Keygen::Steps {

Step::Step(Type type)
: _type(type)
, _widget(std::make_unique<Ui::RpWidget>())
, _scroll((type == Type::Scroll)
	? Ui::CreateChild<Ui::ScrollArea>(_widget.get())
	: nullptr)
, _inner(_scroll
	? _scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll)).data()
	: _widget.get()) {
}

Step::~Step() = default;

not_null<Ui::RpWidget*> Step::widget() const {
	return _widget.get();
}

not_null<Ui::RpWidget*> Step::inner() const {
	return _inner;
}

void Step::setTitle(rpl::producer<TextWithEntities> text) {
	_title.create(inner(), std::move(text));

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_title->resizeToNaturalWidth(size.width());
		_title->move(
			(size.width() - _title->width()) / 2,
			size.height() / 4);
	}, _title->lifetime());
}

void Step::setDescription(rpl::producer<TextWithEntities> text) {
	_description.create(inner(), std::move(text));

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_description->resizeToNaturalWidth(size.width());
		_description->move(
			(size.width() - _description->width()) / 2,
			size.height() / 2);
	}, _description->lifetime());
}

rpl::producer<NextButtonState> Step::nextButtonState() const {
	return _nextButtonState.value();
}

void Step::setFocus() {
	inner()->setFocus();
}

void Step::requestNextButton(NextButtonState state) {
	_nextButtonState = state;
}

} // namespace Keygen::Steps
