// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "base/object_ptr.h"
#include "keygen/steps/step.h"
#include "ui/effects/animations.h"

namespace Ui {
class RpWidget;
class RoundButton;
template <typename Widget>
class FadeWrap;
} // namespace Ui

namespace Keygen::Steps {

class Step;

class Manager final {
public:
	Manager();
	Manager(const Manager &other) = delete;
	Manager &operator=(const Manager &other) = delete;
	~Manager();

	[[nodiscard]] not_null<Ui::RpWidget*> content() const;

	[[nodiscard]] rpl::producer<QByteArray> generateRequests() const;

	void next();
	void back();

	void showIntro();
	void showRandomSeed();
	void showCreated(std::vector<QString> &&words);
	void showWords(std::vector<QString> &&words);
	void showCheck();
	void showDone(const QString &publicKey);

private:
	void showStep(
		std::unique_ptr<Step> step,
		FnMut<void()> next = nullptr,
		FnMut<void()> back = nullptr);

	const std::unique_ptr<Ui::RpWidget> _content;
	const object_ptr<Ui::FadeWrap<Ui::RoundButton>> _nextButton;

	std::unique_ptr<Step> _step;
	NextButtonState _nextState;

	FnMut<void()> _next;
	FnMut<void()> _back;

	rpl::event_stream<QByteArray> _generateRequests;

};

} // namespace Keygen::Steps
