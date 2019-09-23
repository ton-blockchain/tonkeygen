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
#include "ui/lottie_widget.h"
#include "styles/style_keygen.h"
#include "styles/palette.h"

namespace Keygen::Steps {

Step::CoverAnimation::~CoverAnimation() = default;

Step::Step(Type type)
: _type(type)
, _widget(std::make_unique<Ui::RpWidget>())
, _scroll(resolveScrollArea())
, _inner(resolveInner())
, _nextButton(resolveNextButton()) {
	initScroll();
	initNextButton();
	initCover();
}

Step::~Step() = default;

Ui::ScrollArea *Step::resolveScrollArea() {
	return (_type == Type::Scroll)
		? Ui::CreateChild<Ui::ScrollArea>(_widget.get(), st::scrollArea)
		: nullptr;
}

not_null<Ui::RpWidget*> Step::resolveInner() {
	return _scroll
		? _scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll)).data()
		: _widget.get();
}

std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>> Step::resolveNextButton() {
	return _scroll
		? std::make_unique<Ui::FadeWrap<Ui::RoundButton>>(
			_inner,
			object_ptr<Ui::RoundButton>(
				_inner.get(),
				rpl::single(QString()),
				st::nextButton))
		: nullptr;
}

void Step::initScroll() {
	if (!_scroll) {
		return;
	}
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

void Step::initNextButton() {
	if (!_nextButton) {
		return;
	}
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

void Step::initCover() {
	if (_type == Type::Scroll) {
		return;
	} else if (_type == Type::Intro) {
		prepareCoverMask();
	}
	inner()->paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		paintContent(clip);
	}, inner()->lifetime());
}

void Step::paintContent(QRect clip) {
	const auto isIntro = (_type == Type::Intro);
	const auto dt = _a_show.value(1.);
	if (!_a_show.animating()) {
		if (isIntro) {
			auto p = QPainter(inner());
			paintCover(p, 0, clip);
		}
		if (_coverAnimation.title) {
			showFinished();
		}
		return;
	}

	auto p = QPainter(inner());
	const auto progress = isIntro
		? anim::easeOutCirc(1., dt)
		: anim::linear(1., dt);
	const auto arrivingAlpha = progress;
	const auto departingAlpha = 1. - progress;
	const auto showCoverMethod = progress;
	const auto hideCoverMethod = progress;
	const auto coverTop = isIntro
		? anim::interpolate(-st::coverHeight, 0, showCoverMethod)
		: anim::interpolate(0, -st::coverHeight, hideCoverMethod);

	paintCover(p, coverTop, clip);

	const auto positionReady = isIntro ? showCoverMethod : hideCoverMethod;
	_coverAnimation.title->paintFrame(p, positionReady, departingAlpha, arrivingAlpha);
	_coverAnimation.description->paintFrame(p, positionReady, departingAlpha, arrivingAlpha);

	//paintContentSnapshot(p, _coverAnimation.contentSnapshotWas, departingAlpha, showCoverMethod);
	//paintContentSnapshot(p, _coverAnimation.contentSnapshotNow, arrivingAlpha, 1. - hideCoverMethod);
}

void Step::showFinished() {
	_a_show.stop();
	_coverAnimation = CoverAnimation();
	//_slideAnimation.reset();
	//prepareCoverMask();
	setFocus();
}

