// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"
#include "base/object_ptr.h"

namespace Ui {
class FlatLabel;
} // namespace Ui

namespace Keygen::Steps {

class RandomSeed final : public Step {
public:
	RandomSeed();

	void showLimit(int limit);

	[[nodiscard]] rpl::producer<int> length() const;
	[[nodiscard]] QString accumulated() const;

private:
	void initControls();
	void append(const QString &text);

	QString _accumulated;
	rpl::variable<int> _limit = 0;
	rpl::variable<int> _length = 0;

};

} // namespace Keygen::Steps
