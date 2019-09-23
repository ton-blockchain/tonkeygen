// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/intro.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/text/text_utilities.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {

Intro::Intro() : Step(Type::Intro) {
	setTitle(tr::lng_intro_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_intro_description(Ui::Text::RichLangValue));
	initControls();
}

void Intro::initControls() {
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		auto state = NextButtonState();
		state.text = tr::lng_intro_next(tr::now);
		state.width = st::introNextWidth;
		requestNextButton(state);
	}, inner()->lifetime());
}

} // namespace Keygen::Steps
