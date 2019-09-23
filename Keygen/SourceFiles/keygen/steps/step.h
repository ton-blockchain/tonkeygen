// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "base/unique_qptr.h"
#include "ui/effects/animations.h"
#include "ui/widgets/labels.h"

struct TextWithEntities;

namespace Ui {
class LottieAnimation;
class CrossFadeAnimation;
class RpWidget;
class FlatLabel;
class ScrollArea;
class RoundButton;
template <typename Widget>
class FadeWrap;
} // namespace Ui

namespace Keygen::Steps {

struct NextButtonState {
	QString text;
	int top = 0;
	int width = 0;
};

class Step {
public:
	enum class Type {
		Intro,
		Scroll,
		Default
	};

	Step(Type intro);
	Step(const Step &other) = delete;
	Step &operator=(const Step &other) = delete;
	virtual ~Step() = 0;

	[[nodiscard]] virtual int desiredHeight() const;
	[[nodiscard]] not_null<Ui::RpWidget*> widget() const;
	[[nodiscard]] rpl::producer<NextButtonState> nextButtonState() const;
	[[nodiscard]] rpl::producer<> nextClicks() const;
	[[nodiscard]] rpl::producer<float64> coverShown() const;

	void showAnimated(not_null<Step*> previous);
	virtual void setFocus();

	[[nodiscard]] rpl::lifetime &lifetime();

protected:
	[[nodiscard]] not_null<Ui::RpWidget*> inner() const;
	[[nodiscard]] int contentTop() const;
	[[nodiscard]] int contentBottom() const;

	void setTitle(rpl::producer<TextWithEntities> text, int top = 0);
	void setDescription(rpl::producer<TextWithEntities> text, int top = 0);
	void requestNextButton(NextButtonState state);
	void ensureVisible(int top, int height);

	void showLottie(const QString &path, int top, int height);

private:
	struct AnimationData;
	struct CoverAnimation {
		CoverAnimation() = default;
		CoverAnimation(CoverAnimation &&other) = default;
		CoverAnimation &operator=(CoverAnimation &&other) = default;
		~CoverAnimation();

		std::unique_ptr<Ui::LottieAnimation> lottie;
		std::unique_ptr<Ui::CrossFadeAnimation> title;
		std::unique_ptr<Ui::CrossFadeAnimation> description;

		// From description bottom till the next button top.
		QPixmap contentSnapshotWas;
		QPixmap contentSnapshotNow;
		int contentWasBottom = 0;
		int contentNowBottom = 0;
		int lottieTop = 0;
		int lottieHeight = 0;
	};

	[[nodiscard]] Ui::ScrollArea *resolveScrollArea();
	[[nodiscard]] not_null<Ui::RpWidget*> resolveInner();
	[[nodiscard]] auto resolveNextButton()
		->std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>>;
	void initGeometry();
	void initNextButton();
	void initCover();

	void showAnimatedCover(not_null<Step*> previous);
	void prepareCoverMask();
	void paintContent(QRect clip);
	void paintCover(QPainter &p, int top, QRect clip);
	void showFinished();

	[[nodiscard]] AnimationData prepareAnimationData();
	[[nodiscard]] QPixmap prepareContentSnapshot() const;
	void showAnimationCallback();
	void paintContentSnapshot(
		QPainter &p,
		const QPixmap &snapshot,
		int snapshotBottom,
		float64 alpha,
		float64 howMuchHidden);

	[[nodiscard]] QRect lottieGeometry(int top, int height) const;

	const Type _type = Type();
	const std::unique_ptr<Ui::RpWidget> _widget;
	Ui::ScrollArea * const _scroll = nullptr;
	const not_null<Ui::RpWidget*> _inner;

	std::unique_ptr<Ui::LottieAnimation> _lottie;
	int _lottieTop = 0;
	int _lottieHeight = 0;
	base::unique_qptr<Ui::FlatLabel> _title;
	base::unique_qptr<Ui::FlatLabel> _description;

	Ui::Animations::Simple _showAnimation;
	CoverAnimation _coverAnimation;
	//std::unique_ptr<Ui::SlideAnimation> _slideAnimation;
	QPixmap _coverMask;
	rpl::variable<float64> _coverShown = 0.;

	const std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>> _nextButton;
	NextButtonState _lastNextState;
	rpl::variable<NextButtonState> _nextButtonState;

};

} // namespace Keygen::Steps
