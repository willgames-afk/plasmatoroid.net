# How to do regex matching?
- Convert regex text- ie, `(a|b*)+`, into an e-NFA
- Reduce e-NFA to normal NFA.
	- For some target state A, take a state that you can get to using epsilon moves and call it B
	- Take all the moves that start from B and copy them to A (ie, if B: 'a' -> D is a move, add A: 'a' -> D)
	- Delete the epsilon move from A to B
	- Repeat this until there are no more epsilon moves.
- Reduce normal NFA to DFA
	- Take some target state A
	- For each possible input, look at the possible output states. If any input has more than 
		one possible output state, create a new state that is idenified by the set of the possible output states.
		for example, if we have
			a: '0' -> b
			a: '0' -> a
			a: '1' -> a
		there are 2 possible transitions for '0', so we create a new state {a, b}
	- The transitions for states created that way are found by unioning; so like, if we want the possible states for
		{a,b} given input '0', you'd take the possible states for a given input '0' and add the possible states for b given input '0' and that would be the new result
	- If a new state contains a final state, it is also a final state
	- go until no new states are created 
	- This might give you
		- Unreachable states- these should probably be removed
		- Dead states- these cannot lead to a match and they should really be removed as 
			leaving them in means we might get stuck spinning around forever or something
		- Equivalent states- this is 2 states with the exact same transition rules. These
			should probably be combined
- Run the DFA
