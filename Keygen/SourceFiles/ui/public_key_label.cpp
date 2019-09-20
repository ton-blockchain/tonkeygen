// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "ui/public_key_label.h"

#include "ui/widgets/labels.h"
#include "styles/style_widgets.h"
#include "styles/style_keygen.h"
#include "styles/palette.h"

#include <QtGui/QPainter>

namespace Ui {
namespace {

style::font MonospaceFont(const style::font &parent) {
	const auto family = style::MonospaceFont()->family();
	return style::font(parent->size(), parent->flags(), family);
}

style::TextStyle ComputeKeyStyle(const style::TextStyle &parent) {
	auto result = parent;
	result.font = MonospaceFont(result.font);
	result.linkFont = MonospaceFont(result.linkFont);
	result.linkFontOver = MonospaceFont(result.linkFontOver);
	return result;
}

} // namespace

not_null<Ui::RpWidget*> CreatePublicKeyLabel(
		not_null<QWidget*> parent,
		const QString &text) {
	auto result = Ui::CreateChild<Ui::RpWidget>(parent.get());
	const auto st = result->lifetime().make_state<style::FlatLabel>(
		st::doneKeyLabel);
	st->style = ComputeKeyStyle(st->style);
	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		result,
		rpl::single(text),
		*st);
	label->setBreakEverywhere(true);
	const auto half = text.size() / 2;
	const auto first = text.mid(0, half);
	const auto second = text.mid(half);
	const auto width = std::max(
		st->style.font->width(first),
		st->style.font->width(second)
	) + st->style.font->spacew / 2;
	label->resizeToWidth(width);
	label->setSelectable(true);
	label->move(st::doneKeyMargins.left(), st::doneKeyMargins.top());
	const auto height = label->height();
	result->resize(
		st::doneKeyMargins.left() + width + st::doneKeyMargins.right(),
		st::doneKeyMargins.top() + height + st::doneKeyMargins.bottom());
	result->paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		auto p = QPainter(result);
		p.setPen(Qt::NoPen);
		p.setBrush(st::windowBgOver);
		p.drawRoundedRect(
			result->rect(),
			st::roundRadiusSmall,
			st::roundRadiusSmall);
	}, result->lifetime());
	return result;
}

} // namespace Ui
