# TLC Docs

## Overview

TLC is a tool for making websites. It provides templating, markdown rendering,
and server-side scripting using C.

### Philosophy
TLC was born out of a hatred for javascript and a distaste for excessive
dependancies. So, TLC is very small, it is written entirely in C, and it is
scripted entirely in C.

***

## Functions

### Use Header
To pick a template to use for a specific page, add `use templatename` to the 
first line of a TL file. If no use header is present, TLC will default to using
`base.tl` for `.tl` files and `md.tl` for `.md` files.

### TL tags
All TLC does is replace `<tl>` tags with various other content. That content
can come from several different places and so there are a few variations of the
`<tl>` tag.
* `<tl name>` Insert some other file into this one.
* `<tl dy name>` On page load, run a C script and put the results here.
* `<tl add>...</tl>` Add the contents of this tag to the head of the parent
	template.

When files are inserted (with `<tl name>`), they are preprocessed depending on 
their extension:
* `.tl` files are rendered
* `.c` files are compiled and run and their output is inserted
* all other files are output verbatim

### Templates 
TLC templates must have a `<tl content>` tag, where the content of the page is
inserted, and a `<tl head>` tag, where resources intended for the head of the
document (stylesheets, etc) are inserted.

## TLC-flavored Markdown
[Markdown]: https://daringfireball.net/projects/markdown/
TLC has its own blend of [markdown][], because why not. It supports:
* Headings (`# foo`), with automatic or manual html id assignment.
* Italic (`*`)
* Bold (`**`)
* Superscript (`^`)
* Subscript (`~`)
* Strikethru (`~~`)
* Blockquotes (`>`)
* Ordered Lists (`1.`)
* Unordered Lists (`-`,`+`,`*`)
* Task Lists (`- [x]`)
* Inline Code (`` `code` ``)
* Fenced code blocks
		```js
		console.log("Hellorld!");
		```
* Links (`[display text](https://example.com)`)
* Images (`![alt text](image.jpg)`)
* Download links (`!d[display text](file.txt)`)
* Audio (`!a[alt text](audio.mp3)`)
* Video (`!v[alt text](video.mp4)`)
* Footnotes (`[^footnote]`)
* References (`[foo]: https://example.com optional title here`)

It does not support:
* Autolinks, even if wrapped in `<>`, as this is redundant and unnecessary.
* Sextet-style headings (`---`, `===`)- these require excessive lookahead to parse.
* Raw HTML- raw HTML presents both minor safety issues and is extra work to parse.
* Lazy hard-wrapped text- you must put a `>` before every line in a blockquote and 
	you must indent every line that is part of a list element. This makes parsing
	easier and more consistent.

### Common questions
This list is based off the [Why is a spec needed?][] section of the Github-flavored Markdown spec.

[Why is a spec needed?]: https://github.github.com/gfm/#why-is-a-spec-needed-
1. 
	#### How much indentation is needed for a sublist?
	4 spaces, universally. TLC-flavored markdown converts spaces to tabs by
	dividing by four and rounding down, and that is used as the indentation
	level. This is the fabled "four spaces rule".

2. 
	#### Is a blank line needed before a blockquote or heading?
	No.

3. 
	#### Is a blank line needed before an indented code block?
	No.

4. 
	#### What is the exact rule for determining whether list items get wrapped in `<p>` tags?
	TLC will always *start* by producing a tight list; if you just put text in
	a list it will always be tight. However, if a heading or other block-level 
	construct is used, it will revert to wrapping future text in paragraphs. 
	There is no way to get TLC to wrap the entire contents of a list in a `<p>` 
	tag.

5. 
	#### Can list markers be indented? Can ordered list markers be right-aligned?
	Yes, as long as the number of spaces required is 3 or less.
	
	TLC's indentation is done by dividing the number of spaces by four and
	rounding down, so you are allowed to indent list markers by 3 spaces or
	fewer without adverse effects. This means that aligning list markers 
	from 1 to 9999 is possible.

6. 
	#### How do thematic breaks play with list items?
	Thematic breaks take priority over lists, so the following is interpereted
	as two lists seperated by a thematic break.
	```md
	* a
	* * * * *
	* b
	```

7. 
	#### When list markers change from numbers to bullets, do we have 2 lists or one?
	Two seperate lists; one ordered and one unordered.

8. 
	#### What are the precedence rules for inline structure?
	TODO

9. 
	#### What are the precedence rules for markers of emphasis and strong emphasis?
	TLC does not maintain a node higherarchy for any formatting markup 
	(emphasis, strong, strikethrough, subscript, superscript, etc). Instead, it
	tracks the transitions between various markup states and generates whatever
	HTML is required to perform that transition. For example:
	```md
	*foo **bar* baz**
	```
	results in
	```html
	<em>foo<strong>bar</strong></em><strong>baz</strong>
	```
	This is both more expressive and easier to parse, while remaining largely compatible
	with most markdown implementations, so I like it.

10.  
	#### What are the precedence rules between block-level and inline-level structures?
	TODO

11. 
	#### Can list items include headings?
	Yes, as long as they are not on the first line of the list. This is not a heading:
	```md
	1. # heading
	```
	This is a heading:
	```md
	1. 
		# heading
	```

12. 
	#### Can list items be empty?
	Yes! I don't know why you'd want this, but yes they can be empty.

13. 
	#### Can link references be defined inside blockquotes or list items?
	Yes! You can put link reference definitions wherever you want, and you can 
	indent them however you want, and they will not screw up the layout of your
	document. This applies to footnote definitions as well.

14. 
	#### If there are multiple definitions for the same reference, which one wins?
	There are 2 rules for this. First, a definition within the current
	blockquote takes priority over one outside the current blockquote. This
	takes priority over the second rule, which is that the first definition for
	a particular reference takes priority.

	This is so that you can *always* embed any TLC document into another one
	simply by copypasting it and enclosing it in a blockquote.


# this is a ############################################ degenerate heading ##   
- - - - -
***
* * * * 
___
_ _ _ _ _ _ _
* * * - - - * * *
- - - _

_ _ _ _ _ _ _ oh, you thought this was a thematic break? hah! how foolish.