// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "ui/word_suggestions.h"

#include "base/object_ptr.h"
#include "ui/widgets/scroll_area.h"
#include "ui/rp_widget.h"
#include "ui/painter.h"
#include "styles/style_keygen.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include <QtGui/QPainter>

namespace Ui {

WordSuggestions::WordSuggestions(not_null<QWidget*> parent)
: _widget(std::make_unique<RpWidget>(parent))
, _scroll(Ui::CreateChild<ScrollArea>(_widget.get(), st::suggestionsScroll))
, _inner(_scroll->setOwnedWidget(object_ptr<RpWidget>(_widget.get()))) {
	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_scroll->setGeometry({ QPoint(), size });
	}, _widget->lifetime());

	_widget->paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		paintRows();
	}, _widget->lifetime());

	_inner->setMouseTracking(true);
	_inner->events(
	) | rpl::start_with_next([=](not_null<QEvent*> e) {
		if (e->type() == QEvent::MouseMove) {
			selectByMouse(static_cast<QMouseEvent*>(e.get())->pos());
		} else if (e->type() == QEvent::MouseButtonPress) {
			_pressed = _selected;
		} else if (e->type() == QEvent::MouseButtonRelease) {
			_widget->update();
			if (std::exchange(_pressed, -1) == _selected) {
				choose();
			}
		}
	}, _inner->lifetime());
}

void WordSuggestions::show(std::vector<QString> &&words) {
	if (_words == words) {
		return;
	}
	_words = std::move(words);
	select(0);
	const auto height = st::suggestionsSkip * 2
		+ int(_words.size()) * st::suggestionHeight
		+ st::suggestionShadowWidth;
	const auto outerHeight = std::min(height, st::suggestionsHeightMax);
	_inner->resize(_widget->width(), height);
	_widget->resize(_widget->width(), outerHeight);
	_widget->update();
	_widget->show();
}

void WordSuggestions::hide() {
	_hidden.fire({});
}

void WordSuggestions::selectDown() {
	Expects(!_words.empty());

	select(_selected + 1);
	ensureSelectedVisible();
}

void WordSuggestions::selectUp() {
	select(_selected - 1);
	ensureSelectedVisible();
}

void WordSuggestions::select(int index) {
	Expects(!_words.empty());

	index = std::clamp(index, 0, int(_words.size()) - 1);
	if (_selected == index) {
		return;
	}
	_selected = index;
	_inner->update();
}

void WordSuggestions::selectByMouse(QPoint position) {
	select((position.y() - st::suggestionsSkip) / st::suggestionHeight);
}

void WordSuggestions::ensureSelectedVisible() {
	const auto skip = st::suggestionsSkip;
	const auto top = skip + _selected * st::suggestionHeight;
	_scroll->scrollToY(top - skip, top + st::suggestionHeight + skip);
}

void WordSuggestions::choose() {
	Expects(!_words.empty());
	Expects(_selected >= 0 && _selected < _words.size());

	_chosen.fire_copy(_words[_selected]);
}

void WordSuggestions::setGeometry(QPoint position, int width) {
	_widget->setGeometry({ position, QSize{ width, _widget->height() } });
	_inner->resize(width, _inner->height());
}

void WordSuggestions::paintRows() {
	QPainter(_widget.get()).drawImage(0, 0, prepareFrame());
}

QImage WordSuggestions::prepareFrame() const {
	const auto pixelRatio = style::DevicePixelRatio();
	if (_frame.size() != _widget->size() * pixelRatio) {
		_frame = QImage(
			_widget->size() * pixelRatio,
			QImage::Format_ARGB32_Premultiplied);
	}
	_frame.fill(st::windowBg->c);
	_frame.setDevicePixelRatio(pixelRatio);
	{
		auto p = QPainter(&_frame);

		const auto thickness = st::suggestionShadowWidth;

		p.setPen(st::windowFg);
		p.setFont(st::normalFont);
		auto index = 0;
		const auto wordLeft = thickness;
		auto wordTop = st::suggestionsSkip - _scroll->scrollTop();
		const auto wordWidth = _widget->width() - 2 * thickness;
		const auto wordHeight = st::suggestionHeight;
		for (const auto &word : _words) {
			if (index == (_pressed >= 0 ? _pressed : _selected)) {
				p.fillRect(
					wordLeft,
					wordTop,
					wordWidth,
					wordHeight,
					st::windowBgOver);
			}
			p.drawText(
				wordLeft + st::suggestionLeft,
				wordTop + st::suggestionTop + st::normalFont->ascent,
				word);
			wordTop += wordHeight;
			++index;
		}

		const auto radius = st::suggestionsRadius;
		const auto left = float64(thickness) / 2;
		const auto top = -2. * radius;
		const auto width = float64(_widget->width()) - thickness;
		const auto height = float64(_widget->height())
			- top
			+ ((thickness / 2.) - thickness);

		PainterHighQualityEnabler hq(p);
		p.setBrush(Qt::NoBrush);
		auto pen = st::defaultInputField.borderFg->p;
		pen.setWidth(thickness);
		p.setPen(pen);
		p.drawRoundedRect(QRectF{ left, top, width, height }, radius, radius);
	}
	return _frame;
}

rpl::producer<QString> WordSuggestions::chosen() const {
	return _chosen.events();
}

rpl::producer<> WordSuggestions::hidden() const {
	return _hidden.events();
}

rpl::lifetime &WordSuggestions::lifetime() {
	return _widget->lifetime();
}

} // namespace Ui
