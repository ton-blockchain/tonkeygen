// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "core/base_integration.h"

namespace Core {

class Launcher {
public:
	Launcher(int argc, char *argv[]);

	static std::unique_ptr<Launcher> Create(int argc, char *argv[]);

	int exec();

	QString argumentsString() const;

	virtual ~Launcher() = default;

private:
	void prepareSettings();
	void processArguments();

	QStringList readArguments(int argc, char *argv[]) const;

	void init();
	int executeApplication();

	int _argc;
	char **_argv;
	QStringList _arguments;
	BaseIntegration _baseIntegration;

};

} // namespace Core
