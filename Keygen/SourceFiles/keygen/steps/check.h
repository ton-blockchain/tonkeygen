// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"

namespace Keygen::Steps {

class Check final : public Step {
public:
	Check(Fn<bool(QString)> isGoodWord, const std::vector<QString> &values);

	int desiredHeight() const override;

	[[nodiscard]] std::vector<QString> words() const;
	[[nodiscard]] rpl::producer<> submitRequests() const;

	void setFocus() override;
	bool checkAll();

private:
	void initControls(
		Fn<bool(QString)> isGoodWord,
		const std::vector<QString> &values);

	int _desiredHeight = 0;
	Fn<std::vector<QString>()> _words;
	Fn<void()> _setFocus;
	Fn<bool()> _checkAll;

	rpl::event_stream<> _submitRequests;

};

} // namespace Keygen::Steps
