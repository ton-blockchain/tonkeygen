// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/check.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/input_fields.h"
#include "ui/text/text_utilities.h"
#include "base/qt_signal_producer.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {
namespace {

class Word final {
public:
	Word(not_null<QWidget*> parent, int index, const QString &value);

	void move(int left, int top) const;
	int top() const;
	QString word() const;
	void setFocus() const;
	void showError() const;

	[[nodiscard]] rpl::producer<> focused() const;
	[[nodiscard]] rpl::producer<> blurred() const;
	[[nodiscard]] rpl::producer<> tabbed() const;
	[[nodiscard]] rpl::producer<> submitted() const;

private:
	object_ptr<Ui::FlatLabel> _index;
	object_ptr<Ui::InputField> _word;

};

Word::Word(not_null<QWidget*> parent, int index, const QString &value)
: _index(parent, QString::number(index + 1) + '.', st::wordIndexLabel)
, _word(parent, st::checkInputField, rpl::single(QString()), value) {
	_word->customTab(true);
}

void Word::move(int left, int top) const {
	_index->move(left - _index->width() - st::wordIndexSkip, top);
	_word->move(left, top - st::checkInputSkip);
}

void Word::setFocus() const {
	_word->setFocus();
}

void Word::showError() const {
	_word->showError();
}

rpl::producer<> Word::focused() const {
	return base::qt_signal_producer(_word.data(), &Ui::InputField::focused);
}

rpl::producer<> Word::blurred() const {
	return base::qt_signal_producer(_word.data(), &Ui::InputField::blurred);
}

rpl::producer<> Word::tabbed() const {
	return base::qt_signal_producer(_word.data(), &Ui::InputField::tabbed);
}

rpl::producer<> Word::submitted() const {
	return base::qt_signal_producer(
		_word.data(),
		&Ui::InputField::submitted
	) | rpl::map([] { return rpl::empty_value(); });
}

int Word::top() const {
	return _index->y();
}

QString Word::word() const {
	return _word->getLastText();
}

} // namespace

Check::Check(
	Fn<bool(QString)> isGoodWord,
	const std::vector<QString> &values)
: Step(Type::Scroll) {
	setTitle(tr::lng_check_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_check_description(Ui::Text::RichLangValue));
	initControls(isGoodWord, values);
}

std::vector<QString> Check::words() const {
	return _words();
}

rpl::producer<> Check::submitRequests() const {
	return _submitRequests.events();
}

void Check::setFocus() {
	_setFocus();
}

bool Check::checkAll() {
	return _checkAll();
}

int Check::desiredHeight() const {
	return _desiredHeight;
}

void Check::initControls(
		Fn<bool(QString)> isGoodWord,
		const std::vector<QString> &values) {
	auto labels = std::make_shared<std::vector<Word>>();
	const auto wordsTop = st::wordsTop;
	const auto rows = 12;
	const auto count = rows * 2;
	const auto rowsBottom = wordsTop + rows * st::wordHeight;
	const auto value = [&](int index) {
		return (index < values.size()) ? values[index] : QString();
	};
	const auto isValid = [=](int index) {
		Expects(index < count);

		return isGoodWord((*labels)[index].word());
	};
	const auto showError = [=](int index) {
		Expects(index < count);

		if (isValid(index)) {
			return false;
		}
		(*labels)[index].showError();
		return true;
	};
	const auto init = [&](const Word &word, int index) {
		const auto next = [=] {
			return (index + 1 < count) ? &(*labels)[index + 1] : nullptr;
		};

		word.focused(
		) | rpl::start_with_next([=] {
			const auto row = index % rows;
			ensureVisible(wordsTop + row * st::wordHeight, st::wordHeight);
		}, lifetime());

		word.tabbed(
		) | rpl::start_with_next([=] {
			if (const auto word = next()) {
				word->setFocus();
			}
		}, lifetime());

		word.submitted(
		) | rpl::start_with_next([=] {
			if (!showError(index)) {
				if (const auto word = next()) {
					word->setFocus();
				} else {
					_submitRequests.fire({});
				}
			}
		}, lifetime());
	};
	for (auto i = 0; i != count; ++i) {
		labels->emplace_back(inner(), i, value(i));
		init(labels->back(), i);
	}

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto half = size.width() / 2;
		const auto left = half - st::wordSkipLeft;
		const auto right = half + st::wordSkipRight;
		auto x = left;
		auto y = contentTop() + wordsTop;
		for (const auto &word : *labels) {
			word.move(x, y);
			y += st::wordHeight;
			if (y == rowsBottom) {
				x = right;
				y = wordsTop;
			}
		}

		auto state = NextButtonState();
		state.text = tr::lng_view_next(tr::now);
		state.top = rowsBottom + st::wordsNextSkip;
		requestNextButton(state);
	}, inner()->lifetime());

	_words = [=] {
		return (*labels) | ranges::view::transform(
			&Word::word
		) | ranges::to_vector;
	};
	_setFocus = [=] {
		labels->front().setFocus();
	};
	_checkAll = [=] {
		auto result = true;
		for (auto i = count; i != 0;) {
			result = !showError(--i) && result;
		}
		return result;
	};

	_desiredHeight = rowsBottom
		+ st::wordsNextSkip
		+ st::wordsNextBottomSkip;
}

} // namespace Keygen::Steps
