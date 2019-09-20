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

	[[nodiscard]] rpl::producer<> copyKeyRequests() const;
	[[nodiscard]] rpl::producer<> saveKeyRequests() const;

private:
	void initControls(const QString &publicKey);

	rpl::event_stream<> _copyKeyRequests;
	rpl::event_stream<> _saveKeyRequests;

};

} // namespace Keygen::Steps
