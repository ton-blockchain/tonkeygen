// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"

namespace Keygen::Steps {

class View final : public Step {
public:
	explicit View(std::vector<QString> &&words);

	int desiredHeight() const override;

private:
	void initControls(std::vector<QString> &&words);

	int _desiredHeight = 0;

};

} // namespace Keygen::Steps
