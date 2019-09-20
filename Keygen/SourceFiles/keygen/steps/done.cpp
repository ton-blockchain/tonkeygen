// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/done.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"
#include "styles/style_widgets.h"

namespace Keygen::Steps {

Done::Done(const QString &publicKey) : Step(Type::Default) {
	setTitle(tr::lng_done_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_done_description(Ui::Text::RichLangValue));
	initControls(publicKey);
}

rpl::producer<> Done::copyKeyRequests() const {
	return _copyKeyRequests.events();
}

rpl::producer<> Done::saveKeyRequests() const {
	return _saveKeyRequests.events();
}

void Done::initControls(const QString &publicKey) {
	const auto key = Ui::CreateChild<Ui::FlatLabel>(
		inner().get(),
		rpl::single(publicKey));
	const auto copy = Ui::CreateChild<Ui::RoundButton>(
		inner().get(),
		tr::lng_done_copy_key(),
		st::defaultLightButton);
	const auto save = Ui::CreateChild<Ui::RoundButton>(
		inner().get(),
		tr::lng_done_save_key(),
		st::defaultLightButton);

	copy->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	}) | rpl::start_to_stream(_copyKeyRequests, copy->lifetime());

	save->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	}) | rpl::start_to_stream(_saveKeyRequests, copy->lifetime());

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		key->move(
			(size.width() - key->width()) / 2,
			(size.height() * 5 / 8));
		copy->move(
			(size.width() - copy->width()) / 2,
			(size.height() * 3 / 4) - copy->height());
		save->move(
			(size.width() - save->width()) / 2,
			(size.height() * 3 / 4) + save->height());
	}, inner()->lifetime());
}

} // namespace Keygen::Steps
