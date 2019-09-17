// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#pragma once

#include "base/timer.h"
#include "core/ui_integration.h"

#include <QtWidgets/QApplication>
#include <QtCore/QAbstractNativeEventFilter>

namespace Ui {
namespace Animations {
class Manager;
} // namespace Animations
} // namespace Ui

namespace Core {

class Launcher;

class Sandbox final
	: public QApplication
	, private QAbstractNativeEventFilter {
	auto createEventNestingLevel() {
		incrementEventNestingLevel();
		return gsl::finally([=] { decrementEventNestingLevel(); });
	}

public:
	Sandbox(not_null<Launcher*> launcher, int &argc, char **argv);
	Sandbox(const Sandbox &other) = delete;
	Sandbox &operator=(const Sandbox &other) = delete;
	~Sandbox();

	not_null<Launcher*> launcher() const {
		return _launcher;
	}

	void postponeCall(FnMut<void()> &&callable);
	bool notify(QObject *receiver, QEvent *e) override;

	template <typename Callable>
	auto customEnterFromEventLoop(Callable &&callable) {
		registerEnterFromEventLoop();
		const auto wrap = createEventNestingLevel();
		return callable();
	}

	rpl::producer<> widgetUpdateRequests() const;

	static Sandbox &Instance() {
		Expects(QCoreApplication::instance() != nullptr);

		return *static_cast<Sandbox*>(QCoreApplication::instance());
	}

	void run();

	Ui::Animations::Manager &animationManager() const {
		return *_animationsManager;
	}

	void registerLeaveSubscription(not_null<QWidget*> widget);
	void unregisterLeaveSubscription(not_null<QWidget*> widget);

	void handleAppActivated();
	void handleAppDeactivated();

protected:
	bool event(QEvent *e) override;

private:
	static constexpr auto kDefaultSaveDelay = crl::time(1000);

	struct PostponedCall {
		int loopNestingLevel = 0;
		FnMut<void()> callable;
	};

	bool notifyOrInvoke(QObject *receiver, QEvent *e);
	void registerEnterFromEventLoop();
	void incrementEventNestingLevel();
	void decrementEventNestingLevel();
	bool nativeEventFilter(
		const QByteArray &eventType,
		void *message,
		long *result) override;
	void processPostponedCalls(int level);
	void launchApplication();
	void setupScreenScale();
	void checkLocalTime();

	void setScale(int scale);
	void stateChanged(Qt::ApplicationState state);

	const not_null<Launcher*> _launcher;
	UiIntegration uiIntegration;

	const Qt::HANDLE _mainThreadId = nullptr;
	int _eventNestingLevel = 0;
	int _loopNestingLevel = 0;
	std::vector<int> _previousLoopNestingLevels;
	std::vector<PostponedCall> _postponedCalls;

	rpl::event_stream<> _widgetUpdateRequests;

	const std::unique_ptr<Ui::Animations::Manager> _animationsManager;
	int _scale = 0;

	struct LeaveSubscription {
		LeaveSubscription(
			QPointer<QWidget> pointer,
			rpl::lifetime &&subscription)
		: pointer(pointer), subscription(std::move(subscription)) {
		}

		QPointer<QWidget> pointer;
		rpl::lifetime subscription;
	};
	std::vector<LeaveSubscription> _leaveSubscriptions;

	rpl::lifetime _lifetime;

};

} // namespace Core
