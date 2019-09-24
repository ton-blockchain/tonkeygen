// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "keygen/steps/step.h"
#include "base/unique_qptr.h"

namespace Ui {
class IconButton;
class DropdownMenu;
} // namespace Ui

namespace Keygen::Steps {

class Done final : public Step {
public:
	explicit Done(const QString &publicKey);

	[[nodiscard]] rpl::producer<> copyKeyRequests() const;
	[[nodiscard]] rpl::producer<> saveKeyRequests() const;
	[[nodiscard]] rpl::producer<> newKeyRequests() const;
	[[nodiscard]] rpl::producer<> verifyKeyRequests() const;

private:
	void initControls(const QString &publicKey);
	void initShortcuts();
	void showMenu(not_null<Ui::IconButton*> toggle);

	QImage grabForAnimation(QRect rect) const override;

	rpl::event_stream<> _copyKeyRequests;
	rpl::event_stream<> _saveKeyRequests;
	rpl::event_stream<> _newKeyRequests;
	rpl::event_stream<> _verifyKeyRequests;

	base::unique_qptr<Ui::DropdownMenu> _menu;

};

} // namespace Keygen::Steps
