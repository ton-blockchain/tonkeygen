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
	explicit Check(const std::vector<QString> &values);

	int desiredHeight() const override;

	[[nodiscard]] std::vector<QString> words() const;

private:
	void initControls(const std::vector<QString> &values);

	int _desiredHeight = 0;
	Fn<std::vector<QString>()> _words;

};

} // namespace Keygen::Steps
