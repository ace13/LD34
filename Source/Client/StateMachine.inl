template<typename T>
void StateMachine::changeState(bool remove)
{
	IState* toChangeTo = nullptr;
	for (auto& it : mOldStates)
		if (typeid(it) == typeid(T))
			toChangeTo = it;

	if (!toChangeTo)
	{
		toChangeTo = new T();
		toChangeTo->mStateMachine = this;
	}

	changeState(toChangeTo, remove);
}
