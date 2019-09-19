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
#include "styles/style_keygen.h"
#include "styles/style_widgets.h"

namespace Keygen::Steps {
namespace {

class Word final {
public:
	Word(not_null<QWidget*> parent, int index, const QString &value);

	void move(int left, int top) const;
	[[nodiscard]] int top() const;
	[[nodiscard]] QString word() const;
	void setFocus();

private:
	object_ptr<Ui::FlatLabel> _index;
	object_ptr<Ui::InputField> _word;

};

Word::Word(not_null<QWidget*> parent, int index, const QString &value)
: _index(parent, QString::number(index + 1))
, _word(parent, st::checkInputField, rpl::single(QString()), value) {
}

void Word::move(int left, int top) const {
	_index->move(left - _index->width() - 10, top);
	_word->move(left, top);
	_word->resize(style::ConvertScale(80), _word->height());
}

void Word::setFocus() {
	_word->setFocus();
}

int Word::top() const {
	return _index->y();
}

QString Word::word() const {
	return _word->getLastText();
}

} // namespace

Check::Check(const std::vector<QString> &values) : Step(Type::Scroll) {
	setTitle(tr::lng_check_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_check_description(Ui::Text::RichLangValue));
	initControls(values);
}

std::vector<QString> Check::words() const {
	return _words();
}

int Check::desiredHeight() {
	return _desiredHeight;
}

void Check::initControls(const std::vector<QString> &values) {
	auto labels = std::make_shared<std::vector<std::pair<Word, Word>>>();
	const auto rows = 12;
	const auto value = [&](int index) {
		return (index < values.size()) ? values[index] : QString();
	};
	for (auto i = 0; i != rows; ++i) {
		labels->emplace_back(
			Word(inner(), i, value(i)),
			Word(inner(), i + rows, value(i + rows)));
	}

	inner()->widthValue(
	) | rpl::start_with_next([=](int width) {
		const auto half = width / 2;
		const auto left = half - style::ConvertScale(100);
		const auto right = half + style::ConvertScale(20);
		auto top = style::ConvertScale(200);
		for (const auto &pair : *labels) {
			pair.first.move(left, top);
			pair.second.move(right, top);
			top += style::ConvertScale(34);
		}
	}, inner()->lifetime());

	_words = [=] {
		auto result = std::vector<QString>(rows * 2);
		auto i = 0;
		for (const auto &pair : *labels) {
			result[i] = pair.first.word();
			result[rows + i] = pair.second.word();
			++i;
		}
		return result;
	};

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
	}, inner()->lifetime());

	const auto bottom = labels->back().first.top();
	auto state = NextButtonState();
	state.text = tr::lng_view_next(tr::now);
	state.top = bottom + style::ConvertScale(34);
	requestNextButton(state);

	_desiredHeight = state.top + st::nextButton.height + style::ConvertScale(30);
}

} // namespace Keygen::Steps