void Step::paintCover(QPainter &p, int top, QRect clip) {
	const auto coverWidth = inner()->width();
	const auto coverHeight = top + st::coverHeight;
	if (coverHeight <= 0
		|| !clip.intersects({ 0, 0, coverWidth, coverHeight })) {
		return;
	}
	if (coverHeight > 0) {
		p.drawPixmap(
			QRect(0, 0, coverWidth, coverHeight),
			_coverMask,
			QRect(
				0,
				-top * style::DevicePixelRatio(),
				_coverMask.width(),
				coverHeight * style::DevicePixelRatio()));
	}

	auto left = 0;
	auto right = 0;
	if (coverWidth < st::coverMaxWidth) {
		auto iconsMaxSkip = st::coverMaxWidth - st::coverLeft.width() - st::coverRight.width();
		auto iconsSkip = st::coverIconsMinSkip + (iconsMaxSkip - st::coverIconsMinSkip) * (coverWidth - st::windowSizeMin.width()) / (st::coverMaxWidth - st::windowSizeMin.width());
		auto outside = iconsSkip + st::coverLeft.width() + st::coverRight.width() - coverWidth;
		left = -outside / 2;
		right = -outside - left;
	}
	if (top < 0) {
		auto shown = float64(coverHeight) / st::coverHeight;
		auto leftShown = qRound(shown * (left + st::coverLeft.width()));
		left = leftShown - st::coverLeft.width();
		auto rightShown = qRound(shown * (right + st::coverRight.width()));
		right = rightShown - st::coverRight.width();
	}
	st::coverLeft.paint(p, left, coverHeight - st::coverLeft.height(), coverWidth);
	st::coverRight.paint(p, coverWidth - right - st::coverRight.width(), coverHeight - st::coverRight.height(), coverWidth);

	auto planeLeft = (coverWidth - st::coverIcon.width()) / 2 - st::coverIconLeft;
	auto planeTop = top + st::coverIconTop;
	st::coverIcon.paint(p, planeLeft, planeTop, coverWidth);
}

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
		if (_type == Type::Intro) {
			const auto heightMin = st::windowSizeMin.height();
			const auto topMin = st::introTitleTopMin;

			const auto realTop = contentTop() + top;
			const auto adjustedTop = std::max(realTop, st::introTitleTopMin);
			_title->move(0, adjustedTop);
		} else {
			_title->move(0, contentTop() + top);
		}
	}, _title->lifetime());
}

void Step::setDescription(rpl::producer<TextWithEntities> text, int top) {
	Expects(_title != nullptr);

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
	_title->geometryValue(
	) | rpl::start_with_next([=](QRect geometry) {
		_description->resizeToWidth(geometry.width());
		_description->move(0, geometry.y() + top);
	}, _description->lifetime());
}

rpl::producer<NextButtonState> Step::nextButtonState() const {
	return (_type == Type::Scroll)
		? rpl::single(NextButtonState())
		: _nextButtonState.value();
}

void Step::prepareCoverMask() {
	if (!_coverMask.isNull()) {
		return;
	}

	const auto maskWidth = style::DevicePixelRatio();
	const auto maskHeight = st::coverHeight * style::DevicePixelRatio();
	auto mask = QImage(maskWidth, maskHeight, QImage::Format_ARGB32_Premultiplied);
	auto maskInts = reinterpret_cast<uint32*>(mask.bits());
	Assert(mask.depth() == (sizeof(uint32) << 3));
	const auto maskIntsPerLineAdded = (mask.bytesPerLine() >> 2) - maskWidth;
	Assert(maskIntsPerLineAdded >= 0);
	const auto realHeight = static_cast<float64>(maskHeight - 1);
	for (auto y = 0; y != maskHeight; ++y) {
		auto color = anim::color(
			st::introCoverTopBg,
			st::introCoverBottomBg,
			y / realHeight);
		auto colorInt = anim::getPremultiplied(color);
		for (auto x = 0; x != maskWidth; ++x) {
			*maskInts++ = colorInt;
		}
		maskInts += maskIntsPerLineAdded;
	}
	_coverMask = Ui::PixmapFromImage(std::move(mask));
}

void Step::showAnimated() {
	prepareCoverMask();
	_a_show.start([=] { inner()->update(); }, 0., 1., st::coverDuration);
	inner()->update();
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

not_null<Ui::LottieAnimation*> Step::loadLottieAnimation(
		const QString &path) {
	const auto content = [&] {
		auto file = QFile(path);
		return file.open(QIODevice::ReadOnly)
			? file.readAll()
			: QByteArray();
	}();
	return _widget->lifetime().make_state<Ui::LottieAnimation>(
		inner(),
		content);
}

} // namespace Keygen::Steps
