// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"
#include "ui/effects/animations.h"
#include "ui/layers/layer_manager.h"
#include "base/unique_qptr.h"

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
	[[nodiscard]] rpl::producer<std::vector<QString>> checkRequests() const;

	void next();
	void back();

	void showIntro();
	void showRandomSeed();
	void showCreated(std::vector<QString> &&words);
	void showWords(std::vector<QString> &&words);
	void showCheck();
	void showCheckDone(const QString &publicKey);
	void showCheckFail();
	void showDone(const QString &publicKey);

	void showBox(
		object_ptr<Ui::BoxContent> box,
		Ui::LayerOptions options = Ui::LayerOption::KeepOther,
		anim::type animated = anim::type::normal);

private:
	void showStep(
		std::unique_ptr<Step> step,
		FnMut<void()> next = nullptr,
		FnMut<void()> back = nullptr);

	const std::unique_ptr<Ui::RpWidget> _content;
	const base::unique_qptr<Ui::FadeWrap<Ui::RoundButton>> _nextButton;
	NextButtonState _lastNextState;
	Ui::LayerManager _layerManager;

	std::vector<QString> _words;

	std::unique_ptr<Step> _step;

	FnMut<void()> _next;
	FnMut<void()> _back;

	rpl::event_stream<QByteArray> _generateRequests;
	rpl::event_stream<std::vector<QString>> _checkRequests;

};

} // namespace Keygen::Steps
