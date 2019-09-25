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
#include "ui/word_suggestions.h"
#include "base/event_filter.h"
#include "base/qt_signal_producer.h"
#include "styles/style_keygen.h"

#include <QtGui/QtEvents>

namespace Keygen::Steps {
namespace {

const auto kSkipPassword = QString("speakfriendandenter");

class Word final {
public:
	Word(
		not_null<QWidget*> parent,
		int index,
		Fn<std::vector<QString>(QString)> wordsByPrefix);
	Word(const Word &other) = delete;
	Word &operator=(const Word &other) = delete;

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
	void setupSuggestions();
	void createSuggestionsWidget();
	void showSuggestions(const QString &word);

	object_ptr<Ui::FlatLabel> _index;
	object_ptr<Ui::InputField> _word;
	const Fn<std::vector<QString>(QString)> _wordsByPrefix;
	std::unique_ptr<Ui::WordSuggestions> _suggestions;
	bool _chosen = false;

};

Word::Word(
	not_null<QWidget*> parent,
	int index,
	Fn<std::vector<QString>(QString)> wordsByPrefix)
: _index(parent, QString::number(index + 1) + '.', st::wordIndexLabel)
, _word(parent, st::checkInputField, rpl::single(QString()), QString())
, _wordsByPrefix(std::move(wordsByPrefix)) {
	_word->customTab(true);
	_word->customUpDown(true);
	setupSuggestions();
}

void Word::setupSuggestions() {
	base::qt_signal_producer(
		_word.data(),
		&Ui::InputField::changed
	) | rpl::start_with_next([=] {
		_chosen = false;
		showSuggestions(word());
	}, _word->lifetime());

	focused(
	) | rpl::filter([=] {
		return !_chosen;
	}) | rpl::start_with_next([=] {
		showSuggestions(word());
	}, _word->lifetime());

	base::install_event_filter(_word.data(), [=](not_null<QEvent*> e) {
		if (e->type() != QEvent::KeyPress || !_suggestions) {
			return base::EventFilterResult::Continue;
		}
		const auto key = static_cast<QKeyEvent*>(e.get())->key();
		if (key == Qt::Key_Up) {
			_suggestions->selectUp();
			return base::EventFilterResult::Cancel;
		} else if (key == Qt::Key_Down) {
			_suggestions->selectDown();
			return base::EventFilterResult::Cancel;
		}
		return base::EventFilterResult::Continue;
	});
}

void Word::showSuggestions(const QString &word) {
	auto list = _wordsByPrefix(word);
	if (list.empty() || (list.size() == 1 && list.front() == word) || word.size() < 3) {
		if (_suggestions) {
			_suggestions->hide();
		}
	} else {
		if (!_suggestions) {
			createSuggestionsWidget();
		}
		_suggestions->show(std::move(list));
	}
}

void Word::createSuggestionsWidget() {
	_suggestions = std::make_unique<Ui::WordSuggestions>(
		_word->parentWidget());

	_suggestions->chosen(
	) | rpl::start_with_next([=](QString word) {
		_chosen = true;
		_word->setText(word);
		_word->setFocus();
		_word->setCursorPosition(word.size());
		_suggestions = nullptr;
		emit _word->submitted(Qt::KeyboardModifiers());
	}, _suggestions->lifetime());

	_suggestions->hidden(
	) | rpl::start_with_next([=] {
		_suggestions = nullptr;
	}, _suggestions->lifetime());

	_word->geometryValue(
	) | rpl::start_with_next([=](QRect geometry) {
		_suggestions->setGeometry(
			geometry.topLeft() + QPoint(0, geometry.height()),
			geometry.width());
	}, _suggestions->lifetime());

	_word->events(
	) | rpl::filter([](not_null<QEvent*> e) {
		return (e->type() == QEvent::KeyPress);
	}) | rpl::map([=](not_null<QEvent*> e) {
		return static_cast<QKeyEvent*>(e.get())->key();
	}) | rpl::start_with_next([=](int key) {
		if (key == Qt::Key_Up) {
			_suggestions->selectUp();
		} else if (key == Qt::Key_Down) {
			_suggestions->selectDown();
		}
	}, _suggestions->lifetime());

	blurred(
	) | rpl::start_with_next([=] {
		_suggestions->hide();
	}, _suggestions->lifetime());
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
	) | rpl::filter([=] {
		if (_suggestions) {
			_suggestions->choose();
			return false;
		}
		return true;
	}) | rpl::map([] {
		return rpl::empty_value();
	});
}

int Word::top() const {
	return _index->y();
}

QString Word::word() const {
	return _word->getLastText();
}

} // namespace

