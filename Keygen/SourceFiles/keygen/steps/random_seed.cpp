// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/random_seed.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/lottie_widget.h"
#include "styles/style_keygen.h"

#include <QtGui/QtEvents>

namespace Keygen::Steps {

RandomSeed::RandomSeed()
: Step(Type::Default) {
	using namespace rpl::mappers;

	setTitle(tr::lng_random_seed_title(Ui::Text::RichLangValue));
	setDescription(rpl::conditional(
		_limit.value() | rpl::map(!_1),
		tr::lng_random_seed_description(Ui::Text::RichLangValue),
		tr::lng_random_seed_continue(Ui::Text::RichLangValue)));
	initControls();
}

rpl::producer<int> RandomSeed::length() const {
	return _length.value();
}

QString RandomSeed::accumulated() const {
	return _accumulated;
}

void RandomSeed::initControls() {
	using namespace rpl::mappers;

	showLottie(
		":/gui/art/lottie/keyboard.tgs",
		st::randomLottieTop,
		st::randomLottieHeight);

	auto countText = _length.value() | rpl::map([](int value) {
		return QString::number(value);
	});
	const auto counter = Ui::CreateChild<Ui::FadeWrapScaled<Ui::FlatLabel>>(
		inner().get(),
		object_ptr<Ui::FlatLabel>(
			inner().get(),
			std::move(countText),
			st::randomCounter))->setDuration(
				st::coverDuration
			)->show(anim::type::instant);
	const auto counterLabel = Ui::CreateChild<Ui::FadeWrap<Ui::FlatLabel>>(
		inner().get(),
		object_ptr<Ui::FlatLabel>(
			inner().get(),
			tr::lng_random_seed_amount(),
			st::randomCounterLabel))->setDuration(
				st::coverDuration
			)->show(anim::type::instant);
	auto totalText = rpl::combine(
		tr::lng_random_seed_ready_total(),
		_length.value(),
		_limit.value()
	) | rpl::map([](QString phrase, int ready, int total) {
		return phrase.replace(
			"{ready}",
			QString::number(ready)
		).replace(
			"{total}",
			QString::number(total)
		);
	});
	const auto totalLabel = Ui::CreateChild<Ui::FadeWrap<Ui::FlatLabel>>(
		inner().get(),
		object_ptr<Ui::FlatLabel>(
			inner().get(),
			std::move(totalText),
			st::randomCounterLabel))->setDuration(
				st::coverDuration
			)->hide(anim::type::instant);

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		counter->resizeToWidth(size.width());
		counter->move(0, contentTop() + st::randomCounterTop);
		counterLabel->resizeToWidth(size.width());
		counterLabel->move(0, contentTop() + st::randomCounterLabelTop);
		totalLabel->resizeToWidth(size.width());
		totalLabel->move(
			0,
			contentTop() + st::randomReadyTotalLabelTop);
	}, counter->lifetime());

	rpl::combine(
		_limit.value() | rpl::filter(_1 > 0),
		inner()->heightValue(),
		_2
	) | rpl::start_with_next([=](int height) {
		counter->hide(anim::type::normal);
		counterLabel->hide(anim::type::normal);
		totalLabel->show(anim::type::normal);
		auto state = NextButtonState();
		state.text = tr::lng_random_seed_next(tr::now);
		requestNextButton(state);
	}, inner()->lifetime());

	inner()->events(
	) | rpl::filter([](not_null<QEvent*> e) {
		return e->type() == QEvent::KeyPress;
	}) | rpl::map([](not_null<QEvent*> e) {
		return static_cast<QKeyEvent*>(e.get());
	}) | rpl::filter([](not_null<QKeyEvent*> e) {
		return (e->key() != Qt::Key_Escape)
			&& (e->key() != Qt::Key_Enter)
			&& (e->key() != Qt::Key_Return)
			&& !e->text().isEmpty();
	}) | rpl::start_with_next([=](not_null<QKeyEvent*> e) {
		append(e->text());
	}, inner()->lifetime());
}

void RandomSeed::append(const QString &text) {
	_accumulated += text;
	const auto limit = _limit.current();
	if (limit && _accumulated.size() > limit) {
		_accumulated = _accumulated.mid(_accumulated.size() - limit);
	}
	_length = _accumulated.size();
}

void RandomSeed::showLimit(int limit) {
	Expects(limit > 0);

	_limit = limit;
}

} // namespace Keygen::Steps
