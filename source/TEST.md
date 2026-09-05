# Test markdown
This aims to test my Markdown converter.

## Lists
The following should be an unordered list:
* foo
    * this should be 2 levels deep
        (and this should extend the previous paragraph)
    * this should start a new list entry
        (and this should extend it)
* this should go back to the top level
* baz

This should also be an unordered list:
- a
- b
- c

And this as well:
+ thing
+ another thing
+ a third thing

The following should all be checkboxes:
* [ ] `<-` unchecked checkbox
* [] `<-` also unchecked checkbox
* [x] `<-` checked checkbox
- [x] `<-` also checked checkbox
+ [X] `<-` also checked checkbox
+ [a] `<-` also checked checkbox

Lists can have indentation after the dot- this will get removed on rendering
*         this should look normal
*     and this also

List indentation is a bit of a rough subject. To write stuff that's compatible everywhere:
- Don't indent top-level list bullets
- To have multiple lines and/or sublists in a <li>, indent all the lines you want in that li by 4 spaces.
- To put a code block in a <li>, indent all the lines you want in the code block by an additional 4 spaces.
- Don't use very large numbers as <ol> bullets.

Here's how this markdown engine works (for the purposes of processing indentation)
1. Remember the previous indentation level; start at 0.
2. Convert each tab to four spaces
3. Count the number of spaces at the start of each line, then divide by 4 and round down to get the indent level for the current line.
5. If the indent level went UP compared to the previous line, start a code block.
4. If we see a list bullet, add 1 to the indent level for this line.
6. If the indent level went DOWN compared to the previous line, close list elements as necessary.

Invariants
- Top-level list bullets must be indented by 3 or fewer spaces (otherwise they become code blocks)
- Sublists must be indented by between 4 and 7 spaces (inclusive).
    - If they are indented by 3 or fewer spaces, it is considered to be a new list element (not a sublist) because of round-down.
    - If they are indented by 8 or more spaces, it is considered to be a code block.
- Paragraphs must be indented by between 4 and 7 spaces (inclusive).

This:
```
- foo
  bar
  - baz
```
Should obviously be

- foo
    bar
    - baz

and not
- foo
bar
- baz

This
```
100. foo
  bar
  1. baz
```
makes more sense to render flat, but it's still not great

Also, this markdown engine doesn't support doing lists as the first block in a list item:

```
1. - 2. foo 
```
renders as `1. - 2. foo` and not
1. 
    -
        2. foo

## Indented code blocks
If you want to put some code, just slap an indent in front of it:
    this is code
    it is cool

You can put code blocks in lists:
- list element
        code block
    back to list element

done with list

But lists in code blocks aren't formatted
    - this should be a dash, not a dot

    * this should be a star, not a list
        this is indented by four spaces in a code block

    they do still escape <html> tho


## Headings

# this is a h1
## this is a h2
### this is a h3
###### this is a h6
######### this is also an h6

# this is a h1 with no trailing hashtags #####

You can define custom IDs for your headings by putting `{#id}` after your heading:

#### A Heading {#ungabunga}

## Escaping
- the following is not blank and says `<p></p>`: <p></p>
- & becomes an html entity (you may have to look at the html source)
- \*this sentence is not italic, but is enclosed in stars*
- \\ `<-` backslash
- \` `<-` backtick (not in code block)
- \_ `<-` underscore
- \{ `<-` open brace
- \} `<-` close brace
- \[ `<-` open bracket
- \] `<-` close bracket
- \( `<-` open paren
- \) `<-` close paren
- \# `<-` hash
- \+ `<-` plus
- \- `<-` minus/hyphen
- \. `<-` dot
- \! `<-` exclaimation mark
- \a `<-` backslash a
- `` ` `` `<-` single backtick, in code block
- ``` `` ``` `<-` two backticks in a code block
- `\`` `<-` also a singular backtick, in code block
- `\*` `<-` backslash star, in code block
- this * sentence ** has *** a **** bunch ***** of * stars * in * it, but not italics

## Emphasis and friends

