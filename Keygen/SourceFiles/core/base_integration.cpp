// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "core/base_integration.h"

#include "core/launcher.h"
#include "core/sandbox.h"

namespace Core {

BaseIntegration::BaseIntegration(
	int argc,
	char *argv[])
: Integration(argc, argv) {
}

void BaseIntegration::enterFromEventLoop(FnMut<void()> &&method) {
	Core::Sandbox::Instance().customEnterFromEventLoop(
		std::move(method));
}

void BaseIntegration::logMessage(const QString &message) {
}

} // namespace Core
