#include "StateMachine.hpp"
#include "States/IState.hpp"

#include <algorithm>
#include <typeinfo>

StateMachine::StateMachine(Engine& eng) :
	mEngine(eng), mCurState(nullptr)
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
	auto it = std::find_if(mOldStates.begin(), mOldStates.end(), [&to](IState* st) {
		return st == to;
	});

	if (mCurState && remove)
	{
		mCurState->exit(*mLastSeenRT);
		delete mCurState;
	}
	else if (mCurState)
	{
		it = std::find_if(mOldStates.begin(), mOldStates.end(), [this](IState* st) {
			return st == mCurState;
		});

		if (it == mOldStates.end())
			mOldStates.push_front(mCurState);
	}

	if (it == mOldStates.end() && mLastSeenRT && to)
		to->enter(*mLastSeenRT);

	mCurState = to;
}