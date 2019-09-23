// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/created.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/text/text_utilities.h"
#include "ui/lottie_widget.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {

Created::Created() : Step(Type::Default) {
	setTitle(tr::lng_created_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_created_description(Ui::Text::RichLangValue));
	initControls();
}

void Created::initControls() {
	const auto lottie = loadLottieAnimation(":/gui/art/lottie/paper.tgs");

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto lottieWidth = 2 * st::createdLottieHeight;
		lottie->setGeometry({
			(size.width() - lottieWidth) / 2,
			st::createdLottieTop,
			lottieWidth,
			st::createdLottieHeight
		});

		auto state = NextButtonState();
		state.text = tr::lng_created_next(tr::now);
		requestNextButton(state);
	}, inner()->lifetime());
}

} // namespace Keygen::Steps
