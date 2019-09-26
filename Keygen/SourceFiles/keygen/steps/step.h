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
class SlideAnimation;
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

enum class Direction {
	Forward,
	Backward,
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

	void showAnimated(not_null<Step*> previous, Direction direction);
	virtual void setFocus();

	[[nodiscard]] rpl::lifetime &lifetime();

protected:
	[[nodiscard]] not_null<Ui::RpWidget*> inner() const;
	[[nodiscard]] int contentTop() const;

	void setTitle(rpl::producer<TextWithEntities> text, int top = 0);
	void setDescription(rpl::producer<TextWithEntities> text, int top = 0);
	void requestNextButton(NextButtonState state);
	void ensureVisible(int top, int height);

	void showLottie(const QString &path, int top, int height);
	void startLottie();
	void stopLottieOnFrame(int frame);

	[[nodiscard]] virtual QImage grabForAnimation(QRect rect) const;
	virtual void showFinishedHook();

private:
	struct CoverAnimationData;
	struct CoverAnimation {
		CoverAnimation() = default;
		CoverAnimation(CoverAnimation&&) = default;
		CoverAnimation &operator=(CoverAnimation&&) = default;
		~CoverAnimation();

		std::unique_ptr<Ui::LottieAnimation> lottie;
		std::unique_ptr<Ui::CrossFadeAnimation> title;
		std::unique_ptr<Ui::CrossFadeAnimation> description;

		// From description bottom till the bottom.
		QPixmap contentWas;
		QPixmap contentNow;
		int contentWasBottom = 0;
		int contentNowBottom = 0;
		int lottieTop = 0;
		int lottieHeight = 0;
	};
	struct SlideAnimationData;
	struct SlideAnimation {
		SlideAnimation() = default;
		SlideAnimation(SlideAnimation&&) = default;
		SlideAnimation &operator=(SlideAnimation&&) = default;
		~SlideAnimation();

		std::unique_ptr<Ui::LottieAnimation> lottieWas;
		std::unique_ptr<Ui::LottieAnimation> lottieNow;

		std::unique_ptr<Ui::SlideAnimation> slide;
		int slideTop = 0;
		int slideWidth = 0;
		int lottieWasTop = 0;
		int lottieWasHeight = 0;
		int lottieNowTop = 0;
		int lottieNowHeight = 0;
	};

	[[nodiscard]] Ui::ScrollArea *resolveScrollArea();
	[[nodiscard]] not_null<Ui::RpWidget*> resolveInner();
	[[nodiscard]] auto resolveNextButton()
		->std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>>;
	void initGeometry();
	void initNextButton();
	void initCover();

	void showAnimatedCover(not_null<Step*> previous);
	void showAnimatedSlide(not_null<Step*> previous, Direction direction);
	void prepareCoverMask();
	void paintContent(QRect clip);
	void paintCover(QPainter &p, int top, QRect clip);
	void showFinished();

	[[nodiscard]] int coverAnimationContentTop() const;
	[[nodiscard]] int slideAnimationContentTop() const;
	[[nodiscard]] int animationContentBottom() const;

	[[nodiscard]] CoverAnimationData prepareCoverAnimationData();
	[[nodiscard]] QPixmap prepareCoverAnimationContent() const;
	void coverAnimationCallback();
	void paintCoverAnimation(QPainter &p, QRect clip);
	void paintCoverAnimationContent(
		QPainter &p,
		const QPixmap &snapshot,
		int snapshotBottom,
		float64 alpha,
		float64 howMuchHidden);
	[[nodiscard]] SlideAnimationData prepareSlideAnimationData();
	[[nodiscard]] QImage prepareSlideAnimationContent() const;
	void adjustSlideSnapshots(
		SlideAnimationData &was,
		SlideAnimationData &now);
	void slideAnimationCallback();
	void paintSlideAnimation(QPainter &p, QRect clip);

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

	Ui::Animations::Simple _coverAnimationValue;
	CoverAnimation _coverAnimation;
	SlideAnimation _slideAnimation;
	QPixmap _coverMask;
	rpl::variable<float64> _coverShown = 0.;

	const std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>> _nextButton;
	NextButtonState _lastNextState;
	rpl::variable<NextButtonState> _nextButtonState;

};

} // namespace Keygen::Steps
