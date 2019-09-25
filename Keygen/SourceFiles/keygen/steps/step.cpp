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
#include "ui/effects/slide_animation.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/text/text_utilities.h"
#include "ui/lottie_widget.h"
#include "styles/style_keygen.h"
#include "styles/palette.h"

namespace Keygen::Steps {
namespace {

QImage AddImageMargins(const QImage &source, QMargins margins) {
	const auto pixelRatio = style::DevicePixelRatio();
	const auto was = source.size() / pixelRatio;
	const auto size = QRect({}, was).marginsAdded(margins).size();
	auto large = QImage(
		size * pixelRatio,
		QImage::Format_ARGB32_Premultiplied);
	large.setDevicePixelRatio(pixelRatio);
	large.fill(Qt::transparent);
	{
		auto p = QPainter(&large);
		p.drawImage(
			QRect({ margins.left(), margins.top() }, was),
			source);
	}
	return large;
}

} // namespace

struct Step::CoverAnimationData {
	Type type = Type();
	std::unique_ptr<Ui::LottieAnimation> lottie;
	int lottieTop = 0;
	int lottieHeight = 0;
	Ui::CrossFadeAnimation::Data title;
	Ui::CrossFadeAnimation::Data description;
	QPixmap content;
	int contentBottom = 0;
};

struct Step::SlideAnimationData {
	Type type = Type();
	std::unique_ptr<Ui::LottieAnimation> lottie;
	int lottieTop = 0;
	int lottieHeight = 0;
	QImage content;
	int contentTop = 0;
};

Step::CoverAnimation::~CoverAnimation() = default;
Step::SlideAnimation::~SlideAnimation() = default;

Step::Step(Type type)
: _type(type)
, _widget(std::make_unique<Ui::RpWidget>())
, _scroll(resolveScrollArea())
, _inner(resolveInner())
, _coverShown(type == Type::Intro ? 1. : 0.)
, _nextButton(resolveNextButton()) {
	initGeometry();
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
		: Ui::CreateChild<Ui::RpWidget>(_widget.get());
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

void Step::initGeometry() {
	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		if (_scroll) {
			_scroll->setGeometry({ QPoint(), size });
		}
		_inner->setGeometry(
			0,
			0,
			size.width(),
			std::max(desiredHeight(), size.height()));
	}, _inner->lifetime());
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
	}))->setDuration(0);

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
	if (_type == Type::Intro) {
		prepareCoverMask();

		inner()->paintRequest(
		) | rpl::start_with_next([=](QRect clip) {
			auto p = QPainter(inner());
			paintCover(p, 0, clip);
		}, inner()->lifetime());
	}

	_widget->paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		paintContent(clip);
	}, _widget->lifetime());
}

void Step::paintContent(QRect clip) {
	if (_slideAnimation.slide) {
		if (!_slideAnimation.slide->animating()) {
			showFinished();
			return;
		}
	} else if (!_coverAnimationValue.animating()) {
		if (_coverAnimation.title) {
			showFinished();
		}
		return;
	}

	auto p = QPainter(_widget.get());
	if (_slideAnimation.slide) {
		paintSlideAnimation(p, clip);
	} else {
		paintCoverAnimation(p, clip);
	}
}

void Step::paintCoverAnimation(QPainter &p, QRect clip) {
	const auto dt = _coverAnimationValue.value(1.);
	const auto isIntro = (_type == Type::Intro);
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
	_coverAnimation.title->paintFrame(
		p,
		positionReady,
		departingAlpha,
		arrivingAlpha);
	_coverAnimation.description->paintFrame(
		p,
		positionReady,
		departingAlpha,
		arrivingAlpha);

	paintCoverAnimationContent(
		p,
		_coverAnimation.contentWas,
		_coverAnimation.contentWasBottom,
		departingAlpha,
		showCoverMethod);
	paintCoverAnimationContent(
		p,
		_coverAnimation.contentNow,
		_coverAnimation.contentNowBottom,
		arrivingAlpha,
		1. - hideCoverMethod);
}

void Step::paintSlideAnimation(QPainter &p, QRect clip) {
	const auto left = (_widget->width() - _slideAnimation.slideWidth) / 2;
	const auto top = _slideAnimation.slideTop;
	_slideAnimation.slide->paintFrame(p, left, top, _widget->width());
}

