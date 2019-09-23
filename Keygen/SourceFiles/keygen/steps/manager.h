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
class IconButton;
template <typename Widget>
class FadeWrap;
} // namespace Ui

namespace Keygen::Steps {

class Step;

class Manager final {
public:
	explicit Manager(Fn<bool(QString)> isGoodWord);
	Manager(const Manager &other) = delete;
	Manager &operator=(const Manager &other) = delete;
	~Manager();

	[[nodiscard]] not_null<Ui::RpWidget*> content() const;

	[[nodiscard]] rpl::producer<QByteArray> generateRequests() const;
	[[nodiscard]] rpl::producer<std::vector<QString>> checkRequests() const;

	enum class Action {
		ShowWords,
		CopyKey,
		SaveKey,
		NewKey,
	};

	[[nodiscard]] rpl::producer<Action> actionRequests() const;

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
	void showCopyKeyDone();
	void showSaveKeyDone(const QString &path);
	void showSaveKeyFail();
	void showError(const QString &text);

private:
	void showStep(
		std::unique_ptr<Step> step,
		FnMut<void()> next = nullptr,
		FnMut<void()> back = nullptr);
	void confirmNewKey();
	void initButtons();
	void moveNextButton();

	const std::unique_ptr<Ui::RpWidget> _content;
	const base::unique_qptr<Ui::FadeWrap<Ui::RoundButton>> _nextButton;
	const base::unique_qptr<Ui::FadeWrap<Ui::IconButton>> _backButton;
	Ui::Animations::Simple _nextButtonShown;
	NextButtonState _lastNextState;
	Ui::LayerManager _layerManager;

	const Fn<bool(QString)> _isGoodWord;
	std::vector<QString> _tmpwords;

	std::unique_ptr<Step> _step;

	FnMut<void()> _next;
	FnMut<void()> _back;

	rpl::event_stream<QByteArray> _generateRequests;
	rpl::event_stream<std::vector<QString>> _checkRequests;
	rpl::event_stream<Action> _actionRequests;

};

} // namespace Keygen::Steps
