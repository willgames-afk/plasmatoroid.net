use base
<tl add>
<style>
ul.bullet {
	list-style-type: none;
	padding-left: 15px;
}

input[type="checkbox"] {
	position: relative;
	width: 1em;
	height: 1em;
	border: 2px solid #b7f;
	border-radius: 3px;
	outline: 0;
	cursor:default;
	appearance:none;
	background-color: #000;
	border-color: #b7f;
	margin-bottom: 0;
	margin-top: 0;
}

input[type="checkbox"]:checked {
	background-color: #b7f;
}

input[type="checkbox"]:checked::before {
	position: absolute;
	content: "";
	border-width: 0 2px 2px 0;
	transform: rotate(45deg);
	display: block;
	top: -2px;
	left: 3px;
	width: 4px;
	height: 10px;
	border-style: solid;
	border-color: #000;
}

code {
	white-space: pre-wrap;
	font-family: monospace;
	font-size: 15px;
}

</style>
</tl>
<tl content>