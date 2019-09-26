// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"

namespace Keygen::Steps {

class Created final : public Step {
public:
	Created();

private:
	void initControls();
	void showFinishedHook() override;

};

} // namespace Keygen::Steps
