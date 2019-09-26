// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "ui/lottie_widget.h"

#include "ui/rp_widget.h"
#include "ui/style/style_core.h"
#include "lottie/lottie_single_player.h"

#include <QtGui/QPainter>

namespace Ui {

LottieAnimation::LottieAnimation(
	not_null<QWidget*> parent,
	const QByteArray &content)
: _widget(std::make_unique<RpWidget>(parent))
, _lottie(
	std::make_unique<Lottie::SinglePlayer>(
		content,
		Lottie::FrameRequest(),
		Lottie::Quality::High)) {
	_lottie->updates(
	) | rpl::start_with_next([=](Lottie::Update update) {
		const auto data = &update.data;
		if (const auto info = base::get_if<Lottie::Information>(data)) {
			_framesInLoop = info->framesCount;
		}
		_widget->update();
	}, _widget->lifetime());

	_widget->paintRequest(
	) | rpl::filter([=] {
		return _lottie->ready();
	}) | rpl::start_with_next([=] {
		paintFrame();
	}, _widget->lifetime());
}

LottieAnimation::~LottieAnimation() = default;

void LottieAnimation::setGeometry(QRect geometry) {
	_widget->setGeometry(geometry);
	_widget->update();
}

void LottieAnimation::setOpacity(float64 opacity) {
	_opacity = opacity;
	_widget->update();
}

not_null<QWidget*> LottieAnimation::parent() const {
	return _widget->parentWidget();
}

void LottieAnimation::detach() {
	_widget->setParent(nullptr);
}

void LottieAnimation::attach(not_null<QWidget*> parent) {
	_widget->setParent(parent);
	_widget->show();
}

void LottieAnimation::paintFrame() {
	const auto pixelRatio = style::DevicePixelRatio();
	const auto request = Lottie::FrameRequest{
		_widget->size() * pixelRatio
	};
	const auto frame = _lottie->frameInfo(request);
	const auto width = frame.image.width() / pixelRatio;
	const auto height = frame.image.height() / pixelRatio;
	const auto left = (_widget->width() - width) / 2;
	const auto top = (_widget->height() - height) / 2;
	const auto destination = QRect{ left, top, width, height };

	auto p = QPainter(_widget.get());
	p.setOpacity(_opacity);
	p.drawImage(destination, frame.image);

	if (_startPlaying && frame.index == 0) {
		++_loop;
	}
	const auto index = ((_loop - 1) * _framesInLoop + frame.index);
	if (_startPlaying && (!_stopOnFrame || index < _stopOnFrame)) {
		_lottie->markFrameShown();
	}
}

void LottieAnimation::start() {
	_startPlaying = true;
	_widget->update();
}

void LottieAnimation::stopOnFrame(int frame) {
	_stopOnFrame = frame;
}

} // namespace Ui