void Step::showFinished() {
	_coverAnimationValue.stop();
	if (_type != Type::Intro
		&& (_coverAnimation.lottie || _slideAnimation.lottieNow)) {
		_lottie = _coverAnimation.lottie
			? std::move(_coverAnimation.lottie)
			: std::move(_slideAnimation.lottieNow);
		_lottie->attach(inner());
		_lottie->setOpacity(1.);
		_lottie->setGeometry(lottieGeometry(
			contentTop() + _lottieTop,
			_lottieHeight));
	}
	_coverAnimation = CoverAnimation();
	_slideAnimation = SlideAnimation();
	if (_scroll) {
		_scroll->show();
	}
	inner()->show();
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
		const auto iconsMaxSkip = st::coverMaxWidth
			- st::coverLeft.width()
			- st::coverRight.width();
		const auto iconsSkip = st::coverIconsMinSkip
			+ (((iconsMaxSkip - st::coverIconsMinSkip)
				* (coverWidth - st::windowSizeMin.width()))
					/ (st::coverMaxWidth - st::windowSizeMin.width()));
		const auto outside = iconsSkip
			+ st::coverLeft.width()
			+ st::coverRight.width()
			- coverWidth;
		left = -outside / 2;
		right = -outside - left;
	}
	if (top < 0) {
		const auto shown = float64(coverHeight) / st::coverHeight;
		auto leftShown = qRound(shown * (left + st::coverLeft.width()));
		left = leftShown - st::coverLeft.width();
		auto rightShown = qRound(shown * (right + st::coverRight.width()));
		right = rightShown - st::coverRight.width();
	}
	st::coverLeft.paint(
		p,
		left,
		coverHeight - st::coverLeft.height(),
		coverWidth);
	st::coverRight.paint(
		p,
		coverWidth - right - st::coverRight.width(),
		coverHeight - st::coverRight.height(),
		coverWidth);

	const auto iconLeft = (coverWidth - st::coverIcon.width()) / 2;
	const auto iconTop = top + st::coverIconTop;
	st::coverIcon.paint(p, iconLeft, iconTop, coverWidth);
}

