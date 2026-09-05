# Markdown Todo

* [x] support checkboxes in lists
* [x] allow closing `#`-style headers (ie `### heading ###` should render as `<h3>heading</h3>`)
* [x] automatic IDs for headings
* [ ] manual IDs for headings
* [ ] only allow ordered lists to interrupt paragraphs if they start from `1` (hard-wrapped numeral safety).
* [x] both `[text][]` and `[][text]` should be equivalent to `[text][text]`
* [ ] link tooltips with `[text](https://example.com "the example website")`
* [x] references should be caps insensitive
* [ ] scan ahead for references if a definition cannot be found
* [x] strikethru
* [ ] superscript
* [x] thematic breaks
* [ ] audio embeds (`<audio></audio>`)
* [ ] video embeds (`<video></video>`)
* [ ] fenced code blocks
* [ ] fenced code blocks shouldn't output the language to the output HTML
* [ ] fenced code blocks need something to distinguish them from inline code for styling purposes
* [ ] syntax highlighting on code blocks
* [ ] test suite
* [x] bug: `</li>` is always emitted in pairs

## Won't Fix

* Indentation follows the 4-spaces rule.
* The formatting rules for doing italics, bold, and strikethrough are as intended.
* You can't put multiple list bullets one one line in the markdown source
* sextet headers (`===` or `---` underneath heading text)
* Fenced code blocks with `~~~`