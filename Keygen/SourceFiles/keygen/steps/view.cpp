// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/view.h"

#include "keygen/phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {
namespace {

class Word final {
public:
	Word(not_null<QWidget*> parent, int index, const QString &word);

	void move(int left, int top) const;
	[[nodiscard]] int top() const;

private:
	object_ptr<Ui::FlatLabel> _index;
	object_ptr<Ui::FlatLabel> _word;

};

Word::Word(not_null<QWidget*> parent, int index, const QString &word)
: _index(parent, QString::number(index + 1) + '.', st::wordIndexLabel)
, _word(parent, word, st::wordLabel) {
}

void Word::move(int left, int top) const {
	_index->move(left - _index->width() - st::wordIndexSkip, top);
	_word->move(left, top);
}

int Word::top() const {
	return _index->y();
}

} // namespace

View::View(std::vector<QString> &&words) : Step(Type::Scroll) {
	setTitle(tr::lng_view_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_view_description(Ui::Text::RichLangValue));
	initControls(std::move(words));
}

int View::desiredHeight() const {
	return _desiredHeight;
}

void View::initControls(std::vector<QString> &&words) {
	Expects(words.size() % 2 == 0);

	auto labels = std::make_shared<std::vector<std::pair<Word, Word>>>();
	const auto rows = words.size() / 2;
	for (auto i = 0; i != rows; ++i) {
		labels->emplace_back(
			Word(inner(), i, words[i]),
			Word(inner(), i + rows, words[i + rows]));
	}
	const auto rowsBottom = st::wordsTop + rows * st::wordHeight;

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto half = size.width() / 2;
		const auto left = half - st::wordSkipLeft;
		const auto right = half + st::wordSkipRight;
		auto top = contentTop() + st::wordsTop;
		for (const auto &pair : *labels) {
			pair.first.move(left, top);
			pair.second.move(right, top);
			top += st::wordHeight;
		}

		auto state = NextButtonState();
		state.text = tr::lng_view_next(tr::now);
		state.top = rowsBottom + st::wordsNextSkip;
		requestNextButton(state);
	}, inner()->lifetime());

	_desiredHeight = rowsBottom
		+ st::wordsNextSkip
		+ st::wordsNextBottomSkip;
}

} // namespace Keygen::Steps