Markdown can do 
* *italics* 
* **bold** 
* ***both at the same time***
* italicize just the `ian` in tr*ian*gle, 
* or bold the same thing: tr**ian**gle.
* you can *be in italics, and **add bold, and then remove bold**, and then **add it for a bit**, and then remove italics*.
* *youcanmakepartofaworditalic***andthenswitchtoboldimmediately**
* *if you're in italics **and add bold, you can* remove italics while leaving bold**
*  ~~strikethrough~~
* ~subscript~
* ^superscript^
* You can of course put ~~*italics in strikethrough*~~ and also *~~strikethrough in italics~~* and also **~~strikethrough in bold~~**, etc
* e**v**e**r**y **o**t**h**e**r** XD
* it's smart, so you can talk about A* without having to escape the *, and you can also just use * without escaping it.
* if you put something like 
        |*sentence with *two layers* of stars*|
    then whole thing should be italic: |*sentence with *two layers* of stars*| and not have trailing italics.
* `**This **also** applies to bold.** ->` **this **also** applies to bold** with the same no-trailing thing.
* Any of these formatting choices can also be applied to *[the outside of link text](https://example.com/)* and/or [**the inside of link text**](https://example.com/)


## Links (and Friends)

[Links](https://wikipedia.org/wiki/Hyperlink) can be created like this: `[link text](https://link.com/URL)`.

You can also have a link but store the URL somewhere else- Wherever you want the clickable link, put this:
    [link text][identifier]

and wherever you want to put the URL, put this:
    [identifier]: https://link.com/URL

#### You can put [links](https://wikipedia.org/wiki/Hyperlink) in headings!

[URL]: https://wikipedia.org/wiki/URL

* The [URL][URL] can come before or [after][1] the link that references it, and you can reference the same [URL][URL] multiple times.
* If you use only one box, it will be used both as the link text and as the name of the reference- for instance, [URL].
* If you define the same identifier more than once, it will use the definition closest to the top of the file.
* You can link to a specific heading with `[link text](#heading-id)`.

[1]: https://wikipedia.org/wiki/After

Names for links:
* `[text](url)` - standard link 
* `[text][name]` - full reference
* `[both][]` - also a full reference
* `[][both]` - also a full reference
* `[both]` - shortcut reference

### Footnotes
These are the same as links, more or less[^footnote]. Footnote[^2] definitions are places after the next horizontal line, or at the end of the document.

[^footnote]: except you put a caret in the box and you have to use a reference.
    You can have long footnotes by indenting. Also note that footnote references that are not numbers are converted to numbers.
[^2]: Also, the footnote definition is actually included in the output HTML.

Here's another paragraph with no footnotes above it

* * *

The footnotes should be above this paragraph.

### Embedded Media
If you put a `!` before a link, it will try to embed the linked content instead of just having a link to it. The link text becomes alt text in most cases.
The following rules apply based on the URL:
1. If a website is not specified (ie just `file.png` or `/file.png`), it depends on the mime type of the file
    - image/*, audio/*, and video/* embed the associated HTML element if that mime type is supported by browsers.
    - text/* embeds the associated text file as though it were in a fenced code block.
    - All other mime types give a download link.
2. If it's a youtube.com/v=id link, embed the youtube video player for that video
3. If it's anything else, it will fall back to being a normal link.

This also works with references.

Examples:
![a duck](duck.png)
![Audio File](audio.mp3)
![Video File](video.mp4)
[![An image that is also a link](img.png)](https://example.com)

### Link notes

Link precedence (listed first = parsed first)
1. standard link
2. full reference
3. Footnote
4. shortcut reference

- URLs are not validated- if it sees [](), it's going to treat the stuff in the () like a URL whether it's valid or not.
- If it finds a full reference (`[text][reference]`) but it can't resolve the reference, it tries to reinterperet the first box as a footnote, and then as a shortcut reference.
    - `[ref1][notref][ref2]` is equiv to `[ref1][ref1] [notref][ref2]`
    - `[ref1][ref2][ref3]` is equiv to `[ref1][ref2] [ref3][ref3]`
    - `[notref]` is equiv to `\\[notref\\]`
    - `[notref][]` is equiv to `[notref][notref]`
    - `[both][][huh]` is equiv to `[both][both]\[huh\]`
- Links may contain embedded images, but not any other types of embedded content.


If I end it like this will it be happy?