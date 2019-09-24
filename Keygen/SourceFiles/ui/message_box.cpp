// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "ui/message_box.h"

#include "styles/style_layers.h"
#include "styles/style_keygen.h"

#include <QtGui/QtEvents>

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
	box->events(
	) | rpl::filter([](not_null<QEvent*> e) {
		return (e->type() == QEvent::KeyPress);
	}) | rpl::map([](not_null<QEvent*> e) {
		return static_cast<QKeyEvent*>(e.get());
	}) | rpl::start_with_next([=](not_null<QKeyEvent*> e) {
		if (e->key() == Qt::Key_Escape) {
			e->accept();
			box->closeBox();
		} else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
			e->accept();
			box->triggerButton(0);
		}
	}, box->lifetime());
	box->setInnerFocus();
}

} // namespace Ui
