1. Starts a list

    1. Starts an indented list (bc we are in list mode)

ok, resetting back to normal

    1. starts a code block bc we went up by more than one indent level

reset again
1. Starts a list.
	More text in that para

	New para
		1. starts a code block in a list (went up by more than one indent level)
		this code block won't end until i go back down to the indent level I started it at
	ok, it's over now

rest once more

1. starts a list
	1. more indent
		1. more indent
			1. more indent
		> BOLK
		1. mmm
	2. goes back to the level where you would expect it to be

- ilevel 1; delta +1
	- ilevel 2; delta +1
		- ilevel 3; delta +1
		- still ilevel 3; delta 0
			- ilevel 4; delta +1
	- ilevel 2; delta -2

- I'm curious
	- what this does
	1. thing
	2. thing

reset again
> blockquote
> > nested blockquote
> >
> > ooh fancy
>>
 >> still works
>                                             > this also still works (weirdly)
>>       but this doesn't