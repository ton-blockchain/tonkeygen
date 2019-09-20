// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

class QWidget;
class QString;

namespace Ui {

class RpWidget;

[[nodiscard]] not_null<Ui::RpWidget*> CreatePublicKeyLabel(
	not_null<QWidget*> parent,
	const QString &text);

} // namespace Ui