Check::Check(Fn<std::vector<QString>(QString)> wordsByPrefix)
: Step(Type::Scroll) {
	setTitle(tr::lng_check_title(Ui::Text::RichLangValue));
	setDescription(tr::lng_check_description(Ui::Text::RichLangValue));
	initControls(std::move(wordsByPrefix));
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

void Check::initControls(Fn<std::vector<QString>(QString)> wordsByPrefix) {
	constexpr auto rows = 12;
	constexpr auto count = rows * 2;
	auto inputs = std::make_shared<std::vector<std::unique_ptr<Word>>>();
	const auto wordsTop = st::checksTop;
	const auto rowsBottom = wordsTop + rows * st::wordHeight;
	const auto isValid = [=](int index) {
		Expects(index < count);

		const auto word = (*inputs)[index]->word();
		const auto words = wordsByPrefix(word);
		return !words.empty() && (words.front() == word);
	};
	const auto showError = [=](int index) {
		Expects(index < count);

		if (isValid(index)) {
			return false;
		}
		(*inputs)[index]->showError();
		return true;
	};
	const auto init = [&](const Word &word, int index) {
		const auto next = [=] {
			return (index + 1 < count)
				? (*inputs)[index + 1].get()
				: nullptr;
		};

		word.focused(
		) | rpl::start_with_next([=] {
			const auto row = index % rows;
			ensureVisible(
				wordsTop + (row - 1) * st::wordHeight,
				2 * st::wordHeight + st::suggestionsHeightMax);
		}, lifetime());

		word.tabbed(
		) | rpl::start_with_next([=] {
			if (const auto word = next()) {
				word->setFocus();
			}
		}, lifetime());

		word.submitted(
		) | rpl::start_with_next([=] {
			if ((*inputs)[index]->word() == kSkipPassword) {
				_submitRequests.fire({});
			} else if (!showError(index)) {
				if (const auto word = next()) {
					word->setFocus();
				} else {
					_submitRequests.fire({});
				}
			}
		}, lifetime());
	};
	for (auto i = 0; i != count; ++i) {
		inputs->push_back(std::make_unique<Word>(inner(), i, wordsByPrefix));
		init(*inputs->back(), i);
	}

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto half = size.width() / 2;
		const auto left = half - st::wordSkipLeft;
		const auto right = half + st::wordSkipRight;
		auto x = left;
		auto y = contentTop() + wordsTop;
		auto index = 0;
		for (const auto &input : *inputs) {
			input->move(x, y);
			y += st::wordHeight;
			if (++index == rows) {
				x = right;
				y = contentTop() + wordsTop;
			}
		}

		auto state = NextButtonState();
		state.text = tr::lng_view_next(tr::now);
		state.top = rowsBottom + st::wordsNextSkip;
		requestNextButton(state);
	}, inner()->lifetime());

	_words = [=] {
		return (*inputs) | ranges::view::transform(
			[](const std::unique_ptr<Word> &p) { return p->word(); }
		) | ranges::to_vector;
	};
	_setFocus = [=] {
		inputs->front()->setFocus();
	};
	_checkAll = [=] {
		if ((*inputs)[0]->word() == kSkipPassword) {
			return true;
		}
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
