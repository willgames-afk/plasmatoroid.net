# How to highlight?

Works on Modes. Modes have
- Start condition
- End condition (optional)
- List of contained submodes

You can define a mode regex style:

	block begin end
	string " "
	comment // \n
	c-block { }

If your mode is simple enough, you don't have
to put a closing regex:

	number [0-9]+(.[0-9]+)?


If you want to match a space character in a regex,
wrap it in a character class: `[ ]`.

	whitespace [ ]

Alternatively, you can define a mode keyword-style:

	keyword+ if else or

(this just looks a bit nicer than a bazillion `|` seperators)

If your keywords contain spaces, you can do `++` and wrap in
doublequotes.

	keyword++ "this has spaces" "this also has spaces" "why"

Submodes are created with braces:
	mode start end {
		submode ...
		submode ...
		submode {
			subsubmode ...
			etc
		}
	}

The mode named `_` is reserved, and represents an anonymous mode.
This is equivalent to a submode but generates no markup.

You can assign a regex a name by prefixing it with `$`
	$myregex helloworld

and then use it later:
	mylang {
		hi $myregex
	}

You can do the same thing but with language modes with `$$`

	html {
		...
		$$js
		$$css
	}

You can embed a mode inside itself with `$$$`

	obj { } {
		$$$
		string $cstring
	}

finally, you can retroactively append a submode to a mode with 
the following:
	comment.doctag+ TODO: FIXME: NOTE: BUG: OPTIMIZE: HACK: XXX:

Language def for brainfuck:

	brainfuck {
		literal [+-]
		comment [^\[\]\.,\+\-<> \r\n] [\[\]\.,\+\-<> \r\n]
		  title [\[\]]
		 string [\.,]
	}

Language def for C:
	c {
		functiontype $ident
		type {
			_ \b[a-z0-9_]*_t\b
			_ \batomic_[a-z]{3,6}\b
		}
		string (u8?|U|L)?" " {

		}
	}

	$shebang ^#![ ]*/
	$ident [a-zA-Z]\w*
	$uident [a-zA-Z_]\w*
	$backslashEsc \\[\s\S]

Only a subset of regexes are supported:
	[a-z0-9] \w \b \d \W \B \D . 
	^ $ start/end of line
	+ * ? {,} quantifiers
	| alternation 
	() grouping

	Additionally (and this is important): Regexes are NOT matched greedily



The following is a list of stylable scopes
Basic:
- literal- built-in value, (true, false, null)
- number- numeric literal
- string- string literal
- variable- variable
- keyword
- comment

- builtin- library object (constant, class, function)
- type- data type (int, array, etc)
- regex- regular expression literal
- variable.language- variable with special language features (ie this, self, window, super, etc)
- variable.constant- a constant.
- doctag- this is documentation markup inside comments, ie @params
- title- name of a class or function
- title.class
- title.function
- title.function.call
- escape- escape character, ie \r \n etc
- symbol- symbolic constant, goto label, etc
- meta- flags, modifiers, annotations, processing instructions, etc
- operator (+ - * / >> | ==)
- punctuation- stuff that should be subtly highlighted, ie () 

- heading
- tag
- name (of an XML tag, or first word in S-exp)
- attr (ie keys in json)
- attribute 

- bullet (ie markdown list bullets)
- code
- emphasis
- strong
- formula
- link
- quote

- selector-tag
- selector-id
- selector-class
- selector-attr
- selector-pseudo

- addition
- deletion