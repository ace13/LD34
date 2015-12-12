#include "StateMachine.hpp"
#include "States/IState.hpp"

#include <algorithm>
#include <typeinfo>

StateMachine::StateMachine(Engine& eng) :
	mEngine(eng), mCurState(nullptr), mLastSeenRT(nullptr)
{

}
StateMachine::~StateMachine()
{
	for (auto& state : mOldStates)
	{
		if (state == mCurState)
			continue;

		delete state;
	}

	if (mCurState)
		delete mCurState;
}

void StateMachine::event(const sf::Event& ev)
{
	if (mCurState)
		mCurState->event(ev);
}

void StateMachine::tick(const Timespan& dt)
{
	if (mCurState)
		mCurState->tick(dt);
}
void StateMachine::update(const Timespan& dt)
{
	if (mCurState)
		mCurState->update(dt);
}
void StateMachine::draw(sf::RenderTarget& target)
{
	if (mCurState)
		mCurState->draw(target);

	mLastSeenRT = &target;
}
void StateMachine::drawUI(sf::RenderTarget& target)
{
	if (mCurState)
		mCurState->drawUI(target);

	mLastSeenRT = &target;
}

void StateMachine::changeState(IState* to, bool remove)
{
	if (mCurState == to)
		return;

	auto it = std::find_if(mOldStates.begin(), mOldStates.end(), [&to](IState* st) {
		return st == to;
	});

	if (mCurState)
	{
		mCurState->exit(*mLastSeenRT);

		if (remove)
			delete mCurState;
		else
		{
			auto it2 = std::find_if(mOldStates.begin(), mOldStates.end(), [this](IState* st) {
				return st == mCurState;
			});

			if (it2 == mOldStates.end())
				mOldStates.push_front(mCurState);
		}
	}

	if (mLastSeenRT && to)
		to->enter(*mLastSeenRT);
	if (it == mOldStates.end())
		mOldStates.push_back(to);

	mCurState = to;
}