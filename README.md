# SEML

This is a minimal command line utility for formatting plain text into simple `.eml` files with HTML markup.

## Example

Here is an example of a SEML source text. It has similarities to Markdown.

```
Dear Daria,

Are you tired of writing simply-formatted emails in Google Docs, GMail, etc. that still seem not to show up properly on every device you read them on?

! Here are a few things you can do:

* Use SEML
* Use SEML more
* Use SEML all the time

Sincerely,
Toby
```

Here is how you can format it into a `.eml` file using SEML:

```sh
seml < sample.txt > sample.eml
```

The output look like this:

```html
Content-Type: text/html; charset="utf-8"

<html>
<head>
</head>
<body>
<p>Dear Daria,
<p>Are you tired of writing simply-formatted emails in Google Docs, GMail, etc. that still seem not to show up properly on every device you read them on?
<p><strong>Here are a few things you can do:</strong>
<ul>
<li>Use SEML
<li>Use SEML more
<li>Use SEML all the time
</ul>
<p>Sincerely,<br>Toby
</body>
</html>
```

## Getting started

After cloning,

```sh
make
bin/seml sample.txt
```

## License

[MIT License](LICENSE). Go wild.
