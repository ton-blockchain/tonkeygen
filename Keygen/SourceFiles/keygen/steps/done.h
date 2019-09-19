// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"

namespace Keygen::Steps {

class Done final : public Step {
public:
	explicit Done(const QString &publicKey);

private:
	void initControls(const QString &publicKey);

};

} // namespace Keygen::Steps
