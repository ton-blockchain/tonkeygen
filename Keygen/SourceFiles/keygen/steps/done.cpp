// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/done.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"

namespace Keygen::Steps {

Done::Done(const QString &publicKey) : Step(Type::Default) {
	setTitle(tr::lng_done_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_done_description(Ui::Text::RichLangValue));
	initControls(publicKey);
}

void Done::initControls(const QString &publicKey) {
	const auto key = Ui::CreateChild<Ui::FlatLabel>(
		inner().get(),
		rpl::single(publicKey));
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		key->move(
			(size.width() - key->width()) / 2,
			(size.height() * 3 / 4));
	}, inner()->lifetime());
}

} // namespace Keygen::Steps
