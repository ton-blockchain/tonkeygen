// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "ui/message_box.h"

#include "styles/style_layers.h"
#include "styles/style_keygen.h"

namespace Ui {

void InitMessageBox(
		not_null<GenericBox*> box,
		rpl::producer<QString> title,
		rpl::producer<TextWithEntities> text) {
	box->setWidth(st::messageBoxWidth);
	box->setTitle(std::move(title));
	box->addRow(object_ptr<Ui::FlatLabel>(
		box.get(),
		std::move(text),
		st::boxLabel));
}

} // namespace Ui