void Step::paintCoverAnimationContent(
		QPainter &p,
		const QPixmap &snapshot,
		int snapshotBottom,
		float64 alpha,
		float64 howMuchHidden) {
	const auto pixelRatio = style::DevicePixelRatio();
	const auto snapshotWidth = snapshot.width() / pixelRatio;
	if (!snapshotWidth) {
		return;
	}
	const auto contentTop = anim::interpolate(
		snapshotBottom - (snapshot.height() / pixelRatio),
		snapshotBottom,
		howMuchHidden);
	p.setOpacity(alpha);
	p.drawPixmap(
		QPoint((inner()->width() - snapshotWidth) / 2, contentTop),
		snapshot);
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

rpl::producer<float64> Step::coverShown() const {
	return _coverShown.value();
}

not_null<Ui::RpWidget*> Step::widget() const {
	return _widget.get();
}

int Step::desiredHeight() const {
	return st::stepHeight;
}

Step::CoverAnimationData Step::prepareCoverAnimationData() {
	Expects(_title != nullptr);
	Expects(_description != nullptr);

	auto result = CoverAnimationData();
	result.type = _type;
	if (_lottie) {
		_lottie->detach();
		result.lottie = std::move(_lottie);
		result.lottieTop = contentTop() + _lottieTop;
		result.lottieHeight = _lottieHeight;
	}
	result.title = _title->crossFadeData(st::windowBg);
	result.description = _description->crossFadeData(st::windowBg);
	result.content = prepareCoverAnimationContent();
	result.contentBottom = animationContentBottom();
	return result;
}

QPixmap Step::prepareCoverAnimationContent() const {
	Expects(_description != nullptr);

	const auto otherTop = coverAnimationContentTop();
	const auto otherWidth = _description->naturalWidth();
	const auto otherRect = QRect(
		(inner()->width() - otherWidth) / 2,
		otherTop,
		otherWidth,
		animationContentBottom() - otherTop);

	return Ui::PixmapFromImage(grabForAnimation(otherRect));
}

Step::SlideAnimationData Step::prepareSlideAnimationData() {
	Expects(_title != nullptr);

	auto result = SlideAnimationData();
	result.type = _type;
	if (_lottie) {
		_lottie->detach();
		result.lottie = std::move(_lottie);
		result.lottieTop = contentTop() + _lottieTop;
		result.lottieHeight = _lottieHeight;
	}
	result.content = prepareSlideAnimationContent();
	const auto scrollTop = (_scroll ? _scroll->scrollTop() : 0);
	result.contentTop = slideAnimationContentTop() - scrollTop;
	return result;
}

QImage Step::prepareSlideAnimationContent() const {
	Expects(_title != nullptr);

	const auto contentTop = slideAnimationContentTop();
	const auto contentWidth = _description->naturalWidth();
	const auto contentRect = QRect(
		(inner()->width() - contentWidth) / 2,
		contentTop,
		contentWidth,
		animationContentBottom() - contentTop);
	return grabForAnimation(contentRect);
}

not_null<Ui::RpWidget*> Step::inner() const {
	return _inner;
}

int Step::contentTop() const {
	const auto desired = desiredHeight();
	return (std::max(desired, inner()->height()) - desired) / 2;
}

int Step::coverAnimationContentTop() const {
	Expects(_description != nullptr);

	return std::max(
		_description->y() + _description->height(),
		_scroll ? _scroll->scrollTop() : 0);
}

int Step::slideAnimationContentTop() const {
	Expects(_title != nullptr);

	return std::max(
		_title->y(),
		_scroll ? _scroll->scrollTop() : 0);
}

int Step::animationContentBottom() const {
	const auto bottom = contentTop() + desiredHeight();
	return _scroll
		? std::min(bottom, _scroll->scrollTop() + _scroll->height())
		: bottom;
}

QImage Step::grabForAnimation(QRect rect) const {
	return Ui::GrabWidgetToImage(inner(), rect);
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
		: (_nextButtonState.value() | rpl::type_erased());
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

void Step::showAnimated(not_null<Step*> previous, Direction direction) {
	if ((previous->_type == Type::Intro) != (_type == Type::Intro)) {
		showAnimatedCover(previous);
	} else {
		showAnimatedSlide(previous, direction);
	}
}

void Step::showAnimatedCover(not_null<Step*> previous) {
	prepareCoverMask();

	auto was = previous->prepareCoverAnimationData();
	auto now = prepareCoverAnimationData();
	if (was.lottie) {
		_coverAnimation.lottie = std::move(was.lottie);
		_coverAnimation.lottieTop = was.lottieTop;
		_coverAnimation.lottieHeight = was.lottieHeight;
	} else {
		_coverAnimation.lottie = std::move(now.lottie);
		_coverAnimation.lottieTop = now.lottieTop;
		_coverAnimation.lottieHeight = now.lottieHeight;
	}
	_coverAnimation.title = std::make_unique<Ui::CrossFadeAnimation>(
		st::windowBg,
		std::move(was.title),
		std::move(now.title));
	_coverAnimation.description = std::make_unique<Ui::CrossFadeAnimation>(
		st::windowBg,
		std::move(was.description),
		std::move(now.description));
	_coverAnimation.contentWas = std::move(was.content);
	_coverAnimation.contentWasBottom = was.contentBottom;
	_coverAnimation.contentNow = std::move(now.content);
	_coverAnimation.contentNowBottom = now.contentBottom;

	if (_coverAnimation.lottie) {
		_coverAnimation.lottie->attach(_widget.get());
	}

	inner()->hide();
	if (_scroll) {
		_scroll->hide();
	}
	_coverAnimationValue.start(
		[=] { coverAnimationCallback(); },
		0.,
		1.,
		st::coverDuration);
	coverAnimationCallback();
}

void Step::adjustSlideSnapshots(
		SlideAnimationData &was,
		SlideAnimationData &now) {
	const auto pixelRatio = style::DevicePixelRatio();
	const auto wasSize = was.content.size() / pixelRatio;
	const auto nowSize = now.content.size() / pixelRatio;
	const auto wasBottom = was.contentTop + wasSize.height();
	const auto nowBottom = now.contentTop + nowSize.height();
	const auto widthDelta = nowSize.width() - wasSize.width();
	auto wasMargins = QMargins();
	auto nowMargins = QMargins();
	wasMargins.setTop(std::max(was.contentTop - now.contentTop, 0));
	nowMargins.setTop(std::max(now.contentTop - was.contentTop, 0));
	wasMargins.setLeft(std::max(widthDelta / 2, 0));
	nowMargins.setLeft(-std::min(widthDelta / 2, 0));
	wasMargins.setRight(std::max(widthDelta - (widthDelta / 2), 0));
	nowMargins.setRight(-std::min(widthDelta - (widthDelta / 2), 0));
	wasMargins.setBottom(std::max(nowBottom - wasBottom, 0));
	nowMargins.setBottom(std::max(wasBottom - nowBottom, 0));
	was.content = AddImageMargins(was.content, wasMargins);
	now.content = AddImageMargins(now.content, nowMargins);
	was.contentTop = now.contentTop = std::min(
		was.contentTop,
		now.contentTop);
}

void Step::showAnimatedSlide(not_null<Step*> previous, Direction direction) {
	const auto pixelRatio = style::DevicePixelRatio();
	auto was = previous->prepareSlideAnimationData();
	auto now = prepareSlideAnimationData();
	_slideAnimation.slide = std::make_unique<Ui::SlideAnimation>();

	adjustSlideSnapshots(was, now);
	Assert(now.contentTop == was.contentTop);
	Assert(now.content.size() == was.content.size());

	_slideAnimation.slide->setSnapshots(
		Ui::PixmapFromImage(std::move(was.content)),
		Ui::PixmapFromImage(std::move(now.content)));
	_slideAnimation.slideTop = was.contentTop;
	_slideAnimation.slideWidth = was.content.width() / pixelRatio;
	_slideAnimation.lottieWas = std::move(was.lottie);
	_slideAnimation.lottieWasTop = was.lottieTop;
	_slideAnimation.lottieWasHeight = was.lottieHeight;
	_slideAnimation.lottieNow = std::move(now.lottie);
	_slideAnimation.lottieNowTop = now.lottieTop;
	_slideAnimation.lottieNowHeight = now.lottieHeight;

	if (_slideAnimation.lottieWas) {
		_slideAnimation.lottieWas->attach(_widget.get());
	}
	if (_slideAnimation.lottieNow) {
		_slideAnimation.lottieNow->attach(_widget.get());
	}

	inner()->hide();
	if (_scroll) {
		_scroll->hide();
	}
	_slideAnimation.slide->start(
		(direction == Direction::Backward),
		[=] { slideAnimationCallback(); },
		st::coverDuration);
	slideAnimationCallback();
}

void Step::coverAnimationCallback() {
	const auto dt = _coverAnimationValue.value(1.);
	const auto coverShown = (_type == Type::Intro)
		? anim::easeOutCirc(1., dt)
		: (1. - anim::linear(1., dt));
	_coverShown = coverShown;
	if (_coverAnimation.lottie) {
		const auto shown = (1. - coverShown);
		const auto height = shown * _coverAnimation.lottieHeight;
		_coverAnimation.lottie->setOpacity(shown);
		_coverAnimation.lottie->setGeometry(lottieGeometry(
			_coverAnimation.lottieTop + coverShown * st::coverHeight,
			height));
	}
	_widget->update();
}

void Step::slideAnimationCallback() {
	const auto state = _slideAnimation.slide->state();
	if (_slideAnimation.lottieWas) {
		const auto shown = (1. - state.leftProgress);
		const auto scale = state.leftAlpha;
		const auto fullHeight = _slideAnimation.lottieWasHeight;
		const auto height = scale * fullHeight;
		const auto delta = (1. - shown) * _slideAnimation.slideWidth;
		_slideAnimation.lottieWas->setOpacity(state.leftAlpha);
		_slideAnimation.lottieWas->setGeometry(lottieGeometry(
			_slideAnimation.lottieWasTop + (1. - scale) * fullHeight / 2.,
			height).translated(-delta, 0));
	}
	if (_slideAnimation.lottieNow) {
		const auto shown = state.rightProgress;
		const auto scale = state.rightAlpha;
		const auto fullHeight = _slideAnimation.lottieNowHeight;
		const auto height = scale * fullHeight;
		const auto delta = (1. - shown) * _slideAnimation.slideWidth;
		_slideAnimation.lottieNow->setOpacity(state.rightAlpha);
		_slideAnimation.lottieNow->setGeometry(lottieGeometry(
			_slideAnimation.lottieNowTop + (1. - scale) * fullHeight / 2.,
			height).translated(delta, 0));
	}
	_widget->update();
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

void Step::showLottie(const QString &path, int top, int height) {
	const auto lottieWidth = 2 * st::randomLottieHeight;
	const auto content = [&] {
		auto file = QFile(path);
		return file.open(QIODevice::ReadOnly)
			? file.readAll()
			: QByteArray();
	}();
	_lottie = std::make_unique<Ui::LottieAnimation>(
		inner(),
		content);
	_lottieTop = top;
	_lottieHeight = height;

	inner()->sizeValue(
	) | rpl::filter([=] {
		return (_lottie->parent() == inner());
	}) | rpl::start_with_next([=](QSize size) {
		_lottie->setGeometry(lottieGeometry(
			contentTop() + _lottieTop,
			_lottieHeight));
	}, inner()->lifetime());
}

QRect Step::lottieGeometry(int top, int height) const {
	const auto width = 2 * height;
	return {
		(inner()->width() - width) / 2,
		top,
		width,
		height
	};
}

} // namespace Keygen::Steps
