// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "base/object_ptr.h"

struct TextWithEntities;

namespace Ui {
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

	[[nodiscard]] virtual int desiredHeight();
	[[nodiscard]] not_null<Ui::RpWidget*> widget() const;
	[[nodiscard]] rpl::producer<NextButtonState> nextButtonState() const;
	[[nodiscard]] rpl::producer<> nextClicks() const;

	virtual void setFocus();

protected:
	[[nodiscard]] not_null<Ui::RpWidget*> inner() const;

	void setTitle(rpl::producer<TextWithEntities> text);
	void setDescription(rpl::producer<TextWithEntities> text);
	void requestNextButton(NextButtonState state);

private:
	const Type _type = Type();
	const std::unique_ptr<Ui::RpWidget> _widget;
	Ui::ScrollArea * const _scroll = nullptr;
	const not_null<Ui::RpWidget*> _inner;

	object_ptr<Ui::FlatLabel> _title = { nullptr };
	object_ptr<Ui::FlatLabel> _description = { nullptr };

	const std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>> _nextButton;
	NextButtonState _lastNextState;
	rpl::variable<NextButtonState> _nextButtonState;

};

} // namespace Keygen::Steps
