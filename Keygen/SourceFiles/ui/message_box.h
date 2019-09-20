// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "ui/layers/generic_box.h"

namespace Ui {

void InitMessageBox(
	not_null<GenericBox*> box,
	rpl::producer<QString> title,
	rpl::producer<TextWithEntities> text);

} // namespace Ui
