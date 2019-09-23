// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

namespace Lottie {
class SinglePlayer;
} // namespace Lottie

namespace Ui {

class RpWidget;

class LottieAnimation final {
public:
	LottieAnimation(not_null<QWidget*> parent, const QByteArray &content);
	~LottieAnimation();

	void setGeometry(QRect geometry);
	void setOpacity(float64 opacity);

private:
	const std::unique_ptr<RpWidget> _widget;
	const std::unique_ptr<Lottie::SinglePlayer> _lottie;

	float64 _opacity = 1.;

};

} // namespace Ui
